// Core graphics

void pmain(void);
void ptile(int xpos, int ypos, unsigned char tile);
void psprite32(int xpos, int ypos, unsigned char sprite,unsigned char frame,signed char speed);
void psprite16(int xpos, int ypos, unsigned char sprite,unsigned char frame);
void pbridge(int xpos, int ypos);
void Line(int a,int b,int c,int d);

// Auxilliary graphics

void Fade2Black(char);
void FadeFromBlack(char);
void PlotChars(void);
void PlotIcon(void);
void SetGPal(void);
void FixPal(void);
void SavePal(void);
void SetPal(void);
void Clear(void);
void SCopy(void);
void SRCopy(void);
void Screen(char);
void DVR(void);
void Outro(void);

// Audio

void ps2(char);
char LoadAudio(void);

// Interface

char MainMenu(void);
void Options(void);

// Misc Routines

char GameLoop(void);
void LoadGame(void);
void LoadLevel(void);
void LoadTitle(void);
void FinishedLevel(void);
void ReadAxis(void);
void WAIT(void);
void Pause(void);
void UnPause(void);
void QuitPlay(void);
void LoadLogo(char);
char CPU(void);
void interrupt keyb(...);
void interrupt (*oldkeyb)(...);
