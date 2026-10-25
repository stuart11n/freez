if(GRID)
{

// X GRID
asm {
mov ax,wposx
cmp PGRID,0
je  NGRID
sal ax,1
}
NGRID:
asm {
sar ax,1
mov bx,ax
shr ax,5
shl ax,5
sub bx,ax
mov cx,31
sub cx,bx
xchg cx,bx
xor si,si
mov cx,192
}

asm mov al,GRIDCOL

XG:
asm {
mov byte ptr [es:bx+si],al
mov byte ptr [es:bx+si+32],al
mov byte ptr [es:bx+si+64],al
mov byte ptr [es:bx+si+96],al
mov byte ptr [es:bx+si+128],al
mov byte ptr [es:bx+si+160],al
mov byte ptr [es:bx+si+192],al
mov byte ptr [es:bx+si+224],al
mov byte ptr [es:bx+si+256],al
mov byte ptr [es:bx+si+288],al
add si,320
loop XG

mov ax,wposy // Y GRID
cmp PGRID,0
je  NGRIDY
sal ax,1
}
NGRIDY:
asm {
sar ax,1
mov bx,ax
shr ax,5
shl ax,5
sub bx,ax
mov cx,31
sub cx,bx
xchg cx,bx

mov ax,bx
shl ax,8
shl bx,6
add bx,ax

xor si,si
mov cx,320
}

asm mov al,GRIDCOL

YG:
asm mov byte ptr [es:bx+si],al
asm mov byte ptr [es:bx+si+10240],al
asm mov byte ptr [es:bx+si+20480],al
asm mov byte ptr [es:bx+si+30720],al
asm mov byte ptr [es:bx+si+40960],al
asm mov byte ptr [es:bx+si+51200],al
asm inc si
asm loop YG
}

asm mov cx,8		//CrossHair
asm mov si,30544
XC:
asm mov dword ptr [es:si],01010101h
asm add si,4
asm loop XC

asm mov cx,32
asm mov si,25759
YC:
asm mov byte ptr [es:si],1
asm add si,320
asm loop YC

// END OF GRID PLOTTER
