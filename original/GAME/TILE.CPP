extern tile1,tile2;

asm .386
asm SMART
asm WARN ALN
asm WARN BRK
asm WARN ICG
asm WARN PQK

void ptile(int xpos, int ypos, unsigned char tileno)
{
char xend,yend;

asm mov 	ax,xpos		// AX: XPOS
asm mov 	bx,ypos		// BX: YPOS

asm mov 	byte ptr xend,32
asm mov 	byte ptr yend,32

asm mov 	di,288		// Screen offset to add after each line
asm xor 	si,si 		// Start Data offset
asm xor 	dx,dx		// Add source data offset after each line

asm or  	bx,bx		// Check for Y-TOP Clipping
asm jge 	NO_YT_CLIP
asm add 	yend,bx
asm neg 	bx
asm shl 	bx,5
asm mov 	si,bx
asm xor 	bx,bx		// Y = 0
asm jmp		SKIP_Y

NO_YT_CLIP:

asm cmp 	bx,158		// Check for Y-BOTTOM Clipping
asm jle 	SKIP_Y
asm mov 	byte ptr yend,191
asm sub 	yend,bl

SKIP_Y:

asm or  	ax,ax		// Check for X-LEFT Clipping
asm jge 	NO_XL_CLIP
asm add 	xend,ax
asm sub 	si,ax
asm sub 	di,ax
asm neg 	ax
asm mov 	dx,ax		// DX: Y Loop Data Increment
asm xor 	ax,ax
asm jmp 	SKIP_X

NO_XL_CLIP:

asm cmp 	ax,288		// Check for X-RIGHT Clipping
asm jle 	SKIP_X
asm mov 	cx,320
asm sub 	cx,ax
asm mov 	xend,cl
asm mov 	di,ax
asm mov 	dx,32
asm sub 	dx,cx

SKIP_X:

//BX=YPOS:AX=XPOS

asm shl 	bx,6				// Calc screen offset
asm add 	ax,bx
asm shl 	bx,2
asm add 	ax,bx				// AX: Screen Offset - delay putting into DI

asm mov 	bx,di				// BX: Y LOOP Screen Increment

asm mov 	di,ax				// DI: Screen Offset - put soffset in DI now

// SET UP SOURCE AND DESTINATION OFFSETS

asm push 	ds		    		// Save DS 'cos we use & abuse it

asm mov 	ax,SEG tile2		// Default to tset2
asm mov 	ds,ax

asm mov 	al,tileno

asm cmp 	al,95
asm jbe 	TRANS

// Plot a sprite without transparency

asm sub 	al,64
asm shl 	ax,10

asm add 	si,ax

asm cmp 	byte ptr xend,32
asm jne 	SLOW_PLOT				// Do a clipped plot if not full width

asm mov 	ah,yend				// Init _FAST_ DWORD plot

LOOPY:

asm	mov  	cx,8
asm	rep  	movsd
asm	add  	di,bx

asm	dec 	ah
asm	jnz  	LOOPY

asm	pop 	ds// di si bp
//asm ret
return;

SLOW_PLOT:						// Clipped plot

asm xor  	cx,cx				// Clear CX so REP doesn't cry
asm mov		al,yend			// Get Height
asm mov		ah,xend			// Get Width

LOOPY2:

asm mov  	cl,ah				// Restore width
asm test 	cl,1				// Is Width ODD ??
asm jz   	EVENPLOT
asm movsb

EVENPLOT:

asm test 	cl,2				// Is Width divisible by 2 and not 4 ??
asm jz   	DWORDPLOT
asm movsw

DWORDPLOT:

asm shr  	cl,2				// Calc Number of DWORD's left in Width
asm rep  	movsd

asm add  	di,bx               // Add data & screen offsets
asm add  	si,dx

asm dec  	al
asm jnz  	LOOPY2				// Y Loop

asm pop 	ds// di si bp
//asm ret
return;

TRANS:

// Plot a Sprite with transparency

asm cmp 	al,63
asm ja 		S1
asm mov 	cx,SEG tile1
asm mov 	ds,cx
asm sub 	al,64
S1:

asm shl 	ax,10
asm add 	si,ax

asm cmp   	byte ptr xend,32
asm je	   	FAST
asm jmp   	DO_CLP
FAST:
asm add   	bx,32
asm mov   	cl,yend

NPYL:
asm lodsw
asm cmp 	al,255			// Check for empty line
asm jne 	CTXP
asm add 	si,30
asm jmp 	TXP				// Skip if whole line if txp
CTXP:
asm or  	al,al; asm je  GO1; asm mov [es:di],al;   GO1:
asm or  	ah,ah; asm je  GO2; asm mov [es:di+1],ah; GO2:

asm lodsw
asm or  	al,al; asm je  GO3; asm mov [es:di+2],al; GO3:
asm or  	ah,ah; asm je  GO4; asm mov [es:di+3],ah; GO4:

asm lodsw
asm or  	al,al; asm je  GO12; asm mov [es:di+4],al; GO12:
asm or  	ah,ah; asm je  GO22; asm mov [es:di+5],ah; GO22:

asm lodsw
asm or  	al,al; asm je  GO32; asm mov [es:di+6],al; GO32:
asm or  	ah,ah; asm je  GO42; asm mov [es:di+7],ah; GO42:

asm lodsw
asm or  	al,al; asm je  GO13; asm mov [es:di+8],al; GO13:
asm or  	ah,ah; asm je  GO23; asm mov [es:di+9],ah; GO23:

asm lodsw
asm or  	al,al; asm je  GO33; asm mov [es:di+10],al; GO33:
asm or  	ah,ah; asm je  GO43; asm mov [es:di+11],ah; GO43:

asm lodsw
asm or  	al,al; asm je  GO14; asm mov [es:di+12],al; GO14:
asm or  	ah,ah; asm je  GO24; asm mov [es:di+13],ah; GO24:

asm lodsw
asm or  	al,al; asm je  GO34; asm mov [es:di+14],al; GO34:
asm or  	ah,ah; asm je  GO44; asm mov [es:di+15],ah; GO44:

asm lodsw
asm or  	al,al; asm je  GO15; asm mov [es:di+16],al; GO15:
asm or  	ah,ah; asm je  GO25; asm mov [es:di+17],ah; GO25:

asm lodsw
asm or  	al,al; asm je  GO35; asm mov [es:di+18],al; GO35:
asm or  	ah,ah; asm je  GO45; asm mov [es:di+19],ah; GO45:

asm lodsw
asm or  	al,al; asm je  GO16; asm mov [es:di+20],al; GO16:
asm or  	ah,ah; asm je  GO26; asm mov [es:di+21],ah; GO26:

asm lodsw
asm or  	al,al; asm je  GO36; asm mov [es:di+22],al; GO36:
asm or  	ah,ah; asm je  GO46; asm mov [es:di+23],ah; GO46:

asm lodsw
asm or  	al,al; asm je  GO17; asm mov [es:di+24],al; GO17:
asm or  	ah,ah; asm je  GO27; asm mov [es:di+25],ah; GO27:

asm lodsw
asm or  	al,al; asm je  GO37; asm mov [es:di+26],al; GO37:
asm or  	ah,ah; asm je  GO47; asm mov [es:di+27],ah; GO47:

asm lodsw
asm or  	al,al; asm je  GO18; asm mov [es:di+28],al; GO18:
asm or  	ah,ah; asm je  GO28; asm mov [es:di+29],ah; GO28:

asm lodsw
asm or  	al,al; asm je  GO38; asm mov [es:di+30],al; GO38:
asm or  	ah,ah; asm je  GO48; asm mov [es:di+31],ah; GO48:

TXP:
asm add  	di,bx
asm dec  	cl
asm jz   	RET1
asm jmp  	NPYL

RET1:
asm pop 	ds// di si bp
//asm ret
return;

DO_CLP:

YCT:

asm test 	byte ptr xend,1
asm jz   	EVENPLOT2

asm lodsb
asm cmp 	al,255
asm je  	SKIP_CLR_BYTE
asm or  	al,al
asm je  	SKIP_CLR_BYTE
asm mov 	[es:di],al

SKIP_CLR_BYTE:

asm inc 	di

EVENPLOT2:

asm test 	byte ptr xend,2
asm jz   	DWORDPLOT2

asm lodsw
asm cmp 	al,255
asm je  	AL_OKAY
asm or  	al,al
asm je  	AL_OKAY
asm mov 	[es:di],al
AL_OKAY:
asm or  	ah,ah
asm je  	AH_OKAY
asm mov 	[es:di+1],ah
AH_OKAY:

asm add 	di,2

DWORDPLOT2:

asm movzx 	cx,xend

asm cmp   	byte ptr [si],0FFh
asm jne   	TXPDC
asm and   	cx,0FFFCh
asm add   	si,cx
asm add   	di,cx
asm jmp   	TXPCE
TXPDC:

asm shr   	cx,2
asm jz		TXPCE

DWRDL:

asm lodsw
asm or  	al,al; asm je  GO121; asm mov [es:di],al;   GO121:
asm or  	ah,ah; asm je  GO221; asm mov [es:di+1],ah; GO221:

asm lodsw
asm or  	al,al; asm je  GO321; asm mov [es:di+2],al; GO321:
asm or  	ah,ah; asm je  GO421; asm mov [es:di+3],ah; GO421:

asm add  	di,4

asm loop 	DWRDL

TXPCE:
asm add 	di,bx
asm add 	si,dx

asm dec 	byte ptr yend
asm jnz 	YCT

EXIT:

asm pop 	ds
}
