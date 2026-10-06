; name: INTERUPT
; flags: /ml
; Interrupt vector and 8259 PIC helpers.

	.MODEL	LARGE, C
	LOCALS
	.386

PIC1		EQU	20h	; master command port; its mask is PIC1 + 1
PIC2		EQU	0A0h	; slave
EOI		EQU	20h

	.CODE

; void setInterrupt(char vector, void far *handler)
	PUBLIC	setInterrupt
setInterrupt	PROC	FAR vector:BYTE, handler:DWORD
	push	ds
	mov	ah, 25h
	mov	al, vector
	mov	dx, word ptr handler
	mov	ds, word ptr handler + 2
	int	21h
	pop	ds
	ret
setInterrupt	ENDP

; void far *getInterrupt(char vector)
	PUBLIC	getInterrupt
getInterrupt	PROC	FAR vector:BYTE
	push	es
	mov	ah, 35h
	mov	al, vector
	int	21h
	mov	ax, bx
	mov	dx, es
	pop	es
	ret
getInterrupt	ENDP

; void disableInterrupt(char irq): masks the IRQ line.
	PUBLIC	disableInterrupt
disableInterrupt	PROC	FAR irq:BYTE
	push	ax
	push	bx
	push	cx
	push	dx
	pushf
	cli
	mov	bl, 1
	mov	cl, irq
	cmp	cl, 8
	jge	@@slave
	shl	bl, cl
	mov	dx, PIC1 + 1
	in	al, dx
	or	al, bl
	out	dx, al
	jmp	@@done
@@slave:
	sub	cl, 8
	shl	bl, cl
	mov	dx, PIC2 + 1
	in	al, dx
	or	al, bl
	out	dx, al
@@done:
	popf
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
disableInterrupt	ENDP

; void enableInterrupt(char irq): unmasks the IRQ line.
	PUBLIC	enableInterrupt
enableInterrupt	PROC	FAR irq:BYTE
	push	ax
	push	bx
	push	cx
	push	dx
	pushf
	cli
	mov	bl, 1
	mov	cl, irq
	cmp	cl, 8
	jge	@@slave
	shl	bl, cl
	not	bl
	mov	dx, PIC1 + 1
	in	al, dx
	and	al, bl
	out	dx, al
	jmp	@@done
@@slave:
	sub	cl, 8
	shl	bl, cl
	not	bl
	mov	dx, PIC2 + 1
	in	al, dx
	and	al, bl
	out	dx, al
@@done:
	popf
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
enableInterrupt	ENDP

; void sendEoi(char irq): ends the interrupt at the slave PIC too for IRQ 8-15.
	PUBLIC	sendEoi
sendEoi	PROC	FAR irq:BYTE
	push	ax
	push	dx
	pushf
	cli
	mov	al, EOI
	cmp	irq, 8
	jl	@@master
	mov	dx, PIC2
	out	dx, al
@@master:
	mov	dx, PIC1
	out	dx, al
	popf
	pop	dx
	pop	ax
	ret
sendEoi	ENDP

	END
