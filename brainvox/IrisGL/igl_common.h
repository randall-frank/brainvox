#ifndef __IGL_H__
#define __IGL_H__

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#ifndef M_PI
#define M_PI 3.14159265358979
#endif
#else
#include "GL/gl.h"
#include "GL/glx.h"
#endif

#include "ext_names.h"

typedef struct {
  const GLsizei width;
  const GLsizei height;
  const GLfloat xorig;
  const GLfloat yorig;
  const GLfloat advance;
  const GLubyte *bitmap;
} BitmapCharRec, *BitmapCharPtr;

typedef struct {
  const char *name;
  const int num_chars;
  const int first;
  const BitmapCharRec * const *ch;
} BitmapFontRec, *BitmapFontPtr;

#define MAX_CURSORS 32

typedef struct {
	short		hotspot[2];
	unsigned short	data[16];
#ifdef WIN32
        HCURSOR         xcur;
#else
	Cursor		xcur;
#endif
} mycur;

typedef struct _wigl {
	struct _wigl 	*next;
	long int	wid;
#ifdef WIN32
        HWND            hwnd;
        HDC             hdc;
        HGLRC           hglrc;
	int		minsize[2];
	int		hasminsize;
	int		maxsize[2];
	int		hasmaxsize;
	int		insetx[2];
	int		insety[2];
#else
	Window		win;
	Colormap	cmap;
	GLXContext	ctx;
	XVisualInfo	*visinfo;
#endif
	int		size[2];
	int		pos[2];
	int		current_curs;
	int		doublebuffer;
	int		iconic;
} wigl;


/* external references */
int f_x2gl(unsigned short i);
int get_dpyheight(void);
int get_dpywidth(void);
int igl2x11x(int x);
int x112iglx(int x);
int igl2x11y(int y);
int x112igly(int y);
#ifndef WIN32
Atom get_WMAtom(void);
Display *get_display(void);
int get_screen(void);
void buildtables(void);
wigl *find_wigl_win(Window win);
#else
wigl *find_wigl_win(HWND hwnd);
#endif
wigl *get_current(void);
wigl *find_wigl_wid(long int wid);
void gl_setupwindow(void);
int is_big_endian(void);
void  byteswap(void *buffer,int len,int swapsize);
int Checkmapping(void);

#endif
