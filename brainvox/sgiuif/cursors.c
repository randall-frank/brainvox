/*
 *
 *	to use:
 *
 *	.
 *	.
 *	winopen("name");
 *	.
 *	.
 *	percentdone(0.0);
 *	for(y=0; y<ysize; y++) {
 * 	    percentdone(100.0*y/ysize);
 *	    .
 *	    .
 *	}
 *	percentdone(100.0);
 *	.
 *	.
 *
 */
#include "gl.h"
#include "device.h"
#define ACURS		4
#define HCURS		6
#define NCURSORS	12
unsigned short hcurtab[HCURS][16] = {
 {0x0408, 0x0408, 0x0808, 0x1004, 0x2004, 0x2002, 0x2002, 0x1802,
 0x0802, 0x100A, 0x124C, 0x0DB0, 0x0000, 0x0000, 0x0000, 0x0000},
 
 {0x0408, 0x0408, 0x0808, 0x1004, 0x2004, 0x2002, 0x2002, 0x1802,
 0x0802, 0x080A, 0x124C, 0x1270, 0x2580, 0x2400, 0x1800, 0x0000},
 
 {0x0408, 0x0408, 0x0808, 0x1004, 0x2004, 0x2002, 0x2002, 0x1802,
 0x0802, 0x080A, 0x124C, 0x1270, 0x2640, 0x2640, 0x1A40, 0x0180},
 
 {0x0408, 0x0408, 0x0808, 0x1004, 0x2004, 0x2002, 0x2002, 0x1802,
 0x0802, 0x080A, 0x124C, 0x1248, 0x2648, 0x2648, 0x1A70, 0x0180},
 
 {0x0408, 0x0408, 0x0808, 0x1004, 0x2004, 0x2002, 0x2002, 0x1802,
 0x0801, 0x0809, 0x1249, 0x124D, 0x264A, 0x2648, 0x1A70, 0x0180},
 
 {0x0408, 0x0408, 0x0808, 0x1004, 0x2004, 0x2002, 0x4002, 0x8802,
 0x9801, 0x6809, 0x1249, 0x124D, 0x264A, 0x2648, 0x1A70, 0x0180},
};

unsigned short acurtab[ACURS][16] = {
    {0x0000, 0x07C0, 0x19F0, 0x21F8, 
    0x41FC, 0x41FC, 0x81FE, 0x81FE, 
    0xFFFE, 0xFF02, 0xFF02, 0x7F04, 
    0x7F04, 0x3F08, 0x1F30, 0x07C0},

    {0x0000, 0x07C0, 0x1FF0, 0x3FF8, 
    0x5FF4, 0x4FE4, 0x87C2, 0x8382, 
    0x8102, 0x8382, 0x87C2, 0x4FE4, 
    0x5FF4, 0x3FF8, 0x1FF0, 0x07C0},

    {0x0000, 0x07C0, 0x1F30, 0x3F08, 
    0x7F04, 0x7F04, 0xFF02, 0xFF02, 
    0xFFFE, 0x81FE, 0x81FE, 0x41FC, 
    0x41FC, 0x21F8, 0x19F0, 0x07C0},

    {0x0000, 0x07C0, 0x1830, 0x2008, 
    0x701C, 0x783C, 0xFC7E, 0xFEFE, 
    0xFFFE, 0xFEFE, 0xFC7E, 0x783C, 
    0x701C, 0x2008, 0x1830, 0x07C0},

};
unsigned short curtab[NCURSORS+1][16] = {
    {0x7FFE, 0x4002, 0x2004, 0x300C, 
    0x2894, 0x2424, 0x2244, 0x2244, 
    0x2344, 0x23C4, 0x27E4, 0x2FF4, 
    0x3FFC, 0x3FFC, 0x4FF2, 0x7FFE},

    {0x7FFE, 0x43C2, 0x2184, 0x300C, 
    0x2814, 0x2424, 0x2244, 0x2244, 
    0x22C4, 0x23C4, 0x27E4, 0x2FF4, 
    0x3FFC, 0x2FF4, 0x47E2, 0x7FFE},

    {0x7FFE, 0x47E2, 0x23C4, 0x308C, 
    0x2814, 0x2424, 0x2244, 0x2244, 
    0x22C4, 0x23C4, 0x27E4, 0x2FF4, 
    0x3FFC, 0x27F4, 0x4182, 0x7FFE},

    {0x7FFE, 0x4FE2, 0x27C4, 0x318C, 
    0x2914, 0x2424, 0x2244, 0x2244, 
    0x22C4, 0x23C4, 0x27E4, 0x2FF4, 
    0x37FC, 0x23F4, 0x4082, 0x7FFE},

    {0x7FFE, 0x4FF2, 0x27E4, 0x33CC, 
    0x2914, 0x2424, 0x2244, 0x2244, 
    0x2344, 0x23C4, 0x27E4, 0x2FF4, 
    0x37FC, 0x21C4, 0x4002, 0x7FFE},

    {0x7FFE, 0x4FFA, 0x27F4, 0x33EC, 
    0x2994, 0x2424, 0x2244, 0x2244, 
    0x22C4, 0x23C4, 0x27E4, 0x2FF4, 
    0x37CC, 0x2084, 0x4002, 0x7FFE},

    {0x7FFE, 0x5FFA, 0x2FF4, 0x37EC, 
    0x2B94, 0x2424, 0x2244, 0x2244, 
    0x2344, 0x23C4, 0x27E4, 0x2FF4, 
    0x318C, 0x2004, 0x4002, 0x7FFE},

    {0x7FFE, 0x7FFE, 0x3FF4, 0x37EC, 
    0x2BD4, 0x2424, 0x2244, 0x2244, 
    0x22C4, 0x23C4, 0x27E4, 0x2BD4, 
    0x300C, 0x2004, 0x4002, 0x7FFE},

    {0x7FFE, 0x7FFE, 0x3FFC, 0x3FFC, 
    0x2FD4, 0x2424, 0x2244, 0x2244, 
    0x2344, 0x23C4, 0x27E4, 0x2814, 
    0x300C, 0x2004, 0x4002, 0x7FFE},

    {0x7FFE, 0x7FFE, 0x3FFC, 0x3FFC, 
    0x2FF4, 0x26E4, 0x2244, 0x2244, 
    0x22C4, 0x23C4, 0x2424, 0x2814, 
    0x300C, 0x2004, 0x4002, 0x7FFE},

    {0x7FFE, 0x7FFE, 0x3FFC, 0x3FFC, 
    0x2FF4, 0x27E4, 0x23C4, 0x23C4, 
    0x2244, 0x2244, 0x2424, 0x2814, 
    0x300C, 0x2004, 0x4002, 0x7FFE},

/* and a little hand...  RJF*/
    {0x0408, 0x0408, 0x0808, 0x1004, 
    0x2004, 0x2002, 0x4002, 0x8802, 
    0x9801, 0x6809, 0x1269, 0x126D, 
    0x266A, 0x2668, 0x1A70, 0x0180},
/* and a little hourglass...  RJF*/
    {0x1ff0, 0x1ff0, 0x0820, 0x0820, 
    0x0820, 0x0c60, 0x06c0, 0x0100, 
    0x0100, 0x06c0, 0x0c60, 0x0820, 
    0x0820, 0x0820, 0x1ff0, 0x1ff0},

};

static int curindex = -1;
static short oldcursor = 0;
static int  acur = 0;
static int  hcur = 0;


void percentdone_(float *percent)
{
    int index; 
    unsigned short dummy;
    long ldummy;

    if((curindex == -1) && ((*percent) == 0.0)) 
	getcursor(&oldcursor,&dummy,&dummy,&ldummy);
    if (*percent > 99.9) {
	setcursor(oldcursor,0xfff,0xfff);
	curindex = -1;
    } else {
	index = (*percent)*(NCURSORS-2)/100.0;
	if (index<0)
	    index = 0;
	if (index > NCURSORS-2)
	    index = NCURSORS-2;
	if (index != curindex) {
	    curstype(C16X1);
	    defcursor(20,curtab[index]);
	    curorigin(20,7,7);
	    setcursor(20,0xfff,0xfff);
	    curindex = index;
	}
    }
}
void PERCENTDONE(float *percent)
{
percentdone_(percent);
}

void fingers_cursor_()
{
	int i;

	hcur++;
	if (hcur > (HCURS*2-2)) hcur = 0;
	i = hcur;
	if (i >= HCURS) {
		i = (HCURS*2-2) - i;
	}
	curstype(C16X1);
	defcursor(23,hcurtab[i]);
	curorigin(23,8,8);
	setcursor(23,0xfff,0xfff);
}
void FINGERS_CURSOR()
{
fingers_cursor_();
}

void beachball_cursor_()
{
	acur++;
	if (acur >= ACURS) acur = 0;
	curstype(C16X1);
	defcursor(22,acurtab[acur]);
	curorigin(22,8,8);
	setcursor(22,0xfff,0xfff);
}
void BEACHBALL_CURSOR()
{
beachball_cursor_();
}

void hand_cursor_()
{
	    curstype(C16X1);
	    defcursor(21,curtab[NCURSORS-1]);
	    curorigin(21,7,7);
	    setcursor(21,0xfff,0xfff);
}
void HAND_CURSOR()
{
hand_cursor_();
}

void wait_cursor_()
{
	    curstype(C16X1);
	    defcursor(21,curtab[NCURSORS]);
	    curorigin(21,7,7);
	    setcursor(21,0xfff,0xfff);
}
void WAIT_CURSOR()
{
wait_cursor_();
}

void arrow_cursor_()
{
	    setcursor(0,0xfff,0xfff);
}
void ARROW_CURSOR()
{
arrow_cursor_();
}
