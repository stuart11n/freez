#include <stdio.h>
#include <dos.h>

asm .386  			// Enable 386 Instructions
asm SMART 			// Enable Assembler Optimisation
asm WARN ALN
asm WARN BRK
asm WARN ICG
asm WARN PQK

extern keys;
extern buffer;
extern char pal[768],SMode,SPal[768],FadeStore[768];
extern char pal3[768];
extern Fade2Black(char),FadeFromBlack(char),WriteT(int x,int y,unsigned char Message);
extern LoadLogo(char),SetPal(),SavePal();

void SetGPal(void);
void FixPal(void);

void Clear(void)		// Clear buffer
 {
 asm {
  mov  ax,SEG buffer
  mov  es,ax

  sub  di,di
  sub  eax,eax
  mov  cx,16000
  rep  stosd
  }
 }

void SCopy(void)		// Copy Buffer to screen
 {
 asm {
  push es
  push ds

  mov  ax,es
  mov  ds,ax
  mov  ax,40960
  mov  es,ax
  xor  di,di
  xor  si,si
  mov  ecx,16000
  rep  movsd

  pop  ds
  pop  es
  }
 }

void SRCopy(void)		// Copy Screen to Buffer
 {
 asm {
  push ds

  mov  ax,40960
  mov  ds,ax
  sub  di,di
  sub  si,si
  mov  ecx,16000
  rep  movsd

  pop  ds
  }
 }

void QuitPlay(void)
{
SavePal();
Fade2Black(0);
LoadLogo(6);
FadeFromBlack(0);
}

void UnPause(void)
{
Fade2Black(0);
SCopy();
SetPal();
FadeFromBlack(0);
}

void FixPal(void)
{
// Fix Palette (8 -> 6 bit, BGR -> RGB)

asm mov cx,256
asm mov si,OFFSET pal
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
}

void SetGPal(void)
{
FILE *fh;

fh=fopen("dragon.tga","rb");
fseek(fh,0x12,0);
fread(pal,1,768,fh);
fclose(fh);

FixPal();

asm mov  ax,SEG _buffer
asm mov  es,ax
}

void WAIT(void)
{
asm    pushad

asm    mov    dx,0x201		// GamePort
asm    mov    si,OFFSET keys

KEY_LOOP:

asm {

in     al,dx

test   al,16   		// Button 1
je     short GAMEPAD_BUTTON
test   al,32   		// Button 2
je     short GAMEPAD_BUTTON
test   al,64   		// Button 3
je     short GAMEPAD_BUTTON
test   al,128  		// Button 4
je     short GAMEPAD_BUTTON

add    bx,4
and    bx,63
cmp    dword ptr [si+bx],0
jz     KEY_LOOP

}

GAMEPAD_BUTTON:
asm    popad
}

void Pause(void)
{
SavePal();
Fade2Black(1);
WAIT();
SetPal();
FadeFromBlack(1);
}

void Screen(char MODE)
{
asm cmp byte ptr MODE,0
asm jz  M13H
asm mov dx,3D4h
asm mov ax,2C11h                // turn off write protect
asm out dx,ax
asm mov ax,0D06h                // vertical total
asm out dx,ax
asm mov ax,3E07h                // overflow register
asm out dx,ax
asm mov ax,0D010h               // vertical retrace start
asm out dx,ax
asm mov ax,0AC11h               // vertical retrace end AND wr.prot
asm out dx,ax
asm mov ax,9512h                // vertical display enable end
asm out dx,ax
asm mov ax,0E715h               // start vertical blanking
asm out dx,ax
asm mov ax,0616h                // end vertical blanking
asm out dx,ax
asm jmp DONE

M13H:

asm mov dx,3D4h
asm mov ax,02C11h               // turn off write protect
asm out dx,ax
asm mov ax,0BF06h               // vertical total
asm out dx,ax
asm mov ax,01F07h               // overflow register
asm out dx,ax
asm mov ax,09910h               // vertical retrace start
asm out dx,ax
asm mov ax,08011h               // vertical retrace end AND wr.prot
asm out dx,ax
asm mov ax,09012h               // vertical display enable end
asm out dx,ax
asm mov ax,0F015h               // start vertical blanking
asm out dx,ax
asm mov ax,09C16h               // end vertical blanking
asm out dx,ax

DONE:
}

void DVR(void)
{
asm mov  dx,0x3DA
asm in   al,dx
asm test al,08h
asm je   $-5
asm in   al,dx
asm test al,08h
asm jne  $-5
asm in   al,dx
asm test al,08h
asm je   $-5
}
