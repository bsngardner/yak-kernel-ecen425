#YAK kernel Makefile

run.bin: run.s
	nasm run.s -o run.bin -l run.lst

run.s: kern.s app.s
	cat kern.s app.s > run.s

app.s: lab4c_app.c
	cpp lab4c_app.c app.i
	c86 -g app.i app.s

kern.s: clib.s isr.s inth.s yakc.s yaks.s
	cat clib.s isr.s inth.s yakc.s yaks.s > kern.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Reset               ; Int 08h (IRQ 0),dd      reset_isr ; Reset               ; Int 08h (IRQ 0)," kern.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Tick                ; Int 09h (IRQ 1),dd      tick_isr  ; Tick                ; Int 09h (IRQ 1)," kern.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Keyboard            ; Int 0Ah (IRQ 2),dd      key_isr   ; Keyboard            ; Int 0Ah (IRQ 2q)," kern.s

yakc.s: yakc.c yaku.h yakk.h
	cpp yakc.c yakc.i
	c86 -g yakc.i yakc.s

inth.s: inth.c clib.h
	cpp inth.c inth.i
	c86 -g inth.i inth.s

clib.s: 
	curl -O "http://ece425web.groups.et.byu.net/stable/labs/library/clib.s"

clib.h: 
	curl -O "http://ece425web.groups.et.byu.net/stable/labs/library/clib.h"

clean:
	rm -f app-l4b.s kern.s app.s yakc.s inth.s *.i *.bin *.lst
