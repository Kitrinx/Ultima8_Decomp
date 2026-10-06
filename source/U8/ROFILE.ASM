; name: ROFILE
; flags: /ml /m5 /zd
; Opens a file in a given DOS access mode.

	.MODEL	LARGE, C
	LOCALS

DOS_OPEN	EQU	3Dh

	.DATA

	EXTRN	fileerr:WORD

	.CODE

; int SafeOpenFile(const char far *path, int mode)
; Returns the handle, or -1 with carry set.
	PUBLIC	SafeOpenFile
SafeOpenFile	PROC	C FAR, path:DWORD, mode:WORD
	mov	fileerr, 0
	push	ds
	lds	dx, path
	mov	ah, DOS_OPEN
	mov	al, byte ptr mode
	int	21h
	pop	ds
	jnc	@@done
	mov	fileerr, ax
	mov	ax, -1
	stc
@@done:
	ret
SafeOpenFile	ENDP

	END
