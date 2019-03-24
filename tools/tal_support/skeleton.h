/* skeleton and EDM computation routine prototypes */
/* from files: skeleton3d.c and skeleton2d.c */

/* 2D */
long int calc_skel_(unsigned char *in,long int dx,long int dy,long int *hist);
long int calc_edm_(unsigned char *in,long int dx,long int dy,long int feature,
        long int *hist);
long int calc_edm_s_(unsigned short *in,long int dx,long int dy,
        long int feature,long int scale,long int *hist);

/* 3D */
long int skeleton_3d_(unsigned char *data,long int thresh,
        long int dx,long int dy,long int dz,long int *hist);  /* hist[28] */

long int calc_edm_3d_(unsigned char *in,long int dx,long int dy,long int dz,
        long int feature,long int *hist);

long int calc_edm_3d_s_(unsigned short *in,long int dx,long int dy,long int dz,
        long int feature,long int scale,long int *hist,double is,double *ip);

