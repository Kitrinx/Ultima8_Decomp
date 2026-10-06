; name: MSLEFT
; flags: /ml /m5 /zd
; Scrolls a viewport's picture to the left.

	.MODEL	LARGE, C
	.186
	LOCALS
	INCLUDE	VPORT.INC

	.CODE

; void MoveScreenLeft(Vport *vp, int dx)
; Moves the picture dx pixels left, row by row from the top.
; Rows are taken to be one viewport width apart, starting at the first row offset.
	PUBLIC	MoveScreenLeft
MoveScreenLeft	PROC	C FAR USES di si ds es, vp:DWORD, xShift:WORD
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

	; di = top row, si = xShift pixels into it.
	lds	bx, [bx].vpYTable
	mov	di, [bx]
	mov	si, di
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

	; After each row both pointers are xShift short of the next one.
@@nextRow:
	mov	cx, bx
	and	cx, 1
	rep	movsb
	mov	cx, bx
	shr	cx, 1
	rep	movsw
	add	si, ax
	add	di, ax
	dec	dx
	jne	@@nextRow
	popf
	ret
MoveScreenLeft	ENDP

	END
