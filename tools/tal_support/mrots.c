#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/*
 * $Id: mrots.c 1250 2005-09-16 15:51:42Z dforeman $
 */

double deg2rad(double ang);
double reverse(double *point,double pitch1,double roll1,double yaw1,
        double p1,double q1,double r1);


double deg2rad(double ang)
{
	return((ang/180.0)*M_PI);
}

/*antireslice a point*/
double reverse(double *point,double pitch1,double roll1,double yaw1,
        double p1,double q1,double r1)
{
        double x;
        double x1;
        double y;
        double y1;
        double z;
        double z1;

        x=point[0];
        y=point[1];
        z=point[2];

        z=z+r1;
        y=y+q1;
        x=x+p1;
        x1=x*cos(-roll1)+z*sin(-roll1);
        z1=z*cos(-roll1)-x*sin(-roll1);
        x=x1;
        z=z1;
        y1=y*cos(-pitch1)+z*sin(-pitch1);
        z1=z*cos(-pitch1)-y*sin(-pitch1);
        y=y1;
        z=z1;
        x1=x*cos(-yaw1)+y*sin(-yaw1);
        y1=y*cos(-yaw1)-x*sin(-yaw1);
        x=x1;
        y=y1;

        point[0]=x;
        point[1]=y;
        point[2]=z;

        return 1;
}

main(int argc,char **argv)
{
	double	pitch,roll,yaw,tx,ty,tz,p[3],o[3],d[4];

	if (argc != 10) {
		fprintf(stderr,"Usage: %s pitch roll yaw tx ty tz xp yp zp\n",
			argv[0]);
		exit(1);
	}
	pitch = atof(argv[1]);
	roll = atof(argv[2]);
	yaw = atof(argv[3]);
	tx = atof(argv[4]);
	ty = atof(argv[5]);
	tz = atof(argv[6]);
	p[0] = atof(argv[7]);
	p[1] = atof(argv[8]);
	p[2] = atof(argv[9]);
	o[0] = p[0];
	o[1] = p[1];
	o[2] = p[2];
	reverse(o,deg2rad(pitch),deg2rad(roll),deg2rad(yaw),tx,ty,tz);
	d[0] = o[0] - p[0];
	d[1] = o[1] - p[1];
	d[2] = o[2] - p[2];
	d[3] = sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);

	fprintf(stdout,"Rotations: %.3lf %.3lf %.3lf\n",
		deg2rad(pitch),deg2rad(roll),deg2rad(yaw));
	fprintf(stdout,"Translations: %.3lf %.3lf %.3lf\n",tx,ty,tz);
	fprintf(stdout,"Input point: %.3lf %.3lf %.3lf\n",p[0],p[1],p[2]);
	fprintf(stdout,"Output point: %.3lf %.3lf %.3lf\n",o[0],o[1],o[2]);
	fprintf(stdout,"Error: %.3lf %.3lf %.3lf: %.3lf\n",d[0],d[1],d[2],d[3]);

	exit(0);
}
