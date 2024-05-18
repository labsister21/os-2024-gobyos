#include "header/process/process.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"

ProcessManagerState process_manager_state = {0, -1, 0};
ProcessControlBlock _process_list[PROCESS_COUNT_MAX]; // Static array of PCBs

int32_t process_create_user_process(struct FAT32DriverRequest request)
{
    int32_t retcode = PROCESS_CREATE_SUCCESS;
    if (process_manager_state.active_process_count >= PROCESS_COUNT_MAX)
    {
        retcode = PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED;
        goto exit_cleanup;
    }

    // Ensure entrypoint is not located at kernel's section at higher half
    if ((uint32_t)request.buf >= KERNEL_VIRTUAL_ADDRESS_BASE)
    {
        retcode = PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT;
        goto exit_cleanup;
    }

    // Check whether memory is enough for the executable and additional frame for user stack
    uint32_t page_frame_count_needed = ceil_div(request.buffer_size + PAGE_FRAME_SIZE, PAGE_FRAME_SIZE);
    if (!paging_allocate_check(page_frame_count_needed) || page_frame_count_needed > PROCESS_PAGE_FRAME_COUNT_MAX)
    {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    // Process PCB
    int32_t p_index = process_list_get_inactive_index();
    struct ProcessControlBlock *new_pcb = &(_process_list[p_index]);

    new_pcb->metadata.pid = process_generate_new_pid();
    new_pcb->metadata.state = PROCESS_READY;
    memcpy(new_pcb->metadata.name, request.name, PROCESS_NAME_LENGTH_MAX);
    new_pcb->metadata.priority = 1;                                                    
    new_pcb->context.page_directory_virtual_addr = paging_create_new_page_directory(); 

    struct PageDirectory* currentPD = paging_get_current_page_directory_addr();
    paging_allocate_user_page_frame(new_pcb->context.page_directory_virtual_addr, request.buf);
    paging_use_page_directory(new_pcb->context.page_directory_virtual_addr);

    int8_t return_c = read(request);

    if (return_c != 0)
    {
        paging_use_page_directory(currentPD);
        bool success = process_destroy(new_pcb->metadata.pid);
        if (success)
        {
            paging_free_user_page_frame(new_pcb->context.page_directory_virtual_addr, request.buf);
        }

        retcode = PROCESS_CREATE_FAIL_FS_READ_FAILURE;
        goto exit_cleanup;
    }

    new_pcb->context.cpu.eflags = CPU_EFLAGS_BASE_FLAG | CPU_EFLAGS_FLAG_INTERRUPT_ENABLE;
    new_pcb->context.cpu.esp = 0x400000 - 4;
    new_pcb->context.cpu.cs = 0x1B;
    new_pcb->context.cpu.ds = 0x23;
    new_pcb->context.cpu.es = 0x23;
    new_pcb->context.cpu.fs = 0x23;
    new_pcb->context.cpu.gs = 0x23;
    new_pcb->context.cpu.ss = 0x23;

    process_manager_state.active_process_count++;

exit_cleanup:
    return retcode;
}

struct ProcessControlBlock *process_get_current_running_pcb_pointer(void)
{
    if (process_manager_state.current_running_process_index == -1)
    {
        return NULL;
    }
    return &_process_list[process_manager_state.current_running_process_index];
}

bool process_destroy(uint32_t pid)
{
    for (int i = 0; i < PROCESS_COUNT_MAX; i++)
    {
        if (_process_list[i].metadata.pid == pid && _process_list[i].metadata.state != PROCESS_TERMINATED)
        {
            paging_free_page_directory(_process_list[i].context.page_directory_virtual_addr);

            memset(&_process_list[i], 0, sizeof(ProcessControlBlock));
            _process_list[i].metadata.state = PROCESS_TERMINATED;

            process_manager_state.active_process_count--;

            if (process_manager_state.current_running_process_index == i)
            {
                process_manager_state.current_running_process_index = -1;
            }
            return true;
        }
    }
    return false;
}

uint32_t ceil_div(uint32_t numerator, uint32_t denominator)
{
    return (numerator + denominator - 1) / denominator;
}

int32_t process_list_get_inactive_index(void)
{
    for (int i = 0; i < PROCESS_COUNT_MAX; i++)
    {
        if (_process_list[i].metadata.state == PROCESS_TERMINATED)
        {
            return i;
        }
    }
    return -1;
}

int32_t process_generate_new_pid(void)
{
    uint32_t new_pid;

    do
    {
        new_pid = ++process_manager_state.last_pid;
        for (int i = 0; i < PROCESS_COUNT_MAX; i++)
        {
            if (_process_list[i].metadata.pid == new_pid && _process_list[i].metadata.state != PROCESS_TERMINATED)
            {
                new_pid = 0;
                break;
            }
        }
    } while (new_pid == 0);

    return new_pid;
}

// Define uint_to_str function without using sprintf
void uint_to_str(uint32_t num, char* str) {
    int i = 0;
    int len = 0;
    uint32_t temp = num;

    // Find out the number of digits in num
    do {
        len++;
        temp /= 10;
    } while (temp != 0);

    // Null-terminate the string
    str[len] = '\0';

    // Populate the string with digits
    for (i = len - 1; i >= 0; i--) {
        str[i] = '0' + (num % 10);
        num /= 10;
    }
}

int get_process_metadata(char *buf)
{
    int idx = buf[0];
    if (idx < 0 || idx >= PROCESS_COUNT_MAX)
        return -1;
    ProcessControlBlock *pcb = &_process_list[idx];
    if (pcb->metadata.state == PROCESS_TERMINATED)
        return -1;

    static char pid_str[11];
    uint_to_str(pcb->metadata.pid, pid_str);

    char *ptr = buf;
    for (int i = 0; pid_str[i] != '\0'; i++)
        *ptr++ = pid_str[i];
    *ptr++ = ' ';
    for (int i = 0; pcb->metadata.name[i] != '\0'; i++)
        *ptr++ = pcb->metadata.name[i];
    *ptr++ = '\n';
    *ptr = '\0';

    return 0;
}
