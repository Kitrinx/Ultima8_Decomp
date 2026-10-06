; name: CLRVPT
; flags: /ml /m5 /zd
; Fills a viewport's whole clip rectangle with one colour.

	.MODEL	LARGE, C
	.386
	LOCALS
	INCLUDE	VPORT.INC

	.CODE

; void ClearViewport(Vport far *vp, unsigned char color)
	PUBLIC	ClearViewport
ClearViewport	PROC	C FAR USES si di ds es fs gs, vp:DWORD, color:BYTE
	LOCAL	left:WORD
	pushf
	cld
	xor	bx, bx
	lds	bx, vp
	mov	dx, [bx].vpX2
	mov	ax, [bx].vpX1
	mov	left, ax
	sub	dx, ax
	inc	dx
	mov	si, [bx].vpY2
	sub	si, [bx].vpY1
	inc	si
	je	@@done
	mov	cx, [bx].vpSeg
	mov	es, cx
	xor	cx, cx

	; ds:di walks the row table from the top clip line.
	mov	ax, [bx].vpY1
	shl	ax, 1
	lds	di, [bx].vpYTable
	add	di, ax
	mov	al, color
	mov	ah, al
	mov	bx, left
	push	bp			; bp is borrowed as the row pointer
@@nextRow:
	mov	bp, di
	mov	di, [di]
	add	di, bx
	mov	cx, dx
	shr	cx, 1
	rep	stosw
	mov	cx, dx
	and	cx, 1
	rep	stosb
	mov	di, bp
	add	di, 2
	dec	si
	jne	@@nextRow
	pop	bp
@@done:
	popf
	ret
ClearViewport	ENDP

	END
