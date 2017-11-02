#ifndef YAKK_H
#define YAKK_H

typedef struct tcb{
  int id;           //[ptr+0]
  char state;       //[ptr+2]
  char priority;    //[ptr+3]
  int ss;           //[ptr+4]
  int* sp;          //[ptr+6]
  int tick_num;     //[ptr+8]
  struct tcb* next; //[ptr+10]
}tcb_t;

typedef struct {
  int id;
  int counter;
  tcb_t* pend_task;
} YKSEM;

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


extern int YKIdleCount;
extern int YKCallDepth;
extern int YKCtxSwCount;
extern int YKTickNum;

#endif /* YAKK_H */
