; name: DLASM
; flags: /ml /m5 /zd
; Display list queues, drawing, new-item checks and depth sorting.

	.MODEL	LARGE
	.386
	LOCALS
	INCLUDE	DLIST.INC

VT_DRAW		EQU	4	; DisplayListNode::draw in the vtable

SCREEN_W	EQU	320
SCREEN_H	EQU	200

NEWITEM_BYTES	EQU	480h	; one bit per item slot

; The screen is covered by a grid of shadow square nodes.
SQUARE_SHIFT	EQU	6	; 64 pixels a side
SQUARE_COLS	EQU	6
SQUARE_ROWS	EQU	5
SQUARE_ROW_BYTES EQU	SQUARE_COLS * 2

; DL_ItemNode::compare results
CMP_APART	EQU	0	; no overlap on screen
CMP_OVER	EQU	1	; this is drawn over the other node
CMP_UNDER	EQU	2

	EXTRN	@DisplayList@draw$qus:FAR
	EXTRN	@DisplayList@sort$qusus:FAR
	EXTRN	@trace$q10TraceLevelnzce:FAR

	.DATA

	EXTRN	@BaseCamera@needSlam:WORD
	EXTRN	@Camera@_h:WORD
	EXTRN	@Camera@_v:WORD
	EXTRN	@DisplayList@_drawing_queue:WORD
	EXTRN	@DisplayList@_waiting_queue:WORD
	EXTRN	@DisplayList@animating:WORD
	EXTRN	@DisplayList@animatingitem:WORD
	EXTRN	@DisplayList@clip_b:WORD
	EXTRN	@DisplayList@clip_l:WORD
	EXTRN	@DisplayList@clip_r:WORD
	EXTRN	@DisplayList@clip_t:WORD
	EXTRN	@DisplayList@newitem:WORD
	EXTRN	@ItemData@statusArray:WORD
	EXTRN	@ItemData@typeArray:WORD
	EXTRN	@ItemData@xArray:WORD
	EXTRN	@ItemData@yArray:WORD
	EXTRN	@ItemData@zArray:WORD
	EXTRN	_DisplayListEdgeSeg:WORD
	EXTRN	_DisplayListNodeSeg:WORD
	EXTRN	_GlobalTypes:WORD
	EXTRN	_ShadowSquare:WORD
	EXTRN	_ShadowSquareLC:WORD
	EXTRN	_ShadowSquareTC:WORD

dlLoopMsg	db	'dl loop ', 0
	db	0

	.CODE

; unsigned DisplayListQueue::remove_head(void)
; Unlinks the first node and returns it, or 0 when the queue is empty.
	PUBLIC	@DisplayListQueue@remove_head$qv
@DisplayListQueue@remove_head$qv	PROC	CPP FAR USES ds si, queue:DWORD
	LOCAL	unused:DWORD
	les	bx, queue
	mov	si, es:[bx].dlqHead
	or	si, si
	je	@@done
	mov	ds, [_DisplayListNodeSeg]
	mov	ax, [si].dlnNext
	mov	es:[bx].dlqHead, ax
	or	ax, ax
	jne	@@newHead
	mov	es:[bx].dlqTail, ax
	jmp	short @@done
@@newHead:
	mov	bx, ax
	mov	[bx].dlnPrev, 0
@@done:
	mov	ax, si
	ret
@DisplayListQueue@remove_head$qv	ENDP

; void DisplayListQueue::add_tail(unsigned short node)
	PUBLIC	@DisplayListQueue@add_tail$qus
@DisplayListQueue@add_tail$qus	PROC	CPP FAR USES ds si, queue:DWORD, node:WORD
	mov	si, node
	or	si, si
	je	@@done
	les	bx, queue
	mov	cx, es:[bx].dlqTail
	mov	ds, [_DisplayListNodeSeg]
	mov	[si].dlnPrev, cx
	mov	[si].dlnNext, 0
	jcxz	@@noTail
	xchg	bx, cx
	mov	[bx].dlnNext, si
	xchg	bx, cx
@@noTail:
	cmp	es:[bx].dlqHead, 0
	jne	@@setTail
	mov	es:[bx].dlqHead, si
@@setTail:
	mov	es:[bx].dlqTail, si
@@done:
	ret
@DisplayListQueue@add_tail$qus	ENDP

; static void DisplayList::draw(void)
; Moves each waiting node to the drawing queue once nothing below it is still
; waiting, then draws the drawing queue in order. Nodes off screen are dropped.
; While animating, the drawing queue goes back to wait and the pass repeats.
	PUBLIC	@DisplayList@draw$qv
@DisplayList@draw$qv	PROC	CPP FAR USES si di
	LOCAL	left:WORD, top:WORD, right:WORD, bottom:WORD, unused:DWORD, stuck:WORD
	mov	byte ptr [@BaseCamera@needSlam], 1
	mov	byte ptr stuck, 0
	mov	ax, [@DisplayList@clip_l]
	add	ax, [@Camera@_h]
	mov	left, ax
	mov	ax, [@DisplayList@clip_t]
	add	ax, [@Camera@_v]
	mov	top, ax
	mov	ax, [@DisplayList@clip_r]
	add	ax, [@Camera@_h]
	mov	right, ax
	mov	ax, [@DisplayList@clip_b]
	add	ax, [@Camera@_v]
	mov	bottom, ax

	; di is the first node put back since one was last drawn; meeting it
	; again means the nodes below each other form a loop.
	xor	di, di
@@nextWaiting:
	cmp	[@DisplayList@_waiting_queue].dlqHead, 0
	je	@@nextDrawing
	mov	si, [@DisplayList@_waiting_queue].dlqHead
	mov	es, [_DisplayListNodeSeg]
	mov	ax, es:[si].dlnNext
	mov	[@DisplayList@_waiting_queue].dlqHead, ax
	or	ax, ax
	jne	@@waitingLeft
	mov	[@DisplayList@_waiting_queue].dlqTail, ax
	jmp	short @@popped
@@waitingLeft:
	mov	bx, ax
	mov	es:[bx].dlnPrev, 0
@@popped:
	cmp	di, si
	je	@@loopFound

	; Wait while any node below (other than a shadow square) is still waiting.
	mov	ax, ds
	mov	cx, si
	mov	dh, es:[si].dlnFlags
	mov	bx, es:[si].dlnBelow
	mov	ds, [_DisplayListEdgeSeg]
	or	bx, bx
	je	@@belowDone
@@checkBelow:
	mov	si, [bx].dleLower
	mov	dl, es:[si].dlnFlags
	test	dl, DLN_WAITING
	je	@@nextBelow
	test	dl, DLN_SHADOW
	jne	@@nextBelow
	test	dh, DLN_DRAWING
	jne	@@belowDone
	mov	si, cx
	or	es:[si].dlnFlags, DLN_WAITING
	mov	ds, ax
	jmp	short @@putBack
@@nextBelow:
	mov	bx, [bx].dleNextBelow
	or	bx, bx
	jne	@@checkBelow
@@belowDone:
	mov	si, cx
	mov	ds, ax
	cmp	di, si
	jne	@@ready
@@loopFound:
	mov	byte ptr stuck, 1
@@ready:
	and	es:[si].dlnFlags, NOT DLN_WAITING
	xor	di, di
	mov	ax, es:[si].dlnLeft
	cmp	ax, right
	jg	@@nextWaiting
	mov	ax, es:[si].dlnTop
	cmp	ax, bottom
	jg	@@nextWaiting
	mov	ax, es:[si].dlnRight
	cmp	ax, left
	jl	@@nextWaiting
	mov	ax, es:[si].dlnBottom
	cmp	ax, top
	jl	@@nextWaiting

	; Append to the drawing queue.
	mov	bx, [@DisplayList@_drawing_queue].dlqTail
	mov	es:[si].dlnPrev, bx
	mov	es:[si].dlnNext, 0
	or	bx, bx
	je	@@noDrawingTail
	mov	es:[bx].dlnNext, si
@@noDrawingTail:
	cmp	[@DisplayList@_drawing_queue].dlqHead, 0
	jne	@@setDrawingTail
	mov	[@DisplayList@_drawing_queue].dlqHead, si
@@setDrawingTail:
	mov	[@DisplayList@_drawing_queue].dlqTail, si
	or	es:[si].dlnFlags, DLN_DRAWING
	push	si
	call	far ptr @DisplayList@draw$qus
	add	sp, 2
	jmp	@@nextWaiting

	; Back to the end of the waiting queue.
@@putBack:
	or	di, di
	jne	@@appendWaiting
	mov	di, si
@@appendWaiting:
	mov	cx, [@DisplayList@_waiting_queue].dlqTail
	mov	es, [_DisplayListNodeSeg]
	mov	es:[si].dlnPrev, cx
	mov	es:[si].dlnNext, 0
	jcxz	@@noWaitingTail
	xchg	bx, cx
	mov	es:[bx].dlnNext, si
	xchg	bx, cx
@@noWaitingTail:
	cmp	[@DisplayList@_waiting_queue].dlqHead, 0
	jne	@@setWaitingTail
	mov	[@DisplayList@_waiting_queue].dlqHead, si
@@setWaitingTail:
	mov	[@DisplayList@_waiting_queue].dlqTail, si
	jmp	@@nextWaiting

@@nextDrawing:
	cmp	[@DisplayList@_drawing_queue].dlqHead, 0
	je	@@drained
	mov	si, [@DisplayList@_drawing_queue].dlqHead
	mov	es, [_DisplayListNodeSeg]
	mov	ax, es:[si].dlnNext
	mov	[@DisplayList@_drawing_queue].dlqHead, ax
	or	ax, ax
	jne	@@drawingLeft
	mov	[@DisplayList@_drawing_queue].dlqTail, ax
	jmp	short @@drawPopped
@@drawingLeft:
	mov	bx, ax
	mov	es:[bx].dlnPrev, 0
@@drawPopped:
	and	es:[si].dlnFlags, NOT DLN_DRAWING
	cmp	byte ptr [@DisplayList@animating], 0
	je	@@drawNode
	mov	cx, [@DisplayList@_waiting_queue].dlqTail
	mov	es:[si].dlnPrev, cx
	mov	es:[si].dlnNext, 0
	jcxz	@@requeueNoTail
	xchg	bx, cx
	mov	es:[bx].dlnNext, si
	xchg	bx, cx
@@requeueNoTail:
	cmp	[@DisplayList@_waiting_queue].dlqHead, 0
	jne	@@requeueSetTail
	mov	[@DisplayList@_waiting_queue].dlqHead, si
@@requeueSetTail:
	mov	[@DisplayList@_waiting_queue].dlqTail, si
	or	es:[si].dlnFlags, DLN_WAITING
	jmp	short @@nextDrawing
@@drawNode:
	push	es
	push	si
	mov	bx, es:[si].dlnVptr
	call	dword ptr [bx + VT_DRAW]
	add	sp, 4
	jmp	short @@nextDrawing

@@drained:
	cmp	byte ptr stuck, 0
	je	@@noLoop
	push	ds
	push	offset dlLoopMsg
	push	50			; trace level
	call	far ptr @trace$q10TraceLevelnzce
	add	sp, 6
@@noLoop:
	cmp	byte ptr [@DisplayList@animating], 0
	je	@@done
	mov	byte ptr [@DisplayList@animating], 0
	push	cs
	call	near ptr @DisplayList@draw$qv
@@done:
	ret
@DisplayList@draw$qv	ENDP

; static void DisplayList::checkNewItems(void)
; Flags in animatingitem each item of newitem whose shape animates and whose
; box reaches the screen, then clears newitem.
	PUBLIC	@DisplayList@checkNewItems$qv
@DisplayList@checkNewItems$qv	PROC	CPP FAR USES si di
	LOCAL	firstItem:WORD, bit:DWORD, bits:DWORD, newBits:DWORD, typeBits:DWORD, \
		offs:WORD, xSize:WORD, ySize:WORD, zSize:WORD
	mov	xSize, 0
	mov	ySize, 0
	mov	zSize, 0
	mov	firstItem, 0
	mov	eax, dword ptr [@DisplayList@newitem]
	mov	newBits, eax
	mov	offs, 0
@@nextWord:
	les	bx, newBits
	cmp	dword ptr es:[bx], 0
	jne	@@scanWord
@@wordDone:
	add	firstItem, 32
	add	word ptr newBits, 4
	add	offs, 4
	cmp	offs, NEWITEM_BYTES
	jl	@@nextWord
	jmp	@@done

@@scanWord:
	mov	eax, es:[bx]
	mov	bits, eax
	mov	si, firstItem
	xor	di, di
	mov	bit, 1
	jmp	@@nextBit
@@checkItem:
	mov	eax, bit
	test	bits, eax
	je	@@skipItem
	mov	ax, si
	shl	ax, 1
	les	bx, dword ptr [@ItemData@typeArray]
	add	bx, ax
	mov	ax, es:[bx]
	shl	ax, 3
	les	bx, [_GlobalTypes].toFlags
	add	bx, ax
	test	es:[bx].tfAnim, TF_ANIMTYPE
	je	@@skipItem
	mov	eax, dword ptr es:[bx].tfBits
	mov	typeBits, eax

	; dx = x - y, cx = (x + y) / 2 - z
	add	si, si
	les	bx, dword ptr [@ItemData@xArray]
	mov	dx, es:[bx + si]
	shr	dx, 2
	les	bx, dword ptr [@ItemData@yArray]
	mov	cx, es:[bx + si]
	shr	cx, 2
	shr	si, 1
	les	bx, dword ptr [@ItemData@zArray]
	mov	bl, es:[bx + si]
	xor	bh, bh
	mov	ax, dx
	sub	ax, cx
	sar	cx, 1
	sar	dx, 1
	add	cx, dx
	sub	cx, bx
	mov	dx, ax

	; Sizes in pixels; a flat loose item still counts as 1 high.
	mov	ax, si
	shl	ax, 1
	les	bx, dword ptr [@ItemData@statusArray]
	add	bx, ax
	test	word ptr es:[bx], ITEM_FLIPPED
	je	@@notFlipped
	mov	ax, word ptr typeBits + 2
	shr	al, 1
	and	al, 78h
	mov	byte ptr ySize, al
	mov	al, ah
	and	al, 0Fh
	shl	al, 3
	mov	byte ptr xSize, al
	shr	ah, 1
	and	ah, 78h
	mov	byte ptr zSize, ah
	jne	@@sized
	test	byte ptr typeBits, TF_FIXED
	jne	@@sized
	mov	zSize, 1
	jmp	short @@sized
@@notFlipped:
	mov	ax, word ptr typeBits + 2
	shr	al, 1
	and	al, 78h
	mov	byte ptr xSize, al
	mov	al, ah
	and	al, 0Fh
	shl	al, 3
	mov	byte ptr ySize, al
	shr	ah, 1
	and	ah, 78h
	mov	byte ptr zSize, ah
	jne	@@sized
	test	byte ptr typeBits, TF_FIXED
	jne	@@sized
	mov	zSize, 1

	; Skip the item unless its box overlaps the screen.
@@sized:
	mov	bx, dx
	mov	ax, xSize
	sub	bx, ax
	sub	bx, [@Camera@_h]
	cmp	bx, SCREEN_W
	jge	@@skipItem
	sar	ax, 1
	mov	bx, cx
	sub	bx, ax
	mov	ax, ySize
	sar	ax, 1
	sub	bx, ax
	sub	bx, zSize
	sub	bx, [@Camera@_v]
	cmp	bx, SCREEN_H
	jge	@@skipItem
	add	dx, ySize
	sub	dx, [@Camera@_h]
	jl	@@skipItem
	sub	cx, [@Camera@_v]
	jl	@@skipItem
	les	bx, dword ptr [@DisplayList@animatingitem]
	add	bx, offs
	mov	eax, bit
	or	es:[bx], eax
@@skipItem:
	inc	si
	inc	di
	mov	eax, bit
	shl	eax, 1
	mov	bit, eax
@@nextBit:
	cmp	di, 32
	jge	@@clearWord
	jmp	@@checkItem
@@clearWord:
	les	bx, newBits
	mov	dword ptr es:[bx], 0
	jmp	@@wordDone
@@done:
	ret
@DisplayList@checkNewItems$qv	ENDP

; static unsigned char DisplayList::sort(unsigned short node)
; Sorts node against each node in its shadow squares whose rectangle overlaps
; it, and against the squares. Returns 0 as soon as a pairwise sort fails.
	PUBLIC	@DisplayList@sort$qus
@DisplayList@sort$qus	PROC	CPP FAR USES si di, node:WORD
	LOCAL	result:WORD, edge:WORD, firstCol:WORD, lastCol:WORD, lastRow:WORD, \
		col:WORD, row:WORD, square:WORD, marked:WORD, cell:WORD, rowOffset:WORD
	mov	byte ptr result, 1
	mov	si, node
	mov	es, [_DisplayListNodeSeg]

	; Grid squares under the node's rectangle.
	mov	ax, es:[si].dlnLeft
	sub	ax, [_ShadowSquareLC]
	sar	ax, SQUARE_SHIFT
	mov	firstCol, ax
	mov	dx, es:[si].dlnTop
	sub	dx, [_ShadowSquareTC]
	sar	dx, SQUARE_SHIFT
	mov	ax, es:[si].dlnRight
	sub	ax, [_ShadowSquareLC]
	sar	ax, SQUARE_SHIFT
	mov	lastCol, ax
	mov	ax, es:[si].dlnBottom
	sub	ax, [_ShadowSquareTC]
	sar	ax, SQUARE_SHIFT
	mov	lastRow, ax
	mov	marked, 0
	cmp	firstCol, SQUARE_COLS
	jge	@@sorted
	cmp	dx, SQUARE_ROWS
	jge	@@sorted
	cmp	lastCol, 0
	jl	@@sorted
	cmp	lastRow, 0
	jl	@@sorted
	cmp	firstCol, 0
	jge	@@firstColOk
	mov	firstCol, 0
@@firstColOk:
	or	dx, dx
	jge	@@firstRowOk
	xor	dx, dx
@@firstRowOk:
	cmp	lastCol, SQUARE_COLS
	jl	@@lastColOk
	mov	lastCol, SQUARE_COLS - 1
@@lastColOk:
	cmp	lastRow, SQUARE_ROWS
	jl	@@lastRowOk
	mov	lastRow, SQUARE_ROWS - 1
@@lastRowOk:
	mov	row, dx
	mov	ax, dx
	mov	dx, SQUARE_ROW_BYTES
	imul	dx
	mov	rowOffset, ax
	jmp	@@nextRow

@@rowLoop:
	mov	ax, firstCol
	mov	col, ax
	mov	dx, ax
	shl	dx, 1
	mov	ax, rowOffset
	add	ax, dx
	add	ax, offset _ShadowSquare
	mov	cell, ax
	jmp	@@nextCol

	; Sort against each unmarked node above this square that overlaps node.
@@colLoop:
	mov	bx, cell
	mov	ax, [bx]
	mov	square, ax
	mov	es, [_DisplayListNodeSeg]
	mov	bx, square
	mov	bx, es:[bx].dlnAbove
	mov	edge, bx
	or	bx, bx
	je	@@sortSquare
@@edgeLoop:
	mov	es, [_DisplayListEdgeSeg]
	mov	di, es:[bx].dleUpper
	mov	es, [_DisplayListNodeSeg]
	test	es:[di].dlnFlags, DLN_MARKED
	jne	@@nextEdge
	mov	ax, es:[si].dlnLeft
	cmp	ax, es:[di].dlnRight
	jg	@@mark
	mov	ax, es:[si].dlnTop
	cmp	ax, es:[di].dlnBottom
	jg	@@mark
	mov	ax, es:[si].dlnRight
	cmp	ax, es:[di].dlnLeft
	jl	@@mark
	mov	ax, es:[si].dlnBottom
	cmp	ax, es:[di].dlnTop
	jl	@@mark
	push	di
	push	si
	call	far ptr @DisplayList@sort$qusus
	add	sp, 4
	or	al, al
	je	@@exit
	mov	es, [_DisplayListNodeSeg]
@@mark:
	or	es:[di].dlnFlags, DLN_MARKED
	mov	ax, marked
	mov	es:[di].dlnMarkNext, ax
	mov	marked, di
@@nextEdge:
	mov	es, [_DisplayListEdgeSeg]
	mov	bx, edge
	mov	bx, es:[bx].dleNextAbove
	mov	edge, bx
	or	bx, bx
	jne	@@edgeLoop
@@sortSquare:
	push	square
	push	si
	call	far ptr @DisplayList@sort$qusus
	add	sp, 4
	or	al, al
	je	@@exit
	add	cell, 2
	inc	col
@@nextCol:
	mov	ax, col
	cmp	ax, lastCol
	jle	@@colLoop
	add	rowOffset, SQUARE_ROW_BYTES
	inc	row
@@nextRow:
	mov	ax, row
	cmp	ax, lastRow
	jle	@@rowLoop

	; Clear the marks.
	mov	es, [_DisplayListNodeSeg]
	mov	di, marked
	or	di, di
	je	@@sorted
@@unmark:
	and	es:[di].dlnFlags, NOT DLN_MARKED
	mov	di, es:[di].dlnMarkNext
	or	di, di
	jne	@@unmark
@@sorted:
	mov	al, byte ptr result
@@exit:
	ret
@DisplayList@sort$qus	ENDP

; int DL_ItemNode::compare(unsigned short other)
; Orders two item nodes whose boxes overlap on screen: by x, then y, then z,
; then the unknown1 type flag, then by depth sum. Returns a CMP_ value.
	PUBLIC	@DL_ItemNode@compare$qus
@DL_ItemNode@compare$qus	PROC	CPP FAR USES si di, self:DWORD, other:WORD
	LOCAL	item:WORD, xSize:WORD, ySize:WORD, zSize:WORD, flippedYZ:WORD, plainYZ:WORD, \
		myX:WORD, myXZ:WORD, myYZ:WORD, itsYZ:WORD, myScreenX:WORD, itsScreenX:WORD, \
		itsTop:WORD, mySum:WORD, itsSum:WORD

	; si, di, cx = the other item's x, y, z
	mov	si, other
	mov	es, [_DisplayListNodeSeg]
	mov	di, es:[si].dliReferent
	mov	item, di
	shl	di, 1
	les	bx, dword ptr [@ItemData@xArray]
	mov	si, es:[bx + di]
	shr	si, 2
	les	bx, dword ptr [@ItemData@yArray]
	mov	di, es:[bx + di]
	shr	di, 2
	les	bx, dword ptr [@ItemData@zArray]
	add	bx, item
	mov	cl, es:[bx]
	xor	ch, ch

	; Its sizes in pixels; a flat loose item still counts as 1 high.
	mov	ax, item
	shl	ax, 1
	les	bx, dword ptr [@ItemData@statusArray]
	add	bx, ax
	test	word ptr es:[bx], ITEM_FLIPPED
	je	@@notFlipped
	les	bx, dword ptr [@ItemData@typeArray]
	add	bx, ax
	mov	ax, es:[bx]
	shl	ax, 3
	les	bx, [_GlobalTypes].toFlags
	add	bx, ax
	mov	al, es:[bx].tfEquipX
	shr	ax, 4
	and	ax, 0Fh
	shl	ax, 3
	mov	ySize, ax
	mov	al, es:[bx].tfSizeYZ
	mov	byte ptr flippedYZ, al
	and	ax, 0Fh
	shl	ax, 3
	mov	xSize, ax
	mov	al, byte ptr flippedYZ
	shr	ax, 4
	and	ax, 0Fh
	shl	ax, 3
	mov	zSize, ax
	or	ax, ax
	jne	@@sized
	mov	al, es:[bx].tfBits
	and	ax, TF_FIXED
	jne	@@sized
	mov	zSize, 1
	jmp	short @@sized
@@notFlipped:
	mov	ax, item
	shl	ax, 1
	les	bx, dword ptr [@ItemData@typeArray]
	add	bx, ax
	mov	ax, es:[bx]
	shl	ax, 3
	les	bx, [_GlobalTypes].toFlags
	add	bx, ax
	mov	al, es:[bx].tfEquipX
	shr	ax, 4
	and	ax, 0Fh
	shl	ax, 3
	mov	xSize, ax
	mov	al, es:[bx].tfSizeYZ
	mov	byte ptr plainYZ, al
	and	ax, 0Fh
	shl	ax, 3
	mov	ySize, ax
	mov	al, byte ptr plainYZ
	shr	ax, 4
	and	ax, 0Fh
	shl	ax, 3
	mov	zSize, ax
	or	ax, ax
	jne	@@sized
	mov	al, es:[bx].tfBits
	and	ax, TF_FIXED
	jne	@@sized
	mov	zSize, 1

	; Apart unless the two outlines overlap along x - z, y - z and x - y.
@@sized:
	les	bx, self
	mov	ax, es:[bx].dliX
	mov	myX, ax
	sub	ax, es:[bx].dliZ
	mov	myXZ, ax
	sub	ax, es:[bx].dliXSize
	sub	ax, es:[bx].dliZSize
	mov	dx, si
	sub	dx, cx
	cmp	ax, dx
	jge	@@apart
	mov	ax, dx
	sub	ax, xSize
	sub	ax, zSize
	cmp	myXZ, ax
	jle	@@apart
	les	bx, self
	mov	ax, es:[bx].dliY
	sub	ax, es:[bx].dliZ
	mov	myYZ, ax
	sub	ax, es:[bx].dliYSize
	sub	ax, es:[bx].dliZSize
	mov	dx, di
	sub	dx, cx
	mov	itsYZ, dx
	cmp	ax, dx
	jge	@@apart
	mov	ax, itsYZ
	sub	ax, ySize
	sub	ax, zSize
	cmp	myYZ, ax
	jle	@@apart
	les	bx, self
	mov	ax, es:[bx].dliScreenX
	mov	myScreenX, ax
	sub	ax, es:[bx].dliXSize
	mov	dx, si
	sub	dx, di
	mov	itsScreenX, dx
	add	dx, ySize
	cmp	ax, dx
	jge	@@apart
	mov	ax, myScreenX
	add	ax, es:[bx].dliYSize
	mov	dx, itsScreenX
	sub	dx, xSize
	cmp	ax, dx
	jle	@@apart

	; Boxes separated along x, then y.
	mov	ax, myX
	les	bx, self
	sub	ax, es:[bx].dliXSize
	cmp	ax, si
	jl	@@notXOver
	mov	ax, CMP_OVER
	jmp	@@exit
@@notXOver:
	mov	ax, si
	sub	ax, xSize
	cmp	es:[bx].dliX, ax
	jg	@@checkY
	mov	ax, CMP_UNDER
	jmp	@@exit
@@checkY:
	mov	ax, es:[bx].dliY
	sub	ax, es:[bx].dliYSize
	cmp	ax, di
	jl	@@notYOver
	mov	ax, CMP_OVER
	jmp	@@exit
@@notYOver:
	mov	ax, di
	sub	ax, ySize
	cmp	es:[bx].dliY, ax
	jg	@@checkZ
	mov	ax, CMP_UNDER
	jmp	@@exit

	; Then along z.
@@checkZ:
	mov	ax, cx
	add	ax, zSize
	mov	itsTop, ax
	cmp	es:[bx].dliZ, ax
	jl	@@notAbove
	mov	ax, es:[bx].dliZ
	add	ax, es:[bx].dliZSize
	cmp	ax, cx
	jle	@@notAbove
	mov	ax, CMP_OVER
	jmp	@@exit
@@notAbove:
	mov	ax, es:[bx].dliZ
	add	ax, es:[bx].dliZSize
	cmp	ax, cx
	jg	@@flagTest
	mov	ax, es:[bx].dliZ
	cmp	ax, itsTop
	jge	@@flagTest
	mov	ax, CMP_UNDER
	jmp	@@exit

	; A type with unknown1 goes over one without.
@@flagTest:
	les	bx, self
	mov	ax, es:[bx].dliReferent
	shl	ax, 1
	les	bx, dword ptr [@ItemData@typeArray]
	add	bx, ax
	mov	ax, es:[bx]
	shl	ax, 3
	les	bx, [_GlobalTypes].toFlags
	add	bx, ax
	test	es:[bx].tfAnimSpeed, TF_UNKNOWN1
	je	@@myFlagClear
	mov	dl, 1
	jmp	short @@myFlagDone
@@myFlagClear:
	xor	dl, dl
@@myFlagDone:
	mov	ax, item
	shl	ax, 1
	les	bx, dword ptr [@ItemData@typeArray]
	add	bx, ax
	mov	ax, es:[bx]
	shl	ax, 3
	les	bx, [_GlobalTypes].toFlags
	add	bx, ax
	test	es:[bx].tfAnimSpeed, TF_UNKNOWN1
	je	@@itsFlagClear
	mov	ax, 1
	jmp	short @@itsFlagDone
@@itsFlagClear:
	xor	ax, ax
@@itsFlagDone:
	mov	bl, al
	or	dl, dl
	je	@@notMine
	mov	ah, 0
	or	ax, ax
	jne	@@notMine
	mov	ax, CMP_OVER
	jmp	short @@exit
@@notMine:
	or	bl, bl
	je	@@depthSum
	mov	al, dl
	mov	ah, 0
	or	ax, ax
	jne	@@depthSum
	mov	ax, CMP_UNDER
	jmp	short @@exit

	; Last resort: (x - xSize) + (y - ySize) + 2z, the larger goes over.
@@depthSum:
	mov	ax, myX
	les	bx, self
	sub	ax, es:[bx].dliXSize
	add	ax, es:[bx].dliY
	sub	ax, es:[bx].dliYSize
	add	ax, es:[bx].dliZ
	add	ax, es:[bx].dliZ
	mov	mySum, ax
	mov	dx, si
	sub	dx, xSize
	add	dx, di
	sub	dx, ySize
	add	dx, cx
	add	dx, cx
	mov	itsSum, dx
	cmp	ax, dx
	jle	@@notOver
	mov	ax, CMP_OVER
	jmp	short @@exit
@@notOver:
	mov	ax, mySum
	cmp	ax, itsSum
	jge	@@apart
	mov	ax, CMP_UNDER
	jmp	short @@exit
@@apart:
	xor	ax, ax			; CMP_APART
@@exit:
	ret
@DL_ItemNode@compare$qus	ENDP

	END
