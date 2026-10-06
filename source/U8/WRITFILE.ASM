; name: WRITFILE
; flags: /ml /m5 /zd
; Writes a block of any size to a file, a segment's worth at a time.

	.MODEL	LARGE, C
	LOCALS

DOS_SEEK_FROM_START	EQU	4200h
DOS_WRITE	EQU	40h
CHUNK	EQU	0FFF0h			; the most one write takes, so it stays in one segment
SHORT_WRITE	EQU	7Fh			; fileerr when DOS wrote less than asked: the disk is full

	.DATA

	EXTRN	fileerr:WORD

	.CODE

; char WriteFile(int handle, long pos, long count, void far *buffer)
; A pos of -1 writes at the current position. Always returns 1; only fileerr tells of an error.
	PUBLIC	WriteFile
WriteFile	PROC	C FAR USES si di bx cx dx, handle:WORD, pos:DWORD, count:DWORD, buffer:DWORD
	LOCAL	src:DWORD
	xor	ax, ax
	mov	fileerr, ax
	cmp	word ptr buffer+2, ax
	je	@@failed			; NULL buffer, fileerr stays 0
	mov	dx, word ptr buffer+2
	mov	ax, word ptr buffer
	mov	word ptr src+2, dx
	mov	word ptr src, ax
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
	lds	dx, src
	mov	ah, DOS_WRITE
	int	21h
	pop	ds
	jc	@@failed
	cmp	ax, cx
	jb	@@diskFull
	sub	si, ax
	sbb	di, 0
	add	word ptr src, ax
	jmp	short @@nextChunk
@@lastChunk:
	mov	cx, si
	push	ds
	lds	dx, src
	mov	ah, DOS_WRITE
	int	21h
	pop	ds
	jc	@@failed
	cmp	ax, cx
	jae	@@written
@@diskFull:
	mov	ax, SHORT_WRITE
@@failed:
	mov	fileerr, ax
	jmp	short @@written
	xor	ax, ax				; never reached
	jmp	short @@done
@@written:
	mov	ax, 1
@@done:
	ret
WriteFile	ENDP

	END
