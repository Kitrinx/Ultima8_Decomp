; name: DCMP8
; Speech Compression's 1993 DCMP8 speech decoder, linked into U8.EXE as a
; finished object. Disassembled from the shipped code.

DCMP8_TEXT	SEGMENT	WORD PUBLIC 'CODE'
	ASSUME	CS:DCMP8_TEXT

apply_filter	PROC	NEAR
	push	bp
	mov	bp, sp
	mov	ax, word ptr [bp + 4]
	shl	ax, 1
	add	word ptr [bp + 0Ch], ax
L_000B:
	push	bp
	lds	si, dword ptr [bp + 8]
	inc	word ptr [bp + 8]
	les	di, dword ptr [bp + 0Ch]
	mov	cx, word ptr [bp + 4]
	xor	bx, bx
	xor	bp, bp
L_001C:
	lodsb
	xor	al, 80h
	cbw
	imul	word ptr es:[di]
	sub	di, 2
	add	bx, ax
	adc	bp, dx
	loop	L_001C
	mov	dx, bp
	pop	bp
	mov	ax, bx
	mov	cl, 4
	shl	dx, cl
	shr	ax, cl
	or	dl, ah
	add	al, 80h
	adc	dx, 0
	sub	byte ptr [si], dl
	dec	word ptr [bp + 6]
	jne	L_000B
	pop	bp
	ret	0Ch
apply_filter	ENDP

; Trailing zero bits of each byte value
trailing_zeros	LABEL	BYTE
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4
	db	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 8
width_table	LABEL	BYTE
	db	1, 2, 3, 4, 5, 6, 7, 7, 7, 1, 1, 2, 3, 4, 5, 6
	db	7, 7, 0, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 1, 2, 3, 4, 5
	db	6, 6, 6, 6, 2, 2, 3, 4, 5, 6, 7, 7, 7, 0, 0FFh, 0FFh
	db	0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 1, 2, 3, 4, 5, 5, 5, 5, 5, 3
	db	3, 4, 5, 6, 7, 7, 7, 7, 0, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh
	db	0FFh, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 7, 7
	db	7, 7, 7, 0, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 1, 2, 3, 3
	db	3, 3, 3, 3, 3, 5, 5, 6, 7, 7, 7, 7, 7, 7, 0, 0FFh
	db	0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 1, 2, 2, 2, 2, 2, 2, 2, 2
	db	6, 6, 7, 7, 7, 7, 7, 7, 7, 0, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh
	db	0FFh, 0FFh, 1, 1, 1, 1, 1, 1, 1, 1, 1, 7, 7, 7, 7, 7
	db	7, 7, 7, 7, 0, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0, 2, 3
	db	4, 5, 6, 7, 8, 8, 1, 1, 1, 2, 3, 4, 5, 6, 7, 0
	db	0, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0, 2, 3, 4, 5, 6, 7, 7
	db	7, 1, 2, 2, 3, 4, 5, 6, 7, 7, 0, 0, 0FFh, 0FFh, 0FFh, 0FFh
	db	0FFh, 0FFh, 0FFh, 0, 2, 3, 4, 5, 6, 6, 6, 6, 1, 3, 3, 4
	db	5, 6, 7, 7, 7, 0, 0, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0, 2
	db	3, 4, 5, 5, 5, 5, 5, 1, 4, 4, 5, 6, 7, 7, 7, 7
	db	0, 0, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0, 2, 3, 4, 4, 4, 4
	db	4, 4, 1, 5, 5, 6, 7, 7, 7, 7, 7, 0, 0, 0FFh, 0FFh, 0FFh
	db	0FFh, 0FFh, 0FFh, 0FFh, 0, 2, 3, 3, 3, 3, 3, 3, 3, 1, 6, 6
	db	7, 7, 7, 7, 7, 7, 0, 0, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0
	db	2, 2, 2, 2, 2, 2, 2, 2, 1, 7, 7, 7, 7, 7, 7, 7
	db	7, 0, 0, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh

unpack_samples	PROC	NEAR
	push	bp
	mov	bp, sp
	mov	si, word ptr [bp + 4]
	mov	ax, word ptr [bp + 6]
	lds	di, dword ptr [bp + 8]
	les	bp, dword ptr [bp + 0Ch]
	mov	ch, 8
	sub	si, 8
	mov	dx, si
	shl	dx, 1
	shl	dx, 1
	shl	dx, 1
	add	si, dx
	mov	dx, si
	shl	dx, 1
	add	si, dx
	lea	si, [si + width_table]
	xor	bx, bx
	mov	dx, word ptr [di]
	add	di, 2
L_02F2:
	push	ax
	mov	bl, dl
	mov	bl, byte ptr cs:[bx + trailing_zeros]
	mov	cl, byte ptr cs:[bx + si]
	cmp	ch, cl
	jae	L_030E
	xchg	ch, cl
	shr	dx, cl
	sub	ch, cl
	mov	dh, byte ptr [di]
	inc	di
	mov	cl, ch
	mov	ch, 8
L_030E:
	shr	dx, cl
	sub	ch, cl
	mov	cl, byte ptr cs:[bx + si + 9]
	mov	al, dl
	ror	al, cl
	cbw
	xor	ah, byte ptr cs:[bx + si + 12h]
	shl	ax, cl
	xor	ah, 80h
	mov	byte ptr es:[bp], ah
	inc	bp
	cmp	ch, cl
	jae	L_033A
	xchg	ch, cl
	shr	dx, cl
	sub	ch, cl
	mov	dh, byte ptr [di]
	inc	di
	mov	cl, ch
	mov	ch, 8
L_033A:
	shr	dx, cl
	sub	ch, cl
	pop	ax
	dec	ax
	jne	L_02F2
	pop	bp
	ret	0Ch
unpack_samples	ENDP

	db	'Copyright 1993 Speech Compression; all rights reserved'

	PUBLIC	DCMP8
DCMP8	PROC	FAR
	push	si
	push	di
	push	bp
	mov	bp, sp
	push	ds
	cld
	lds	si, dword ptr [bp + 0Eh]
	mov	di, si
	mov	cx, word ptr [si]
	shr	cx, 1
	xor	bx, bx
L_038E:
	lodsw
	xor	bx, ax
	loop	L_038E
	cmp	bx, 0ACEDh
	je	L_039F
	mov	ax, 0FFFFh
	jmp	short L_03E5
	db	90h
L_039F:
	xor	ax, ax
	mov	al, byte ptr [di + 7]
	push	ax
	shl	ax, 1
	add	ax, 8
	push	ax
	les	si, dword ptr [bp + 0Ah]
	push	es
	add	si, word ptr [bp - 4]
	push	si
	push	ds
	mov	si, di
	add	di, word ptr [bp - 6]
	push	di
	push	word ptr [si + 2]
	xor	ax, ax
	mov	al, byte ptr [si + 6]
	push	ax
	call	near ptr unpack_samples
	cmp	word ptr [bp - 4], 0
	je	L_03E3
	les	di, dword ptr [bp + 0Eh]
	lea	di, [di + 6]
	push	es
	push	di
	lds	si, dword ptr [bp + 0Ah]
	push	ds
	push	si
	push	word ptr es:[di - 4]
	push	word ptr [bp - 4]
	call	near ptr apply_filter
L_03E3:
	xor	ax, ax
L_03E5:
	mov	ds, word ptr [bp - 2]
	mov	sp, bp
	pop	bp
	pop	di
	pop	si
	retf	8
DCMP8	ENDP

DCMP8_TEXT	ENDS

	END
