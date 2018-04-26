

gameover_isr:
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

	call 	gameover_handler
	
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

newpiece_isr:
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

	call 	newpiece_handler
	
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

command_isr:
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

	call 	command_handler
	
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

touchdown_isr:
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

	call 	touchdown_handler
	
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

lineclear_isr:
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

	call 	lineclear_handler
	
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

