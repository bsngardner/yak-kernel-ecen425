//Broderick Gardner
//EcEn 427
//Lab 3 - Interrupts

//Interrupt handlers


#include "clib.h"
#include "yakk.h"
#include "yaku.h"

extern int KeyBuffer;

void reset_handler(){
  exit(0);

}

void tick_handler(){
  static unsigned int tick_count = 0;
  //  printNewLine();
  printString("TICK ");
  ++tick_count;
  printUInt(tick_count);
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
  }else{
    printString("KEYPRESS (");
    printChar((char) KeyBuffer);
    printString(") IGNORED");
  }
}
