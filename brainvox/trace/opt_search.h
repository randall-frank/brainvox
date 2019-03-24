/* optimal surface detection routine */

/* options */
#define OPT_NONE 0
#define OPT_VERB 1
#define OPT_VVERB 2
#define OPT_TRUNC 4
#define OPT_NORM 8

/* return values: errors/warnings */
#define OPT_NOERROR 0
#define OPT_FATAL 1
#define OPT_WARN_TRUNC 2
#define OPT_WARN_SHIFT 4

/* maximum number of dimensions allowed */
#define MAX_DIM	6

long int opt_search(long int n,long int *size,float *cost,long int *depth,
        long int *range,long int *wrap,long int opt);

/*
	n - dimension of the problem (currently only 3 and 4 are implemented)
	size[n] - size of each of the n dimensions
	cost[size[0]*size[1]...*size[n]] - cost function to minimize
	depth[size[0]*size[1]...*size[n-1]] - output depth in n'th dimension
		on input, non-zero values are considered "holes"
	range[n] - defines the connectivity rules in the n-1 dimensions
		connectivity = 2r+1  (for holes = range[n])
	wrap[n] - is (1) if the dimension should be "wrapped" around
	opt - options (or'ed set of OPT_ defines)
*/	
