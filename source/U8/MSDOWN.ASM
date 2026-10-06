; name: MSDOWN
; flags: /ml /m5 /zd
; Scrolls a viewport's picture down.

	.MODEL	LARGE, C
	.386
	LOCALS
	INCLUDE	VPORT.INC

	.CODE

; void MoveScreenDown(Vport *vp, int dy)
; Moves the picture dy pixels down as one backward block copy.
; Rows are taken to be one viewport width apart, starting at the first row offset.
	PUBLIC	MoveScreenDown
MoveScreenDown	PROC	C FAR USES di si ds es, vp:DWORD, yShift:WORD
	pushf

	; cx = width, dx = height
	xor	cx, cx
	xor	dx, dx
	xor	ax, ax
	lds	bx, vp
	mov	cx, [bx].vpX2
	mov	ax, [bx].vpX1
	sub	cx, ax
	inc	cx
	mov	dx, [bx].vpY2
	mov	ax, [bx].vpY1
	sub	dx, ax
	inc	dx

	; di = last pixel, si = yShift rows above it.
	lds	bx, [bx].vpYTable
	mov	di, [bx]
	mov	si, di
	mov	ax, cx
	imul	ax, dx
	add	di, ax
	dec	di
	mov	si, di
	xor	ax, ax
	mov	ax, yShift
	sub	dx, ax
	imul	ax, cx
	sub	si, ax
	imul	cx, dx
	mov	bx, cx			; bytes to move
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
	and	cx, 1
	rep	movsb
	mov	cx, bx
	dec	di
	dec	si
	shr	cx, 1
	rep	movsw
	popf
	ret
MoveScreenDown	ENDP

	END
