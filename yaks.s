;;; yaks.s

YKEnterMutex:
        cli
        ret

YKExitMutex:
        sti
        ret

YKDispatcher:
	push	bp
	mov	bp, sp
	push	bx
	cli			;Atomic

	mov	bx, word[bp+4]	;get arg - tcb pointer
	mov	ss, word[bx+4]	;restore ss
	mov	sp, word[bx+6]	;restore sp
	inc	word[YKCtxSwCount] ;Increment context switch counter
	mov	[active_task],bx
	
	sti			;Atomic
	pop	bx
	ret
