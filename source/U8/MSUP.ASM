; name: MSUP
; flags: /ml /m5 /zd
; Scrolls a viewport's picture up.

	.MODEL	LARGE, C
	.386
	LOCALS
	INCLUDE	VPORT.INC

	.CODE

; void MoveScreenUp(Vport *vp, int dy)
; Moves the picture dy pixels up as one forward block copy.
; Rows are taken to be one viewport width apart, starting at the first row offset.
	PUBLIC	MoveScreenUp
MoveScreenUp	PROC	C FAR USES di si ds es, vp:DWORD, yShift:WORD
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

	; di = top row, si = yShift rows below it.
	lds	bx, [bx].vpYTable
	mov	di, [bx]
	mov	si, di
	mov	ax, yShift
	sub	dx, ax
	imul	ax, cx
	add	si, ax
	imul	cx, dx
	mov	bx, cx			; bytes to move
	cld
	push	bx
	push	ax
	lds	bx, vp
	mov	ax, [bx].vpSeg
	mov	es, ax
	mov	ds, ax
	pop	ax
	pop	bx

	and	cx, 1
	rep	movsb
	mov	cx, bx
	shr	cx, 1
	rep	movsw
	popf
	ret
MoveScreenUp	ENDP

	END
