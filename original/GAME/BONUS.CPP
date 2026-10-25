extern buffer,BTYPE;

asm .386
asm SMART

void psprite16(int xpos, int ypos, unsigned char sprite, unsigned char frame)
{
unsigned char loopx,loopy,yend,xend,xend2;
unsigned int  doffset,addsoffs;

doffset=0;
addsoffs=304;
xend=yend=16;

asm mov bx,ypos		// BX: YPOS

asm cmp bx,0		// Check for Y-TOP Clipping
asm jge short NO_YT_CLIP
asm add yend,bx
asm neg bx
asm shl bx,4
asm mov doffset,bx
asm xor bx,bx
asm jmp short SKIP_Y:

NO_YT_CLIP:

asm cmp bx,175		// Check for Y-BOTTOM Clipping
asm jle short SKIP_Y
asm mov yend,byte ptr 192
asm sub yend,bl

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

asm cmp 	ax,304		// Check for X-RIGHT Clipping
asm jle 	short SKIP_X
asm mov 	cx,320
asm sub 	cx,ax
asm mov 	xend,cl
asm mov 	addsoffs,ax
asm mov 	dx,16
asm sub 	dx,cx

SKIP_X:

asm shl 	bx,6		// Calculate Screen Offset
asm add 	ax,bx
asm shl 	bx,2
asm add 	ax,bx
asm mov 	di,ax		// DI: Screen Offset

// SET UP SOURCE AND DESTINATION OFFSETS

asm mov 	si,OFFSET BTYPE // Bonus Data Resides in DS

asm movzx 	ax,sprite
asm shl 	ax,11
asm add 	si,ax

asm mov 	al,frame
asm and 	ax,0x00FC
asm shl 	ax,6

asm add 	ax,doffset
asm add 	si,ax

asm mov 	bx,addsoffs	// BX: Y LOOP Screen Increment

if(xend==16)
 {
 asm add bx,16

 for(loopy=0;loopy<yend;loopy++)
  {
   asm lodsw
   asm cmp al,0
   asm je short GO1
   asm mov [es:di],al
   GO1:
   asm or  ah,ah
   asm je short GO2
   asm mov [es:di+1],ah
   GO2:
   asm lodsw
   asm or  al,al
   asm je short GO3
   asm mov [es:di+2],al
   GO3:
   asm or  ah,ah
   asm je short GO4
   asm mov [es:di+3],ah
   GO4:

   asm lodsw
   asm or  al,al
   asm je short GO12
   asm mov [es:di+4],al
   GO12:
   asm or  ah,ah
   asm je short GO22
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

  asm add  di,bx
  }
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

  xend=xend>>2;

  for(loopx=0;loopx<xend;loopx++)
   {
   asm lodsw

   asm or  al,al
   asm je short GO121
   asm mov [es:di],al

   GO121:
   asm or  ah,ah
   asm je short GO221
   asm mov [es:di+1],ah

   GO221:

   asm lodsw

   asm or  al,al
   asm je short GO321
   asm mov [es:di+2],al

   GO321:
   asm or  ah,ah
   asm je short GO421
   asm mov [es:di+3],ah

   GO421:

   asm add  di,4
   }
  asm add di,bx
  asm add si,dx
  }
}
