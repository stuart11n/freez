asm mov  bx,LevelDataP
asm	add	 bx,74
asm mov  ByteLoop,16

asm jmp  BRIDGE_LOOP
NEXT_SEG_B:
asm jmp  NEXT_SEG

BRIDGE_LOOP:

asm {
xor  si,si						// Counter for current bridge segment

mov  ax,wposy
sub  ax,8
mov  cx,[bx+2]					// Get bridge y pos to see if MC is on it
or   cx,cx						// Skip if no bridge is stored
jz   NEXT_SEG_B
cmp  cx,ax; jl   NEXT_SEG_B		// Is bridge onscreen?
add  ax,198
cmp  cx,ax; jg   NEXT_SEG_B		// Skip if not in y-plane
mov  ax,[bx]					// Get xpos
mov  dx,wposx
sub  dx,255
cmp  ax,dx; jl   NEXT_SEG_B
add  dx,575
cmp  ax,dx; jg   NEXT_SEG_B		// Skip if not in x-plane

mov  ax,wposy
add  ax,100
cmp  cx,ax
jle  DD							// JMP to DD if not on bridge
add  ax,20
cmp  cx,ax
jge  DD

mov  cx,[bx]					// Get X position

mov  ax,wposx					// Check in x-plane
mov  dx,ax
add  ax,153
add  dx,161
}

DLS:							// Find current bridge position
asm cmp  cx,ax
asm jl   NEXTDD					// If AX<CX<DX its this segment - CX
asm cmp  cx,dx
asm jle  DD
NEXTDD:
asm add  cx,8					// Spacing in pixels
asm inc  si
asm cmp  si,31
asm jb   DLS
DD:

asm shl  si,6                   // SI : OFFSET to bridge dynamics for current segment
asm add  si,OFFSET BridgeDynamics

asm mov  di,[bx]				// DI : X pos
asm mov  ByteLoop2,32

SL:

 asm {
 mov  dx,[bx+2]					// DX : Y pos
 add  dx,[si]					// Apply dynamics
 sub  dx,wposy

 mov  ax,di						// AX : X pos
 sub  ax,wposx

 cmp  ax,-8;  jl	  OLB
 cmp  ax,319; jg	  OLB
 cmp  dx,-8;  jl	  OLB
 cmp  dx,190; jg	  OLB

 push bx dx ax					// (BX),Y,X
 call far ptr @pbridge$qii
 add  sp,4						// Correct Stack
 pop  bx 						// Restore BPos offset pointer
 }

 OLB:

 asm add  si,2
 asm add  di,8					// Spacing
 asm dec  ByteLoop2
 asm jnz  SL

NEXT_SEG:

asm add  bx,4					// Point to next BPos record
asm dec  ByteLoop
asm jz   EX_BL
asm jmp  BRIDGE_LOOP         	// Do it again
EX_BL: