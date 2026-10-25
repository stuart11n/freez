int Circular[256]={
0,40,2,40,4,40,6,40,8,39,10,39,12,38,13,38,
15,37,17,36,19,35,21,34,22,33,24,32,25,31,27,30,
28,28,30,27,31,25,32,24,33,22,34,21,35,19,36,17,
37,15,38,13,38,12,39,10,39,8,40,6,40,4,40,2,
40,-0,40,-2,40,-4,40,-6,39,-8,39,-10,38,-12,38,-13,
37,-15,36,-17,35,-19,34,-21,33,-22,32,-24,31,-25,30,-27,
28,-28,27,-30,25,-31,24,-32,22,-33,21,-34,19,-35,17,-36,
15,-37,13,-38,12,-38,10,-39,8,-39,6,-40,4,-40,2,-40,-
0,-40,-2,-40,-4,-40,-6,-40,-8,-39,-10,-39,-12,-38,-13,-38,-
15,-37,-17,-36,-19,-35,-21,-34,-22,-33,-24,-32,-25,-31,-27,-30,-
28,-28,-30,-27,-31,-25,-32,-24,-33,-22,-34,-21,-35,-19,-36,-17,-
37,-15,-38,-13,-38,-12,-39,-10,-39,-8,-40,-6,-40,-4,-40,-2,-
40,0,-40,2,-40,4,-40,6,-39,8,-39,10,-38,12,-38,14,-
37,15,-36,17,-35,19,-34,21,-33,22,-32,24,-31,25,-30,27,-
28,28,-27,30,-25,31,-24,32,-22,33,-21,34,-19,35,-17,36,-
15,37,-13,38,-12,38,-10,39,-8,39,-6,40,-4,40,-2,40};

int BridgeDynamics[1024]=
{
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
0,1,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
0,1,2,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
0,1,2,2,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,0,0,0,0,
0,1,2,2,3,4,4,4,4,3,3,3,3,3,3,2,2,2,2,2,2,2,2,1,1,1,1,1,1,0,0,0,
0,1,1,2,3,3,4,4,4,4,3,3,3,3,3,3,3,2,2,2,2,2,2,1,1,1,1,1,1,0,0,0,
0,1,1,2,2,3,3,4,4,4,3,3,3,3,3,3,3,2,2,2,2,2,2,1,1,1,1,1,1,0,0,0,
0,1,1,2,2,3,3,4,4,4,4,3,3,3,3,3,3,3,2,2,2,2,2,2,1,1,1,1,1,1,0,0,
0,1,1,2,2,3,3,4,4,5,5,5,4,4,4,4,4,3,3,3,3,2,2,2,2,2,1,1,1,1,0,0,
0,1,1,2,2,3,3,4,4,5,5,5,5,4,4,4,4,4,3,3,3,3,2,2,2,2,1,1,1,1,0,0,
0,0,1,1,2,2,3,3,4,4,5,5,5,4,4,4,4,4,3,3,3,3,2,2,2,2,1,1,1,1,0,0,
0,0,1,1,2,2,3,3,3,4,4,5,5,5,4,4,4,4,4,3,3,3,3,2,2,2,2,1,1,1,1,0,
0,0,1,1,2,2,2,3,3,3,4,4,5,5,5,4,4,4,4,4,3,3,3,2,2,2,2,1,1,1,1,0,
0,0,1,1,2,2,3,3,3,4,4,5,5,6,6,6,6,5,5,5,4,4,4,3,3,2,2,2,1,1,1,0,
0,0,1,1,2,2,2,3,3,4,4,4,5,5,6,6,6,6,5,5,4,4,4,3,3,3,2,2,1,1,1,0,
0,1,1,1,2,2,3,3,3,4,4,4,5,5,6,6,6,6,5,5,4,4,4,3,3,2,2,2,1,1,0,0,
0,1,1,1,2,2,2,3,3,4,4,4,5,5,5,6,6,6,6,5,5,4,4,3,3,3,2,2,1,1,0,0,
0,1,1,1,1,2,2,2,2,3,3,3,4,4,4,4,4,5,5,5,4,4,3,3,3,2,2,2,1,1,0,0,
0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,4,5,5,5,4,4,3,3,3,2,2,1,1,0,0,
0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,4,5,5,5,4,4,3,3,2,2,1,1,0,0,
0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,4,5,5,5,5,4,4,3,3,2,2,1,1,0,
0,0,1,1,1,1,2,2,2,2,2,3,3,3,3,4,4,4,4,4,5,5,5,4,4,3,3,2,2,1,1,0,
0,0,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,3,3,3,4,4,4,4,3,3,2,2,1,1,0,
0,0,0,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,3,3,3,4,4,4,3,3,2,2,1,1,0,
0,0,0,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,3,3,3,4,4,4,4,3,3,2,1,1,0,
0,0,0,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,4,4,4,4,3,2,2,1,0,
0,0,0,0,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,2,2,1,0,
0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,2,1,0,
0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,1,0,
0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Structure for Level Specification

typedef struct
		 {
		 unsigned int  Sx;			// Start World X Position
		 unsigned int  Sy;			// Start World Y Position
		 unsigned char Face;		// Direction to face
		 unsigned char Traction;
		 unsigned char Gems;
		 unsigned char Coins;
		 unsigned int  Time;		// Level completion time
		 unsigned int  ATiles[32];  // Animated tiles list
		 unsigned int  BPos[32];	// Positions of bridges
		 unsigned char Bridge[30];	// Bridge graphic
		 unsigned char Tiles1[30];  // Tiles  0-63
		 unsigned char Tiles2[30];  // Tiles 64-127
		 unsigned char Enemy1[30];	// Enemy 1 graphics
		 unsigned char Enemy2[30];  // Enemy 2 graphics
		 unsigned char Enemy3[30];  // Enemy 3 graphics
		 unsigned char Enemy4[30];  // Enemy 4 graphics
		 unsigned char TMap[30];    // Land tile map
		 unsigned char BMap[30];	// Bonus map
		 unsigned char EMap[30];	// Enemy map
		 unsigned char SMap[30];    // Sky map
		 }LevelSpec;                // Record size : 468 bytes

// Structure for Enemy 32x32 objects

typedef struct
		 {
		 unsigned char ctrl; 	//  0 : 0 - inactive; 1 - Ground Based; 2 - Airborne; 3 - Invisible Deadly; 4 - Invisible Spring; 5 - Platform
		 unsigned char status;	//  1 : 0 - Normal; >0 - Frozen
		 unsigned char fcount;	//  2 : Frozen time counter
		 signed   int  x;	    //  3 : World X Position
		 signed   int  y;       //  5 : World Y Position
		 unsigned char type;	//  7 : Sprite data index/character selection
		 unsigned char frame;	//  8 : Current frame
		 unsigned char disp;	//  9 : Current displacement from origin/Energy Lost (ctrl-3)/Jump Height (ctrl-4)
		 unsigned char maxdisp;	// 10 : Maximum displacement from origin
		 signed   char speed;	// 11 : Speed (pels/frame)
		 unsigned char Dodge;	// 12 : Dodge direction flag
		 }MObject32;

// Structure for Static 16x16 objects

typedef struct
		 {
		 unsigned char ctrl; 	// 0 : 0 - inactive, 1 - Collect, 2 - Energy etc
		 signed   int  x;		// 1 : World X Position
		 signed   int  y;       // 3 : World Y Position
		 }BObject;


LevelSpec   Level[10];

MObject32	Enemy[200];
BObject		Bonus[400];

#if DESIGN

unsigned int BPos[32];
unsigned int AnimTile[32];

#endif

unsigned char skymap[2048];
unsigned char tilemap[8192];

unsigned char BTYPE[31488]; 		// 6 Allocated (2048 Bytes (8 frames) per bonus type)
unsigned char BridgeGFX[64];
unsigned char StatGFX[1088];
unsigned char far tile1[65535];
unsigned char far tile2[65535];
unsigned char far MCANIM[36864];	// Main Character Animation Data
unsigned char far enemy0[19456];
unsigned char far enemy1[19456];
unsigned char far enemy2[19456];
unsigned char far enemy3[19456];
unsigned char far buffer[65535];	// Screen Buffer

// Various Variables

unsigned char   GameSetup;

unsigned char   pal[768],SPal[768],FadeStore[768];

unsigned char 	CurrentLevel=1;

unsigned char 	RVal;

unsigned char 	AudioOn;

unsigned int 	S,E;

int				STK1,STK2,STK3,STK4;

FILE *fh;

unsigned char 	far *SFX[255];

unsigned int  	SFXL[255];

// Global Main Character Control Data

//           CB - Control Block   	Test	Set(OR)	Clear(AND)

//            0 - Facing    	  	1		1 		254
//            1 - Falling      	  	2		2       253
//            2 - Hit		  		4		4       251
//            3 - Jumping  	  		8		8       247
//            4 - JumpPressed		16		16      239
//            5 -                 	32		32      223
//            6 - Slowing         	64		64      191
//            7 - Fade In         	128		128     127

asm {

SEED  			dd	?
SEED2 			dd 	?

WordLoop		dw  ?			// Loop variables
ByteLoop		db	?
ByteLoop2		db	?

MCB				db  ?			// See key above
Collision   	db  ?			// Landscape collision status
Firing      	db  ?			// Firing status and delay ctrl
Frame       	db  ?			// Main characters current frame
MomLeft     	db  ?			// Left Momentum
MomRight    	db  ?			// Right Momentum
JumpStat    	db  ?			// Jump height
LastStat    	db  ?			// Character status in last frame
GravStat    	db  ?			// Gravity value
Lives       	db  ?			// No. of lives
Energy      	db  ?			// Current energy
Timer			dw	?			// Time left
Score			dw	?
TTimer			dw	?			// Time left displayed on statbar
TEnergy     	db  ?			// Energy displayed on statbar
TScore			dw  ?			// Score dispalyed on statbar

LevelEnd		db	?			// Level complete indicator & delay
HitCount		db	?			// Hit duration timer
IceBallNo		db	4			// Active iceball counter

RapidFire		dw	?			// Rapid fire timer
SuperJ			dw	?			// Super jump timer
SteamCount		db	?

LevelDataP  	dw  ?			// Pointer to current levels record

wposx           dw	?			// World pixel position
wposy			dw	?

DXPos 			dw 	?			// World motion since last frame
DYPos 			dw 	?
DXPos2 			dw 	?
DYPos2 			dw 	?

FYpos			dw	?
TileX			dw	?
FTileX			dw	?
TileIndex		dw	?
FTileIndex		dw	?

RangeTest		dw	?			// Object scope processing variable

Drops 			dw 	800 DUP (?)	// Rain drops

SkyBright		db  63
SkyBrightDr		db  ?
SkyBrightCt		db  ?
Lightning		dw  40	DUP (?) //?????????????????? WHY ?????????????

LivesString 	db  2   DUP (?)
ScoreString 	db  6   DUP (?)
TimerString 	db  3   DUP (?)
EnergyString 	db  3   DUP (?)
LevelString  	db  2   DUP (?)

TractionType 	db  ?

frameinc		dw	?
BonusFrame		db	?
HalfFrame		db	?
SlowFrameInc	db 	?

atilecount		db	0
Steam 			db 	80 	DUP (?) // 1-Ctrl 	1-Frame	2-X 	2-Y 	2-Dummy
IceBall 		db  24 	DUP (?) // 1-Ctrl 	2-X 	2-Y 	1-Frame

}

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл