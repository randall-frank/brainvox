/*
 *	ui.h
 *  Written by Gavin Bell for Silicon Graphics, November 1988.
 *
 * This interface uses the LEFTMOUSE and MIDDLEMOUSE buttons to
 * implement 3D rotations and translations.  X-Y translation (pan) is
 * done using the left mousebutton.  Z translation (zoom) is done
 * using the left AND middle mouse buttons, held down together.  XYZ
 * rotations are done using the middle mouse button; the center of the
 * window becomes the center of a virtual trackball.
 *
 * The interface uses the input-queue event handler program; see
 * 'event.h' for a description of those routines.  Also see
 * 'example.c' for a complete but simple example of how to use all of
 * these routines.
 */

#include "event.h"
#include "trackball.h"
#include "vect.h"

/*
 * The main routine takes one argument; the function to be called when
 * the user interacts with the interface.  This function will be
 * passed two arguments; a rotation vector, and a translation vector
 * (both arrays of 3 floats).  See 'example.c' for an example
 * application using this interface.
 *
 * Since the user interface does several gl calls, a window must be
 * open and active when ui() is first called (this user interface will
 * respond only to events which happen in the window that is active
 * when ui() is first called).
 *
 * Note that this function does not terminate until ui_exit is called.
 */
void ui(void (*fn)(float *, float *));

/*
 *	Call ui_active(FALSE) to make the user interface become inactive.
 * Calling ui_active(TRUE) will make it active again.  This allows the
 * program to free-up the left and middle buttons for other tasks.
 */
void ui_active(int);

/*
 *	Call ui_exit when it is time for the interface to go away permanently.
 */
void ui_exit(void);

/*
 *	These two flags are always opposites, and keep track of if the
 * interface is being used or not.
 */
extern int ui_noisy, ui_quiet;
