; name: SCALE
; flags: /ml
; data: 2B52h
; Hand-written shrinking draw of a shape frame into a viewport rectangle.
; extern "C" void scale(Vport *dest, int x1, int y1, int x2, int y2, void *frame)

	.MODEL	LARGE
	.386

; Vport layout
VP_SEG		EQU	0	; selector of the pixel memory
VP_X1		EQU	7	; clip rectangle, inclusive
VP_Y1		EQU	9
VP_X2		EQU	0Bh
VP_Y2		EQU	0Dh

; Arguments
dest		EQU	dword ptr [bp + 6]
x1		EQU	word ptr [bp + 0Ah]
y1		EQU	word ptr [bp + 0Ch]
x2		EQU	word ptr [bp + 0Eh]
y2		EQU	word ptr [bp + 10h]
frame		EQU	dword ptr [bp + 12h]

; Locals
srcW		EQU	word ptr [bp - 2]	; frame size
srcH		EQU	word ptr [bp - 4]
dstW		EQU	word ptr [bp - 6]	; target size
dstH		EQU	word ptr [bp - 8]
xStep		EQU	word ptr [bp - 0Ah]	; source pixels per target pixel
yStep		EQU	word ptr [bp - 0Ch]
xExtra		EQU	word ptr [bp - 0Eh]	; remainders of those divisions
yExtra		EQU	word ptr [bp - 10h]
xCount		EQU	word ptr [bp - 12h]
xExtraLeft	EQU	word ptr [bp - 14h]
yCount		EQU	word ptr [bp - 16h]
clipL		EQU	word ptr [bp - 18h]	; target pixels clipped off each side
clipT		EQU	word ptr [bp - 1Ah]
visW		EQU	word ptr [bp - 1Ch]
visH		EQU	word ptr [bp - 1Eh]
skipL		EQU	word ptr [bp - 20h]
leftW		EQU	word ptr [bp - 22h]
rowStart	EQU	word ptr [bp - 24h]
pitch		EQU	word ptr [bp - 26h]
compressed	EQU	byte ptr [bp - 28h]

	.DATA

; One decoded line of the frame.
lineBuffer	db	324 dup (0)

	.CODE

	PUBLIC	_scale
_scale	PROC	FAR
	enter	28h, 0
	push	di
	push	si
	push	ds
	push	es
	cld
	lds	si, frame
	les	di, dest
	mov	ax, word ptr es:[di + VP_X1]
	cmp	x2, ax
	jl	L_025F
	mov	ax, word ptr es:[di + VP_X2]
	cmp	x1, ax
	jg	L_025F
	mov	ax, word ptr es:[di + VP_Y1]
	cmp	y2, ax
	jl	L_025F
	mov	ax, word ptr es:[di + VP_Y2]
	cmp	y1, ax
	jg	L_025F
	mov	ax, word ptr es:[di + VP_X2]
	sub	ax, word ptr es:[di + VP_X1]
	inc	ax
	mov	pitch, ax
	mov	clipL, 0
	mov	clipT, 0
	mov	ax, x2
	sub	ax, x1
	inc	ax
	mov	visW, ax
	mov	ax, y2
	sub	ax, y1
	inc	ax
	mov	visH, ax
	mov	ax, word ptr es:[di + VP_X1]
	cmp	x1, ax
	jg	L_0079
	sub	ax, x1
	mov	clipL, ax
	sub	visW, ax
L_0079:
	mov	ax, x2
	cmp	word ptr es:[di + VP_X2], ax
	jg	L_008B
	sub	ax, word ptr es:[di + VP_X2]
	sub	visW, ax
L_008B:
	mov	ax, word ptr es:[di + VP_Y1]
	cmp	y1, ax
	jg	L_009F
	sub	ax, y1
	add	clipT, ax
	sub	visH, ax
L_009F:
	mov	ax, y2
	cmp	word ptr es:[di + VP_Y2], ax
	jg	L_00B1
	sub	ax, word ptr es:[di + VP_Y2]
	sub	visH, ax
L_00B1:
	mov	al, byte ptr [si]
	mov	compressed, al
	mov	ax, word ptr [si + 2]
	mov	srcW, ax
	mov	ax, word ptr [si + 4]
	mov	srcH, ax
	mov	ax, x2
	sub	ax, x1
	inc	ax
	mov	dstW, ax
	mov	ax, y2
	sub	ax, y1
	inc	ax
	mov	dstH, ax
	mov	ax, srcW
	xor	dx, dx
	div	dstW
	cmp	ax, 0
	jne	L_00F8
	mov	xStep, 1
	mov	xExtra, 0
	mov	ax, word ptr [si + 2]
	mov	dstW, ax
	jmp	L_00FE
L_00F8:
	mov	xStep, ax
	mov	xExtra, dx
L_00FE:
	mov	ax, srcH
	xor	dx, dx
	div	dstH
	cmp	ax, 0
	jne	L_0120
	mov	yStep, 1
	mov	yExtra, 0
	mov	ax, word ptr [si + 4]
	mov	dstH, ax
	jmp	L_0126
L_0120:
	mov	yStep, ax
	mov	yExtra, dx
L_0126:
	mov	ax, word ptr es:[di + VP_SEG]
	mov	es, ax
	mov	dx, pitch
	mov	ax, y1
	mul	dx
	mov	di, ax
	add	di, x1
	add	si, 0Ah
L_013B:
	mov	ax, yStep
	mov	yCount, ax
	mov	ax, clipL
	mov	skipL, ax
	mov	ax, visW
	mov	leftW, ax
	mov	rowStart, di
L_0150:
	dec	yCount
	je	L_015C
	add	si, 2
	jmp	L_0150
L_015C:
	cmp	yExtra, 0
	je	L_016A
	dec	yExtra
	add	si, 2
L_016A:
	cmp	clipT, 0
	je	L_017A
	dec	clipT
	push	si
	push	ds
	jmp	L_024D
L_017A:
	cmp	visH, 0
	je	L_025F
	dec	visH
	push	si
	push	es
	push	di
	add	si, word ptr [si]
	mov	ax, DGROUP
	mov	es, ax
	mov	di, offset lineBuffer
	mov	dx, di
	add	dx, srcW
	mov	cx, srcW
	shr	cx, 1
	mov	ax, 0FFFFh
	inc	cx
	push	di
	rep stosw
	pop	di
L_01A4:
	movzx	cx, byte ptr [si]
	inc	si
	add	di, cx
	cmp	di, dx
	jl	L_01B3
	jmp	L_01D3
L_01B3:
	movzx	cx, byte ptr [si]
	inc	si
	cmp	compressed, 1
	jne	L_01CD
	rcr	cx, 1
	jae	L_01CD
	mov	al, byte ptr [si]
	inc	si
	rep stosb
	jmp	L_01CF
L_01CD:
	rep movsb
L_01CF:
	cmp	di, dx
	jl	L_01A4
L_01D3:
	pop	di
	pop	es
	push	ds
	mov	ax, DGROUP
	mov	ds, ax
	mov	si, offset lineBuffer
	mov	cx, dstW
	mov	bx, xExtra
	mov	xExtraLeft, bx
L_01E7:
	mov	bx, xStep
	mov	xCount, bx
L_01ED:
	lodsb
	dec	xCount
	jne	L_01ED
	cmp	xExtraLeft, 0
	je	L_01FF
	dec	xExtraLeft
	lodsb
L_01FF:
	cmp	al, 0FFh
	je	L_0223
	cmp	skipL, 0
	je	L_0214
	dec	skipL
	inc	di
	jmp	L_021C
L_0214:
	stosb
	dec	leftW
	je	L_024D
L_021C:
	dec	cx
	je	L_024D
	jmp	L_01E7
L_0223:
	cmp	skipL, 0
	je	L_0236
	dec	skipL
	inc	di
	dec	cx
	je	L_024D
	jmp	L_01E7
L_0236:
	inc	di
	dec	cx
	je	L_024D
	cmp	leftW, 0
	je	L_024B
	dec	leftW
	je	L_024D
L_024B:
	jmp	L_01E7
L_024D:
	pop	ds
	pop	si
	add	si, 2
	mov	di, rowStart
	add	di, pitch
	dec	dstH
	jne	L_013B
L_025F:
	pop	es
	pop	ds
	pop	si
	pop	di
	leave
	retf
_scale	ENDP

	END
