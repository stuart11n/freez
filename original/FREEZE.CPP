#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <key.h>
#include <alloc.h>
//#include <c:\c\game\headers\sound.h>
#include <c:\c\game\headers\sound.c>

#define DEV		1
#define MENU    1
#define TITLE	1
#define SOUND	0

#include "game\headers\ptypes.h"
#include "game\headers\structs.h"

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

void main(void)
{
asm .8086

asm mov ax,3
asm int 16

printf("ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл\n");
printf("FREEZ is now loading...\n");
printf("ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл\n");

RVal=CPU();

if(RVal<3)
 {
 printf("\n16bit %d86 Processor Detected - 386+ required.,RVal");exit(0);
 }
if(RVal>2&&RVal<5)		printf("\n%d86 ",RVal);
if(RVal>4)				printf("\nPentium Class ");
						printf("Processor Detected - OK.");

printf("\n%lu Bytes Conventional Memory Free - OK.",(unsigned long) farcoreleft());

asm .386  						// Enable 386 Instructions
asm SMART 						// Enable Assembler Optimisation
asm WARN  ALN					// Assembler Warnings...
asm WARN  BRK
asm WARN  ICG
asm WARN  PQK

// жФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Install Keyboard Handler К
// гФФФФФФФФФФФФФФФФФФФФФФФФФФН

oldkeyb=getvect(9);  			// Save the old interrupt 9 vector
setvect(9,keyb);  				// Install new keyboard interrupt handler

ps2(0);

// жФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Install Sound Services К
// гФФФФФФФФФФФФФФФФФФФФФФФФН

#if SOUND

if(SB_Setup())
 {
 printf("\nSoundBlaster compatible detected - OK.");
 Go_Varmint();
 AudioOn=1;
 }
else
 {
 printf("\nSoundBlaster initialisation failure!\n\nCheck BLASTER variable is correct...");
 delay(1000);
 AudioOn=0;
 }

#endif

// жФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Init Random Number Seeds К
// гФФФФФФФФФФФФФФФФФФФФФФФФФФН

asm push 0
asm pop  gs
asm mov  eax,[gs:46Ch]
asm mov  SEED,eax
asm xor  eax,1D872B41h
asm mov  SEED2,eax

// жФФФФФФФФФФФЗ
// К Set Video К
// гФФФФФФФФФФФН

asm mov ax,13h
asm int 10h

fh=fopen("setup.dat","rb");	// Load gamesetup
fread(&GameSetup,1,1,fh);
fclose(fh);

asm mov bx,SEG _buffer
asm mov es,bx

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#if TITLE

 LoadTitle();

#endif

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

if(GameSetup&8)Screen(1);

LoadGame();

//if(AudioOn&&!LoadAudio())GameSetup&=254;// Turn off sound if loading sfx fails
if(!AudioOn)GameSetup&=254;				// Turn off sound if handler not installed

//Setup H/W Palette

asm mov   dx,3C8h			// Clear H/W palette
asm sub   eax,eax
asm out   dx,al
asm inc   dx
asm mov   cx,192
INIT_PAL:
asm out   dx,al
asm out   dx,al
asm out   dx,al
asm loop  INIT_PAL

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

//ReadAxis();					// ReadAxis returns 3 if there is no GamePad
//asm cmp al,3
//asm jne GAMEPAD_OK
//asm btr word ptr GameSetup,4// Turn off gamepad if one is not detected
//GAMEPAD_OK:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

STARTMENU:

#if MENU

 if(MainMenu())goto EXIT;

#endif


// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

fh=fopen("ldata.dat","rb");
fread(Level,1,sizeof(Level),fh);
fclose(fh);

asm mov Score,0
asm mov CurrentLevel,1
asm mov Lives,3

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

STARTLEVEL:

asm movzx ax,CurrentLevel		// Calculate current level's data offset
asm imul  ax,468
asm add   ax,OFFSET Level
asm mov   LevelDataP,ax

asm mov   si,LevelDataP
asm mov   al,[si+5]
asm mov	  TractionType,al

LoadLevel();

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// Initialise Weather
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm mov  si,OFFSET Drops
asm mov  cx,400

S_W_INIT:

asm {
shl  dword ptr SEED2,2		// Random Drop Generator
jnc  NO_XOR_W
xor  dword ptr SEED2,0x1D872B41
mov  eax,SEED2
}

NO_XOR_W:

asm {
and  ax,0x3FF				// Setup new droplet parameters
mov  [si],ax
sub  word ptr [si],350

and  ax,0xFF
add  ax,ax
mov  [si+2],ax

add  si,4
loop S_W_INIT
}

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Initialise Misc Values К                                              К
// гФФФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
mov		si,LevelDataP		// SI points to start of current level record
mov		ax,[si]  			// Get Sx
mov		bx,[si+2]			// Get Sy
mov		cx,[si+8]			// Get Time
mov		wposx,ax
mov		wposy,bx
mov		Timer,cx            // Setup timer variables
mov		TTimer,cx
mov     ax,Score
mov		TScore,ax

mov		SkyBright,63
mov		SkyBrightDr,0
mov		SkyBrightCt,0

or   	MCB,1               // Face Right
or   	MCB,128             // Fade In
and 	MCB,239             // No   Jump keypress
and  	MCB,247             // Not  Jumping
and 	MCB,251             // Not  Hit

mov  	Frame,0             // Init all the `bender' variables
mov 	SuperJ,0
mov  	MomLeft,0
mov  	MomRight,0
mov  	JumpStat,0
mov  	LastStat,0
mov  	Energy,100
mov  	TEnergy,100
mov		RapidFire,0
mov		SteamCount,0
mov 	atilecount,0
}

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

RVal=GameLoop();

switch (RVal)
 {
 case	0	:	Fade2Black(0);goto STARTMENU;	// Quit/No lives
 case	1	:	goto STARTLEVEL;                // Level Completed
 case	2	:	goto EXIT;                      // Fast quit
 }

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

EXIT:

ptile(1,1,1);
psprite16(1,1,1,1);
psprite32(1,1,1,1,1);
pmain();
pbridge(1,1);

#if SOUND

if(AudioOn)Dropdead_Varmint(); // Release Varmint's interrupt if installed

#endif

setvect(9,oldkeyb);  					// Restore to original interrupt 9 kbd routine

fh=fopen("setup.dat","wb");
fwrite(&GameSetup,1,1,fh);
fclose(fh);

#if TITLE

Outro();

#endif

asm mov ax,3;
asm int 16;

//printf("FREEZ (C) Digital Synapse 1996.");
//printf("\n\n\nPlease visit again...\n\n");

}

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФЗ
// К CORE GRAPHIC ROUTINES К
// гФФФФФФФФФФФФФФФФФФФФФФФН

#include "game\headers\core.h"

// лллллллллллллллллллллллллллллллллллллллллл

char GameLoop(void)
{
asm mov 	ax,SEG _buffer
asm mov 	es,ax

asm mov 	eax,dword ptr wposx       // Init World Motion Values (X & Y)
asm mov 	dword ptr DXPos,eax

// лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

START_MAIN_GAME_LOOP:

// лллллллллллллллллллллллллллллллллллллллллл

if(keys[kF1])
 {
 while(keys[kF1]);
 asm xor byte ptr GameSetup,1
 }
if(keys[kF2])
 {
 while(keys[kF2]);
 asm xor byte ptr GameSetup,2
 }
if(keys[kF3])
 {
 while(keys[kF3]);
 asm xor byte ptr GameSetup,4
 }
if(keys[kF4])
 {
 while(keys[kF4]);
 asm xor byte ptr GameSetup,8
 if(GameSetup&8)Screen(1);
 else Screen(0);
 }
if(keys[kF5])
 {
 while(keys[kF5]);
 asm xor byte ptr GameSetup,16
 }

// лллллллллллллллллллллллллллллллллллллллллл

#if DEV

if(keys[kL])asm mov LevelEnd,10
if(keys[kT])asm mov Timer,1500

if(keys[kA])
 {
 asm mov byte ptr SteamCount,8
 }

#endif

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#include "game\headers\skyrain.h"

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#include "game\headers\skyplot.h"

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#include "game\headers\bridge.h"

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#include "game\headers\backplot.h"

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#include "game\headers\tileanim.h"

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#include "game\headers\statbar.h"

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Collision Detection And Animation К                                   К
// гФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#include "game\headers\collani.h"

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Main Character Control Mechanism К                                    К
// гФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Gravity К                                                             К
// гФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
mov  	di,32160     	// DI: Screen Offset for center of sprite
mov  	cx,255			// Max distance to measure
xor  	ah,ah			// Set distance counter to zero
}

HLOOP:

asm {
mov  	al,[es:di]
cmp  	al,128
jae  	HEND

add  	di,320
inc  	ah    	     	// AH : distance to ground
loop 	HLOOP
}

HEND:

asm {
cmp  	ah,19
jbe  	ONGROUND
or   	byte ptr MCB,2
jmp  	FINISHED
}

ONGROUND:

asm and byte ptr MCB,253

FINISHED:

asm {
or   	ah,ah
jl   	HOR
cmp  	ah,19
jl   	HELSE
test 	byte ptr MCB,8
jne  	HELSE
}

HOR:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Falling К                                                             К
// гФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
movzx	bx,GravStat
cmp  	bl,129
jae 	HNGI
inc  	byte ptr GravStat
}

HNGI:

asm {
shr  	bx,5
add  	wposy,bx
jmp  	HEND2
}

HELSE:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К On Ground К                                                           К
// гФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
cmp  ah,16
ja   HEND2
mov  byte ptr GravStat,64
}

HEND2:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Terrain К  if(MC.charstat>32&&MC.charstat<35)wposy+=(MC.charstat-32); К
// К Handler К  if(MC.charstat<32&&MC.charstat>25)wposy-=(32-MC.charstat); К
// гФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
xor  bh,bh
mov  bl,ah
mov  cx,bx

cmp  ah,16
jbe  HEND3
cmp  ah,19
jae  HEND3

sub  bx,16
add  wposy,bx
}

HEND3:

asm {
cmp  ah,16
jae  HEND4
cmp  ah,4
jbe  HEND4

mov  bx,16
sub  bx,cx
sub  wposy,bx
}

HEND4:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Plot Main Character К                                                 К
// гФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
mov 	dl,MCB
mov 	ah,Frame
call    far ptr @pmain$qv
mov 	Collision,dh
}

asm {

cmp		LastStat,12
je		NOEXTRAMC

test	MCB,4						// Don't plot new head if hit
jnz		NOEXTRAMC

cmp  	byte ptr Firing,0			// Plot open mouthed head if firing
jz   	NOEXTRAMC

test 	MCB,1						// Which way is Dermot facing?
jne  	DFR
}

asm {
mov  	ax,144						// Standing/Walking Right Start Position
mov  	bx,84

test 	dh,1
jne  	J34521
mov  	bx,86
mov 	ax,142
}

J34521:

asm {
test 	MCB,8
je   	J122311
mov  	bx,84
mov  	ax,142
}

J122311:

asm {

push 	8 15 bx ax
call 	far  ptr @psprite16$qiiucuc	// Plot the new head
add  	sp,8

jmp  	NOEXTRAMC
}

DFR:

asm {
mov  	ax,160						// Standing/Walking Right Start Position
mov  	bx,84

test 	dh,1
jne  	J3452
mov  	bx,86
mov  	ax,162
}

J3452:

asm {
test 	MCB,8
je   	J12231
mov  	bx,84
mov  	ax,162
}

J12231:

asm {
push 	4 15 bx ax
call 	far  ptr @psprite16$qiiucuc	// Plot the new head
add  	sp,8
}

NOEXTRAMC:

//  PMAIN RETURN VALUES
//
//              2
//
//             кСП
//             Г Г
//  12 ФФТ 4 ФкФФФПФ 16 ТФФ 48
//       Г    ГM  Г     Г
//       Р 8 ФГ  CГФ 32 й
//            РФФФй
//            ГГ ГГ
//            ГРТйГ
//            Г Г Г
//            Г   Г
//	      	  Г 1 Г
//            Г   Г
//            РФТФй
//				Г
//
//				64

asm cmp word ptr Timer,0
asm jz  NOSWEAT
asm cmp word ptr Timer,1344
asm ja  NOSWEAT

// Sweat:

asm push 	word ptr BonusFrame 5 84 152	// Setup stack: Frame Type Y X
asm call 	far  ptr @psprite16$qiiucuc		// Plot the Bonus
asm add  	sp,8

NOSWEAT:

#include "game\headers\steam.h"
#include "game\headers\flndrain.h"

asm cmp  LastStat,12				// Get laststat
asm jbe  CONTINUE
//asm jmp  NREADYTJ      			// Skip if Dying
CONTINUE:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm mov  di,OFFSET keys			// DI : pointer to array `keys'

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Suicide К                                                             К
// гФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#if DEV

asm cmp  byte ptr [di+kS],0 	// Is 'S' pressed?
asm je   N31
asm mov  byte ptr Energy,0		// Energy = 0
N31:

#endif

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Quit К                                                                К
// гФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#if DEV

asm cmp byte ptr [di+kESC],0
asm je  N1
return(0);
N1:

#endif

asm cmp byte ptr [di+kQ],0
asm je  N16

//Fade2Black(0);

//QuitPlay();

//RSTQ:
//ReadAxis();
//asm cmp al,1
//asm je  YPQ
//asm cmp al,2
//asm je  NPQ
//asm cmp byte ptr [di+kY],0
//asm jne YPQ
//asm cmp byte ptr [di+kN],0
//asm jne NPQ
//asm jmp RSTQ
//YPQ:
return(0);

//NPQ:
//UnPause();

N16:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Left К                                                                К
// гФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm xor  al,al
asm test byte ptr GameSetup,16
asm jz   NOREADAXIS

ReadAxis();

NOREADAXIS:

// Slope Detector

asm xor   bx,bx
asm xor   cx,cx
asm cmp   byte ptr [es:36656],128	// Check Right Slope
asm setae cl
asm cmp   byte ptr [es:36624],128	// Check Left Slope
asm setae bl

asm add	  cl,cl
asm add   bl,bl

// Start LR Control

asm mov  dl,1						// Set DX to 1 if traction is normal

asm cmp  TractionType,0
asm jz   NTRAC1

asm mov  dl,HalfFrame				// Set DX to 35Hz if not normal traction
NTRAC1:

asm cmp  al,1
asm je   GPL

asm cmp  byte ptr [di+203],0
asm je   N203

GPL:

asm test byte  ptr Collision,1
asm jnz  J2031

asm mov  dl,1						// Airborne so default to normal traction

asm test byte  ptr Collision,12
asm jnz  N203            		 	// Check for airborne landscape detection

J2031:

asm test byte  ptr Collision,4		// Check for ground based ""
asm jz   J2032
asm mov  byte  ptr MomRight,0
asm jmp  N203

J2032:

asm and  byte  ptr MCB,254			// Facing Left
asm mov  byte  ptr MomLeft,0		// mleft = 0

asm cmp  byte  ptr MomRight,30		// Is mright > 30
asm jbe  J2033
asm sub  bx,5
asm add  wposx,bx
asm jmp  N203

J2033:

asm cmp  byte  ptr MomRight,15		// Is mright >= 15
asm jb   J2034
asm cmp  byte  ptr MomRight,30		// Is mright <= 30
asm ja   J2034

asm sub  word  ptr wposx,2      	// Move left 2 pels
asm add  MomRight,dl
asm add  MomRight,dl
asm jmp  N203

J2034:

asm dec  word  ptr wposx       		// Else < 15
asm add  MomRight,dl

N203:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Right К                                                               К
// гФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm cmp  al,2
asm je   GPR

asm cmp  byte ptr [di+205],0
asm je   N205

GPR:

asm test byte  ptr Collision,1
asm jnz  J2051

asm mov  dl,1                       // Airborne so default to normal traction

asm test byte  ptr Collision,48
asm jnz  N205             			// Check for airborne landscape detection

J2051:

asm test byte  ptr Collision,16		// Check for ground based ""
asm jz   J2052
asm mov  byte  ptr MomLeft,0
asm jmp  N205

J2052:

asm or   byte  ptr MCB,1			// Facing Left
asm mov  byte  ptr MomRight,0		// mleft = 0

asm cmp  byte  ptr MomLeft,30		// Is mright > 30
asm jbe  J2053
asm sub  cx,5
asm sub  wposx,cx
asm jmp  N205

J2053:

asm cmp  byte  ptr MomLeft,15		// Is mright >= 15
asm jb   J2054
asm cmp  byte  ptr MomLeft,30		// Is mright <= 30
asm ja   J2054

asm add  word  ptr wposx,2			// Move right 2  pels
asm add  MomLeft,dl
asm add  MomLeft,dl
asm jmp  N205

J2054:

asm inc  word ptr wposx				// Else < 15
asm add  MomLeft,dl					// Move right 1 pels

N205:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Momentum Handler К                                                    К
// гФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

//  PMAIN RETURN VALUES

//              2
//
//             кСП
//             Г Г
//  12 ФФТ 4 ФкФФФПФ 16 ТФФ 48
//       Г    ГM  Г     Г
//       Р 8 ФГ  CГФ 32 й
//            РФФФй
//            ГГ ГГ
//            ГРТйГ
//            Г Г Г
//            Г   Г
//	      	  Г 1 Г
//            Г   Г
//            РФТФй
//				Г
//
//			   64

asm and  MCB,191					// Character is not decelerating

asm or   al,al						// D-Pad Pressed ?
asm jnz  SKIP_TO_MN205

asm cmp  dword ptr [di+203],0		// Directional keys not pressed
asm jnz  SKIP_TO_MN205

asm or	 MCB,64    					// Character is decelerating

asm mov	 dl,25						// Set to normal traction

asm cmp  TractionType,0				// Get traction factor (Normal = 0)
asm jz   NTRAC
asm mov  dl,15						// Set slippery traction value
NTRAC:

asm test Collision,1				// Find out whether we're on the ground
asm jne  L_MF

asm mov  dl,25						// If airborne, overide normal momentum

L_MF:

asm test Collision,4				// Check for Landscape Upper Left
asm jnz  MN2052

MN20511:

asm cmp  byte  ptr MomRight,dl		// MRight
asm jb   MN2051
asm sub  word  ptr wposx,3
asm sub  byte  ptr MomRight,1		// Dec MRight
SKIP_TO_MN205:
asm jmp  MN205

MN2051:

asm cmp  byte  ptr MomRight,10		// MRight
asm jbe  MN2052
asm dec  word  ptr wposx
asm sub  byte  ptr MomRight,2		// Dec MRight
asm jmp  MN205

MN2052:

asm mov  byte  ptr MomRight,0		// MRight = 0

// Start Right Momentum ----------------//

asm test byte  ptr Collision,16		// Check for Landscape Upper Right
asm jnz  MN2055

MN20522:

asm cmp  byte  ptr MomLeft,dl
asm jb   MN2054
asm add  word  ptr wposx,3
asm sub  byte  ptr MomLeft,1
asm jmp  MN205

MN2054:

asm cmp  byte  ptr MomLeft,10
asm jbe  MN2055
asm inc  word  ptr wposx
asm sub  byte  ptr MomLeft,2
asm jmp  MN205

MN2055:

asm mov  byte  ptr MomLeft,0

MN205:								// End of Momentum

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Pause К                                                               К
// гФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#if DEV

asm cmp  byte ptr [di+kO],0
asm je   N25B
msdelay(50);
N25B:

#endif

asm cmp  byte ptr [di+kP],0
asm je   N25

Pause();

N25:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Fire К                                                                К
// гФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
mov  dx,0x201
in   al,dx

cmp  byte  ptr Firing,0
je   FIRING
dec  byte  ptr Firing				// Decrement Firing if firing
jz   FIRING							// Skip RF Fire check if delay not finished

cmp  word  ptr RapidFire,0
je   FIRING
dec  byte  ptr Firing 		// Decrement Firing again if Rapid Firing
}

FIRING:

asm {
test al,32					// Button 2
je   GPF

cmp  byte ptr [di+56],0		// Left ctrl
je   N29
}

GPF:

asm {
cmp  IceBallNo,0				// Check for max no of iceballs
je   N29

cmp  byte  ptr Firing,0			// Check okay to fire (delay)
jne  N29

mov  byte  ptr Firing,20		// Setup new iceball (Picked up in COLLANI)
}

N29:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Jump К                                                                К
// гФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm	test 	al,16				// Button 1
asm	je   	GPJ

asm	cmp  	byte ptr [di+29],0
asm	jz   	N56

GPJ:

asm test 	Collision,1			// If Dermot is on the ground ...
asm jnz  	J21

asm test 	MCB,16				// ... Skip check as to whether Jump was
asm jz  	N56S				// pressed during the last frame
								//  - skip jump processing stuff if it wasn't.
J21:

asm or		MCB,16				// Jump pressed this frame

asm mov		dl,10				// Normal Jump max value
asm cmp		SuperJ,0
asm je	    NOSJ
asm mov		dl,15				// Super Jump max value

NOSJ:

asm cmp 	JumpStat,dl
asm jae  	N56R
asm add 	JumpStat,2
asm or 		MCB,8				// Set Jumping
asm jmp     N56S

N56R:

asm mov		GravStat,64			// Reset gravity pull when at max height

N56:

asm and  	MCB,239				// Clr Jump key pressed flag

N56S:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Jumping К                                                             К
// гФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm test MCB,8						// Jumping already?
asm jz   NREADYTJ

asm dec  JumpStat

asm movzx ax,JumpStat

asm cmp  al,5
asm jae  NAE10

asm sub  wposy,ax					// Jump with gravity effect
asm jmp  NAE10OR

NAE10:

asm sub  wposy,8					// Jump with no grav

NAE10OR:

asm cmp  JumpStat,0					// @ max height
asm jnz  NJZERO
asm and  MCB,247					// Set not jumping

NJZERO:

asm test Collision,2
asm jz   NREADYTJ
asm mov  JumpStat,0					// Hit ceiling, set for fall
asm and  MCB,247					// Set not jumping
asm and  MCB,239					// Clr Jump key pressed flag

asm mov  GravStat,64				// Reset Gravity pull
asm jmp  FALLING

NREADYTJ:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Main Character Frame Control К                                        К
// гФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Dying К                                                               К
// гФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm cmp  byte  ptr LastStat,13		// If Dying (Laststat)
asm jne  NDIE

asm cmp  byte  ptr Frame,143
asm jne  NDIEINC
asm mov  byte  ptr LastStat,14
asm jmp  ENDANIM

NDIEINC:

asm inc  byte  ptr Frame
asm jmp  ENDANIM

NDIE:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Standing : Still & Teetering К                                        К
// гФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {

cmp  word  ptr MomLeft,0			// Is Dragon still?
jne  NSTILL

and  MCB,191

mov  al,Collision					// AL : charstat
and  al,65
cmp  al,65
jne  NOTTEETERING					// Weee're NOT teetering:

cmp  byte  ptr LastStat,1			// If t last frame
jne  TLELSE

cmp  byte  ptr Frame,83				// @ max frame?
jne  TFINC
mov  byte  ptr Frame,68 			// Reset
jmp  NSTILL
}

TFINC:

asm inc  byte  ptr Frame			// Goto next frame
asm jmp  NSTILL

TLELSE:

asm mov  byte  ptr Frame,68
asm mov  byte  ptr LastStat,1
asm jmp  NSTILL

NOTTEETERING:
									// WE ARE STANDING!
asm {
mov  al,MCB
test al,2
jnz  NSTILL
cmp  byte  ptr LastStat,2			// Doin' this last frame?
jne  STARTFSTANDING

cmp  byte  ptr Frame,3
jne  FSTANDINGINC

mov  byte  ptr Frame,0	 			// Reset if at max frame
jmp  NSTILL
}

FSTANDINGINC:

asm inc  byte  ptr Frame
asm jmp  NSTILL

STARTFSTANDING:

asm mov  byte  ptr Frame,0 			// Reset if at max frame
asm mov  byte  ptr LastStat,2

NSTILL:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Hit К     							           						   К
// гФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm test byte  ptr MCB,4
asm jz   NOTHIT

asm cmp  byte  ptr LastStat,3
asm jne  STARTHIT

asm cmp  byte  ptr Frame,103
asm jne  HITINC
asm mov  byte  ptr Frame,88
asm jmp  ENDHIT

HITINC:

asm inc  byte  ptr Frame
asm jmp  ENDHIT

STARTHIT:

asm mov  byte  ptr Frame,88
asm mov  byte  ptr LastStat,3

ENDHIT:

asm dec  HitCount
asm jnz  NHT
asm and  byte  ptr MCB,251
asm jmp  QUITHIT

NHT:
asm test byte  ptr Collision,1
asm jz   QUITHIT
asm test byte  ptr MCB,8
asm jnz  QUITHIT
asm and  byte  ptr MCB,251      // If not falling or jumping clr hit flag

QUITHIT:

asm jmp  ENDANIM

NOTHIT:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Slowing К                                                             К
// гФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm cmp  TractionType,0			// Is terrain traction normal?
asm jz   ENDSW					// Yes so skip this bit.

asm test MCB,64                 // Is Dermot slowing?
asm jz   ENDSW					// No, so skip this bit.

asm cmp  byte  ptr LastStat,16
asm jne  SSTART
asm cmp  byte  ptr Frame,87
asm jne  SE
asm mov  byte  ptr Frame,84
asm jmp  SW

SE:

asm inc  byte  ptr Frame
asm jmp  SW

SSTART:

asm mov  byte  ptr Frame,84
asm mov  byte  ptr LastStat,16

SW:

asm jmp  NW

ENDSW:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Walking К                                                             К
// гФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm cmp  word ptr MomLeft,0		// Cmp ML & MR to see if zero...not walking
asm jz   NW

asm test MCB,64
asm jnz  SLOWFRAMES

asm cmp  MomLeft,30
asm ja   RUNNING
asm cmp  MomRight,30
asm ja   RUNNING

SLOWFRAMES:

asm cmp  byte  ptr LastStat,4
asm jne  WSTART
asm cmp  byte  ptr Frame,35
asm jne  WE
asm mov  byte  ptr Frame,4
asm jmp  NW

WE:

asm mov  dl,HalfFrame
asm add  Frame,dl
asm jmp  NW

WSTART:

asm mov  byte  ptr Frame,4
asm mov  byte  ptr LastStat,4
asm jmp  NW

RUNNING:

asm cmp  byte  ptr LastStat,4
asm jne  RSTART
asm cmp  byte  ptr Frame,35
asm jne  RE
asm mov  byte  ptr Frame,4
asm jmp  NW

RE:

asm inc  byte  ptr Frame
asm jmp  NW

RSTART:

asm mov  byte  ptr Frame,4
asm mov  byte  ptr LastStat,4

NW:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Jumping К                                                             К
// гФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm test byte ptr MCB,8
asm je   NJ

asm cmp  byte  ptr LastStat,5
asm jne  JSTART
asm cmp  byte  ptr Frame,51
asm jne  JE
asm mov  byte  ptr Frame,36
asm jmp  ENDANIM

JE:

asm inc  byte  ptr Frame
asm jmp  ENDANIM

JSTART:

asm mov  byte  ptr Frame,36
asm mov  byte  ptr LastStat,5
asm jmp  ENDANIM

NJ:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Falling К                                                             К
// гФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

FALLING:

asm test byte ptr  MCB,2
asm je   ENDANIM

asm cmp  byte  ptr LastStat,6
asm jne  FLSTART
asm cmp  byte  ptr Frame,67
asm jne  FLE
asm mov  byte  ptr Frame,52
asm jmp  ENDANIM

FLE:

asm cmp  byte  ptr Frame,60
asm jne  NOWIND

asm push 13
asm call far ptr @ps2$quc
asm add  sp,2

NOWIND:

asm inc  byte  ptr Frame
asm jmp  ENDANIM

FLSTART:

asm mov  byte  ptr Frame,52
asm mov  byte  ptr LastStat,6

ENDANIM:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К End Main Character Control Mechanism К                                К
// гФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Zero Time Checking К                                                  К
// гФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
cmp   Timer,0
jz    SKIP_TIMER_CHECK		// Skip All, nothing needs doing

dec   Timer					// Decrement timer
jz    KILL_TIME_BONUSES		// If time is now zero - convert bonuses
jmp   SKIP_TIMER_CHECK		// Otherwise continue as normal
}

KILL_TIME_BONUSES:			// Convert the bonus

asm {

mov   cx,400				// Setup the loop
mov   si,OFFSET Bonus

}

KILL_LOOP:					// Start of the loop

asm {

cmp   byte ptr [si],7
jne   NEXT_KILL

shl   dword ptr SEED,1		// Generate random number
jnc   NO_XORa
xor   SEED,0x1D872B41
}

NO_XORa:

asm {
mov   al,byte ptr SEED
and   al,3					// Make number between 0 & 3
add   al,10					// Make number between 10 & 13

mov   [si],al				// Put RN into bonus ctrl byte
}

NEXT_KILL:

asm {

add   si,5     					// Inc SI to next bonus record
loop  KILL_LOOP					// Loop if not finished

}

SKIP_TIMER_CHECK:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Zero Energy Checking К                                                К
// гФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
cmp   	TEnergy,0				// Is energy zero?
jnz   	TIME_OK

cmp   	LastStat,13				// Start death sequence
jae   	GOIN_TO_DIE
mov   	LastStat,13
mov   	Frame,112
}

GOIN_TO_DIE:

asm {
cmp	  	LastStat,14				// Death happens...
jne   	NO_DEATH

cmp		Lives,0					// But is it your last life?
jne		NOT_GAME_OVER
}

//msdelay(1000);
return(0);                      // GAME OVER !!! :-(Delay & return)

NOT_GAME_OVER:

//msdelay(1000);					// Lose a life matey
Fade2Black(0);

asm {
dec		Lives					// Init the new life -
mov     Energy,100
mov		TEnergy,100             // - its a new beginning

mov     si,LevelDataP

mov		eax,[si]   				// Get the last stored x/y start positions
mov     dword ptr wposx,eax		// Set 'em up

mov		dx,3DAh					// Wait for Retrace and skip straight to
in 		al,dx					// buffer copy as inbetween stuff causes
test 	al,8h					// trouble with the fading
je		$-5

jmp		NOSCCS
}

NO_DEATH:

asm mov 	eax,dword ptr wposx // Init World Motion Values
asm mov 	dword ptr DXPos,eax

TIME_OK:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К End of Level! К                                                       К
// гФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
cmp 	LevelEnd,0					// Check for level completion
jz  	NOT_EOL_YET

dec 	LevelEnd					// Decrease completion delay
cmp 	LevelEnd,1					// If at 1, set to zero and end the level

jne 	LE_NE_1
dec 	LevelEnd
}

FinishedLevel();					// Call end screen with stats
return(1);

LE_NE_1:

asm {
cmp 	Timer,0						// Keep the timer going if necessary
jz  	NOT_EOL_YET
inc 	Timer
}

NOT_EOL_YET:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Sky Color Ctrl (64 to 95) К										   К
// гФФФФФФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm mov		ax,frameinc             // Day/Night Cycle Time
asm and 	ax,4095					
asm cmp		ax,4095
asm jne		CTSET
asm mov 	SkyBrightCt,1
CTSET:

asm and		ax,3				 	// Fade Speed
asm cmp		ax,3
asm jne		ENDSCCC

asm cmp		SkyBrightCt,0			// Is a fade needed ?
asm jz		ENDSCCC

asm mov  	di,(OFFSET pal + 192)
asm mov		cx,32

asm cmp		SkyBrightDr,1
asm je		SCC_SR

SCC_SS:								// Sunset
asm dec		byte ptr [di+1]
asm add		di,3
asm loop	SCC_SS

asm jmp		ENDSCC

SCC_SR:								// Sunrise
asm inc		byte ptr [di+1]
asm add		di,3
asm loop	SCC_SR

ENDSCC:

asm dec		SkyBright
asm jnz		ENDSCCC
asm mov		SkyBright,63
asm xor		SkyBrightDr,1
asm mov		SkyBrightCt,0
ENDSCCC:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Cycle Colors  (96 to 103) К										   К
// гФФФФФФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
mov  	si,(OFFSET pal + 288)
mov  	dx,[si]
mov		bl,[si+2]
mov  	cx,7
}

PAL_LOOP:

asm {
mov  	eax,[si+3]
mov  	[si],eax
add  	si,3
loop 	PAL_LOOP

mov  	[si],dx
mov		[si+2],bl
}

NOSCCSA:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
	/*
asm {
xor		al,al
out		0x43,al					// Latch count in timer 0
in		al,0x40   				// Read low byte of latched count
mov     ah,al
in		al,0x40   				// Read high byte
xchg    ah,al
mov     _E,ax
}     */

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Wait for Vertical Retrace К                       					   К
// гФФФФФФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
mov		dx,3DAh
in 		al,dx
test 	al,8h
je		$-5

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Send Cycled Colors К              									   К
// гФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

test	MCB,128					// Don't do it if we're in a fadey fader
jnz		NOSCCS

mov  	dx,0x3C8				// DX : Index port
mov  	al,64
out  	dx,al					// Send start index
inc  	dx		    			// DX : Color port <- RGB:RGB:RGB:RGB...
mov  	cx,120					// Sky and Water
mov 	si,(OFFSET pal + 192)
rep  	outsb
}

NOSCCS:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Copy Buffer to Video К            					   				   К
// гФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {

mov  	bx,ds					// Save ES & DS (DS:SI -> ES:DI)
mov  	dx,es

mov  	ds,dx
mov  	ax,0A000h				// Address of Video Memory
mov  	es,ax

sub  	di,di
sub  	si,si

mov  	cx,16000                // 16000 32bit WORDS to move

rep  	movsd					// CISC in the MIX!

mov  	es,dx					// Restore ES & DS
mov  	ds,bx
}

/*
gotoxy(19,25);printf("%d FPS",E-S);//(1850000/((S-E)));

asm {
xor		al,al
out		0x43,al					// Latch count in timer 0
in		al,0x40   				// Read low byte of latched count
mov     ah,al
in		al,0x40   				// Read high byte
xchg    ah,al
mov     _S,ax
}
*/

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Screen Fade Control К            					   К
// гФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm test	MCB,128
asm	jz		NO_FADE_IN

FadeFromBlack(0);

asm	and		MCB,127

NO_FADE_IN:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К New Life initialise К            					   				   К
// гФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {

cmp 	LastStat,14			// Need to init a new life ?
jne     NO_NEW_LIFE

mov		LastStat,2			// Do it...
mov		MomLeft,0
mov		MomRight,0
mov		SteamCount,0
mov		Frame,0

mov     ax,Score
mov		TScore,ax
mov     ax,Timer
mov		TTimer,ax
mov     al,Energy
mov		TEnergy,al

or		MCB,129
and		MCB,247
and		MCB,251

mov		si,LevelDataP
cmp		byte ptr [si+4],1	// Get direction to face
jne		START_F_LEFT
or		MCB,1				// Start level facing Right
jmp		NO_NEW_LIFE
}

START_F_LEFT:

asm	and	MCB,254				// Start level facing left


NO_NEW_LIFE:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Random Steam Generator К         					   				   К
// гФФФФФФФФФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm shl 	dword ptr SEED,1
asm jnc 	NO_XOR
asm xor 	SEED,1D872B41h
NO_XOR:

asm cmp 	word  ptr SEED,127
asm ja  	NO_START_STEAM
asm mov 	byte  ptr SteamCount,8

asm push 	9
asm call 	far ptr @ps2$quc
asm add  	sp,2

NO_START_STEAM:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm mov		HalfFrame,0
asm inc		frameinc
asm setp	HalfFrame

asm jmp 	START_MAIN_GAME_LOOP

return(2);

} //-- END OF MAIN --//
