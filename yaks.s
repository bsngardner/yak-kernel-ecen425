;;; yaks.s

YKEnterMutex:
        cli
        ret

YKExitMutex:
        sti
        ret

YKIdleTask:
	inc	word [YKIdleCount]
	nop
	nop
	jmp	YKIdleTask

YKScheduler:
	; >>>>> if(!kernel_running || active_task == ready_task){ 
	mov	ax, word [kernel_running]
	test	ax, ax
	je	sched_ret
	mov	ax, word [ready_task]
	cmp	ax, word [active_task]
	je	sched_ret
	; >>>>> YKDispatcher(ready_task); 
	pushf
	push	cs
	call	YKDispatcher
sched_ret:
	; >>>>> return; 
	ret

YKDispatcher:
	push	bp
	mov	bp, sp
	push	bx
	
	mov	bx, word[active_task]
	mov	word[bx+4],ss
	mov	word[bx+6],sp

	mov	bx, word[ready_task]	;get arg - tcb pointer
	mov	ss, word[bx+4]		;restore ss
	mov	sp, word[bx+6]		;restore sp
	inc	word[YKCtxSwCount] 	;Increment context switch counter
	mov	[active_task],bx
	
	pop	bx
	pop	bp
	iret
