;;; yaks.s

YKEnterMutex:
        cli
        ret

YKExitMutex:
        sti
        ret

YKEnterISR:
	; >>>>> void YKEnterISR(void){ 
	; >>>>> YKCallDepth++; 
	inc	word [YKCallDepth]
	ret

YKExitISR:
	; >>>>> void YKExitISR(void){ 
	; >>>>> if((--YKCallDepth) == 0){ 
	mov	ax, word [YKCallDepth]
	dec	ax
	mov	word [YKCallDepth], ax
	test	ax, ax
	jne	exitisr_ret
	; >>>>> YKScheduler(); 
	call	YKScheduler
exitisr_ret:
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
	
	call	YKDispatcher
sched_ret:
	; >>>>> return;
	ret

YKDispatcher:
	mov	ax,[YKCallDepth]
	test	ax,ax
	jne	dispatch_l1
;;; Store context
	add	sp,2		;Valid for both cases
	pop	ax
	pushf
	push	cs
	push	ax

	mov	ax,0
	push	ax		;ax?
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	bp
	push	es
	push	ds
	jmp	dispatch_l2

dispatch_l1:
	add	sp,4
dispatch_l2:	
	mov	bx, word[active_task]
	mov	word[bx+4],ss
	mov	word[bx+6],sp

	mov	bx, word[ready_task]	;get arg - tcb pointer
	mov	ss, word[bx+4]		;restore ss
	mov	sp, word[bx+6]		;restore sp
	inc	word[YKCtxSwCount] 	;Increment context switch counter
	mov	[active_task],bx
	
;;; Restore context
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
