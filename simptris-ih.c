/*
 * Simptris interrupt handlers
 *
 *
*/

#include "clib.h"
#include "yakk.h"
#include "simptris.h"
#include "simp.h"

extern unsigned NewPieceID;
extern unsigned NewPieceType;
extern unsigned NewPieceOrientation;
extern unsigned NewPieceColumn;

extern unsigned ScreenBitMap0;
extern unsigned ScreenBitMap1;
extern unsigned ScreenBitMap2;
extern unsigned ScreenBitMap3;
extern unsigned ScreenBitMap4;
extern unsigned ScreenBitMap5;


struct piece new_piece[8];
struct piece* piece;
unsigned int score;

//Game is over
void gameover_handler(){
  printString("Game over. Lines cleared: ");
  printInt(score);
  printNewLine();
}

//New piece has appeared 
void newpiece_handler(){
  static int dex = 0;
  piece = new_piece + dex;
  if(++dex >= 8)
    dex = 0;
  piece->id = NewPieceID;
  piece->type = NewPieceType;
  piece->x = NewPieceColumn;
  piece->rot = NewPieceOrientation;
  YKQPost(new_q,piece);
}

void command_handler(){
  YKSemPost(cmd_sem);
}

void touchdown_handler(){
  unsigned int b0;
  unsigned int b1;
  unsigned int bc0;
  unsigned int bc1;
  bc0 = 0;
  bc1 = 0;
//bin 0
  b0 = (ScreenBitMap0 | ScreenBitMap1 | ScreenBitMap2);
  while(b0 != 0){
    bc0++;
    b0 = b0 << 1;
  }
  bin[0].ht = bc0;
  b1 = (ScreenBitMap3 | ScreenBitMap4 | ScreenBitMap5);
  while(b1 != 0){
    bc1++;
    b1 = b1 << 1;
  }
  bin[1].ht = bc1;
}

void lineclear_handler(){
  score++;
}
