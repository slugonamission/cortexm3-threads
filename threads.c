#include "threads.h"
#include <lpc17xx_timer.h>
#include <stdlib.h>
#include "usbcon.h"

#define MAX_TASKS 10
#define STACK_SIZE 1024

typedef struct {
    void* stack;
    void* orig_stack;
    uint8_t inUse;
} tcb_t;

tcb_t* tasks;
int lastTask;
uint8_t running;
uint8_t enable_cs;

void __attribute__((naked)) TIMER0_IRQHandler()
{
    // Clear the interrupt flag
    LPC_TIM0->IR |= TIM_IR_CLR(TIM_MR0_INT);

    if(enable_cs)
    {
        // if we're not running, PSP won't be assigned and we'll hard-fault.
        if(running)
        {
            // Save the registers
            __asm__("MRS r12, PSP;"
                    "STMDB r12!, {r4-r11,LR};"
                    "MSR PSP, r12;"
                );
            // Save out the stack pointer
            tasks[lastTask].stack = (void*)__get_PSP();
        }
        
        // Find a new task to run
        lastTask++;
        while(1)
        {
            if(lastTask == MAX_TASKS)
                lastTask = 0;
            if(tasks[lastTask].inUse)
            {
                // Restore the PSP
                __set_PSP((uint32_t)tasks[lastTask].stack);
                running = 1;
                break;
            }
            lastTask++;
        }
    
        // Restore registers, restore PSP and jump back
        __asm__("MRS r12, PSP;");
        __asm__("LDMFD r12!, {r4-r11,LR};");
        __asm__("MSR PSP, r12;");
    }
    __asm__("BX lr;");
    __asm__("NOP;");
}

void thread_init(uint32_t quantaUs)
{
    // Set up the timers
    TIM_TIMERCFG_Type timCfg;
    TIM_MATCHCFG_Type matCfg;
    int i;

    lastTask = 0;
    running = 0;
    enable_cs = 0;

    timCfg.PrescaleOption = TIM_PRESCALE_USVAL;
    timCfg.PrescaleValue = 1;
    
    matCfg.MatchChannel = 0;
    matCfg.IntOnMatch = TRUE;
    matCfg.ResetOnMatch = TRUE;
    matCfg.StopOnMatch = FALSE;
    matCfg.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    matCfg.MatchValue = quantaUs;
    
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timCfg);
    TIM_ConfigMatch(LPC_TIM0, &matCfg);
    
    NVIC_SetPriority(TIMER0_IRQn, ((0x01 << 3) | 0x01));
    NVIC_EnableIRQ(TIMER0_IRQn);
    
    
    // Set up the storage space for the TCBs
    tasks = (tcb_t*)malloc(sizeof(tcb_t) * MAX_TASKS);
    
    for(i = 0; i < MAX_TASKS; i++)
    {
        tasks[i].stack = 0x00;
        tasks[i].inUse = 0;
    }
}

void thread_go()
{
    uint32_t msp;
    //usbcon_write("Starting threading system\r\n");

    // Just incase...
    msp = __get_MSP();
    __set_PSP(msp);
    
    TIM_Cmd(LPC_TIM0, ENABLE);

    // Enable PSP
    uint32_t CONTROL = __get_CONTROL();    

    CONTROL |= 0x02;
    __set_CONTROL(CONTROL);
    __ISB();
    
    enable_cs = 1;

    // Hang and wait to be pre-empted
    while(1);
}

int thread_create(void (*run)(void*), void* userdata)
{
    // Find a free thing
    int threadId = 0;
    uint32_t* stack;
    
    for(threadId = 0; threadId < MAX_TASKS; threadId++)
    {
        if(tasks[threadId].inUse == 0)
            break;
    }
    
    if(threadId == MAX_TASKS)
        return -1;
    
    // Create the stack
    stack = (uint32_t*)malloc(STACK_SIZE);
    tasks[threadId].orig_stack = stack;
    if(stack == 0)
        return -1;
    
    // Remember...stacks grow downwards
    stack = (uint32_t*)((uintptr_t)stack + STACK_SIZE);
    
    // Init the stack according to the calling conventions
    *(--stack) = 0x21000000; // xPSR - apparently this is the default value
    *(--stack) = (uint32_t)run; // Program counter
    *(--stack) = (uint32_t)&thread_self_term; // Link register. Don't bother with it for now
    *(--stack) = 0x00; // r12
    *(--stack) = 0x00; // r3
    *(--stack) = 0x00; // r2
    *(--stack) = 0x00; // r1    
    *(--stack) = (uint32_t)userdata; // r0
    *(--stack) = 0xFFFFFFFD; // LR
    *(--stack) = 0x00; // r11
    *(--stack) = 0x00; // r10
    *(--stack) = 0x00; // r9
    *(--stack) = 0x00; // r8
    *(--stack) = 0x00; // r7
    *(--stack) = 0x00; // r6
    *(--stack) = 0x00; // r5
    *(--stack) = 0x00; // r4
    
    // Create the control block
    tasks[threadId].stack = stack;
    tasks[threadId].inUse = 1;

    return threadId;
}

void thread_kill(int thread_id)
{
    tasks[thread_id].inUse = 0;
    
    // Free the stack
    free(tasks[thread_id].orig_stack);
}

void thread_self_term()
{
    usbcon_write("Terminating thread\r\n");
    
    // This will kill the stack. For now, disable context switches to save ourselves.
    enable_cs = 0;
    thread_kill(lastTask);
    usbcon_write("Thread killed\r\n");
    enable_cs = 1;

    // And now wait for death to kick in
    while(1);
}
