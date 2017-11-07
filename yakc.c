
#include "clib.h"
#include "yaku.h"
#include "yakk.h"

//Defines
#define LOWEST_PRIORITY 100
#define INITIAL_FLAGS 0x0200
#define INTSIZE 2

//Local prototypes
static void add_task(tcb_t** root, tcb_t* task);
static tcb_t* pop_task(tcb_t** root);
static void block_task(tcb_t* task);

void YKIdleTask(void);
void YKScheduler(void);
void YKDispatcher(void);
void YKIdleTask(void);

tcb_t init_tcb;
tcb_t tcb_memory[MAX_TCB_COUNT];
int tcb_dex = 0;

tcb_t* active_task = &init_tcb;
tcb_t* ready_task = 0;
tcb_t* blocked_task = 0;

int idle_stack[STACK_SIZE];

YKSEM semaphore_memory[MAX_SEMAPHORE_COUNT];
int sem_dex = 0;

int kernel_running = 0;

int YKIdleCount = 0;
int YKCallDepth = 0;
int YKCtxSwCount = 0;
int YKTickNum = 0;

void YKInitialize(void){

  YKNewTask(YKIdleTask, &idle_stack[STACK_SIZE], LOWEST_PRIORITY);

     //create idle task tcb and put it in the ready queue
     //??
     //profit
}

void YKNewTask(void (* task)(void), void *taskStack, unsigned char priority){
  tcb_t* task_tcb;
  YKEnterMutex();
  if(tcb_dex >= MAX_TCB_COUNT){
    printString("Out of available TCB memory");
    printNewLine();
    return;
  }

  task_tcb = tcb_memory + tcb_dex;
  task_tcb->id = tcb_dex++;
  //  task_tcb->state = 1;   //Removed -- unneeded for linked list implementation
  task_tcb->priority = priority;
  task_tcb->next = 0;
  task_tcb->sp = (int*)((char*)taskStack);
  task_tcb->ss = 0;
  task_tcb->tick_num = 0;
  //  *(tcb->sp - 1) = INITIAL_FLAGS;
  //  *(tcb->sp - 2) = 0;
  *(task_tcb->sp - 5) = (int) 0;//bx
  *(task_tcb->sp - 4) = (int) 0;//bp
  *(task_tcb->sp - 3) = (int) task;//start point
  *(task_tcb->sp - 2) = 0;//cs register
  *(task_tcb->sp - 1) = INITIAL_FLAGS;//flags
  task_tcb->sp -= 3 * INTSIZE - 1;

  add_task(&ready_task,task_tcb);

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
  task = pop_task(&ready_task);
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
    task = pop_task(&blocked_task);
    task->tick_num = 0;
    add_task(&ready_task,task);
  }
}

YKSEM* YKSemCreate(int initial_value){
  YKSEM* sem;
  if(sem_dex >= MAX_SEMAPHORE_COUNT){
    printString("Out of available semaphore memory");
    printNewLine();
    return 0;
  }
  if(initial_value < 0){
    printString("Semaphore starting value less than 0");
    printNewLine();
    return 0;
  }

  sem = semaphore_memory + sem_dex;
  sem->id = sem_dex++;
  sem->counter = initial_value;
  sem->pend_task = 0;

  return sem;
}

void YKSemPend(YKSEM* semaphore){
  tcb_t* task;
  /*
  if(!semaphore){
    printString("YKSemPend called on null semaphore");
    printNewLine();
    return;
  }
  
  if(YKCallDepth > 0){
    printString("YKSemPost called within interrupt");
    printNewLine();
    return;
  }
  */
  
  YKEnterMutex();
  
  if(semaphore->counter-- <= 0){
    task = ready_task;
    ready_task = task->next;
    //task = pop_task(&ready_task);
    add_task(&semaphore->pend_task,task);
    YKScheduler();
  }
  
  YKExitMutex();
  return;
}

void YKSemPost(YKSEM* semaphore){
  tcb_t* task;

  //if(!semaphore){
  //  printString("YKSemPost called on null semaphore");
  //  printNewLine();
  //  return;
  //}

  

  YKEnterMutex();

  semaphore->counter++;
  if(semaphore->pend_task != 0){
    task = semaphore->pend_task;
    semaphore->pend_task = task->next;
    //task = pop_task(&semaphore->pend_task);
    add_task(&ready_task,task);

    if(YKCallDepth == 0){
      YKScheduler();
    }
  }

  YKExitMutex();
  return;
}
 
static tcb_t* pop_task(tcb_t** root){
  tcb_t* task = *root;
  *root = task->next;
  return task;
}

static void add_task(tcb_t** root, tcb_t* task){
  tcb_t* current;
  task->next = 0;
  if(*root == 0){
    *root = task;
    return;
  }

  if(task->priority < (*root)->priority){
    task->next = *root;
    *root = task;
    return;
  }

  current = *root;
  while(current->next && (current->next->priority < task->priority)){
    current = current->next;
  }
  task->next = current->next;
  current->next = task;
}

static void block_task(tcb_t* task){
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
