; name: SETRGB
; flags: /ml /m5 /zd
; Writes VGA palette entries to the DAC.

	.MODEL	LARGE
	LOCALS

DAC_WRITE_INDEX	EQU	3C8h		; the data port follows it

; A short jump to the next instruction, to give the card time between port accesses.
IO_DELAY	MACRO
	jmp	$+2
	ENDM

	.CODE

; void pascal SETCOLORRGB(int index, unsigned char far *rgb)
	PUBLIC	SETCOLORRGB
SETCOLORRGB	PROC	PASCAL FAR USES si ds, index:WORD, rgb:DWORD
	mov	dx, DAC_WRITE_INDEX
	lds	si, rgb
	mov	ax, index
	out	dx, al
	IO_DELAY
	inc	dx
	lodsb				; red
	out	dx, al
	IO_DELAY
	lodsb				; green
	out	dx, al
	IO_DELAY
	lodsb				; blue
	out	dx, al
	IO_DELAY
	ret
SETCOLORRGB	ENDP

; void pascal SETCOLORBANKRGB(int first, int count, unsigned char far *rgb)
; Writes count entries from first on, three bytes each.
	PUBLIC	SETCOLORBANKRGB
SETCOLORBANKRGB	PROC	PASCAL FAR USES si ds, first:WORD, count:WORD, rgb:DWORD
	mov	dx, DAC_WRITE_INDEX
	lds	si, rgb
	mov	cx, count
	mov	bx, first
@@next:
	mov	al, bl
	out	dx, al
	IO_DELAY
	inc	dx
	lodsb
	out	dx, al
	IO_DELAY
	lodsb
	out	dx, al
	IO_DELAY
	lodsb
	out	dx, al
	IO_DELAY
	dec	dx
	inc	bl
	loop	@@next
	ret
SETCOLORBANKRGB	ENDP

	END
