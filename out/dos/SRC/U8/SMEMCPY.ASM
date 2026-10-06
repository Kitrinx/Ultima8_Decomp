; name: SMEMCPY
; flags: /ml /m5 /zd
; Copies a block between 32-bit linear addresses.

	.MODEL	LARGE, C
	.386

	.DATA

	EXTRN	NOLANGUAGE @SelectorManager@globalSel:WORD	; covers all of memory

	.CODE

; void superMemCopy(unsigned long dest, unsigned long src, unsigned long count)
	PUBLIC	superMemCopy
superMemCopy	PROC	C FAR USES ds es edi esi, dest:DWORD, src:DWORD, count:DWORD
	mov	ax, @SelectorManager@globalSel
	mov	ds, ax
	mov	es, ax
	mov	edi, dest
	mov	esi, src
	mov	eax, count
	pushf
	cld
	mov	ecx, eax
	shr	ecx, 2
	rep	movs dword ptr es:[edi], dword ptr [esi]
	mov	ecx, eax
	and	ecx, 3
	rep	movs byte ptr es:[edi], byte ptr [esi]
	popf
	ret
superMemCopy	ENDP

	END
