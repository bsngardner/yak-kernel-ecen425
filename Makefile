#YAK kernel Makefile

app.bin: app.s
	nasm lab3-asm.s -o lab3.bin -l lab3.lst

lab3-asm.s: clib.s lab3.s lab3asm.s primes.s
	cat clib.s primes.s lab3.s lab3asm.s > lab3-asm.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Reset               ; Int 08h (IRQ 0),dd      reset_isr ; Reset               ; Int 08h (IRQ 0)," lab3-asm.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Tick                ; Int 09h (IRQ 1),dd      tick_isr  ; Tick                ; Int 09h (IRQ 1)," lab3-asm.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Keyboard            ; Int 0Ah (IRQ 2),dd      key_isr   ; Keyboard            ; Int 0Ah (IRQ 2q)," lab3-asm.s

lab3.s: lab3.c clib.h
	cpp lab3.c lab3.i
	c86 -g lab3.i lab3.s



clib.s: 
	curl -O "http://ece425web.groups.et.byu.net/stable/labs/library/clib.s"

clib.h: 
	curl -O "http://ece425web.groups.et.byu.net/stable/labs/library/clib.h"


clean:
	rm -f lab3-asm.s lab3.s primes.s clib.s *.i *.bin *.lst
