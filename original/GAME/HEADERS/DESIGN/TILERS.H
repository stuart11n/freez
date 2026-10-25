// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФЗ
// К SKY TILE PLOTTER К
// гФФФФФФФФФФФФФФФФФФН

asm {

cmp  byte ptr LAYER1,0
jne  SKYOK
jmp  NO_SKY
}

SKYOK:

asm {
				// Setup Tile Loop
mov  ax,wposx
sar  ax,1
mov  bx,ax
and  ax,0xFFE0
sub  ax,bx           		// AX : XSTART POS

mov  dx,wposy
sar  dx,1
mov  bx,dx
and  dx,0xFFE0			// Shift Left by 5 & Shift Right by 5
mov  cx,dx			// Start Tile Index (Y)
sub  dx,bx			// DX : YSTART POS (Screen)

sal  cx,1
mov  bx,wposx
sar  bx,6
add  bx,cx			// BX : TILEINDEX

mov  TileX,ax
mov  TileY,dx
mov  TileIndex,bx


// Tile Loop

mov  tyloop,7			// Init Y loop
}

TYLOOPa:

asm {
mov  ax,TileX
mov  TileX2,ax

mov  txloop,11			// Init X loop
}

TXLOOPa:

asm {
cmp  TileY,190
jg   OFFLIMITSa
cmp  TileY,-31
jl   OFFLIMITSa
cmp  TileX2,319
jg   OFFLIMITSa
cmp  TileX2,-31
jl   OFFLIMITSa

mov  si,OFFSET skymap
mov  bx,TileIndex

cmp  byte ptr [si+bx],0		// Check for transparent tile
je   short OFFLIMITSa

push word ptr [si+bx]           // Init Stack
push TileY
push TileX2

call far ptr @ptile$qiiuc

add  sp,6			// Correct stack
}

OFFLIMITSa:

asm {
add  TileX2,32
inc  TileIndex

dec  txloop
jnz  TXLOOPa			// Goto X loop

add  TileY,32
add  TileIndex,53

dec  tyloop
jnz  TYLOOPa			// Goto Y loop
}

NO_SKY:

#include "game\headers\bridge.h"

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФЗ
// К TILE PLOTTER К
// гФФФФФФФФФФФФФФН

//entry=0;

asm {

cmp  byte ptr LAYER2,0
jne  LANDOK
jmp  NO_LAND
}

LANDOK:

asm {

// Setup Tile Loop

mov  ax,wposx
mov  bx,ax
and  ax,0FFE0h
sub  ax,bx           		// AX : XSTART POS

mov  dx,wposy
mov  bx,dx
and  dx,0FFE0h
mov  cx,dx
sub  dx,bx					// BX : YSTART POS

sal  cx,2
mov  bx,wposx
sar  bx,5
add  bx,cx					// DX : TILEINDEX

mov  TileX,ax
mov  TileY,dx
mov  TileIndex,bx

// Tile Loop

mov  tyloop,7				// Init Y loop

}

TYLOOP:

asm {

mov  ax,TileX
mov  TileX2,ax

mov  txloop,10				// Init X loop

}

TXL2:

//entry++;

asm {
cmp  TileX2,319
jg   OFFLIMITS
cmp  TileX2,-32
jl   OFFLIMITS
}

TXLOOP:

asm {
cmp  TileY,190
jg   OFFLIMITS
cmp  TileY,-31
jl   OFFLIMITS

mov  si,OFFSET tilemap		// Init Stack
mov  bx,TileIndex

mov  al,[si+bx]         	// Fix foreground and flash
btr  ax,7
jnc  NOIND2
test BonusFrame,15
je	 OFFLIMITS
}
NOIND2:

asm {
cmp  al,0				// Check for transparent tile
je   short OFFLIMITS

push ax
push TileY
push TileX2

call far ptr @ptile$qiiuc

add  sp,6			// Correct stack

}

OFFLIMITS:

asm {

add  TileX2,32
inc  TileIndex

dec  txloop
jg   TXLOOP         // Goto X loop
jz   TXL2

add  TileY,32
add  TileIndex,117

dec  tyloop
jnz  TYLOOP			// Goto Y loop
}

NO_LAND:

//gotoxy(1,24);printf("X CLPD TILES: %d",entry);
