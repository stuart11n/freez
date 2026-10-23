#define DEBUG 0

extern buffer,MCANIM;

asm .386
asm SMART

// ON ENTRY  :	DL = MCB
//              AH = Frame
// ON RETURN :  DH = Collision Status

void pmain(void)
{
asm xor dh,dh

// Check \/ landscape [1] 10894

asm cmp byte ptr [es:37918],128
asm jb  CHKNEXT6
asm or  dh,1b
#if DEBUG
asm mov byte ptr [es:37918],3
#endif
CHKNEXT6:

asm cmp byte ptr [es:37922],128
asm jb  CHKNEXT7
asm or  dh,1b
#if DEBUG
asm mov byte ptr [es:37922],3
#endif
CHKNEXT7:

// Check \/ landscape [64] 15690 Teetering

asm cmp byte ptr [es:(42074-1600-320)],128
asm jb  CHKNEXT6b
asm or  dh,1000000b
#if DEBUG
asm mov byte ptr [es:42074],3
#endif
CHKNEXT6b:

asm cmp byte ptr [es:(42086-1600-320)],128
asm jb  short CHKNEXT7b
asm xor dh,1000000b
#if DEBUG
asm mov byte ptr [es:42086],3
#endif
CHKNEXT7b:

// Check /\ landscape [2] +4 //27024

asm cmp byte ptr [es:27024+20-(6*320)],128
asm jb  CLD1
asm or  dh,10b
CLD1:

// Check /\ landscape [2] -4

asm cmp byte ptr [es:27024+12-(6*320)],128
asm jb  CLD2
asm or  dh,10b
CLD2:

// Check << landscape [4]	Upper

asm cmp byte ptr [es:28308],128
asm jb  CHKNEXT2
asm or  dh,100b
#if DEBUG
asm mov byte ptr [es:28308],3
#endif
CHKNEXT2:

asm cmp byte ptr [es:28308+2560],128
asm jb  CHKNEXT3Q
asm or  dh,100b
#if DEBUG
asm mov byte ptr [es:28308+2560],3
#endif
CHKNEXT3Q:

asm cmp byte ptr [es:28308+2560+1280],128
asm jb  CHKNEXT3QQ
asm or  dh,100b
#if DEBUG
asm mov byte ptr [es:28308+2560+1280],3
#endif
CHKNEXT3QQ:

asm cmp byte ptr [es:28308+2560+2560],128
asm jb  CHKNEXT5QQQQ
asm or  dh,1000b
#if DEBUG
asm mov byte ptr [es:28308+2560+2560],3
#endif
CHKNEXT5QQQQ:

asm cmp byte ptr [es:28308+2560+2560+1280],128
asm jb  CHKNEXT5QQQQQ
asm or  dh,1000b
#if DEBUG
asm mov byte ptr [es:28308+2560+2560+1280],3
#endif
CHKNEXT5QQQQQ:

asm cmp byte ptr [es:28308+2560+2560+2560+1280+4],128
asm jb  CHKNEXT5QWQQQQQ
asm or  dh,1000b
#if DEBUG
asm mov byte ptr [es:28308+2560+2560+2560+1280+4],3
#endif
CHKNEXT5QWQQQQQ:

asm cmp byte ptr [es:35988],128
asm jb  CHKNEXT3
asm or  dh,1000b
#if DEBUG
asm mov byte ptr [es:35988],3
#endif
CHKNEXT3:

// Check >> landscape [16]	Upper

asm cmp byte ptr [es:28332],128
asm jb  CHKNEXT4
asm or  dh,10000b
#if DEBUG
asm mov byte ptr [es:28332],3
#endif
CHKNEXT4:

asm cmp byte ptr [es:28332+2560],128
asm jb  CHKNEXT5Q
asm or  dh,10000b
#if DEBUG
asm mov byte ptr [es:28332+2560],3
#endif
CHKNEXT5Q:

asm cmp byte ptr [es:28332+2560+1280],128
asm jb  CHKNEXT5QQ
asm or  dh,10000b
#if DEBUG
asm mov byte ptr [es:28332+2560+1280],3
#endif
CHKNEXT5QQ:

asm cmp byte ptr [es:28332+2560+2560],128
asm jb  CHKNEXT5QQQ
asm or  dh,100000b
#if DEBUG
asm mov byte ptr [es:28332+2560+2560],3
#endif
CHKNEXT5QQQ:

asm cmp byte ptr [es:28332+2560+2560+1280],128
asm jb  CHKNEXT5QQQQQQ
asm or  dh,100000b
#if DEBUG
asm mov byte ptr [es:28332+2560+2560+1280],3
#endif
CHKNEXT5QQQQQQ:

asm cmp byte ptr [es:28332+2560+2560+2560+1280-4],128
asm jb  CHKNEXT5QQQQQQQ
asm or  dh,100000b
#if DEBUG
asm mov byte ptr [es:28332+2560+2560+2560+1280-4],3
#endif
CHKNEXT5QQQQQQQ:

asm cmp byte ptr [es:36012],128
asm jb  CHKNEXT5
asm or  dh,100000b
#if DEBUG
asm mov byte ptr [es:36012],3
#endif
CHKNEXT5:

//------------------------------//
//  END OF LANDSCAPE DETECTION  //
//------------------------------//

asm mov  bx,ds						// Save DS

asm mov  cx,32
asm mov  di,27024

asm test dl,1
asm jz   short OK					// Facing Left so do reverse plot
asm jmp  OTHERDIR					// Do normal plot...
OK:

asm and  ax,0FC00h
asm mov  si,ax

asm mov  ax,SEG MCANIM
asm mov  ds,ax

LOOPY:

 asm lodsw              			// Get data to plot
 asm cmp al,0						// Check for transparency
 asm je  short GO1
 asm mov [es:di+31],al             	// Plot if opaque
 GO1:
 asm cmp ah,0
 asm je  short GO2
 asm mov [es:di+30],ah
 GO2:
 asm lodsw
 asm cmp al,0
 asm je  short GO3
 asm mov [es:di+29],al
 GO3:
 asm cmp ah,0
 asm je  short GO4
 asm mov [es:di+28],ah
 GO4:

 asm lodsw
 asm cmp al,0
 asm je  short GO12
 asm mov [es:di+27],al
 GO12:
 asm cmp ah,0
 asm je  short GO22
 asm mov [es:di+26],ah
 GO22:
 asm lodsw
 asm cmp al,0
 asm je  short GO32
 asm mov [es:di+25],al
 GO32:
 asm cmp ah,0
 asm je  short GO42
 asm mov [es:di+24],ah
 GO42:

 asm lodsw
 asm cmp al,0
 asm je  short GO13
 asm mov [es:di+23],al
 GO13:
 asm cmp ah,0
 asm je  short GO23
 asm mov [es:di+22],ah
 GO23:
 asm lodsw
 asm cmp al,0
 asm je  short GO33
 asm mov [es:di+21],al
 GO33:
 asm cmp ah,0
 asm je  short GO43
 asm mov [es:di+20],ah
 GO43:

 asm lodsw
 asm cmp al,0
 asm je  short GO14
 asm mov [es:di+19],al
 GO14:
 asm cmp ah,0
 asm je  short GO24
 asm mov [es:di+18],ah
 GO24:
 asm lodsw
 asm cmp al,0
 asm je  short GO34
 asm mov [es:di+17],al
 GO34:
 asm cmp ah,0
 asm je  short GO44
 asm mov [es:di+16],ah
 GO44:

 asm lodsw
 asm cmp al,0
 asm je  short GO15
 asm mov [es:di+15],al
 GO15:
 asm cmp ah,0
 asm je  short GO25
 asm mov [es:di+14],ah
 GO25:
 asm lodsw
 asm cmp al,0
 asm je  short GO35
 asm mov [es:di+13],al
 GO35:
 asm cmp ah,0
 asm je  short GO45
 asm mov [es:di+12],ah
 GO45:

 asm lodsw
 asm cmp al,0
 asm je  short GO16
 asm mov [es:di+11],al
 GO16:
 asm cmp ah,0
 asm je  short GO26
 asm mov [es:di+10],ah
 GO26:
 asm lodsw
 asm cmp al,0
 asm je  short GO36
 asm mov [es:di+9],al
 GO36:
 asm cmp ah,0
 asm je  short GO46
 asm mov [es:di+8],ah
 GO46:

 asm lodsw
 asm cmp al,0
 asm je  short GO17
 asm mov [es:di+7],al
 GO17:
 asm cmp ah,0
 asm je  short GO27
 asm mov [es:di+6],ah
 GO27:
 asm lodsw
 asm cmp al,0
 asm je  short GO37
 asm mov [es:di+5],al
 GO37:
 asm cmp ah,0
 asm je  short GO47
 asm mov [es:di+4],ah
 GO47:

 asm lodsw
 asm cmp al,0
 asm je  short GO18
 asm mov [es:di+3],al
 GO18:
 asm cmp ah,0
 asm je  short GO28
 asm mov [es:di+2],ah
 GO28:
 asm lodsw
 asm cmp al,0
 asm je  short GO38
 asm mov [es:di+1],al
 GO38:
 asm cmp ah,0
 asm je  short GO48
 asm mov [es:di],ah
 GO48:

 asm add  di,320
 asm dec  cx
 asm jz   short EXIT
 asm jmp  LOOPY

EXIT:

asm mov ds,bx
return;

OTHERDIR:

//asm mov ah,frame
asm and ax,0xFC00
asm mov si,ax

asm mov  ax,SEG MCANIM
asm mov  ds,ax

LOOPY2:

 asm lodsw              	// Get data to plot
 asm cmp al,0			// Check for transparency
 asm je  short GOA1
 asm mov [es:di],al             // Plot if opaque
 GOA1:
 asm cmp ah,0
 asm je  short GOA2
 asm mov [es:di+1],ah
 GOA2:
 asm lodsw
 asm cmp al,0
 asm je  short GOA3
 asm mov [es:di+2],al
 GOA3:
 asm cmp ah,0
 asm je  short GOA4
 asm mov [es:di+3],ah
 GOA4:

 asm lodsw
 asm cmp al,0
 asm je  short GOA12
 asm mov [es:di+4],al
 GOA12:
 asm cmp ah,0
 asm je  short GOA22
 asm mov [es:di+5],ah
 GOA22:
 asm lodsw
 asm cmp al,0
 asm je  short GOA32
 asm mov [es:di+6],al
 GOA32:
 asm cmp ah,0
 asm je  short GOA42
 asm mov [es:di+7],ah
 GOA42:

 asm lodsw
 asm cmp al,0
 asm je  short GOA13
 asm mov [es:di+8],al
 GOA13:
 asm cmp ah,0
 asm je  short GOA23
 asm mov [es:di+9],ah
 GOA23:
 asm lodsw
 asm cmp al,0
 asm je  short GOA33
 asm mov [es:di+10],al
 GOA33:
 asm cmp ah,0
 asm je  short GOA43
 asm mov [es:di+11],ah
 GOA43:

 asm lodsw
 asm cmp al,0
 asm je  short GOA14
 asm mov [es:di+12],al
 GOA14:
 asm cmp ah,0
 asm je  short GOA24
 asm mov [es:di+13],ah
 GOA24:
 asm lodsw
 asm cmp al,0
 asm je  short GOA34
 asm mov [es:di+14],al
 GOA34:
 asm cmp ah,0
 asm je  short GOA44
 asm mov [es:di+15],ah
 GOA44:

 asm lodsw
 asm cmp al,0
 asm je  short GOA15
 asm mov [es:di+16],al
 GOA15:
 asm cmp ah,0
 asm je  short GOA25
 asm mov [es:di+17],ah
 GOA25:
 asm lodsw
 asm cmp al,0
 asm je  short GOA35
 asm mov [es:di+18],al
 GOA35:
 asm cmp ah,0
 asm je  short GOA45
 asm mov [es:di+19],ah
 GOA45:

 asm lodsw
 asm cmp al,0
 asm je  short GOA16
 asm mov [es:di+20],al
 GOA16:
 asm cmp ah,0
 asm je  short GOA26
 asm mov [es:di+21],ah
 GOA26:
 asm lodsw
 asm cmp al,0
 asm je  short GOA36
 asm mov [es:di+22],al
 GOA36:
 asm cmp ah,0
 asm je  short GOA46
 asm mov [es:di+23],ah
 GOA46:

 asm lodsw
 asm cmp al,0
 asm je  short GOA17
 asm mov [es:di+24],al
 GOA17:
 asm cmp ah,0
 asm je  short GOA27
 asm mov [es:di+25],ah
 GOA27:
 asm lodsw
 asm cmp al,0
 asm je  short GOA37
 asm mov [es:di+26],al
 GOA37:
 asm cmp ah,0
 asm je  short GOA47
 asm mov [es:di+27],ah
 GOA47:

 asm lodsw
 asm cmp al,0
 asm je  short GOA18
 asm mov [es:di+28],al
 GOA18:
 asm cmp ah,0
 asm je  short GOA28
 asm mov [es:di+29],ah
 GOA28:
 asm lodsw
 asm cmp al,0
 asm je  short GOA38
 asm mov [es:di+30],al
 GOA38:
 asm cmp ah,0
 asm je  short GOA48
 asm mov [es:di+31],ah
 GOA48:

 asm add  di,320
 asm dec  cx
 asm jz   short EXIT2
 asm jmp  LOOPY2

EXIT2:

asm mov ds,bx

return;
}