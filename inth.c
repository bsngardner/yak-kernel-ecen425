//Broderick Gardner
//EcEn 427
//Lab 3 - Interrupts

//Interrupt handlers


#include "clib.h"
#include "yakk.h"
#include "yaku.h"

extern int KeyBuffer;
extern YKSEM* NSemPtr;

void reset_handler(){
  exit(0);

}

void tick_handler(){
  printNewLine();
  printString("TICK ");
  printUInt(YKTickNum);
  printNewLine();
}

void key_handler(){
  unsigned int wait_count = 5000;
  printNewLine();
  if((char) KeyBuffer == 'd'){
    printString("\nDELAY KEY PRESSED");
    while(wait_count-->0);
    printNewLine();
    printString("DELAY COMPLETE");
  }else if((char) KeyBuffer == 'p'){
    YKSemPost(NSemPtr);
  }else{
    printString("KEYPRESS (");
    printChar((char) KeyBuffer);
    printString(") IGNORED");
  }
}
