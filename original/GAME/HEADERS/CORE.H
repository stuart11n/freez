//-----------------------------------------------------
// Name		: 	Keyb
// Input	:	None
// Output	:	None
// Function	:	Replacement keyboard handler
//-----------------------------------------------------

void interrupt keyb(...)
{

asm {

in      al,0x60         	// Get scan code in AL
cmp     al,0xE0         	// was it an E0 key?
jne     SetScanCode

mov     [e0flag],128
mov     al,20h         		// Send generic EOI to PIC
out     20h,al         		// 001 00 000
							//  |   |  |
							//  |   |  +--- INT request level
							//  |   +------ OCW2
							//  +---------- non-specific EOI command
jmp 	ENDKB

}

SetScanCode:

asm {

mov     bl,al               // Save scan code in BL
and     bx,7Fh				// clr BH
add     bl,[e0flag]
and     al,80h            	// keep break bit, if set
xor     al,80h            	// flip bit - 1 means pressed
							//          - 0 means released
rol     al,1                // put it in bit 0
mov     [keys+bx],al        // set index for key
mov     [e0flag],0

mov     al,20h          	// Send generic EOI to PIC
out     20h,al          	// 001 00 000
							//  |   |  |
							//  |   |  +--- INT request level
							//  |   +------ OCW2
							//  +---------- non-specific EOI command
}

ENDKB:

}

// Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û

void LoadLevel(void)
{
FILE *fh;

Fade2Black(0);
LoadLogo(CurrentLevel+9);		// Load level image
FadeFromBlack(0);

fh=fopen("brdg1.tga","rb");//Level[CurrentLevel].Bridge,"rb");
fseek(fh,0x312,0);
fread(BridgeGFX,1,64,fh);
fclose(fh);

fh=fopen("tset1a.tga","rb");//Level[CurrentLevel].Tiles1,"rb");
fseek(fh,0x312,0);
fread(tile1,1,13107,fh);
fread(tile1+13107,1,13107,fh);
fread(tile1+13107+13107,1,13107,fh);
fread(tile1+13107+13107+13107,1,13107,fh);
fread(tile1+13107+13107+13107+13107,1,13107,fh);
fclose(fh);

fh=fopen("tset1b.tga","rb");//Level[CurrentLevel].Tiles2,"rb");
fseek(fh,0x312,0);
fread(tile2,1,13107,fh);
fread(tile2+13107,1,13107,fh);
fread(tile2+13107+13107,1,13107,fh);
fread(tile2+13107+13107+13107,1,13107,fh);
fread(tile2+13107+13107+13107+13107,1,13107,fh);
fclose(fh);

fh=fopen("enemy1a.tga","rb");//Level[CurrentLevel].Enemy1,"rb");
fseek(fh,0x312,0);
fread(enemy0,1,19456,fh);
fclose(fh);

fh=fopen("enemy1b.tga","rb");//Level[CurrentLevel].Enemy2,"rb");
fseek(fh,0x312,0);
fread(enemy1,1,19456,fh);
fclose(fh);

//fh=fopen("enemy1c.tga","rb");//Level[CurrentLevel].Enemy3,"rb");
//fseek(fh,0x312,0);
//fread(enemy2,1,19456,fh);
//fclose(fh);

//fh=fopen("enemy1d.tga","rb");//Level[CurrentLevel].Enemy4,"rb");
//fseek(fh,0x312,0);
//fread(enemy3,1,19456,fh);
//lose(fh);

fh=fopen("tmap1.map","rb");//Level[CurrentLevel].TMap,"rb");
fread(tilemap,1,8192,fh);
fclose(fh);

fh=fopen("emap1.map","rb");//Level[CurrentLevel].EMap,"rb");
fread(Enemy,1,sizeof(Enemy),fh);
fclose(fh);

fh=fopen("bmap1.map","rb");//Level[CurrentLevel].BMap,"rb");
fread(Bonus,1,sizeof(Bonus),fh);
fclose(fh);

fh=fopen("smap1.map","rb");//Level[CurrentLevel].SMap,"rb");
fread(skymap,1,sizeof(skymap),fh);
fclose(fh);

Fade2Black(0);

fh=fopen("dragon.tga","rb");
fseek(fh,0x12,0);
fread(pal,1,768,fh);
fclose(fh);

FixPal();

asm mov  ax,SEG _buffer
asm mov  es,ax
}

// Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û
// Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û Û

void LoadGame(void)
{
FILE *fh;

fh=fopen("dragon.tga","rb");
fseek(fh,0x312,0);
fread(MCANIM,36864,1,fh);
fclose(fh);

fh=fopen("bonuses.tga","rb");
fseek(fh,0x312,0);
fread(BTYPE,1,31488,fh);
fclose(fh);

fh=fopen("statgfx.tga","rb");
fseek(fh,0x312,0);
fread(StatGFX,1,1088,fh);
fclose(fh);

asm mov  ax,SEG _buffer
asm mov  es,ax
}

#if DESIGN
#else

void LoadTitle(void)
{
FILE *fh;

fh=fopen("freez.tga","rb");  // Load title image
fseek(fh,0x12,0);
fread(FadeStore,1,768,fh);
fseek(fh,0x312,0);
fread(buffer,1,64000,fh);
fclose(fh);

asm mov cx,256
asm mov si,OFFSET FadeStore
FSCL:
asm shr  byte ptr [si],2
asm shr  byte ptr [si+1],2
asm shr  byte ptr [si+2],2
asm mov  al,[si]
asm mov  ah,[si+2]
asm mov  [si],ah
asm mov  [si+2],al
asm add  si,3
asm loop FSCL

GameSetup|=1;
SFX[0]=loadwave("intro.wav",&SFXL[0]);
if(!SFX[0])GameSetup&=254;

asm mov  di,OFFSET pal 	// Set grey palette
asm sub  al,al
asm sub  bl,bl
asm mov  cx,256
PL:
asm mov  al,bl
asm shr  al,2
asm stosb
asm stosb
asm stosb
asm inc  bl
asm loop PL

asm mov  si,OFFSET SPal      // Set black palette
asm mov  cx,768
asm mov  dx,3C8h
asm sub  al,al
asm out  dx,al
asm inc  dx
asm rep  outsb

asm push 40960
asm pop  es

asm mov  bx,64

RL:

DVR();

asm mov  cx,16000
asm sub  di,di

YL:
asm shl  SEED,1			// Plot snow
asm jnc  NO_XOR
asm xor  SEED,1D872B41h
NO_XOR:
asm mov  eax,SEED
asm stosd
asm loop YL

asm mov  si,OFFSET SPal  // Modify palette
asm mov  cx,256
ML:
asm mov  dl,[si]
asm cmp  dl,[si-768]
asm je   J1
asm add  dword ptr [si],010101h
J1:
asm add  si,3
asm loop ML

asm mov  si,OFFSET SPal 	// Send new palette
asm mov  cx,768
asm mov  dx,3C8h
asm sub  al,al
asm out  dx,al
asm inc  dx
asm rep  outsb

asm dec	 bx
asm jnz	 RL

// FINISHED FADING IN !

asm cmp  [keys+kSPACE],0
asm jz   QUICKS
asm jmp  QUICK
QUICKS:

asm mov  bx,100

RL2:

DVR();

asm mov  cx,16000
asm sub  di,di

YL2:
asm shl  SEED,1			// Plot snow
asm jnc  NO_XOR2
asm xor  SEED,1D872B41h
NO_XOR2:
asm mov  eax,SEED
asm stosd
asm loop YL2

asm cmp  bx,75			// Random tuning effect
asm ja   NOIM
asm cmp  al,90
asm ja   NOIM
asm push ds SEG _buffer	// Plot on the image
asm pop  ds
asm sub  si,si
asm sub  di,di
asm mov  cx,16000
asm rep  movsd
asm pop  ds
NOIM:

asm mov  di,ax			// Plot noise bands
asm and  di,80FFh
asm add  di,12000
asm mov  cx,ax
asm and  cx,4095
asm xor  eax,1D872B41h
asm rep  stosd

asm dec  bx
asm jnz  RL2

asm cmp  [keys+kSPACE],0
asm jz   QUICKS2
asm jmp  QUICK
QUICKS2:

asm mov  bx,50

NNB:

DVR();

asm push ds SEG _buffer	// Plot on the image
asm pop  ds
asm sub  si,si
asm sub  di,di
asm mov  cx,16000
asm rep  movsd
asm pop  ds

asm cmp  bx,1
asm jz   OUT

asm shl  SEED,1			// Plot snow
asm jnc  NO_XOR3
asm xor  SEED,1D872B41h
NO_XOR3:
asm mov  eax,SEED
asm mov  di,ax   		// A few more noise bands please
asm cmp  di,47000
asm jb   OK_DI
asm mov  di,32000
OK_DI:
asm mov  cx,di
asm and  cx,4095
asm rep  stosd

asm dec  bx
asm jmp  NNB

OUT:

asm cmp  [keys+kSPACE],0
asm jz   QUICKS3
asm jmp  QUICK
QUICKS3:

delay(1000);
ps2(0);
asm mov bx,60			// Convert to colour

BWC:

DVR();
DVR();
DVR();
DVR();

asm mov  si,OFFSET pal
asm mov  cx,768

CCONV:
asm mov  al,[si]
asm cmp  al,[si+768+768]
asm je	 NXC
asm jl	 CCL

asm dec  byte ptr [si]
asm jmp  NXC
CCL:
asm inc  byte ptr [si]
NXC:
asm inc  si

asm loop CCONV

asm mov  si,OFFSET pal 	// Send new palette
asm mov  cx,768
asm mov  dx,3C8h
asm sub  al,al
asm out  dx,al
asm inc  dx
asm rep  outsb

asm dec  bx
asm jnz  BWC

QUICK:

Fade2Black(0);
farfree(SFX[255]);			// Free intro sample memory
}

#endif

void FinishedLevel(void)
{
Fade2Black(0);			// Fade the screen out
LoadLogo(7);
SRCopy();

FadeFromBlack(0);

WAIT();

Fade2Black(0);			// Fade the screen back out

// Set up data

//Enemy[txloop].type=127; //*8<*************************************

CurrentLevel++;			// End of Level
asm mov LastStat,14		// Fade in new level

//----------------------//----------------------------------------------------

if(CurrentLevel==7)		// Finished Game
 {

 }
}

#if DESIGN
#else

char LoadAudio(void)
{

SFX[0] = loadwave("23.wav",&SFXL[0]); 	// *Hurry Up
SFX[1] = loadwave("26.wav",&SFXL[1]);   	// *Coins & Gems
SFX[2] = loadwave("21.wav",&SFXL[2]);   	// *Bonuses
SFX[3] = loadwave("6.wav",&SFXL[3]);		// *Rapid Fire & Super Jump
SFX[4] = loadwave("7.wav",&SFXL[4]);		// *Extra Life
SFX[5] = loadwave("4.wav",&SFXL[5]);		// *Time
SFX[6] = loadwave("2.wav",&SFXL[6]);		// *Energy
SFX[7] = loadwave("8.wav",&SFXL[7]);		// *Killed Enemy
SFX[8] = loadwave("18.wav",&SFXL[8]);	// *Hit Restart Point
SFX[9] = loadwave("25.wav",&SFXL[9]);	// *Steam
SFX[10]= loadwave("11.wav",&SFXL[10]);	// *Spring
SFX[11]= loadwave("19.wav",&SFXL[11]);	// Low energy
SFX[12]= loadwave("3.wav",&SFXL[12]);	// *Fire
SFX[13]= loadwave("50.wav",&SFXL[13]);	// *Flap

if(!SFX[13])
 {
 asm mov ax,3
 asm int 16
 printf("\nAudio Load Error!! (Any Key)");
 getch();
 asm and byte ptr GameSetup,254	// Turn off sound then
 }

return(1);
}

#endif

void Line(int a,int b,int c,int d)
{
int d1x,d1y,d2x,d2y,m,n;

asm mov  ax,c 		// AX : U
asm sub  ax,a

asm mov  bx,d 		// BX : V
asm sub  bx,b

asm mov  cx,ax
asm and  cx,0x8000
asm rol  cx,1
asm jc   short N1
asm dec  cx
N1:
asm neg  cx             // CX : D1X

asm mov  dx,bx
asm and  dx,0x8000
asm rol  dx,1
asm jc   short N2
asm dec  dx
N2:
asm neg  dx             // DX : D1Y

asm mov d1x,cx
asm mov d1y,dx
asm mov d2x,cx
asm mov d2y,0 		// Mov regs into vars

asm or  ax,ax           // get abs of u -> m
asm jge short POS1
asm neg ax
POS1:
asm mov m,ax

asm or  bx,bx           // get abs of v -> n
asm jge short POS2
asm neg bx
POS2:
asm mov n,bx

asm cmp ax,bx
asm jg  short J1

asm mov d2x,0
asm mov d2y,dx
asm mov m,bx
asm mov n,ax

J1:

asm mov bx,m
asm mov dx,bx
asm mov cx,bx
asm shr bx,1

PLOT_LOOP:

 asm mov di,a
 asm mov ax,b
 asm shl ax,6
 asm add di,ax
 asm shl ax,2
 asm add di,ax
 asm mov ax,101h
 asm stosw

 asm mov ax,n
 asm add bx,ax

 asm cmp bx,dx
 asm jl  short J2

 asm sub bx,dx
 asm mov ax,d1x
 asm add a,ax
 asm mov ax,d1y
 asm add b,ax
 asm jmp short J3

 J2:

 asm mov ax,d2x
 asm add a,ax
 asm mov ax,d2y
 asm add b,ax

 J3:

asm loop short PLOT_LOOP
}

void Outro(void)
{
// Fancy cakes

asm mov ax,13h
asm int 10h

Clear();			// Clear Buffer and VRAM
SCopy();

asm mov  dx,3C8h
asm mov  al,1
asm out  dx,al      // Blank index 0
asm inc  dx
asm sub	 al,al
asm out	 dx,al		// R
asm out	 dx,al		// G
asm out	 dx,al		// B

asm push 40960
asm pop	 es
asm mov  cx,16000
asm mov  eax,01010101h
asm sub  di,di
asm rep  stosd

asm mov  cx,32		// Fade up to white
asm sub	 bl,bl
BUP:
asm mov	 dx,3DAh
asm in 	 al,dx
asm test al,8h
asm jne	 $-5
asm in 	 al,dx
asm test al,8h
asm je	 $-5
asm mov	 al,1
asm mov  dx,3C8h
asm out	 dx,al
asm inc  dx
asm mov  al,bl
asm out  dx,al
asm out  dx,al
asm out  dx,al
asm inc  bl
asm inc  bl
asm loop BUP

asm mov  bx,25				// Shrink vertically
asm sub  di,di
asm mov  si,64315

YTO:
asm mov	 dx,3DAh
asm in 	 al,dx
asm test al,8h
asm jne	 $-5
asm in 	 al,dx
asm test al,8h
asm je	 $-5

asm mov  cx,320

XTO:
asm mov  dword ptr [es:di],0
asm mov  dword ptr [es:si],0
asm add  di,4
asm sub  si,4
asm loop XTO

asm dec  bx
asm jnz	 YTO
						// Shrink horizontally
asm mov  cx,40
asm mov  di,32000
asm mov  si,32317

XCL:
asm mov	 dx,3DAh
asm in 	 al,dx
asm test al,8h
asm jne	 $-5
asm in 	 al,dx
asm test al,8h
asm je	 $-5
asm mov  dword ptr [es:di],0
asm mov  dword ptr [es:si],0
asm add  di,4
asm sub  si,4
asm loop XCL

asm mov  cx,63		// Fade down
asm mov	 bl,63
BDN:
asm mov	 dx,3DAh
asm in 	 al,dx
asm test al,8h
asm jne	 $-5
asm in 	 al,dx
asm test al,8h
asm je	 $-5
asm mov	 al,1
asm mov  dx,3C8h
asm out	 dx,al
asm inc  dx
asm mov  al,bl
asm out  dx,al
asm out  dx,al
asm out  dx,al
asm dec  bl
asm loop BDN

delay(1000);
}