asm .386

char CPU(void)
{

asm {

	xor cl,cl                       // processor type 0 in case of exit

	pushf                           // transfer FLAGS to BX
	pop bx

	mov ax,bx                       // try to clear high 4 bits of FLAGS
	and ah,0fh

	push ax                         // transfer AX to FLAGS
	popf
	pushf                           // transfer FLAGS back to AX
	pop ax

	and ah,0f0h                     // isolate high 4 bits
	cmp ah,0f0h
	je short detect_processordone   // if bits are set, CPU is 8086/8

	mov cl,2                        // processor type 2 in case of exit

	or bh,0f0h                      // try to set high 4 bits of FLAGS

	push bx                         // transfer BX to FLAGS
	popf
	pushf                           // transfer FLAGS to AX
	pop ax

	and ah,0f0h                     // isolate high 4 bits
	jz short detect_processordone   // if bits are not set, CPU is 80286

	inc cx                          // processor type 3 in case of exit

	pushfd                          // transfer EFLAGS to EBX
	pop ebx

	mov eax,ebx                     // try to flip AC bit in EFLAGS
	xor eax,40000h

	push eax                        // transfer EAX to EFLAGS
	popfd
	pushfd                          // transfer EFLAGS back to EAX
	pop eax

	xor eax,ebx                     // AC bit fliped?
	jz short detect_processordone 	// if no, CPU is 386

	inc cx                          // processor type 4 in case of exit

	mov eax,ebx                     // try to flip ID bit in EFLAGS
	xor eax,200000h

	push eax                        // transfer EAX to EFLAGS
	popfd
	pushfd                          // transfer EFLAGS back to EAX
	pop eax

	xor eax,ebx                     // ID bit flipped?
	jz short detect_processordone   // if no, CPU is 486

	inc cx                          // processor type 5, CPU is 586
}

detect_processordone:

return(_CL);
}

/*
char FPU(void)
{
asm {

 Scratch	dw	?

 mov  	si,OFFSET Scratch
 mov  	word ptr [si],4321h

 .386
 .387

 fninit
 fnstsw [si]
 mov  	al,0
 cmp	byte ptr [si],0
 jne 	Exit
 mov    al,1
 }
Exit:

return(_AL);
} */