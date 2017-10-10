#ifndef YAKK_H
#define YAKK_H

typedef struct tcb{
  int id;           //[ptr+0]
  char state;       //[ptr+2]
  char priority;    //[ptr+3]
  int ss;           //[ptr+4]
  int* sp;          //[ptr+6]
  struct tcb* next; //[ptr+8]
}tcb_t;

void YKInitialize(void);
void YKNewTask(void (* task)(void), void *taskStack, unsigned char priority);
void YKRun(void);
void YKDelayTask(unsigned count);
void YKEnterISR(void);
void YKExitISR(void);
void YKExitMutex(void);
void YKEnterMutex(void);

extern int YKIdleCount;
extern int YKCallDepth;
extern int YKCtxSwCount;

#endif /* YAKK_H */
