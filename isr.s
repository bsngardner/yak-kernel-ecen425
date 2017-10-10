	;; isr.s
	
reset_isr:
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	bp
	push	es
	push	ds
	sti
	call	YKEnterISR

	call	reset_handler

	cli
	call	signalEOI
	call	YKExitISR
	pop	ds
	pop	es
	pop	bp
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	iret

tick_isr:
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	bp
	push	es
	push	ds
	sti
	call	YKEnterISR

	call	tick_handler

	cli
	call	signalEOI
	call	YKExitISR
	pop	ds
	pop	es
	pop	bp
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	iret

key_isr:
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	bp
	push	es
	push	ds
	sti
	call	YKEnterISR
	
	call	key_handler

	cli
	call	signalEOI
	call	YKExitISR
	pop	ds
	pop	es
	pop	bp
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	iret
