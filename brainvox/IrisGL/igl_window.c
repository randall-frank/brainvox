#ifdef WIN32
#include <windows.h>
#endif

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

/*
 * Issues:
 *	Handle multiple screens???
 *
 */

/* vars */
#ifdef WIN32
static HINSTANCE hInstance = 0;
static int insetx[2] = {-1,-1};
static int insety[2] = {-1,-1};
#else
static Display	*dpy = NULL;
static Atom	aWMDeleteWin;
#endif
static int	dpyHeight;
static int	dpyWidth;
static int	dpyOrigX;
static int	dpyOrigY;
static int	iNoFork = 0;

/* current and window list */
static wigl    	*wlist;
static wigl    	*wcurrent = NULL;

/* cursors */
static mycur	curlist[MAX_CURSORS] = {0};

/* constraints */
static int	constraints = 0;
static int	c_minsize[2];
static int	c_maxsize[2];
static int	c_prefpos[2];
static int	c_prefsize[2];
static int	c_temppos[2];

#define CONS_MINSIZE	1
#define CONS_MAXSIZE	2
#define CONS_PREFPOS	4
#define CONS_PREFSIZE	8
#define CONS_TEMPPOS	16

/* protos */
static void set_current(wigl *w);
static wigl *new_wigl(void);
static void rem_wigl(wigl *w);
static int get_wid(void);

/* functions */

int is_big_endian(void)
{
	static int endian = -1;
	if (endian == -1) {
		short   sh[] = {1};
		char    *by;
		by = (char *)sh;
		endian = by[0] ? 0 : 1;
	}
	return(endian);
}

int igl2x11x(int x)
{
	return(x+dpyOrigX);
}
int x112iglx(int x)
{
	return(x-dpyOrigX);
}
int igl2x11y(int y)
{
	return(dpyHeight+dpyOrigY-y);
}
int x112igly(int y)
{
	return(dpyHeight+dpyOrigY-y);
}

#ifndef WIN32
static void getscreensize(Display *dpy) 
{
	Window	rw, pw, *cw;
	unsigned int	nkids;

	dpyOrigX = 0;
	dpyOrigY = 0;
	dpyHeight = DisplayHeight(dpy, DefaultScreen( dpy ));
	dpyWidth = DisplayWidth(dpy, DefaultScreen( dpy ));

	/* try to trim the display to the "exposed" region */
	/* by trimming full width and full height "panel" windows */
	if (XQueryTree(dpy, RootWindow( dpy, DefaultScreen( dpy ) ), &rw, 
		&pw, &cw, &nkids)) {
		int scrn[4];
		int i;
		scrn[0] = dpyOrigX;  scrn[1] = dpyOrigY;
		scrn[2] = dpyWidth;  scrn[3] = dpyHeight;
		for(i=0;i<nkids;i++) {
			int		x0,y0;
			int		x1,y1;
			unsigned int	w,h,bw,d;
			if (XGetGeometry(dpy,cw[i],&rw,&x0,&y0,&w,&h,&bw,&d)) {
			    x1 = x0 + w;
			    y1 = y0 + h;
#ifdef DEBUG
			    printf("Window: %d,%d %d,%d\n",x0,y0,x1,y1);
#endif
			    /* only "thin" windows */
			    if ((w < 150) || (h < 150)) {
				/* full width windows */
				if ((x0 <= 0) && (x1 >= dpyWidth)) {
					if (y0 <= 0) {
						if (y1 > scrn[1]) scrn[1]=y1;
					} else if (y1 >= dpyHeight) {
						if (y0 < scrn[3]) scrn[3]=y0;
					}
				} else
				/* full width windows */
				if ((y0 <= 0) && (y1 >= dpyHeight)) {
					if (x0 <= 0) {
						if (x1 > scrn[0]) scrn[0]=x1;
					} else if (x1 >= dpyWidth) {
						if (x0 < scrn[2]) scrn[2]=x0;
					}
				}
			    }
			}
		}
		XFree(cw);
		dpyOrigX = scrn[0];  dpyOrigY = scrn[1];
		dpyWidth = scrn[2] - scrn[0];  dpyHeight = scrn[3] - scrn[1];
#ifdef DEBUG       
		printf("Result: %d,%d %d,%d\n",dpyOrigX,dpyOrigY,
				dpyWidth,dpyHeight);
#endif
	}
}
#endif

#ifndef WIN32
Display *get_display(void)
{
	int i;
	if (!dpy) {
		dpy = XOpenDisplay(NULL);
		if (!dpy)  return(NULL);
		aWMDeleteWin = XInternAtom( dpy, "WM_DELETE_WINDOW", False);
		buildtables();
		getscreensize(dpy);
		for(i=0;i<MAX_CURSORS;i++) curlist[i].xcur = 0;
	}
	return(dpy);
}

int get_screen(void)
{
	return(DefaultScreen( get_display() ));
}

Atom get_WMAtom(void)
{
	get_display();
	return(aWMDeleteWin);
}
#endif

#ifdef WIN32
static int _display_init = 0;
void get_display(void)
{
	if (!_display_init) {
		RECT r;
		_display_init = 1;
		SystemParametersInfo(SPI_GETWORKAREA,0,&r,0);
	        dpyOrigX = r.left;
	        dpyOrigY = r.top;
	        dpyHeight = r.bottom - r.top;
	        dpyWidth = r.right - r.left;
	}
}
#endif

int get_dpyheight(void)
{
	get_display();
	return(dpyHeight);
}

int get_dpywidth(void)
{
	get_display();
	return(dpyWidth);
}

/* find the lowest unused window ID */
static int get_wid(void)
{
	wigl    *p;
	int	wid = 0;
	int	good = 0;

	while(!good) {
		wid += 1;
		p = wlist;
		good = 1;
		while(p) {
			if (p->wid == wid) {
				good = 0;
				p = NULL;
			} else {
				p = p->next;
			}
		}
	}
	return(wid);
}

static void set_current(wigl *w)
{
	if (w != wcurrent) { 
		wcurrent = w;
#ifndef WIN32
   		if (!glXMakeCurrent( get_display(), w->win, w->ctx )) {
#else
		if (!wglMakeCurrent(w->hdc,w->hglrc)) {
#endif
			printf("Unable to make current: %d\n",w);
		}
	} 
}

#ifdef WIN32
HWND get_current_hwnd(void)
{
	return(wcurrent->hwnd);
}
#endif

wigl *get_current(void)
{
	return(wcurrent);
}

wigl *find_wigl_wid(long int wid)
{
	wigl	*p = wlist;
	while(p) {
		if (p->wid == wid) return(p);
		p = p->next;
	}
	return(NULL);
}

#ifdef WIN32
wigl *find_wigl_win(HWND hwnd)
{
	wigl	*p = wlist;
	while(p) {
		if (p->hwnd == hwnd) return(p);
		p = p->next;
	}
	return(NULL);
}
#else
wigl *find_wigl_win(Window win)
{
	wigl	*p = wlist;
	while(p) {
		if (p->win == win) return(p);
		p = p->next;
	}
	return(NULL);
}
#endif

static wigl *new_wigl(void)
{
	wigl	*p = (wigl *)calloc(1,sizeof(wigl));
	if (!p) return(NULL);

	p->wid = get_wid();
	p->next = wlist;
	wlist = p;

	return(p);
}

static void rem_wigl(wigl *w)
{
	wigl	*p = wlist;
	wigl	*l = NULL;

	while(p) {
		if (p == w) {
			if (l) {
				l->next = p->next;
			} else {
				wlist = p->next;
			}
			free(w);
			return;
		}
		l = p;
		p = l->next;
	}
	return;
}

#ifndef WIN32
int attrib[] = { GLX_RGBA,
                 GLX_RED_SIZE, 1,
                 GLX_GREEN_SIZE, 1,
                 GLX_BLUE_SIZE, 1,
                 GLX_DOUBLEBUFFER,
                 GLX_DEPTH_SIZE, 16,
                 None };

long winopen(char *title)
{
	XSetWindowAttributes attr;
	unsigned long 	mask;
	wigl		*w;
	int		width=256, height=256;
	int		i,x=0,y=0;

	if (!iNoFork) {
		if (fork() != 0) _exit(0);
		iNoFork = 1;
	}
	get_display();
        if (!dpy) return(-1);

	w = new_wigl();
	if (!w) return(-1);

	w->visinfo = glXChooseVisual( get_display(), 
		DefaultScreen( get_display() ), attrib);
	if (!w->visinfo) {
		rem_wigl(w);
		return(-1);
	}

        w->cmap = XCreateColormap( get_display(), 
		RootWindow( get_display(), DefaultScreen( get_display() )), 
		w->visinfo->visual, AllocNone);
	if (!w->cmap) {
		XFree(w->visinfo);
		rem_wigl(w);
		return(-1);
	}

	attr.background_pixel = 0;
   	attr.border_pixel = 0;
   	attr.colormap = w->cmap;
   	attr.event_mask = StructureNotifyMask | ExposureMask;
   	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	if (constraints & CONS_PREFSIZE) {
		width = c_prefsize[0];
		height = c_prefsize[1];
	}
	if (constraints & CONS_MINSIZE) {
		if (width < c_minsize[0]) width = c_minsize[0];
		if (height < c_minsize[1]) height = c_minsize[1];
	}
	if (constraints & CONS_MAXSIZE) {
		if (width < c_maxsize[0]) width = c_maxsize[0];
		if (height < c_maxsize[1]) height = c_maxsize[1];
	}
	/* set the window position */
	if (constraints & CONS_PREFPOS) {
		x = igl2x11x(c_prefpos[0]);
		y = igl2x11y(c_prefpos[1]+c_prefsize[1]);
	} else {
		/* if not specified, place it near the cursor */
		int	xx,yy;
		unsigned int	mask;
		Window	rw,cw;
		XQueryPointer(get_display(), 
			RootWindow(get_display(),DefaultScreen(get_display())), 
			&rw,&cw, &x,&y, &xx,&yy, &mask);
		x = x - (width*0.5);
		y = y - (height*0.5);
		if (x < 0) x = 0;
		if (y < 0) y = 0;
		constraints |= CONS_TEMPPOS;
		c_temppos[0] = x;
		c_temppos[1] = y;
	}
   	w->win = XCreateWindow( get_display(), 
		RootWindow( get_display(),DefaultScreen( get_display() )), 
		x,y, width, height, 0, w->visinfo->depth, InputOutput,
                w->visinfo->visual, mask, &attr );
	if (!w->win) {
		XFreeColormap(get_display(), w->cmap);
		XFree(w->visinfo);
		rem_wigl(w);
		return(-1);
	}

	w->size[0] = width;
	w->size[1] = height;
	w->doublebuffer = 1;
	w->iconic = 0;
	wcurrent = w;
	winconstraints();
	wcurrent = NULL;

	XSelectInput(get_display(),w->win,StructureNotifyMask|ButtonPressMask|
		ButtonReleaseMask|EnterWindowMask|ExposureMask|
		KeyPressMask|KeyReleaseMask);

   	w->ctx = glXCreateContext( get_display(), w->visinfo, NULL, True );
	if (!w->ctx) {
		XDestroyWindow(get_display(), w->win);
		XFreeColormap(get_display(), w->cmap);
		XFree(w->visinfo);
		rem_wigl(w);
		return(-1);
	}

	XStoreName(get_display(), w->win, title);
	XSetWMProtocols( get_display(), w->win, &aWMDeleteWin, 1);

        XMapWindow( get_display(), w->win );

	wcurrent = NULL;
	set_current(w);

	qdevice(INPUTCHANGE);
	qdevice(REDRAW);

	setcursor(0,0,0); 

	gl_setupwindow();

	return(w->wid);
}
void win2clip_(long *wid)
{
}
void WIN2CLIP(long *wid)
{
   win2clip_(wid);
}
#else

static void  get_insets(HWND hWnd, int dx[2], int dy[2])
{
	RECT r,cr;
	GetWindowRect(hWnd, &r);
	GetClientRect(hWnd, &cr);
	ClientToScreen(hWnd, (LPPOINT)&(cr.left));
	ClientToScreen(hWnd, (LPPOINT)&(cr.right));
	dx[0] = cr.left - r.left;
	dx[1] = r.right - cr.right;
	dy[1] = r.bottom - cr.bottom;
	dy[0] = cr.top - r.top;
	return;
}

LONG WINAPI IGLWinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND
CreateOpenGLWindow(char* title,int x,int y,int width,int height,DWORD flags)
{
    int         pf;
    HDC         hDC;
    HWND        hWnd;
    WNDCLASS    wc;
    PIXELFORMATDESCRIPTOR pfd;

    /* only register the window class once - use hInstance as a flag. */
    if (!hInstance) {
        hInstance = GetModuleHandle(NULL);
        wc.style         = CS_OWNDC;
        wc.lpfnWndProc   = (WNDPROC)IGLWinProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
        wc.hCursor       = NULL; /* LoadCursor(NULL, IDC_ARROW); */
        wc.hbrBackground = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = "OpenGL";
        if (!RegisterClass(&wc)) return NULL;

        hWnd = CreateWindow("OpenGL", "temp", 
                    WS_CAPTION|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_THICKFRAME, 
                    10, 10, 100, 100, NULL, NULL, hInstance, NULL);
        if (!hWnd) return NULL;
        get_insets(hWnd,insetx,insety);
        DestroyWindow(hWnd);
#if 0
printf("INIT inset = %d %d : %d %d\n",insetx[0],insetx[1],insety[0],insety[1]);
#endif
    }

    hWnd = CreateWindow("OpenGL", title, WS_OVERLAPPEDWINDOW |
                      WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
                      x-insetx[0], y-insety[0], width+insetx[0]+insetx[1], 
                      height+insety[0]+insety[1], NULL, NULL, hInstance, NULL);
    if (hWnd == NULL) return NULL;

    hDC = GetDC(hWnd);

    /* there is no guarantee that the contents of the stack that become
       the pfd are zeroed, therefore _make sure_ to clear these bits. */
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | flags;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 32;
    pfd.cDepthBits   = 16;

    pf = ChoosePixelFormat(hDC, &pfd);
    if (pf == 0) return 0;

    if (SetPixelFormat(hDC, pf, &pfd) == FALSE) return 0;

    DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    ReleaseDC( hWnd, hDC);

    buildtables();

    return hWnd;
}

static void update_size(wigl *w)
{
   int resize = 0;
   RECT r;
   GetClientRect(w->hwnd,&r);
   w->size[0] = r.right - r.left; /* LOWORD(lParam); */
   w->size[1] = r.bottom - r.top; /* HIWORD(lParam); */
#if 0
printf("RJF %d %d : %d : %d %d\n",w->size[0],w->size[1],w->hasminsize,w->minsize[0],w->minsize[1]);
#endif
   if (w->hasminsize) {
      if (w->size[0] < w->minsize[0]) {
         w->size[0] = w->minsize[0];
         resize = 1;
      }
      if (w->size[1] < w->minsize[1]) {
         w->size[1] = w->minsize[1];
         resize = 1;
      }
   }
   if (w->hasmaxsize) {
      if (w->size[0] > w->maxsize[0]) {
         w->size[0] = w->maxsize[0];
         resize = 1;
      }
      if (w->size[1] > w->maxsize[1]) {
         w->size[1] = w->maxsize[1];
         resize = 1;
      }
   }
   if (resize) {
      SetWindowPos(w->hwnd,NULL,0,0,
         w->size[0]+w->insetx[0]+w->insetx[1],
         w->size[1]+w->insety[0]+w->insety[1],
         SWP_NOMOVE|SWP_NOZORDER);
   }
#if 0
printf("RJF = SIZE= %d %d\n",w->size[0],w->size[1]);
#endif
}

LONG WINAPI 
IGLWinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    wigl *w = NULL; 
    int i = 0;
    static PAINTSTRUCT ps;

    if (uMsg == WM_CREATE) {
	w = get_current();
	if (!w) return 0;
        w->hwnd = hWnd;
	get_insets(hWnd,w->insetx,w->insety);
/*        update_size(w); (with proper inset guess, not needed) */
        return 0;
    }

    w = find_wigl_win(hWnd);
    if (!w) return DefWindowProc(hWnd, uMsg, wParam, lParam);

#if 0
printf("WinProc: %X\n",uMsg);
#endif
    switch(uMsg) {
    case WM_GETMINMAXINFO:
/* TODO: make this work - much better than the current approach */
	break;
    case WM_ACTIVATE:
        if (wParam) qenter_collapse(INPUTCHANGE,w->wid);
/*        return 0; */
	break;

    case WM_LBUTTONDOWN:
        i = 1;
    case WM_LBUTTONUP:
        if (isqueued(MOUSE3)) qenter(MOUSE3,i);
        return 1;
	break;

    case WM_MBUTTONDOWN:
        i = 1;
    case WM_MBUTTONUP:
        if (isqueued(MOUSE2)) qenter(MOUSE2,i);
        return 1;
	break;

    case WM_RBUTTONDOWN:
        i = 1;
    case WM_RBUTTONUP:
        if (isqueued(MOUSE1)) qenter(MOUSE1,i);
        return 1;
	break;

    case WM_KEYDOWN:
        i = 1;
    case WM_KEYUP:
        if (isqueued(f_x2gl(wParam))) qenter(f_x2gl(wParam),i);
        return 1;
	break;

    case WM_PAINT:
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        qenter_collapse(REDRAW,w->wid);
        return 1;
	break;

    case WM_MOVE:
        w->pos[0] = (short)(LOWORD(lParam));
        w->pos[1] = (short)(HIWORD(lParam));
        qenter_collapse(REDRAW,w->wid);
#if 0
printf("RJF - WM_MOVE %d %d\n",w->pos[0],w->pos[1]);
#endif
	break;

    case WM_SIZE:
#if 0
printf("RJF - WM_SIZE\n");
#endif
        update_size(w);
        qenter_collapse(REDRAW,w->wid);
	break;

    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT) {
            if (w->current_curs) {
                SetCursor(curlist[w->current_curs].xcur);
            } else {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
            }
            return 1;
        }
	break;

    case WM_CHAR:
        if (isqueued(KEYBD)) qenter(KEYBD, wParam);
        return 1;
	break;

    case WM_CLOSE:
        if (isqueued(WINQUIT)) {
            qenter(WINQUIT,w->wid);
        } else {
            PostQuitMessage(0);
        }
        return 1;
	break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/*
 * Messages to be mapped:

REDRAW
enter/focus -> INPUTCHANGE 
MOUSE1/MOUSE2/MOUSE3 if (dev_table[MOUSEX])
WINQUIT if (dev_table[WINQUIT]) exit, else map
WINTHAW/WINFREEZE - min/max
KEYBD -> ascii code if (dev_table[KEYBD]) map KEYBD,ascii on down
check dev_table[x2gl[VK_...]] then map x2gl[VK_...],(up and down)

timers are done in the qread() poll

 */

void win2clip_(long *wid)
{
  RECT r;
  HDC hdc;
  HBITMAP hbm,sbm;

  wigl	*w = find_wigl_wid(*wid);
  if (!w) return;
  GetClientRect(w->hwnd,&r);
  hdc = CreateCompatibleDC(w->hdc);
  hbm = CreateCompatibleBitmap(w->hdc,r.right-r.left,r.bottom-r.top);
  sbm = SelectObject(hdc,hbm);
  BitBlt(hdc,0,0,r.right-r.left,r.bottom-r.top,w->hdc,0,0,SRCCOPY);
  if (OpenClipboard(w->hwnd)) {
     EmptyClipboard();
     SetClipboardData( CF_BITMAP, hbm);
     CloseClipboard();
  }
  SelectObject(hdc,sbm);
  DeleteObject(hbm);
  DeleteDC(hdc);
}
void WIN2CLIP(long *wid)
{
  win2clip_(wid);
}

long winopen(char *title)
{
	int dx = 10;
	int dy = 10;
    	POINT pt;
	wigl *w;

	if (!iNoFork) {
/* 
 *       with CreateProcess()... ???
 *
 *		if (fork() != 0) exit(0);
 */
		iNoFork = 1;
	}
	get_display();

	w = new_wigl();
	if (!w) return(-1);

	wcurrent = w;  /* for the WM_CREATE message */
        /* these are not accurate, but will be fixed in WM_CREATE handler */
        if (constraints & CONS_PREFSIZE) {
                dx = c_prefsize[0];
                dy = c_prefsize[1];
        }
        if (constraints & CONS_MINSIZE) {
                if (dx < c_minsize[0]) dx = c_minsize[0];
                if (dy < c_minsize[1]) dy = c_minsize[1];
        }
        if (constraints & CONS_MAXSIZE) {
                if (dx > c_maxsize[0]) dx = c_maxsize[0];
                if (dy > c_maxsize[1]) dy = c_maxsize[1];
        }
        /* set the window position */
        if (constraints & CONS_PREFPOS) {
                pt.x = igl2x11x(c_prefpos[0]);
                pt.y = igl2x11y(c_prefpos[1]+c_prefsize[1]);
        } else {
                /* if not specified, place it near the cursor */
                GetCursorPos(&pt);
                pt.x = pt.x - (dx*0.5);
                pt.y = pt.y - (dy*0.5);
                if (pt.x < 0) pt.x = 0;
                if (pt.y < 0) pt.y = 0;
                constraints |= CONS_TEMPPOS;
                c_temppos[0] = pt.x;
                c_temppos[1] = pt.y;
        }
        w->hwnd = CreateOpenGLWindow(title,pt.x,pt.y,dx,dy,PFD_DOUBLEBUFFER);
        w->size[0] = dx;
        w->size[1] = dy;
	w->hdc = GetDC(w->hwnd);
	w->hglrc = wglCreateContext(w->hdc);
        wglMakeCurrent(w->hdc, w->hglrc);

        wcurrent = NULL;  /* forces a "makecurrent" */
	set_current(w);  

	gl_setupwindow();

	qdevice(INPUTCHANGE);
	qdevice(REDRAW);

	setcursor(0,0,0); 

        winconstraints();

        /* try to set the icon... */
        if (strstr(title,"Brainvox") == title) {
            HICON hicon = LoadImage(hInstance,"brainvox",IMAGE_ICON,
                                    64,64,LR_SHARED);
            if (hicon) {
                SendMessage(w->hwnd, WM_SETICON, ICON_BIG, (LPARAM)hicon);
                SendMessage(w->hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hicon);
            }
        }

	return(w->wid);
}
#endif


void winclose( long wid )
{
	wigl	*w = find_wigl_wid(wid);
	if (!w) return;

#ifndef WIN32
/*   	glXMakeCurrent( get_display(), 0, NULL ); */
	XUnmapWindow( get_display(), w->win);
	XDestroyWindow( get_display(), w->win);
	glXDestroyContext( get_display(), w->ctx );
	XFreeColormap( get_display(), w->cmap);
	XFree( w->visinfo );
	XFlush( get_display() );
#else
	wglMakeCurrent(NULL,NULL);
	ReleaseDC(w->hwnd,w->hdc);
	wglDeleteContext(w->hglrc);
	DestroyWindow(w->hwnd);
#endif
	rem_wigl(w);
}

void winpop(void)
{
	wigl    *w = get_current();
	if (!w) return;
#ifndef WIN32
	XRaiseWindow(get_display(),w->win);
#else
	BringWindowToTop(w->hwnd);
#endif
}

void winpush(void)
{
	wigl    *w = get_current();
	if (!w) return;
#ifndef WIN32
	XLowerWindow(get_display(),w->win);
#else
	printf("winpush() unimplemented on WIN32\n");
#endif
}

void winiconify(long i)
{
	wigl    *w = get_current();
	if (!w) return;
#ifndef WIN32
	if (i && !w->iconic) {
		XIconifyWindow(get_display(),w->win,DefaultScreen(get_display));
	}
	if (!i && w->iconic) {
		/* ??? */
		XMapWindow(get_display(),w->win);
	}
#else
	printf("winiconify() unimplemented on WIN32\n");
#endif
}

long winget(void)
{
	wigl    *w = get_current();
	if (w) return(w->wid);
	return(-1);
}

void winset(long wid)
{
	wigl	*w = find_wigl_wid(wid);
#ifdef DEBUG
printf("winset(%d) = %d\n",wid,w);
#endif
	if (!w) return;
	set_current(w);
}

void wintitle(char *title)
{
	wigl	*w = get_current();
	if (!w) return;
#ifndef WIN32
	XStoreName(get_display(), w->win, title);
#else
	SetWindowText(w->hwnd, title);
#endif
}

/* apply current constraints to the current window */
void winconstraints(void)
{
#ifdef WIN32
        int x=0,y=0,dx=0,dy=0;
        UINT flags = SWP_NOZORDER;

	wigl	*w = get_current();
	if (!w) return;

	if (!constraints) return;

	if (!(constraints & CONS_PREFSIZE)) flags |= SWP_NOSIZE;
	if (!(constraints & CONS_PREFPOS)) flags |= SWP_NOMOVE;

	x = igl2x11x(c_prefpos[0]-w->insetx[0]);
	y = igl2x11y(c_prefpos[1]+c_prefsize[1]+w->insety[0]);
	if (constraints & CONS_TEMPPOS)  {
            x = c_temppos[0];
	    y = c_temppos[1];
            constraints &= (~CONS_TEMPPOS);
	}
	dx = c_prefsize[0];
	dy = c_prefsize[1];

	w->hasminsize = 0;
	w->hasmaxsize = 0;

	if (constraints & CONS_MINSIZE)  {
	    w->minsize[0] = c_minsize[0];
	    w->minsize[1] = c_minsize[1];
	    if (dx < c_minsize[0]) dx = c_minsize[0];
	    if (dy < c_minsize[1]) dy = c_minsize[1];
            w->hasminsize = 1;
        }
	if (constraints & CONS_MAXSIZE)  {
	    w->maxsize[0] = c_maxsize[0];
	    w->maxsize[1] = c_maxsize[1];
	    if (dx > c_maxsize[0]) dx = c_maxsize[0];
	    if (dy > c_maxsize[1]) dy = c_maxsize[1];
            w->hasmaxsize = 1;
        }

	SetWindowPos(w->hwnd,NULL,x,y,
           dx+w->insetx[0]+w->insetx[1],dy+w->insety[0]+w->insety[1],flags);

	if (constraints & CONS_PREFSIZE) {
	    w->size[0] = dx;
	    w->size[1] = dy;
            SetWindowLong(w->hwnd,GWL_STYLE,
		WS_CAPTION | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_THICKFRAME);
/*                | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX); */
	    SetWindowPos(w->hwnd,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER
                         |SWP_FRAMECHANGED);
	} else {
            SetWindowLong(w->hwnd,GWL_STYLE,
		WS_CAPTION | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS
                | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
	    SetWindowPos(w->hwnd,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER
                         |SWP_FRAMECHANGED);
    	}
	constraints = 0;
#else
	XSizeHints *sizehints;

	wigl	*w = get_current();
	if (!w) return;

	if (!constraints) return;

	sizehints = XAllocSizeHints();

	sizehints->x = igl2x11x(c_prefpos[0]);
	sizehints->y = igl2x11y(c_prefpos[1]+c_prefsize[1]);
	if (constraints & CONS_TEMPPOS)  {
		sizehints->x = c_temppos[0];
		sizehints->y = c_temppos[1];
	}
	sizehints->width  = c_prefsize[0];
	sizehints->height = c_prefsize[1];
	sizehints->base_width  = c_prefsize[0];
	sizehints->base_height = c_prefsize[1];
	sizehints->min_width = c_minsize[0];
	sizehints->min_height = c_minsize[1];
	sizehints->max_width = c_maxsize[0];
	sizehints->max_height = c_maxsize[1];
	sizehints->win_gravity = NorthWestGravity;

	sizehints->flags = 0;
	if (constraints & CONS_PREFSIZE) sizehints->flags |= USSize;
	if (constraints & CONS_PREFSIZE) sizehints->flags |= PSize;
	if (constraints & CONS_PREFPOS)  sizehints->flags |= USPosition;
	if (constraints & CONS_PREFPOS)  sizehints->flags |= PPosition;
	if (constraints & CONS_PREFPOS)  sizehints->flags |= PWinGravity;
	if (constraints & CONS_TEMPPOS)  {
		sizehints->flags |= PPosition;
		sizehints->flags |= USPosition;
		constraints &= (~CONS_TEMPPOS);
	}
	if (constraints & CONS_MINSIZE)  sizehints->flags |= PMinSize;
	if (constraints & CONS_MAXSIZE)  sizehints->flags |= PMaxSize;

	XSetNormalHints( get_display(), w->win, sizehints );
	XSetZoomHints( get_display(), w->win, sizehints );
	XSetWMNormalHints( get_display(), w->win, sizehints );

	if (constraints & CONS_PREFSIZE) {
		w->size[0] = sizehints->width;
		w->size[1] = sizehints->height;
	}

	XFree(sizehints);
	constraints = 0;
#endif
}

void minsize(long x, long y)
{
	c_minsize[0] = x;
	c_minsize[1] = y;
	constraints |= CONS_MINSIZE;
}

void maxsize(long x, long y)
{
	c_maxsize[0] = x;
	c_maxsize[1] = y;
	constraints |= CONS_MAXSIZE;
}

void prefposition2(long x1, long y1)
{
#if defined(osx)
	get_display();
	c_prefpos[0] = x1;
	c_prefpos[1] = y1; 
	constraints |= CONS_PREFPOS;
#else
	prefposition(x1,x1+c_minsize[0]-1,y1-(c_minsize[1]),y1);
#endif
}

void prefposition(long x1, long x2, long y1, long y2)
{
	get_display();
	c_prefsize[0] = abs(x1-x2);
	c_prefsize[1] = abs(y1-y2);
	if (c_prefsize[0] <= 0) c_prefsize[0] = 1;
	if (c_prefsize[1] <= 0) c_prefsize[1] = 1;
	c_prefpos[0] = x1;
        if (x2 < x1) c_prefpos[0] = x2;
	c_prefpos[1] = y1; 
        if (y2 < y1) c_prefpos[1] = y2;
	c_maxsize[0] = c_prefsize[0];
	c_maxsize[1] = c_prefsize[1];
	c_minsize[0] = c_prefsize[0];
	c_minsize[1] = c_prefsize[1];
	constraints |= CONS_PREFSIZE;
	constraints |= CONS_PREFPOS;
	constraints |= CONS_MAXSIZE;
	constraints |= CONS_MINSIZE;
}

void prefsize(long x, long y)
{
	c_prefsize[0] = x;
	c_prefsize[1] = y;
	c_maxsize[0] = c_prefsize[0];
	c_maxsize[1] = c_prefsize[1];
	c_minsize[0] = c_prefsize[0];
	c_minsize[1] = c_prefsize[1];
	constraints |= CONS_PREFSIZE;
	constraints |= CONS_MAXSIZE;
	constraints |= CONS_MINSIZE;
}

void	getsize(long *x, long *y)
{
	wigl	*w = get_current();
	if (!w) {
		*x = 0;
		*y = 0;
		return;
	}

	*x = w->size[0];
	*y = w->size[1];
}

void	getorigin(long *x,long *y)
{
	wigl	*w = get_current();
	if (!w) {
		*x = 0;
		*y = 0;
		return;
	}

	*x = x112iglx(w->pos[0]);
	*y = x112igly(w->pos[1]+w->size[1]);
}

void	reshapeviewport(void)
{
	wigl	*w = get_current();
#ifdef DEBUG
printf("Reshap=%d\n",w);
#endif
	if (!w) return;
	glViewport(0,0,w->size[0],w->size[1]);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0,w->size[0],0.0,w->size[1],-1,1);
	glMatrixMode(GL_MODELVIEW);
}

void foreground(void)
{
	iNoFork = 1;
}

/* not fully supported */

long getgdesc(long i)
{
	switch(i) {
		case GD_XPMAX:
			return(get_dpywidth());
			break;
		case GD_YPMAX:
			return(get_dpyheight());
			break;
		case GD_BITS_STENCIL:
			return(0);
			break;
		default:
			printf("Unknown getgdesc(%d)\n",i);
			break;
	}
	return(0);
}

long getdisplaymode(void)
{
	wigl	*w = get_current();
	if (!w) return(0);

	if (w->doublebuffer) return(DMRGBDOUBLE);

	return(DMRGB);
}

void RGBmode(void)
{
}

void gconfig(void)
{
}

void doublebuffer(void)
{
	wigl	*w = get_current();
	if (!w) return;
	w->doublebuffer = 1;
	glDrawBuffer(GL_BACK);
}

void singlebuffer(void)
{
	wigl	*w = get_current();
	if (!w) return;
	w->doublebuffer = 0;
	glDrawBuffer(GL_FRONT_AND_BACK);
}

void ringbell(void)
{
#ifdef WIN32
	MessageBeep(MB_ICONEXCLAMATION);
#else
	XBell(get_display(),0);
#endif
}

void mapcolor(unsigned short i,short r,short g,short b)
{
}


/* FORTRAN interface */
void winico_(long *i)
{
	winiconify(*i);
}

void wincon_(void)
{
	winconstraints();
}

long	winget_(void)
{
	return(winget());
}

void winset_(long *wid)
{
	winset(*wid);
}

void wintit_(char *title,long int *len)
{
	char	tmp[1024];
	memcpy(tmp,title,*len);
	tmp[*len] = '\0';
	wintitle(tmp);
}

void reshap_(void)
{
	reshapeviewport();
}

void getori_(long *wx,long *wy)
{
        getorigin(wx,wy);
}

void	getsiz_(long *x, long *y)
{
	getsize(x,y);
}

void minsiz_(long *x,long *y)
{
	minsize(*x,*y);
}

long winope_(char *s,long *len)
{
	char	tmp[256];
	memcpy(tmp,s,*len);
	tmp[*len] = '\0';
	return(winopen(tmp));
}

void winclo_( long *wid )
{
	winclose(*wid);
}

void foregr_(void)
{
	foreground();
}

void rgbmod_(void)
{
	RGBmode();
}

void double_(void)
{
	doublebuffer();
}
void doubleb_(void)
{
	doublebuffer();
}

void single_(void)
{
	singlebuffer();
}

void gconfi_(void)
{
	gconfig();
}


long getdis_(void)
{
        return(getdisplaymode());
}

long getgde_(long *i)
{
	return(getgdesc(*i));
}

void mapcol_(long *i,long *r,long *g,long *b)
{
	mapcolor(*i,*r,*g,*b);
}

void ringbe_(void)
{
	ringbell();
}

void prefpo_(long *x1, long *x2, long *y1, long *y2)
{
	prefposition(*x1,*x2,*y1,*y2);
}

void prefpo2_(long *x1, long *y1)
{
	prefposition2(*x1,*y1);
}

void prefsi_(long *x1,long *x2)
{
	prefsize(*x1,*x2);
}


/* ----------------- cursor code -------------------- */
void curstype(long typ)
{
	/* always C16X1 */
}

void defcursor(short n, const unsigned short c[128])
{
	int	i;

	if (n > MAX_CURSORS) return;
	if (n < 1) return;

	get_display();

	for(i=0;i<16;i++) {
		curlist[n].data[15-i] = c[i];
	}
#ifndef WIN32
	if (is_big_endian()) {
		byteswap(curlist[n].data,16*sizeof(short),sizeof(short));
	}
	if (curlist[n].xcur > 0) XFreeCursor(get_display(), curlist[n].xcur);
#else
	byteswap(curlist[n].data,16*sizeof(short),sizeof(short));
	if (curlist[n].xcur) DestroyCursor(curlist[n].xcur);
#endif
	curlist[n].xcur = 0;
}

void curorigin(short n,short x, short y)
{
	if (n > MAX_CURSORS) return;
	if (n < 1) return;

	get_display();

	curlist[n].hotspot[0] = x;
	curlist[n].hotspot[1] = 16-y;

#ifndef WIN32
	if (curlist[n].xcur > 0) XFreeCursor(get_display(), curlist[n].xcur);
#else
	if (curlist[n].xcur) DestroyCursor(curlist[n].xcur);
#endif
	curlist[n].xcur = 0;
}

void setcursor(short n,unsigned short c1,unsigned short c2)
{
#ifndef WIN32
	XColor	fg,bg;
#endif
	wigl	*w = get_current();
	if (!w) return;
	if (n > MAX_CURSORS) return;

#ifndef WIN32
	if (curlist[n].xcur == 0) {
		if (n == 0) {
			curlist[n].xcur = XCreateFontCursor( get_display(), XC_top_left_arrow);
		} else {
			Pixmap	ps;
			fg.red = 65535;
			fg.green = 0;
			fg.blue = 0;
			bg.red = 65535;
			bg.green = 65535;
			bg.blue = 65535;
			ps = XCreatePixmapFromBitmapData(get_display(), w->win, 
				(char *)curlist[n].data, 16, 16, 
				WhitePixel( get_display(), DefaultScreen(get_display())),
				BlackPixel( get_display(), DefaultScreen(get_display())), 1);
			curlist[n].xcur = XCreatePixmapCursor( get_display(), ps, ps,
				&fg, &bg, 
				curlist[n].hotspot[0], curlist[n].hotspot[1]);
			XFreePixmap( get_display(), ps);
		}
	}

	XDefineCursor( get_display(), w->win, curlist[n].xcur );
#else
	if (curlist[n].xcur == 0) {
            unsigned short tmp[16];
	    int i;
	    memcpy(tmp,curlist[n].data,sizeof(tmp));
	    for(i=0;i<16;i++) tmp[i] ^= 0xffff;
	    curlist[n].xcur = CreateCursor(GetModuleHandle(NULL),
                                       curlist[n].hotspot[0],
                                       curlist[n].hotspot[1],16,16,
                                       tmp,curlist[n].data);
	}
        SetCursor(curlist[n].xcur);
#endif
	w->current_curs = n;
}

void getcursor(short *cur,unsigned short *d1,unsigned short *d2,long *d3)
{
	wigl	*w = get_current();

	*cur = 0;
	if (!w) return;

	*cur = w->current_curs;
}

void srand_(long int *t)
{
	srand(*t);
}

void gexit(void)
{
        exit(0);
}

long gversion(char s[12])
{
	strcpy(s,"OpenGL EMU");
	return(0);
}

