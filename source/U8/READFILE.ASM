; name: READFILE
; flags: /ml /m5 /zd
; Reads a block of any size from a file, a segment's worth at a time.

	.MODEL	LARGE, C
	LOCALS

DOS_SEEK_FROM_START	EQU	4200h
DOS_READ	EQU	3Fh
CHUNK	EQU	0FFF0h			; the most one read takes, so it stays in one segment

	.DATA

	EXTRN	fileerr:WORD

	PUBLIC	_FileBytesTransferred
_FileBytesTransferred	dd	0

	.CODE

; long ReadFile(int handle, long pos, long count, void far *buffer)
; A pos of -1 reads from the current position. Returns the bytes read, fewer at the end of
; the file, or 0 with carry set on an error (fileerr 1 for a NULL buffer).
	PUBLIC	ReadFile
ReadFile	PROC	C FAR USES si di bx cx, handle:WORD, pos:DWORD, count:DWORD, buffer:DWORD
	LOCAL	dest:DWORD
	mov	fileerr, 0
	mov	ax, word ptr count
	or	ax, word ptr count+2
	cmp	ax, 0
	jne	@@start
	clc
	xor	dx, dx
	jmp	@@done
@@start:
	xor	ax, ax
	mov	word ptr _FileBytesTransferred+2, ax
	mov	word ptr _FileBytesTransferred, ax
	inc	ax
	cmp	word ptr buffer+2, 0
	jne	@@haveBuffer
	cmp	word ptr buffer, 0
	jne	@@haveBuffer
	jmp	short @@failed
@@haveBuffer:
	mov	dx, word ptr buffer+2
	mov	ax, word ptr buffer
	mov	word ptr dest+2, dx
	mov	word ptr dest, ax
	mov	bx, handle
	mov	dx, word ptr pos
	mov	cx, word ptr pos+2
	cmp	cx, -1
	je	@@positioned
	mov	ax, DOS_SEEK_FROM_START
	int	21h
	jc	@@failed
@@positioned:
	mov	di, word ptr count+2
	mov	si, word ptr count
@@nextChunk:
	or	di, di
	jne	@@fullChunk
	cmp	si, CHUNK
	jbe	@@lastChunk
@@fullChunk:
	mov	cx, CHUNK
	push	ds
	lds	dx, dest
	mov	ah, DOS_READ
	int	21h
	pop	ds
	jc	@@failed
	add	word ptr _FileBytesTransferred, ax
	adc	word ptr _FileBytesTransferred+2, 0
	cmp	ax, 0
	je	@@finish			; end of file
	sub	si, ax
	sbb	di, 0
	add	word ptr dest, ax
	jmp	short @@nextChunk
@@lastChunk:
	mov	cx, si
	push	ds
	lds	dx, dest
	mov	ah, DOS_READ
	int	21h
	pop	ds
	jnc	@@finish
@@failed:
	mov	fileerr, ax
	xor	ax, ax
	xor	dx, dx
	stc
	jmp	short @@done
@@finish:
	add	word ptr _FileBytesTransferred, ax
	adc	word ptr _FileBytesTransferred+2, 0
	mov	ax, word ptr _FileBytesTransferred
	mov	dx, word ptr _FileBytesTransferred+2
@@done:
	ret
ReadFile	ENDP

	END
