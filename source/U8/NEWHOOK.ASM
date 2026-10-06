; name: NEWHOOK
; flags: /ml /m5 /zd
; Mouse driver event hook: passes each event to the registered mouse handlers.

	.MODEL	LARGE
	.386
	LOCALS

MAX_HANDLERS	EQU	4
STACK_GUARD	EQU	0FFFFh

; MouseHandler as the C++ side lays it out.
MOUSEHANDLER	STRUC
mhVtable	dw	?	; near pointer to the vtable; handle() is its first entry
mhSlot		dw	?
mhMask		dw	?	; events this handler wants
MOUSEHANDLER	ENDS

	.DATA

	EXTRN	@MouseDevice@mouseLastX:WORD
	EXTRN	@MouseDevice@mouseLastY:WORD
	EXTRN	@MouseHandler@handlers:DWORD

; Set while an event is being handled, so a nested call returns at once.
	PUBLIC	_mouseActive
_mouseActive	db	0
savedSS		dw	0
savedSP		dw	0
handlerIndex	dw	0
eventMask	dw	0
	PUBLIC	_fakeStackOverflowFlag
_fakeStackOverflowFlag	dw	0

; Private stack for the handlers, with a guard word at each end. The bottom
; one is overwritten if the handlers run out of stack.
	PUBLIC	_fakeStack
_fakeStack	dw	STACK_GUARD
		dw	511 dup (0)
fakeStackTop	dw	0
		dw	STACK_GUARD
		db	0

	.CODE

; void far mouseHook(void)
; Called by the mouse driver with AX = events, BX = buttons, CX = x, DX = y.
; Switches to the private stack and calls handle(events, buttons, x, y) on
; every handler whose mask has one of the events.
	PUBLIC	_mouseHook
_mouseHook	PROC	FAR
	pushf
	cli
	cld
	push	ds
	push	ax
	mov	ax, DGROUP
	mov	ds, ax
	cmp	byte ptr [_mouseActive], 0
	je	@@enter
	pop	ax
	pop	ds
	popf
	retf
@@enter:
	mov	byte ptr [_mouseActive], 1
	pop	ax
	mov	[eventMask], ax
	mov	[savedSS], ss
	mov	[savedSP], sp
	mov	ax, SEG fakeStackTop
	mov	ss, ax
	mov	ax, OFFSET DGROUP:fakeStackTop
	mov	sp, ax
	sti
	mov	ax, [eventMask]
	pushad
	push	fs
	push	gs
	push	ds
	push	es

	; Arguments for handle(), shared by every call.
	push	dx
	push	cx
	push	bx
	push	ax
	mov	[@MouseDevice@mouseLastX], cx
	mov	[@MouseDevice@mouseLastY], dx

	; Last handler first.
	mov	si, MAX_HANDLERS
	dec	si
	js	@@done
@@nextHandler:
	shl	si, 2
	les	bx, @MouseHandler@handlers[si]
	mov	dx, es
	or	dx, dx
	je	@@skip
	mov	ax, [eventMask]
	and	ax, es:[bx].mhMask
	je	@@skip
	push	es
	push	bx
	mov	[handlerIndex], si
	mov	bx, es:[bx].mhVtable
	call	dword ptr [bx]
	add	sp, 4
	mov	si, [handlerIndex]
@@skip:
	shr	si, 2
	dec	si
	jns	@@nextHandler
@@done:
	add	sp, 8

	mov	ax, word ptr [_fakeStack]
	cmp	ax, STACK_GUARD
	je	@@stackOk
	mov	[_fakeStackOverflowFlag], 1
@@stackOk:
	pop	es
	pop	ds
	pop	gs
	pop	fs
	popad
	cli
	mov	ss, [savedSS]
	mov	sp, [savedSP]
	mov	byte ptr [_mouseActive], 0
	pop	ds
	popf
	retf
_mouseHook	ENDP

	END
