// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Tile Animator К                                                       К
// гФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

asm {
test byte ptr BonusFrame,7		// Approx 8 fps
jnz  NO_T_ANIM

inc  atilecount

mov  cx,32						// Init loops
mov  di,OFFSET tilemap			// & data word pointer to tilemap
mov  si,LevelDataP
add	 si,10						// & animated tile list

cmp  atilecount,4
jne  INC_T_ANIM
}

A_T_LOOP1:		       			// Reset if at max frame count

asm {
mov  bx,[si]					// Get current tile number
sub  [di+bx],3
add  si,2
loop A_T_LOOP1

mov  atilecount,0
jmp  NO_T_ANIM
}

INC_T_ANIM:           			// Inc frame otherwise

asm {
mov  bx,[si]
inc  [di+bx]
add  si,2
loop INC_T_ANIM
}

NO_T_ANIM:
