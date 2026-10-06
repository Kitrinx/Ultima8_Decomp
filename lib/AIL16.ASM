; name: AIL16
; flags: /ml /m5 /zd
; Miles AIL 16-bit timer services, linked into U8.EXE as a finished object.
; Disassembled from the shipped code. U8 supplies the PIT hooks it calls.

	EXTRN	_hook_timer_process:FAR
	EXTRN	_set_PIT_period:FAR
	EXTRN	_unhook_timer_process:FAR

DGROUP	GROUP	_DATA

_DATA	SEGMENT	WORD PUBLIC 'DATA'

active_timers	dw	0
timer_busy	dw	0		; set while _API_timer runs
timer_callback	dd	16 dup (0)
timer_ds	dw	16 dup (0)	; caller's DS for each callback
timer_status	dw	16 dup (0)	; 0 free, 1 stopped, 2 running
timer_elapsed	dd	16 dup (0)
timer_value	dd	16 dup (0)	; period in microseconds
current_period	dd	0
current_timer	dw	0
min_period	dd	0
	dw	0D3h

_DATA	ENDS

AIL16_TEXT	SEGMENT	WORD PUBLIC 'CODE'
	ASSUME	CS:AIL16_TEXT, DS:DGROUP

	PUBLIC	_API_timer
_API_timer	PROC	FAR
	cld
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	bp
	push	es
	push	ds
	mov	ax, SEG timer_busy
	mov	ds, ax
	cmp	word ptr [timer_busy], 0
	je	L_0018
	jmp	short L_0080
L_0018:
	mov	word ptr [timer_busy], 1
	mov	word ptr [current_timer], 0
L_0024:
	mov	si, word ptr [current_timer]
	shl	si, 1
	cmp	word ptr [si + timer_status], 2
	jne	L_006F
	shl	si, 1
	mov	ax, word ptr [si + timer_elapsed]
	mov	dx, word ptr [si + timer_elapsed + 2]
	add	ax, word ptr [current_period]
	adc	dx, word ptr [current_period + 2]
	cmp	dx, word ptr [si + timer_value + 2]
	jb	L_0051
	ja	L_005B
	cmp	ax, word ptr [si + timer_value]
	jae	L_005B
L_0051:
	mov	word ptr [si + timer_elapsed], ax
	mov	word ptr [si + timer_elapsed + 2], dx
	jmp	short L_006F
L_005B:
	sub	ax, word ptr [si + timer_value]
	sbb	dx, word ptr [si + timer_value + 2]
	mov	word ptr [si + timer_elapsed], ax
	mov	word ptr [si + timer_elapsed + 2], dx
	call	dword ptr [si + timer_callback]
L_006F:
	inc	word ptr [current_timer]
	cmp	word ptr [current_timer], 0Fh
	jbe	L_0024
	mov	word ptr [timer_busy], 0
L_0080:
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
_API_timer	ENDP

init_timers	PROC	FAR
	push	ds
	push	si
	push	di
	pushf
	cli
	cld
	mov	word ptr [current_period], 0FFFFh
	mov	word ptr [current_period + 2], 0FFFFh
	push	ds
	pop	es
	mov	di, OFFSET DGROUP:timer_status
	mov	cx, 10h
	mov	ax, 0
	rep stosw
	mov	di, OFFSET DGROUP:timer_elapsed
	mov	cx, 20h
	rep stosw
	mov	di, OFFSET DGROUP:timer_value
	mov	cx, 20h
	rep stosw
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_00C4
	sti
L_00C4:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	retf
init_timers	ENDP

udiv32	PROC	FAR
	push	bp
	mov	bp, sp
	push	ds
	push	si
	push	di
	mov	ax, word ptr [bp + 6]
	mov	dx, word ptr [bp + 8]
	mov	bx, word ptr [bp + 0Ah]
	mov	cx, word ptr [bp + 0Ch]
	or	cx, cx
	jne	L_00E8
	or	dx, dx
	je	L_010C
	or	bx, bx
	je	L_010C
L_00E8:
	mov	bp, cx
	mov	cx, 20h
	xor	di, di
	xor	si, si
L_00F1:
	shl	ax, 1
	rcl	dx, 1
	rcl	si, 1
	rcl	di, 1
	cmp	di, bp
	jb	L_0108
	ja	L_0103
	cmp	si, bx
	jb	L_0108
L_0103:
	sub	si, bx
	sbb	di, bp
	inc	ax
L_0108:
	loop	L_00F1
	jmp	short L_0110
L_010C:
	div	bx
	xor	dx, dx
L_0110:
	pop	di
	pop	si
	pop	ds
	pop	bp
	retf
udiv32	ENDP

program_timer	PROC	FAR
	push	ds
	push	si
	push	di
	pushf
	cli
	cld
	mov	word ptr [min_period], 0FFFFh
	mov	word ptr [min_period + 2], 0FFFFh
	mov	si, 0
L_012A:
	mov	bx, si
	shl	bx, 1
	cmp	word ptr [bx + timer_status], 0
	je	L_0154
	shl	bx, 1
	mov	ax, word ptr [bx + timer_value]
	mov	dx, word ptr [bx + timer_value + 2]
	cmp	dx, word ptr [min_period + 2]
	jb	L_014D
	ja	L_0154
	cmp	ax, word ptr [min_period]
	jae	L_0154
L_014D:
	mov	word ptr [min_period], ax
	mov	word ptr [min_period + 2], dx
L_0154:
	inc	si
	cmp	si, 0Fh
	jbe	L_012A
	mov	ax, word ptr [min_period]
	mov	dx, word ptr [min_period + 2]
	cmp	ax, word ptr [current_period]
	jne	L_016D
	cmp	dx, word ptr [current_period + 2]
	je	L_0190
L_016D:
	mov	word ptr [current_timer], 0FFFFh
	mov	word ptr [current_period], ax
	mov	word ptr [current_period + 2], dx
	push	ax
	call	far ptr _set_PIT_period
	add	sp, 2
	push	ds
	pop	es
	mov	di, OFFSET DGROUP:timer_elapsed
	mov	cx, 20h
	mov	ax, 0
	rep stosw
L_0190:
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_019B
	sti
L_019B:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	retf
program_timer	ENDP

	PUBLIC	_AIL_startup
_AIL_startup	PROC	FAR
	push	ds
	push	si
	push	di
	pushf
	cli
	mov	word ptr [active_timers], 0
	mov	word ptr [timer_busy], 0
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_01BD
	sti
L_01BD:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	retf
_AIL_startup	ENDP

	PUBLIC	_AIL_shutdown
_AIL_shutdown	PROC	FAR
	push	bp
	mov	bp, sp
	push	ds
	push	si
	push	di
	pushf
	cli
	push	cs
	call	near ptr _AIL_release_all_timers
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_01DA
	sti
L_01DA:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	pop	bp
	retf
_AIL_shutdown	ENDP

	PUBLIC	_AIL_register_timer
_AIL_register_timer	PROC	FAR
	push	bp
	mov	bp, sp
	push	ds
	push	si
	push	di
	pushf
	cli
	mov	cx, ds
	mov	bx, 0
L_01EE:
	cmp	word ptr [bx + timer_status], 0
	je	L_0202
	add	bx, 2
	cmp	bx, 20h
	jb	L_01EE
	mov	ax, 0FFFFh
	jmp	short L_0257
L_0202:
	mov	ax, bx
	shr	ax, 1
	mov	word ptr [bx + timer_status], 1
	mov	word ptr [bx + timer_ds], cx
	shl	bx, 1
	les	si, dword ptr [bp + 6]
	mov	word ptr [bx + timer_callback], si
	mov	word ptr [bx + timer_callback + 2], es
	mov	word ptr [bx + timer_value], 0FFFFh
	mov	word ptr [bx + timer_value + 2], 0FFFFh
	inc	word ptr [active_timers]
	cmp	word ptr [active_timers], 1
	jne	L_0257
	push	ax
	push	cs
	call	near ptr init_timers
	call	far ptr _hook_timer_process
	pop	ax
	mov	bx, ax
	shl	bx, 1
	mov	word ptr [bx + timer_status], 1
	shl	bx, 1
	mov	word ptr [bx + timer_value], 0FFFFh
	mov	word ptr [bx + timer_value + 2], 0FFFFh
L_0257:
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_0262
	sti
L_0262:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	pop	bp
	retf
_AIL_register_timer	ENDP

	PUBLIC	_AIL_release_timer_handle
_AIL_release_timer_handle	PROC	FAR
	push	bp
	mov	bp, sp
	push	ds
	push	si
	push	di
	pushf
	cli
	mov	bx, word ptr [bp + 6]
	cmp	bx, -1
	je	L_0293
	shl	bx, 1
	cmp	word ptr [bx + timer_status], 0
	je	L_0293
	mov	word ptr [bx + timer_status], 0
	dec	word ptr [active_timers]
	jne	L_0293
	call	far ptr _unhook_timer_process
L_0293:
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_029E
	sti
L_029E:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	pop	bp
	retf
_AIL_release_timer_handle	ENDP

	PUBLIC	_AIL_release_all_timers
_AIL_release_all_timers	PROC	FAR
	push	ds
	push	si
	push	di
	pushf
	cli
	mov	si, 0Fh
L_02AD:
	push	si
	push	cs
	call	near ptr _AIL_release_timer_handle
	add	sp, 2
	dec	si
	jge	L_02AD
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_02C3
	sti
L_02C3:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	retf
_AIL_release_all_timers	ENDP

	PUBLIC	_AIL_start_timer
_AIL_start_timer	PROC	FAR
	push	bp
	mov	bp, sp
	push	ds
	push	si
	push	di
	pushf
	cli
	mov	bx, word ptr [bp + 6]
	shl	bx, 1
	cmp	word ptr [bx + timer_status], 1
	jne	L_02E3
	mov	word ptr [bx + timer_status], 2
L_02E3:
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_02EE
	sti
L_02EE:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	pop	bp
	retf
_AIL_start_timer	ENDP

	PUBLIC	_AIL_start_all_timers
_AIL_start_all_timers	PROC	FAR
	push	ds
	push	si
	push	di
	pushf
	cli
	mov	si, 0Fh
L_02FD:
	push	si
	push	cs
	call	near ptr _AIL_start_timer
	add	sp, 2
	dec	si
	jge	L_02FD
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_0313
	sti
L_0313:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	retf
_AIL_start_all_timers	ENDP

	PUBLIC	_AIL_stop_timer
_AIL_stop_timer	PROC	FAR
	push	bp
	mov	bp, sp
	push	ds
	push	si
	push	di
	pushf
	cli
	mov	bx, word ptr [bp + 6]
	shl	bx, 1
	cmp	word ptr [bx + timer_status], 2
	jne	L_0333
	mov	word ptr [bx + timer_status], 1
L_0333:
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_033E
	sti
L_033E:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	pop	bp
	retf
_AIL_stop_timer	ENDP

	PUBLIC	_AIL_stop_all_timers
_AIL_stop_all_timers	PROC	FAR
	push	ds
	push	si
	push	di
	pushf
	cli
	mov	si, 0Fh
L_034D:
	push	si
	push	cs
	call	near ptr _AIL_stop_timer
	add	sp, 2
	dec	si
	jge	L_034D
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_0363
	sti
L_0363:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	retf
_AIL_stop_all_timers	ENDP

	PUBLIC	_AIL_set_timer_period
_AIL_set_timer_period	PROC	FAR
	push	bp
	mov	bp, sp
	push	ds
	push	si
	push	di
	pushf
	cli
	mov	bx, word ptr [bp + 6]
	shl	bx, 1
	mov	ax, word ptr [bx + timer_status]
	push	ax
	mov	word ptr [bx + timer_status], 1
	shl	bx, 1
	mov	ax, word ptr [bp + 8]
	mov	dx, word ptr [bp + 0Ah]
	mov	word ptr [bx + timer_value], ax
	mov	word ptr [bx + timer_value + 2], dx
	mov	word ptr [bx + timer_elapsed], 0
	mov	word ptr [bx + timer_elapsed + 2], 0
	push	cs
	call	near ptr program_timer
	pop	ax
	mov	bx, word ptr [bp + 6]
	shl	bx, 1
	mov	word ptr [bx + timer_status], ax
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_03B6
	sti
L_03B6:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	pop	bp
	retf
_AIL_set_timer_period	ENDP

	PUBLIC	_AIL_set_timer_frequency
_AIL_set_timer_frequency	PROC	FAR
	push	bp
	mov	bp, sp
	push	ds
	push	si
	push	di
	pushf
	cli
	push	word ptr [bp + 0Ah]
	push	word ptr [bp + 8]
	push	ax
	push	bp
	mov	bp, sp
	mov	word ptr [bp + 2], 0Fh
	pop	bp
	push	ax
	push	bp
	mov	bp, sp
	mov	word ptr [bp + 2], 4240h
	pop	bp
	push	cs
	call	near ptr udiv32
	add	sp, 8
	push	dx
	push	ax
	push	word ptr [bp + 6]
	push	cs
	call	near ptr _AIL_set_timer_period
	add	sp, 6
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_03FD
	sti
L_03FD:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	pop	bp
	retf
_AIL_set_timer_frequency	ENDP

	PUBLIC	_AIL_set_timer_divisor
_AIL_set_timer_divisor	PROC	FAR
	push	bp
	mov	bp, sp
	push	ds
	push	si
	push	di
	pushf
	cli
	cmp	word ptr [bp + 8], 0
	jne	L_041A
	mov	ax, 0D68Dh
	mov	dx, 0
	jmp	short L_0428
L_041A:
	mov	ax, 2710h
	mov	bx, 2E9Ch
	mul	word ptr [bp + 8]
	div	bx
	mov	dx, 0
L_0428:
	push	dx
	push	ax
	push	word ptr [bp + 6]
	push	cs
	call	near ptr _AIL_set_timer_period
	add	sp, 6
	push	bp
	mov	bp, sp
	test	byte ptr [bp + 3], 2
	cli
	je	L_043F
	sti
L_043F:
	pop	bp
	popf
	pop	di
	pop	si
	pop	ds
	pop	bp
	retf
_AIL_set_timer_divisor	ENDP

AIL16_TEXT	ENDS

	END
