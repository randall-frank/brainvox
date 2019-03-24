#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif

#include "igl_common.h"

#ifndef WIN32
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#endif

#include "gl.h"
#include "device.h"
#include "get.h"

/* Event interfaces */
static double gettimer(void);
static void check_for_events(void);

/* device tables */
static short	dev_table[0x7fff] = {0};
static double	noise_delta[4] = {0};
static double	timer_last[4] = {0};

/* device queue */
#define	Q_MAX_SIZE	500

typedef struct {
	short	dev;
	short	val;
} q_elem;

static  q_elem	q_list[Q_MAX_SIZE];

static	int	q_top = 0;
static 	int	q_bottom = 0;

/* mapping between X11 and GL keys */
static unsigned short *gl2x = NULL;
static unsigned short *x2gl = NULL;

static int glkeys[] = {AKEY,BKEY,CKEY,DKEY,EKEY,FKEY,GKEY,HKEY,IKEY,
	JKEY,KKEY,LKEY,MKEY, NKEY, OKEY, PKEY, QKEY, RKEY, SKEY, TKEY,
	UKEY, VKEY, WKEY, XKEY, YKEY, ZKEY,
	ZEROKEY, ONEKEY, TWOKEY, THREEKEY, FOURKEY, FIVEKEY, SIXKEY, SEVENKEY, EIGHTKEY, NINEKEY, BREAKKEY, SETUPKEY,
CTRLKEY, CAPSLOCKKEY, RIGHTSHIFTKEY, LEFTSHIFTKEY, NOSCRLKEY, ESCKEY, TABKEY,
RETKEY, SPACEKEY, LINEFEEDKEY, BACKSPACEKEY, DELKEY, SEMICOLONKEY, PERIODKEY,
COMMAKEY, QUOTEKEY, ACCENTGRAVEKEY, MINUSKEY, VIRGULEKEY, BACKSLASHKEY,
EQUALKEY, LEFTBRACKETKEY, RIGHTBRACKETKEY, LEFTARROWKEY, DOWNARROWKEY,
RIGHTARROWKEY, UPARROWKEY, PAD0, PAD1, PAD2, PAD3, PAD4, PAD5, PAD6, PAD7,
PAD8, PAD9, PADPF1, PADPF2, PADPF3, PADPF4, PADPERIOD, PADMINUS, PADCOMMA,
PADENTER, LEFTALTKEY, RIGHTALTKEY, RIGHTCTRLKEY, F1KEY, F2KEY, F3KEY,
F4KEY, F5KEY, F6KEY, F7KEY, F8KEY, F9KEY, F10KEY, F11KEY, F12KEY,
PRINTSCREENKEY, SCROLLLOCKKEY, PAUSEKEY, INSERTKEY, HOMEKEY, PAGEUPKEY,
ENDKEY, PAGEDOWNKEY, NUMLOCKKEY, PADVIRGULEKEY, PADASTERKEY, PADPLUSKEY,-1};

#ifdef WIN32
static int wkeys[] = {0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
                      0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
                      0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
                      0x59, 0x5a, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
                      0x36, 0x37, 0x38, 0x39,
VK_PAUSE, VK_SELECT, VK_LCONTROL, VK_CAPITAL, VK_RSHIFT, VK_LSHIFT, VK_SCROLL,
VK_ESCAPE, VK_TAB, VK_RETURN, VK_SPACE, VK_RETURN, VK_BACK,
VK_DELETE, VK_OEM_1, VK_OEM_PERIOD, VK_OEM_COMMA, VK_OEM_7, VK_OEM_3,
VK_OEM_MINUS, VK_OEM_3, VK_OEM_5, VK_OEM_PLUS, VK_OEM_4, VK_OEM_6,
VK_LEFT, VK_DOWN, VK_RIGHT, VK_UP, VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2,
VK_NUMPAD3, VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8,
VK_NUMPAD9, VK_HELP, VK_SNAPSHOT, VK_INSERT, VK_SLEEP, VK_DECIMAL, VK_SUBTRACT, VK_SEPARATOR,
VK_ACCEPT, VK_LMENU, VK_RMENU, VK_CONTROL, VK_F1, VK_F2, VK_F3, VK_F4,
VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,
VK_PRINT, VK_SCROLL, VK_PAUSE, VK_INSERT, VK_HOME, VK_PRIOR,
VK_END, VK_NEXT, VK_NUMLOCK, VK_OEM_2, VK_MULTIPLY, VK_ADD, -1};
#else
static int xkeys[] = { 'A','B','C','D','E','F','G','H','I','J','K',
	'L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
	'0','1','2','3','4','5','6','7','8','9',XK_Break,XK_Select,
XK_Control_L, XK_Caps_Lock, XK_Shift_R, XK_Shift_L, XK_Scroll_Lock, 
XK_Escape, XK_Tab, XK_Return, XK_space, XK_Linefeed, XK_BackSpace, 
XK_Delete, XK_semicolon, XK_period, XK_comma, XK_apostrophe, XK_grave, 
XK_minus, XK_asciitilde, XK_backslash, XK_equal, XK_braceleft, XK_braceright, 
XK_Left, XK_Down, XK_Right, XK_Up, XK_KP_0, XK_KP_1, XK_KP_2, XK_KP_3, 
XK_KP_4, XK_KP_5, XK_KP_6, XK_KP_7, XK_KP_8, XK_KP_9, XK_KP_F1, XK_KP_F2, 
XK_KP_F3, XK_KP_F4, XK_KP_Decimal, XK_KP_Subtract, XK_KP_Separator,
XK_KP_Enter, XK_Alt_L, XK_Alt_R, XK_Control_R, XK_F1, XK_F2, XK_F3, 
XK_F4, XK_F5, XK_F6, XK_F7,  XK_F8, XK_F9, XK_F10, XK_F11, XK_F12,
XK_Print, XK_Scroll_Lock, XK_Pause, XK_Insert, XK_Home, XK_Page_Up,
XK_End, XK_Page_Down, XK_Num_Lock, XK_Atilde, XK_KP_Multiply, XK_KP_Add,-1};
#endif

/* PADVIRGULEKEY = tilde */

int f_x2gl(unsigned short i)
{
	return(x2gl[i]);
}

Boolean isqueued( Device dev)
{
	return(dev_table[dev]);
}

void     qdevice( unsigned short dev )
{
	dev_table[dev] = 1;
}

void	unqdevice( unsigned short dev )
{
	dev_table[dev] = 0;
}

void     qenter_collapse( unsigned short dev, short val)
{
	int	i;
	if (q_top != q_bottom) {
		i = q_bottom;
		if (i >= Q_MAX_SIZE) i = 0;
		if ((q_list[i].dev == dev) && (q_list[i].val == val)) return;
		if (dev ==  REDRAW) {
			while((q_list[i].dev == dev) && (i != q_top)) {
				if (q_list[i].val == val) return;
				i = i + 1;
				if (i >= Q_MAX_SIZE) i = 0;
			}
		}
	}
	qenter(dev, val);
}

void     qenter( unsigned short dev, short val)
{
	q_bottom += 1;
	if (q_bottom >= Q_MAX_SIZE) q_bottom = 0;

/* wrap-around?? (lose an event) */
	if (q_bottom == q_top) {
#ifdef DEBUG
		printf("Warning: Event queue overflow\n");
#endif
		q_top += 1;
		if (q_top >= Q_MAX_SIZE) q_top = 0;
	}

	q_list[q_bottom].dev = dev;
	q_list[q_bottom].val = val;
}

long     qread( short *val )
{
	while(q_top == q_bottom) {
		check_for_events();
		if (q_top == q_bottom) {
#ifdef WIN32
                        Sleep(10);
#else
                	usleep(10000);
#endif
   		}
	}

	q_top += 1;
	if (q_top >= Q_MAX_SIZE) q_top = 0;

	*val = q_list[q_top].val;

	return(q_list[q_top].dev);
}

void     qreset( void )
{
	q_bottom = 0;
	q_top = 0;
}

long     qtest( void )
{
	int	i;

	check_for_events();
	if (q_bottom == q_top) return(0);

	i = q_top + 1;
	if (i >= Q_MAX_SIZE) i = 0;

	return(q_list[i].dev);
}

void	noise(unsigned short dev, short delta)
{
	noise_delta[dev-TIMER0] = delta;
	timer_last[dev-TIMER0] = gettimer();
}

long	getvaluator(unsigned short dev)
{	
#ifdef WIN32
        POINT   pnt;
#else
	Window	rw,cw;
#endif
	int	rx,ry,x,y;
	unsigned int	mask;
	int	i = -1;

	wigl	*w = get_current();
	if (!w) return(-1);

	if (dev == CURSORX) i = 0;
	if (dev == CURSORY) i = 1;
	if (dev == MOUSEX) i = 0;
	if (dev == MOUSEY) i = 1;

	if (i == -1) return(-1);

#ifdef WIN32
        GetCursorPos(&pnt);
        rx = pnt.x;
        ry = pnt.y;
#else
	XQueryPointer(get_display(), w->win, &rw,&cw, &rx,&ry, &x,&y, &mask); 
#endif

	if (!i) return( x112iglx(rx) );
	return( x112igly(ry) );
}

static int bTraditionalButtonMap = -1;
int Checkmapping(void) 
{
    if (bTraditionalButtonMap == -1) {
       bTraditionalButtonMap = 0;
       if (getenv("BRAINVOX_SWAP_BUTTONS"))  {
          if (atoi(getenv("BRAINVOX_SWAP_BUTTONS")) == 0) {
             bTraditionalButtonMap = 1;
          }
       }
    }
    return(bTraditionalButtonMap);
}

long guibtn_(void)
{  
    long ret = MOUSE3;
    if (Checkmapping()) ret = MOUSE1;
    return(ret);
}

long	getbutton(unsigned short dev)
{
#ifdef WIN32
        int i,j;
	wigl	*w = get_current();
	if (!w) return(-1);
      
	i = 0;
	if (dev == MOUSE1) i = VK_RBUTTON;
	if (dev == MOUSE2) i = VK_MBUTTON;
	if (dev == MOUSE3) i = VK_LBUTTON;
	if (i == 0) { 
        	i = gl2x[dev];
		if (i <= 0) return(0);
	}
	if (GetAsyncKeyState(i) & 0x8000) return(1);
#else
	Window  rw,cw;
	int     rx,ry,x,y;
	unsigned int    mask,i;

	wigl	*w = get_current();
	if (!w) return(-1);

	i = 0;
	if (dev == MOUSE1) i = Button3Mask;
	if (dev == MOUSE2) i = Button2Mask;
	if (dev == MOUSE3) i = Button1Mask;
	if (i) {
		XQueryPointer(get_display(),w->win, &rw,&cw, &rx,&ry, &x,&y, &mask);
		if (i & mask) return(1);
		return(0);
	}

	i = gl2x[dev];
	if (i > 0) {
		char 	keys[32];
		long	u;

		XQueryKeymap( get_display(), keys );
		u = i/8;
		i = i-(u*8);

		if (keys[u] & (1 << i)) return(1);
	}
#endif
	return(0);
}
/*
	MotionNotify??? (qenter(CURSORX?,MOUSEX?)
*/

static void check_for_events(void)
{
	int	i;
	double  c_time;
#ifndef WIN32
	wigl	*w;
	int	count;

	XFlush(get_display());
	count = XPending(get_display());

	/* read the events */
	while (count>0) {
		XEvent	event;

		count--;
		XNextEvent( get_display(), &event);

		w = find_wigl_win(event.xany.window);
		if (!w) continue;

		switch(event.type) {

			case Expose:
				if (event.xexpose.count == 0) {
					qenter_collapse(REDRAW,w->wid);
#ifdef DEBUG
	printf("Expose: %d\n",w->wid);
#endif
				}
				break;

			case EnterNotify:
				qenter_collapse(INPUTCHANGE,w->wid);
				break;

			case ButtonPress:
			case ButtonRelease:
				i = 1;
				if (event.type == ButtonRelease) i = 0;
				switch(event.xbutton.button) {
					case Button3:
						if (dev_table[MOUSE1]) {
							qenter(MOUSE1, i);
						}
						break;
					case Button2:
						if (dev_table[MOUSE2]) {
							qenter(MOUSE2, i);
						}
						break;
					case Button1:
						if (dev_table[MOUSE3]) {
							qenter(MOUSE3, i);
						}
						break;
                                }
				break;

			case ClientMessage:
				if (event.xclient.data.l[0] == get_WMAtom()) {
					if (dev_table[WINQUIT]) {
						qenter_collapse(WINQUIT,w->wid);
					} else {
						exit(0);
					}
				}
				break;

			case UnmapNotify: 
				w->iconic = 1;
				if (dev_table[WINFREEZE]) {
					qenter(WINFREEZE, w->wid);
				}
				break;

			case MapNotify:
				if (w->iconic) {
					w->iconic = 0;
					if (dev_table[WINTHAW]) {
						qenter(WINTHAW, w->wid);
					}
				}
				/* fall-thru */
			case ConfigureNotify:
				{
					int		x,y,xr,yr;
					unsigned int	wx,hy,bw,d;
					Window		rw,cw;

					XGetGeometry(get_display(),w->win,&rw,
						&x,&y,&wx,&hy,&bw,&d);
					XTranslateCoordinates( get_display(), 
						w->win,rw,0,0,&xr,&yr,&cw);

					if ((w->pos[0] != xr) ||
					    (w->pos[1] != yr)  ||
					    (w->size[0] != wx) ||
					    (w->size[1] != hy)) {
						qenter_collapse(REDRAW,w->wid);
						w->pos[0] = xr;
						w->pos[1] = yr;
						w->size[0] = wx;
						w->size[1] = hy;
#ifdef DEBUG
	printf("ConfigureNotify: %d\n",w->wid);
#endif
					}
				}
				break;

			case KeyPress:
			case KeyRelease: 
				{
				char buffer[20];
				int code,bufsize = 20;
				KeySym key;
				XComposeStatus compose;
				i = 0;
				if (event.xkey.type == KeyPress) i = 1;

				/* KEYBD device */
		                if (XLookupString(&event.xkey,buffer,
					bufsize,&key,&compose)) {
					if ((dev_table[KEYBD]) && i) {
						qenter(KEYBD,buffer[0]);
					}
				}

				/* queued key device (directly) */
				code = event.xkey.keycode;
				if (x2gl[code] > 0) {
					if (dev_table[x2gl[code]]) {
						qenter(x2gl[code],i);
					}
				}

				}
				break;

			case MappingNotify:
				XRefreshKeyboardMapping((XMappingEvent*)&event);
				buildtables();
				break;

			default:
				break;
		}

	}
#else
	{
	    MSG msg;
            BOOL hasm = 1;
            while(hasm) {
                hasm = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE);
                if (hasm) {
                    TranslateMessage(&msg); 
                    DispatchMessage(&msg); 
                }
            }
        }
#endif

	/* check timers */
	c_time = gettimer();
#ifdef DEBUG
printf("c_time=%f\n",c_time);
#endif
	for(i=0;i<4;i++) {
		if (dev_table[TIMER0+i]) {
#ifdef DEBUG
printf("timer %d time_last noise delta=%f %f\n",i,timer_last[i],noise_delta[i]);
#endif
			while(fabs(c_time - timer_last[i])>noise_delta[i]) {
				timer_last[i] += noise_delta[i];
				qenter(TIMER0+i,1);
#ifdef DEBUG
	printf("Timer: %d\n",TIMER0+i);
#endif
			}
		}
	}
	return;
}

/* FORTRAN */
void qdevic_(long *dev)
{
	qdevice(*dev);
}

void qenter_(long *dev,long *data)
{
	qenter(*dev,*data);
}

long qread_(short *data)
{
        return(qread(data));
}


long getval_(long *val)
{
	return(getvaluator(*val));
}

long getbut_(long *but)
{
	return(getbutton(*but));
}


/* handlers for timing and keycode tables */
static double gettimer(void)
{
	double	d;
#ifdef WIN32
	d = ((double)(GetTickCount())/1000.0)*60.0;
#else
	struct timeval 	t;
	gettimeofday(&t,NULL);
	d = ( (double)t.tv_sec + (((double)t.tv_usec)/1000000.))*60.;
#endif
	return(d);
}

void buildtables(void)
{
	int	i;
	int localx11[] = {131,133,132,134};
	int localgl[] = {LEFTARROWKEY,DOWNARROWKEY,RIGHTARROWKEY,UPARROWKEY};

	if (!gl2x) {

/* gl2x == IrisGL name -> X11 Keycode */
		gl2x = (unsigned short *)malloc(65536*sizeof(unsigned short));
/* x2gl == X11 Keycode -> IrisGL name */
		x2gl = (unsigned short *)malloc(65536*sizeof(unsigned short));
		if ((!gl2x) || (!x2gl))  {
			printf("Unable to allocate table memory.\n");
			exit(1);
		}
	}

	for(i=0;i<65536;i++) gl2x[i] = 0;
	for(i=0;i<65536;i++) x2gl[i] = 0;

#ifdef WIN32
	i = 0;
	while(glkeys[i] >= 0) {
		gl2x[glkeys[i]] = wkeys[i];
		x2gl[wkeys[i]] = glkeys[i];
		i++;
	}
#else
	i = 0;
	while(glkeys[i] >= 0) {
		KeyCode k = XKeysymToKeycode(get_display(),xkeys[i]);
		gl2x[glkeys[i]] = k;
		x2gl[k] = glkeys[i];
		i++;
	}
        for(i=0;i<sizeof(localgl)/sizeof(localgl[0]);i++) {
        	gl2x[localgl[i]] = localx11[i];
        	x2gl[localx11[i]] = localgl[i];
        }
#endif
	return;
}

void     qreset_( void )
{
	qreset();
}

void	noise_(long *dev, long *delta)
{
	noise(*dev,*delta);
}

long     qtest_( void )
{
	return(qtest());
}
