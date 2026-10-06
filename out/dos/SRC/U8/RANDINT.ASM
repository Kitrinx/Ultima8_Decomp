; name: RANDINT
; data: 2AAh
; Hand-written random number generator: seed = seed * 10003h + 1, 32 bits.

	.MODEL	LARGE
	LOCALS

	.DATA

seed		dd	0BAD0BADh

	.CODE

; int RandInt(void)
; Steps the seed and returns its high word.
	PUBLIC	_RandInt
_RandInt	PROC	FAR
	mov	cx, word ptr seed
	mov	bx, cx
	mov	ax, word ptr seed + 2
	add	cx, cx
	rcl	ax, 1
	add	cx, bx
	adc	ax, word ptr seed + 2
	add	ax, bx
	inc	cx
	adc	ax, 0
	mov	word ptr seed, cx
	mov	word ptr seed + 2, ax
	retf
_RandInt	ENDP

; int Ultima5Random(int a, int b)
; a to b inclusive, either way round (unsigned).
	PUBLIC	_Ultima5Random
_Ultima5Random	PROC	FAR
	push	bp
	mov	bp, sp
	push	cs
	call	near ptr _RandInt
	mov	bx, [bp + 8]
	mov	cx, [bp + 6]
	cmp	bx, cx
	jae	@@ordered
	xchg	bx, cx
@@ordered:
	xor	dx, dx
	sub	bx, cx
	inc	bx
	je	@@full
	div	bx
@@full:
	add	dx, cx
	mov	ax, dx
	pop	bp
	retf
_Ultima5Random	ENDP

; void SetRandomSeed(long s)
; The low word is made odd.
	PUBLIC	_SetRandomSeed
_SetRandomSeed	PROC	FAR
	push	bp
	mov	bp, sp
	mov	ax, [bp + 6]
	or	ax, 1
	mov	word ptr seed, ax
	mov	ax, [bp + 8]
	mov	word ptr seed + 2, ax
	pop	bp
	retf
_SetRandomSeed	ENDP

; long GetRandomSeed(void)
	PUBLIC	_GetRandomSeed
_GetRandomSeed	PROC	FAR
	mov	ax, word ptr seed
	mov	dx, word ptr seed + 2
	retf
_GetRandomSeed	ENDP

; int PseudoRandom(int n)
; 0 to n - 1; with n 0, the whole word.
	PUBLIC	_PseudoRandom
_PseudoRandom	PROC	FAR
	push	bp
	mov	bp, sp
	push	cs
	call	near ptr _RandInt
	mov	cx, [bp + 6]
	xor	dx, dx
	jcxz	@@done
	div	cx
@@done:
	mov	ax, dx
	pop	bp
	retf
_PseudoRandom	ENDP

	END
