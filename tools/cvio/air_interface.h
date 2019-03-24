#ifndef __AIR_INTERFACE__

typedef struct {
	int verbosity;
	int model;
	int costfxn;
	float precision;
	int samplefactor;
	int samplefactor2;
	int sffactor;
	int partitions1;
	int partitions2;
	int interp;
	int threshold1;
	int threshold2;
} AlignParams;

#ifdef __cplusplus
extern "C" {
#endif

void align_param_init(AlignParams *p);
int align_images(char *fixed,char *in,char *out, int *dims, 
		AlignParams *params, double *fit,double *fitparams, int fitonly);

#ifdef __cplusplus
}
#endif

#endif /* __AIR_INTERFACE__ */
