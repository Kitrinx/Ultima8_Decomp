; name: VIEWFILE
; Hand-written line splitter of ViewFile.

	.MODEL	LARGE
	.386
	LOCALS

MAX_LINES	EQU	2000

	.CODE

; int ViewFile::parse(unsigned long *offsets, char *text, unsigned long length, unsigned long base)
; Ends each line of text with a NUL in place of the CR before its LF and stores base plus the line's start for
; every line. Scans from the start of text's segment. Returns the number of lines.
	PUBLIC	@ViewFile@parse$qnulnzculul
@ViewFile@parse$qnulnzculul	PROC	CPP FAR USES ds, file:DWORD, offsets:DWORD, text:DWORD, len:DWORD, base:DWORD
	lds	bx, offsets
	mov	ax, word ptr text + 2
	mov	es, ax
	xor	edi, edi
	xor	dx, dx
	mov	ecx, len
	mov	al, 0Ah
@@line:
	mov	esi, base
	add	esi, edi
	mov	[bx], esi
	add	bx, 4
	repne	scas byte ptr es:[edi]
	dec	edi
	dec	edi
	mov	byte ptr es:[edi], 0
	inc	edi
	inc	edi
	inc	dx
	cmp	dx, MAX_LINES
	je	@@done
	or	ecx, ecx
	jne	@@line
@@done:
	mov	ax, dx
	ret
@ViewFile@parse$qnulnzculul	ENDP

	END
