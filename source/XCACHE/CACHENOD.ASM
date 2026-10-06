; name: ..\XCACHE\CACHENOD
; CacheNodePtr's linked-list and header field code, written in assembly (.386 flat
; offsets into the cache memory block at CacheNodePtr::memBase).


	.MODEL	LARGE
	.386

; Arguments: the CacheNodePtr (a far pointer to its 32-bit offset), then the second argument.
node		EQU	dword ptr [bp + 6]
arg		EQU	dword ptr [bp + 0Ah]

; CacheNode, the 29-byte header in front of every cache node
CN_TYPE		EQU	0
CN_HANDLE	EQU	2
CN_SIZE		EQU	4
CN_SIZEDIFF	EQU	8
CN_SUM		EQU	9
CN_NEXT		EQU	0Dh	; block thread, in address order
CN_PREV		EQU	11h
CN_LESS		EQU	15h	; most recently used thread
CN_MORE		EQU	19h

	.DATA

	EXTRN	@CacheNodePtr@freeBytes:WORD
	EXTRN	@CacheNodePtr@freeRoot:WORD
	EXTRN	@CacheNodePtr@largestFreeNode:WORD
	EXTRN	@CacheNodePtr@blockRoot:WORD
	EXTRN	@CacheNodePtr@cache:WORD

	PUBLIC	@CacheNodePtr@memBase
@CacheNodePtr@memBase	dd	0


	.CODE

	PUBLIC	@CacheNodePtr@freeSpaceToBlockMru$qv
@CacheNodePtr@freeSpaceToBlockMru$qv	PROC	FAR
	enter	0, 0
	push	ds
	push	es
	push	esi
	push	edi
	mov	ax, ds
	mov	es, ax
	lds	si, node
	mov	ebx, dword ptr [si]
	mov	dx, word ptr es:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	eax, dword ptr es:@CacheNodePtr@blockRoot
	mov	dword ptr [ebx + CN_MORE], eax
	mov	ecx, dword ptr [eax + CN_LESS]
	mov	dword ptr [ebx + CN_LESS], ecx
	mov	dword ptr [eax + CN_LESS], ebx
	mov	eax, dword ptr [ebx + CN_LESS]
	mov	dword ptr [eax + CN_MORE], ebx
	pop	edi
	pop	esi
	pop	es
	pop	ds
	leave
	retf
@CacheNodePtr@freeSpaceToBlockMru$qv	ENDP

	PUBLIC	@CacheNodePtr@makeMru$qv
@CacheNodePtr@makeMru$qv	PROC	FAR
	enter	0, 0
	push	ds
	push	es
	push	esi
	push	edi
	les	si, node
	mov	eax, dword ptr es:[si]
	push	node
	push	cs
	call	near ptr @CacheNodePtr@unlinkFromMruThread$qv
	add	sp, 4
	mov	cx, ds
	mov	es, cx
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ebx, dword ptr es:@CacheNodePtr@blockRoot
	mov	ecx, dword ptr [ebx + CN_LESS]
	mov	dword ptr [eax + CN_LESS], ecx
	mov	dword ptr [eax + CN_MORE], ebx
	mov	dword ptr [ebx + CN_LESS], eax
	mov	ecx, dword ptr [eax + CN_LESS]
	mov	dword ptr [ecx + CN_MORE], eax
	pop	edi
	pop	esi
	pop	es
	pop	ds
	leave
	retf
@CacheNodePtr@makeMru$qv	ENDP

	PUBLIC	@CacheNodePtr@unlinkFromMruThread$qv
@CacheNodePtr@unlinkFromMruThread$qv	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ebx, dword ptr [eax + CN_LESS]
	mov	ecx, dword ptr [eax + CN_MORE]
	mov	dword ptr [ecx + CN_LESS], ebx
	mov	ebx, dword ptr [eax + CN_MORE]
	mov	ecx, dword ptr [eax + CN_LESS]
	mov	dword ptr [ecx + CN_MORE], ebx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@unlinkFromMruThread$qv	ENDP

	PUBLIC	@CacheNodePtr@unlinkFromBlockThread$qv
@CacheNodePtr@unlinkFromBlockThread$qv	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ebx, dword ptr [eax + CN_NEXT]
	mov	ecx, dword ptr [eax + CN_PREV]
	mov	dword ptr [ecx + CN_NEXT], ebx
	mov	ebx, dword ptr [eax + CN_PREV]
	mov	ecx, dword ptr [eax + CN_NEXT]
	mov	dword ptr [ecx + CN_PREV], ebx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@unlinkFromBlockThread$qv	ENDP

	PUBLIC	@CacheNodePtr@blockThreadInsertAfter$q12CacheNodePtr
@CacheNodePtr@blockThreadInsertAfter$q12CacheNodePtr	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ebx, arg
	mov	ecx, dword ptr [ebx + CN_NEXT]
	mov	dword ptr [eax + CN_NEXT], ecx
	mov	dword ptr [eax + CN_PREV], ebx
	mov	dword ptr [ecx + CN_PREV], eax
	mov	dword ptr [ebx + CN_NEXT], eax
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@blockThreadInsertAfter$q12CacheNodePtr	ENDP

	PUBLIC	@CacheNodePtr@mruThreadReplace$q12CacheNodePtr
@CacheNodePtr@mruThreadReplace$q12CacheNodePtr	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ebx, arg
	mov	ecx, dword ptr [ebx + CN_LESS]
	mov	dword ptr [eax + CN_LESS], ecx
	mov	ecx, dword ptr [ebx + CN_MORE]
	mov	dword ptr [eax + CN_MORE], ecx
	mov	ecx, dword ptr [eax + CN_LESS]
	mov	dword ptr [ecx + CN_MORE], eax
	mov	ecx, dword ptr [eax + CN_MORE]
	mov	dword ptr [ecx + CN_LESS], eax
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@mruThreadReplace$q12CacheNodePtr	ENDP

	PUBLIC	@CacheNodePtr@addToFreeChain$qv
@CacheNodePtr@addToFreeChain$qv	PROC	FAR
	enter	0, 0
	push	ds
	push	es
	push	esi
	push	edi
	mov	ax, ds
	mov	es, ax
	lds	si, node
	mov	eax, dword ptr [si]
	mov	ebx, dword ptr es:@CacheNodePtr@freeRoot
	mov	dx, word ptr es:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ecx, dword ptr [ebx + CN_LESS]
	mov	dword ptr [eax + CN_LESS], ecx
	mov	dword ptr [eax + CN_MORE], ebx
	mov	ecx, dword ptr [ebx + CN_LESS]
	mov	dword ptr [ecx + CN_MORE], eax
	mov	dword ptr [ebx + CN_LESS], eax
	pop	edi
	pop	esi
	pop	es
	pop	ds
	leave
	retf
@CacheNodePtr@addToFreeChain$qv	ENDP

	PUBLIC	@CacheNodePtr@setMergeLink$q12CacheNodePtr
@CacheNodePtr@setMergeLink$q12CacheNodePtr	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ebx, arg
	mov	ecx, dword ptr [ebx + CN_NEXT]
	mov	dword ptr [eax + CN_NEXT], ecx
	mov	ecx, dword ptr [eax + CN_NEXT]
	mov	dword ptr [ecx + CN_PREV], eax
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@setMergeLink$q12CacheNodePtr	ENDP

	PUBLIC	@CacheNodePtr@reset$q12CacheNodePtr
@CacheNodePtr@reset$q12CacheNodePtr	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ebx, arg
	mov	ecx, dword ptr [eax + CN_PREV]
	mov	dword ptr [ecx + CN_NEXT], ebx
	mov	ecx, dword ptr [eax + CN_NEXT]
	mov	dword ptr [ecx + CN_PREV], ebx
	mov	ecx, dword ptr [eax + CN_LESS]
	mov	dword ptr [ecx + CN_MORE], ebx
	mov	ecx, dword ptr [eax + CN_MORE]
	mov	dword ptr [ecx + CN_LESS], ebx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@reset$q12CacheNodePtr	ENDP

	PUBLIC	@CacheNodePtr@shiftUpSet$q12CacheNodePtr
@CacheNodePtr@shiftUpSet$q12CacheNodePtr	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ebx, arg
	mov	ecx, dword ptr [eax + CN_PREV]
	mov	edx, dword ptr [ecx + CN_PREV]
	mov	dword ptr [edx + CN_NEXT], ebx
	mov	ecx, dword ptr [eax + CN_NEXT]
	mov	dword ptr [ecx + CN_PREV], ebx
	mov	ebx, dword ptr [eax + CN_PREV]
	mov	ecx, dword ptr [ebx + CN_MORE]
	mov	edx, dword ptr [ebx + CN_LESS]
	mov	dword ptr [edx + CN_MORE], ecx
	mov	ecx, dword ptr [ebx + CN_LESS]
	mov	edx, dword ptr [ebx + CN_MORE]
	mov	dword ptr [edx + CN_LESS], ecx
	mov	ebx, arg
	mov	ecx, dword ptr [eax + CN_LESS]
	mov	dword ptr [ecx + CN_MORE], ebx
	mov	ecx, dword ptr [eax + CN_MORE]
	mov	dword ptr [ecx + CN_LESS], ebx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@shiftUpSet$q12CacheNodePtr	ENDP

	PUBLIC	@CacheNodePtr@setDummyNodes$qv
@CacheNodePtr@setDummyNodes$qv	PROC	FAR
	enter	0, 0
	push	ds
	push	es
	push	esi
	push	edi
	mov	eax, dword ptr [@CacheNodePtr@cache]
	add	eax, 3Ah
	mov	ebx, dword ptr [@CacheNodePtr@blockRoot]
	mov	cx, ds
	mov	es, cx
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	dword ptr [ebx + CN_NEXT], eax
	mov	dword ptr [ebx + CN_PREV], eax
	mov	dword ptr [ebx + CN_LESS], ebx
	mov	dword ptr [ebx + CN_MORE], ebx
	mov	cx, 0FFh
	mov	word ptr [ebx + CN_TYPE], cx
	mov	ecx, 0FFFFh
	mov	word ptr [ebx + CN_HANDLE], cx
	xor	ecx, ecx
	mov	dword ptr [ebx + CN_SIZE], ecx
	mov	byte ptr [ebx + CN_SIZEDIFF], cl
	mov	ecx, dword ptr es:@CacheNodePtr@blockRoot
	mov	dword ptr [eax + CN_NEXT], ecx
	mov	dword ptr [eax + CN_PREV], ecx
	mov	ecx, dword ptr es:@CacheNodePtr@freeRoot
	mov	dword ptr [eax + CN_LESS], ecx
	mov	dword ptr [eax + CN_MORE], ecx
	mov	cx, 0FFh
	mov	word ptr [eax + CN_TYPE], cx
	mov	cx, 0FFFEh
	mov	word ptr [eax + CN_HANDLE], cx
	mov	ecx, dword ptr es:@CacheNodePtr@freeBytes
	mov	dword ptr [eax + CN_SIZE], ecx
	xor	cl, cl
	mov	byte ptr [eax + CN_SIZEDIFF], cl
	mov	ebx, dword ptr es:@CacheNodePtr@freeRoot
	mov	dword ptr [ebx + CN_NEXT], ebx
	mov	dword ptr [ebx + CN_PREV], ebx
	mov	dword ptr [ebx + CN_LESS], eax
	mov	dword ptr [ebx + CN_MORE], eax
	mov	cx, 0FFh
	mov	word ptr [ebx + CN_TYPE], cx
	mov	cx, 0FFFFh
	mov	word ptr [ebx + CN_HANDLE], cx
	xor	ecx, ecx
	mov	dword ptr [ebx + CN_SIZE], ecx
	mov	byte ptr [ebx + CN_SIZEDIFF], cl
	mov	dword ptr es:@CacheNodePtr@largestFreeNode, eax
	pop	edi
	pop	esi
	pop	es
	pop	ds
	leave
	retf
@CacheNodePtr@setDummyNodes$qv	ENDP

	PUBLIC	@CacheNodePtr@getSum$qv
@CacheNodePtr@getSum$qv	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	ebx, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	eax, dword ptr [ebx + CN_SUM]
	push	eax
	pop	ax
	pop	dx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@getSum$qv	ENDP

	PUBLIC	@CacheNodePtr@getSize$qv
@CacheNodePtr@getSize$qv	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	ebx, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	eax, dword ptr [ebx + CN_SIZE]
	push	eax
	pop	ax
	pop	dx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@getSize$qv	ENDP

	PUBLIC	@CacheNodePtr@getLess$qv
@CacheNodePtr@getLess$qv	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	ebx, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	eax, dword ptr [ebx + CN_LESS]
	push	eax
	pop	ax
	pop	dx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@getLess$qv	ENDP

	PUBLIC	@CacheNodePtr@getMore$qv
@CacheNodePtr@getMore$qv	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	ebx, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	eax, dword ptr [ebx + CN_MORE]
	push	eax
	pop	ax
	pop	dx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@getMore$qv	ENDP

	PUBLIC	@CacheNodePtr@getNext$qv
@CacheNodePtr@getNext$qv	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	ebx, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	eax, dword ptr [ebx + CN_NEXT]
	push	eax
	pop	ax
	pop	dx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@getNext$qv	ENDP

	PUBLIC	@CacheNodePtr@getPrev$qv
@CacheNodePtr@getPrev$qv	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	ebx, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	eax, dword ptr [ebx + CN_PREV]
	push	eax
	pop	ax
	pop	dx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@getPrev$qv	ENDP

	PUBLIC	@CacheNodePtr@getHandle$qv
@CacheNodePtr@getHandle$qv	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	ebx, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ax, word ptr [ebx + CN_HANDLE]
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@getHandle$qv	ENDP

	PUBLIC	@CacheNodePtr@getSizeDiff$qv
@CacheNodePtr@getSizeDiff$qv	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	ebx, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	movzx	ax, byte ptr [ebx + CN_SIZEDIFF]
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@getSizeDiff$qv	ENDP

	PUBLIC	@CacheNodePtr@getType$qv
@CacheNodePtr@getType$qv	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	ebx, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ax, word ptr [ebx + CN_TYPE]
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@getType$qv	ENDP

	PUBLIC	@CacheNodePtr@setSize$ql
@CacheNodePtr@setSize$ql	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ebx, arg
	mov	dword ptr [eax + CN_SIZE], ebx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@setSize$ql	ENDP

	PUBLIC	@CacheNodePtr@setSum$ql
@CacheNodePtr@setSum$ql	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ebx, arg
	mov	dword ptr [eax + CN_SUM], ebx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@setSum$ql	ENDP

	PUBLIC	@CacheNodePtr@setNext$q12CacheNodePtr
@CacheNodePtr@setNext$q12CacheNodePtr	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ebx, arg
	mov	dword ptr [eax + CN_NEXT], ebx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@setNext$q12CacheNodePtr	ENDP

	PUBLIC	@CacheNodePtr@setPrev$q12CacheNodePtr
@CacheNodePtr@setPrev$q12CacheNodePtr	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ebx, arg
	mov	dword ptr [eax + CN_PREV], ebx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@setPrev$q12CacheNodePtr	ENDP

	PUBLIC	@CacheNodePtr@setType$q13CacheNodeType
@CacheNodePtr@setType$q13CacheNodeType	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	bx, word ptr arg
	mov	word ptr [eax + CN_TYPE], bx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@setType$q13CacheNodeType	ENDP

	PUBLIC	@CacheNodePtr@setSizeDiff$quc
@CacheNodePtr@setSizeDiff$quc	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	bl, byte ptr arg
	mov	byte ptr [eax + CN_SIZEDIFF], bl
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@setSizeDiff$quc	ENDP

	PUBLIC	@CacheNodePtr@setHandle$qui
@CacheNodePtr@setHandle$qui	PROC	FAR
	enter	0, 0
	push	es
	push	ds
	push	si
	les	si, node
	mov	eax, dword ptr es:[si]
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	bx, word ptr arg
	mov	word ptr [eax + CN_HANDLE], bx
	pop	si
	pop	ds
	pop	es
	leave
	retf
@CacheNodePtr@setHandle$qui	ENDP

	PUBLIC	@CacheNodePtr@shiftUpAsm$ql
@CacheNodePtr@shiftUpAsm$ql	PROC	FAR
	enter	0, 0
	push	ds
	push	es
	push	esi
	push	edi
	les	si, node
	mov	ebx, dword ptr es:[si]
	mov	ecx, ebx
	sub	ecx, arg
	mov	ax, ds
	mov	es, ax
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	movzx	eax, word ptr [ebx + CN_HANDLE]
	cmp	ax, 0FFFDh
	jae	L_05C9
	push	ecx
	add	ecx, 1Dh
	xor	edi, edi
	mov	di, 2A40h
	mov	dword ptr es:[edi + eax*4], ecx
	pop	ecx
L_05C9:
	push	es
	push	ds
	pop	es
	pop	ds
	push	ebx
	push	ecx
	push	ecx
	push	node
	push	cs
	call	near ptr @CacheNodePtr@reset$q12CacheNodePtr
	add	sp, 8
	pop	ecx
	pop	ebx
	push	es
	push	ds
	pop	es
	pop	ds
	mov	eax, dword ptr [ebx + CN_SIZE]
	xchg	eax, ecx
	mov	dx, ds
	mov	es, dx
	push	ecx
	shr	ecx, 1
	mov	edi, eax
	mov	esi, ebx
	rep movs	word ptr es:[edi], word ptr [esi]
	pop	ecx
	push	ecx
	and	ecx, 1
	rep movs	byte ptr es:[edi], byte ptr [esi]
	pop	ecx
	mov	ax, word ptr [eax + CN_TYPE]
	pop	edi
	pop	esi
	pop	es
	pop	ds
	leave
	retf
@CacheNodePtr@shiftUpAsm$ql	ENDP

	PUBLIC	@CacheNodePtr@shiftFreeBlock$ql
@CacheNodePtr@shiftFreeBlock$ql	PROC	FAR
	enter	0, 0
	push	ds
	push	es
	push	esi
	push	edi
	les	si, node
	mov	ebx, dword ptr es:[si]
	mov	eax, ebx
	sub	eax, arg
	push	eax
	push	ebx
	push	eax
	push	node
	push	cs
	call	near ptr @CacheNodePtr@reset$q12CacheNodePtr
	add	sp, 8
	pop	ebx
	pop	eax
	mov	dx, word ptr ds:@CacheNodePtr@memBase + 2
	mov	ds, dx
	mov	ecx, dword ptr [ebx + CN_SIZE]
	add	ecx, arg
	mov	dx, ds
	mov	es, dx
	push	ecx
	mov	ecx, 1Dh
	mov	edi, eax
	mov	esi, ebx
	rep movs	byte ptr es:[edi], byte ptr [esi]
	pop	ecx
	mov	dword ptr [eax + CN_SIZE], ecx
	pop	edi
	pop	esi
	pop	es
	pop	ds
	leave
	retf
@CacheNodePtr@shiftFreeBlock$ql	ENDP

	END
