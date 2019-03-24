
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef LN_PI
#define LN_PI 1.14472988584940016
#endif
#ifndef LNSQRT_2PI
#define LNSQRT_2PI 0.9189385332046727
#endif

double  betai(double a,double b,double x);
double  gammln(double xx);
double  gammp(double a,double x);
double  gammq(double a,double x);
void    chstwo(double *bins1,double *bins2,long int nbins,long int knstrn,
        double *df,double *chsq,double *prob);
void    avevar(double *data,long int n,double *ave,double *svar);
void    ftest(double *data1,long int n1,double *data2,long int n2,
                double *f,double *prob);
void tutest(double *data1,long int n1,double *data2,long int n2,
                double *t,double *prob);
void    moment(double *data,long int n,double *ave,double *adev,double *sdev,
                double *svar,double *skew,double *curt);
void    kstwo(double *data1,long int n1,double *data2,long int n2,
                double *d,double *prob);
void fit(double *x,double *y,long int n,double *a,double *b,double *chi);
double  erfc(double x);
double  erf(double x);

void kurt(double *data,long int num,double *v,double *s,double *t);
void skew(double *data,long int num,double *v,double *s,double *t);

float   median(float *x,long int n);
double	factorial(long int n);

double CDF_t(double x,double *df);
double PDF_t(double x,double *df);
double CDF_normal(double x,double *unused);
double PDF_normal(double x,double *unused);
double PDF_f(double x,double *df1_df2);
double CDF_f(double x,double *df1_df2);
double PDF_chi(double x,double *df);
double CDF_chi(double x,double *df);

