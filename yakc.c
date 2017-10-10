#include "clib.h"
#include "yaku.h"
#include "yakk.h"

//Defines
#define LOWEST_PRIORITY 100
#define INITIAL_FLAGS 0x0020

//Local prototypes
void make_ready_task(tcb_t* tcb);

void YKScheduler(void);
void YKDispatcher(struct tcb* tcb);
void YKIdleTask(void);

tcb_t* active_task = 0;
tcb_t* ready_task = 0;
tcb_t* blocked_list = 0;

tcb_t tcb_memory[MAX_TCB_COUNT];
tcb_t* next_tcb_ptr = tcb_memory;
int task_id_count = 0;

int idle_stack[STACK_SIZE];

int kernel_running = 0;

int YKIdleCount = 0;
int YKCallDepth = 0;
int YKCtxSwCount = 0;

void YKInitialize(void){
  tcb_t* idle = next_tcb_ptr++;
  idle->id = task_id_count++;
  idle->state = 1;
  idle->priority = LOWEST_PRIORITY;
  idle->next = 0;
  idle->sp = &idle_stack[STACK_SIZE];
  idle->ss = 0;
  //  *(idle->sp - 1) = INITIAL_FLAGS;
  *(idle->sp - 2) = 0;//bx
  *(idle->sp - 1) = (int) YKIdleTask;//start point
  idle->sp -= 1*sizeof(int);
  ready_task = idle;


     //create idle task tcb and put it in the ready queue
     //??
     //profit
}

void YKIdleTask(void){
     while(1){
        YKIdleCount++;	//Make sure this loop is 4 instructions
     }
}

void YKNewTask(void (* task)(void), void *taskStack, unsigned char priority){
  tcb_t* tcb;
  YKEnterMutex();
  tcb = next_tcb_ptr++;
  tcb->id = task_id_count++;
  tcb->state = 1;
  tcb->priority = priority;
  tcb->next = 0;
  tcb->sp = (int*)((char*)taskStack);
  tcb->ss = 0;
  //  *(tcb->sp - 1) = INITIAL_FLAGS;
  //  *(tcb->sp - 2) = 0;
  *(tcb->sp - 2) = (int) 0;//bx
  *(tcb->sp - 1) = (int) task;//start point
  tcb->sp -= 1 * sizeof(int);

  make_ready_task(tcb);
  YKExitMutex();

  YKScheduler();
  //Create and initialize TCB
     //Store initial flags and IP to stack
     //
}

void YKRun(void){
     //Calls the scheduler, sets global variable to activate user tasks
  kernel_running = 1;
  YKScheduler();
}

void YKDelayTask(unsigned count){
     //If count == 0; return
     //Remove TCB from ready queue
     //Add TCB to blocking queue with wait count

}

void YKEnterISR(void){
     //Increment call depth counter
  YKCallDepth++;
}

void YKExitISR(void){
     //Decrement call depth counter
     //If the call depth is 0, call scheduler
     //Return
  if((--YKCallDepth) == 0){
    YKScheduler();
  }
}

void YKScheduler(void){
  if(!kernel_running || active_task == ready_task){
    return;
  }
  //YKEnterMutex();//TODO necessary?
  YKDispatcher(ready_task);
  //YKExitMutex();
  return;
  
     //Disable interrupts
     //Check ready queue for highest priority task
     //Update active TCB pointer
     //if it differs from current, call dispatcher
     //else, enable interrupts and return
}

/*void YKDispatcher(struct tcb* tcb){
     //Restore registers from tcb
     //
     //
     //iret
     }*/

void make_ready_task(tcb_t* tcb){
  tcb_t* current;

  if(tcb->priority < ready_task->priority){
    tcb->next = ready_task;
    ready_task = tcb;
    return;
  }

  current = ready_task;
  //  YKEnterMutex();//Necessary?
  while(current->next && (current->next->priority < tcb->priority)){
    current = current->next;
  }
  tcb->next = current->next;
  current->next = tcb;
  //  YKExitMutex();//Necessary?
  return;
}
