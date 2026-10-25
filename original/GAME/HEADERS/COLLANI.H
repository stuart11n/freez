// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// л                                                                         л
// л B O N U S    C O L L I S I O N    A N D    A N I M A T I O N    L O O P л
// л                                                                         л
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

// ллллллллллллл
// л           л
// л ANIMATION л
// л           л
// ллллллллллллл

  asm {

  xor  	bh,bh
  mov  	si,OFFSET Bonus

  mov	WordLoop,400

  jmp  	BONUSLP

  }

  SKIP:

  asm  	jmp  SKIPBONUS

  BONUSLP:

  asm {

  mov  	bl,[si]							// Load Bonus Type
  or   	bl,bl
  jz   	SKIP							// Skip if ctrl byte = 0  - Inactive
										//                   _______
  cmp  	bl,128							// If bonus is dying ______ \
  jb   	NODI     						//						   ||
										//						  _||
  inc  	byte ptr [si]					// Inc frame in ctrl byte __/
  }

  NODI:

  asm {
  mov  	ax,[si+1]						// Bonus X pos
  mov  	cx,ax
  sub  	ax,wposx

  cmp  	ax,-16
  jl   	SKIP
  cmp  	ax,319
  jg   	SKIP							// Check Processing Scope

  mov 	dx,[si+3]     					// Bonus Y pos
  sub  	dx,wposy

  cmp  	dx,-16
  jl 	SKIP   							// Check Processing Scope
  cmp  	dx,192
  jg 	SKIP

  cmp  	bl,128               			// Is bonus dying?
  jb   	DONT_DESTROY_BONUS				// ctrl byte is used as a counter _upto_ zero

  sub  	bl,224							// Normalise ctrl to frame number
										// Dying frame(s) index = 14
  push 	bx 14 dx ax   					// Setup stack: Frame Type Y X
  call 	far ptr @psprite16$qiiucuc		// Plot the Bonus
  pop  	ax dx bx						// Clear up stack
  add  	sp,2

  jmp  	SKIPBONUS						// Bonus is dying so skip collision detect
  }

  DONT_DESTROY_BONUS:					// Normal bonus processing

  asm {
  add  	cl,BonusFrame
  and  	cx,31

  push 	cx bx dx ax   					// Setup stack: Frame Type Y X
  call 	far ptr @psprite16$qiiucuc		// Plot the Bonus
  pop  	ax dx bx						// Clear up stack
  add  	sp,2

// ллллллллллллллллллллллл
// л                     л
// л COLLISION DETECTION л
// л                     л
// ллллллллллллллллллллллл

  cmp  	ax,169
  jg   	SKIP							// X collision test
  cmp  	ax,136
  jl   	SKIP
  cmp  	dx,108
  jg   	SKIP							// Y collision test
  cmp  	dx,76
  jl   	SKIP

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// жФФФФФФФФФФФФФФФФвФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЗ
// К Bonus Selector К                                                      К
// гФФФФФФФФФФФФФФФФаФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФН
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

  add	bx,bx
  jmp   word ptr ($+5)[bx]

  dw   	EBD						//  0
  dw   	ITS_A_GEM    			//  1
  dw   	ITS_A_COIN   			//  2
  dw   	ITS_A_ENERGY 			//  3
  dw   	EBD						//  4
  dw   	EBD			 			//  5
  dw   	ITS_A_RAPID  			//  6
  dw   	ITS_A_TIME   			//  7
  dw   	ITS_A_LIFE 	  			//  8
  dw   	ITS_A_SJUMP  			//  9
  dw   	ITS_A_MISC   			// 10
  dw   	ITS_A_MISC   			// 11
  dw   	ITS_A_MISC   			// 12
  dw   	ITS_A_MISC   			// 13
  dw   	ITS_A_TELE				// 14

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

  ITS_A_MISC:

  add  	Score,50				// Add to score
  push  2
  jmp  	ENDBONUSDETECT

  ITS_A_TELE:

  push  2
  jmp  	ENDBONUSDETECT

  ITS_A_GEM:

  add  	Score,150				// Add to score
  push  1
  jmp   ENDBONUSDETECT

  ITS_A_COIN:

  add  	Score,50				// Add to score
  push  1
  jmp  	ENDBONUSDETECT

  ITS_A_ENERGY:

  cmp  	Energy,100
  jne  	E_NEEDED
  jmp  	SKIPBONUS
  }

  E_NEEDED:

  asm {
  cmp  	Energy,60
  jae  	TOOMUCH
  push  6
  add  	Energy,40				// Add 5 to energy
  jmp  	EDONE
  }

  TOOMUCH:

  asm {
  mov  	Energy,100				// Set to MAX.
  push  6
  }

  EDONE:

  asm {
  add   Score,100				// Add to score
  jmp   ENDBONUSDETECT

  ITS_A_LIFE:

  push  4
  inc  	Lives					// inc lives
  add  	Score,150				// add to score
  jmp  	ENDBONUSDETECT

  ITS_A_SJUMP:

  cmp  	SuperJ,0                // If 0, haven't currently got super jump
  jnz  	SKIPBONUS
  push  3
  mov  	SuperJ,2048
  add  	Score,50				// Add to score
  jmp  	ENDBONUSDETECT

  ITS_A_RAPID:

  cmp  	RapidFire,0				// If 0, haven't currently got rapid fire
  jnz  	SKIPBONUS
  push  3
  mov  	RapidFire,2048
  add  	Score,50				// Add to score
  jmp  	ENDBONUSDETECT

  ITS_A_TIME:

  cmp  	Timer,0					// Skip if no time left
  jz 	SKIPBONUS
  push  5
  add  	Timer,3240
  add  	Score,100				// Add to score

  EBD:
  }

  // ллллллллллллллллллллллллллллллллллллллллллллл

  ENDBONUSDETECT:						// BC++ is bent.

  asm 	call far ptr @ps2$quc			// Play sound set in each bonus module
  asm 	add  sp,2

  asm 	mov  byte ptr [si],224			// Set the bonus to destroy

  SKIPBONUS:

  asm 	add  si,5				    	// Next Please

  asm 	dec  WordLoop					// Do the loop
  asm 	jz   ENDOFBLP
  asm 	jmp  BONUSLP

  ENDOFBLP:

  asm 	inc  byte ptr BonusFrame		// Increment frame..
  asm 	and  byte ptr BonusFrame,31		// ...to a max of 32

  // Tastey!

// л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л
//  л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// л                                                                         л
// л E N E M Y    C O L L I S I O N    A N D    A N I M A T I O N    L O O P л
// л                                                                         л
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
//  л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л
// л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л л

  asm mov	WordLoop,200
  asm mov   si,OFFSET Enemy

  asm jmp	ECAL
  CYBERSKIP:
  asm jmp	ENDMOLP
  ECAL:

  asm {
  cmp	byte ptr [si],0
  je	CYBERSKIP 						// Skip if inactive
  cmp	byte ptr [si],2
  ja	CYBERSKIP 						// Skip if not an animated enemy

  // Rangetest - Part A

  mov	ax,[si+3]						// Get X pos
  mov	bx,[si+5]						// Get Y pos

  sub	ax,wposx						// Normalise to processing window...
  sub	bx,wposy

  cmp   byte ptr [si+1],5				// Still process if frozen
  je	FROZEN_CONT

  cmp	ax,640
  jg 	CYBERSKIP						// Check object scope...
  cmp	ax,-320
  jl 	CYBERSKIP
  cmp	bx,400
  jg 	CYBERSKIP
  cmp	bx,-200
  jl 	CYBERSKIP						// ...Skip if outside window
  }

  FROZEN_CONT:

  // Plot object onscreen

  asm {
  btr	[si+12],2						// Default to enemy not on onscreen

  cmp	ax,319
  jg 	OL1       						// Check object is onscreen...
  cmp	ax,-31
  jl 	OL1
  cmp	bx,190
  jg 	OL1
  cmp	bx,-31
  jl 	OL1								// ...Skip if not

  mov	dl,[si+11]
  push  dx								// X Y TYPE FRAME SPEED <- In reverse
  mov	dl,[si+8]
  push	dx
  mov	dl,[si+7]
  push	dx bx ax
  call  far ptr @psprite32$qiiucuczc	// Plot it
  add	sp,10							// Fix Stack

  bts	[si+12],2						// Set enemy is onscreen
  }

  OL1:

  asm {
  bt	word ptr BonusFrame,1
  adc   [si+8],0 							// Inc frame

  //----------------------------------------//
  // Motion Control ------------------------//
  //----------------------------------------//

  cmp   byte ptr [si+1],0
  jnz 	DONT_MOVE 							// Check status is okay to move (0)
  }

  // Check for Main Character/Enemy Y-Plane alignment

//  asm 	cmp		byte ptr [si],1				// Skip if not a land enemy
//  asm	jne		NYR
  asm 	cmp		byte ptr IceBallNo,4		// Skip if not firing
  asm	je		NYR

  asm 	mov		ax,[si+5]					// Enemy Y Pos
  asm	mov     bx,wposy
  asm	add		bx,60
  asm   cmp		ax,bx
  asm 	jl		NYR
  asm	add		bx,40
  asm	cmp		ax,bx
  asm	jg		NYR

  // In alignment so change direction if facing each other and MC is firing

  asm 	mov   	ax,[si+3]					// Enemy X position
  asm 	sub   	ax,wposx
  asm 	cmp   	ax,144
  asm 	jg 		RIGHTHALFA					// Left or right of main char?

  asm 	test  	MCB,1
  asm 	jnz		NYR
  asm 	cmp   	byte ptr [si+11],0
  asm 	jl 		NYR

  asm 	jmp		OK_TO_SWITCH

  RIGHTHALFA:

  asm 	test  	MCB,1
  asm 	jz 		NYR							// Skip if facing left
  asm 	cmp   	byte ptr [si+11],0
  asm 	jg 		NYR							// Skip if not looking at iceball

  OK_TO_SWITCH:

  asm	mov		bl,[si+9]						// Get current displacement
  asm	cmp		bl,[si+10]
  asm	je		NYR								// Skip if at max
  asm 	or 		bl,bl							// or min displacement
  asm	je		NYR

  asm	neg		byte ptr [si+11]				// Reverse object direction

  NYR:											// Not in Y-Plane alignment

  //--------------------------------------------//

  asm 	movsx	ax,[si+11]						// Get speed
  asm	add		[si+3],ax						// Add it to the object x position

  asm	mov   	al,[si+11]
  asm	add		[si+9],al						// Alter displacement tracker

  // Enemy range bound checking

  asm	mov		al,[si+9]						// Get current displacement

  asm	cmp		al,[si+10]
  asm	jb 		MD_OK							// Is object at maximum displacement ?
  asm	neg		byte ptr [si+11]				// Reverse object direction
  MD_OK:

  asm 	or		al,al
  asm	jnz		MD2_OK							// Is object at minimum displacement
  asm	neg		byte ptr [si+11]				// Reverse object direction
  MD2_OK:

  asm	cmp		byte ptr [si+8],32
  asm	jne 	DONT_MOVE						// Is object at last frame
  asm	mov  	byte ptr [si+8],0				// Reset
  DONT_MOVE:

  //----------------------------------------//
  // Status Control ------------------------//
  //----------------------------------------//

  asm mov	al,[si+1]						// Get status

  asm or	al,al
  asm jnz 	CONT_						// Set flags for AL
  asm jmp   ENDMOLP
  CONT_:

  asm cmp	al,1
  asm jne 	NOT_HIT						// Hit?
  asm mov	byte ptr [si+8],32			// Set Frame
  asm mov	byte ptr [si+1],5			// Set status high to mask from rest of code
  asm jmp	NOT_KLD
  NOT_HIT:

  asm cmp	al,2
  asm jne 	NOT_FRZ						// Frozen?
  asm mov	byte ptr [si+8],44			// Set Frame
  asm mov	byte ptr [si+1],5			// Set status high to mask from rest of code
  asm mov	byte ptr [si+2],0			// Reset freeze timer
  asm jmp	NOT_KLD
  NOT_FRZ:

  asm cmp	al,3
  asm jne 	NOT_FRD						// Freed?
  asm mov	byte ptr [si+8],48			// Set Frame
  asm mov	byte ptr [si+1],5			// Set status high to mask from rest of code
  asm jmp	NOT_KLD
  NOT_FRD:

  asm cmp	al,4
  asm jne 	NOT_KLD						// Killed?
  asm mov	byte ptr [si+8],60			// Set Frame
  asm mov	byte ptr [si+1],6			// Set status high to mask from rest of code
  asm add	Score,100
  asm push  7
  asm call  far ptr @ps2$quc
  asm add   sp,2
  NOT_KLD:

  asm mov	al,[si+8]					// Get current object's frame

  asm cmp	al,43
  asm jne 	NOT_EOH						// End of hit (freezing) frame sequence?
  asm mov	byte ptr [si+1],2			// Hit -> Frozen
  asm jmp	NOT_EOE
  NOT_EOH:

  asm cmp	al,47
  asm jne 	NOT_EOF						// End of frozen frame sequence?
  asm mov	byte ptr [si+8],44
  asm jmp	NOT_EOE
  NOT_EOF:

  asm cmp	al,59
  asm jne 	NOT_EOFF					// End of defrost frame sequence?
  asm mov	byte ptr [si+1],0			// Frozen -> Normal
  asm mov 	byte ptr [si+8],0			// Reset normal running parameters
  asm jmp	NOT_EOE
  NOT_EOFF:

  asm cmp	al,75
  asm jne 	NOT_EOE						// End of existance?
  asm mov	byte ptr [si],0				// It's dead - hurrah!
  NOT_EOE:

  asm {
  cmp		byte ptr [si],2
  jne		NOT_AIRBORNE				// Is the beast airborne?

  mov		al,[si+1]					// Get status
  or		al,al
  jz 		AB_OK						// Its OK so skip gravity ctrl
  cmp		al,6
  je 		AB_OK						// Its OK so skip gravity ctrl

  mov		ax,[si+5]					// Y pos
  sub		ax,wposy
  cmp		ax,190
  jle 		DO_G_CHECK					// If it hasn't hit a border do ground check

  mov		byte ptr [si],0		    	// Destroy if hit border
  add		Score,100
  jmp		AB_OK
  }

  DO_G_CHECK:

  // Ground Check (Below 128 = OK)

  asm {

  mov   	bx,[si+3]
  sub   	bx,wposx
  add		bx,16						// Centre X

  mov		ax,[si+5]					// Y pos
  sub		ax,wposy
  add   	ax,32						// Centre Y

  shl		ax,6						// Calc offset into BX
  add		bx,ax
  shl		ax,2
  add		bx,ax

  cmp		byte ptr [es:bx],128
  jb		NOT_GNDED
  mov		byte ptr [si+1],4
  }

  NOT_GNDED:

  asm 		add	word ptr [si+5],3		// Object fall

  AB_OK:

  NOT_AIRBORNE:

  asm {
  cmp		byte ptr [si+2],255			// Is freeze counter at max ?
  jne		DO_FC
  mov		byte ptr [si+1],3
  cmp		Score,100
  jb 		SAMAX
  sub		Score,100
  jmp		DO_FC
  }
  SAMAX:
  asm mov	Score,0
  DO_FC:
  asm inc	byte ptr [si+2]

  //--------------------------------------------//
  // End of Enemy Animation/Motion Control code //
  //--------------------------------------------//

  ENDMOLP:

  asm {

  cmp 	byte ptr [si],10
  jne 	NOT_MAPPED						// Is it a motion mapped platform ?

  mov   di,frameinc
  add   di,[si+3]
  and   di,127
  shl   di,2
  add   di,OFFSET Circular

  mov   ax,[di]							// Get x offset
  sar   ax,1
  mov   bx,[di+2]						// Get y offset
  sar   bx,1
  add	ax,[si+3]						// Add x pos
  add	bx,[si+5]						// Add y pos
  sub	ax,wposx						// Place in world
  sub	bx,wposy						// " ---    --- "

  cmp	ax,319          				// Check object is onscreen...
  jg 	OLA
  cmp	ax,-31
  jl 	OLA
  cmp	bx,190
  jg 	OLA
  cmp	bx,-31
  jl 	OLA								// ...Skip if not

  movzx dx,byte ptr [si+7]
  push	dx bx ax                		// X Y TYPE <- In reverse
  call  far ptr @ptile$qiiuc			// Plot it
  add	sp,6							// Fix Stack
  }

  OLA:

  asm jmp DONEE

  NOT_MAPPED:

  asm {
  cmp	byte ptr [si],9
  jne	NOT_A_PLATFORM					// Check if a platform

  mov	ax,[si+3]
  mov	bx,[si+5]
  sub	ax,wposx
  sub	bx,wposy

  cmp	ax,319
  jg 	OL2         					// Check object is onscreen...
  cmp	ax,-31
  jl 	OL2
  cmp	bx,190
  jg 	OL2
  cmp	bx,-31
  jl 	OL2								// ...Skip if not

  movzx	dx,byte ptr [si+7]
  push	dx bx ax                		// X Y TYPE <- In reverse
  call  far ptr @ptile$qiiuc			// Plot it
  add	sp,6							// Fix Stack
  }

  OL2:

  asm {
  movsx	ax,byte ptr [si+11]				// Get speed
  add   [si+3],ax						// Alter x pos
  mov   al,[si+11]
  add	[si+9],al						// Alter displacement

  mov	al,[si+9]
  cmp	al,[si+10]
  jb 	MD3_OK							// Check for max displacement
  neg	byte ptr [si+11]				// Reverse object direction
  }
  MD3_OK:

  asm or	al,al
  asm jnz 	MD4_OK						// Is object at minimum displacement
  asm neg	byte ptr [si+11]			// Reverse object direction
  MD4_OK:

  asm jmp DONEE

  NOT_A_PLATFORM:

  //------------------------------------//

  asm {
  cmp	byte ptr [si],7
  je 	ITS_A_RESTART_FLAG 				// Check for Restart flag and EOL
  cmp	byte ptr [si],8
  je 	ITS_A_RESTART_FLAG
  jmp   NOT_A_FLAG
  }
  ITS_A_RESTART_FLAG:

  asm {
  mov	ax,[si+3]						// Get X pos
  mov	bx,[si+5]						// Get Y pos

  sub	ax,wposx						// Normalise to processing window...
  sub	bx,wposy

  cmp	ax,640
  jg	OL3								// Check object scope...
  cmp	ax,-320
  jl	OL3
  cmp	bx,400
  jg	OL3
  cmp	bx,-200
  jl	OL3      						// ...Skip if outside window

  // Plot object onscreen

  cmp	ax,319
  jg	OL3								// Check object is onscreen...
  cmp	ax,-31
  jl	OL3
  cmp	bx,190
  jg	OL3
  cmp	bx,-31
  jl	OL3								// ...Skip if not

  movzx	dx,byte ptr [si+7]
  push	dx bx ax                		// X Y TYPE <- In reverse
  call  far ptr @ptile$qiiuc			// Plot it
  add	sp,6 							// Fix Stack
  }

  OL3:

  asm jmp DONEE

  NOT_A_FLAG:

// ллллллллллллллллллллллл
// л                     л
// л COLLISION DETECTION л
// л                     л
// ллллллллллллллллллллллл

  asm jmp   INTOCD
  CYBERSKIP2:
  asm jmp	ENDCDELP
  NOT_IN_RNG:
  asm jmp	DONEE
  INTOCD:

  asm {
  test	MCB,4
  jnz	CYBERSKIP2
  mov 	al,[si]							// Check its alive
  or    al,al
  je	CYBERSKIP2
  cmp	al,8
  ja	CYBERSKIP2

  mov	bx,[si+3]						// Do range checking
  sub	bx,wposx
  add	bx,16

  cmp	bx,184
  jge 	NOT_IN_RNG
  cmp	bx,139
  jle 	NOT_IN_RNG

  mov	bx,[si+5]
  sub	bx,wposy
  add	bx,16

  cmp	bx,76
  jle 	NOT_IN_RNG
  cmp	bx,124
  jge 	NOT_IN_RNG           			// End of range checking

  cmp	al,3
  jne 	NOT_ACID						// Check for acid

  mov	JumpStat,15
  or	MCB,12

  cmp	Energy,24
  jle 	E_LOW							// Sort out energy
  sub	Energy,25
  jmp	ENDDETECT
  }
  E_LOW:
  asm mov	Energy,0
  asm jmp	ENDDETECT
  NOT_ACID:

  asm cmp	al,4
  asm jne 	NOT_A_SPRING				// Check for spring
  asm mov	GravStat,64
  asm mov	ah,[si+10]
  asm mov	JumpStat,ah
  asm or	MCB,8
  asm push  10
  asm call  far ptr @ps2$quc
  asm add   sp,2
  asm jmp	ENDDETECT
  NOT_A_SPRING:

  asm cmp	byte ptr [si+1],0;			// If enemy is okay - it's a collision
  asm jne   ENDDETECT					// sort out all the shit

  asm cmp 	Energy,4
  asm jle 	L_ENG
  asm sub	Energy,5
  asm jmp 	K_ENG
  L_ENG:
  asm mov	Energy,0
  K_ENG:
  asm mov	JumpStat,10
  asm or	MCB,12
  asm mov   HitCount,35

  asm cmp	byte ptr [si+11],0
  asm jl 	ADD_M_R
  asm mov	MomRight,80
  asm or	MCB,1
  asm jmp	ENDDETECT
  ADD_M_R:
  asm mov	MomLeft,80
  asm and	MCB,254

  ENDDETECT:

  asm {
  cmp   byte ptr [si+1],0
  je	DONEE       					// Jump on enemy to kill
  cmp	byte ptr [si+1],6
  je	DONEE
  cmp	JumpStat,0
  jne	DONEE

  mov	ax,[si+5]
  sub	ax,wposy
  cmp	ax,100
  jle 	DONEE

  mov	byte ptr [si+1],4				// Make main character jump
  mov	GravStat,64
  mov	JumpStat,8
  or	MCB,8
  }

  DONEE:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// л                                                                         л
// л I C E  B A L L  C O L L I S I O N  C O N T R O L  L O O P               л
// л                                                                         л
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

  asm cmp	IceBallNo,4
  asm jne	IB_STAT_OK 					// Is the max currently active?
  SKIPALL:
  asm jmp	ENDCDELP
  IBOOR:
  asm jmp	IBOOR2
  IB_STAT_OK:

  asm mov   di,OFFSET IceBall			// Set up pointer and loop
  asm mov	ByteLoop2,4

  IB_LP:

  asm {
  cmp	byte ptr [di],0
  je 	IBOOR							// Search for an active iceball

  mov	ax,[di+3]						// Range check Y
  add	ax,8
  mov	bx,[si+5]

  sub   dl,dl

  cmp	ax,bx
  jle 	IBOOR
  add	bx,32
  cmp 	ax,bx
  jge 	IBOOR

  inc   dl

  mov	ax,[di+1]						// Range check X
  add	ax,8
  mov	bx,[si+3]

  cmp	ax,bx
  jle 	IBOOR
  add	bx,32
  cmp	ax,bx
  jge 	IBOOR							// End range checking

  cmp	byte ptr [si],7
  jne 	NOT_RS							// Check for restart point

  mov   bx,LevelDataP
  mov	ax,[si+3]
  sub	ax,112
  mov	[bx],ax
  mov	ax,[si+5]
  sub	ax,84
  mov	[bx+2],ax						// Set new start points
  mov	al,[si+8]
  mov	[bx+4],al						// and direction to face at restart

  mov   cx,100							// Clear all restart points
  mov	bx,OFFSET Enemy
  }

  CRSL:

  asm cmp	byte ptr [bx],7
  asm jne	NXTCRSL
  asm mov	byte ptr [bx+7],60			// Reset if a flag

  NXTCRSL:

  asm add	bx,13
  asm loop	CRSL						// Goto next object in array

  asm mov	byte ptr [si+7],61			// Set the new one

  NOT_RS:

  // Check for end of level

  asm cmp	byte ptr [si],8
  asm jne	NOT_EOL

  asm cmp	LevelEnd,0
  asm jne 	NOT_EOL_YET2
  asm mov	LevelEnd,128
  NOT_EOL_YET2:
  asm mov	byte ptr [si+7],63
  NOT_EOL:

  asm cmp	byte ptr [si+1],0
  asm jnz 	NOFC
  asm mov   byte ptr [si+1],1           // Set enemy status as `HIT'
  NOFC:

  asm mov	byte ptr [di],0
  asm inc	IceBallNo

  IBOOR2:

  // Airborne enemy Iceball dodging mechanism

  // In [di] 1 = LEFT, 2 = RIGHT

  asm cmp   byte ptr [si+1],0			// If not normal, HIT -> ignore
  asm jne	NOTARGET
  asm cmp   byte ptr [si],2;
  asm jne	NOTARGET 					// Skip if not airborne
  asm bt	[si+12],2;
  asm jnc 	RESETARGET					// Skip if not onscreen
  asm cmp   dl,1;
  asm jne 	RESETARGET					// Skip if not Y aligned

  asm mov   ax,[si+3]					// Enemy X position
  asm sub   ax,wposx
  asm cmp   ax,144;
  asm jg 	RIGHTHALF					// Left or right of main char?

  asm test  MCB,1;
  asm jnz	NOTARGET

  asm jmp	OK_TO_DODGE

  RIGHTHALF:

  asm test  MCB,1;
  asm jz 	NOTARGET					// Skip if facing left

  OK_TO_DODGE:

  asm bts   [si+12],1					// Dodging in progress

  asm movzx ax,HalfFrame

  asm bt	[si+12],0
  asm jc 	MOVETDN						// Which direction?
  asm neg   ax

  MOVETDN:
  asm add   [si+5],ax              		// Move target
  asm jmp   NOTARGET

  RESETARGET:
  asm btr	[si+12],1
  asm jnc 	NOTARGET   					// Set not dodging
  asm xor	[si+12],1					// Toggle dir if just finished dodging

  NOTARGET:
  asm add	di,6
  asm dec   ByteLoop2
  asm jz    ENDCDELP
  asm jmp   IB_LP

  ENDCDELP:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

  asm add	si,13
  asm dec	WordLoop
  asm jz	END_ME_LP
  asm jmp	ECAL
  END_ME_LP:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// л                                                                         л
// л E N D  C O L L A N I  L O O P                                           л
// л                                                                         л
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// л                                                                         л
// л I N I T I A T E  I C E B A L L                                          л
// л                                                                         л
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

  asm {
  cmp 	Firing,20						// Fire just pressed last frame?
  jne	NO_NIB_INIT

  mov   ByteLoop,4
  mov	si,OFFSET IceBall
  }

  INIB_LP:

  asm {
  cmp	byte ptr [si],0
  jnz	NEXT_NIB						// Is this slot free?

  test	MCB,1
  je 	NIB_ELSE						// Fire iceball in direction set in MCB

  mov	byte ptr [si],2
  mov	ax,wposy
  add	ax,87
  mov	[si+3],ax
  mov	ax,wposx
  add	ax,168
  mov	[si+1],ax
  jmp	SET_NEW_IB
  }

  NIB_ELSE:

  asm {
  mov	byte ptr [si],1
  mov	ax,wposy
  add	ax,87
  mov	[si+3],ax
  mov	ax,wposx
  add	ax,136
  mov	[si+1],ax
  }

  SET_NEW_IB:

  asm mov	byte ptr [si+5],0
  asm dec	IceBallNo

  asm push  12
  asm call  far ptr @ps2$quc
  asm add   sp,2

  asm jmp	NO_NIB_INIT

  NEXT_NIB:

  asm add	si,6
  asm dec   ByteLoop
  asm jnz	INIB_LP

  NO_NIB_INIT:

// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
// л                                                                         л
// л A N I M A T E  I C E B A L L S                                          л
// л                                                                         л
// ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

  asm cmp 	IceBallNo,4
  asm jne	NIBSK

  asm jmp	NOICEBALLS
  IB_DEAD:
  asm jmp	ENDICE
  NIBSK:

  asm mov	ByteLoop,4
  asm mov	si,OFFSET IceBall

  AIBLP:

  asm {

  cmp	byte ptr [si],0
  jz    IB_DEAD

  // Ground Check (Below 128 = OK)

  mov	bx,[si+1]						// Get X Position
  sub	bx,wposx
  mov	cx,bx
  add	bx,8							// Centre X

  mov	ax,[si+3]						// Get Y Position
  sub	ax,wposy
  mov	dx,ax
  add   ax,8 							// Centre Y

  shl	ax,6							// Calc offset into BX
  add	bx,ax
  shl	ax,2
  add	bx,ax

  cmp	byte ptr [es:bx],128
  jb	NOT_GND
  mov	byte ptr [si],0
  inc	IceBallNo
  jmp	ENDICE
  }
  NOT_GND:

  // CX : x - Saved above
  // DX : y -  "-"   "-"

  asm {
  cmp	dx,192
  jg 	OLIA
  cmp	dx,-16
  jl 	OLIA
  cmp	cx,319
  jg 	OLIA
  cmp	cx,-16
  jl 	OLIA

  push  [si+5] 0 dx cx
  call  far  ptr @psprite16$qiiucuc
  add	sp,8
  }

  OLIA:

  asm {
  cmp	byte ptr [si],1
  jne	NOT_LEFT
  sub	word ptr [si+1],6
  cmp   RapidFire,0
  jz	NOT_LEFT
  sub	word ptr [si+1],3
  jmp	NOT_RGHT
  }

  NOT_LEFT:
  asm {
  cmp	byte ptr [si],2
  jne	NOT_RGHT
  add	word ptr [si+1],6
  cmp   RapidFire,0
  jz	NOT_RGHT
  add	word ptr [si+1],3
  }
  NOT_RGHT:

  asm {
  mov	cx,[si+1]
  sub	cx,wposx

  cmp	cx,319
  jle 	CLPOS
  inc	IceBallNo
  mov	byte ptr [si],0
  jmp   DCLP
  }
  CLPOS:

  asm {
  cmp	cx,-16
  jg 	DCLP
  inc	IceBallNo
  mov	byte ptr [si],0
  }
  DCLP:

  asm inc	byte ptr [si+5]
  asm and	byte ptr [si+5],31

  ENDICE:

  asm add	si,6
  asm dec	ByteLoop
  asm jz	NOICEBALLS
  asm jmp	AIBLP

  NOICEBALLS: