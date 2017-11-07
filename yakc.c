
#include "clib.h"
#include "yaku.h"
#include "yakk.h"

//Defines
#define LOWEST_PRIORITY 100
#define INITIAL_FLAGS 0x0200
#define INTSIZE 2

#define READY 0
#define DELAYED 1
#define PENDING 2

//Local prototypes
static void insert_task(tcb_t* task);
static void find_ready_task(void);

void YKIdleTask(void);
void YKScheduler(void);
void YKDispatcher(void);
void YKIdleTask(void);

tcb_t init_tcb;
tcb_t tcb_memory[MAX_TCB_COUNT];
int tcb_dex = 0;

tcb_t* task_list[MAX_TCB_COUNT];

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
  ready_task = task_list[0];
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
  task_tcb->priority = priority;
  task_tcb->state = READY;
  task_tcb->info = 0;
  task_tcb->sp = (int*)((char*)taskStack);
  task_tcb->ss = 0;
  //  *(tcb->sp - 1) = INITIAL_FLAGS;
  //  *(tcb->sp - 2) = 0;
  *(task_tcb->sp - 5) = (int) 0;//bx
  *(task_tcb->sp - 4) = (int) 0;//bp
  *(task_tcb->sp - 3) = (int) task;//start point
  *(task_tcb->sp - 2) = 0;//cs register
  *(task_tcb->sp - 1) = INITIAL_FLAGS;//flags
  task_tcb->sp -= 3 * INTSIZE - 1;
  
  insert_task(task_tcb);
  find_ready_task();

  //  add_task(&ready_task,task_tcb);

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
  task->state = DELAYED;
  task->info = YKTickNum + count;
  find_ready_task();
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
  tcb_t *task, *end;
  int delta;
  YKTickNum++;
  delta = 0;
  task = &tcb_memory[0];
  end = &tcb_memory[tcb_dex];
  for(;task < end;task++){
    if(task->state == DELAYED && YKTickNum >= task->info){
      task->state = READY;
      task->info = 0;
      delta = 1;
    }
  }
  if(delta)
    find_ready_task();
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
    task = active_task;
    task->state = PENDING;
    task->info = semaphore->id;
    find_ready_task();
    YKScheduler();
  }
  
  YKExitMutex();
  return;
}

void YKSemPost(YKSEM* semaphore){
  tcb_t **task_p, **end;

  //if(!semaphore){
  //  printString("YKSemPost called on null semaphore");
  //  printNewLine();
  //  return;
  //}

  YKEnterMutex();
  
  if(semaphore->counter++ <= 0){
    task_p = &task_list[0];
    end = &task_list[tcb_dex];
    for(;task_p < end; task_p++){
      if((*task_p)->state == PENDING && (*task_p)->info == semaphore->id){
	(*task_p)->state = READY;
	(*task_p)->info = 0;
	find_ready_task();
	break;
      }
    } 
    if(YKCallDepth == 0){
      YKScheduler();
    }
  }

  YKExitMutex();
  return;
}

static void find_ready_task(){
  int i;
  i = 0;

  while(task_list[i]->state != READY){
    i++;
  }
  ready_task = task_list[i];
}

static void insert_task(tcb_t* task){
  int i;
  i = tcb_dex - 1;
  
  while(i > 0 &&  task->priority < task_list[i-1]->priority){
    task_list[i] = task_list[i-1];
    i--;
  }
  task_list[i] = task;
}
