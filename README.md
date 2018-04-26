# yak-kernel-ecen425
The YAK kernel runs on a simulated 8080, compiled by a modified C86 compiler and assembled by NASM
Implementing this kernel is a class project for BYU EcEn 425 - Real Time Operating Systems.  

## Kernel files
yaku.h : User header file, used to set some kernel parameters
yakk.h : Header for kernel, contains struct definitions and the like
yakc.c : C code for kernel
yaks.s : Assembly code for kernel, including scheduler, dispatcher
isr.s  : Contains ISRs

## Libraries
clib.h : Header file for clib library functions

## Application code
lab\*app.c
lab\*defs.h

## Simptris
Simptris is a very simple implementation of a Tetris-like game in the console. There are several Simptris-related files.

## Other files
Other .s files are generated by the compiler

