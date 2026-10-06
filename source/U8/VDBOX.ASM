; name: VDBOX
; flags: /ml /m5 /zd
; Fills a rectangle of a viewport with one colour.

	.MODEL	LARGE, C
	.386
	LOCALS
	INCLUDE	VPORT.INC

	.CODE

; void DrawBox(Vport far *vp, int x1, int y1, int x2, int y2, unsigned char color)
; Corners are inclusive and clipped to the viewport.
	PUBLIC	DrawBox
DrawBox	PROC	C FAR USES si di es ds, vp:DWORD, x1:WORD, y1:WORD, x2:WORD, y2:WORD, color:BYTE
	LOCAL	rowSkip:WORD, words:WORD, oddByte:WORD
	pushf
	cld
	lds	bx, vp

	; Clip the corners; give up when the box misses the viewport.
	mov	cx, x1
	cmp	cx, [bx].vpX2
	jg	@@done
	cmp	cx, [bx].vpX1
	jge	@@x1Ok
	mov	cx, [bx].vpX1
	mov	x1, cx
@@x1Ok:
	mov	cx, x2
	cmp	cx, [bx].vpX1
	jl	@@done
	cmp	cx, [bx].vpX2
	jle	@@x2Ok
	mov	cx, [bx].vpX2
	mov	x2, cx
@@x2Ok:
	mov	cx, y1
	cmp	cx, [bx].vpY2
	jg	@@done
	cmp	cx, [bx].vpY1
	jge	@@y1Ok
	mov	cx, [bx].vpY1
	mov	y1, cx
@@y1Ok:
	mov	cx, y2
	cmp	cx, [bx].vpY1
	jl	@@done
	cmp	cx, [bx].vpY2
	jle	@@y2Ok
	mov	cx, [bx].vpY2
	mov	y2, cx
@@y2Ok:

	; Each row is stored as words plus an odd byte.
	mov	dx, x2
	sub	dx, x1
	inc	dx
	mov	ax, dx
	and	ax, 1
	mov	oddByte, ax
	mov	ax, dx
	shr	ax, 1
	mov	words, ax

	; The row pitch is the distance between the first two row offsets.
	les	di, [bx].vpYTable
	mov	cx, es:[di+2]
	sub	cx, es:[di]
	sub	cx, dx
	mov	rowSkip, cx
	push	bx
	mov	bx, y1
	shl	bx, 1
	mov	bx, es:[bx+di]
	add	bx, x1
	mov	di, bx
	pop	bx
	mov	si, y2
	sub	si, y1
	inc	si
	mov	dx, rowSkip
	mov	ax, [bx].vpSeg
	mov	es, ax
	mov	al, color
	mov	ah, al
@@nextRow:
	mov	cx, words
	rep	stosw
	mov	cx, oddByte
	rep	stosb
	add	di, dx
	dec	si
	jne	@@nextRow
@@done:
	popf
	ret
DrawBox	ENDP

	END
