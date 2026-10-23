// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Sky Tile Plotter К                                                    К
// гФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {

test byte ptr GameSetup,4
jne  DO_PARALLAX
jmp  NO_PARALLAX
}

DO_PARALLAX:

asm {
							// Setup Tile Loop
mov  ax,wposx
mov  bx,ax
shr  ax,1
and  ax,01Fh                // AX : XSTART POS & SUBSEQUENT X PLOT POS
neg  ax

mov  TileX,ax

mov  si,wposy
shr  si,1
mov  cx,si
and  si,01Fh
neg  si   					// DX : YSTART POS & SUBSEQUENT Y PLOT POS

and  cx,0FFE0h              // Nearest sprite boundary
add  cx,cx					// Multiply cx by 2

shr  bx,6					// Div X-pos by 32 to get X-pos in tilemap
add  bx,cx					// BX : TILEINDEX

mov  TileIndex,bx

mov  ByteLoop2,7				// Init Y loop
}

TYLOOPS:

asm {
mov  di,TileX
mov  ah,11					// Init X loop
}

TXLOOPS:

asm {
cmp  di,319;jg OFFLIMITSS
cmp  di,-31;jl OFFLIMITSS
cmp  si,190;jg OFFLIMITSS
cmp  si,-31;jl OFFLIMITSS

mov  bx,TileIndex

mov	 al,[OFFSET skymap+bx]	// Get tile value at index
or   al,al					// Check for no tile
je   OFFLIMITSS

push ax si di
call far ptr @ptile$qiiuc
pop  di si ax
}

OFFLIMITSS:

asm {
add  di,32
inc  TileIndex

dec  ah
jnz  TXLOOPS				// Goto X loop

add  si,32
add  TileIndex,53

dec  ByteLoop2
jnz  TYLOOPS				// Goto Y loop
}

NO_PARALLAX:

