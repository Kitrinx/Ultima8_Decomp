; name: SB
; flags: /ml /m5 /zd
; Sound Blaster driver: DSP reset and commands, IRQ probing and DMA playback.

	.MODEL	LARGE, C
	LOCALS
	JUMPS

; DSP ports, from the card's base port
DSP_RESET	EQU	6
DSP_READ	EQU	0Ah
DSP_WRITE	EQU	0Ch		; bit 7 set while busy
DSP_STATUS	EQU	0Eh		; bit 7 set when data is waiting; reading it acknowledges the IRQ

; DSP commands
DSP_PLAY_8BIT	EQU	14h		; single-cycle 8-bit DMA output, length - 1 follows
DSP_TIME_CONST	EQU	40h
DSP_HALT_DMA	EQU	0D0h
DSP_SPEAKER_ON	EQU	0D1h
DSP_SPEAKER_OFF	EQU	0D3h
DSP_VERSION	EQU	0E1h
DSP_RAISE_IRQ	EQU	0F2h
DSP_READY	EQU	0AAh		; read back after a reset

DSP_TIMEOUT	EQU	1000		; status polls before giving up

; 8259 interrupt controllers
PIC1_CMD	EQU	20h
PIC1_MASK	EQU	21h
PIC2_CMD	EQU	0A0h
PIC2_MASK	EQU	0A1h
PIC_EOI		EQU	20h
PIC_SPECIFIC_EOI EQU	60h		; or'd with the IRQ

; 8237 DMA controller
DMA_MASK	EQU	0Ah		; channel + 4 masks it, channel alone unmasks
DMA_MODE	EQU	0Bh
DMA_CLEAR_FF	EQU	0Ch
DMA_AUTO_READ	EQU	58h		; single transfers, auto-initialise, memory to card
DMA_SINGLE_READ	EQU	48h		; single transfers, memory to card

; Two short jumps, to give the hardware time between port accesses.
IO_DELAY	MACRO
	jmp	$+2
	jmp	$+2
	ENDM

; A slightly longer pause, leaving every register and flag alone.
SHORT_WAIT	MACRO
	LOCAL	@@again
	pushf
	push	cx
	mov	cx, 20
@@again:
	IO_DELAY
	push	ax
	pop	ax
	loop	@@again
	pop	cx
	popf
	ENDM

; Writes AH to the DSP. Carry set if it stayed busy. Uses AL, CX, DX.
DSP_WRITE_AH	MACRO
	LOCAL	@@busy, @@done
	mov	dx, sbPort
	add	dx, DSP_WRITE
	mov	cx, DSP_TIMEOUT
@@busy:
	in	al, dx
	IO_DELAY
	and	al, 80h
	loopne	@@busy
	stc
	jne	@@done
	mov	al, ah
	out	dx, al
	IO_DELAY
	clc
@@done:
	ENDM

; Reads a DSP byte into AL. Carry set if none arrived. Uses CX, DX.
DSP_READ_AL	MACRO
	LOCAL	@@empty, @@done
	mov	dx, sbPort
	add	dx, DSP_STATUS
	mov	cx, DSP_TIMEOUT
@@empty:
	in	al, dx
	IO_DELAY
	and	al, 80h
	loope	@@empty
	stc
	je	@@done
	sub	dx, DSP_STATUS - DSP_READ
	in	al, dx
	IO_DELAY
	clc
@@done:
	ENDM

; End of interrupt to the controllers sbIrq sits behind.
PIC_EOI_SB	MACRO
	LOCAL	@@first
	cli
	mov	al, PIC_EOI
	cmp	sbIrq, 8
	jl	@@first
	out	PIC2_CMD, al
@@first:
	out	PIC1_CMD, al
	sti
	ENDM

	EXTRN	sndSETPASSTOPROTVEC:FAR
	EXTRN	sndSETREALPROTVEC:FAR

	.DATA

dmaBuffer	dd	0		; as given to sbsetdmabuf
dmaPage		db	0		; physical address bits 16-23
dmaRunning	db	0
dmaAddress	dw	0		; physical address bits 0-15
dmaLength	dw	0
dmaLastByte	dw	0
sbIrq		dw	0
sbPort		dw	220h

; Old vectors of a hooked IRQ: protected-mode handler, then real-mode vector.
; Slots start 4 bytes apart, so each overlaps the next.
savedVectors	db	120 dup (0)

picMask		db	0		; first controller's mask before any hooking
probePorts	dw	220h, 240h, 210h, 230h, 250h, 260h

; DMA settings for the channel sbdmachannel picks; channel 1 to begin with.
dmaMask		db	5
dmaAutoMode	db	59h
dmaPagePort	db	83h
dmaCountPort	db	3
dmaSingleMode	db	48h
pagePorts	db	87h, 83h, 81h, 82h	; by channel
countPorts	db	1, 3, 5, 7
dmaChannel	dw	1
		db	10 dup (0)

; Interrupt number by IRQ, 0FFh where there is none to probe.
irqVectors	db	0FFh, 0FFh, 0Ah, 0Bh, 0Ch, 0Dh, 0FFh, 0Fh, 0FFh, 0FFh, 72h

	.CODE	_TEXT

dspWrite	PROC	NEAR
	DSP_WRITE_AH
	ret
dspWrite	ENDP

dspRead	PROC	NEAR
	DSP_READ_AL
	ret
dspRead	ENDP

; void sb_IRQ_and_port(int irq, int port)
	PUBLIC	sb_IRQ_and_port
sb_IRQ_and_port	PROC	C FAR USES ax, irq:WORD, port:WORD
	in	al, PIC1_MASK
	mov	picMask, al
	mov	word ptr savedVectors, 0
	mov	word ptr savedVectors+2, 0
	mov	word ptr savedVectors+4, 0
	mov	word ptr savedVectors+6, 0
	mov	ax, port
	mov	sbPort, ax
	mov	ax, irq
	mov	sbIrq, ax
	call	dspReset
	ret
sb_IRQ_and_port	ENDP

; int get_DSP_version(void)
; Major version in AH, minor in AL; 0 if the DSP does not answer.
	PUBLIC	get_DSP_version
get_DSP_version	PROC	FAR
	push	bp
	mov	bp, sp
	push	cx
	mov	ah, DSP_VERSION
	call	dspWrite
	jc	@@fail
	call	dspRead
	jc	@@fail
	mov	ah, al
	call	dspRead
	jc	@@fail
	pop	cx
	pop	bp
	ret
@@fail:
	mov	ax, 0
	pop	cx
	pop	bp
	ret
get_DSP_version	ENDP

; BX = IRQ, CS:DX = handler, SI = slot from vectorSlot.
; Saves the old vectors in the slot, installs the handler and unmasks the IRQ.
hookIrq	PROC	NEAR
	push	ds
	cli
	push	bx
	push	si
	mov	si, bx
	xor	bx, bx
	mov	bl, irqVectors[si]
	pop	si
	push	bx
	push	cs
	push	dx
	push	ds
	push	si
	add	si, 4
	push	ds
	push	si
	call	sndSETPASSTOPROTVEC
	add	sp, 14
	pop	cx
	mov	bx, cx
	cmp	cl, 8
	jl	@@firstPic
	mov	al, PIC_SPECIFIC_EOI
	or	al, cl
	out	PIC2_CMD, al
	IO_DELAY
	mov	ah, 4			; IRQ 10, the only one probed up here
	not	ah
	in	al, PIC2_MASK
	IO_DELAY
	and	al, ah
	out	PIC2_MASK, al
	IO_DELAY
	mov	cl, 2			; the cascade
@@firstPic:
	mov	al, PIC_SPECIFIC_EOI
	or	al, cl
	out	PIC1_CMD, al
	IO_DELAY
	mov	ah, 1
	shl	ah, cl
	not	ah
	in	al, PIC1_MASK
	IO_DELAY
	and	al, ah
	out	PIC1_MASK, al
	IO_DELAY
	sti
	pop	ds
	ret
hookIrq	ENDP

; BX = IRQ, SI = its slot. Puts the saved vectors back and masks the IRQ.
unhookIrq	PROC	NEAR
	push	ds
	cli
	push	bx
	push	si
	push	si
	mov	si, bx
	xor	bx, bx
	mov	bl, irqVectors[si]
	pop	si
	push	bx
	push	word ptr [si+2]
	push	word ptr [si]
	push	word ptr [si+6]
	push	word ptr [si+4]
	push	ds
	push	si
	push	ds
	push	si
	call	sndSETREALPROTVEC
	add	sp, 18
	pop	si
	pop	cx
	mov	bx, cx
	mov	al, PIC_SPECIFIC_EOI
	or	al, cl
	out	PIC1_CMD, al
	IO_DELAY
	mov	ah, 1
	shl	ah, cl
	in	al, PIC1_MASK
	IO_DELAY
	or	al, ah
	out	PIC1_MASK, al
	IO_DELAY
	sti
	pop	ds
	ret
unhookIrq	ENDP

; void sbdmachannel(int channel)
	PUBLIC	sbdmachannel
sbdmachannel	PROC	C NEAR channel:WORD
	push	channel
	pop	dmaChannel
	push	si
	push	ax
	mov	al, byte ptr dmaChannel
	mov	dmaMask, al
	add	dmaMask, 4
	mov	dmaAutoMode, al
	mov	dmaSingleMode, al
	add	dmaAutoMode, DMA_AUTO_READ
	add	dmaSingleMode, DMA_SINGLE_READ
	xor	ax, ax
	mov	al, byte ptr dmaChannel
	mov	si, ax
	mov	al, pagePorts[si]
	mov	dmaPagePort, al
	mov	al, countPorts[si]
	mov	dmaCountPort, al
	pop	ax
	pop	si
	ret
sbdmachannel	ENDP

; BX = IRQ 2, 3, 5, 7 or 10. Returns its savedVectors slot in SI.
vectorSlot	PROC	NEAR
	push	bx
	pushf
	shr	bx, 1
	adc	bx, -1
	shl	bx, 1
	shl	bx, 1
	lea	si, savedVectors[bx]
	popf
	pop	bx
	ret
vectorSlot	ENDP

; Handlers hooked while probing: each stores its IRQ in sbIrq.
probeIrq2:
	push	ax
	mov	ax, 2
	jmp	probeCommon
probeIrq3:
	push	ax
	mov	ax, 3
	jmp	probeCommon
probeIrq5:
	push	ax
	mov	ax, 5
	jmp	probeCommon
probeIrq7:
	push	ax
	mov	ax, 7
	jmp	probeCommon
probeIrq10:
	push	ax
	mov	ax, 10
probeCommon:
	; Swap the pushed AX for DGROUP and pop it into DS.
	push	ds
	push	ax
	push	bp
	mov	bp, sp
	mov	word ptr [bp+2], DGROUP
	pop	bp
	pop	ds
	mov	sbIrq, ax
	push	dx
	mov	dx, sbPort
	add	dx, DSP_STATUS
	in	al, dx
	IO_DELAY
	PIC_EOI_SB
	pop	dx
	pop	ds
	pop	ax
	iret

; int sbinit(int port, int irq)
; Resets the DSP: 0 if it answered, else -1 with carry set.
	PUBLIC	sbinit
sbinit	PROC	C NEAR port:WORD, irq:WORD
	in	al, PIC1_MASK
	mov	picMask, al
	mov	word ptr savedVectors, 0
	mov	word ptr savedVectors+2, 0
	mov	word ptr savedVectors+4, 0
	mov	word ptr savedVectors+6, 0
	mov	ax, port
	mov	sbPort, ax
	mov	ax, irq
	mov	sbIrq, ax
	call	dspReset
	ret
sbinit	ENDP

; void sbdeinit(void)
	PUBLIC	sbdeinit
sbdeinit	PROC	NEAR
	call	sbstopdma
	mov	al, picMask
	out	PIC1_MASK, al
	call	dspReset
	ret
sbdeinit	ENDP

; int sbdetect(void)
; Finds the card: port in AX, IRQ in DX. Both -1 with carry set if none.
	PUBLIC	sbdetect
sbdetect	PROC	NEAR USES si di
	in	al, PIC1_MASK
	mov	picMask, al
	mov	di, 6
	mov	si, offset probePorts
@@nextPort:
	mov	ax, [si]
	mov	sbPort, ax
	call	dspReset
	mov	ax, sbPort
	jnc	@@found
	add	si, 2
	dec	di
	jne	@@nextPort
@@fail:
	mov	ax, -1
	mov	dx, -1
	stc
	ret

	; Hook every likely IRQ and have the card raise its own.
@@found:
	push	ax
	push	si
	in	al, PIC1_MASK
	mov	picMask, al
	mov	bx, 10
	mov	dx, offset probeIrq10
	call	vectorSlot
	call	hookIrq
	mov	bx, 7
	mov	dx, offset probeIrq7
	call	vectorSlot
	call	hookIrq
	mov	bx, 2
	mov	dx, offset probeIrq2
	call	vectorSlot
	call	hookIrq
	mov	bx, 3
	mov	dx, offset probeIrq3
	call	vectorSlot
	call	hookIrq
	mov	bx, 5
	mov	dx, offset probeIrq5
	call	vectorSlot
	call	hookIrq
	pop	si
	mov	sbIrq, -1
	mov	ah, DSP_RAISE_IRQ
	call	dspWrite
	mov	cx, 5000
@@waitIrq:
	SHORT_WAIT
	cmp	sbIrq, -1
	loope	@@waitIrq

	mov	al, picMask
	out	PIC1_MASK, al
	mov	bx, 2
	call	vectorSlot
	call	unhookIrq
	mov	bx, 3
	call	vectorSlot
	call	unhookIrq
	mov	bx, 5
	call	vectorSlot
	call	unhookIrq
	mov	bx, 7
	call	vectorSlot
	call	unhookIrq
	mov	dx, sbIrq
	pop	ax
	cmp	dx, -1
	je	@@fail
	clc
	ret
sbdetect	ENDP

; Resets the DSP at sbPort: AX 0 if it answered, else -1 with carry set.
dspReset	PROC	NEAR
	mov	dx, sbPort
	add	dx, DSP_STATUS
	in	al, dx
	IO_DELAY
	mov	dx, sbPort
	add	dx, DSP_RESET
	mov	al, 1
	out	dx, al
	IO_DELAY
	SHORT_WAIT
	SHORT_WAIT
	SHORT_WAIT
	dec	al
	out	dx, al
	IO_DELAY
	SHORT_WAIT
	SHORT_WAIT
	SHORT_WAIT
	mov	cx, DSP_TIMEOUT
@@retry:
	push	cx
	DSP_READ_AL
	pop	cx
	cmp	al, DSP_READY
	loopne	@@retry
	stc
	jne	@@done
	clc
@@done:
	sbb	ax, ax
	rol	ax, 1
	ret
dspReset	ENDP

; int sbversion(void)
; Major version in AH, minor in AL; -1 with carry set if the DSP does not answer.
	PUBLIC	sbversion
sbversion	PROC	NEAR
	mov	ah, DSP_VERSION
	call	dspWrite
	jc	@@fail
	call	dspRead
	mov	ah, al
	jc	@@fail
	call	dspRead
	jc	@@fail
	ret
@@fail:
	mov	ax, -1
	ret
sbversion	ENDP

; void sbsetdmabuf(void far *buf, int len, int realSeg)
; realSeg is the buffer's real-mode segment, for its physical address.
	PUBLIC	sbsetdmabuf
sbsetdmabuf	PROC	C NEAR buf:DWORD, len:WORD, realSeg:WORD
	mov	ax, len
	mov	dmaLength, ax
	dec	ax
	mov	dmaLastByte, ax
	les	ax, buf
	mov	dx, es
	mov	word ptr dmaBuffer, ax
	mov	word ptr dmaBuffer+2, dx
	mov	dx, realSeg
	mov	cl, 4
	rol	dx, cl
	mov	bx, dx
	and	dx, 0Fh
	and	bx, 0FFF0h
	add	ax, bx
	adc	dx, 0
	mov	dmaAddress, ax
	mov	dmaPage, dl
	ret
sbsetdmabuf	ENDP

; int sbspeakeron(void)
; 0, or -1 with carry set if the DSP is stuck busy.
	PUBLIC	sbspeakeron
sbspeakeron	PROC	NEAR
	mov	ah, DSP_SPEAKER_ON
	call	dspWrite
	sbb	ax, ax
	rol	ax, 1
	ret
sbspeakeron	ENDP

; Nothing calls this.
speakerOff	PROC	NEAR
	mov	ah, DSP_SPEAKER_OFF
	call	dspWrite
	sbb	ax, ax
	rol	ax, 1
	ret
speakerOff	ENDP

; int sbsamplerate(int rate)
; Sets the time constant 256 - 1000000 / rate; rate 0 changes nothing.
	PUBLIC	sbsamplerate
sbsamplerate	PROC	C NEAR rate:WORD
	mov	bx, rate
	or	bx, bx
	je	@@done
	mov	dx, 1000000 SHR 16
	mov	ax, 1000000 AND 0FFFFh
	div	bx
	neg	ax
	mov	bx, ax
	mov	ah, DSP_TIME_CONST
	call	dspWrite
	jc	@@done
	mov	ah, bl
	call	dspWrite
@@done:
	sbb	ax, ax
	rol	ax, 1
	ret
sbsamplerate	ENDP

; IRQ handler for the end of a DSP block. The DMA channel loops over the
; buffer by itself, so this only restarts the DSP for another 64K bytes.
dmaBlockDone:
	push	ds
	push	ax
	mov	ax, DGROUP
	mov	ds, ax
	push	cx
	push	dx
	mov	dx, sbPort
	add	dx, DSP_STATUS
	in	al, dx
	IO_DELAY
	mov	ah, DSP_PLAY_8BIT
	DSP_WRITE_AH
	mov	ah, 0FFh
	DSP_WRITE_AH
	DSP_WRITE_AH
	PIC_EOI_SB
	pop	dx
	pop	cx
	pop	ax
	pop	ds
	iret

; int sbstartdma(void)
; Hooks the IRQ, sets the DMA channel looping over the buffer and starts the DSP.
; 0, or -1 with carry set if the DSP is stuck busy. Does nothing if already running.
	PUBLIC	sbstartdma
sbstartdma	PROC	NEAR
	mov	al, dmaRunning
	or	al, al
	clc
	jne	@@done
	mov	al, 1
	mov	dmaRunning, al
	mov	bx, sbIrq
	mov	dx, offset dmaBlockDone
	push	si
	mov	si, offset savedVectors
	call	hookIrq
	pop	si
	push	dx
	xor	dx, dx
	mov	al, dmaMask
	out	DMA_MASK, al
	IO_DELAY
	mov	al, 0
	out	DMA_CLEAR_FF, al
	IO_DELAY
	mov	al, dmaAutoMode
	out	DMA_MODE, al
	IO_DELAY
	mov	ax, dmaAddress
	mov	dl, byte ptr dmaChannel
	shl	dl, 1			; the channel's address port
	out	dx, al
	IO_DELAY
	mov	al, ah
	out	dx, al
	IO_DELAY
	mov	al, dmaPage
	mov	dl, dmaPagePort
	out	dx, al
	IO_DELAY
	mov	bx, dmaLength
	dec	bx
	mov	dl, dmaCountPort
	mov	al, bl
	out	dx, al
	IO_DELAY
	mov	al, bh
	out	dx, al
	IO_DELAY
	mov	al, byte ptr dmaChannel
	out	DMA_MASK, al
	IO_DELAY
	pop	dx
	mov	ah, DSP_PLAY_8BIT
	call	dspWrite
	jc	@@done
	mov	cx, 20000
@@pause:
	SHORT_WAIT
	loop	@@pause
	mov	ah, 0FFh		; length 0FFFFh, low byte then high
	call	dspWrite
	jc	@@done
	call	dspWrite
	jc	@@done
@@done:
	sbb	ax, ax
	rol	ax, 1
	ret
sbstartdma	ENDP

; Puts back the vector sbstartdma hooked, if it did.
releaseDmaIrq	PROC	NEAR
	push	si
	mov	si, offset savedVectors
	mov	bx, [si+6]
	or	bx, bx
	je	@@done
	mov	bx, sbIrq
	call	unhookIrq
	xor	ax, ax
	mov	word ptr savedVectors, ax
	mov	word ptr savedVectors+2, ax
	mov	word ptr savedVectors+4, ax
	mov	word ptr savedVectors+6, ax
@@done:
	pop	si
	ret
releaseDmaIrq	ENDP

; Sends DSP_HALT_DMA once the write port has gone busy and then ready, or timed out.
dspHaltDma	PROC	NEAR
	push	ax
	push	dx
	push	cx
	push	si
	pushf
	mov	ah, DSP_HALT_DMA
	mov	dx, sbPort
	add	dl, DSP_WRITE
	mov	si, 100
	dec	si
	or	si, si
	je	@@send
	mov	cx, 100
@@waitBusy:
	sti
	dec	cx
	jcxz	@@send
	cli
	in	al, dx
	IO_DELAY
	or	al, al
	jns	@@waitBusy
	mov	cx, 100
@@waitReady:
	dec	cx
	jcxz	@@send
	in	al, dx
	IO_DELAY
	or	al, al
	js	@@waitReady
	jmp	@@send
@@send:
	mov	al, ah
	mov	cx, 1
@@write:
	out	dx, al
	IO_DELAY
	loop	@@write
	sti
	popf
	pop	si
	pop	cx
	pop	dx
	pop	ax
	ret
dspHaltDma	ENDP

; void sbstopdma(void)
	PUBLIC	sbstopdma
sbstopdma	PROC	NEAR
	cmp	dmaRunning, 0
	clc
	je	@@done
	call	dspHaltDma
	cli
	mov	al, dmaMask
	out	DMA_MASK, al
	IO_DELAY
	mov	dx, sbPort
	add	dx, DSP_STATUS
	in	al, dx
	IO_DELAY
	xor	al, al
	mov	dmaRunning, al
	mov	bx, word ptr savedVectors
	or	bx, bx
	je	@@done
	call	releaseDmaIrq
	mov	cx, sbIrq
	cmp	cx, -1
	je	@@done
	mov	ah, 1
	shl	ah, cl
	in	al, PIC1_MASK
	or	al, ah
	out	PIC1_MASK, al
@@done:
	sti
	ret
sbstopdma	ENDP

; int sbfifopos(void)
; How far into the buffer the DMA channel has got.
	PUBLIC	sbfifopos
sbfifopos	PROC	NEAR
	push	dx
	xor	dx, dx
	mov	dl, dmaCountPort
	in	al, dx
	IO_DELAY
	mov	ah, al
	in	al, dx
	pop	dx
	IO_DELAY
	xchg	al, ah
	mov	bx, ax
	mov	ax, dmaLength
	dec	ax
	sub	ax, bx
	ret
sbfifopos	ENDP

	END
