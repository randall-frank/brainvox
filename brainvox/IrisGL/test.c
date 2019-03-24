#include <stdio.h>
#include <stdlib.h>
#include "gl.h"
#include "device.h"
#include "get.h"


unsigned short mycur[] =     {0x0408, 0x0408, 0x0808, 0x1004,
    0x2004, 0x2002, 0x4002, 0x8802,
    0x9801, 0x6809, 0x1269, 0x126D,
    0x266A, 0x2668, 0x1A70, 0x0180};

int main(int argc, char **argv)
{
	long	dev;
	short   val;
	short   c = 0;
	int	pup,p2;
	long	wid;

	prefposition(100,200,600,800);
	wid = winopen("hello");

	qdevice(WINQUIT);
	qdevice(ESCKEY);
	qdevice(KEYBD);
	qdevice(TIMER0);
	noise(TIMER0,180);
	qdevice(MOUSE1);
	qdevice(MOUSE3);

	minsize(256,256);
	maxsize(512,512);
	winconstraints();

	defcursor(1, mycur);
	curorigin(1, 8, 8); 

	p2 = defpup("Op1|%lop2|op3");
	pup = defpup("%tMy Title|%mm1",p2);

	while (1) {
		dev = qread(&val);
		if (dev == WINQUIT) { exit(1);
		} else if (dev == REDRAW) { 
			long int	x,y,wx,wy;
			reshapeviewport();
			getorigin(&x,&y);
			getsize(&wx,&wy);
			printf("REDRAW: %d %d %d %d\n",x,y,wx,wy);
			cpack(0x00808080);
			clear();
			swapbuffers();
		} else if (dev == INPUTCHANGE) { printf("INPUTCHANGE\n");
		} else if (dev == ESCKEY) { exit(0);
		} else if (dev == KEYBD) { printf("char: %c\n",val);
		} else if (dev == TIMER0) { 
			printf("Timer:%d\n",val);
			printf("Poll:");
			printf("%d %d - %d - %d %d\n",getvaluator(MOUSEX),
			 getvaluator(MOUSEY),getbutton(MOUSE2),
                         getbutton(ZEROKEY),getbutton(AKEY));
		} else if (dev == MOUSE1) { printf("Mouse1:%d\n",val);
			if (val == 1) {
				c = 1 - c;
				setcursor(c,0,0); 
			}
		} else if (dev == MOUSE3) {
			printf("dopup=%d\n",dopup(pup));
		} else {
			printf("dev=%d val=%d\n",dev,val);
		}
	}
}
