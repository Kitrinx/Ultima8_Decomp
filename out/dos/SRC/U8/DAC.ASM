; name: DAC
; flags: /ml /m5 /zd
; Mixes four 8-bit sample channels into a buffer for the sound card, 128 samples at a time.

	.MODEL	LARGE, C
	JUMPS
	LOCALS

NUM_CHANNELS	EQU	4
BLOCK_SAMPLES	EQU	128		; output samples mixed per update
MIX_RATE	EQU	11025		; output rate in Hz
SILENT_BLOCKS	EQU	20		; silent blocks sent once idle, before pausing the DAC
MAX_VOLUME	EQU	255

CH_IDLE		EQU	0
CH_PLAYING	EQU	1
CH_RELEASED	EQU	2		; plays on to the end without looping

; One sound channel. Offsets are into the sample's segment; positions and
; steps are 16.16 fixed point.
Channel	STRUC
chState		db	?
chPad		db	?
chLoopLen	dw	?		; nonzero makes the sample loop
chSeg		dw	?
chStart		dw	?
chLoopStart	dw	?
chLoopEnd	dw	?
chEnd		dw	?
chRestart	dw	?		; where play resumes on reaching chStop when looping
chStop		dw	?
chBaseStep	dw	?		; step as last set, kept beside the live one
chBaseFrac	dw	?
chPos		dw	?
chPosFrac	dw	?
chStep		dw	?		; advance per output sample: rate / MIX_RATE
chStepFrac	dw	?
chSpare		dw	?
chVolume	dw	?		; 0..255
chVolTable	dw	?		; VolumeTable row for chVolume
Channel	ENDS

	EXTRN	digital_call_back:FAR

	.DATA

; 16 rows of 256 bytes, built at start-up: row v holds s * v / 16 for each
; signed sample byte s.
VolumeTable	db	4096 dup (0)

chan0		Channel	<>
chan1		Channel	<>
chan2		Channel	<>
chan3		Channel	<>
chanTable	dw	chan0, chan1, chan2, chan3

; The slow path mixes in runs that end at chStop.
blockStart	dw	0		; position the current run started from
runLength	dw	0
runEnd		dw	0
mixIndex	db	0		; next DACBuffer word the slow path adds to
		db	0
silentBlocks	dw	SILENT_BLOCKS

	PUBLIC	DACBuffer
DACBuffer	dw	256 dup (0)	; signed sums; the first BLOCK_SAMPLES are mixed
	PUBLIC	DAC_IsPaused
DAC_IsPaused	dw	0

	.CODE	_TEXT

; The card driver shares this code segment and is called near.
	EXTRN	DAC_Init:NEAR
	EXTRN	DAC_Deinit:NEAR
	EXTRN	DAC_Pause:NEAR
	EXTRN	DAC_UnPause:NEAR
	EXTRN	DAC_Output:NEAR

; Points si at the record for `channel`, or jumps to `skip` when it is out of range.
LOAD_CHANNEL	MACRO	channel, skip
	mov	si, channel
	cmp	si, NUM_CHANNELS - 1
	jg	skip
	cmp	si, 0
	jl	skip
	shl	si, 1
	mov	si, chanTable[si]
	ENDM

; Sets the step of the channel at si to rate / MIX_RATE.
SET_STEP	MACRO	rate
	mov	cx, MIX_RATE
	xor	dx, dx
	mov	ax, rate
	div	cx
	mov	[si].chStep, ax
	mov	[si].chBaseStep, ax
	xor	ax, ax
	div	cx
	mov	[si].chStepFrac, ax
	mov	[si].chBaseFrac, ax
	ENDM

BuildVolumeTables	PROC	NEAR
	mov	dx, 0
@@row:
	mov	di, OFFSET VolumeTable
	xor	ax, ax
	mov	ah, dl
	add	di, ax
	mov	cx, 100h
	xor	bx, bx
	mov	si, dx
	REPT	4
	shl	si, 1
	ENDM
	; bx runs 0..127 then -128..-1, the signed value of each sample byte.
@@entry:
	mov	ax, si
	push	dx
	imul	bx
	pop	dx
	mov	[di], ah
	inc	bx
	mov	ax, bx
	cbw
	mov	bx, ax
	inc	di
	loop	@@entry
	inc	dx
	cmp	dx, 10h
	jne	@@row
	ret
BuildVolumeTables	ENDP

StopAllChannels	PROC	NEAR
	push	si
	mov	cx, NUM_CHANNELS
	mov	al, CH_IDLE
	xor	si, si
@@next:
	mov	bx, chanTable[si]
	mov	[bx].chState, al
	inc	si
	inc	si
	loop	@@next
	pop	si
	ret
StopAllChannels	ENDP

; int DGTL_init(int port, int irq, int dma, unsigned dmaSelector, unsigned dmaOffset, unsigned dmaParagraph)
; Returns -1 when the card cannot be set up.
	PUBLIC	DGTL_init
DGTL_init	PROC	C FAR USES si, port:WORD, irq:WORD, dma:WORD, dmaSel:WORD, dmaOff:WORD, dmaPara:WORD
	call	near ptr BuildVolumeTables
	call	near ptr StopAllChannels
	mov	bx, dmaOff
	mov	cx, dmaSel
	mov	si, dmaPara
	mov	ax, port
	mov	dx, irq
	mov	di, dma
	call	near ptr DAC_Init
	ret
DGTL_init	ENDP

; void DGTL_shutdown(void)
	PUBLIC	DGTL_shutdown
DGTL_shutdown	PROC	C FAR
	call	near ptr StopAllChannels
	call	near ptr DAC_Deinit
	ret
DGTL_shutdown	ENDP

; void DGTL_channel_volume(int channel, int volume)
	PUBLIC	DGTL_channel_volume
DGTL_channel_volume	PROC	C FAR USES si di, channel:WORD, volume:WORD
	mov	si, volume
	mov	si, channel
	cmp	si, NUM_CHANNELS - 1
	jg	@@done
	cmp	si, 0
	jl	@@done
	mov	ax, volume
	shl	si, 1
	mov	si, chanTable[si]
	lea	bx, [si].chVolume
	mov	[bx], ax
	lea	si, [si].chVolTable
	mov	ax, volume
	cmp	ax, MAX_VOLUME
	jle	@@inRange
	mov	ax, MAX_VOLUME
@@inRange:
	; Row volume / 16 starts 256 * (volume / 16) bytes in.
	REPT	4
	shl	ax, 1
	ENDM
	xor	al, al
	mov	bx, OFFSET VolumeTable
	add	ax, bx
	mov	[si], ax
@@done:
	ret
DGTL_channel_volume	ENDP

; void DGTL_channel_rate(int channel, int rate)
	PUBLIC	DGTL_channel_rate
DGTL_channel_rate	PROC	C FAR USES si, channel:WORD, rate:WORD
	LOAD_CHANNEL	channel, @@done
	SET_STEP	rate
@@done:
	ret
DGTL_channel_rate	ENDP

; void DGTL_play(int channel, char far *sample, int length, int rate, int volume)
	PUBLIC	DGTL_play
DGTL_play	PROC	C FAR USES si, channel:WORD, sample:DWORD, len:WORD, rate:WORD, volume:WORD
	LOAD_CHANNEL	channel, @@done
	cli
	mov	ax, word ptr sample
	mov	dx, word ptr sample + 2
	mov	[si].chSeg, dx
	mov	[si].chStart, ax
	mov	[si].chLoopStart, ax
	mov	[si].chLoopEnd, ax
	add	ax, len
	mov	[si].chEnd, ax
	mov	[si].chLoopLen, 0
	SET_STEP	rate
	push	volume
	push	channel
	push	cs
	call	near ptr DGTL_channel_volume
	add	sp, 4
	mov	ax, [si].chStart
	mov	[si].chRestart, ax
	mov	ax, [si].chEnd
	mov	[si].chStop, ax
	mov	ax, [si].chStart
	mov	[si].chPos, ax
	xor	ax, ax
	mov	[si].chPosFrac, ax
	mov	al, CH_PLAYING
	mov	[si].chState, al
	sti
@@done:
	ret
DGTL_play	ENDP

; void DGTL_stop(int channel)
	PUBLIC	DGTL_stop
DGTL_stop	PROC	C FAR USES si, channel:WORD
	LOAD_CHANNEL	channel, @@done
	mov	[si].chState, CH_IDLE
@@done:
	ret
DGTL_stop	ENDP

; void DGTL_play_ambient(int channel, char far *sample, int length, int loopStart, int loopLength, int rate, int volume)
; Plays to the loop end, then repeats the loop until released.
	PUBLIC	DGTL_play_ambient
DGTL_play_ambient	PROC	C FAR USES si, channel:WORD, sample:DWORD, len:WORD, loopStart:WORD, loopLen:WORD, rate:WORD, volume:WORD
	LOAD_CHANNEL	channel, @@done
	cli
	mov	ax, word ptr sample
	mov	dx, word ptr sample + 2
	mov	[si].chSeg, dx
	mov	[si].chStart, ax
	mov	bx, loopStart
	add	bx, ax
	mov	[si].chLoopStart, bx
	add	bx, loopLen
	mov	[si].chLoopEnd, bx
	add	ax, len
	mov	[si].chEnd, ax
	mov	ax, loopLen
	mov	[si].chLoopLen, ax
	SET_STEP	rate
	push	volume
	push	channel
	push	cs
	call	near ptr DGTL_channel_volume
	add	sp, 4
	mov	ax, [si].chLoopStart
	mov	[si].chRestart, ax
	mov	ax, [si].chLoopEnd
	cmp	[si].chLoopLen, 0
	jne	@@stopSet
	mov	ax, [si].chEnd
@@stopSet:
	mov	[si].chStop, ax
	mov	ax, [si].chStart
	mov	[si].chPos, ax
	xor	ax, ax
	mov	[si].chPosFrac, ax
	mov	al, CH_PLAYING
	mov	[si].chState, al
	sti
@@done:
	ret
DGTL_play_ambient	ENDP

; void DGTL_release(int channel)
; Lets a looping channel play on to its end.
	PUBLIC	DGTL_release
DGTL_release	PROC	C FAR USES si, channel:WORD
	LOAD_CHANNEL	channel, @@done
	cli
	mov	ax, [si].chEnd
	mov	[si].chStop, ax
	mov	al, CH_RELEASED
	mov	[si].chState, al
	sti
@@done:
	ret
DGTL_release	ENDP

; void DGTL_set_voice(int channel, char far *sample, int length, int loopStart, int loopLength)
; Swaps in a new sample without moving the play position.
	PUBLIC	DGTL_set_voice
DGTL_set_voice	PROC	C FAR USES si, channel:WORD, sample:DWORD, len:WORD, loopStart:WORD, loopLen:WORD
	LOAD_CHANNEL	channel, @@done
	mov	ax, word ptr sample
	mov	dx, word ptr sample + 2
	mov	[si].chSeg, dx
	mov	[si].chStart, ax
	mov	bx, len
	add	bx, ax
	mov	[si].chEnd, bx
	mov	bx, loopStart
	add	bx, ax
	mov	[si].chLoopStart, bx
	add	bx, loopLen
	mov	[si].chLoopEnd, bx
	mov	ax, loopLen
	mov	[si].chLoopLen, ax
@@done:
	ret
DGTL_set_voice	ENDP

; Adds BLOCK_SAMPLES samples from es:si, scaled through the volume row at bx,
; into DACBuffer. si:cx is the position and di:dx the step; both advance.
MixBlock	PROC	NEAR
n = 0
	REPT	BLOCK_SAMPLES
	mov	al, es:[si]
	xlatb
	cbw
	add	DACBuffer[n], ax
	add	cx, dx
	adc	si, di
n = n + 2
	ENDM
	ret
MixBlock	ENDP

; MixBlock at full volume. Unreferenced.
MixBlockFull	PROC	NEAR
n = 0
	REPT	BLOCK_SAMPLES
	mov	al, es:[si]
	cbw
	add	DACBuffer[n], ax
	add	cx, dx
	adc	si, di
n = n + 2
	ENDM
	ret
MixBlockFull	ENDP

; Adds one playing channel into DACBuffer. A channel reaching chStop loops back
; to chRestart while looping, else goes idle and calls back with its number.
MIX_CHANNEL	MACRO	chan, number
	LOCAL	slow, run, sample, ended, stop, save, next
	cmp	chan.chState, CH_IDLE
	je	next
	; Mix the whole block at once when its last sample, pos + 128 * step - 1,
	; still lies before chStop.
	xor	bx, bx
	mov	dx, chan.chStep
	mov	ax, chan.chStepFrac
	REPT	7
	shl	ax, 1
	rcl	dx, 1
	rcl	bx, 1
	ENDM
	add	ax, 0FFFFh
	adc	dx, 0
	adc	bx, 0
	add	ax, chan.chPosFrac
	adc	dx, chan.chPos
	cmp	dx, chan.chStop
	jae	slow
	mov	dx, chan.chSeg
	mov	ax, chan.chPos
	mov	si, ax
	mov	blockStart, ax
	mov	es, dx
	mov	cx, chan.chPosFrac
	mov	di, chan.chStep
	mov	dx, chan.chStepFrac
	mov	bx, chan.chVolTable
	call	near ptr MixBlock
save:
	mov	chan.chPosFrac, cx
	sub	si, blockStart
	add	chan.chPos, si
	jmp	next

	; Otherwise one sample at a time, up to chStop.
slow:
	xor	ah, ah
	mov	mixIndex, ah
run:
	mov	dx, chan.chSeg
	mov	ax, chan.chPos
	mov	bx, chan.chStop
	sub	bx, ax
	mov	runLength, bx
	mov	si, ax
	mov	blockStart, ax
	add	ax, runLength
	mov	runEnd, ax
	mov	es, dx
	mov	cx, chan.chPosFrac
	mov	di, chan.chStep
	mov	dx, chan.chStepFrac
sample:
	mov	al, es:[si]
	mov	bx, chan.chVolTable
	xlatb
	mov	bl, mixIndex
	xor	bh, bh
	cbw
	shl	bx, 1
	add	DACBuffer[bx], ax
	inc	mixIndex
	add	cx, dx
	adc	si, di
	cmp	si, runEnd
	jae	ended
	cmp	mixIndex, BLOCK_SAMPLES
	jne	sample
	jmp	short save
ended:
	mov	al, chan.chState
	cmp	al, CH_PLAYING
	jne	stop
	mov	ax, chan.chLoopLen
	or	ax, ax
	je	stop
	mov	ax, chan.chRestart
	mov	dx, chan.chSeg
	mov	chan.chPosFrac, cx
	mov	chan.chPos, ax
	cmp	mixIndex, BLOCK_SAMPLES
	jne	run
	jmp	short next
stop:
	mov	al, CH_IDLE
	mov	chan.chState, al
	mov	ax, number
	push	ax
	call	far ptr digital_call_back
	add	sp, 2
next:
	ENDM

; void DRV_UpdateHardware(void)
; Mixes the next block and hands it to the card. With every channel idle it
; sends SILENT_BLOCKS more of silence, then pauses the DAC.
	PUBLIC	DRV_UpdateHardware
DRV_UpdateHardware	PROC	C FAR USES es ds si di
	LOCAL	spare:WORD
	cld
	mov	ax, ds
	mov	es, ax
	mov	al, chan0.chState
	add	al, chan1.chState
	add	al, chan2.chState
	add	al, chan3.chState
	jne	@@mix
	mov	ax, DAC_IsPaused
	or	ax, ax
	je	@@idle
	ret
@@idle:
	mov	ax, silentBlocks
	or	ax, ax
	js	@@pause
	dec	ax
	mov	silentBlocks, ax
	mov	cx, BLOCK_SAMPLES
	xor	ax, ax
	mov	di, offset DACBuffer
	cld
	rep	stosw
	mov	cx, BLOCK_SAMPLES
	mov	si, offset DACBuffer
	call	near ptr DAC_Output
	ret
@@pause:
	call	near ptr DAC_Pause
	ret

@@mix:
	mov	ax, SILENT_BLOCKS
	mov	silentBlocks, ax
	mov	ax, DAC_IsPaused
	or	ax, ax
	je	@@running
	push	es
	call	near ptr DAC_UnPause
	pop	es
@@running:
	mov	cx, BLOCK_SAMPLES
	xor	ax, ax
	mov	di, offset DACBuffer
	rep	stosw
	MIX_CHANNEL	chan0, 0
	MIX_CHANNEL	chan1, 1
	MIX_CHANNEL	chan2, 2
	MIX_CHANNEL	chan3, 3
	mov	cx, BLOCK_SAMPLES
	mov	si, offset DACBuffer
	call	near ptr DAC_Output
	ret
DRV_UpdateHardware	ENDP

	END
