; name: OPENFILE
; flags: /ml /m5 /zd
; Opens a file for reading and writing, and closes a file handle.

	.MODEL	LARGE, C
	LOCALS

DOS_OPEN_READ_WRITE	EQU	3D02h
DOS_CLOSE	EQU	3Eh

	.DATA

	EXTRN	fileerr:WORD

	.CODE

; int OpenFile(const char far *path)
; Returns the handle, or -1 with carry set.
	PUBLIC	OpenFile
OpenFile	PROC	C FAR, path:DWORD
	mov	fileerr, 0
	push	ds
	lds	dx, path
	mov	ax, DOS_OPEN_READ_WRITE
	int	21h
	pop	ds
	jnc	@@done
	mov	fileerr, ax
	mov	ax, -1
	stc
@@done:
	ret
	retf				; never reached
OpenFile	ENDP

; int CloseFile(int handle)
	PUBLIC	CloseFile
CloseFile	PROC	C FAR, handle:WORD
	mov	bx, handle
	xor	ax, ax
	mov	fileerr, ax
	mov	ah, DOS_CLOSE
	int	21h
	jnc	@@done
	mov	fileerr, ax
@@done:
	ret
CloseFile	ENDP

	END
