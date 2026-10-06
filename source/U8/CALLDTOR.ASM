; name: CALLDTOR
; Process::callDestructor, written in assembly: calls the virtual destructor with flag 2
; and no frame of its own, so [bp + 6] is the calling member function's this, the same
; process.

	.MODEL	LARGE
	.386

VT_DESTRUCTOR	EQU	4	; Process::~Process in the vtable

	.CODE

	PUBLIC	@Process@callDestructor$qv
@Process@callDestructor$qv	PROC	FAR
	push	2
	les	bx, dword ptr [bp + 6]
	push	dword ptr [bp + 6]
	mov	bx, es:[bx]
	call	dword ptr [bx + VT_DESTRUCTOR]
	add	sp, 6
	retf
@Process@callDestructor$qv	ENDP

	END
