; name: MSRIGHT
; flags: /ml /m5 /zd
; Scrolls a viewport's picture to the right.

	.MODEL	LARGE, C
	.186
	LOCALS
	INCLUDE	VPORT.INC

	.CODE

; void MoveScreenRight(Vport *vp, int dx)
; Moves the picture dx pixels right, copying each row backwards from the top.
; Rows are taken to be one viewport width apart, starting at the first row offset.
	PUBLIC	MoveScreenRight
MoveScreenRight	PROC	C FAR USES di si ds es, vp:DWORD, xShift:WORD
	pushf

	; cx = width, dx = height
	xor	ax, ax
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

	; di = last pixel of the top row, si = xShift pixels before it.
	lds	bx, [bx].vpYTable
	mov	di, [bx]
	mov	si, di
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
	dec	di
	dec	si
	mov	cx, bx
	shr	cx, 1
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
MoveScreenRight	ENDP

	END
