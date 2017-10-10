#YAK kernel Makefile

app-l4b.bin: app-l4b.s
	nasm app-l4b.s -o app-l4b.bin -l app-l4b.lst

app-l4b.s: kern.s l4b.s
	cat kern.s l4b.s > app-l4b.s

l4b.s: lab4b.c
	cpp l4b.c l4b.i
	c86 -g l4b.i l4b.s

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
	rm -f app-l4b.s kern.s l4b.s yakc.s inth.s clib.s *.i *.bin *.lst
