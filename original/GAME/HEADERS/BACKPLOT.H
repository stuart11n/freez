// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Land Tile Plotter К                                                   К
// гФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
							// Setup Tile Loop
mov  ax,wposx
mov  bx,ax
and  ax,01Fh                // AX : XSTART POS & SUBSEQUENT X PLOT POS
neg  ax

mov  TileX,ax
mov  FTileX,ax

mov  si,wposy
mov  cx,si
and  si,01Fh
neg  si   					// DX : YSTART POS & SUBSEQUENT Y PLOT POS

mov  FYpos,si

and  cx,0FFE0h              // Nearest sprite boundary
shl  cx,2

sar  bx,5					// Div X-pos by 32 to get X-pos in tilemap
add  bx,cx					// BX : TILEINDEX

mov  TileIndex,bx
mov  FTileIndex,bx

mov  ByteLoop,7			// Init Y loop
}

TYLOOP:

asm {
mov  di,TileX
mov  ah,11					// Init X loop
}

TXLOOP:

asm {
cmp  di,319;jg OFFLIMITS
cmp  di,-31;jl OFFLIMITS
cmp  si,190;jg OFFLIMITS
cmp  si,-31;jl OFFLIMITS

mov  bx,TileIndex

mov	 al,[OFFSET tilemap+bx]	// Check it's a background tile
btr  ax,7
jc	 OFFLIMITS
or   al,al					// Check for transparent tile
je   OFFLIMITS

push ax si di

call far ptr @ptile$qiiuc

pop  di si ax
}

OFFLIMITS:

asm {
add  di,32
inc  TileIndex

dec  ah
jnz  TXLOOP					// Goto X loop

add  si,32
add  TileIndex,117

dec  ByteLoop
jnz  TYLOOP			// Goto Y loop
}