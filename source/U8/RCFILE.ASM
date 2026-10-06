; name: RCFILE
; flags: /ml /m5 /zd
; Creates a file, or truncates an existing one.

	.MODEL	LARGE, C
	LOCALS

DOS_CREATE	EQU	3Ch

	.DATA

	EXTRN	fileerr:WORD

	.CODE

; int SafeCreateFile(const char far *path, int attribs)
; Returns the handle, or -1 with carry set.
	PUBLIC	SafeCreateFile
SafeCreateFile	PROC	C FAR USES si di, path:DWORD, attribs:WORD
	xor	ax, ax
	mov	fileerr, ax
	push	ds
	lds	dx, path
	xor	cx, cx
	mov	cl, byte ptr attribs
	xor	ax, ax
	mov	ah, DOS_CREATE
	int	21h
	pop	ds
	jnc	@@done
	mov	fileerr, ax
	mov	ax, -1
	stc
@@done:
	ret
SafeCreateFile	ENDP

	END
