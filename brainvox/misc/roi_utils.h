
#define MAX_POINTS	10000

typedef struct {
        int x;
        int y;
} Point;

typedef struct {
        char filename[256];
        char name[80];
        int  num_points;
        Point thepnts[MAX_POINTS];
        int  visible;
        int  dirty;
        int  template;
} Trace;

void    init_roi_utils(void);
void    roi_set_image(long int dx,long int dy,unsigned char *in,
		unsigned char *in2);
void    roi_set_data_rep(double *rep);
long int read_roi(char *file,Trace *roi);
long int save_roi(char *file,Trace *roi);

long	clipped_point_img(long int x,long int y);
void	clip_point_img(long int *x,long int *y);
void 	init_roi(Trace *roi,char *name);
void   	roi_bounds(Trace *roi,Point *min,Point *max);
void   	roi_centroid(Trace *roi,Point *com);
double 	roi_area(Trace *roi);
double 	roi_perimeter(Trace *roi);
double 	roi_length(Trace *roi);
double 	roi_angle(Trace *roi);
void 	roi_axis(Trace *roi,double *maj,double *min);
void 	roi_axis_points(Trace *roi,double *maj,double *min,Point *ma,Point *mn);
void	fill_roi(Trace *roi,long int in,long int out);
void 	roi_data_rep(Trace *roi,double *min,double *max,double *mean,
	   double *stdev,double *pixels,double *sum,double *comx,double *comy);
void 	thin_roi(Trace *roi);
long int closed_roi(Trace *roi);
void 	edit_roi(Trace *roi,Point *edits);
void 	rotate_roi(Trace *roi,Point *edits);
void 	rotate_roi_ang(Trace *roi,double ang,long int x,long int y);
void 	move_roi(Trace *roi,long int dx, long int dy);
void    scale_roi(Trace *roi,double zoom);
void 	addpnt_roi(Trace *roi,long int x,long int y);
void    roi_2_strips(Trace *roi,int *strips,int *npts);
