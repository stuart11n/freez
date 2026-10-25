extern buffer,ScoreGFX,ScoreString;

asm .386
asm SMART

void pscore(void)
{
asm mov di,0				// DI: Screen Offset
asm mov bx,OFFSET ScoreString

asm mov dx,6

SLOOP:

asm mov si,OFFSET ScoreGFX
asm xor ax,ax
asm mov al,[bx]				// Get 1st number
asm shl ax,6
asm add si,ax

asm mov   cx,2
asm movsd
asm mov   cx,2
asm movsd
asm mov   cx,2
asm movsd
asm mov   cx,2
asm movsd
asm mov   cx,2
asm movsd
asm mov   cx,2
asm movsd
asm mov   cx,2
asm movsd
asm mov   cx,2
asm movsd

asm inc bx
asm dec dx
asm jnz SLOOP

}