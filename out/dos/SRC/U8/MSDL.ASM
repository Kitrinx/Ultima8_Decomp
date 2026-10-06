; name: MSDL
; flags: /ml /m5 /zd
; Scrolls a viewport's picture down and to the left.

	.MODEL	LARGE, C
	.386
	LOCALS
	INCLUDE	VPORT.INC

	.CODE

; void MoveScreenDownLeft(Vport *vp, int dx, int dy)
; Moves the picture dx pixels left and dy pixels down, working from the bottom row up.
; Rows are taken to be one viewport width apart, starting at the first row offset.
	PUBLIC	MoveScreenDownLeft
MoveScreenDownLeft	PROC	C FAR USES di si es ds, vp:DWORD, xShift:WORD, yShift:WORD
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

	; di = start of the bottom row, si = yShift rows above it, xShift pixels in.
	lds	bx, [bx].vpYTable
	mov	di, [bx]
	mov	ax, cx
	imul	ax, dx
	add	di, ax
	sub	di, cx
	mov	si, di
	xor	ax, ax
	mov	ax, yShift
	sub	dx, ax
	imul	ax, cx
	sub	si, ax
	xor	ax, ax
	mov	ax, xShift
	add	si, ax
	sub	cx, ax
	mov	bx, cx			; bytes per row
	cld
	push	bx
	push	ax
	lds	bx, vp
	mov	ax, [bx].vpSeg
	mov	es, ax
	mov	ds, ax
	pop	ax
	pop	bx

	; Copy each row, then step both pointers back to the start of the row above.
@@nextRow:
	mov	cx, bx
	and	cx, 1
	rep	movsb
	mov	cx, bx
	shr	cx, 1
	rep	movsw
	sub	di, bx
	sub	di, bx
	sub	di, ax
	sub	si, bx
	sub	si, bx
	sub	si, ax
	dec	dx
	jne	@@nextRow
	popf
	ret
MoveScreenDownLeft	ENDP

	END
