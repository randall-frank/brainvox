#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
 
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double dotprod(double *v1,double *v2);
void normalize(double *v1);
void xprod(double *v1,double *v2,double *out);
double get_d(char *s,double v);
void rotaxis(double *axis,double angle,double *inpt,double *outpt);

void main()
{
	double	slew[2],tilt[2],xoff,uptrans[3];
	double	nslew,ntilt,temp,nxoff,temp2,xplaneshift;
	double	ipixel,p[3];
	double  fpxaxis[3]= {1,0,0};
	double  fpyaxis[3]= {0,0,1};
	double  fpnorm[3]= {0,-1,0};
	double	upxaxis[3],upyaxis[3],upnorm[3];
	double	transz[3],transy[3],transx[3];
	double	pp[3],pv[3];
	long 	i,done;
	char	s[256];
	
	printf("\nMP_FIT calibration parameter correction program.\n");
	printf("Version 1.1, 28 April 1994\n\n");
	
	ipixel = 1.0;

	done = 0;

while (!done) {

	printf("\n\n");
	printf("For MP_FIT planning parameters, calibration slew and tilt\n");
	printf("as well as Xoff and Xshift must be 0.0.\n");
	printf("X offset is positive to the patient's left.\n");
	printf("\nEnter the MP_FIT planning parameters:\n");
#ifdef OLD_MANNER
	slew[0] = get_d("Calibration Slew:",0.0);
	tilt[0] = get_d("Calibration Tilt:",0.0);
#else
	tilt[0] = 0;
	slew[0] = 0;
#endif	
	slew[1] = get_d("Total Slew:",0.0);
	tilt[1] = get_d("Total Tilt:",0.0);
	slew[1] = slew[1] - slew[0];
	tilt[1] = tilt[1] - tilt[0];

#ifdef OLD_MANNER
	xoff = get_d("X offset:",0.0);
#endif
	uptrans[1] = get_d("Table Y:",0.0);
	uptrans[2] = get_d("Table Z:",0.0);
	uptrans[0] = 0;

	printf("\nEnter the scan time calibration parameters:\n");
	nslew = get_d("Calibration Slew:",0.0);
	ntilt = get_d("Calibration Tilt:",0.0);
#ifndef OLD_MANNER
	xoff = get_d("X offset:",0.0);
#endif

/* compute the original calibrated axis */
	for(i=0;i<3;i++) {
		upnorm[i] = fpnorm[i];
		upyaxis[i] = fpyaxis[i];
		upxaxis[i]  = fpxaxis[i];

		transz[i] = fpnorm[i];
		transy[i] = fpyaxis[i];
		transx[i] = fpxaxis[i];
	}
/* additional slew */
	temp = -slew[1];
	rotaxis(upyaxis,temp,upnorm,upnorm);
	rotaxis(upyaxis,temp,upxaxis,upxaxis);
/* calibration slew */
	temp = slew[0];
	rotaxis(transy,temp,transz,transz);
	rotaxis(transy,temp,transx,transx);
/* additional tilt */
	temp = tilt[1];
	rotaxis(upxaxis,temp,upnorm,upnorm);
	rotaxis(upxaxis,temp,upyaxis,upyaxis);
/* calibration tilt */
	temp = -tilt[0];
	rotaxis(transx,temp,transz,transz);
	rotaxis(transx,temp,transy,transy);
/* compute pv and pp to correct for additional s/t about an extern point */
	for(i=0;i<3;i++) pv[i] = 48.9*fpnorm[i];
	temp = -slew[1];
	rotaxis(fpyaxis,temp,pv,pp);
	temp = tilt[1];
	rotaxis(fpxaxis,temp,pp,pp);

/* move from 0,0,0 to P */
	p[0] = 0; p[1] = 0; p[2] = 0;

/* move along Xoff */
	for(i=0;i<3;i++) {
		p[i] = p[i] - xoff*fpxaxis[i];
	}

/* move along the original calibration axis */
	for(i=0;i<3;i++) {
		p[i] = p[i] + (-uptrans[0])*transx[i];
	}
	for(i=0;i<3;i++) {
		p[i] = p[i] + (-uptrans[1])*transy[i];
	}
	for(i=0;i<3;i++) {
		p[i] = p[i] + (-uptrans[2])*transz[i];
	}

/* correct for additional S/T about a point PV from the base plane */
/*
	for(i=0;i<3;i++) {
		p[i] = p[i] - pv[i];
		p[i] = p[i] + pp[i];
	}
*/

/* now, given that the real calib slew and tilt have changed,
	compute the real transX vectors */
	for(i=0;i<3;i++) {
		transz[i] = fpnorm[i];
		transy[i] = fpyaxis[i];
		transx[i] = fpxaxis[i];
	}

	temp = nslew;
	rotaxis(transy,temp,transz,transz);
	rotaxis(transy,temp,transx,transx);

	temp = -ntilt;
	rotaxis(transx,temp,transz,transz);
	rotaxis(transx,temp,transy,transy);

/* find the xplaneshift such that dotprod(p+(shift),transx) == 0 */
	temp = 0;
	temp2 = 0;
	for(i=0;i<3;i++) {
		temp += (p[i]*transx[i]);
		temp2 += (upxaxis[i]*transx[i]);
	}
	if (temp2 == 0L) {
		printf("Error, xplaneshift cannot be computed!\n");
		printf("Planning plane xaxis and table X axis are the same!\n");
		exit(0);
	}
	xplaneshift = -(temp/temp2);

/* apply the xplaneshift */
	for(i=0;i<3;i++) {
		p[i] = p[i] + (xplaneshift)*upxaxis[i];
	}

#ifdef OLD_MANNER
/* find the Xoff such that dotprod(p+(xoffD),transx) == 0) */
/* remove old xoff */
	for(i=0;i<3;i++) {
		p[i] = p[i] + (-xoff)*fpxaxis[i];
	}

/* compute the new xoff */
	temp = 0;
	temp2 = 0;
	for(i=0;i<3;i++) {
		temp += (p[i]*transx[i]);
		temp2 += (fpxaxis[i]*transx[i]);
	}
	if (temp2 == 0L) {
		printf("Error, new Xoff cannot be computed!\n");
		exit(0);
	}
	nxoff = -(temp/temp2);

/* apply the new xoff */
	for(i=0;i<3;i++) {
		p[i] = p[i] + (nxoff)*fpxaxis[i];
	}

/* for reporting purposes */
	nxoff -= xoff;
#endif

/* find the projection of P onto the new vectors */

	printf("\nMP_FIT parameters corrected for calibration slew and tilt:\n");

	printf("\tCalibration Tilt: %.3lf\n",ntilt);
	printf("\tCalibration Slew: %.3lf\n",nslew);
	printf("\tTotal Tilt: %.3lf\n",ntilt+tilt[1]);
	printf("\tTotal Slew: %.3lf\n",nslew+slew[1]);
	printf("\tTable Z: %.3lf\n",-dotprod(p,transz));
	printf("\tTable Y: %.3lf\n",-dotprod(p,transy));
	if (fabs(dotprod(p,transx)) > 0.001) {
		printf("\tTable X: %.3lf\n",-dotprod(p,transx));
	}
#ifdef OLD_MANNER
	printf("\tXoff: %.3lf\n",-nxoff);
#else
	printf("\tXoff: %.3lf\n",xoff);
	printf("\tXshift: %.3lf\n",-xplaneshift);
#endif

printf("\n\nAnother run (Y/N)?");
fgets(s,256,stdin);
if ((s[0] == 'n') || (s[0] == 'N')) done = 1;
}


	exit(0);
}

double get_d(char *s,double v)
{
	double	out;
	char	tmp[256];

	printf("\t%s (%.3lf):",s,v);

	if (fgets(tmp,256,stdin) == 0L) {
		out = v;
		return(out);
	}

	if (sscanf(tmp,"%lf",&out) != 1) out = v;

	return(out);
}

double dotprod(double *v1,double *v2)
{
	return((v1[0]*v2[0])+(v1[1]*v2[1])+(v1[2]*v2[2]));
}

void normalize(double *v1)
{
	double d;
	
	d = sqrt(dotprod(v1,v1));
	if (d  > 0) {
		v1[0] /= d;
		v1[1] /= d;
		v1[2] /= d;
	}
	return;
}

void xprod(double *v1,double *v2,double *out)
{
        out[0] = (v1[1]*v2[2]) - (v1[2]*v2[1]);
        out[1] = (v1[2]*v2[0]) - (v1[0]*v2[2]);
        out[2] = (v1[0]*v2[1]) - (v1[1]*v2[0]);

	return;
}

void rotaxis(double *axis,double angle,double *inpt,double *outpt)
{
	double	mat[3][3],s,c,t,tmp[3];
        long	i;
/*
C       Build rotation matrix from angle and axis
C       (See Graphics Gems, Glassner Page 446)
*/
        s = sin(angle*(M_PI/180.0));
        c = cos(angle*(M_PI/180.0));
        t = 1.0 - cos(angle*(M_PI/180.0));

        mat[0][0] = t*axis[0]*axis[0] + c;
        mat[1][0] = t*axis[0]*axis[1] + s*axis[2];
        mat[2][0] = t*axis[0]*axis[2] - s*axis[1];
        mat[0][1] = t*axis[0]*axis[1] - s*axis[2];
        mat[1][1] = t*axis[1]*axis[1] + c;
        mat[2][1] = t*axis[1]*axis[2] + s*axis[0];
        mat[0][2] = t*axis[0]*axis[2] + s*axis[1];
        mat[1][2] = t*axis[1]*axis[2] - s*axis[0];
        mat[2][2] = t*axis[2]*axis[2] + c;
/*
C       Multiply the point by the matrix
*/
        for(i=0;i<3;i++) {
                tmp[i]=mat[0][i]*inpt[0]+mat[1][i]*inpt[1]+mat[2][i]*inpt[2];
        }
/*
C       Copy the result to the output array (this allows the input and
C       output arrays to be the same)
*/
	outpt[0] = tmp[0];
	outpt[1] = tmp[1];
	outpt[2] = tmp[2];

	return;
}
