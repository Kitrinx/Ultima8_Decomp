; name: GETREGS
; flags: /ml /m5 /zd
; Reads the CPU's descriptor table registers.

	.MODEL	LARGE, C
	.286P

	.CODE

; void getRegisters(unsigned *gdtr, unsigned *ldtr, unsigned *idtr)
; gdtr and idtr each get limit and base, three words; ldtr gets the selector.
	PUBLIC	getRegisters
getRegisters	PROC	C FAR USES di si es, gdtr:DWORD, ldtr:DWORD, idtr:DWORD
	les	di, gdtr
	sgdt	es:[di]
	les	di, ldtr
	sldt	word ptr es:[di]
	les	di, idtr
	sidt	es:[di]
	ret
getRegisters	ENDP

	END
