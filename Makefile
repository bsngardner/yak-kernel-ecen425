#YAK kernel Makefile

run.bin: run.s
	nasm run.s -o run.bin -l run.lst

run.s: kern.s app.s
	cat kern.s app.s > run.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Simptris Game Over  ; Int 0Bh (IRQ 3),dd      gameover_isr ; Simptris Game Over  ; Int 0Bh (IRQ 3)," run.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Simptris New Piece  ; Int 0Ch (IRQ 4),dd      newpiece_isr ; Simptris New Piece  ; Int 0Ch (IRQ 4)," run.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Simptris Received   ; Int 0Dh (IRQ 5),dd      command_isr ; Simptris Received   ; Int 0Dh (IRQ 5)," run.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Simptris Touchdown  ; Int 0Eh (IRQ 6),dd      touchdown_isr ; Simptris Touchdown  ; Int 0Eh (IRQ 6)," run.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Simptris Clear      ; Int 0Fh (IRQ 7),dd      lineclear_isr ; Simptris Clear      ; Int 0Fh (IRQ 7)," run.s

app.s: simptris-isr.s simptris-ih.c simpmain.c simptris.h simptris.s
	cpp simpmain.c main.i
	c86 -g main.i main.s
	cpp simptris-ih.c sih.i
	c86 -g sih.i sih.s
	cat simptris.s simptris-isr.s sih.s main.s > app.s


kern.s: clib.s isr.s inth.s yakc.s yaks.s
	cat clib.s isr.s inth.s yakc.s yaks.s > kern.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Reset               ; Int 08h (IRQ 0),dd      reset_isr ; Reset               ; Int 08h (IRQ 0)," kern.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Tick                ; Int 09h (IRQ 1),dd      tick_isr  ; Tick                ; Int 09h (IRQ 1)," kern.s
	sed -i "$(_LINE_NUM)s,dd      0 ; Keyboard            ; Int 0Ah (IRQ 2),dd      key_isr   ; Keyboard            ; Int 0Ah (IRQ 2q)," kern.s

yakc.s: yakc.c yaku.h yakk.h
	cpp yakc.c yakc.i
	c86 -g yakc.i yakc.s

inth.s: inth.c clib.h lab6defs.h
	cpp inth.c inth.i
	c86 -g inth.i inth.s

clib.s: 
	curl -O "http://ece425web.groups.et.byu.net/stable/labs/library/clib.s"

clib.h: 
	curl -O "http://ece425web.groups.et.byu.net/stable/labs/library/clib.h"

clean:
	rm -f app-l4b.s kern.s app.s yakc.s inth.s *.i *.bin *.lst
