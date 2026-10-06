; name: GETRGB
; flags: /ml /m5 /zd
; Reads VGA palette entries back from the DAC.

	.MODEL	LARGE
	LOCALS

DAC_READ_INDEX	EQU	3C7h
DAC_DATA	EQU	3C9h

; A short jump to the @@next instruction, to give the card time between port accesses.
IO_DELAY	MACRO
	jmp	$+2
	ENDM

	.CODE

; void pascal GETCOLORRGB(int index, unsigned char far *rgb)
	PUBLIC	GETCOLORRGB
GETCOLORRGB	PROC	PASCAL FAR USES di, index:WORD, rgb:DWORD
	pushf
	cld
	push	ds
	pop	es
	mov	dx, DAC_READ_INDEX
	les	di, rgb
	or	di, di
	je	@@done
	mov	ax, index
	out	dx, al
	IO_DELAY
	mov	dx, DAC_DATA
	xor	ah, ah
	in	al, dx			; red
	IO_DELAY
	stosb
	in	al, dx			; green
	IO_DELAY
	stosb
	in	al, dx			; blue
	IO_DELAY
	stosb
@@done:
	popf
	ret
GETCOLORRGB	ENDP

; void pascal GETCOLORBANKRGB(int first, int count, unsigned char far *rgb)
; Reads count entries from first on, three bytes each.
	PUBLIC	GETCOLORBANKRGB
GETCOLORBANKRGB	PROC	PASCAL FAR USES di, first:WORD, count:WORD, rgb:DWORD
	pushf
	cld
	push	ds
	pop	es
	les	di, rgb
	or	di, di
	je	@@done
	mov	cx, count
	mov	bx, first
@@next:
	mov	dx, DAC_READ_INDEX
	mov	al, bl
	out	dx, al
	IO_DELAY
	mov	dx, DAC_DATA
	xor	ah, ah
	in	al, dx
	IO_DELAY
	stosb
	in	al, dx
	IO_DELAY
	stosb
	in	al, dx
	IO_DELAY
	stosb
	inc	bl
	loop	@@next
@@done:
	popf
	ret
GETCOLORBANKRGB	ENDP

	END
