/*
 *	A function (ui) taking one argument, which is a pointer to a
 * function to draw a 3-dimensional scene.  The function will be
 * passed an incremental rotation matrix as the user manipulates the
 * mouse, and should then draw the scene.
 *	Implemented by Gavin Bell, lots of ideas from Thant Tessman and
 * the August '88 issue of Siggraph's "Computer Graphics," pp. 121-129.
 *
 * See 'ui.h' for visible programmers interface.
 */
#include <stdio.h>
#include <math.h>
#include <gl.h>
#include <device.h>
#include "ui.h"

/* Externally visible state: */
/* Is the user currently using the interface? */
int ui_quiet = TRUE;
int ui_noisy = FALSE;

/* States global to only this file */
static int exitflag = FALSE;	/* Becomes 1 when user quits */
static int activeflag = TRUE;	/* User interface active */
static int rotateflag = FALSE;	/* When rotating with middle mouse */
static int zoomflag = FALSE;	/* Left AND Middle, zoom */
static int panflag = FALSE;	/* Left mouse, pan */

/*
 * Window dimensions, used to convert mouse-clicks into a more
 * convenient coordinate system.
 */
static int sizex, sizey, origx, origy;
/*
 *	And a record of where the last mouse and current mouse motion was
 */
static short omx, omy, nmx, nmy;

/*
 *	Function prototypes for the functions local to this file
 */
void ui_init(void), ui_update(void) ;
void ui_to_worldspace(short, short, float *, float *);
void ui_zoom(void), ui_pan(void), ui_redraw(void);
void ui_lmdown(void), ui_lmup(void);
void ui_mmdown(void), ui_mmup(void);

/*
 *	Used to remember what function was passed
 */
static void (*user_fn)(float *, float*);

/*
 *	And now....
 * The routines.
 */
void
ui(fn)
void (*fn)(float *, float *);
{
	static int initialized = 0;	/* Initialized yet? */

	user_fn = fn;
	if (!initialized)
	{
		ui_init();
		initialized = 1;
	}

	while (exitflag == 0)
	{
		/* All the action occurs in response to add_updates */
		event();
	}
}

void
ui_init()
{
	long gid;
	
	gid = winget();
	getsize(&sizex, &sizey);	/* Gotta know where center of */
	getorigin(&origx, &origy);	/* window is */

	qdevice(REDRAW);	/* Keep track of window size changes */
	qdevice(LEFTMOUSE);
	qdevice(MIDDLEMOUSE);

	add_event(gid, LEFTMOUSE, DOWN, ui_lmdown, NULL);
	add_event(gid, LEFTMOUSE, UP, ui_lmup, NULL);
	add_event(gid, MIDDLEMOUSE, DOWN, ui_mmdown, NULL);
	add_event(gid, MIDDLEMOUSE, UP, ui_mmup, NULL);
	add_event(gid, REDRAW, ANY, ui_redraw, NULL);

	add_update(&ui_noisy, ui_update, NULL);
}

/*
 *	This function is repeatedly called as the user manipulates the
 * interface.
 */
void
ui_update()
{
	nmx = getvaluator(MOUSEX);
	nmy = getvaluator(MOUSEY);

	if (panflag)
		ui_pan();
	else if (zoomflag)
		ui_zoom();
	else if (rotateflag)
	{
		float p1x, p1y, p2x, p2y ;
		float r[4], t[3] ;

		vzero(t);
		
		ui_to_worldspace(omx-origx, omy-origy, &p1x, &p1y);
		ui_to_worldspace(nmx-origx, nmy-origy, &p2x, &p2y);
		trackball(r, p1x, p1y, p2x, p2y);
		(*user_fn)(r, t);
	}
	omx = nmx; omy = nmy;
}

/*
 *	Map mouse click sx, sy to a more convenient (-1.0,1.0)
 * range, based on window size.
 */
void
ui_to_worldspace(sx, sy, wx, wy)
short sx, sy;
float *wx, *wy;
{
	(*wx) = (2.0 * sx) / (float) sizex - 1.0;
	(*wy) = (2.0 * sy) / (float) sizey - 1.0;
}

/*
 *	Zoom in/out; a translation of 1.0 is equal to a full sweep across
 * the window-- the user's function must scale accordingly.
 */
void
ui_zoom()
{
	float r[4], t[3];

	vzero(r); r[3] = 1.0;
	vzero(t);

	t[2] = (float)(nmx-omx)/(float)sizex +
		(float)(nmy-omy)/(float)sizey;

	(*user_fn)(r, t);
}

/*
 *	Translate in xy plane.  The window is assumed to be unit-sized in
 * the x and y directions; the values returned must be scaled
 * accordingly.
 */
void
ui_pan()
{
	float r[4], t[3];
	nmx = getvaluator(MOUSEX);
	nmy = getvaluator(MOUSEY);

	vzero(r); r[3] = 1.0;
	vset(t, (float)(nmx-omx)/(float)sizex, 
		(float)(nmy-omy)/(float)sizey, 0.0);

	(*user_fn)(r, t);
}

/*
 *	Called in case of REDRAW events to keep track of window size.
 */
void
ui_redraw()
{
	reshapeviewport();
	getsize(&sizex, &sizey);
	getorigin(&origx, &origy);
}

void
ui_exit()
{
	exitflag = 1;
}

void
ui_active(flag)
int flag;
{
	activeflag = flag ;
	if (!flag)
	{
		zoomflag = panflag = rotateflag = flag;
		ui_noisy = FALSE;
		ui_quiet = TRUE;
	}
}

void
ui_lmdown()
{
	if (activeflag)
	{
		omx = getvaluator(MOUSEX);
		omy = getvaluator(MOUSEY);

		if (rotateflag == TRUE)
		{
			zoomflag = TRUE;
			rotateflag = FALSE;
		}
		else
		{
			ui_quiet = FALSE;
			ui_noisy = !ui_quiet;
			panflag = TRUE;
		}
	}
}

void
ui_lmup()
{
	if (activeflag)
	{
		nmx = getvaluator(MOUSEX);
		nmy = getvaluator(MOUSEY);

		if (zoomflag == TRUE)
		{
			zoomflag = FALSE;
			rotateflag = TRUE;
		}
		else
		{
			ui_quiet = TRUE;
			ui_noisy = !ui_quiet;
			panflag = FALSE;
		}
	}
}

void
ui_mmdown()
{
	if (activeflag)
	{
		omx = getvaluator(MOUSEX);
		omy = getvaluator(MOUSEY);

		if (panflag == TRUE)
		{
			zoomflag = TRUE;
			panflag = FALSE;
		}
		else
		{
			ui_quiet = FALSE;
			ui_noisy = !ui_quiet;
			rotateflag = TRUE;
		}
	}
}

void
ui_mmup()
{
	if (activeflag)
	{
		nmx = getvaluator(MOUSEX);
		nmy = getvaluator(MOUSEY);

		if (zoomflag == TRUE)
		{
			zoomflag = FALSE;
			panflag = TRUE;
		}
		else
		{
			ui_quiet = TRUE;
			ui_noisy = !ui_quiet;
			rotateflag = FALSE;
		}
	}
}
