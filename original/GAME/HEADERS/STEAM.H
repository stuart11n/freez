// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Steam Animation Control К                                             К
// гФФФФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

// Add a Puff of steam:

asm {
mov  al,SteamCount
or   al,al
jz   NOSTEAM

mov  ah,BonusFrame
and  ah,1
sub  al,ah			// Dec Steamcount by 1 per 2 frames

mov  di,OFFSET Steam// DI : offset in DS to Steam

xor  bh,bh
mov  bl,al
shl  bx,3			// BX : pointer into steam records
add  di,bx			// DI : pointer to record

mov  bx,wposx

test byte ptr MCB,1
jz   OTHERDIR

add  bx,164
mov  [di+2],bx			// Set X pos 4 right
jmp  ENDADDSTEAMX
}

OTHERDIR:

asm {
add  bx,141
mov  [di+2],bx              	// Set X pos 4 left
}

ENDADDSTEAMX:

asm {
mov  bx,wposy
add  bx,79
mov  [di+4],bx			// Set Y pos
inc  byte ptr [di]		// Set ctrl byte
mov  ah,al
shl  ah,2

mov  [di+1],ah			// Set frame
mov  SteamCount,al
}

NOSTEAM:

// Steam Animation Loop : Steam record size : 8 bytes

asm {
mov  byte ptr ByteLoop,8		// Init x loop
mov  si,OFFSET Steam
}

STEAMXL:

asm {
cmp  byte ptr [si],0		// Is steam puff active?
je   SINACTIVE

mov  ax,[si+2]
sub  ax,wposx       		// X test
mov  bx,[si+4]
sub  bx,wposy       		// Y test

cmp  ax,319                 	// Bound tests...
jg   SOFFLIMITS
cmp  ax,-16
jl   SOFFLIMITS
cmp  bx,190
jg   SOFFLIMITS
cmp  bx,-16
jl   SOFFLIMITS

mov  dl,[si+1]			// Initialise Stack
push dx	4 bx ax			// Frame,Bonus Type,Y,X

call far ptr @psprite16$qiiucuc

add  sp,8			// Correct Stack
}

SOFFLIMITS:

asm {
mov  dl,BonusFrame
and  dl,1
add  [si+1],dl			// Inc frame by one every 2 vsyncs

cmp  byte ptr [si+1],32		// Reset frame if @ max
jne  SDEC
mov  byte ptr [si],0
}

SDEC:

asm dec  word ptr [si+4] 	// Rise the steam

SINACTIVE:

asm add  si,8			// Goto next record

asm dec  ByteLoop
asm jnz  STEAMXL                // Loop
