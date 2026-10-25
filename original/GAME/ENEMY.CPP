extern enemy0,enemy1,enemy2,enemy3;

asm .386
asm SMART

void psprite32(int xpos, int ypos, unsigned char sprite, unsigned char frame,signed char speed)
{
unsigned char loopx,loopy,yend,xend,xend2;
unsigned int  doffset,addsoffs;

doffset=0;
addsoffs=288;
xend=yend=32;

asm xor dx,dx

asm mov bx,ypos		// BX: YPOS

asm or  bx,bx		// Check for Y-TOP Clipping
asm jge short NO_YT_CLIP
asm add yend,bx
asm neg bx
asm shl bx,5
asm mov doffset,bx
asm xor bx,bx
asm jmp short SKIP_Y:

NO_YT_CLIP:

asm cmp bx,159		// Check for Y-BOTTOM Clipping
asm jle short SKIP_Y
asm mov byte ptr yend,192
asm sub byte ptr yend,bl

SKIP_Y:

asm mov ax,xpos		// AX: XPOS

asm cmp ax,0		// Check for X-LEFT Clipping
asm jge short NO_XL_CLIP
asm add xend,ax
asm sub doffset,ax
asm sub addsoffs,ax
asm neg ax
asm mov dx,ax		// DX: Y Loop Data Increment
asm xor ax,ax
asm jmp short SKIP_X

NO_XL_CLIP:

asm cmp ax,288		// Check for X-RIGHT Clipping
asm jle short SKIP_X
asm mov cx,320
asm sub cx,ax
asm mov xend,cl
asm mov addsoffs,ax
asm mov dx,32
asm sub dx,cx

SKIP_X:

// BX = YPOS :: AX = XPOS

asm shl bx,6
asm add ax,bx
asm shl bx,2
asm add ax,bx
asm mov di,ax		// DI: Screen Offset

// SET UP SOURCE AND DESTINATION OFFSETS

asm push ds

asm mov al,byte ptr sprite

asm cmp al,0
asm jne short E2
asm mov si,SEG enemy0
asm mov ds,si
asm mov si,OFFSET enemy0
asm jmp short EX

E2:
asm cmp al,1
asm jne short E3
asm mov bx,SEG enemy1
asm mov ds,bx
asm mov si,OFFSET enemy1
asm jmp short EX

E3:
asm cmp al,2
asm jne short E4
asm mov bx,SEG enemy2
asm mov ds,bx
asm mov si,OFFSET enemy2
asm jmp short EX

E4:
asm mov bx,SEG enemy3
asm mov ds,bx
asm mov si,OFFSET enemy3

EX:

asm cmp  byte ptr speed,0
asm jl   short REVERSE		// Facing Left so do reverse plot
asm jmp  NORMAL  		// Do normal plot...
REVERSE:

asm mov bh,frame
asm and bx,0xFC00
asm add bx,doffset
asm add si,bx		// Source data offset

asm mov bx,addsoffs	// BX: Y LOOP Screen Increment

if(xend==32)
 {
 asm add bx,32

 asm xor ch,ch
 asm mov cl,yend

 YLOOP1:

   asm lodsw
   asm or  al,al
   asm je  short GOA1
   asm mov [es:di+31],al
   GOA1:
   asm or  ah,ah
   asm je  short GOA2
   asm mov [es:di+30],ah
   GOA2:
   asm lodsw
   asm or  al,al
   asm je  short GOA3
   asm mov [es:di+29],al
   GOA3:
   asm or  ah,ah
   asm je  short GOA4
   asm mov [es:di+28],ah
   GOA4:

   asm lodsw
   asm or  al,al
   asm je  short GOA12
   asm mov [es:di+27],al
   GOA12:
   asm or  ah,ah
   asm je  short GOA22
   asm mov [es:di+26],ah
   GOA22:
   asm lodsw
   asm or  al,al
   asm je short GOA32
   asm mov [es:di+25],al
   GOA32:
   asm or  ah,ah
   asm je short GOA42
   asm mov [es:di+24],ah
   GOA42:

   asm lodsw
   asm or  al,al
   asm je short GOA13
   asm mov [es:di+23],al
   GOA13:
   asm or  ah,ah
   asm je short GOA23
   asm mov [es:di+22],ah
   GOA23:
   asm lodsw
   asm or  al,al
   asm je short GOA33
   asm mov [es:di+21],al
   GOA33:
   asm or  ah,ah
   asm je short GOA43
   asm mov [es:di+20],ah
   GOA43:

   asm lodsw
   asm or  al,al
   asm je short GOA14
   asm mov [es:di+19],al
   GOA14:
   asm or  ah,ah
   asm je short GOA24
   asm mov [es:di+18],ah
   GOA24:
   asm lodsw
   asm or  al,al
   asm je short GOA34
   asm mov [es:di+17],al
   GOA34:
   asm or  ah,ah
   asm je short GOA44
   asm mov [es:di+16],ah
   GOA44:

   asm lodsw
   asm or  al,al
   asm je short GOA15
   asm mov [es:di+15],al
   GOA15:
   asm or  ah,ah
   asm je short GOA25
   asm mov [es:di+14],ah
   GOA25:
   asm lodsw
   asm or  al,al
   asm je short GOA35
   asm mov [es:di+13],al
   GOA35:
   asm or  ah,ah
   asm je short GOA45
   asm mov [es:di+12],ah
   GOA45:

   asm lodsw
   asm or  al,al
   asm je short GOA16
   asm mov [es:di+11],al
   GOA16:
   asm or  ah,ah
   asm je short GOA26
   asm mov [es:di+10],ah
   GOA26:
   asm lodsw
   asm or  al,al
   asm je short GOA36
   asm mov [es:di+9],al
   GOA36:
   asm or  ah,ah
   asm je short GOA46
   asm mov [es:di+8],ah
   GOA46:

   asm lodsw
   asm or  al,al
   asm je short GOA17
   asm mov [es:di+7],al
   GOA17:
   asm or  ah,ah
   asm je short GOA27
   asm mov [es:di+6],ah
   GOA27:
   asm lodsw
   asm or  al,al
   asm je short GOA37
   asm mov [es:di+5],al
   GOA37:
   asm or  ah,ah
   asm je short GOA47
   asm mov [es:di+4],ah
   GOA47:

   asm lodsw
   asm or  al,al
   asm je short GOA18
   asm mov [es:di+3],al
   GOA18:
   asm or  ah,ah
   asm je short GOA28
   asm mov [es:di+2],ah
   GOA28:
   asm lodsw
   asm or  al,al
   asm je short GOA38
   asm mov [es:di+1],al
   GOA38:
   asm or  ah,ah
   asm je short GOA48
   asm mov [es:di],ah
   GOA48:

  asm add  di,bx

  asm dec  cx
  asm jz   short QUIT1
  asm jmp  YLOOP1

 QUIT1:
 asm pop ds
 return;
 }

asm add  si,31				// Goto new start of data
asm cmp  xpos,0
asm jge  short RIGHTCLIPPING// OK if Right Clipping
asm sub  si,dx              // Otherwise setup SI properly
asm sub  si,dx
RIGHTCLIPPING:

asm sub  dx,64				// PreCorrect reversed SI

xend2=xend;

asm std

 for(loopy=0;loopy<yend;loopy++)
  {
  xend=xend2;

  asm shr   byte ptr xend,1
  asm jnc   short EVENPLOT2
  asm lodsb
  asm or    al,al
  asm jz    short NO_O_BYTE
  asm mov   [es:di],al
  NO_O_BYTE:
  asm inc   di
  EVENPLOT2:

  for(loopx=0;loopx<xend;loopx++)
   {
   asm lodsb
   asm mov   ah,al
   asm lodsb

   asm or    ah,ah
   asm jz    NO_L_BYTE
   asm mov   [es:di],ah
   NO_L_BYTE:
   asm inc   di

   asm or    al,al
   asm jz    NO_H_BYTE
   asm mov   [es:di],al
   NO_H_BYTE:
   asm inc   di
   }
  asm add di,bx
  asm sub si,dx
  }

asm cld

asm pop ds
return;

// End of reverse Plot // End of reverse Plot // End of reverse Plot

NORMAL:

asm mov bh,frame
asm and bx,0FC00h
asm add bx,doffset
asm add si,bx		// Source data offset

asm mov bx,addsoffs	// BX: Y LOOP Screen Increment

if(xend==32)
 {
 asm add bx,32

 asm xor ch,ch
 asm mov cl,yend

 YLOOP2:

   asm lodsw
   asm or  al,al
   asm je  short GO1
   asm mov [es:di],al
   GO1:
   asm or  ah,ah
   asm je  short GO2
   asm mov [es:di+1],ah
   GO2:
   asm lodsw
   asm or  al,al
   asm je  short GO3
   asm mov [es:di+2],al
   GO3:
   asm or  ah,ah
   asm je  short GO4
   asm mov [es:di+3],ah
   GO4:

   asm lodsw
   asm or  al,al
   asm je  short GO12
   asm mov [es:di+4],al
   GO12:
   asm or  ah,ah
   asm je  short GO22
   asm mov [es:di+5],ah
   GO22:
   asm lodsw
   asm or  al,al
   asm je short GO32
   asm mov [es:di+6],al
   GO32:
   asm or  ah,ah
   asm je short GO42
   asm mov [es:di+7],ah
   GO42:

   asm lodsw
   asm or  al,al
   asm je short GO13
   asm mov [es:di+8],al
   GO13:
   asm or  ah,ah
   asm je short GO23
   asm mov [es:di+9],ah
   GO23:
   asm lodsw
   asm or  al,al
   asm je short GO33
   asm mov [es:di+10],al
   GO33:
   asm or  ah,ah
   asm je short GO43
   asm mov [es:di+11],ah
   GO43:

   asm lodsw
   asm or  al,al
   asm je short GO14
   asm mov [es:di+12],al
   GO14:
   asm or  ah,ah
   asm je short GO24
   asm mov [es:di+13],ah
   GO24:
   asm lodsw
   asm or  al,al
   asm je short GO34
   asm mov [es:di+14],al
   GO34:
   asm or  ah,ah
   asm je short GO44
   asm mov [es:di+15],ah
   GO44:

   asm lodsw
   asm or  al,al
   asm je short GO15
   asm mov [es:di+16],al
   GO15:
   asm or  ah,ah
   asm je short GO25
   asm mov [es:di+17],ah
   GO25:
   asm lodsw
   asm or  al,al
   asm je short GO35
   asm mov [es:di+18],al
   GO35:
   asm or  ah,ah
   asm je short GO45
   asm mov [es:di+19],ah
   GO45:

   asm lodsw
   asm or  al,al
   asm je short GO16
   asm mov [es:di+20],al
   GO16:
   asm or  ah,ah
   asm je short GO26
   asm mov [es:di+21],ah
   GO26:
   asm lodsw
   asm or  al,al
   asm je short GO36
   asm mov [es:di+22],al
   GO36:
   asm or  ah,ah
   asm je short GO46
   asm mov [es:di+23],ah
   GO46:

   asm lodsw
   asm or  al,al
   asm je short GO17
   asm mov [es:di+24],al
   GO17:
   asm or  ah,ah
   asm je short GO27
   asm mov [es:di+25],ah
   GO27:
   asm lodsw
   asm or  al,al
   asm je short GO37
   asm mov [es:di+26],al
   GO37:
   asm or  ah,ah
   asm je short GO47
   asm mov [es:di+27],ah
   GO47:

   asm lodsw
   asm or  al,al
   asm je short GO18
   asm mov [es:di+28],al
   GO18:
   asm or  ah,ah
   asm je short GO28
   asm mov [es:di+29],ah
   GO28:
   asm lodsw
   asm or  al,al
   asm je short GO38
   asm mov [es:di+30],al
   GO38:
   asm or  ah,ah
   asm je short GO48
   asm mov [es:di+31],ah
   GO48:

  asm add  di,bx

  asm dec  cx
  asm jz   short QUIT2
  asm jmp  YLOOP2

 QUIT2:
 asm pop ds
 return;
 }

 xend2=xend;

 for(loopy=0;loopy<yend;loopy++)
  {
  xend=xend2;
  asm test byte ptr xend,1
  asm jp short EVENPLOT

  asm lodsb

  asm or  al,al
  asm je  short SKIP_CLR_BYTE

  asm mov [es:di],al

  SKIP_CLR_BYTE:

  asm inc di

  EVENPLOT:

  asm test byte ptr xend,2
  asm jp short DWORDPLOT

  asm lodsw

  asm or  al,al
  asm je  short AL_OKAY
  asm mov [es:di],al

  AL_OKAY:

  asm or  ah,ah
  asm je  short AH_OKAY
  asm mov [es:di+1],ah

  AH_OKAY:

  asm add di,2

  DWORDPLOT:

  asm shr byte ptr xend,2

  for(loopx=0;loopx<xend;loopx++)
   {
   asm lodsw

   asm or  al,al
   asm je short GOA121
   asm mov [es:di],al
   GOA121:

   asm or  ah,ah
   asm je short GOA221
   asm mov [es:di+1],ah
   GOA221:

   asm lodsw

   asm or  al,al
   asm je short GOA321
   asm mov [es:di+2],al
   GOA321:

   asm or  ah,ah
   asm je short GOA421
   asm mov [es:di+3],ah
   GOA421:

   asm add  di,4
   }
  asm add di,bx
  asm add si,dx
  }

asm pop ds
}
