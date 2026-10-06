; name: SBDAC
; flags: /ml /m5 /zd
; Plays the mixer's output on a Sound Blaster through a looping two-half DMA buffer.

	.MODEL	LARGE
	LOCALS

DMA_HALF	EQU	128		; bytes in each half of the DMA buffer
SAMPLE_RATE	EQU	11025

	EXTRN	_DAC_IsPaused:WORD
	EXTRN	_DRV_UpdateHardware:FAR

	.DATA

fillHalf	db	0		; half DAC_Output fills next: 0 or DMA_HALF
dmaStarted	dw	0
	PUBLIC	_mondo
_mondo	dw	0			; counts DAC_update calls
	db	0

; Turns a mixed sample from -512 to 511, at clipTable + 512, into an unsigned byte.
clipTable	db	384 dup (0)
ramp = 0
	REPT	256
	db	ramp
ramp = ramp + 1
	ENDM
	db	384 dup (0FFh)

dmaChannel	dw	1
bufOffset	dw	0
bufSelector	dw	0
bufSegment	dw	0		; real-mode segment of the buffer
sbPort		dw	220h
sbIrq		dw	7

	.CODE	_TEXT

; The Sound Blaster routines share this code segment and are called near.
	EXTRN	_sbdmachannel:NEAR
	EXTRN	_sbinit:NEAR
	EXTRN	_sbdeinit:NEAR
	EXTRN	_sbdetect:NEAR
	EXTRN	_sbversion:NEAR
	EXTRN	_sbsetdmabuf:NEAR
	EXTRN	_sbspeakeron:NEAR
	EXTRN	_sbsamplerate:NEAR
	EXTRN	_sbstartdma:NEAR
	EXTRN	_sbstopdma:NEAR
	EXTRN	_sbfifopos:NEAR

; unsigned char DAC_update(void)
; Mixes into the next half once the card is no longer playing it.
	PUBLIC	_DAC_update
_DAC_update	PROC	FAR
	inc	_mondo
	call	near ptr _sbfifopos
	cmp	ax, DMA_HALF
	jge	@@second
	mov	al, 0
	jmp	@@check
@@second:
	mov	al, DMA_HALF
@@check:
	mov	bl, fillHalf
	cmp	al, bl
	je	@@done
	call	_DRV_UpdateHardware
@@done:
	ret
_DAC_update	ENDP

; int DAC_AreYouHungry(void)
; 1 once the card is no longer playing the half DAC_Output fills next.
	PUBLIC	_DAC_AreYouHungry
_DAC_AreYouHungry	PROC	NEAR
	call	near ptr _sbfifopos
	cmp	ax, DMA_HALF
	jge	@@second
	mov	al, 0
	jmp	@@check
@@second:
	mov	al, DMA_HALF
@@check:
	mov	bl, fillHalf
	cmp	al, bl
	je	@@full
	mov	ax, 1
	ret
@@full:
	mov	ax, 0
	ret
_DAC_AreYouHungry	ENDP

; Nothing calls this: stops the DMA loop if it is running.
stopPlayback	PROC	NEAR
	cmp	dmaStarted, 0
	je	@@done
	xor	ax, ax
	mov	dmaStarted, ax
	call	near ptr _sbstopdma
@@done:
	ret
stopPlayback	ENDP

; Sets up the card and starts the DMA loop if not yet started. 0, or -1 on failure.
startPlayback	PROC	NEAR
	cmp	dmaStarted, 0
	jne	@@done
	push	dmaChannel
	call	near ptr _sbdmachannel
	pop	dmaChannel
	mov	ax, sbPort
	mov	dx, sbIrq
	push	dx
	push	ax
	call	near ptr _sbinit
	pop	bx
	pop	bx
	jc	@@fail
	call	near ptr _sbversion
	jc	@@fail
	push	bufSegment
	mov	ax, 2 * DMA_HALF
	push	ax
	push	bufSelector
	mov	ax, bufOffset
	push	ax
	call	near ptr _sbsetdmabuf
	add	sp, 8
	mov	ax, SAMPLE_RATE
	push	ax
	call	near ptr _sbsamplerate
	pop	ax
	call	near ptr _sbspeakeron
	jc	@@fail
	call	near ptr _sbstartdma
	jc	@@fail
	mov	ax, 1
	mov	dmaStarted, ax
@@done:
	xor	ax, ax
	ret
@@fail:
	xor	ax, ax
	mov	dmaStarted, ax
	call	near ptr _sbdeinit
	mov	ax, -1
	ret
startPlayback	ENDP

; int DAC_Init(void)
; AX = port and DX = IRQ (detected unless both are nonzero), BX:CX = buffer
; offset and selector, SI = its real-mode segment, DI = DMA channel.
; 0, or -1 on failure.
	PUBLIC	_DAC_Init
_DAC_Init	PROC	NEAR
	push	ax
	push	dx
	mov	bufOffset, bx
	mov	bufSelector, cx
	mov	bufSegment, si
	mov	dmaChannel, di

	; Both halves must lie in one 64K DMA page.
	mov	dx, si
	xor	ax, ax
	call	toPhysical
	mov	bx, dx
	mov	dx, si
	mov	ax, DMA_HALF
	call	toPhysical
	cmp	bx, dx
	jne	@@fail			; with AX and DX still pushed
	pop	dx
	pop	ax
	pushf
	sti

	; BX = -1 only when both port and IRQ are nonzero.
	mov	bx, dx
	mov	cx, ax
	neg	bx
	sbb	bx, bx
	neg	cx
	sbb	cx, cx
	and	bx, cx
	jne	@@known
	call	near ptr _sbdetect
	jc	@@fail
@@known:
	mov	sbPort, ax
	mov	sbIrq, dx
	call	startPlayback
	popf
	ret
@@fail:
	xor	ax, ax
	mov	dmaStarted, ax
	call	near ptr _sbdeinit
	mov	ax, -1
	popf
	ret
_DAC_Init	ENDP

; void DAC_Deinit(void)
	PUBLIC	_DAC_Deinit
_DAC_Deinit	PROC	NEAR
	push	si
	pushf
	cmp	dmaStarted, 0
	je	@@done
	xor	ax, ax
	mov	dmaStarted, ax
	call	near ptr _sbdeinit
@@done:
	popf
	pop	si
	ret
_DAC_Deinit	ENDP

; int DAC_Pause(void)
	PUBLIC	_DAC_Pause
_DAC_Pause	PROC	NEAR
	mov	ax, 1
	mov	_DAC_IsPaused, ax
	ret
_DAC_Pause	ENDP

; int DAC_UnPause(void)
	PUBLIC	_DAC_UnPause
_DAC_UnPause	PROC	NEAR
	xor	ax, ax
	mov	_DAC_IsPaused, ax
	ret
_DAC_UnPause	ENDP

; char DAC_Output(void)
; SI = DMA_HALF mixed samples, a word each. Clips them into the half at
; fillHalf, then moves fillHalf to the other half.
	PUBLIC	_DAC_Output
_DAC_Output	PROC	NEAR
	push	si
	push	di
	push	es
	cld
	mov	ax, bufSelector
	mov	es, ax
	mov	cx, DMA_HALF
	xor	ax, ax
	mov	al, fillHalf
	mov	di, bufOffset
	add	di, ax
@@next:
	mov	bx, [si]
	add	si, 2
	mov	al, clipTable[bx + 512]
	stosb
	loop	@@next
	xor	fillHalf, DMA_HALF
	pop	es
	pop	di
	pop	si
	ret
_DAC_Output	ENDP

; DX:AX = physical address of segment DX, offset AX.
toPhysical	PROC	NEAR
	push	cx
	push	cx
	mov	cl, 4
	rol	dx, cl
	mov	cx, dx
	and	dx, 0Fh
	and	cx, 0FFF0h
	add	ax, cx
	adc	dx, 0
	pop	cx
	pop	cx
	ret
toPhysical	ENDP

	END
