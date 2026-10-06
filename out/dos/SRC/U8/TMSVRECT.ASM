; name: TMSVRECT
; flags: /ml /m5 /zd
; Saves a block of viewport pixels to a buffer.

	.MODEL	LARGE, C
	.186
	LOCALS
	INCLUDE	VPORT.INC
	INCLUDE	RECT.INC

; Shares CPYVD's code segment, so the linker places it there.
	.CODE	CPYVD_TEXT

; void SaveRect(Vport far *vp, unsigned char far *buffer, Rect far *rect)
; The buffer is laid out as the whole rect, one row after another; only the part inside
; the viewport's clip rectangle is read. RestoreRect is the reverse.
	PUBLIC	SaveRect
SaveRect	PROC	C FAR USES si di ds es, vp:DWORD, buffer:DWORD, rect:DWORD
	LOCAL	rectTop:WORD, rectLeft:WORD, bufPitch:WORD, rectHigh:WORD, top:WORD, left:WORD, \
		wide:WORD, rows:WORD, screenSeg:WORD, spare:DWORD, screenPitch:WORD
	pushf
	cld
	les	si, rect
	lds	bx, vp
	mov	ax, [bx].vpSeg
	mov	screenSeg, ax		; kept but never used
	mov	cx, es:[si].rcX1
	mov	ax, es:[si].rcX2
	sub	ax, cx
	inc	ax
	mov	bufPitch, ax
	mov	wide, ax
	mov	ax, es:[si].rcY2
	mov	cx, es:[si].rcY1
	sub	ax, cx
	inc	ax
	mov	rectHigh, ax
	mov	rows, ax

	; Clip the rect to the viewport.
	mov	ax, es:[si].rcX1
	cmp	ax, [bx].vpX2
	jg	@@farDone
	mov	rectLeft, ax
	mov	left, ax
	mov	ax, [bx].vpX1
	sub	ax, left
	jl	@@leftOk
	sub	wide, ax
	mov	ax, [bx].vpX1
	mov	left, ax
@@leftOk:
	mov	ax, es:[si].rcY1
	cmp	ax, [bx].vpY2
	jle	@@topIn
@@farDone:
	jmp	@@done
@@topIn:
	mov	rectTop, ax
	mov	top, ax
	mov	ax, [bx].vpY1
	sub	ax, top
	jl	@@topOk
	sub	rows, ax
	mov	ax, [bx].vpY1
	mov	top, ax
@@topOk:
	mov	ax, es:[si].rcX2
	cmp	ax, [bx].vpX1
	jl	@@outside
	sub	ax, [bx].vpX2
	jle	@@rightOk
	sub	wide, ax
@@rightOk:
	mov	ax, es:[si].rcY2
	cmp	ax, [bx].vpY1
@@outside:
	jl	@@done			; flags still hold either compare
	sub	ax, [bx].vpY2
	jle	@@bottomOk
	sub	rows, ax
@@bottomOk:

	; es:di at the matching buffer byte, ds:si at the first clipped screen pixel.
	mov	ax, top
	sub	ax, rectTop
	mul	bufPitch
	les	di, buffer
	add	di, ax
	mov	ax, left
	sub	ax, rectLeft
	add	di, ax
	mov	ax, [bx].vpSeg
	lds	si, [bx].vpYTable
	mov	bx, [si+2]
	sub	bx, [si]
	mov	screenPitch, bx
	mov	bx, top
	shl	bx, 1
	mov	si, [bx+si]
	mov	ds, ax
	add	si, left
@@nextRow:
	push	di
	push	si
	mov	cx, wide
	push	cx
	shr	cx, 1
	rep	movsw
	pop	cx
	and	cx, 1
	rep	movsb
	pop	si
	pop	di
	add	di, bufPitch
	add	si, screenPitch
	dec	rows
	jne	@@nextRow
@@done:
	popf
	ret
SaveRect	ENDP

	END
