; name: ..\ITEM\AREAFIND
; Hand-written search loop of AreaItemFinder.

	.MODEL	LARGE
	.386
	LOCALS

; AreaItemFinder layout
IF_VPTR		EQU	2
AF_LEFT		EQU	26h	; then top, right, bottom

VT_IN_AREA	EQU	4	; AreaItemFinder::inArea in the vtable

	EXTRN	@AreaItemFinder@gotoNextList$qv:FAR
	EXTRN	@ItemFinder@meetsSearchCriteria$qv:FAR

	.DATA

	EXTRN	@ItemData@nextArray:WORD
	EXTRN	@ItemData@xArray:WORD
	EXTRN	@ItemData@yArray:WORD

	.CODE

; int AreaItemFinder::inArea(void)
; 1 when the found item lies inside the finder's rectangle.
	PUBLIC	@AreaItemFinder@inArea$qv
@AreaItemFinder@inArea$qv	PROC	CPP FAR USES ds si, finder:DWORD
	les	bx, finder
	mov	si, es:[bx]
	add	si, si
	les	bx, dword ptr [@ItemData@xArray]
	mov	cx, es:[bx + si]
	lds	bx, dword ptr [@ItemData@yArray]
	mov	dx, [bx + si]
	xor	bx, bx
	lds	si, finder
	add	si, AF_LEFT
	lodsw
	cmp	ax, cx
	ja	@@done
	lodsw
	cmp	ax, dx
	ja	@@done
	lodsw
	cmp	ax, cx
	jb	@@done
	lodsw
	cmp	ax, dx
	jb	@@done
	inc	bx
@@done:
	mov	ax, bx
	ret
@AreaItemFinder@inArea$qv	ENDP

; void AreaItemFinder::findNext(void)
; Steps along the region lists to the next item in the area that meets the criteria.
	PUBLIC	@AreaItemFinder@findNext$qv
@AreaItemFinder@findNext$qv	PROC	CPP FAR USES si di, finder:DWORD
	les	di, finder
	mov	si, es
@@next:
	mov	es, si
	cmp	word ptr es:[di], 0
	je	@@done
	mov	ax, es:[di]
	les	bx, dword ptr [@ItemData@nextArray]
	add	bx, ax
	add	bx, ax
	mov	ax, es:[bx]
	mov	es, si
	mov	es:[di], ax
	or	ax, ax
	jne	@@check
	push	finder
	call	far ptr @AreaItemFinder@gotoNextList$qv
	add	sp, 4
	mov	es, si
	cmp	word ptr es:[di], 0
	je	@@done
@@check:
	push	es
	push	di
	mov	bx, es:[di + IF_VPTR]
	call	dword ptr [bx + VT_IN_AREA]
	add	sp, 4
	or	al, al
	je	@@next
	push	finder
	call	far ptr @ItemFinder@meetsSearchCriteria$qv
	add	sp, 4
	or	al, al
	je	@@next
@@done:
	ret
@AreaItemFinder@findNext$qv	ENDP

	END
