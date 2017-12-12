/* 
File: lab7inth.c
Revision date: 4 November 2009
Description: Sample interrupt handler code for EE 425 lab 6 (Message queues)
*/

#include "yakk.h"
#include "lab7defs.h"
#include "clib.h"

extern char KeyBuffer;

void myreset(void)
{
    exit(0);
}

void mytick(void)
{

}	       

void mykeybrd(void)
{
    char c;
}
