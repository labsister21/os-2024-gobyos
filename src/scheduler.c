#include "header/scheduler/scheduler.h"
#include "header/process/process.h"
#include "header/text/framebuffer.h"
#include "header/kernel-entrypoint.h"


// Scheduler state
static int current_process_index = -1;

// Initialize the scheduler
void scheduler_init(void) {
    current_process_index = 0;
    process_manager_state.current_running_process_index = current_process_index;
    
    
    // Ensure the first process is in the correct state
    if (process_manager_state.active_process_count > 0) {
        _process_list[current_process_index].metadata.state = PROCESS_RUNNING;
    }

}


// Save the current context to the currently running PCB
void scheduler_save_context_to_current_running_pcb(struct ProcessContext ctx) {
    if (current_process_index >= 0 && current_process_index < PROCESS_COUNT_MAX) {
        _process_list[current_process_index].context.cpu = ctx.cpu;
        _process_list[current_process_index].context.page_directory_virtual_addr = ctx.page_directory_virtual_addr;
    }
}

// Find the next process based on priority
int scheduler_find_next_process(void) {
    uint32_t highest_priority = -1;
    int next_process_index = -1;

    for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
        if (_process_list[i].metadata.state == PROCESS_READY) {
            if (_process_list[i].metadata.priority > highest_priority) {
                highest_priority = _process_list[i].metadata.priority;
                next_process_index = i;
                
            }
        }
    }

    return next_process_index;
}


void running_to_ready() {
    for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
        if (_process_list[i].metadata.state == PROCESS_RUNNING) {
           _process_list[i].metadata.state = PROCESS_READY;
           break;    
        }
    }
}


// Switch to the next process
void scheduler_switch_to_next_process(void) {
    if (process_manager_state.active_process_count <= 0) {
        framebuffer_write(6, 8,  'a', 0, 0xF);
        while (1) { asm volatile ("hlt"); }
    }
    int next_process_index;
    if (process_manager_state.active_process_count == 1) {
        next_process_index = current_process_index;
    }else{
        next_process_index = scheduler_find_next_process();
    }
    if (next_process_index == -1) {
        framebuffer_write(8, 8,  'n', 0, 0xF);
        while (1) { asm volatile ("hlt"); }
    }

    current_process_index = next_process_index;
    process_manager_state.current_running_process_index = current_process_index;
    
    process_context_switch(_process_list[current_process_index].context);  
}

