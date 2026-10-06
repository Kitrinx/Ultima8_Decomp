; Vport and Rect as the C++ side lays them out (GRAPHICS\CVPORT.H).

VPORT	STRUC
vpSeg		dw	?	; selector of the pixel memory
vpYTable	dd	?	; row offsets, one word per line
vpYTableFlag	db	?
vpX1		dw	?	; clip rectangle, inclusive
vpY1		dw	?
vpX2		dw	?
vpY2		dw	?
VPORT	ENDS
