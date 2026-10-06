; name: VDRECT
; flags: /ml /m5 /zd
; Draws a one-pixel rectangle outline in a viewport.

	.MODEL	LARGE, C
	.386
	LOCALS
	INCLUDE	VPORT.INC

	.CODE

; void DrawRectangle(Vport far *vp, int x1, int y1, int x2, int y2, unsigned char color)
; Corners are inclusive. Each side clipped off by the viewport is left out.
	PUBLIC	DrawRectangle
DrawRectangle	PROC	C FAR USES si di es ds, vp:DWORD, x1:WORD, y1:WORD, x2:WORD, y2:WORD, color:BYTE
	LOCAL	pitch:WORD, drawLeft:WORD, drawRight:WORD, drawTop:WORD, drawBottom:WORD, \
		topLeft:WORD, topRight:WORD, bottomLeft:WORD, words:WORD, oddByte:WORD
	pushf
	cld
	lds	bx, vp

	; Clip the corners, noting which sides are still visible.
	mov	drawLeft, 1
	mov	cx, x1
	cmp	cx, [bx].vpX2
	jg	@@done
	cmp	cx, [bx].vpX1
	jge	@@x1Ok
	mov	drawLeft, 0
	mov	cx, [bx].vpX1
	mov	x1, cx
@@x1Ok:
	mov	drawRight, 1
	mov	cx, x2
	cmp	cx, [bx].vpX1
	jl	@@done
	cmp	cx, [bx].vpX2
	jle	@@x2Ok
	mov	drawRight, 0
	mov	cx, [bx].vpX2
	mov	x2, cx
@@x2Ok:
	mov	drawTop, 1
	mov	cx, y1
	cmp	cx, [bx].vpY2
	jg	@@done
	cmp	cx, [bx].vpY1
	jge	@@y1Ok
	mov	drawTop, 0
	mov	cx, [bx].vpY1
	mov	y1, cx
@@y1Ok:
	mov	drawBottom, 1
	mov	cx, y2
	cmp	cx, [bx].vpY1
	jl	@@done
	cmp	cx, [bx].vpY2
	jle	@@y2Ok
	mov	drawBottom, 0
	mov	cx, [bx].vpY2
	mov	y2, cx
@@y2Ok:

	; Horizontal sides are stored as words plus an odd byte.
	mov	dx, x2
	sub	dx, x1
	inc	dx
	mov	ax, dx
	and	ax, 1
	mov	oddByte, ax
	mov	ax, dx
	shr	ax, 1
	mov	words, ax

	; Screen offsets of three corners; the row pitch comes from the first two row offsets.
	les	si, [bx].vpYTable
	mov	cx, es:[si+2]
	sub	cx, es:[si]
	mov	pitch, cx
	mov	cx, y1
	shl	cx, 1
	add	cx, si
	mov	ax, x1
	push	bx
	mov	bx, cx
	add	ax, es:[bx]
	pop	bx
	mov	topLeft, ax
	add	ax, dx
	dec	ax
	mov	topRight, ax
	mov	cx, y2
	shl	cx, 1
	add	cx, si
	mov	ax, x1
	push	bx
	mov	bx, cx
	add	ax, es:[bx]
	pop	bx
	mov	bottomLeft, ax
	mov	si, y2
	sub	si, y1
	inc	si
	mov	ax, [bx].vpSeg
	mov	es, ax
	mov	bx, pitch
	mov	al, color
	mov	ah, al
	xor	cx, cx

	test	drawTop, 1
	je	@@noTop
	mov	di, topLeft
	mov	cx, words
	rep	stosw
	mov	cx, oddByte
	rep	stosb
@@noTop:
	test	drawBottom, 1
	je	@@noBottom
	mov	di, bottomLeft
	mov	cx, words
	rep	stosw
	mov	cx, oddByte
	rep	stosb
@@noBottom:
	test	drawLeft, 1
	je	@@noLeft
	mov	di, topLeft
	mov	cx, si
@@leftPixel:
	mov	es:[di], al
	add	di, bx
	loop	@@leftPixel
@@noLeft:
	test	drawRight, 1
	je	@@done
	mov	di, topRight
	mov	cx, si
@@rightPixel:
	mov	es:[di], al
	add	di, bx
	loop	@@rightPixel
@@done:
	popf
	ret
DrawRectangle	ENDP

	END
