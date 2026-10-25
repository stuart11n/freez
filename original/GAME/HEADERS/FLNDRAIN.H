// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Foreground Land Tile Plotter К                                        К
// гФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
						// Setup Tile Loop
mov	 dx,FYpos           // Restore previously calculated stuff
mov  ByteLoop2,7			// Init Y loop
}

TYLOOP2:

asm {
mov  cx,FTileX
mov  ByteLoop,11			// Init X loop
}

TXLOOP2:

asm {
cmp  dx,190
jg   OFFLIMITS2
cmp  dx,-31
jl   OFFLIMITS2
cmp  cx,319
jg   OFFLIMITS2
cmp  cx,-31
jl   OFFLIMITS2

mov  bx,FTileIndex

mov	 al,[OFFSET tilemap+bx]	// Check it's a background tile
btr  ax,7
jnc	 OFFLIMITS2
or   al,al					// Check for transparent tile
je   OFFLIMITS2

push ax dx cx
call far ptr @ptile$qiiuc
pop  cx dx
add  sp,2 					// Correct stack
}

OFFLIMITS2:

asm {
add  cx,32
inc  FTileIndex

dec  ByteLoop
jnz  TXLOOP2			// Goto X loop

add  dx,32
add  FTileIndex,117

dec  ByteLoop2
jnz  TYLOOP2			// Goto Y loop
}

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Foreground Rain К                                                     К
// гФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm  	jmp     START_FR
SKIP_FR:
asm     jmp		NO_WEATHER2
START_FR:

asm 	test	byte ptr GameSetup,2//cmp		RAIN,1
asm 	jz		SKIP_FR

asm  	cmp 	SkyBrightDr,0				// Is sun setting?
asm  	jz      GOFR
											// It's Rising
asm		cmp		SkyBright,30
asm		jbe		SKIP_FR
asm		jmp		STFR

GOFR:

asm 	cmp		SkyBright,30				// Check in right phase
asm		ja		SKIP_FR

STFR:

asm {

mov  	si,(OFFSET Drops+800)
mov  	cx,200

mov  	bx,DXPos2 				// Apply world motion for last frame
mov 	dx,DYPos2
}

WEATHER_DROPS_F:

asm {
add  	[si],bx
add  	[si+2],dx

mov  	di,[si]					// Calc Screen offset and clip at same time
or   	di,di
jl   	NO_PLOT_F
cmp  	di,319
jg   	NO_PLOT_F

mov  	ax,[si+2]
or   	ax,ax
jl   	NO_PLOT_F
cmp  	ax,190
jg   	NO_PLOT_F

shl  	ax,6
add  	di,ax
shl  	ax,2
add  	di,ax

mov  	al,byte ptr SEED2
xor  	ax,si
and  	al,7
add  	al,96 					// Set to Cycled Color

stosb                       	// Plot droplet
}

NO_PLOT_F:

asm {
add  	word ptr [si+2],3		// Check if drop is OK or needs replacing
cmp  	word ptr [si+2],350
jg   	RENEW_F
cmp  	word ptr [si+2],-300
jge  	DROP_OK_F
}

RENEW_F:

asm {
shl  	dword ptr SEED2,1		// Random Drop Generator
jnc  	NO_XOR_W_F
xor  	dword ptr SEED2,0x1D872B41
mov  	eax,SEED2
}

NO_XOR_W_F:

asm {
and  	ax,0x3FF				// Setup new droplet parameters (0 - 1023)
sub  	ax,352
mov  	[si],ax					// Write new X value
and  	ax,0x0F
mov  	[si+2],ax
}

DROP_OK_F:

asm add  si,4
asm loop WEATHER_DROPS_F		// Loop to next droplet

NO_WEATHER2:
