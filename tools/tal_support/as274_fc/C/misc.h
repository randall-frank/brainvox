
void putdvec(const char *s, double *x, int l, int h);
void pr_utdm_v(double *x, int N, int width, int precision);

double *dvector(int l, int h);
int *ivector(int l, int h);
double **dmatrix(int rl, int rh, int cl, int ch);

extern struct {
    int ix, iy, iz;
} randc;

double ranwm(void);              /* wichman-hill RNG */
