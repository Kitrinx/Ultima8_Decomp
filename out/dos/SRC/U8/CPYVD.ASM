; name: CPYVD
; flags: /ml /m5 /zd
; Whole-screen fills and copies for a 320x200 byte-per-pixel screen.

	.MODEL	LARGE, C
	.386
	LOCALS

SCREEN_BYTES	EQU	320*200

	.CODE

; void ClearScreenOnly(unsigned screen, int color)
; Fills the screen segment with the low byte of color.
	PUBLIC	ClearScreenOnly
ClearScreenOnly	PROC	C FAR USES di es, screen:WORD, color:WORD
	pushf
	cld
	mov	es, screen
	xor	di, di
	mov	ax, color
	mov	ah, al
	mov	ecx, SCREEN_BYTES / 2
	rep	stosw
	popf
	ret
ClearScreenOnly	ENDP

; void SlamScreen(unsigned screen, void far *image)
; Copies a full screen image into the screen segment.
	PUBLIC	SlamScreen
SlamScreen	PROC	C FAR USES di si ds es, screen:WORD, image:DWORD
	pushf
	cld
	lds	si, image
	mov	es, screen
	xor	di, di
	mov	ecx, SCREEN_BYTES / 4
	rep	movsd
	popf
	ret
SlamScreen	ENDP

; void SlamWordScreen(unsigned screen, void far *image)
; SlamScreen a word at a time.
	PUBLIC	SlamWordScreen
SlamWordScreen	PROC	C FAR USES di si ds es, screen:WORD, image:DWORD
	pushf
	cld
	lds	si, image
	mov	es, screen
	xor	di, di
	mov	ecx, SCREEN_BYTES / 2
	rep	movsw
	popf
	ret
SlamWordScreen	ENDP

; void SlamClearScreenOnly(void)
; Does nothing. Without args the frame has to be spelled out.
	PUBLIC	SlamClearScreenOnly
SlamClearScreenOnly	PROC	C FAR
	enter	0, 0
	push	di
	push	es
	pushf
	cld
	popf
	pop	es
	pop	di
	leave
	ret
SlamClearScreenOnly	ENDP

; void TransformScreen(unsigned screen, unsigned char far *table)
; Maps every pixel through a 256-byte colour table, in place.
	PUBLIC	TransformScreen
TransformScreen	PROC	C FAR USES di si es ds, screen:WORD, table:DWORD
	pushf
	cld
	lds	bx, table
	mov	es, screen
	xor	di, di
	mov	si, si			; never zeroed: reads start wherever si was
	mov	ecx, SCREEN_BYTES
	xor	eax, eax
@@next:
	mov	al, es:[si]
	inc	si
	xlat
	stosb
	dec	cx
	jne	@@next
	popf
	ret
TransformScreen	ENDP

; void ClearAnyScreenOnly(unsigned screen, unsigned char color)
	PUBLIC	ClearAnyScreenOnly
ClearAnyScreenOnly	PROC	C FAR USES di es, screen:WORD, color:BYTE
	pushf
	cld
	mov	es, screen
	xor	di, di
	mov	al, color
	mov	ah, al
	mov	ecx, SCREEN_BYTES / 2
	rep	stosw
	popf
	ret
ClearAnyScreenOnly	ENDP

	END
