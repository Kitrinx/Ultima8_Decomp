; name: CPYVPT
; flags: /ml /m5 /zd
; Copies one viewport's pixels into another.

	.MODEL	LARGE, C
	.386
	LOCALS
	INCLUDE	VPORT.INC

	.CODE

; void CopyViewport(Vport far *dest, Vport far *src)
; Copies the overlap of both clip sizes, from each one's top left corner. Both sides
; start at src's top row.
	PUBLIC	CopyViewport
CopyViewport	PROC	C FAR USES si di ds es, dest:DWORD, src:DWORD
	LOCAL	wide:WORD, high:WORD, srcX:WORD, destX:WORD, rows:WORD
	pushf
	cld

	; Size: the smaller of the two clip rectangles.
	lds	bx, src
	mov	ax, [bx].vpX2
	mov	cx, [bx].vpX1
	mov	srcX, cx
	sub	ax, cx
	inc	ax
	je	@@done
	mov	wide, ax
	xor	si, si
	mov	si, [bx].vpY2
	sub	si, [bx].vpY1
	inc	si
	mov	high, si
	lds	di, dest
	mov	ax, [di].vpX2
	mov	cx, [di].vpX1
	mov	destX, cx
	sub	ax, cx
	inc	ax
	cmp	ax, wide
	jge	@@wideOk
	mov	wide, ax
@@wideOk:
	xor	si, si
	mov	si, [di].vpY2
	sub	si, [di].vpY1
	inc	si
	cmp	si, high
	jge	@@highOk
	mov	high, si
@@highOk:
	mov	dx, high
	cmp	dx, 0
	je	@@done

	; Row table pointers: es:di into dest's, ds:si into src's.
	les	di, [di].vpYTable
	push	es
	lds	bx, src
	mov	si, [bx].vpY1
	shl	si, 1
	add	di, si
	les	si, [bx].vpYTable
	push	di
	mov	di, [bx].vpY1
	shl	di, 1
	add	si, di
	pop	di
	lds	bx, dest
	mov	ax, [bx].vpSeg
	lds	bx, src
	push	dx
	mov	dx, [bx].vpSeg
	push	es
	pop	ds
	pop	rows
	pop	es
	mov	bx, wide

@@nextRow:
	push	es
	push	di
	push	ds
	push	si
	mov	di, es:[di]
	mov	es, ax
	add	di, destX
	mov	si, [si]
	add	si, srcX
	mov	ds, dx
	mov	cx, bx
	shr	cx, 1
	rep	movsw
	mov	cx, bx
	and	cx, 1
	rep	movsb
	pop	si
	pop	ds
	pop	di
	pop	es
	add	di, 2
	add	si, 2
	dec	rows
	jne	@@nextRow

@@done:
	popf
	ret
CopyViewport	ENDP

	END
