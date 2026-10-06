; name: FILEUTIL
; flags: /ml /m5 /zd
; Reads a file's DOS attributes; owns fileerr, the last DOS error of the file routines.

	.MODEL	LARGE, C
	LOCALS

DOS_ATTRIBUTES	EQU	43h		; AL = 0 gets them into CX

	.DATA

	PUBLIC	fileerr
fileerr	dw	0

	.CODE

; int file_attribs(const char far *path)
; Returns the attributes, or the DOS error with bit 15 set.
	PUBLIC	file_attribs
file_attribs	PROC	C FAR, path:DWORD
	push	ds
	xor	ax, ax
	mov	fileerr, ax
	lds	dx, path
	mov	ah, DOS_ATTRIBUTES
	mov	al, 0
	int	21h
	pop	ds
	jnc	@@found
	mov	fileerr, ax
	or	ax, 8000h
	jmp	short @@done
@@found:
	mov	ax, cx
@@done:
	ret
file_attribs	ENDP

; unsigned char file_exists(const char far *path)
	PUBLIC	file_exists
file_exists	PROC	C FAR, path:DWORD
	push	ds
	xor	ax, ax
	mov	fileerr, ax
	lds	dx, path
	mov	ah, DOS_ATTRIBUTES
	mov	al, 0
	int	21h
	pop	ds
	jnc	@@found
	mov	fileerr, ax
	xor	ax, ax
	jmp	short @@done
@@found:
	mov	ax, 1
@@done:
	ret
file_exists	ENDP

	END
