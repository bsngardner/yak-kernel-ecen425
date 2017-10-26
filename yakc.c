#include "clib.h"
#include "yaku.h"
#include "yakk.h"

//Defines
#define LOWEST_PRIORITY 100
#define INITIAL_FLAGS 0x0200
#define INTSIZE 2

//Local prototypes
void make_ready_task(tcb_t* tcb);
void block_task(tcb_t* task);

void YKScheduler(void);
void YKDispatcher(void);
void YKIdleTask(void);

tcb_t init_tcb;
tcb_t tcb_memory[MAX_TCB_COUNT];
tcb_t* next_tcb_ptr = tcb_memory;
int task_id_count = 0;

tcb_t* active_task = &init_tcb;
tcb_t* ready_task = 0;
tcb_t* blocked_task = 0;

int idle_stack[STACK_SIZE];

int kernel_running = 0;

int YKIdleCount = 0;
int YKCallDepth = 0;
int YKCtxSwCount = 0;
int YKTickNum = 0;

void YKInitialize(void){
  tcb_t* idle = next_tcb_ptr++;
  idle->id = task_id_count++;
  idle->state = 1;
  idle->priority = LOWEST_PRIORITY;
  idle->next = 0;
  idle->sp = &idle_stack[STACK_SIZE];
  idle->ss = 0;
  //  *(idle->sp - 1) = INITIAL_FLAGS;
  *(idle->sp - 5) = 0;//bx
  *(idle->sp - 4) = 0;//bp
  *(idle->sp - 3) = (int) YKIdleTask;//start point
  *(idle->sp - 2) = 0;//cs register
  *(idle->sp - 1) = INITIAL_FLAGS;
  idle->sp -= 3*INTSIZE - 1;
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
  tcb->tick_num = 0;
  //  *(tcb->sp - 1) = INITIAL_FLAGS;
  //  *(tcb->sp - 2) = 0;
  *(tcb->sp - 5) = (int) 0;//bx
  *(tcb->sp - 4) = (int) 0;//bp
  *(tcb->sp - 3) = (int) task;//start point
  *(tcb->sp - 2) = 0;//cs register
  *(tcb->sp - 1) = INITIAL_FLAGS;//flags
  tcb->sp -= 3 * INTSIZE - 1;

  make_ready_task(tcb);

  YKScheduler();

  YKExitMutex();
  //Create and initialize TCB
     //Store initial flags and IP to stack
     //
}

void YKRun(void){
     //Calls the scheduler, sets global variable to activate user tasks
  kernel_running = 1;
  YKEnterMutex();
  YKScheduler();
  YKExitMutex();
}

void YKDelayTask(unsigned count){
     //If count == 0; return
     //Remove TCB from ready queue
     //Add TCB to blocking queue with wait count
  tcb_t* task;
  if(!count)
    return;
  YKEnterMutex();
  task = active_task;
  ready_task = task->next;
  task->tick_num = YKTickNum + count;
  block_task(task);
  YKScheduler();
  YKExitMutex();
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

void YKTickHandler(void){
  tcb_t* task;
  YKTickNum++;
  while(blocked_task && (blocked_task->tick_num <= YKTickNum)){
    task = blocked_task;
    blocked_task = task->next;
    task->tick_num = 0;
    make_ready_task(task);
  }
}

void make_ready_task(tcb_t* tcb){
  tcb_t* current;
  tcb->next = 0;
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

void block_task(tcb_t* task){
  tcb_t* current;
  task->next = 0;
  if(blocked_task == 0){
    blocked_task = task;
    return;
  }
  if(task->tick_num < blocked_task->tick_num){
    task->next = blocked_task;
    blocked_task = task;
    return;
  }
  current =  blocked_task;
  while(current->next && (current->next->tick_num < task->tick_num)){
    current = current->next;
  }
  task->next = current->next;
  current->next = task;
  
  return;
}
