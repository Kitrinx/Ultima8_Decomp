; Shape frame layout and clip flags shared by the skip drawers.
;
; Each row is a list of skip/run pairs that ends once the frame's width is
; used up:  skip, run, pixels..., skip, run, pixels..., ...
; skip is a count of transparent pixels, run a count of pixels that follow.
; In a compressed frame run holds count*2 + fill; with fill set one pixel
; byte follows and is repeated count times.

SKIPFRAME	STRUC
sfFlags		dw	?	; SF_COMPRESSED
sfWidth		dw	?
sfHeight	dw	?
sfXOff		dw	?	; hot spot, from the frame's left
sfYOff		dw	?	; hot spot, from the frame's top
sfRows		dw	?	; per row, offset of its data from its own entry
SKIPFRAME	ENDS

SF_COMPRESSED	EQU	1

; Which end of each row, in stored order, the viewport cuts off.
CLIP_HEAD	EQU	1
CLIP_TAIL	EQU	2
