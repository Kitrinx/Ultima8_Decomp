; name: REDIRECT
; flags: /ml /m5 /zd
; The intercepting interrupt handlers: one entry per interrupt, all funnelled into a handler
; that samples the profiler on the timer tick, then passes the interrupt on to the original
; handler through a self-patched INT instruction.

	.MODEL	LARGE
	.386
	LOCALS

	EXTRN	@Spanky@setIdtrToIntercept$qv:FAR
	EXTRN	@Spanky@setIdtrToOriginal$qv:FAR

	.DATA

	EXTRN	@Spanky@csIpPtrArray:DWORD
	EXTRN	@Spanky@numCsIpPtrs:WORD
	EXTRN	@Spanky@csIpPtrOverflow:BYTE
	EXTRN	@Spanky@profiling:BYTE
	EXTRN	@Spanky@intJumpTable:DWORD
	EXTRN	@Spanky@patchPointer:DWORD

TIMER_INT	EQU	8
BREAKPOINT_INT	EQU	3
CSIP_SLOTS	EQU	25			; profiler samples held until the next flush
IF_FLAG	EQU	200h
INT_OPCODE	EQU	0CDh
NOP_INT3	EQU	0CC90h			; nop; int 3, as a word

; The handler's stack once it has saved everything, from SP:
;   +00h es, +02h ds, +04h pusha block, +14h high words of the 32-bit registers,
;   +22h flags on entry, +24h return address into InterruptEntries,
;   +26h interrupted CS:IP (or a CPU error code), +2Ah interrupted flags.
ENTRY_FLAGS	EQU	22h
TABLE_RETURN	EQU	24h
INT_CSIP	EQU	26h
INT_FLAGS	EQU	2Ah
; After the passed-on interrupt returns, its flags sit on top and the rest is 2 bytes further.
RESULT_FLAGS	EQU	14h

; Swaps the halves of every 32-bit register but esp.
SWAP_HALVES	MACRO
	rol	eax, 10h
	rol	ebx, 10h
	rol	ecx, 10h
	rol	edx, 10h
	rol	esi, 10h
	rol	edi, 10h
	rol	ebp, 10h
	ENDM

; pusha keeps only the low words; these keep the high ones.
PUSH_HIGH_WORDS	MACRO
	SWAP_HALVES
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	bp
	SWAP_HALVES
	ENDM

POP_HIGH_WORDS	MACRO
	SWAP_HALVES
	pop	bp
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	SWAP_HALVES
	ENDM

	.CODE

; void getIntJumpTable(void)
	PUBLIC	_getIntJumpTable
_getIntJumpTable	PROC	FAR
	mov	word ptr @Spanky@intJumpTable, OFFSET InterruptEntries
	mov	word ptr @Spanky@intJumpTable+2, cs
	ret
_getIntJumpTable	ENDP

; void getPatchPosition(void)
; Points patchPointer at the INT instruction the handler rewrites.
	PUBLIC	_getPatchPosition
_getPatchPosition	PROC	FAR
	mov	word ptr @Spanky@patchPointer, OFFSET PatchedInt
	mov	word ptr @Spanky@patchPointer+2, cs
	ret
_getPatchPosition	ENDP

; Interrupt n enters at InterruptEntries + 3*n; the call's return address tells which.
InterruptEntries	LABEL	NEAR
	REPT	256
	call	Interceptor
	ENDM

Interceptor	PROC	NEAR
	pushf
	PUSH_HIGH_WORDS
	pusha
	push	ds
	push	es
	mov	ax, DGROUP
	mov	ds, ax
	mov	bx, sp

	; A CPU exception that pushed an error code leaves CS where the flags should be.
	mov	ax, ss:[bx+ENTRY_FLAGS]
	mov	dx, ss:[bx+INT_FLAGS]
	mov	cx, NOT IF_FLAG
	and	ax, cx
	and	dx, cx
	cmp	ax, dx
	je	@@passOn

	; Drop the error code and return to the faulting instruction under the original table.
	call	@Spanky@setIdtrToOriginal$qv
	pop	es
	pop	ds
	popa
	POP_HIGH_WORDS
	add	sp, 6
	iret

@@passOn:
	mov	dx, ss:[bx+TABLE_RETURN]
	mov	cx, OFFSET InterruptEntries
	sub	dx, cx
	sub	dx, 3
	mov	ax, dx
	mov	dl, 3
	div	dl
	mov	cl, al				; interrupt number
	cmp	al, TIMER_INT
	jne	@@patch
	mov	al, @Spanky@profiling
	cmp	al, 1
	jne	@@patch

	; Record the interrupted CS:IP for the profiler.
	mov	ax, @Spanky@numCsIpPtrs
	cmp	ax, CSIP_SLOTS
	je	@@full
	mov	eax, ss:[bx+INT_CSIP]
	rol	ebx, 10h			; keep the frame pointer in the high word
	mov	bx, @Spanky@numCsIpPtrs
	les	di, @Spanky@csIpPtrArray
	shl	bx, 2
	mov	es:[bx+di], eax
	ror	ebx, 10h
	inc	@Spanky@numCsIpPtrs
	jmp	short @@patch
@@full:
	mov	@Spanky@csIpPtrOverflow, 1

	; Rewrite PatchedInt through the writable alias in patchPointer.
@@patch:
	mov	al, cl
	cmp	al, BREAKPOINT_INT
	jne	@@intN
	mov	ax, NOP_INT3
	jmp	short @@store
@@intN:
	mov	ah, al
	mov	al, INT_OPCODE
@@store:
	les	di, @Spanky@patchPointer
	mov	es:[di], ax

	call	@Spanky@setIdtrToOriginal$qv
	mov	ax, ss:[bx+ENTRY_FLAGS]
	push	ax
	popf
	pop	es
	pop	ds
	popa
PatchedInt:
	int	0
	cli
	pushf
	pusha
	push	ds
	push	es
	mov	ax, DGROUP
	mov	ds, ax
	mov	si, sp

	; Return the handler's flags, but with the interrupted code's IF.
	mov	ax, ss:[si+RESULT_FLAGS]
	mov	bx, ss:[si+INT_FLAGS+2]
	and	bx, IF_FLAG
	or	ax, bx
	mov	ss:[si+INT_FLAGS+2], ax
	call	@Spanky@setIdtrToIntercept$qv
	pop	es
	pop	ds
	popa
	popf
	POP_HIGH_WORDS
	popf
	add	sp, 2				; the return address into InterruptEntries
	iret
Interceptor	ENDP

	END
