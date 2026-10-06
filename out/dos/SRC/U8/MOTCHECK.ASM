; name: MOTCHECK
; MotionTracker::check, written in assembly (.386: ENTER frame, near Jcc).
; Tests the item's footprint along a move against the collision buffer.


	.MODEL	LARGE
	.386

; Arguments
tracker		EQU	dword ptr [bp + 6]	; MotionTracker *
item		EQU	word ptr [bp + 0Ah]
mflags		EQU	dword ptr [bp + 0Ch]	; MotionTrackerInternalFlags &

; MotionTrackerInternalFlags
MF_DX		EQU	0
MF_DY		EQU	2
MF_DZ		EQU	4
MF_TO_Z		EQU	0Fh
MF_NEEDSUPPORT	EQU	10h

; Locals
xFoot		EQU	word ptr [bp - 2]	; footprint from the type flags
yFoot		EQU	word ptr [bp - 4]
zFoot		EQU	word ptr [bp - 6]
moveX		EQU	word ptr [bp - 28h]	; copies of the move
moveY		EQU	word ptr [bp - 2Ah]
moveZ		EQU	word ptr [bp - 2Ch]
typeFlag	EQU	dword ptr [bp - 30h]	; first four bytes of the item's TypeFlag

	EXTRN	@CollisionBuffer@surfaceFlags$qm8TypeFlag:FAR
	EXTRN	@CollisionBuffer@volumeFlags$qm8TypeFlag:FAR
	EXTRN	@MotionTracker@collision$qussssuc:FAR

	.DATA

	EXTRN	@ItemData@statusArray:WORD
	EXTRN	@ItemData@typeArray:WORD
	EXTRN	@ItemData@xArray:WORD
	EXTRN	@ItemData@yArray:WORD
	EXTRN	@ItemData@zArray:WORD
	EXTRN	_GlobalTypes:BYTE


	.CODE

	PUBLIC	@MotionTracker@check$qusm26MotionTrackerInternalFlags
@MotionTracker@check$qusm26MotionTrackerInternalFlags	PROC	FAR
	enter	36h, 0
	push	si
	les	bx, mflags
	mov	ax, word ptr es:[bx + MF_DX]
	mov	moveX, ax
	mov	ax, word ptr es:[bx + MF_DY]
	mov	moveY, ax
	mov	ax, word ptr es:[bx + MF_DZ]
	mov	moveZ, ax
	mov	ax, item
	shl	ax, 1
	les	bx, dword ptr [@ItemData@typeArray]
	add	bx, ax
	mov	ax, word ptr es:[bx]
	shl	ax, 3
	les	bx, dword ptr [_GlobalTypes + 4]
	add	bx, ax
	mov	eax, dword ptr es:[bx]
	mov	typeFlag, eax
	mov	al, byte ptr [bp - 2Eh]
	and	ax, 0F0h
	add	ax, ax
	mov	xFoot, ax
	mov	al, byte ptr [bp - 2Dh]
	and	ax, 0Fh
	shl	ax, 5
	mov	yFoot, ax
	mov	al, byte ptr [bp - 2Dh]
	and	ax, 0F0h
	shr	ax, 1
	mov	zFoot, ax
	or	ax, ax
	jne	L_006C
	test	byte ptr [bp - 30h], 1
	jne	L_006C
	mov	zFoot, 1
L_006C:
	mov	si, item
	add	si, si
	les	bx, dword ptr [@ItemData@statusArray]
	test	word ptr es:[bx + si], 20h
	je	L_0088
	mov	ax, xFoot
	mov	bx, yFoot
	mov	xFoot, bx
	mov	yFoot, ax
L_0088:
	les	bx, mflags
	cmp	byte ptr es:[bx + MF_NEEDSUPPORT], 0
	je	L_01E2
	mov	al, byte ptr es:[bx + MF_TO_Z]
	mov	ah, 0
	les	bx, dword ptr [@ItemData@zArray]
	add	bx, item
	mov	dl, byte ptr es:[bx]
	mov	dh, 0
	add	dx, zFoot
	sub	ax, dx
	mov	word ptr [bp - 36h], ax
	cmp	word ptr [bp - 36h], 0
	jl	L_01E2
	les	bx, tracker
	mov	ax, word ptr es:[bx + 1Ch]
	cmp	ax, word ptr [bp - 36h]
	jl	L_01E2
	mov	si, item
	add	si, si
	les	bx, dword ptr [@ItemData@xArray]
	mov	ax, word ptr es:[bx + si]
	sub	ax, xFoot
	inc	ax
	cwd
	mov	word ptr [bp - 8], dx
	mov	word ptr [bp - 0Ah], ax
	mov	si, item
	add	si, si
	les	bx, dword ptr [@ItemData@yArray]
	mov	ax, word ptr es:[bx + si]
	sub	ax, yFoot
	inc	ax
	cwd
	mov	word ptr [bp - 0Ch], dx
	mov	word ptr [bp - 0Eh], ax
	mov	si, item
	add	si, si
	les	bx, dword ptr [@ItemData@xArray]
	mov	ax, word ptr es:[bx + si]
	cwd
	mov	word ptr [bp - 10h], dx
	mov	word ptr [bp - 12h], ax
	mov	si, item
	add	si, si
	les	bx, dword ptr [@ItemData@yArray]
	mov	ax, word ptr es:[bx + si]
	cwd
	mov	word ptr [bp - 14h], dx
	mov	word ptr [bp - 16h], ax
	les	si, mflags
	mov	ax, word ptr es:[si + 0Bh]
	mov	bx, ax
	les	si, tracker
	sub	ax, word ptr es:[si + 4]
	inc	ax
	xor	dx, dx
	cmp	dx, word ptr [bp - 10h]
	jg	L_01E2
	jne	L_013B
	cmp	ax, word ptr [bp - 12h]
	ja	L_01E2
L_013B:
	les	si, mflags
	mov	ax, word ptr es:[si + 0Dh]
	mov	cx, ax
	les	si, tracker
	sub	ax, word ptr es:[si + 6]
	inc	ax
	xor	dx, dx
	cmp	dx, word ptr [bp - 14h]
	jg	L_01E2
	jne	L_015E
	cmp	ax, word ptr [bp - 16h]
	ja	L_01E2
L_015E:
	mov	ax, bx
	xor	dx, dx
	cmp	dx, word ptr [bp - 8]
	jl	L_01E2
	jne	L_016E
	cmp	ax, word ptr [bp - 0Ah]
	jb	L_01E2
L_016E:
	mov	ax, cx
	xor	dx, dx
	cmp	dx, word ptr [bp - 0Ch]
	jl	L_01E2
	jne	L_017E
	cmp	ax, word ptr [bp - 0Eh]
	jb	L_01E2
L_017E:
	push	ss
	lea	ax, [bp - 30h]
	push	ax
	call	far ptr @CollisionBuffer@surfaceFlags$qm8TypeFlag
	add	sp, 4
	mov	word ptr [bp - 20h], ax
	les	bx, tracker
	test	word ptr es:[bx + 2], ax
	je	L_01A1
	les	bx, mflags
	mov	byte ptr es:[bx + 11h], 1
	jmp	short L_01E2
L_01A1:
	test	byte ptr [bp - 30h], 2
	je	L_01E2
	les	bx, tracker
	mov	ax, word ptr es:[bx + 2]
	not	ax
	test	word ptr [bp - 20h], ax
	je	L_01E2
	les	bx, mflags
	mov	byte ptr es:[bx + 12h], 1
	les	bx, tracker
	mov	ax, word ptr [bp - 36h]
	mov	word ptr es:[bx + 1Ch], ax
	or	ax, ax
	jne	L_01E2
	push	0
	push	0
	push	0
	push	0
	push	item
	push	word ptr tracker + 2
	push	bx
	call	far ptr @MotionTracker@collision$qussssuc
	add	sp, 0Eh
L_01E2:
	push	ss
	lea	ax, [bp - 30h]
	push	ax
	call	far ptr @CollisionBuffer@volumeFlags$qm8TypeFlag
	add	sp, 4
	les	bx, tracker
	and	al, byte ptr es:[bx]
	mov	byte ptr [bp - 24h], al
	test	byte ptr [bp - 30h], 40h
	jne	L_0208
	les	bx, mflags
	cmp	byte ptr es:[bx + 13h], 0
	je	L_020D
L_0208:
	mov	ax, 1
	jmp	short L_020F
L_020D:
	xor	ax, ax
L_020F:
	mov	byte ptr [bp - 26h], al
	cmp	byte ptr [bp - 24h], 0
	jne	L_021E
	or	al, al
	je	L_07BE
L_021E:
	mov	si, item
	add	si, si
	les	bx, dword ptr [@ItemData@xArray]
	mov	ax, word ptr es:[bx + si]
	sub	ax, xFoot
	inc	ax
	les	bx, mflags
	sub	ax, word ptr es:[bx + 6]
	cwd
	mov	word ptr [bp - 8], dx
	mov	word ptr [bp - 0Ah], ax
	mov	si, item
	add	si, si
	les	bx, dword ptr [@ItemData@yArray]
	mov	ax, word ptr es:[bx + si]
	sub	ax, yFoot
	inc	ax
	les	bx, mflags
	sub	ax, word ptr es:[bx + 8]
	cwd
	mov	word ptr [bp - 0Ch], dx
	mov	word ptr [bp - 0Eh], ax
	mov	si, item
	add	si, si
	les	bx, dword ptr [@ItemData@xArray]
	mov	ax, word ptr es:[bx + si]
	les	bx, tracker
	add	ax, word ptr es:[bx + 4]
	dec	ax
	les	bx, mflags
	sub	ax, word ptr es:[bx + 6]
	cwd
	mov	word ptr [bp - 10h], dx
	mov	word ptr [bp - 12h], ax
	mov	si, item
	add	si, si
	les	bx, dword ptr [@ItemData@yArray]
	mov	ax, word ptr es:[bx + si]
	les	bx, tracker
	add	ax, word ptr es:[bx + 6]
	dec	ax
	les	bx, mflags
	sub	ax, word ptr es:[bx + 8]
	cwd
	mov	word ptr [bp - 14h], dx
	mov	word ptr [bp - 16h], ax
	les	bx, dword ptr [@ItemData@zArray]
	add	bx, item
	mov	al, byte ptr es:[bx]
	mov	ah, 0
	mov	word ptr [bp - 32h], ax
	add	ax, zFoot
	dec	ax
	les	bx, mflags
	mov	dl, byte ptr es:[bx + 0Ah]
	mov	dh, 0
	mov	word ptr [bp - 34h], dx
	sub	ax, dx
	cwd
	mov	word ptr [bp - 1Ch], dx
	mov	word ptr [bp - 1Eh], ax
	mov	ax, word ptr [bp - 32h]
	les	bx, tracker
	sub	ax, word ptr es:[bx + 8]
	inc	ax
	sub	ax, word ptr [bp - 34h]
	cwd
	mov	word ptr [bp - 18h], dx
	mov	word ptr [bp - 1Ah], ax
	cmp	moveX, 0
	je	L_03AC
	mov	ax, moveX
	cwd
	cmp	dx, word ptr [bp - 8]
	jl	L_03AC
	jne	L_02F7
	cmp	ax, word ptr [bp - 0Ah]
	jb	L_03AC
L_02F7:
	cmp	word ptr [bp - 8], 0
	jl	L_03AC
	jg	L_0309
	cmp	word ptr [bp - 0Ah], 0
	jbe	L_03AC
L_0309:
	mov	ax, moveY
	imul	word ptr [bp - 0Ah]
	mov	cx, dx
	mov	bx, ax
	mov	ax, moveX
	imul	word ptr [bp - 16h]
	cmp	dx, cx
	jl	L_03AC
	jne	L_0327
	cmp	ax, bx
	jb	L_03AC
L_0327:
	mov	ax, moveX
	imul	word ptr [bp - 0Eh]
	cmp	dx, cx
	jg	L_03AC
	jne	L_0337
	cmp	ax, bx
	ja	L_03AC
L_0337:
	mov	ax, moveZ
	imul	word ptr [bp - 0Ah]
	mov	cx, dx
	mov	bx, ax
	mov	ax, moveX
	imul	word ptr [bp - 1Eh]
	cmp	dx, cx
	jl	L_03AC
	jne	L_0351
	cmp	ax, bx
	jb	L_03AC
L_0351:
	mov	ax, moveX
	imul	word ptr [bp - 1Ah]
	cmp	dx, cx
	jg	L_03AC
	jne	L_0361
	cmp	ax, bx
	ja	L_03AC
L_0361:
	cmp	byte ptr [bp - 24h], 0
	je	L_0377
	les	bx, mflags
	mov	byte ptr es:[bx + 14h], 0
	les	bx, tracker
	or	word ptr es:[bx + 23h], 1
L_0377:
	sub	word ptr [bp - 0Ah], 1
	sbb	word ptr [bp - 8], 0
	mov	al, byte ptr [bp - 24h]
	push	ax
	mov	ax, moveZ
	imul	word ptr [bp - 0Ah]
	idiv	moveX
	push	ax
	mov	ax, moveY
	imul	word ptr [bp - 0Ah]
	idiv	moveX
	push	ax
	push	word ptr [bp - 0Ah]
	push	item
	push	tracker
	call	far ptr @MotionTracker@collision$qussssuc
	add	sp, 0Eh
	jmp	L_07BE
L_03AC:
	cmp	moveY, 0
	je	L_047D
	mov	ax, moveY
	cwd
	cmp	dx, word ptr [bp - 0Ch]
	jl	L_047D
	jne	L_03C8
	cmp	ax, word ptr [bp - 0Eh]
	jb	L_047D
L_03C8:
	cmp	word ptr [bp - 0Ch], 0
	jl	L_047D
	jg	L_03DA
	cmp	word ptr [bp - 0Eh], 0
	jbe	L_047D
L_03DA:
	mov	ax, moveX
	imul	word ptr [bp - 0Eh]
	mov	cx, dx
	mov	bx, ax
	mov	ax, moveY
	imul	word ptr [bp - 12h]
	cmp	dx, cx
	jl	L_047D
	jne	L_03F8
	cmp	ax, bx
	jb	L_047D
L_03F8:
	mov	ax, moveY
	imul	word ptr [bp - 0Ah]
	cmp	dx, cx
	jg	L_047D
	jne	L_0408
	cmp	ax, bx
	ja	L_047D
L_0408:
	mov	ax, moveZ
	imul	word ptr [bp - 0Eh]
	mov	cx, dx
	mov	bx, ax
	mov	ax, moveY
	imul	word ptr [bp - 1Eh]
	cmp	dx, cx
	jl	L_047D
	jne	L_0422
	cmp	ax, bx
	jb	L_047D
L_0422:
	mov	ax, moveY
	imul	word ptr [bp - 1Ah]
	cmp	dx, cx
	jg	L_047D
	jne	L_0432
	cmp	ax, bx
	ja	L_047D
L_0432:
	cmp	byte ptr [bp - 24h], 0
	je	L_0448
	les	bx, mflags
	mov	byte ptr es:[bx + 14h], 0
	les	bx, tracker
	or	word ptr es:[bx + 23h], 2
L_0448:
	sub	word ptr [bp - 0Eh], 1
	sbb	word ptr [bp - 0Ch], 0
	mov	al, byte ptr [bp - 24h]
	push	ax
	mov	ax, moveZ
	imul	word ptr [bp - 0Eh]
	idiv	moveY
	push	ax
	push	word ptr [bp - 0Eh]
	mov	ax, moveX
	imul	word ptr [bp - 0Eh]
	idiv	moveY
	push	ax
	push	item
	push	tracker
	call	far ptr @MotionTracker@collision$qussssuc
	add	sp, 0Eh
	jmp	L_07BE
L_047D:
	cmp	moveZ, 0
	je	L_054E
	mov	ax, moveZ
	cwd
	cmp	dx, word ptr [bp - 18h]
	jl	L_054E
	jne	L_0499
	cmp	ax, word ptr [bp - 1Ah]
	jb	L_054E
L_0499:
	cmp	word ptr [bp - 18h], 0
	jl	L_054E
	jg	L_04AB
	cmp	word ptr [bp - 1Ah], 0
	jbe	L_054E
L_04AB:
	mov	ax, moveX
	imul	word ptr [bp - 1Ah]
	mov	cx, dx
	mov	bx, ax
	mov	ax, moveZ
	imul	word ptr [bp - 12h]
	cmp	dx, cx
	jl	L_054E
	jne	L_04C9
	cmp	ax, bx
	jb	L_054E
L_04C9:
	mov	ax, moveZ
	imul	word ptr [bp - 0Ah]
	cmp	dx, cx
	jg	L_054E
	jne	L_04D9
	cmp	ax, bx
	ja	L_054E
L_04D9:
	mov	ax, moveY
	imul	word ptr [bp - 1Ah]
	mov	cx, dx
	mov	bx, ax
	mov	ax, moveZ
	imul	word ptr [bp - 16h]
	cmp	dx, cx
	jl	L_054E
	jne	L_04F3
	cmp	ax, bx
	jb	L_054E
L_04F3:
	mov	ax, moveZ
	imul	word ptr [bp - 0Eh]
	cmp	dx, cx
	jg	L_054E
	jne	L_0503
	cmp	ax, bx
	ja	L_054E
L_0503:
	cmp	byte ptr [bp - 24h], 0
	je	L_0519
	les	bx, mflags
	mov	byte ptr es:[bx + 14h], 0
	les	bx, tracker
	or	word ptr es:[bx + 23h], 4
L_0519:
	sub	word ptr [bp - 1Ah], 1
	sbb	word ptr [bp - 18h], 0
	mov	al, byte ptr [bp - 24h]
	push	ax
	push	word ptr [bp - 1Ah]
	mov	ax, moveY
	imul	word ptr [bp - 1Ah]
	idiv	moveZ
	push	ax
	mov	ax, moveX
	imul	word ptr [bp - 1Ah]
	idiv	moveZ
	push	ax
	push	item
	push	tracker
	call	far ptr @MotionTracker@collision$qussssuc
	add	sp, 0Eh
	jmp	L_07BE
L_054E:
	cmp	moveX, 0
	je	L_061F
	mov	ax, moveX
	cwd
	cmp	dx, word ptr [bp - 10h]
	jg	L_061F
	jne	L_056A
	cmp	ax, word ptr [bp - 12h]
	ja	L_061F
L_056A:
	cmp	word ptr [bp - 10h], 0
	jg	L_061F
	jl	L_057C
	cmp	word ptr [bp - 12h], 0
	jae	L_061F
L_057C:
	mov	ax, moveY
	imul	word ptr [bp - 12h]
	mov	cx, dx
	mov	bx, ax
	mov	ax, moveX
	imul	word ptr [bp - 16h]
	cmp	dx, cx
	jg	L_061F
	jne	L_059A
	cmp	ax, bx
	ja	L_061F
L_059A:
	mov	ax, moveX
	imul	word ptr [bp - 0Eh]
	cmp	dx, cx
	jl	L_061F
	jne	L_05AA
	cmp	ax, bx
	jb	L_061F
L_05AA:
	mov	ax, moveZ
	imul	word ptr [bp - 12h]
	mov	cx, dx
	mov	bx, ax
	mov	ax, moveX
	imul	word ptr [bp - 1Eh]
	cmp	dx, cx
	jg	L_061F
	jne	L_05C4
	cmp	ax, bx
	ja	L_061F
L_05C4:
	mov	ax, moveX
	imul	word ptr [bp - 1Ah]
	cmp	dx, cx
	jl	L_061F
	jne	L_05D4
	cmp	ax, bx
	jb	L_061F
L_05D4:
	cmp	byte ptr [bp - 24h], 0
	je	L_05EA
	les	bx, mflags
	mov	byte ptr es:[bx + 14h], 0
	les	bx, tracker
	or	word ptr es:[bx + 23h], 1
L_05EA:
	add	word ptr [bp - 12h], 1
	adc	word ptr [bp - 10h], 0
	mov	al, byte ptr [bp - 24h]
	push	ax
	mov	ax, moveZ
	imul	word ptr [bp - 12h]
	idiv	moveX
	push	ax
	mov	ax, moveY
	imul	word ptr [bp - 12h]
	idiv	moveX
	push	ax
	push	word ptr [bp - 12h]
	push	item
	push	tracker
	call	far ptr @MotionTracker@collision$qussssuc
	add	sp, 0Eh
	jmp	L_07BE
L_061F:
	cmp	moveY, 0
	je	L_06F0
	mov	ax, moveY
	cwd
	cmp	dx, word ptr [bp - 14h]
	jg	L_06F0
	jne	L_063B
	cmp	ax, word ptr [bp - 16h]
	ja	L_06F0
L_063B:
	cmp	word ptr [bp - 14h], 0
	jg	L_06F0
	jl	L_064D
	cmp	word ptr [bp - 16h], 0
	jae	L_06F0
L_064D:
	mov	ax, moveX
	imul	word ptr [bp - 16h]
	mov	cx, dx
	mov	bx, ax
	mov	ax, moveY
	imul	word ptr [bp - 12h]
	cmp	dx, cx
	jg	L_06F0
	jne	L_066B
	cmp	ax, bx
	ja	L_06F0
L_066B:
	mov	ax, moveY
	imul	word ptr [bp - 0Ah]
	cmp	dx, cx
	jl	L_06F0
	jne	L_067B
	cmp	ax, bx
	jb	L_06F0
L_067B:
	mov	ax, moveZ
	imul	word ptr [bp - 16h]
	mov	cx, dx
	mov	bx, ax
	mov	ax, moveY
	imul	word ptr [bp - 1Eh]
	cmp	dx, cx
	jg	L_06F0
	jne	L_0695
	cmp	ax, bx
	ja	L_06F0
L_0695:
	mov	ax, moveY
	imul	word ptr [bp - 1Ah]
	cmp	dx, cx
	jl	L_06F0
	jne	L_06A5
	cmp	ax, bx
	jb	L_06F0
L_06A5:
	cmp	byte ptr [bp - 24h], 0
	je	L_06BB
	les	bx, mflags
	mov	byte ptr es:[bx + 14h], 0
	les	bx, tracker
	or	word ptr es:[bx + 23h], 2
L_06BB:
	add	word ptr [bp - 16h], 1
	adc	word ptr [bp - 14h], 0
	mov	al, byte ptr [bp - 24h]
	push	ax
	mov	ax, moveZ
	imul	word ptr [bp - 16h]
	idiv	moveY
	push	ax
	push	word ptr [bp - 16h]
	mov	ax, moveX
	imul	word ptr [bp - 16h]
	idiv	moveY
	push	ax
	push	item
	push	tracker
	call	far ptr @MotionTracker@collision$qussssuc
	add	sp, 0Eh
	jmp	L_07BE
L_06F0:
	cmp	moveZ, 0
	je	L_07BE
	mov	ax, moveZ
	cwd
	cmp	dx, word ptr [bp - 1Ch]
	jg	L_07BE
	jne	L_070C
	cmp	ax, word ptr [bp - 1Eh]
	ja	L_07BE
L_070C:
	cmp	word ptr [bp - 1Ch], 0
	jg	L_07BE
	jl	L_071E
	cmp	word ptr [bp - 1Eh], 0
	jae	L_07BE
L_071E:
	mov	ax, moveX
	imul	word ptr [bp - 1Eh]
	mov	cx, dx
	mov	bx, ax
	mov	ax, moveZ
	imul	word ptr [bp - 12h]
	cmp	dx, cx
	jg	L_07BE
	jne	L_073C
	cmp	ax, bx
	ja	L_07BE
L_073C:
	mov	ax, moveZ
	imul	word ptr [bp - 0Ah]
	cmp	dx, cx
	jl	L_07BE
	jne	L_074C
	cmp	ax, bx
	jb	L_07BE
L_074C:
	mov	ax, moveY
	imul	word ptr [bp - 1Eh]
	mov	cx, dx
	mov	bx, ax
	mov	ax, moveZ
	imul	word ptr [bp - 16h]
	cmp	dx, cx
	jg	L_07BE
	jne	L_0766
	cmp	ax, bx
	ja	L_07BE
L_0766:
	mov	ax, moveZ
	imul	word ptr [bp - 0Eh]
	cmp	dx, cx
	jl	L_07BE
	jne	L_0776
	cmp	ax, bx
	jb	L_07BE
L_0776:
	cmp	byte ptr [bp - 24h], 0
	je	L_078C
	les	bx, mflags
	mov	byte ptr es:[bx + 14h], 0
	les	bx, tracker
	or	word ptr es:[bx + 23h], 4
L_078C:
	add	word ptr [bp - 1Eh], 1
	adc	word ptr [bp - 1Ch], 0
	mov	al, byte ptr [bp - 24h]
	push	ax
	push	word ptr [bp - 1Eh]
	mov	ax, moveY
	imul	word ptr [bp - 1Eh]
	idiv	moveZ
	push	ax
	mov	ax, moveX
	imul	word ptr [bp - 1Eh]
	idiv	moveZ
	push	ax
	push	item
	push	tracker
	call	far ptr @MotionTracker@collision$qussssuc
	add	sp, 0Eh
L_07BE:
	pop	si
	leave
	retf
@MotionTracker@check$qusm26MotionTrackerInternalFlags	ENDP

	END
