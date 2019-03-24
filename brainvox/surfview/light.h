/* lighting and material stuff */

/* Use defineshading() before trying to use other routines */
void defineshading(void) ;

/*
 * setmaterial expects a value in the range of 0...NUM_MATERIALS, and
 * will make that material the current material.  Materials
 * NUM_MATERIALS...NUM_MATERIALS*2 are defined to be transparent
 * versions of the materials on machines that can do alpha-blending.
 * Materials are defined at the end of light.c
 */
#define NUM_MATERIALS 10
void setmaterial(int);
extern char *matnames[NUM_MATERIALS];

/*
 *	switch_light will toggle the given light (rand 0..NUM_LIGHTS) on
 * and off.
 */
#define NUM_LIGHTS 8
void switch_light(int) ;
extern char *lightnames[NUM_LIGHTS];

/*
 *	rebind_lights will rebind all the lights that are turned on,
 * causing them to change position if the viewing matrix has changed.
 */
void rebind_lights(void);

/*
 *	draw_lights will draw a representation of each of the lights that
 * is turned on; local lights are represented by small spheres
 * (icosahedron, really) and infinite lights are arrows pointing in
 * the direction they are shining.  It returns the number of polygons
 * required to draw the lights.
 */
int draw_lights(void);

/*
 *	setmodel will set the lighting model (range 0..NUM_LMODELS)
 */
#define NUM_LMODELS 2
void setmodel(int);
extern char *lmodelnames[NUM_LMODELS];

/*
 *	And resetmodel() should be used to turn lighting back on after it
 * has been turned off.
 */
void resetmodel(void);
