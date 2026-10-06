; name: VDLINE
; flags: /ml /m5 /zd
; Draws a clipped straight line in a viewport.

	.MODEL	LARGE, C
	.386
	LOCALS
	INCLUDE	VPORT.INC

	.CODE

; void DrawLine(Vport far *vp, int x1, int y1, int x2, int y2, unsigned char color)
; Both ends are drawn. The line is clipped to the viewport by similar triangles, then
; drawn with Bresenham's stepping from its left end.
	PUBLIC	DrawLine
DrawLine	PROC	C FAR USES si di es ds, vp:DWORD, x1:WORD, y1:WORD, x2:WORD, y2:WORD, color:BYTE
	LOCAL	rowStep:WORD, xLen:WORD, yLen:WORD, upward:WORD, pitch:WORD
	pushf
	cld
	lds	si, vp
	mov	ax, [si].vpSeg
	mov	es, ax
	push	ds
	lds	bx, [si].vpYTable
	mov	ax, [bx+2]
	sub	ax, [bx]
	mov	pitch, ax
	pop	ds

	; Make x1 the left end.
	mov	ax, x1
	mov	bx, x2
	mov	cx, y1
	mov	dx, y2
	cmp	bx, ax
	jge	@@leftFirst
	xchg	ax, bx
	xchg	cx, dx
	mov	x1, ax
	mov	x2, bx
	mov	y1, cx
	mov	y2, dx
@@leftFirst:

	; A line rising to the right steps rows upward.
	push	ax
	mov	ax, pitch
	cmp	dx, cx
	jge	@@goingDown
	xchg	cx, dx
	neg	ax
	mov	upward, 1
	jmp	short @@setStep
@@goingDown:
	mov	upward, 0
@@setStep:
	mov	rowStep, ax
	pop	ax

	; Give up when the bounding box misses the viewport, then clip x.
	cmp	bx, [si].vpX1
	jl	@@done
	cmp	ax, [si].vpX2
	jg	@@done
	cmp	dx, [si].vpY1
	jl	@@done
	cmp	cx, [si].vpY2
	jg	@@done
	cmp	bx, [si].vpX2
	jle	@@rightIn
	mov	bx, x2
	sub	bx, x1
	mov	dx, y2
	sub	dx, y1
	mov	ax, [si].vpX2
	sub	ax, x1
	imul	dx
	idiv	bx
	add	ax, y1
	mov	y2, ax
	mov	ax, [si].vpX2
	mov	x2, ax
@@rightIn:
	mov	ax, x1
	cmp	ax, [si].vpX1
	jge	@@leftIn
	mov	bx, x2
	sub	bx, x1
	mov	dx, y2
	sub	dx, y1
	mov	ax, [si].vpX1
	sub	ax, x1
	imul	dx
	idiv	bx
	add	ax, y1
	mov	y1, ax
	mov	ax, [si].vpX1
	mov	x1, ax
@@leftIn:

	; Clip y with the ends ordered top first.
	mov	ax, y1
	mov	bx, y2
	mov	cx, x1
	mov	dx, x2
	test	upward, 0FFh
	je	@@topFirst
	xchg	ax, bx
	xchg	cx, dx
	mov	y1, ax
	mov	y2, bx
	mov	x1, cx
	mov	x2, dx
@@topFirst:
	cmp	bx, [si].vpY1
	jl	@@done
	cmp	ax, [si].vpY2
	jg	@@done
	cmp	bx, [si].vpY2
	jle	@@bottomIn
	sub	bx, ax
	mov	dx, x2
	sub	dx, x1
	mov	ax, [si].vpY2
	sub	ax, y1
	imul	dx
	idiv	bx
	add	ax, x1
	mov	x2, ax
	mov	ax, [si].vpY2
	mov	y2, ax
@@bottomIn:
	mov	ax, y1
	mov	bx, y2
	cmp	ax, [si].vpY1
	jge	@@topIn
	mov	bx, y2
	sub	bx, y1
	mov	dx, x2
	sub	dx, x1
	mov	ax, [si].vpY1
	sub	ax, y1
	imul	dx
	idiv	bx
	add	ax, x1
	mov	x1, ax
	mov	ax, [si].vpY1
	mov	y1, ax
@@topIn:
	mov	bx, y2
	sub	bx, y1
	mov	yLen, bx

	; Back to left end first.
	test	upward, 0FFh
	je	@@ordered
	mov	ax, y1
	mov	bx, y2
	mov	cx, x1
	mov	dx, x2
	xchg	ax, bx
	xchg	cx, dx
	mov	y1, ax
	mov	y2, bx
	mov	x1, cx
	mov	x2, dx
@@ordered:
	mov	bx, x2
	sub	bx, x1
	mov	xLen, bx

	; es:di at the left end.
	push	es
	les	si, [si].vpYTable
	mov	cx, y1
	shl	cx, 1
	add	cx, si
	mov	ax, x1
	push	bx
	mov	bx, cx
	add	ax, es:[bx]
	pop	bx
	mov	di, ax
	pop	es
	mov	al, color
	xor	cx, cx
	mov	bx, x1
	cmp	bx, x2
	je	@@vertical
	mov	bx, y1
	cmp	bx, y2
	je	@@horizontal
	mov	bx, xLen
	mov	dx, yLen
	cmp	dx, bx
	jg	@@steep

	; Mostly sideways: one pixel per column, cx counts toward a row change.
	mov	cx, bx
	shr	cx, 1
	mov	si, bx
	inc	si
@@shallowPixel:
	stosb
	dec	si
	je	@@done
	add	cx, dx
	cmp	cx, bx
	jl	@@shallowPixel
	add	di, rowStep
	sub	cx, bx
	jmp	short @@shallowPixel

	; Mostly up or down: one pixel per row, cx counts toward a column change.
@@steep:
	mov	cx, dx
	shr	cx, 1
	mov	si, dx
	inc	si
	dec	rowStep			; stosb has already moved one right
@@steepPixel:
	stosb
	add	di, rowStep
	dec	si
	je	@@done
	add	cx, bx
	cmp	cx, dx
	jl	@@steepPixel
	inc	di
	sub	cx, dx
	jmp	short @@steepPixel

@@vertical:
	mov	cx, yLen
	inc	cx
	mov	bx, rowStep
	dec	bx
@@verticalPixel:
	stosb
	add	di, bx
	loop	@@verticalPixel
	jmp	short @@done

@@horizontal:
	mov	cx, xLen
	inc	cx
	mov	ah, al
	mov	bx, cx
	and	bx, 1
	shr	cx, 1
	rep	stosw
	mov	cx, bx
	rep	stosb
@@done:
	popf
	ret
DrawLine	ENDP

	END
