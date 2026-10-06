; name: SEEKFILE
; flags: /ml /m5 /zd
; Moves a file's read/write position.

	.MODEL	LARGE, C
	LOCALS

DOS_SEEK	EQU	42h		; AL: 0 from start, 1 from current, 2 from end

	.DATA

	EXTRN	fileerr:WORD

	.CODE

; long SeekFile(int handle, long pos, int from)
; Returns the new position, or -1 with carry set.
	PUBLIC	SeekFile
SeekFile	PROC	C FAR USES si di, handle:WORD, pos:DWORD, from:WORD
	mov	fileerr, 0
	mov	bx, handle
	mov	ax, from
	mov	dx, word ptr pos
	mov	cx, word ptr pos+2
	mov	ah, DOS_SEEK
	int	21h
	jnc	@@done
	mov	fileerr, ax
	mov	ax, -1
	mov	dx, ax
	stc
@@done:
	ret
SeekFile	ENDP

	END
