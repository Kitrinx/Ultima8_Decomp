; name: SKIPDRAW
; flags: /ml /m5 /zd
; Draws a run-length shape frame into a viewport, clipped.

	.MODEL	LARGE, C
	.386
	LOCALS
	INCLUDE	VPORT.INC
	INCLUDE	SKIPDRAW.INC

	.CODE

; int SkipDraw(Vport far *vp, int x, int y, SKIPFRAME far *frame)
; x, y is where the frame's hot spot lands. Returns 1 when the frame misses
; the viewport, else 0.
	PUBLIC	SkipDraw
SkipDraw	PROC	C FAR USES di si es ds, vp:DWORD, x:WORD, y:WORD, frame:DWORD
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
@@rawSkip:
	mov	cl, [si]
	inc	si
	sub	ax, cx
	je	@@rawRowDone
	add	di, cx
	mov	cl, [si]
	mov	dx, cx
	inc	si
	shr	cl, 1
	rep movsw
	mov	cl, dl
	and	cl, 1
	rep movsb
	sub	ax, dx
	jne	@@rawSkip
@@rawRowDone:
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
	push	ax
	mov	al, [si]
	inc	si
	mov	ah, al
	shr	cl, 1
	rep stosw
	mov	cl, dl
	and	cl, 1
	rep stosb
	pop	ax
	jmp	short @@packNext
@@packCopy:
	mov	dx, cx
	shr	cl, 1
	rep movsw
	mov	cl, dl
	and	cl, 1
	rep movsb
@@packNext:
	sub	ax, dx
	jne	@@packSkip
@@packRowDone:
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
	jmp	short @@rawTail

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
	shr	cx, 1
	rep movsw
	mov	cx, dx
	and	cx, 1
	rep movsb
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
	shr	cx, 1
	rep movsw
	mov	cx, dx
	and	cx, 1
	rep movsb
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
	push	ax
	mov	al, [si]
	inc	si
	mov	ah, al
	mov	dx, cx
	shr	cx, 1
	rep stosw
	mov	cx, dx
	and	cx, 1
	rep stosb
	pop	ax
	jmp	short @@packHeadNext
@@packHeadCopy:
	cmp	ax, cx
	jg	@@packCopyFits
	mov	cx, ax
@@packCopyFits:
	mov	dx, cx
	shr	cx, 1
	rep movsw
	mov	cx, dx
	and	cx, 1
	rep movsb
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
	jmp	short @@done

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
	push	ax
	mov	al, [si]
	inc	si
	mov	ah, al
	mov	dx, cx
	shr	cx, 1
	rep stosw
	mov	cx, dx
	and	cx, 1
	rep stosb
	pop	ax
	jmp	short @@packTailNext
@@packTailCopy:
	cmp	ax, cx
	jg	@@packTailCopyFits
	mov	cx, ax
@@packTailCopyFits:
	mov	dx, cx
	shr	cx, 1
	rep movsw
	mov	cx, dx
	and	cx, 1
	rep movsb
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
SkipDraw	ENDP

	END
