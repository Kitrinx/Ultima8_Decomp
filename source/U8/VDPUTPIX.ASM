; name: VDPUTPIX
; flags: /ml /m5 /zd
; Plots one pixel in a viewport.

	.MODEL	LARGE, C
	.186
	LOCALS
	INCLUDE	VPORT.INC

	.CODE

; int PutPixel(Vport far *vp, int x, int y, unsigned char color)
; Points outside the clip rectangle are skipped and return -1.
	PUBLIC	PutPixel
PutPixel	PROC	C FAR USES ds es di, vp:DWORD, x:WORD, y:WORD, color:BYTE
	lds	bx, vp
	mov	ax, -1
	mov	dx, y
	cmp	dx, [bx].vpY2
	jg	@@done
	cmp	dx, [bx].vpY1
	jl	@@done
	mov	cx, x
	cmp	cx, [bx].vpX1
	jl	@@done
	cmp	cx, [bx].vpX2
	jg	@@done
	les	di, [bx].vpYTable
	shl	dx, 1
	add	di, dx
	mov	di, es:[di]
	add	di, cx
	push	[bx].vpSeg
	pop	ds
	mov	al, color
	mov	[di], al
@@done:
	ret
PutPixel	ENDP

	END
