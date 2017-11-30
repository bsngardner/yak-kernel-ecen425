
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

YKQ q_memory[MAX_QUEUE_COUNT];
int q_dex = 0;

YKEVENT event_memory[MAX_EVENT_GROUP_COUNT];
int event_dex = 0;

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
  *(task_tcb->sp - 12) = (int) 0;//ds
  *(task_tcb->sp - 11) = (int) 0;//es
  *(task_tcb->sp - 10) = (int) 0;//bp
  *(task_tcb->sp - 9) = (int) 0;//di
  *(task_tcb->sp - 8) = (int) 0;//si
  *(task_tcb->sp - 7) = (int) 0;//dx
  *(task_tcb->sp - 6) = (int) 0;//cx
  *(task_tcb->sp - 5) = (int) 0;//bx
  *(task_tcb->sp - 4) = (int) 0;//ax
  *(task_tcb->sp - 3) = (int) task;//start point
  *(task_tcb->sp - 2) = 0;//cs register
  *(task_tcb->sp - 1) = INITIAL_FLAGS;//flags
  task_tcb->sp -= 12;

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

  if(!semaphore){
    printString("YKSemPend called on null semaphore");
    printNewLine();
    return;
  }
  
  if(YKCallDepth > 0){
    printString("YKSemPend called within interrupt");
    printNewLine();
    return;
  }
  
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

YKQ* YKQCreate(void **start, unsigned size){
  YKQ* q;
  if(q_dex >= MAX_QUEUE_COUNT){
    printString("Out of available queue memory");
    printNewLine();
    return 0;
  }

  q = q_memory + q_dex++;
  q->buffer = start;
  q->size = size;
  q->tail = 0;
  q->count = 0;
  q->pend_task = 0;
  return q;
}


void* YKQPend(YKQ *queue){
  void* msg;
  tcb_t* task;

  if(!queue || !queue->size){
    printString("YKQPend called on invalid queue");
    printNewLine();
    return 0;
  }

  YKEnterMutex();

  //Should this be a while or if
  while(queue->count == 0){  //queue is empty, pend task
    task = ready_task;
    ready_task = ready_task->next;
    add_task(&queue->pend_task,task);
    YKScheduler();
  }
  
  msg = *(queue->buffer + queue->tail);
  queue->tail++;
  if(queue->tail >= queue->size)
    queue->tail -= queue->size;
  queue->count--;

  YKExitMutex();
  return msg;
    
}


int YKQPost(YKQ *queue, void *msg){
  tcb_t* task;
  int head;

  if(!queue || !queue->size){
    printString("YKQPost called on invalid queue");
    printNewLine();
    return 0;
  }

  YKEnterMutex();

  if(queue->count == queue->size){
    goto err_return; //queue is full
  }

  head = queue->tail + queue->count;
  if(head >= queue->size)
    head -= queue->size;
  *(queue->buffer + head) = msg;
  queue->count++;

  if(queue->pend_task){
    task = queue->pend_task;
    queue->pend_task = task->next;
    add_task(&ready_task,task);

    if(YKCallDepth == 0){
      YKScheduler();
    }
  }

  YKExitMutex();
  return 1;

 err_return:
  YKExitMutex();
  return 0; //return with error
}

YKEVENT *YKEventCreate(unsigned initialValue){
  YKEVENT* event;
  if(event_dex >= MAX_EVENT_GROUP_COUNT){
    printString("Out of available event memory");
    printNewLine();
    return 0;
  }

  event = event_memory + event_dex;
  event->id = event_dex++;
  event->flags = initialValue;
  event->pend_task = 0;

  return event;
}

unsigned YKEventPend(YKEVENT *event, unsigned eventMask, int waitMode){
  tcb_t* task;
  unsigned wait_value;
  
  if(!event){
    printString("YKEventPend called on null event group");
    printNewLine();
    return event->flags;
  }

  if(YKCallDepth > 0){
    printString("YKEventPend called within interrupt");
    printNewLine();
    return event->flags;
  }

  YKEnterMutex();
  if(waitMode > EVENT_WAIT_ANY){
    printString("YKEventPend called with invalid wait mode");
    printNewLine();
    return event->flags;
  }

  if(((waitMode == EVENT_WAIT_ALL) && ((event->flags & eventMask) != eventMask)) || ((waitMode == EVENT_WAIT_ANY) && ((event->flags & eventMask) == 0))){
    task = ready_task;
    ready_task = task->next;
    task->next = event->pend_task;
    event->pend_task = task;
    event->status[task->id].event_mask = eventMask;
    event->status[task->id].wait_mode = waitMode;
    YKScheduler();
  }

  YKExitMutex();
  return event->flags;
}

void YKEventSet(YKEVENT *event, unsigned eventMask){
  tcb_t* task;
  tcb_t* prev;
  unsigned event_mask;
  unsigned wait_mode;

  YKEnterMutex();

  event->flags |= eventMask;
  task = event->pend_task;
  prev = 0;
  while(task){
    event_mask = event->status[task->id].event_mask;
    wait_mode = event->status[task->id].wait_mode;
    if(((wait_mode == EVENT_WAIT_ALL) && ((event->flags & event_mask) == event_mask)) || ((wait_mode == EVENT_WAIT_ANY) && ((event->flags & event_mask) != 0))){
      if(prev){
	prev->next = task->next;
	add_task(&ready_task,task);
	task = prev->next;
      }else{
	event->pend_task = task->next;
	add_task(&ready_task,task);
	task = event->pend_task;
      }

    }else{
      prev = task;
      task = task->next;
    }
  }

  YKExitMutex();
  return;
}

void YKEventReset(YKEVENT *event, unsigned eventMask){
  YKEnterMutex();
  event->flags &= ~eventMask;
  YKExitMutex();
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
