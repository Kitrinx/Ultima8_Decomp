; name: COLLIDE
; flags: /ml /m5 /zd
; Tests whether a point lands on a solid pixel of a shape frame.

	.MODEL	LARGE, C
	.186
	LOCALS

; A shape frame from its compression word on. Each line offset counts from its own slot.
SHAPEFRAME	STRUC
sfCompressed	dw	?	; bit 0: runs may be one repeated colour
sfWidth		dw	?
sfHeight	dw	?
sfXOff		dw	?
sfYOff		dw	?
sfLines		dw	?	; one per line
SHAPEFRAME	ENDS

	.CODE

; int Collision(ShapeFrame far *frame, int originX, int originY, int x, int y)
; Returns 1 when (x, y) hits a drawn pixel of the frame placed at the origin.
	PUBLIC	Collision
Collision	PROC	C FAR USES si ds, frame:DWORD, originX:WORD, originY:WORD, x:WORD, y:WORD
	LOCAL	spare:DWORD, wide:WORD, compressed:WORD, row:WORD, col:WORD
	pushf
	mov	ax, x
	sub	ax, originX
	mov	x, ax
	mov	ax, y
	sub	ax, originY
	mov	y, ax

	; Turn the point into a line and column of the frame.
	lds	si, frame
	mov	ax, [si].sfCompressed
	mov	compressed, ax
	mov	ax, [si].sfYOff
	add	ax, y
	jl	@@miss
	cmp	ax, [si].sfHeight
	jge	@@miss
	mov	row, ax
	mov	ax, [si].sfXOff
	add	ax, x
	jl	@@miss
	cmp	ax, [si].sfWidth
	jge	@@miss
	mov	col, ax
	mov	ax, [si].sfWidth
	mov	wide, ax
	mov	bx, row
	shl	bx, 1
	add	bx, sfLines
	add	bx, si
	mov	si, [bx]
	add	si, bx

	; Walk the line: a skip count, then a run of pixels, until the column is passed.
	xor	cx, cx
@@nextRun:
	xor	ax, ax
	lodsb
	add	cx, ax
	cmp	cx, col
	jg	@@miss
	cmp	cx, wide
	jge	@@miss
	lodsb
	mov	bx, ax
	test	compressed, 1
	je	@@literal
	shr	bx, 1
	jnc	@@literal
	mov	dx, 1			; a repeated run stores one colour
	jmp	short @@skipData
@@literal:
	mov	dx, bx
@@skipData:
	add	cx, bx
	cmp	cx, col
	jg	@@hit
	add	si, dx
	jmp	short @@nextRun
@@miss:
	xor	ax, ax
	jmp	short @@done
@@hit:
	mov	ax, 1
@@done:
	popf
	ret
Collision	ENDP

	END
