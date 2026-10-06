; name: TMRSRECT
; flags: /ml /m5 /zd
; Copies a saved block of pixels back into a viewport.

	.MODEL	LARGE, C
	.186
	LOCALS
	INCLUDE	VPORT.INC
	INCLUDE	RECT.INC

; Shares CPYVD's code segment, so the linker places it there.
	.CODE	CPYVD_TEXT

; void RestoreRect(Vport far *vp, unsigned char far *buffer, Rect far *rect)
; The buffer holds the whole rect, one row after another; only the part inside the
; viewport's clip rectangle is written back. SaveRect is the reverse.
	PUBLIC	RestoreRect
RestoreRect	PROC	C FAR USES si di ds es, vp:DWORD, buffer:DWORD, rect:DWORD
	LOCAL	rectTop:WORD, rectLeft:WORD, bufPitch:WORD, rectHigh:WORD, top:WORD, left:WORD, \
		wide:WORD, rows:WORD, spare:WORD:3, screenPitch:WORD
	pushf
	cld
	les	di, rect
	lds	bx, vp
	mov	ax, es:[di].rcX2
	mov	cx, es:[di].rcX1
	sub	ax, cx
	inc	ax
	mov	bufPitch, ax
	mov	wide, ax
	mov	ax, es:[di].rcY2
	mov	cx, es:[di].rcY1
	sub	ax, cx
	inc	ax
	mov	rectHigh, ax
	mov	rows, ax

	; Clip the rect to the viewport.
	mov	ax, es:[di].rcX1
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
	mov	ax, es:[di].rcY1
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
	mov	ax, es:[di].rcX2
	cmp	ax, [bx].vpX1
	jl	@@outside
	sub	ax, [bx].vpX2
	jle	@@rightOk
	sub	wide, ax
@@rightOk:
	mov	ax, es:[di].rcY2
	cmp	ax, [bx].vpY1
@@outside:
	jl	@@done			; flags still hold either compare
	sub	ax, [bx].vpY2
	jle	@@bottomOk
	sub	rows, ax
@@bottomOk:

	; es:di at the first clipped screen pixel, ds:si at the matching buffer byte.
	mov	ax, top
	sub	ax, rectTop
	mul	bufPitch
	mov	cx, [bx].vpSeg
	les	di, [bx].vpYTable
	mov	bx, es:[di+2]
	sub	bx, es:[di]
	mov	screenPitch, bx
	mov	bx, top
	shl	bx, 1
	mov	di, es:[bx+di]
	mov	es, cx
	add	di, left
	lds	si, buffer
	add	si, ax
	mov	ax, left
	sub	ax, rectLeft
	add	si, ax
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
	add	di, screenPitch
	add	si, bufPitch
	dec	rows
	jne	@@nextRow
@@done:
	popf
	ret
RestoreRect	ENDP

	END
