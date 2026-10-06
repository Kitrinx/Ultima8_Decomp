; name: SETREGS
; flags: /ml /m5 /o /zd
; Ring 0 routines, reached through call gates, that switch the IDT register between the
; original interrupt table and the intercepting one.

	.MODEL	LARGE
	.386P
	LOCALS

	.DATA

	EXTRN	@Spanky@intercepting:BYTE
	EXTRN	@Spanky@originalIdtAddress:DWORD
	EXTRN	@Spanky@newIdtAddress:DWORD
	EXTRN	@Spanky@setRegsCodeSeg:WORD
	EXTRN	@Spanky@setRegsOrigCodeOff:WORD
	EXTRN	@Spanky@setRegsNewCodeOff:WORD

	.CODE

; 2000 zero bytes ahead of the code; nothing is known to use them.
Reserved	db	2000 dup (0)

; void getSetRegsCodeInfo(void)
; Tells Spanky where the two routines below are, so it can build call gates to them.
	PUBLIC	_getSetRegsCodeInfo
_getSetRegsCodeInfo	PROC	FAR
	mov	@Spanky@setRegsCodeSeg, cs
	mov	@Spanky@setRegsOrigCodeOff, OFFSET SetOriginalIdt
	mov	@Spanky@setRegsNewCodeOff, OFFSET SetInterceptIdt
	ret
_getSetRegsCodeInfo	ENDP

; Each routine reads the IDT register, swaps in a new table base and keeps the limit.
SetOriginalIdt	PROC	C FAR USES eax di es
	LOCAL	idtr:FWORD
	pushf
	cli
	mov	@Spanky@intercepting, 0
	push	ss
	pop	es
	lea	di, idtr
	sidt	es:[di]
	mov	eax, @Spanky@originalIdtAddress
	mov	es:[di+2], eax
	lidt	es:[di]
	popf
	ret
SetOriginalIdt	ENDP

SetInterceptIdt	PROC	C FAR USES eax di es
	LOCAL	idtr:FWORD
	pushf
	cli
	mov	@Spanky@intercepting, 1
	push	ss
	pop	es
	lea	di, idtr
	sidt	es:[di]
	mov	eax, @Spanky@newIdtAddress
	mov	es:[di+2], eax
	lidt	es:[di]
	popf
	ret
SetInterceptIdt	ENDP

	END
