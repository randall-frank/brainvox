#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PERCENT 0.05

int main(int argc,char **argv)
{
	double		*x,*y,*z;
	long int	n,na;
	char		str[256];
	double		r,dd,d;
	double		rs;
	long int	i,j,k,add;
	double		percent = PERCENT;

	if (argc > 1) percent = atof(argv[1]);

	na = 100;
	n = 0;
	x = (double *)malloc(sizeof(double)*na);
	y = (double *)malloc(sizeof(double)*na);
	z = (double *)malloc(sizeof(double)*na);
	if ((x == 0L) || (y == 0L) || (z == 0l)) exit(1);

	while (fgets(str,256,stdin) != 0) {
		if (n+5 > na) {
			na += 100;
			x = (double *)realloc(x,sizeof(double)*na);
			y = (double *)realloc(y,sizeof(double)*na);
			z = (double *)realloc(z,sizeof(double)*na);
			if ((x == 0L) || (y == 0L) || (z == 0l)) exit(1);
		}
		if (sscanf(str,"%lf %lf %lf",&(x[n]),&(y[n]),&(z[n]))==3) n++;
	}
/* look for what percent within r */
	r = 0.0;
	dd = 10.0;  /* increment */
	while(dd > 0.01) {  /* until very accurate */
/* look around each point */
		rs = (r+dd)*(r+dd);
		add = 1;
		for(i=0;i<n;i++) {
		k = 0;
		for(j=0;j<n;j++) {
			d = ((x[i] - x[j])*(x[i] - x[j]));
			d += ((y[i] - y[j])*(y[i] - y[j]));
			d += ((z[i] - z[j])*(z[i] - z[j]));
			if (d > rs) k++;
		}
		if (k < percent*n) {
			add = 0;
			break;
		}
		}
		if (add) {
			r = r + dd;
		} else {
			dd = dd/2;
		}
	}
	d = 0; dd = 0; rs = 0;
	for(i=0;i<n;i++) {
		d += x[i]; dd += y[i]; rs += z[i];
	}
	d /= (double)(n);
	dd /= (double)(n);
	rs /= (double)(n);

	printf("%ld\t%lf\t%lf\t%lf\t%lf\t%lf\n",n,d,dd,rs,percent,r);

	free(x);
	free(y);
	free(z);

	exit(0);
}
