#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"
#include "header/filesystem/fat32.h"

// Constants
#define PROCESS_COUNT_MAX 256
#define PROCESS_NAME_LENGTH_MAX 32
#define PROCESS_PAGE_FRAME_COUNT_MAX 1024
#define PROCESS_CREATE_SUCCESS 0
#define PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED -1
#define PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT -2
#define PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY -3
#define PROCESS_CREATE_FAIL_FS_READ_FAILURE -4

#define KERNEL_VIRTUAL_ADDRESS_BASE 0xC0000000
#define CPU_EFLAGS_BASE_FLAG 0x00000200
#define CPU_EFLAGS_FLAG_INTERRUPT_ENABLE 0x00000200

// Enum for process state
typedef enum {
    PROCESS_TERMINATED,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED
} ProcessState;

// Structure for process metadata
typedef struct {
    uint32_t pid;
    ProcessState state;
    char name[PROCESS_NAME_LENGTH_MAX];
    uint32_t priority;
} ProcessMetadata;

// Structure for CPU context
typedef struct {
    uint32_t eip;
    uint32_t esp;
    uint32_t eflags;
    uint32_t cs;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t ss;
} CPUContext;

// Structure for process context
typedef struct {
    CPUContext cpu;
    struct PageDirectory* page_directory_virtual_addr; // Change from uint32_t to struct PageDirectory*
} ProcessContext;

// Structure for process control block
typedef struct ProcessControlBlock {
    ProcessMetadata metadata;
    ProcessContext context;
} ProcessControlBlock;

// Structure for process manager state
typedef struct {
    int active_process_count;
    int current_running_process_index;
    int last_pid;
} ProcessManagerState;

// External variables
extern ProcessManagerState process_manager_state;
extern ProcessControlBlock _process_list[PROCESS_COUNT_MAX];

// Function prototypes
int32_t process_create_user_process(struct FAT32DriverRequest request);
struct ProcessControlBlock *process_get_current_running_pcb_pointer(void);
bool process_destroy(uint32_t pid);
uint32_t ceil_div(uint32_t numerator, uint32_t denominator);
int32_t process_list_get_inactive_index(void);
int32_t process_generate_new_pid(void);
int get_process_metadata(char *buf);

// Add uint_to_str declaration
void uint_to_str(uint32_t num, char* str);

#endif // PROCESS_H