asm .386

extern StatGFX;

void PlotChars(void) // DX : No of Chars, BX : Offset of string, DI : Pos
{

SLOOP:

asm mov si,OFFSET StatGFX
asm xor ax,ax
asm mov al,[bx]
asm shl ax,6
asm add si,ax

asm mov   cx,2
asm rep   movsd

asm add   di,312
asm mov   cx,2
asm rep   movsd
asm add   di,312
asm mov   cx,2
asm rep   movsd
asm add   di,312
asm mov   cx,2
asm rep   movsd
asm add   di,312
asm mov   cx,2
asm rep   movsd
asm add   di,312
asm mov   cx,2
asm rep   movsd
asm add   di,312
asm mov   cx,2
asm rep   movsd
asm add   di,312
asm mov   cx,2
asm rep   movsd

asm sub   di,(320*7)
asm inc   bx
asm dec   dx
asm jnz   SLOOP

}

void PlotIcon(void) // AL : Icon No, DI : Pos
{

asm mov   si,OFFSET StatGFX
asm xor   ah,ah
asm shl	  ax,6
asm add   si,ax

asm mov   cx,2
asm rep   movsd

asm add   di,312
asm mov   cx,2
asm rep   movsd
asm add   di,312
asm mov   cx,2
asm rep   movsd
asm add   di,312
asm mov   cx,2
asm rep   movsd
asm add   di,312
asm mov   cx,2
asm rep   movsd
asm add   di,312
asm mov   cx,2
asm rep   movsd
asm add   di,312
asm mov   cx,2
asm rep   movsd
asm add   di,312
asm mov   cx,2
asm rep   movsd

}