 ECount=0;
 for(txloop=0;txloop<NENEMIES;txloop++)
  {
  if(Enemy[txloop].ctrl)ECount++;
  if(Enemy[txloop].ctrl==0||Enemy[txloop].ctrl>2)goto ENDMOLOOP; 	// Skip if Object Destroyed or Invisible

   if((Enemy[txloop].y-wposy)>191||(Enemy[txloop].y-wposy)<(-31)||(Enemy[txloop].x-wposx)>319||(Enemy[txloop].x-wposx)<-31){goto GOFFLIMITS;}
   psprite32(Enemy[txloop].x-wposx,Enemy[txloop].y-wposy,Enemy[txloop].type,Enemy[txloop].frame,Enemy[txloop].speed); // Plot
   GOFFLIMITS:
   Enemy[txloop].frame+=BonusFrame&1;

   if(!Enemy[txloop].status)
	{
	Enemy[txloop].x+=Enemy[txloop].speed;;
	Enemy[txloop].disp+=Enemy[txloop].speed;;

	if(Enemy[txloop].disp>=Enemy[txloop].maxdisp)
	 {
	 Enemy[txloop].speed=(-Enemy[txloop].speed);
	 Enemy[txloop].frame=16;
	 }
	if(Enemy[txloop].disp<=0)
	 {
	 Enemy[txloop].speed=(-Enemy[txloop].speed);
	 Enemy[txloop].frame=0;
	 }

	if(Enemy[txloop].speed>0){if(Enemy[txloop].frame==16)Enemy[txloop].frame=0;}
	else{if(Enemy[txloop].frame==32)Enemy[txloop].frame=16;}
	}

  ENDMOLOOP:

  asm {

  movzx ax,txloop
  mov   bl,13
  mul   bl
  mov   si,OFFSET Enemy
  add   si,ax

  cmp 	byte ptr [si],10		// Is it a motion mapped platform ?
  jne 	OLA

  mov   di,frameinc
  add   di,[si+3]
  and   di,127
  add   di,di
  add   di,di
  add   di,OFFSET Circular

  mov   ax,[di]					// Get x offset
  sar   ax,1
  mov   bx,[di+2]				// Get y offset
  sar   bx,1
  add	ax,[si+3]				// Add x pos
  add	bx,[si+5]				// Add y pos
  sub	ax,wposx				// Place in world
  sub	bx,wposy				// " ---    --- "

  cmp	ax,319;jg OLA         	// Check object is onscreen...
  cmp	ax,-31;jl OLA
  cmp	bx,190;jg OLA
  cmp	bx,-31;jl OLA			// ...Skip if not

  movzx dx,byte ptr [si+7]
  push	dx bx ax                // X Y TYPE <- In reverse
  call  far ptr @ptile$qiiuc	// Plot it
  add	sp,6					// Fix Stack
  }

  OLA:

  //----------------------------//
  //----------------------------//

  if(Enemy[txloop].ctrl==9)			// Platform
   {
   if((Enemy[txloop].y-wposy)>191||(Enemy[txloop].y-wposy)<-31||(Enemy[txloop].x-wposx)>319||(Enemy[txloop].x-wposx)<-31){goto OFFLIMITS4;}
   ptile(Enemy[txloop].x-wposx,Enemy[txloop].y-wposy,Enemy[txloop].type); // Plot
   OFFLIMITS4:

   //-------------------------------------------//

if(!MOTION)goto ENDMOLOOP2;

   Enemy[txloop].x+=Enemy[txloop].speed; 	// Alter X Pos
   Enemy[txloop].disp+=Enemy[txloop].speed; 	// Alter Disp

   if(Enemy[txloop].disp>=Enemy[txloop].maxdisp)// If at max disp
	Enemy[txloop].speed=(-Enemy[txloop].speed); // Reverse object motion
   if(Enemy[txloop].disp<=0)			// If at min disp
	Enemy[txloop].speed=(-Enemy[txloop].speed); // Reverse object motion
   }

  ENDMOLOOP2:

  if(Enemy[txloop].ctrl==7)			// Restart FLAG
   {
   if((Enemy[txloop].y-wposy)>191||(Enemy[txloop].y-wposy)<-31||(Enemy[txloop].x-wposx)>319||(Enemy[txloop].x-wposx)<-31){goto OFFLIMITS5;}
   ptile(Enemy[txloop].x-wposx,Enemy[txloop].y-wposy,Enemy[txloop].type); // Plot
   OFFLIMITS5:
   }

  ENDMOLOOP3:

  if(Enemy[txloop].ctrl==8)			// End Level FLAG
   {
   if((Enemy[txloop].y-wposy)>191||(Enemy[txloop].y-wposy)<-31||(Enemy[txloop].x-wposx)>319||(Enemy[txloop].x-wposx)<-31){goto OFFLIMITS6;}
   ptile(Enemy[txloop].x-wposx,Enemy[txloop].y-wposy,Enemy[txloop].type); // Plot
   OFFLIMITS6:
   }

  ENDMOLOOP4:

DONEE:

  if(GRID&&(Enemy[txloop].ctrl==3||Enemy[txloop].ctrl==4))			// SPRING & ACID Indicator
   {
   entry=Enemy[txloop].x;
   entry-=wposx;

   if(!(entry<288&&entry>0))goto NOIND;

   asm mov di,entry

   entry=Enemy[txloop].y;
   entry-=wposy;

   if(!(entry<167&&entry>0))goto NOIND;

   entry*=320;

   asm add di,entry

   for(entry=0;entry<32;entry++)
	{
	for(tyloop=0;tyloop<32;tyloop++)
	 {
	 asm xor byte ptr [es:di],128
	 asm inc di
	 }
	asm add di,288
	}
   }
   NOIND:

  ENDCDELOOP:	// END ENEMY COLLANI LOOP
  }


 BCount=0;
 for(looper=0;looper<NBONUSES;looper++)
  {
  if(Bonus[looper].ctrl==0)goto ENDCDBLOOP; 		// Skip if Object Destroyed
  BCount++;
  if((Bonus[looper].y-wposy)>191||(Bonus[looper].y-wposy)<-15||(Bonus[looper].x-wposx)>319||(Bonus[looper].x-wposx)<-15){goto ENDCDBLOOP;}
  psprite16(Bonus[looper].x-wposx,Bonus[looper].y-wposy,Bonus[looper].ctrl,BonusFrame); // Plot
  ENDCDBLOOP:
  }

BonusFrame++;
BonusFrame&=31;