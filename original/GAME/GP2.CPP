asm .386  			// Enable 386 Instructions
asm SMART 			// Enable Assembler Optimisation
asm WARN ALN			// Assembler Warnings ...
asm WARN BRK
asm WARN ICG
asm WARN PQK

extern max_loop_count,xlow,xhigh;

/*

//#include <stdio.h>
//#include <dos.h>
//#include <conio.h>

void ReadAxis(void);

char Val;

void main(void)
{

while(!kbhit())
{

ReadAxis();
if(_AL!=Val)
 {
 Val=_AL;
 printf("\n%d \t%d",_AL,_CX);
 }

//delay(100);
}

}*/

void ReadAxis(void)
{
asm {

mov  cx,1000
mov  dx,201h
out  dx,al	// Tempt gameport into action

}

//asm cli

RETEST:

asm {

in     al,dx
test   al,1
loopnz RETEST

jcxz   NO_GP

}

//asm sti

asm {
		cmp	cx,900
		jl      NOTGPLEFT
		mov	al,1			// Left
}
		return;
NOTGPLEFT:
asm {
		cmp 	cx,540
		jg  	NOTGPRIGHT
		mov 	al,2			// Right
}
		return;
NOTGPRIGHT:
asm {
		xor 	al,al			// None
}
		return;

NO_GP:
asm		mov	al,3			// Not connected

}