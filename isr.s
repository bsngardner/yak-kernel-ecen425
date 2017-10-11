	;; isr.s
reset_isr:
	sti
	call	YKEnterISR

	call	reset_handler

	cli
	call	signalEOI
	call	YKExitISR
	iret

tick_isr:
	sti
	call	YKEnterISR

	call	tick_handler

	cli
	call	signalEOI
	call	YKExitISR
	iret

key_isr:
	sti
	call	YKEnterISR

	call	key_handler

	cli
	call	signalEOI
	call	YKExitISR
	iret
