/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/4/01 */

/* void dqk15()
 *
 * compute:
 * I=integral of F over (A,B) with error estimate
 * J=integral of abs(F) over (A,B)
 *
 * Ported from the SLATEC Fortran routine which was written by Robert
 * Piessens and Elise de Doncker.
 */

#include "AIR.h"

void AIR_dqk15(double (*f)(const double), const double a, const double b, double *result, double *abserr, double *resabs, double *resasc)

{
	double	fv1[7];
	double	fv2[7];
	static double wg[4]={0.129484966168869693270611432679082,0.279705391489276667901467771423780,0.381830050505118944950369775488975,0.417959183673469387755102040816327};
	static double wgk[8]={0.022935322010529224963732008058970,0.063092092629978553290700663189204,0.104790010322250183839876322541518,0.140653259715525918745189590510238,0.169004726639267902826583426598550,0.190350578064785409913256402421014,0.204432940075298892414161999234649,0.209482141084727828012999174891714};
	static double xgk[8]={0.991455371120812639206854697526329,0.949107912342758524526189684047851,0.864864423359769072789712788640926,0.741531185599394439863864773280788,0.586087235467691130294144838258730,0.405845151377397166906606412076961,0.207784955007898467600689403773245,0.0};

	double centr=0.5*(a+b);
	double hlgth=0.5*(b-a);


	/*Compute the 15 point Kronrod approximation to the integral and estimate the error*/
	double fc=(*f)(centr);
	double resg=wg[3]*fc;
	double resk=wgk[7]*fc;
	*resabs=fabs(resk);
	{
		unsigned int j;
		
		for(j=0;j<3;j++){
			unsigned int jtw=j*2+1;
			double absc=hlgth*xgk[jtw];
			double fval1=(*f)(centr-absc);
			double fval2=(*f)(centr+absc);
			fv1[jtw]=fval1;
			fv2[jtw]=fval2;
			{
				double fsum=fval1+fval2;
				resg+=wg[j]*fsum;
				resk+=wgk[jtw]*fsum;
			}
			(*resabs)+=wgk[jtw]*(fabs(fval1)+fabs(fval2));
		}
	}
	{
		unsigned int j;
		
		for(j=0;j<=3;j++){
			unsigned int jtwm1=j*2;
			double absc=hlgth*xgk[jtwm1];
			double fval1=(*f)(centr-absc);
			double fval2=(*f)(centr+absc);
			fv1[jtwm1]=fval1;
			fv2[jtwm1]=fval2;
			{
				double fsum=fval1+fval2;
				resk+=wgk[jtwm1]*fsum;
			}
			(*resabs)+=wgk[jtwm1]*(fabs(fval1)+fabs(fval2));
		}
	}
	
	{
		double reskh=resk*0.5;
		(*resasc)=wgk[7]*fabs(fc-reskh);
		{
			unsigned int j;
			for(j=0;j<7;j++){
				(*resasc)+=wgk[j]*(fabs(fv1[j]-reskh)+fabs(fv2[j]-reskh));
			}
		}
	}
	*result=resk*hlgth;
	{
		double dhlgth=fabs(hlgth);
		(*resasc)*=dhlgth;
		(*resabs)*=dhlgth;
	}
	*abserr=fabs((resk-resg)*hlgth);
	if(*resasc!=0.0 && *abserr!=0.0){
		double temp=sqrt(200**abserr/(*resasc));
		temp=temp*temp*temp;
		if(1.0<temp) temp=1.0;
		*abserr=*resasc*temp;
	}
	{
		double epmach=AIR_d1mach(4);
		if(*resabs>AIR_d1mach(1)/(50*epmach)){
			double temp=epmach*50**resabs;
			if(temp>*abserr) *abserr=temp;
		}
	}
}
