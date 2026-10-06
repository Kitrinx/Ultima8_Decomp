; name: MSUR
; flags: /ml /m5 /zd
; Scrolls a viewport's picture up and to the right.

	.MODEL	LARGE, C
	.386
	LOCALS
	INCLUDE	VPORT.INC

	.CODE

; void MoveScreenUpRight(Vport *vp, int dx, int dy)
; Moves the picture dx pixels right and dy pixels up, working from the top row down
; and copying each row backwards.
; Rows are taken to be one viewport width apart, starting at the first row offset.
	PUBLIC	MoveScreenUpRight
MoveScreenUpRight	PROC	C FAR USES di si ds es, vp:DWORD, xShift:WORD, yShift:WORD
	pushf

	; cx = width, dx = height
	xor	cx, cx
	xor	dx, dx
	lds	bx, vp
	mov	cx, [bx].vpX2
	mov	ax, [bx].vpX1
	sub	cx, ax
	inc	cx
	mov	dx, [bx].vpY2
	mov	ax, [bx].vpY1
	sub	dx, ax
	inc	dx

	; di = last pixel of the top row, si = yShift rows below it, xShift pixels back.
	lds	bx, [bx].vpYTable
	mov	di, [bx]
	mov	si, di
	mov	ax, yShift
	sub	dx, ax
	imul	ax, cx
	add	si, ax
	xor	ax, ax
	mov	ax, xShift
	add	si, cx
	add	di, cx
	sub	si, ax
	sub	cx, ax
	mov	bx, cx			; bytes per row
	dec	si
	dec	di
	std
	push	bx
	push	ax
	lds	bx, vp
	mov	ax, [bx].vpSeg
	mov	es, ax
	mov	ds, ax
	pop	ax
	pop	bx

	; Odd byte first, then back up one so the words copy whole.
@@nextRow:
	push	di
	push	si
	mov	cx, bx
	and	cx, 1
	rep	movsb
	mov	cx, bx
	shr	cx, 1
	dec	si
	dec	di
	rep	movsw
	pop	si
	pop	di
	add	si, bx
	add	si, ax
	add	di, bx
	add	di, ax
	dec	dx
	jne	@@nextRow
	popf
	ret
MoveScreenUpRight	ENDP

	END
