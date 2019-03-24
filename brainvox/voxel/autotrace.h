
typedef struct {
        unsigned char   *data;
        unsigned char   *data2;
        long int        d[3];
        long int        start[3],end[3];
} Dvol;

typedef struct {
        short 	x;
        short 	y;
        short 	z;
        short 	col;
} Point3D;

/* routines for implementing the volumetric dataset segmenting scheme */
/* which reserves the high order bit for a connectivity tag boolean */

/* generally visible routines */
/* prepare the volume by dividing it by 2 with optional histogram computation *//* fills in hist[0-255] with a histogram of values if hist != 0L */
void volume_shift(Dvol *set,long int *hist);

/* clear all the high oder bits in the volume */
void volume_clear(Dvol *set,long int *hist);

/* recompute the volume histogram */
void volume_hist(Dvol *set,long int *hist);

/* perform 3D flood fill from a set of preselected seeds */
void volume_autotrace(Dvol *set,long int low,long int high,
        Point3D *seeds,long int nseeds,long int *count);

/* Set the high order bits in a cubic region around a list of points */
void volume_tag(Dvol *set,Point3D *seeds,long int npts,long int t_size,
        long int i);

/* perform 2D isovalue contour following */
void autotrace_2d(unsigned char *image,long int dx,long int dy,long int val,
        long int sx,long int sy,long int *out,long int *num);

/* message printing function */
void    vl_puts(char *);

