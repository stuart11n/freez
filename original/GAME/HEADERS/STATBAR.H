asm mov di,320*191		// Clear status area
asm mov cx,9*80
asm xor eax,eax
asm rep stosd

// Convert current level number into literal string

asm mov si,OFFSET LevelString
asm xor ax,ax
asm mov al,CurrentLevel
asm mov bx,10

asm xor dx,dx
asm div bx
asm mov [si+1],dl
asm xor dx,dx
asm div bx
asm mov [si],dl

// Convert score into literal string

asm mov si,OFFSET ScoreString
asm mov ax,TScore
asm mov bx,10

asm xor dx,dx
asm div bx
asm mov [si+5],dl
asm xor dx,dx
asm div bx
asm mov [si+4],dl
asm xor dx,dx
asm div bx
asm mov [si+3],dl
asm xor dx,dx
asm div bx
asm mov [si+2],dl
asm xor dx,dx
asm div bx
asm mov [si+1],dl
asm xor dx,dx
asm div bx
asm mov [si],dl

// Convert timer into literal string

asm mov si,OFFSET TimerString
asm mov bx,10
asm mov ax,TTimer
asm shr ax,6

asm xor dx,dx
asm div bx
asm mov [si+2],dl
asm xor dx,dx
asm div bx
asm mov [si+1],dl
asm xor dx,dx
asm div bx
asm mov [si],dl
NOTCONV:

// Convert energy into literal string

asm mov si,OFFSET EnergyString
asm xor ax,ax
asm mov al,TEnergy
asm mov bx,10

asm xor dx,dx
asm div bx
asm mov [si+2],dl
asm xor dx,dx
asm div bx
asm mov [si+1],dl
asm xor dx,dx
asm div bx
asm mov [si],dl

// Convert lives into literal string

asm mov si,OFFSET LivesString
asm xor ax,ax
asm mov al,Lives
asm mov bx,10

asm xor dx,dx
asm div bx
asm mov [si+1],dl
asm xor dx,dx
asm div bx
asm mov [si],dl

// Plot numerics and icons

asm cmp word ptr Timer,0
asm jz  TMROK					// Normal if Timer = 0 secs
asm cmp word ptr Timer,1344
asm ja  TMROK					// Normal if Timer > 20 secs
asm test byte ptr frameinc,16
asm jnz FTMRICON				// Flash icon otherwise
TMROK:
asm mov di,(192*320+8)
asm mov al,12
PlotIcon();
FTMRICON:

asm mov di,320*192+18
asm mov bx,OFFSET TimerString
asm mov dx,3
PlotChars();

asm cmp byte ptr Energy,20
asm ja  NRGOK
asm test byte ptr frameinc,16
asm jnz FNRGICON
NRGOK:
asm mov di,(192*320+58)
asm mov al,10
PlotIcon();
FNRGICON:
asm mov di,320*192+68
asm mov bx,OFFSET EnergyString
asm mov dx,3
PlotChars();

asm mov di,(192*320+108)
asm mov al,11
PlotIcon();
asm mov di,320*192+118
asm mov bx,OFFSET LivesString
asm mov dx,2
PlotChars();

asm mov di,(192*320+150)
asm mov al,14
PlotIcon();
asm mov di,320*192+160
asm mov bx,OFFSET LevelString
asm mov dx,2
PlotChars();

//asm mov di,(192*320+250)
//asm mov al,13
//PlotIcon();
asm mov di,320*192+260
asm mov bx,OFFSET ScoreString
asm mov dx,6
PlotChars();

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Bonus Icons К                                                         К
// гФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
cmp		word ptr RapidFire,0
jz		RFFLASH
dec  	word ptr RapidFire			// Decrement Rapid Fire
cmp  	word ptr RapidFire,350		// Check for 5 secs left
ja   	RFOKAY
test 	byte  ptr frameinc,16
jnz  	RFFLASH
}

RFOKAY:

asm mov di,(192*320+190)
asm mov al,16
PlotIcon();

RFFLASH:

asm {
cmp		word ptr SuperJ,0
jz		SJFLASH
dec  	word ptr SuperJ			// Decrement SuperJ
cmp  	word ptr SuperJ,350		// Check for 5 secs left
ja   	SJOKAY
test 	byte  ptr frameinc,16
jnz  	SJFLASH
}
SJOKAY:

asm mov di,(192*320+210)
asm mov al,15
PlotIcon();

SJFLASH:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФЗ
// К Smooth Score/Time/Energy Transition - One Unit/FrameК                 К
// гФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
mov 	ax,Score
cmp 	TScore,ax
je  	TS_OK
jl  	TS_BW
dec 	TScore
dec 	TScore
jmp 	TS_OK
}
TS_BW:
asm add TScore,10
TS_OK:


asm {
mov 	al,Energy
cmp 	TEnergy,al
je  	TE_OK
jl  	TE_BW
dec 	TEnergy
jmp 	TE_OK
}
TE_BW:
asm inc TEnergy
TE_OK:

asm {
mov 	ax,Timer
cmp 	TTimer,ax

je  	TT_OK
jl  	TT_BW
sub 	TTimer,100

cmp 	TTimer,ax
jge 	TT_OK
mov 	TTimer,ax

jmp 	TT_OK
}

TT_BW:
asm {
add 	TTimer,100

cmp 	TTimer,ax
jle 	TT_OK
mov 	TTimer,ax
}

TT_OK:
