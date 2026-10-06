; Display list classes as the C++ side lays them out.
; Nodes and edges live in their own segments and are passed around as offsets.
;
;   upper node --dlnBelow--> edge --dleLower--> lower node
;   lower node --dlnAbove--> edge --dleUpper--> upper node
;
; A lower node is drawn before every upper node that covers it.

DLQUEUE	STRUC
dlqHead		dw	?	; first node, 0 when empty
dlqTail		dw	?
DLQUEUE	ENDS

DLNODE	STRUC
dlnVptr		dw	?
dlnMarkNext	dw	?	; chains the nodes marked during a sort
dlnNext		dw	?	; queue links
dlnPrev		dw	?
dlnAbove	dw	?	; edges to the nodes drawn over this one
dlnBelow	dw	?	; edges to the nodes this one is drawn over
dlnLeft		dw	?	; screen rectangle, camera included
dlnTop		dw	?
dlnRight	dw	?
dlnBottom	dw	?
dlnFlags	db	?
DLNODE	ENDS

DLN_WAITING	EQU	01h	; in the waiting queue
DLN_MARKED	EQU	08h	; already visited by this sort
DLN_DRAWING	EQU	20h	; in the drawing queue
DLN_SHADOW	EQU	40h	; one of the fixed 64x64 screen squares

DLEDGE	STRUC
dleUpper	dw	?
dleLower	dw	?
dleNextAbove	dw	?	; links in the lower node's dlnAbove list
dlePrevAbove	dw	?
dleNextBelow	dw	?	; links in the upper node's dlnBelow list
dlePrevBelow	dw	?
DLEDGE	ENDS

; An item's node. Positions and sizes are world units / 4.
DLITEMNODE	STRUC
dliNode		DLNODE	<>
dliReferent	dw	?	; item number
dliScreenX	dw	?	; x - y
dliScreenY	dw	?	; (x + y) / 2 - z
dliX		dw	?
dliY		dw	?
dliZ		dw	?
dliXSize	dw	?
dliYSize	dw	?
dliZSize	dw	?
DLITEMNODE	ENDS

; One typeflag.dat record (TypeFlag).
TYPEFLAG	STRUC
tfBits		db	?	; fixed, solid, sea, land, occl, bag, damaging, noisy
tfBits2		db	?	; draw, ignore, roof, trans; family in the high nibble
tfEquipX	db	?	; equip type; x size in the high nibble
tfSizeYZ	db	?	; y size; z size in the high nibble
tfAnim		db	?	; anim type; anim data in the high nibble
tfAnimSpeed	db	?	; anim speed; editor, explode, unknown1, unknown2 bits
tfWeight	db	?
tfVolume	db	?
TYPEFLAG	ENDS

TF_FIXED	EQU	01h	; in tfBits
TF_ANIMTYPE	EQU	0Fh	; in tfAnim
TF_UNKNOWN1	EQU	40h	; in tfAnimSpeed

TYPEOBJECT	STRUC
toNames		dd	?
toFlags		dd	?	; TYPEFLAG far array, one per shape
TYPEOBJECT	ENDS

ITEM_FLIPPED	EQU	20h	; status bit: x and y sizes swap
