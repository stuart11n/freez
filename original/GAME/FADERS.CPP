asm .386  			// Enable 386 Instructions
asm SMART 			// Enable Assembler Optimisation
asm WARN ALN
asm WARN BRK
asm WARN ICG
asm WARN PQK

extern unsigned char pal[768],FadeStore[768],SPal[768];

void Fade2Black(char FLAG)
{
asm mov 	si,OFFSET pal
asm mov 	di,OFFSET FadeStore
asm mov 	cx,192
FSU:
asm mov 	eax,[si]
asm mov 	[di],eax
asm add 	si,4
asm add 	di,4
asm loop	FSU

asm mov		bx,64
asm cmp		FLAG,0
asm jz		F2BLOOP
asm mov		bx,20

F2BLOOP:

 asm mov  si,OFFSET FadeStore
 asm mov  cx,256

 F2B:

 asm cmp  byte ptr [si],0
 asm jz   B_RED
 asm dec  byte ptr [si]
 B_RED:
 asm cmp  byte ptr [si+1],0
 asm jz   B_GRN
 asm dec  byte ptr [si+1]
 B_GRN:
 asm cmp  byte ptr [si+2],0
 asm jz   B_BLU
 asm dec  byte ptr [si+2]
 B_BLU:

 asm add  si,3
 asm loop F2B

 asm mov  dx,0x3DA
 asm in   al,dx
 asm test al,08h
 asm je   $-5

 asm mov  si,OFFSET FadeStore
 asm sub  al,al		// Init xfer
 asm mov  cx,768
 asm mov  dx,0x3C8
 asm out  dx,al
 asm inc  dx		// Send Palette
 asm rep  outsb

asm dec  bx
asm jnz  F2BLOOP
}

void FadeFromBlack(char FLAG)
{
asm mov		bx,64
asm cmp		FLAG,0
asm jz		FFBLOOP
asm mov		bx,24

FFBLOOP:

 asm mov  dx,0x3DA
 asm in   al,dx
 asm test al,08h
 asm je   $-5

 asm mov  si,OFFSET FadeStore
 asm mov  di,OFFSET pal

 asm mov  cx,256

 FFB:

 asm mov  al,[di]
 asm cmp  [si],al
 asm je   F_RED
 asm inc  byte ptr [si]
 F_RED:
 asm mov  al,[di+1]
 asm cmp  [si+1],al
 asm je   F_GRN
 asm inc  byte ptr [si+1]
 F_GRN:
 asm mov  al,[di+2]
 asm cmp  [si+2],al
 asm je   F_BLU
 asm inc  byte ptr [si+2]
 F_BLU:

 asm add  di,3
 asm add  si,3
 asm loop FFB

 asm mov  si,OFFSET FadeStore

 asm sub  al,al		// Init xfer
 asm mov  dx,0x3C8
 asm out  dx,al

 asm inc  dx		// Send Palette
 asm mov  cx,768
 asm rep  outsb

asm dec  bx
asm jnz  FFBLOOP
}

void SavePal(void)
{
asm mov  si,OFFSET pal
asm mov  di,OFFSET SPal
asm mov  cx,192
SPL:
asm mov  eax,[si]
asm mov  [di],eax
asm add  si,4
asm add  di,4
asm loop SPL
}

void SetPal(void)
{
asm mov  si,OFFSET SPal
asm mov  di,OFFSET pal
asm mov  cx,192
SPL:
asm mov  eax,[si]
asm mov  [di],eax
asm add  si,4
asm add  di,4
asm loop SPL
}