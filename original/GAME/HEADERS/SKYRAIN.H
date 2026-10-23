// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Sky Gradient К                                                        К
// гФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

// COLOR RANGE : INDEX 64-95 INC.

asm {

mov  bx,wposy
mov  dx,bx

shr  bl,2		// Get start color
and  bl,31		// Place in DWORD
mov  bh,bl

mov  ax,bx
shl  eax,16
mov  ax,bx              // EAX : Start color DWORD

not  dx			// 2-1-0 : First color length operator (!!)
and  dx,3

xor  cl,cl
mov  ch,dl
shl  dx,6
add  cx,dx
shr  cx,2		// CX : DWORD's of first color to plot

xor  di,di		// Set to start of screen memory

add  eax,40404040h      // Add 32

rep  stosd		// Write the first color

add  eax,01010101h   	// 0-31
and  eax,1f1f1f1fh   	// Round 32
add  eax,40404040h      // Add 32

mov  bl,64
}

SKY:

asm {
mov  cx,240
rep  stosd

add  eax,01010101h
and  eax,1f1f1f1fh
add  eax,40404040h

dec  bl
jnz  SKY
}

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Lightning К             		                                       К
// гФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm  	test 	byte ptr GameSetup,2
asm  	jnz 	START_LGHT
SKIP_LGHT:
asm     jmp		NO_WEATHER1
START_LGHT:

asm  	cmp 	SkyBrightDr,0				// Is sun setting?
asm  	jz      SSETTING
											// It's Rising
asm		cmp		SkyBright,30
asm		jbe		SKIP_LGHT
asm     jmp		RISE_CONT

SSETTING:

asm 	cmp		SkyBright,30				// Check in right phase
asm		ja		SKIP_LGHT

RISE_CONT:

asm 	shl  	dword ptr SEED2,1
asm 	jnc  	RND1
asm 	xor  	dword ptr SEED2,0x1D872B41
asm 	mov  	ax,word ptr SEED2			// Number to decide wether to strike
asm		mov		dx,word ptr SEED2+2			// Number from which x start is calced
RND1:

asm 	cmp 	al,20						// Lower for less frequent strikes
asm		ja 		SK_LGHT

asm		and		ax,3
asm		mov		STK4,ax						// STK4 : Gradient factor
asm 	and		dx,255
asm 	add		dx,32						// DX : X start
asm		cmp		dx,160
asm		jb		IOK
asm		neg		STK4						// Reverse grad if starting in 2nd half
IOK:

// Plot the lightning

asm		mov		di,OFFSET Lightning
asm 	mov		cx,40

asm		mov		STK1,dx						// DX : X Start Position
asm		mov		STK2,0
asm 	xor		bx,bx						// Gradient Offset

D_LOOP:

asm		shl  	dword ptr SEED2,1
asm		jnc  	RND2
asm		xor  	dword ptr SEED2,0x1D872B41
asm		mov  	ax,word ptr SEED2
RND2:

asm     and     ax,15						// Get random offset
asm		add		ax,dx                       // Add start pos
asm		add		ax,bx						// Add gradient factor
asm		mov		STK3,ax

asm 	push	ax bx cx dx di
				Line(STK1,STK2,STK3,STK2+5);// Draw current segment
asm 	pop		di dx cx bx ax

asm		mov		STK1,ax
asm		add		STK2,5

asm		add		bx,STK4
asm		add		di,2

asm		loop	D_LOOP

SK_LGHT:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Background Rain К                                                     К
// гФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {

mov  ax,wposx
mov  bx,wposy
sub  DXPos,ax
sub  DYPos,bx

mov  bx,DXPos
mov  dx,DYPos
mov  DXPos2,bx		       	// Save values for foreground routine
mov  DYPos2,dx

mov  si,OFFSET Drops
mov  cx,200
}

WEATHER_DROPS:

asm {

add  [si],bx                	// Apply world motion for last frame
add  [si+2],dx

mov  di,[si]			// Calc Screen offset and clip at same time
cmp  di,0
jl   NO_PLOT
cmp  di,319
jg   NO_PLOT

mov  ax,[si+2]
cmp  ax,0
jl   NO_PLOT
cmp  ax,190
jg   NO_PLOT

shl  ax,6
add  di,ax
shl  ax,2
add  di,ax

mov  al,byte ptr SEED2
xor  ax,si
and  al,7
add  al,96                  	// Set to Cycled Color

stosb                       	// Plot droplet
}

NO_PLOT:

asm {
add  word ptr [si+2],2		// Check if drop is OK or needs replacing
cmp  word ptr [si+2],350
jg   RENEW
cmp  word ptr [si+2],-300
jge  DROP_OK
}

RENEW:

asm {
shl  dword ptr SEED2,1		// Random Drop Generator
jnc  NO_XOR_W
xor  dword ptr SEED2,0x1D872B41
mov  eax,SEED2
}

NO_XOR_W:

asm {
and  ax,0x3FF			// Setup new droplet parameters (0 - 1023)
sub  ax,352
mov  [si],ax			// Write new X value
and  ax,0Fh
mov  [si+2],ax
}

DROP_OK:

asm {
add  si,4
loop WEATHER_DROPS		// Loop

mov eax,dword ptr wposx     	// Init World Motion Values
mov dword ptr DXPos,eax
}

NO_WEATHER1:
