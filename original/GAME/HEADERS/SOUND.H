#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <alloc.h>
#include <sys\stat.h>
#include <string.h>
#include <values.h>
#include <ctype.h>

#define PTYPE far

#define MAXSOUNDS 8

#define WORD  unsigned int
#define DWORD unsigned long int
#define BYTE  unsigned char
#define FALSE 0
#define TRUE  1

#define DSP_RESET   6
#define DSP_READ    0x0a
#define DSP_WRITE   0x0c
#define DSP_WSTATUS 0x0c
#define DSP_RSTATUS 0x0e

#define DSP_DETECT 2

#define DSP_DMA_8BIT    0x14
#define DSP_DMA_ADC     0x24
#define DSP_SAMPLE_RATE 0x40

//#define DSP_MIDI_WRITE  0x38
//#define DSP_MIDI_READI  0x31
//#define DSP_MIDI_UART   0x35
#define DSP_SILENCE     0x80
#define DSP_SPKR_ON     0xd1
#define DSP_SPKR_OFF    0xd3
#define DSP_GET_VERS    0xe1

#define DSP_INVOKE_INTR 0xf2

void SetRate(DWORD rate);
int SB_Setup(void);

/* timerx prototypes */

void timer_on(void);
WORD timer_off(void);
WORD to_micro(WORD clk);
void clkdelay(WORD clicks);
void mdelay(WORD delay);
WORD mcalc(WORD micro);
void msdelay(WORD delay);

// function prototypes

unsigned char PTYPE *loadwave(char *wavefile,WORD *length);
void Go_Varmint(void);
void Dropdead_Varmint(void);

extern char 	*errname[];
extern DWORD 	vclock;
extern int 		sounds_in_queue;
extern WORD 	dma_bufferlen;