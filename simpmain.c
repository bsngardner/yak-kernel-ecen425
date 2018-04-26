/*
 * Main c code for simptris game
 *
 *
 * Broderick Gardner and Thomas Tryon
 */

#include "clib.h"
#include "yakk.h"
#include "simptris.h"
#include "simp.h"

#define TASK_STACK_SIZE 256
#define QSIZE 64
#define FLAT 1

int stats_task_stack[TASK_STACK_SIZE];
int cmd_task_stack[TASK_STACK_SIZE];
int placement_task_stack[TASK_STACK_SIZE];

YKSEM* cmd_sem;

void* cmd_buffer[QSIZE];
YKQ* cmd_q;

void* new_buffer[QSIZE];
YKQ* new_q;

void command_task(void);
void comm_task(void);
void placement_task(void);
void stats_task(void);

#define pack_cmd(id,mode,dir) ((id << 2) | (mode << 1) | dir)
#define unpack_id(n) (n >> 2)
#define unpack_mode(n) ((n >> 1) & 1)
#define unpack_dir(n) (n & 1)

#define SEED 73706

void main(void){
  printString("Initializing kernel");
  printNewLine();
  YKInitialize();

  printString("Creating queues and semaphores");
  printNewLine();

  cmd_sem = YKSemCreate(1);
  cmd_q = YKQCreate(cmd_buffer, QSIZE);
  new_q = YKQCreate(new_buffer,QSIZE);
  YKNewTask(stats_task, (void*) &stats_task_stack[TASK_STACK_SIZE],0);

  printString("Finished initializing kernel");
  printNewLine();
  SeedSimptris(SEED);
  YKRun();
}

void cmd_task(void){
  int cmd;
  int dir;
  int mode;
  int id;
  
  printString("cmd task begun");
  printNewLine();

  while(1){
    cmd = (int) YKQPend(cmd_q);
    id = unpack_id(cmd);
    dir = unpack_dir(cmd);
    mode = unpack_mode(cmd);

    YKSemPend(cmd_sem);
    if(mode){
      SlidePiece(id,dir);
    }else{
      RotatePiece(id,dir);
    }
  }
}

struct bin bin[2];
#define MAX_PIECES 16
#define BIN0_FLAT 0x01
#define BIN1_FLAT 0x02
struct piece pieces[MAX_PIECES];

void placement_task(void){
  struct piece* new;
  int dir;
  int cmd;

  bin[0].ht = 0;
  bin[1].ht = 1;
  bin[0].flat = BIN0_FLAT;
  bin[1].flat = BIN1_FLAT;
  bin[0].x = 0;
  bin[1].x = 5;

  printString("placement task begun");
  printNewLine();

 while(1){
   new = (struct piece*)YKQPend(new_q);
   if(new->x == 0){
     cmd = pack_cmd(new->id,1,1);
     YKQPost(cmd_q,(void*)cmd);
     new->x = 1;
   }else if(new->x == 5){
     cmd = pack_cmd(new->id,1,0);
     YKQPost(cmd_q,(void*)cmd);
     new->x = 4;
   }
   if(new->type == FLAT){
     switch(bin[0].flat + bin[1].flat){
     case 1:
       new->trans = bin[0].x + 1 - new->x;
       break;
     case 2:
       new->trans = bin[1].x - 1 - new->x;
       break;
     case 3:
       if(bin[0].ht == bin[1].ht){
	 if(new->x <= 3){
	   new->trans = bin[0].x + 1 - new->x;	 
	 }else{
	   new->trans = bin[1].x - 1 - new->x;	 
	 }
       }else if(bin[0].ht<bin[1].ht){
	 new->trans = bin[0].x + 1 - new->x;	 
       }else{
	 new->trans = bin[1].x - 1 - new->x;	 
       }
       break;
     }
   }else{ //Corner

     switch(bin[0].flat + bin[1].flat){
     case 1:
       //bin 1
       new->trans = bin[1].x -2 - new->x;
       new->rot = 3-new->rot;
       bin[1].flat = BIN1_FLAT;
       break;
     case 2:
       //bin 0
       new->trans = bin[0].x + 2 - new->x;
       new->rot = 2-new->rot;
       bin[0].flat = BIN0_FLAT;
       break;
     case 3:
       if(bin[0].ht == bin[1].ht){
	 if(new->x <= 3){
	   new->trans = bin[0].x-new->x;
	   new->rot = 0 - new->rot;
	   bin[0].flat = 0;
	 }else{
	   new->trans = bin[1].x - new->x;
	   new->rot = 1 - new->rot;
	   bin[1].flat = 0;
	 }
       }else if(bin[0].ht<bin[1].ht){
	 //bin 0
	 new->trans = bin[0].x-new->x;
	 new->rot = 0 - new->rot;
	 bin[0].flat = 0;
       }else{
	 //bin 1
	 new->trans = bin[1].x - new->x;
	 new->rot = 1 - new->rot;
	 bin[1].flat = 0;
       }
       break;
     }
   }

   dir = new->rot < 0;
   cmd = pack_cmd(new->id,0,dir);
   while(new->rot){
     YKQPost(cmd_q,(void*)cmd);
     if(dir)
       new->rot++;
     else
       new->rot--;
   }

   dir = new->trans > 0;
   cmd = pack_cmd(new->id,1,dir);
   while(new->trans){
     YKQPost(cmd_q,(void*)cmd);
     if(dir)
       new->trans--;
     else
       new->trans++;
   }
 }
}

void stats_task(void){
  unsigned max, switchCount, idleCount;
  int tmp;

  YKDelayTask(1);
  printString("Welcome to the YAK kernel\r\n");
  printString("Determining CPU capacity\r\n");
  YKDelayTask(1);
  YKIdleCount = 0;
  YKDelayTask(5);
  max = YKIdleCount / 25;
  YKIdleCount = 0;
  
  YKNewTask(placement_task, (void *) &placement_task_stack[TASK_STACK_SIZE], 2);
  YKNewTask(cmd_task, (void *) &cmd_task_stack[TASK_STACK_SIZE], 1);
  StartSimptris();
    
  while (1){
    YKDelayTask(20);
    
    YKEnterMutex();
    switchCount = YKCtxSwCount;
    idleCount = YKIdleCount;
    YKExitMutex();
    
    printString("<CS: ");
    printInt((int)switchCount);
    printString(", CPU: ");
    tmp = (int) (idleCount/max);
    printInt(100-tmp);
    printString("%>\r\n");
    
    YKEnterMutex();
    YKCtxSwCount = 0;
    YKIdleCount = 0;
    YKExitMutex();
  }
}
