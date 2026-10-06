; name: SDDXF
; flags: /ml /m5 /zd
; Blends a run-length shape frame with the screen through a table, clipped.

	.MODEL	LARGE, C
	.386
	LOCALS
	INCLUDE	VPORT.INC
	INCLUDE	SKIPDRAW.INC

; A shape pixel whose mix entry is this is drawn as is.
KEEP_PIXEL	EQU	0FFh

	.CODE

; int SkipDoubleTransformDraw(Vport far *vp, int x, int y, SKIPFRAME far *frame,
;                             unsigned char far *mix)
; A shape pixel s over screen pixel p draws mix[mix[s] * 256 + p], or s
; itself when mix[s] is KEEP_PIXEL. Returns 1 when the frame misses the
; viewport, else 0.
	PUBLIC	SkipDoubleTransformDraw
SkipDoubleTransformDraw	PROC	C FAR USES di si es ds, vp:DWORD, x:WORD, y:WORD, frame:DWORD, mix:DWORD
	LOCAL	view:VPORT, pitch:WORD, skipRows:WORD, rows:WORD, cols:WORD, skipLeft:WORD, clip:WORD
	pushf
	cld

	; Work from a copy of the viewport.
	lds	si, vp
	push	ss
	pop	es
	lea	di, view
	mov	cx, SIZE VPORT
	mov	ax, cx
	shr	cx, 1
	rep movsw
	mov	cx, ax
	and	cx, 1
	rep movsb

	lds	si, frame
	mov	ax, x
	sub	ax, [si].sfXOff
	cmp	ax, view.vpX2
	jg	@@missed
	add	ax, [si].sfWidth
	dec	ax
	cmp	ax, view.vpX1
	jl	@@missed
	mov	ax, y
	sub	ax, [si].sfYOff
	cmp	ax, view.vpY2
	jg	@@missed
	add	ax, [si].sfHeight
	dec	ax
	cmp	ax, view.vpY1
	jge	@@visible
@@missed:
	mov	ax, 1
	jmp	@@done

	; Trim the frame to the clip rectangle.
@@visible:
	mov	ax, [si].sfHeight
	mov	rows, ax
	mov	ax, [si].sfWidth
	mov	cols, ax
	mov	es, view.vpSeg
	xor	ax, ax
	mov	skipRows, ax
	mov	skipLeft, ax
	mov	clip, ax
	mov	ax, y
	mov	bx, [si].sfYOff
	sub	ax, bx
	mov	bx, view.vpY1
	cmp	ax, bx
	jge	@@topOk
	sub	bx, ax
	mov	skipRows, bx
	sub	rows, bx
@@topOk:
	mov	bx, [si].sfHeight
	add	ax, bx
	dec	ax
	mov	bx, view.vpY2
	cmp	ax, bx
	jle	@@bottomOk
	sub	ax, bx
	sub	rows, ax
@@bottomOk:
	mov	ax, x
	mov	bx, [si].sfXOff
	sub	ax, bx
	mov	bx, view.vpX1
	cmp	ax, bx
	jge	@@leftOk
	sub	bx, ax
	mov	skipLeft, bx
	sub	cols, bx
	or	clip, CLIP_HEAD
@@leftOk:
	mov	bx, [si].sfWidth
	add	ax, bx
	dec	ax
	mov	bx, view.vpX2
	cmp	ax, bx
	jle	@@rightOk
	sub	ax, bx
	sub	cols, ax
	or	clip, CLIP_TAIL
@@rightOk:
	mov	ax, clip
	or	ax, ax
	je	@@whole
	jmp	@@clipped

	; Whole rows: di = screen offset of the first row's left edge.
@@whole:
	mov	cx, y
	sub	cx, [si].sfYOff
	add	cx, skipRows
	shl	cx, 1
	push	ds
	lds	bx, view.vpYTable
	mov	ax, [bx]
	mov	dx, [bx+2]
	sub	dx, ax
	mov	pitch, dx
	add	bx, cx
	mov	di, [bx]
	pop	ds
	sub	di, [si].sfXOff
	add	di, x
	mov	cx, [si].sfFlags
	test	cx, SF_COMPRESSED
	je	@@raw
	jmp	short @@packed

@@raw:
	mov	bx, si
	add	bx, sfRows
	mov	cx, skipRows
	shl	cx, 1
	add	bx, cx
	mov	dx, rows
	xor	cx, cx
@@rawRow:
	mov	ax, cols
	mov	si, [bx]
	add	si, bx
	add	bx, 2
	push	di
	push	dx
	push	bx
@@rawSkip:
	mov	cl, [si]
	inc	si
	sub	ax, cx
	je	@@rawRowDone
	add	di, cx
	mov	cl, [si]
	mov	dx, cx
	inc	si
	push	dx
	xor	bx, bx
	xor	dx, dx
@@rawBlend:
	mov	bl, [si]
	inc	si
	push	ds
	push	si
	lds	si, mix
	mov	dl, [bx+si]
	cmp	dl, KEEP_PIXEL
	je	@@rawKeep
	mov	bl, es:[di]
	shl	dx, 8
	add	bx, dx
	mov	dl, [bx+si]
	xor	dh, dh
	mov	bx, dx
@@rawKeep:
	mov	es:[di], bl
	inc	di
	pop	si
	pop	ds
	dec	cx
	jne	@@rawBlend
	pop	dx
	sub	ax, dx
	jne	@@rawSkip
@@rawRowDone:
	pop	bx
	pop	dx
	pop	di
	add	di, pitch
	dec	dx
	jne	@@rawRow
	xor	ax, ax
	jmp	@@done

@@packed:
	mov	bx, si
	add	bx, sfRows
	mov	cx, skipRows
	shl	cx, 1
	add	bx, cx
	mov	dx, rows
	xor	cx, cx
@@packRow:
	mov	ax, cols
	mov	si, [bx]
	add	si, bx
	add	bx, 2
	push	di
	push	dx
	push	bx
@@packSkip:
	mov	cl, [si]
	inc	si
	sub	ax, cx
	je	@@packRowDone
	add	di, cx
	mov	cl, [si]
	inc	si
	shr	cl, 1
	jae	@@packCopy
	mov	dx, cx
	push	dx
	xor	bx, bx
	xor	dx, dx
	mov	bl, [si]
	inc	si
@@packFillBlend:
	push	bx
	push	ds
	push	si
	lds	si, mix
	mov	dl, [bx+si]
	cmp	dl, KEEP_PIXEL
	je	@@packFillKeep
	mov	bl, es:[di]
	shl	dx, 8
	add	bx, dx
	mov	dl, [bx+si]
	xor	dh, dh
	mov	bx, dx
@@packFillKeep:
	mov	es:[di], bl
	inc	di
	pop	si
	pop	ds
	pop	bx
	dec	cx
	jne	@@packFillBlend
	pop	dx
	jmp	short @@packNext
@@packCopy:
	mov	dx, cx
	push	dx
	xor	bx, bx
	xor	dx, dx
@@packCopyBlend:
	mov	bl, [si]
	inc	si
	push	ds
	push	si
	lds	si, mix
	mov	dl, [bx+si]
	cmp	dl, KEEP_PIXEL
	je	@@packCopyKeep
	mov	bl, es:[di]
	shl	dx, 8
	add	bx, dx
	mov	dl, [bx+si]
	xor	dh, dh
	mov	bx, dx
@@packCopyKeep:
	mov	es:[di], bl
	inc	di
	pop	si
	pop	ds
	dec	cx
	jne	@@packCopyBlend
	pop	dx
@@packNext:
	sub	ax, dx
	jne	@@packSkip
@@packRowDone:
	pop	bx
	pop	dx
	pop	di
	add	di, pitch
	dec	dx
	jne	@@packRow
	xor	ax, ax
	jmp	@@done

	; Clipped rows: runs are trimmed to cols pixels, starting skipLeft in.
@@clipped:
	mov	cx, y
	sub	cx, [si].sfYOff
	add	cx, skipRows
	shl	cx, 1
	push	ds
	lds	bx, view.vpYTable
	mov	ax, [bx]
	mov	dx, [bx+2]
	sub	dx, ax
	mov	pitch, dx
	add	bx, cx
	mov	di, [bx]
	pop	ds
	sub	di, [si].sfXOff
	add	di, x
	mov	bx, si
	add	bx, sfRows
	mov	cx, skipRows
	shl	cx, 1
	add	bx, cx
	mov	dx, rows
	mov	cx, [si].sfFlags
	test	cx, SF_COMPRESSED
	je	@@rawClipped
	jmp	@@packClipped

@@rawClipped:
	mov	cx, clip
	cmp	cx, CLIP_HEAD
	jne	@@rawNotHead
	jmp	short @@rawHead
@@rawNotHead:
	cmp	cx, CLIP_TAIL
	jne	@@rawHead
	jmp	@@rawTail

	; Raw rows cut at the start (and maybe the end).
@@rawHead:
	xor	cx, cx
@@rawHeadRow:
	mov	ax, cols
	mov	si, [bx]
	add	si, bx
	add	bx, 2
	push	di
	push	dx
	push	bx
	mov	dx, skipLeft
	add	di, dx
	; Walk the hidden part; dx counts the pixels still to pass.
@@rawHidden:
	mov	cl, [si]
	inc	si
	sub	dx, cx
	je	@@rawHeadRun
	jl	@@rawInSkip
	mov	cl, [si]
	inc	si
	add	si, cx
	sub	dx, cx
	je	@@rawHeadSkip
	jb	@@rawInRun
	jmp	short @@rawHidden
@@rawInRun:
	sub	si, cx
	add	dx, cx
	sub	cx, dx
	add	si, dx
	jmp	short @@rawHeadCopy
@@rawInSkip:
	not	dx
	inc	dx
	mov	cx, dx
	jmp	short @@rawHeadGap
@@rawHeadSkip:
	mov	cl, [si]
	inc	si
@@rawHeadGap:
	cmp	ax, cx
	jg	@@rawGapFits
	mov	cx, ax
@@rawGapFits:
	sub	ax, cx
	je	@@rawHeadRowDone
	add	di, cx
@@rawHeadRun:
	mov	cl, [si]
	inc	si
@@rawHeadCopy:
	cmp	ax, cx
	jg	@@rawRunFits
	mov	cx, ax
@@rawRunFits:
	mov	dx, cx
	push	dx
	xor	bx, bx
	xor	dx, dx
@@rawHeadBlend:
	mov	bl, [si]
	inc	si
	push	ds
	push	si
	lds	si, mix
	mov	dl, [bx+si]
	cmp	dl, KEEP_PIXEL
	je	@@rawHeadKeep
	mov	bl, es:[di]
	shl	dx, 8
	add	bx, dx
	mov	dl, [bx+si]
	xor	dh, dh
	mov	bx, dx
@@rawHeadKeep:
	mov	es:[di], bl
	inc	di
	pop	si
	pop	ds
	dec	cx
	jne	@@rawHeadBlend
	pop	dx
	sub	ax, dx
	jne	@@rawHeadSkip
@@rawHeadRowDone:
	pop	bx
	pop	dx
	pop	di
	add	di, pitch
	dec	dx
	jne	@@rawHeadRow
	xor	ax, ax
	jmp	@@done

	; Raw rows cut at the end only.
@@rawTail:
	xor	cx, cx
@@rawTailRow:
	mov	ax, cols
	mov	si, [bx]
	add	si, bx
	add	bx, 2
	push	di
	push	dx
	push	bx
@@rawTailSkip:
	mov	cl, [si]
	inc	si
	sub	ax, cx
	jle	@@rawTailRowDone
	add	di, cx
	mov	cl, [si]
	inc	si
	cmp	ax, cx
	jg	@@rawTailFits
	mov	cx, ax
@@rawTailFits:
	mov	dx, cx
	push	dx
	xor	bx, bx
	xor	dx, dx
@@rawTailBlend:
	mov	bl, [si]
	inc	si
	push	ds
	push	si
	lds	si, mix
	mov	dl, [bx+si]
	cmp	dl, KEEP_PIXEL
	je	@@rawTailKeep
	mov	bl, es:[di]
	shl	dx, 8
	add	bx, dx
	mov	dl, [bx+si]
	xor	dh, dh
	mov	bx, dx
@@rawTailKeep:
	mov	es:[di], bl
	inc	di
	pop	si
	pop	ds
	dec	cx
	jne	@@rawTailBlend
	pop	dx
	sub	ax, dx
	jne	@@rawTailSkip
@@rawTailRowDone:
	pop	bx
	pop	dx
	pop	di
	add	di, pitch
	dec	dx
	jne	@@rawTailRow
	xor	ax, ax
	jmp	@@done

@@packClipped:
	mov	cx, clip
	cmp	cx, CLIP_HEAD
	jne	@@packNotHead
	jmp	short @@packHead
@@packNotHead:
	cmp	cx, CLIP_TAIL
	jne	@@packHead
	jmp	@@packTail

	; Compressed rows cut at the start (and maybe the end).
@@packHead:
	xor	cx, cx
@@packHeadRow:
	mov	ax, cols
	mov	si, [bx]
	add	si, bx
	add	bx, 2
	push	di
	push	dx
	push	bx
	mov	dx, skipLeft
	add	di, dx
	; Walk the hidden part; bx keeps the run byte for its fill bit.
@@packHidden:
	mov	cl, [si]
	inc	si
	sub	dx, cx
	je	@@packHeadRun
	jl	@@packInSkip
	mov	cl, [si]
	mov	bx, cx
	inc	si
	inc	si
	shr	cx, 1
	jb	@@packPassed
	dec	si
	add	si, cx
@@packPassed:
	sub	dx, cx
	je	@@packHeadSkip
	jb	@@packInRun
	jmp	short @@packHidden
@@packInRun:
	dec	si
	shr	bx, 1
	jb	@@packInFill
	inc	si
	sub	si, cx
	add	dx, cx
	sub	cx, dx
	add	si, dx
	jmp	short @@packHeadCopy
@@packInFill:
	add	dx, cx
	sub	cx, dx
	jmp	short @@packHeadFill
@@packInSkip:
	not	dx
	inc	dx
	mov	cx, dx
	jmp	short @@packHeadGap
@@packHeadSkip:
	mov	cl, [si]
	inc	si
@@packHeadGap:
	cmp	ax, cx
	jg	@@packGapFits
	mov	cx, ax
@@packGapFits:
	sub	ax, cx
	je	@@packHeadRowDone
	add	di, cx
@@packHeadRun:
	mov	cl, [si]
	inc	si
	shr	cx, 1
	jae	@@packHeadCopy
@@packHeadFill:
	cmp	ax, cx
	jg	@@packFillFits
	mov	cx, ax
@@packFillFits:
	mov	dx, cx
	push	dx
	xor	bx, bx
	xor	dx, dx
	mov	bl, [si]
	inc	si
@@packHeadFillBlend:
	push	bx
	push	ds
	push	si
	lds	si, mix
	mov	dl, [bx+si]
	cmp	dl, KEEP_PIXEL
	je	@@packHeadFillKeep
	mov	bl, es:[di]
	shl	dx, 8
	add	bx, dx
	mov	dl, [bx+si]
	xor	dh, dh
	mov	bx, dx
@@packHeadFillKeep:
	mov	es:[di], bl
	inc	di
	pop	si
	pop	ds
	pop	bx
	dec	cx
	jne	@@packHeadFillBlend
	pop	dx
	jmp	short @@packHeadNext
@@packHeadCopy:
	cmp	ax, cx
	jg	@@packCopyFits
	mov	cx, ax
@@packCopyFits:
	mov	dx, cx
	push	dx
	xor	bx, bx
	xor	dx, dx
@@packHeadCopyBlend:
	mov	bl, [si]
	inc	si
	push	ds
	push	si
	lds	si, mix
	mov	dl, [bx+si]
	cmp	dl, KEEP_PIXEL
	je	@@packHeadCopyKeep
	mov	bl, es:[di]
	shl	dx, 8
	add	bx, dx
	mov	dl, [bx+si]
	xor	dh, dh
	mov	bx, dx
@@packHeadCopyKeep:
	mov	es:[di], bl
	inc	di
	pop	si
	pop	ds
	dec	cx
	jne	@@packHeadCopyBlend
	pop	dx
@@packHeadNext:
	sub	ax, dx
	jne	@@packHeadSkip
@@packHeadRowDone:
	pop	bx
	pop	dx
	pop	di
	add	di, pitch
	dec	dx
	jne	@@packHeadRow
	xor	ax, ax
	jmp	@@done

	; Compressed rows cut at the end only.
@@packTail:
	xor	cx, cx
@@packTailRow:
	mov	ax, cols
	mov	si, [bx]
	add	si, bx
	add	bx, 2
	push	di
	push	dx
	push	bx
@@packTailSkip:
	mov	cl, [si]
	inc	si
	sub	ax, cx
	jle	@@packTailRowDone
	add	di, cx
	mov	cl, [si]
	inc	si
	shr	cx, 1
	jae	@@packTailCopy
	cmp	ax, cx
	jg	@@packTailFillFits
	mov	cx, ax
@@packTailFillFits:
	mov	dx, cx
	push	dx
	xor	bx, bx
	xor	dx, dx
	mov	bl, [si]
	inc	si
@@packTailFillBlend:
	push	bx
	push	ds
	push	si
	lds	si, mix
	mov	dl, [bx+si]
	cmp	dl, KEEP_PIXEL
	je	@@packTailFillKeep
	mov	bl, es:[di]
	shl	dx, 8
	add	bx, dx
	mov	dl, [bx+si]
	xor	dh, dh
	mov	bx, dx
@@packTailFillKeep:
	mov	es:[di], bl
	inc	di
	pop	si
	pop	ds
	pop	bx
	dec	cx
	jne	@@packTailFillBlend
	pop	dx
	jmp	short @@packTailNext
@@packTailCopy:
	cmp	ax, cx
	jg	@@packTailCopyFits
	mov	cx, ax
@@packTailCopyFits:
	mov	dx, cx
	push	dx
	xor	bx, bx
	xor	dx, dx
@@packTailCopyBlend:
	mov	bl, [si]
	inc	si
	push	ds
	push	si
	lds	si, mix
	mov	dl, [bx+si]
	cmp	dl, KEEP_PIXEL
	je	@@packTailCopyKeep
	mov	bl, es:[di]
	shl	dx, 8
	add	bx, dx
	mov	dl, [bx+si]
	xor	dh, dh
	mov	bx, dx
@@packTailCopyKeep:
	mov	es:[di], bl
	inc	di
	pop	si
	pop	ds
	dec	cx
	jne	@@packTailCopyBlend
	pop	dx
@@packTailNext:
	sub	ax, dx
	jne	@@packTailSkip
@@packTailRowDone:
	pop	bx
	pop	dx
	pop	di
	add	di, pitch
	dec	dx
	jne	@@packTailRow
	xor	ax, ax
	jmp	short @@done

@@done:
	popf
	ret
SkipDoubleTransformDraw	ENDP

	END
