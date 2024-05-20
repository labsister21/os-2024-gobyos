#include "header/scheduler/scheduler.h"
#include "header/process/process.h"
#include "header/text/framebuffer.h"
#include "header/kernel-entrypoint.h"


__attribute__((noreturn)) extern void process_context_switch(struct ProcessContext ctx);

void scheduler_init(void){
    activate_timer_interrupt();
    paging_use_page_directory(_process_list[0].context.page_directory_virtual_addr);
    _process_list[0].metadata.state = PROCESS_RUNNING;
}

void scheduler_save_context_to_current_running_pcb(struct ProcessContext ctx){
    struct ProcessControlBlock *current_pcb = process_get_current_running_pcb_pointer();
    memcpy(&(*current_pcb).context,&ctx, sizeof(struct ProcessContext));
}

__attribute__((noreturn)) void scheduler_switch_to_next_process(void){
    int i;
    for (i = 0; i < PROCESS_COUNT_MAX; i++){
        if (_process_list[i].metadata.state == PROCESS_RUNNING){
             _process_list[i].metadata.state = PROCESS_READY;
            break;
        }
    }
    
    if (i == PROCESS_COUNT_MAX){

    }
    
    i = (i + 1)% 16;
    bool found = false;
    while (!found){
        if (_process_list[i].metadata.state == PROCESS_READY){
             found = true;
        } else{
             i = (i + 1) % 16;
        }
 
    }
   ;
    _process_list[i].metadata.state = PROCESS_RUNNING;
    paging_use_page_directory(_process_list[i].context.page_directory_virtual_addr);
    pic_ack(IRQ_TIMER + PIC1_OFFSET);
    process_context_switch(_process_list[i].context);
}
