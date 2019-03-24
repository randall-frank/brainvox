/*
 *	flip.c
 * A complete face-lift for spin
 * New, much better user interface
 * Better lighting controls
 *
 * Now, all we need is a better object format...
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <gl.h>
#include "ui.h"
#include "light.h"
#include "flip.h"

/* Global variables */

static int nobjs;	/* How many objects we're spinning */
static flipobj **fobj;	/* Array of flipobj pointers */
static flipobj lobj;	/* Stores lights' transformations */
static float zmax;
static int totalpolys = 0;
int		SurfView = 1;
long int	backcolor = 0;

static enum
{
	MACH_ECLIPSE,
	MACH_CLOVER2,
	MACH_CLOVER1
} machine;

Matrix idmat =
{	/* Useful Matrix to have around... */
	{1.0, 0.0, 0.0, 0.0} ,
	{0.0, 1.0, 0.0, 0.0} ,
	{0.0, 0.0, 1.0, 0.0} ,
	{0.0, 0.0, 0.0, 1.0}
};

/* Prototypes for local functions */
int main(int, char **); 
void parse_args(int, char **);
void init_windows(char *);
void init_menus(void), do_menus(void);
void read_files(void), draw_scene(void);
void remember_view(float *, float *);
void draw_rate(int);
void draw_objects(void);
void update_objeulers(void);
void rand_rotation(float *);
void alpha_on(void), alpha_off(void);
void toggle_display(int), toggle_select(int);
void toggle_alpha(int), toggle_drawtype(int);
void toggle_swirl(int);
void toggle_lightsource(int);
void toggle_spinlights(void), toggle_displights(void);
void select_all(void);
void deselect_all(void);
void do_objmaterials(int);
void remake_objmenu(int), remake_menus(void);
void remake_lightmenu(void);
void remake_mmenu(int), remake_dtmenu(int);
void remake_lsmenu(void);
void select_lmodel(int), remake_lmmenu(int);
void set_view(void);
void make_lights(void);

int
main(int argc, char **argv)
{
	parse_args(argc, argv);	/* This reads in the files, too */
	init_windows(argv[0]);
	make_lights(); 

	add_event(ANY, ESCKEY, UP, ui_exit, 0);
	qdevice(ESCKEY);
	add_event(ANY, WINQUIT, ANY, ui_exit, 0);
	qdevice(WINQUIT);
	add_update(&ui_quiet, draw_scene, NULL);

	init_menus();
	
	ui(remember_view);	/* Time is spent in ui, interacting */
	exit(0);
}

/*
 *	Called by ui interface, passed a float[4] that is a rotation
 * specified in Euler paramaters and a float[3] that is xyz
 * translation.  (no rotation is {0.0, 0.0, 0.0, 1.0}, no translation
 * is {0.0, 0.0, 0.0})
 */
void remember_view(float *rot, float *trans)
{
	int i;

	if (lobj.select)	/* Moving lights, too */
	{
		vcopy(rot, lobj.espin);	/* vcopy copies 3 elements */
		lobj.espin[3] = rot[3];	/* So copy fourth here */

		vadd(trans, lobj.trans, lobj.trans);
	}

	for (i = 0; i < nobjs; i++)
	{
		if (fobj[i]->select)
		{
			vcopy(rot, fobj[i]->espin);
			fobj[i]->espin[3] = rot[3];

			vadd(trans, fobj[i]->trans, fobj[i]->trans);
		}
	}
	update_objeulers();	/* Spin those puppies */
	draw_objects();
}

/*
 *	This function is called whenever the user isn't interacting with
 * the program (when ui_quiet is TRUE).  It just keeps on applying the
 * last rotation and drawing the scene over and over.
 */

void draw_scene(void)
{
	int i;

	update_objeulers();
	draw_objects();
}

/*
 *	This routine updates an object's total rotation by applying its
 * spin rotation, and coming up with a new total rotation.  Also spins
 * lights.
 */
void update_objeulers(void)
{
	int i;
	for (i = 0; i < nobjs; i++)
	{
		add_eulers(fobj[i]->espin, fobj[i]->er, fobj[i]->er);
	}
	add_eulers(lobj.espin, lobj.er, lobj.er);
}

/*
 *	Draw the objects (and lights)
 */
void draw_objects(void)
{
	Matrix m;
	int i;
	int polysdrawn = 0;

	set_view();

	lmbind(LMODEL, 0);	/* Lighting off */
	if (machine == MACH_ECLIPSE)
		czclear(backcolor, 0xff800000);
	else
		czclear(backcolor, 0);
	resetmodel();	/* Lighting back on */

	/* Transform the lights */
	pushmatrix();
	translate(lobj.trans[0], lobj.trans[1], lobj.trans[2]);
	build_rotmatrix(m, lobj.er);
	multmatrix(m);
	rebind_lights();
	if (lobj.display)	/* And maybe display them */
		polysdrawn += draw_lights();
	popmatrix();

	for (i = 0; i < nobjs; i++)
	{
		if (fobj[i]->display && !fobj[i]->ablend)
		{
			pushmatrix();
			setmaterial(fobj[i]->material);

			/* Translate */
			translate(fobj[i]->trans[0],
				fobj[i]->trans[1], fobj[i]->trans[2]);

			/* And then rotate */
			build_rotmatrix(m, fobj[i]->er);
			multmatrix(m);

			if (fobj[i]->type == SUBSMOOTHLINES
				&& machine == MACH_CLOVER2)
			{
				if (!fobj[i]->ablend)
					blendfunction(BF_SA, BF_MSA);
				smoothline(TRUE);
				subpixel(TRUE);
			}

			drawflipobj(fobj[i]);

			if (machine == MACH_CLOVER2)
			{
				if (!fobj[i]->ablend)
				    blendfunction(BF_ONE, BF_ZERO);
				smoothline(FALSE);
				subpixel(FALSE);
			}

			polysdrawn += fobj[i]->npoints / 4;
			popmatrix();
		}
	}
/* Alpha blend, if we're on a machine that can do it */
	if (machine == MACH_CLOVER2)
	{
		alpha_on();
		for (i = 0; i < nobjs; i++)
		{
			if (fobj[i]->display && fobj[i]->ablend)
			{
				pushmatrix();
				setmaterial(NUM_MATERIALS+fobj[i]->material);

				translate(fobj[i]->trans[0], 
					fobj[i]->trans[1], fobj[i]->trans[2]);

				build_rotmatrix(m, fobj[i]->er);
				multmatrix(m);

				if (fobj[i]->type == SUBSMOOTHLINES)
				{
					smoothline(TRUE);
					subpixel(TRUE);
				}

				drawflipobj(fobj[i]);

				smoothline(FALSE);
				subpixel(FALSE);

				polysdrawn += fobj[i]->npoints / 4;
				popmatrix();
			}
		}
		alpha_off();
	}
	draw_rate(polysdrawn);
	
	swapbuffers();
}

#include <sys/types.h>
#include <sys/times.h>
#include <sys/param.h>

#define FRAMES 10	/* Update polygons/sec every FRAMES frames */
void draw_rate(int n)
{
	static int numdrawn = 0;
	static long lastt = 0;
	static int frames = 0;
	struct tms buf;
	long t;
	Matrix tm;
	static char s[30];

	numdrawn += n;
	if (lastt == 0)
	{
		sprintf(s, "%d polygons per frame", n);
		lastt = times(&buf);
	}
	if (++frames >= FRAMES)
	{
		t = times(&buf);
		sprintf(s, "%d polygons/second",
			(numdrawn * HZ) / (t - lastt));
		frames = 0;
		lastt = t;
		numdrawn = 0;
	}
	lmbind(LMODEL, 0);	/* Lighting off */
	mmode(MPROJECTION);
	getmatrix(tm);
#if 0	
	printf("%f %f %f %f\n",tm[0][0],tm[1][0],tm[2][0],tm[3][0]);
	printf("%f %f %f %f\n",tm[0][1],tm[1][1],tm[2][1],tm[3][1]);
	printf("%f %f %f %f\n",tm[0][2],tm[1][2],tm[2][2],tm[3][2]);
	printf("%f %f %f %f\n",tm[0][3],tm[1][3],tm[2][3],tm[3][3]);
#endif
	ortho(0.0, 100.0, 0.0, 100.0, -100.0, 100.0 );
	mmode(MVIEWING);
	cpack(0xffffffff);
	cmovi(3, 3, 0);
	charstr(s);
	mmode(MPROJECTION);
	loadmatrix(tm);
	mmode(MVIEWING);
	resetmodel();	/* Lighting back on */
}

/*
 * Initialize graphics
 */
void init_windows(char *title)
{
	char machinetype[20];

	/* Open with the executable's name (stripped of directory) */
	{
		char *t;
		winopen((t=strrchr(title, '/')) != NULL ? t+1 : title);
	}
	wintitle(title);

	reshapeviewport();

	/* Figure out what machine we're runnin on */
	gversion(machinetype);
	printf("Machine type: %s\n",machinetype);
	if (strncmp(machinetype, "GL4DPI", strlen("GL4DPI")) == 0)
		machine = MACH_ECLIPSE;
	else if (strncmp(machinetype, "GL4DGT", strlen("GL4DGT")) == 0)
		machine = MACH_CLOVER2;
	else if (strncmp(machinetype, "GL4DVGX", strlen("GL4DVGX")) == 0)
		machine = MACH_CLOVER2;
	else if (strncmp(machinetype, "GL4DLG", strlen("GL4DLG")) == 0)
		machine = MACH_CLOVER2;
	else if (strncmp(machinetype, "GL4DXG", strlen("GL4DXG")) == 0)
		machine = MACH_CLOVER2;
	else machine = MACH_ECLIPSE;

	RGBmode();
	doublebuffer();
	gconfig();
	zbuffer(TRUE);

	set_view();
}

void set_view(void) {

	int xorg, yorg, xdim, ydim;
	float near, far, aspect;

	getsize(&xdim,&ydim);
	aspect = (float) xdim/(float) ydim;

	near = 0.2;		/* Should be smarter about clipping planes */
	far = 1.1 + zmax + 3.0;
	mmode(MPROJECTION);
	loadmatrix(idmat);
	perspective(750, (float) aspect, near, far);
	mmode(MVIEWING);
	loadmatrix(idmat);
	translate(0.0, 0.0, -1.1);
}

void parse_args(int argc,char **argv)
{
	int whichobj=0;	/* Which one we're doing now */
	int i, c, err, still,r,g,b;
	float objzmax, objmaxpoint();
	extern int optind;
	extern char *optarg;

	err = FALSE; still=FALSE;

	while ((c = getopt(argc, argv, "shfb:")) != -1)
	{
		switch(c)
		{
			case 's':	/* Still (don't move) */
				still = TRUE;
				break;
			case 'f':	
				SurfView = 0;
				break;
			case 'b': 	/* background color */
				sscanf(optarg,"%ld %ld %ld",&r,&g,&b);
				backcolor = r | (g << 8) | (b <<16);
				break;
			case 'h':	/* Help */
			default:
				err=TRUE;
				break;
		}
	}

	/* First pass, figure out how many arguments */
	nobjs = 0;
	for (i = optind; i < argc; i++) {
		if (argv[i][0] != '-') ++nobjs;
	}

	if (err || (nobjs == 0)) {
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "%s [-shf] modelname [modelname]\n",argv[0]);
		fprintf(stderr, "\t-s Means stay still, objects won't get random rotation\n");
		fprintf(stderr, "\t-f Flip mode\n");
		fprintf(stderr, "\t-b \"red green blue\" RGB background color (0-255)\n");
		fprintf(stderr, "\t-h Help (this message)\n");
		exit(1);
	}

	/* allocate the objects */
	fobj = (flipobj **)malloc( sizeof(flipobj *) * nobjs);
	for (i = optind; i < argc; i++) {
		char temp[64];
		FILE *fp;
		int j, loaded;

/*
 *	First check to see if this object has already been loaded
 */
		loaded = (-1);
		for (j = optind; j < i; j++) {
			if (strcmp(argv[j], argv[i]) == 0) {
				loaded = j-optind;
			}
		}
		if (loaded == (-1)) {
/*
 * Ok, try to read in first from current directory,
 *	then from the demos directory
 */
			if ((fp = fopen(argv[i], "r")) != NULL) {
				fclose(fp);
				fobj[whichobj] = readflipobj(argv[i]);
			} else {
				strcpy(temp, MODELDIR);
				strcat(temp, argv[i]);
				fobj[whichobj] = readflipobj(temp);
			}
			if (fobj[whichobj] == NULL) {
				sprintf(temp, "%s, error reading %s:",
					argv[0], argv[i]);
				perror(temp);
				exit(1);
			}
			{
				char *t;
				t = strrchr(argv[i], '/');
				t =  (t == NULL) ? argv[i] : t+1 ;
				fobj[whichobj]->fname = t;
			}
		} else {	/* Share data with previous instance */
			fobj[whichobj] = (flipobj *)malloc(sizeof(flipobj));
			memcpy(fobj[whichobj], fobj[loaded], sizeof(flipobj));
		}
/* Set-up defaults */
		fobj[whichobj]->display = TRUE;
		fobj[whichobj]->type = POLYGONS;	/* draw polygons */
		fobj[whichobj]->select = TRUE;
		fobj[whichobj]->material = 2;
		fobj[whichobj]->ablend = FALSE;
		fobj[whichobj]->swirl = 0;
		vzero(fobj[whichobj]->trans);

		vzero(fobj[whichobj]->er);
		fobj[whichobj]->er[3] = 1.0;
		if (still) {
			vzero(fobj[whichobj]->espin);
			fobj[whichobj]->espin[3] = 1.0;
		} else {
			if (whichobj != 0) {
				fobj[whichobj]->espin[0] = 
					fobj[whichobj-1]->espin[0];
				fobj[whichobj]->espin[1] = 
					fobj[whichobj-1]->espin[1];
				fobj[whichobj]->espin[2] = 
					fobj[whichobj-1]->espin[2];
				fobj[whichobj]->espin[3] = 
					fobj[whichobj-1]->espin[3];
			} else {
				rand_rotation(fobj[whichobj]->espin);
			}
		}

		totalpolys += (fobj[whichobj]->npoints) / 4;
		if ((objzmax = objmaxpoint(fobj[whichobj])) > zmax)
			zmax = objzmax;
		++whichobj;
	}
	fobj[0]->select = TRUE;

/* Initialize lights */
	lobj.select = FALSE;	/* Don't rotate by default */
	lobj.display = TRUE;	/* But do display */
	vzero(lobj.er);	/* Start with no rotation */
	lobj.er[3] = 1.0;
	vzero(lobj.espin);	/* no spin */
	lobj.espin[3] = 1.0;
	vzero(lobj.trans);	/* and no translation */
}
void rand_rotation(float *e)
{
	static int init = 0;
	int i;
	float a[3];	/* Pick a random axis to rotate about */
	float phi;	/* And a speed of rotation */
	
	if (!init)
	{
		srand48(getpid());
		init = 1;
	}
	for (i = 0 ; i < 3 ; i++)
		a[i] = drand48() - 0.5;

/* Un-comment this out to get random rotation speeds
 *	phi = drand48() * .3;
 */
	phi = 0.1;

/* Now figure out Euler paramaters for given axis */
	axis_to_euler(a, phi, e);
}

static int lights[NUM_LIGHTS] ;

/*
 * Define the lighting model, lights, and initial material 
 */
void make_lights(void)
{
	int i;
	for (i = 0 ; i < NUM_LIGHTS ; i++) lights[i] = 0;	/* Off */

    	defineshading();	/* lmdef everything used */
	
    	setmodel(0);	/* Use infinite viewer */

	switch_light(0);	/* Infinite White on */
	lights[0] = 1;
}

static long flipmenu;	/* Top-level menu */
static long lightmenu;	/* Lights */
static long lmmenu;	/* Light models submenu */
static long lsmenu;	/* Light sources submenu */
static long *objmenus;	/* Menu for each object */
static long *mmenus;	/* Materials for each object */
static long *dtmenus;	/* Draw types for each object */

void init_menus(void)
{
	int i;
	char temp[64];

	lsmenu = newpup();
	remake_lsmenu();
	lmmenu = newpup();
	remake_lmmenu(0);
	lightmenu = newpup();
	remake_lightmenu();

	objmenus = (long *) malloc(sizeof(long) * nobjs);
	dtmenus = (long *) malloc(sizeof(long) * nobjs);
	mmenus = (long *) malloc(sizeof(long) * nobjs);
	for (i = 0; i < nobjs; i++)
	{
		mmenus[i] = newpup();
		remake_mmenu(i);
		dtmenus[i] = newpup();
		remake_dtmenu(i);
		objmenus[i] = newpup();
		remake_objmenu(i);
	}
	flipmenu = newpup();

	remake_menus();
	
	qdevice(RIGHTMOUSE);
	add_event(ANY, RIGHTMOUSE, DOWN, do_menus, NULL);
}
void do_menus(void)
{
	dopup(flipmenu);
}

/*
 *	Scheme for re-making minumum number of menus:
 * don't remake the whole structure, just what could have possibly
 * changed (the lowest-level object menus).  Assumes all lowest-level
 * object menus are already made.
 */
void remake_menus(void)
{
	int i, j;
	char temp[64];

	freepup(flipmenu);
	flipmenu = newpup();
	addtopup(flipmenu, "Surfview %t");
	addtopup(flipmenu, "Lights %m", lightmenu);
	for (i = 0; i < nobjs; i++)
	{
		sprintf(temp, "%s %%m", fobj[i]->fname);
		addtopup(flipmenu, temp, objmenus[i]); 
	}
	if (nobjs > 1) {
		addtopup(flipmenu, "Select all %f", select_all);
		addtopup(flipmenu, "Deselect all %f", deselect_all);
	}
	addtopup(flipmenu, "Exit %f", ui_exit);
}

void remake_lightmenu(void)
{
	freepup(lightmenu);
	lightmenu = newpup();
	addtopup(lightmenu, "Sources %m", lsmenu);
	addtopup(lightmenu, "Lighting Models %m", lmmenu);
	if (lobj.select == TRUE)
		addtopup(lightmenu, "Deselect lights %f",toggle_spinlights);
	else
		addtopup(lightmenu, "Select lights %f",toggle_spinlights);
	if (lobj.display == TRUE)
		addtopup(lightmenu, "Hide local lights %f", toggle_displights);
	else
		addtopup(lightmenu, "Show local lights %f", toggle_displights);
}

void remake_mmenu(int n)
{
	int i, j, start;
	char temp[64];
	start = n*NUM_MATERIALS;

	freepup(mmenus[n]);
	mmenus[n] = newpup();
	addtopup(mmenus[n], "Material Properties %t %F", do_objmaterials);
	for (j = 0 ; j < NUM_MATERIALS ; j++)
	{
		if (fobj[n]->material == j)
			sprintf(temp, "->%s %%x%d", matnames[j], start+j);
		else
			sprintf(temp, "%s %%x%d", matnames[j], start+j);
		addtopup(mmenus[n], temp);
	}
}

void remake_dtmenu(int n)
{
	char temp[64];
	int i;
	
	i = n * (int)NUM_DrawTypes;

	freepup(dtmenus[n]);
	dtmenus[n] = newpup();
	addtopup(dtmenus[n], "Drawing Modes %t %F", toggle_drawtype);
	if (fobj[n]->type == POLYGONS)
		sprintf(temp, "->Polygons %%x%d", i+(int)POLYGONS);
	else
		sprintf(temp, "Polygons %%x%d", i+(int)POLYGONS);
	addtopup(dtmenus[n], temp);
	if (fobj[n]->type == LINES)
		sprintf(temp, "->Lines %%x%d", i+(int)LINES);
	else
		sprintf(temp, "Lines %%x%d", i+(int)LINES);
	addtopup(dtmenus[n], temp);

	if (machine == MACH_CLOVER2)
	{
		if (fobj[n]->type == SUBSMOOTHLINES)
			sprintf(temp, "->Subpixel, antialised lines %%x%d", 
				i+(int)SUBSMOOTHLINES);
		else
			sprintf(temp, "Subpixel, antialised lines %%x%d", 
				i+(int)SUBSMOOTHLINES);
		addtopup(dtmenus[n], temp);
	}
}

void remake_objmenu(int n)
{
	char temp[64];

	freepup(objmenus[n]);
	objmenus[n] = newpup();

	if (fobj[n]->display == TRUE)
		sprintf(temp, "Hide %%f %%x%d", n);
	else
		sprintf(temp, "Show %%f %%x%d", n);
	addtopup(objmenus[n], temp, toggle_display);

	if (fobj[n]->select == TRUE)
		sprintf(temp, "Deselect %%f %%x%d", n);
	else
		sprintf(temp, "Select %%f %%x%d", n);
	addtopup(objmenus[n], temp, toggle_select);

	addtopup(objmenus[n], "Object Materials %m", mmenus[n]);

	if (machine != MACH_CLOVER1)
	{
		if (fobj[n]->ablend == TRUE)
			sprintf(temp, "Make Opaque %%f %%x%d", n);
		else
			sprintf(temp, "Make Transparent %%f %%x%d", n);
		addtopup(objmenus[n], temp, toggle_alpha);
	}

	addtopup(objmenus[n], "Display Object as... %m", dtmenus[n]);

}

void do_objmaterials(int n)
{
	int obj, m;

	obj = n/NUM_MATERIALS;
	m = n % NUM_MATERIALS;

	fobj[obj]->material = m;
	remake_mmenu(obj);
	remake_objmenu(obj);
	remake_menus();
}

void toggle_spinlights(void)
{
	lobj.select = !lobj.select;

	remake_lightmenu();
	remake_menus();
}
void toggle_display(int n)
{
	fobj[n]->display = !fobj[n]->display;

	remake_objmenu(n);
	remake_menus();
}
void toggle_drawtype(int n)
{
	int obj;
	enum DrawType dt;

	obj = n / (int)NUM_DrawTypes;
	dt = (enum DrawType)(n % (int)NUM_DrawTypes);

	fobj[obj]->type = dt;

	remake_dtmenu(obj);
	remake_objmenu(obj);
	remake_menus();
}
void toggle_displights(void)
{
	lobj.display = !lobj.display;

	remake_lightmenu();
	remake_menus();
}
void toggle_alpha(int n)
{
	fobj[n]->ablend = !fobj[n]->ablend;

	remake_objmenu(n);
	remake_menus();
}
void toggle_swirl(int n)
{
	fobj[n]->swirl = !fobj[n]->swirl;

	remake_objmenu(n);
	remake_menus();
}

void select_all(void)
{
	int i;
	for (i = 0; i < nobjs; i++)
	{
		if (fobj[i]->select == FALSE)
		{
			fobj[i]->select = TRUE;
			remake_objmenu(i) ;
		}
	}
	remake_menus() ;
}

void deselect_all(void)
{
	int i;
	for (i = 0; i < nobjs; i++)
	{
		if (fobj[i]->select == TRUE)
		{
			fobj[i]->select = FALSE;
			remake_objmenu(i) ;
		}
	}
	remake_menus() ;
}

void toggle_select(int n)
{
	int t ;

	fobj[n]->select = !fobj[n]->select;

	remake_objmenu(n);
	remake_menus();
}

void toggle_lightsource(int ls)
{
	switch_light(ls);
	lights[ls] = !lights[ls];

	remake_lsmenu();
	remake_lightmenu();
	remake_menus();
}

void remake_lsmenu(void)
{
	int i;
	char temp[64];

	freepup(lsmenu);
	lsmenu = newpup();
	addtopup(lsmenu, "Light Sources %t %F", toggle_lightsource);
	for (i = 0 ; i < NUM_LIGHTS; i++)
	{
		if (lights[i])
			sprintf(temp, "->%s %%x%d", lightnames[i], i);
		else
			sprintf(temp, "%s %%x%d", lightnames[i], i);
		addtopup(lsmenu, temp);
	}
}

void select_lmodel(int n)
{
	setmodel(n);
	remake_lmmenu(n);
	remake_lightmenu();
	remake_menus();
}

void remake_lmmenu(int n)
{
	int i;
	char temp[64];
	
	freepup(lmmenu);
	lmmenu = newpup();
	addtopup(lmmenu, "Light Models %t %F", select_lmodel);
	for (i = 0 ; i < NUM_LMODELS; i++)
	{
		if (i == n)
			sprintf(temp, "->%s %%x%d", lmodelnames[i], i);
		else
			sprintf(temp, "%s %%x%d", lmodelnames[i], i);
		addtopup(lmmenu, temp, select_lmodel);
	}
}

void alpha_on(void)
{
    blendfunction(BF_SA, BF_MSA);
    zwritemask(0x0);
}

void alpha_off(void)
{
    blendfunction(BF_ONE, BF_ZERO);
    zwritemask(0xffffff);
}
