asm .386  			// Enable 386 Instructions
asm SMART 			// Enable Assembler Optimisation
asm WARN ALN
asm WARN BRK
asm WARN ICG
asm WARN PQK

#include <conio.h>
#include <stdio.h>
#include <k.h>
#include <dos.h>

// External data references

extern unsigned char far buffer[64000];
extern char pal[768];
extern char col,GameSetup,AudioOn;
extern unsigned int entry;
extern unsigned char keys[256];

// External function references

extern Clear(),SCopy(),SRCopy(),FadeFromBlack(char),Fade2Black(char);
extern ps2(unsigned char);
extern ReadAxis(),FixPal(),Screen(char);

// Function prototypes for those defined in here

void Options(void);
void ScreenMode(void);
void Speed(void);
void LoadLogo(char);
void Calib(void);
void DBox(int,char);
void DSBox(int,char);
void Light(int,char);
char IsAnyKey(void);
void Information(void);

//---------------------------------------------------------
//                  M A I N   M E N U
//---------------------------------------------------------

char MainMenu(void)
{
char txloop,LastKey=0;

STARTMENU:

txloop=0;

// Load image

LoadLogo(1);

// Draw Default Highlights

DBox(37810,215);
DBox(37872,0);
DBox(37934,0);
DBox(37996,0);

// Fade in Image

FadeFromBlack(0);

KEYCHECK:

// Read gamepad L&R -- returns in AL

ReadAxis();

 if(_AL==1||(keys[kLARROW]&&!keys[kRARROW]))
  if(LastKey!=1)
   {
   ps2(8);
   LastKey=1;
   if(txloop==0)txloop+=3;
   else txloop--;
   }

 if(_AL==2||(keys[kRARROW]&&!keys[kLARROW]))
  if(LastKey!=2)
   {
   ps2(8);
   LastKey=2;
   if(txloop==3)txloop-=3;
   else txloop++;
   }

 // Draw Highlights

 switch (txloop)
  {
  case 0:	DBox(37810,215);
			DBox(37872,0);
			DBox(37934,0);
			DBox(37996,0);
			break;
  case 1:   DBox(37810,0);
			DBox(37872,215);
			DBox(37934,0);
			DBox(37996,0);
			break;
  case 2:	DBox(37810,0);
			DBox(37872,0);
			DBox(37934,215);
			DBox(37996,0);
			break;
  case 3:	DBox(37810,0);
			DBox(37872,0);
			DBox(37934,0);
			DBox(37996,215);
  }

 ReadAxis();

 if((_AL==0||_AL==3)&&!keys[kLARROW]&&!keys[kRARROW])LastKey=0;

 if(IsAnyKey()) goto LeaveMenu;

 asm jmp  KEYCHECK		// Infinite Loop Scenario

LeaveMenu:

ps2(8);		// Play `click'

//Clear();

switch (txloop)
 {
 case 1:	Fade2Black(0);
			Options();			// Call options
			goto STARTMENU;
 case 2:    Fade2Black(0);
			Information();		// Call instructions
			goto STARTMENU;
 case 3:    Fade2Black(0);      // Quit
			return(1);
 }

return(0);						// Start Game
}

////////////////////////////////////////////////////////////////////////

void Information(void)
{
FILE *fh;
LoadLogo(3);
FadeFromBlack(0);

KEYCHECKA:
if(!IsAnyKey()) goto KEYCHECKA;

Fade2Black(0);
}

void Options(void)
{
FILE *fh;
char txloop=0,LastKey=0;

// Load image

LoadLogo(2);

// Draw Default Highlights

DSBox(4247,37);
DSBox(13847,0);
DSBox(23127,0);
DSBox(32407,0);
DSBox(41687,0);
DSBox(54487,0);

if(GameSetup&1)	Light(6173,37);
if(GameSetup&2)	Light(15773,37);
if(GameSetup&4)	Light(25053,37);
if(GameSetup&8)	Light(34333,37);
if(GameSetup&16)Light(43613,37);

// Fade in Image

FadeFromBlack(0);

KEYCHECK:

// Read gamepad L&R -- returns in AL

ReadAxis();

 if(_AL==1||(keys[kLARROW]&&!keys[kRARROW]))
  if(LastKey!=1)
   {
   ps2(8);
   LastKey=1;
   if(txloop==0)txloop+=5;
   else txloop--;
   }

 if(_AL==2||(keys[kRARROW]&&!keys[kLARROW]))
  if(LastKey!=2)
   {
   ps2(8);
   LastKey=2;
   if(txloop==5)txloop-=5;
   else txloop++;
   }

 // Draw Highlights

 switch (txloop)
  {
  case 0:	DSBox(4247,37);
			DSBox(13847,0);
			DSBox(23127,0);
			DSBox(32407,0);
			DSBox(41687,0);
			DSBox(54487,0);
			break;
  case 1:	DSBox(4247,0);
			DSBox(13847,37);
			DSBox(23127,0);
			DSBox(32407,0);
			DSBox(41687,0);
			DSBox(54487,0);
			break;
  case 2:   DSBox(4247,0);
			DSBox(13847,0);
			DSBox(23127,37);
			DSBox(32407,0);
			DSBox(41687,0);
			DSBox(54487,0);
			break;
  case 3:   DSBox(4247,0);
			DSBox(13847,0);
			DSBox(23127,0);
			DSBox(32407,37);
			DSBox(41687,0);
			DSBox(54487,0);
			break;
  case 4:   DSBox(4247,0);
			DSBox(13847,0);
			DSBox(23127,0);
			DSBox(32407,0);
			DSBox(41687,37);
			DSBox(54487,0);
			break;
  case 5:   DSBox(4247,0);
			DSBox(13847,0);
			DSBox(23127,0);
			DSBox(32407,0);
			DSBox(41687,0);
			DSBox(54487,37);
  }

 // Draw lights

 if(GameSetup&1) Light(6173,37);
 else 			 Light(6173,0);
 if(GameSetup&2) Light(15773,37);
 else			 Light(15773,0);
 if(GameSetup&4) Light(25053,37);
 else			 Light(25053,0);
 if(GameSetup&8) Light(34333,37);
 else			 Light(34333,0);
 if(GameSetup&16)Light(43613,37);
 else			 Light(43613,0);

 ReadAxis();

 if((_AL==0||_AL==3)&&!keys[kLARROW]&&!keys[kRARROW])LastKey=0;

 if(IsAnyKey())
  {
  switch (txloop)
   {
   case 0 : if(AudioOn)     		// Turn on sound if installed
			GameSetup^=1;
			break;
   case 1 : GameSetup^=2;
			break;
   case 2 : GameSetup^=4;
			break;
   case 3 : GameSetup^=8;			// WideScreen arama
			if(GameSetup&8)Screen(1);
			if(!(GameSetup&8))Screen(0);
			break;
   case 4 : ReadAxis();if(_AL!=3)	GameSetup^=16;	break; // Turn on gamepad if installed
   case 5 : goto LeaveMenu;
   }
  ps2(8);		// Play `click'
  while(IsAnyKey());
  }

 asm jmp  KEYCHECK		// Infinite Loop Scenario

LeaveMenu:

ps2(8);                  // Play `click'

fh=fopen("setup.dat","wb");
fwrite(&GameSetup,1,1,fh);
fclose(fh);

Fade2Black(0);
}

void LoadLogo(char IMG)
{
FILE *fh;

asm pushad
asm push ds
asm push es
asm push fs
asm push gs

switch (IMG)
 {
 case 1  	:	fh=fopen("mainmenu.tga","rb");	break;
 case 2  	:	fh=fopen("setup.tga","rb");		break;
 case 3  	:	fh=fopen("info.tga","rb");		break;
 case 6  	:	fh=fopen("quit.tga","rb");		break;
 case 7  	:	fh=fopen("lev_comp.tga","rb");	break;
 case 10	:	fh=fopen("level1.tga","rb");	break;
 case 11	:	fh=fopen("level1.tga","rb");	break;
 case 12	:	fh=fopen("level2.tga","rb");	break;
 case 13	:	fh=fopen("level2.tga","rb");	break;
 case 14	:	fh=fopen("level3.tga","rb");	break;
 case 15	:	fh=fopen("level3.tga","rb");	break;
 case 16	:	fh=fopen("level4.tga","rb");	break;
 case 17	:	fh=fopen("level4.tga","rb");	break;
 }

fseek(fh,0x12,0);
fread(pal,1,768,fh);
fseek(fh,0x312,0);
fread(buffer,1,64000,fh);
fclose(fh);

asm mov  ax,SEG buffer
asm mov  es,ax

SCopy();

FixPal();

asm pop gs
asm pop fs
asm pop es
asm pop ds
asm popad

//if(IMG<5)SRCopy();
}

//***************************************************************************

void DBox(int OSET,char Col)
{
  asm mov   bx,OSET
  asm mov   al,Col
  asm mov   ah,al
  asm push  es 40960
  asm pop   es
  asm mov   di,bx
  asm mov   cx,17
  asm rep   stosw
  asm mov   di,bx
  asm add   di,10240
  asm mov   cx,17
  asm rep   stosw
  asm mov   di,bx
  asm mov   cx,33
  Y1:
  asm stosb
  asm add   di,319
  asm loop  Y1
  asm mov   di,bx
  asm add   di,33
  asm mov   cx,33
  Y2:
  asm stosb
  asm add   di,319
  asm loop  Y2
  asm pop   es
}

void DSBox(int OSET,char Col)
{
  asm mov   bx,OSET
  asm mov   al,Col
  asm mov   ah,al
  asm push  es 40960
  asm pop   es
  asm mov   di,bx
  asm mov   cx,15
  asm rep   stosb
  asm mov   di,bx
  asm add   di,4800-320
  asm mov   cx,15
  asm rep   stosb
  asm mov   di,bx
  asm mov   cx,15
  Y1:
  asm stosb
  asm add   di,319
  asm loop  Y1
  asm mov   di,bx
  asm add   di,15
  asm mov   cx,15
  Y2:
  asm stosb
  asm add   di,319
  asm loop  Y2
  asm pop   es
}

void Light(int OSET,char Col)
{
  asm mov   bx,OSET
  asm mov   al,Col
  asm mov   ah,al
  asm mov   dx,ax
  asm shl   eax,16
  asm mov   ax,dx
  asm push  es 40960
  asm pop   es
  asm mov   di,bx
  asm stosd
  asm add   di,316
  asm stosd
  asm add   di,316
  asm stosd
  asm add   di,316
  asm pop   es
}

char IsAnyKey(void)
{
 asm mov  dx,201h		// Break out if enter, space or GP button pressed
 asm in   al,dx
 asm test al,16
 asm jz   Leave
 asm test al,32
 asm jz   Leave
 asm test al,64
 asm jz   Leave
 asm test al,128
 asm jz   Leave
 asm cmp  byte ptr [keys+kSPACE],0
 asm jnz  Leave
 asm cmp  byte ptr [keys+kENTER],0
 asm jnz  Leave
 return(0);
 Leave:
 return(1);
}