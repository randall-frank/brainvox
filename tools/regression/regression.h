#ifndef __REGRESSION_H__
#define __REGRESSION_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Public (external) interface for the regression library */

typedef struct {
	double 	*r, *dd, *theta;
	double 	ssumy, sumy, sser;
} Individual;

typedef struct {
	Individual	*reg;
	long int 	num_vars;
	long int 	*num_classes;
	long int 	num_values;
	char 		*mask_value;
	long int 	np, nrbar, nobs;
	long int 	*XofV;
	double		*xtmp;
	double		*ss,*tol,*work;
	long int 	*vorder, *norder, *lindep;
} Regression;

/* prototypes */
Regression *reg_create(long int nvars, long int *nclasses, long int nvalues, 
	char *valuemask);
void reg_destroy(Regression *r);
int reg_add_obs(Regression *r, double *y, double w, double **obs);
int reg_remove_obs(Regression *r, double *y, double w, double **obs);
int reg_calc_ss(Regression *r, char *mask, long int partial, double *ss_total, 
	double *ss_error, double *ss_var);
int reg_calc_coef(Regression *r, char *mask, double *coef,double *sterr);

#ifdef __cplusplus
}
#endif

#endif
