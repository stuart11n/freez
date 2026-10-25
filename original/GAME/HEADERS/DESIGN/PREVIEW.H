// Plot Previews

if(PREVIEW)
{
asm xor di,di
asm mov eax,40404040h
asm mov cx,2560
asm rep stosd

ptile(-16,0,(TIndex-5)&127);
ptile(16,0,(TIndex-4)&127);
ptile(48,0,(TIndex-3)&127);
ptile(80,0,(TIndex-2)&127);
ptile(112,0,(TIndex-1)&127);
ptile(144,0,TIndex&127);
ptile(176,0,(TIndex+1)&127);
ptile(208,0,(TIndex+2)&127);
ptile(240,0,(TIndex+3)&127);
ptile(272,0,(TIndex+4)&127);
ptile(304,0,(TIndex+5)&127);

asm mov si,10384
asm mov cx,31
ULINE:
asm mov  byte ptr [es:si],0
asm mov  byte ptr [es:si+320],0
asm inc  si
asm loop ULINE
}
