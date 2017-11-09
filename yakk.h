#ifndef YAKK_H
#define YAKK_H

typedef struct tcb{
  char id;           //[ptr+0]
  char priority;    //[ptr+1]
  int tick_num;     //[ptr+2]
  int ss;           //[ptr+4]
  int* sp;          //[ptr+6]
  struct tcb* next; //[ptr+8]
}tcb_t;

typedef struct {
  int id;
  int counter;
  tcb_t* pend_task;
} YKSEM;

typedef struct {
  void** buffer;
  int size;
  int tail;
  int count;
  tcb_t* pend_task;
}YKQ; //5 words / 10 bytes

void YKInitialize(void);
void YKNewTask(void (* task)(void), void *taskStack, unsigned char priority);
void YKRun(void);
void YKDelayTask(unsigned count);
void YKEnterISR(void);
void YKExitISR(void);
void YKExitMutex(void);
void YKEnterMutex(void);
void YKTickHandler(void);
YKSEM* YKSemCreate(int initial_value);
void YKSemPend(YKSEM* semaphore);
void YKSemPost(YKSEM* semaphore);
YKQ* YKQCreate(void **start, unsigned size);
void *YKQPend(YKQ *queue);
int YKQPost(YKQ *queue, void *msg);

extern int YKIdleCount;
extern int YKCallDepth;
extern int YKCtxSwCount;
extern int YKTickNum;

#endif /* YAKK_H */
