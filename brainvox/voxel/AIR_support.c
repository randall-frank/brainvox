#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#undef DEBUG 
#define VPRINTF(n,v) printf("Vector %s: %f %f %f\n",n,v[0],v[1],v[2])
#define VPRINTD(n,v) printf("Vector %s: %lf %lf %lf\n",n,v[0],v[1],v[2])

#define DEG_TO_RAD (M_PI/180.0)
#define RAD_TO_DEG (180.0/M_PI)

typedef struct {
	long int	dx,dy,count;
	double		ip,is;
} Volume_params;

typedef struct {
	int 	valid;
	double	pitch,roll,yaw;
	double	tx,ty,tz;
	Volume_params	bv;
	Volume_params	fit;
	Volume_params	out;
	char	filename[256];
} AIR_data;

AIR_data AIR_params = {0, 0.0,0.0,0.0, 0.0,0.0,0.0 };

void air_reverse(double *p,double pitch,double roll,double yaw,
	double tx,double ty,double tz);
void air_forward(double *p,double pitch,double roll,double yaw,
	double tx,double ty,double tz);
void air_xform_(float *p,float *x,float *y,float *z);
void air_load_(char *filename,float *ipixel,float *islice,long int *dx,
	long int *dy,long int *count);
void out_to_fit(double *p);
static void AIR_norm(double *p);

static void AIR_norm(double *p)
{
	double d;

	d = (p[0]*p[0])+(p[1]*p[1])+(p[2]*p[2]);
	d = sqrt(d);
	p[0] = p[0]/d;
	p[1] = p[1]/d;
	p[2] = p[2]/d;

	return;
}

void air_xform_(float *o,float *x,float *y,float *z)
{
	double	p[3],zoom;
	double	xv[3],yv[3],zv[3];
	float	mx[3],my[3],mz[3];
	double	d[3],ratio,h_height;
	long int	i;

	if (AIR_params.valid == 0) return;

#ifdef DEBUG
	VPRINTF("O",o);
	VPRINTF("x",x);
	VPRINTF("y",y);
	VPRINTF("z",z);
#endif
	for(i=0;i<3;i++) z[i] = -z[i];
	for(i=0;i<3;i++) y[i] = -y[i];
/* start at BASE of OUT volume */
	p[0] = (AIR_params.out.dx/2);
	p[1] = (AIR_params.out.dy/2);
	p[2] = 0;
	out_to_fit(p);
	xv[0] = (AIR_params.out.dx/2)+1;
	xv[1] = (AIR_params.out.dy/2);
	xv[2] = 0;
	out_to_fit(xv);
	for(i=0;i<3;i++) xv[i] = xv[i] - p[i];
	yv[0] = (AIR_params.out.dx/2);
	yv[1] = (AIR_params.out.dy/2)+1;
	yv[2] = 0;
	out_to_fit(yv);
	for(i=0;i<3;i++) yv[i] = yv[i] - p[i];
	zv[0] = (AIR_params.out.dx/2);
	zv[1] = (AIR_params.out.dy/2);
	zv[2] = 0 + 1;
	out_to_fit(zv);
	for(i=0;i<3;i++) zv[i] = zv[i] - p[i];
	AIR_norm(xv);
	AIR_norm(yv);
	AIR_norm(zv);


/* I now have the OUT base in the MP_FIT volume */
/* transform the o,x,y,z to correspond to this new location */
/* calc the delta for the origin point */
	ratio = AIR_params.fit.ip/AIR_params.bv.ip;
	zoom = AIR_params.fit.is/AIR_params.fit.ip;
	h_height = (((AIR_params.fit.count-1)*zoom+1.0)+1.0)/2.0; 
/*	h_height = (((AIR_params.fit.count-1)*zoom+1.0))/2.0; */
	d[0] = p[0]*ratio;
	d[1] = p[1]*ratio;
	d[2] = (p[2]+h_height)*ratio;
#ifdef DEBUG
	VPRINTD("p",p);
	VPRINTD("xv",xv);
	VPRINTD("yv",yv);
	VPRINTD("zv",zv);
	VPRINTD("d",d);
#endif
/* move P by O bv units by the x,y,z vectors */
	for(i=0;i<3;i++) {
		o[i] = o[i] + d[0]*x[i];
		o[i] = o[i] + d[1]*y[i];
		o[i] = o[i] + d[2]*z[i];
	}
	
/* x,y,z vectors are fairly easy, just multiply */
	for(i=0;i<3;i++) {
		mx[i] = xv[0]*x[i] + xv[1]*y[i] + xv[2]*z[i];
		my[i] = yv[0]*x[i] + yv[1]*y[i] + yv[2]*z[i];
		mz[i] = zv[0]*x[i] + zv[1]*y[i] + zv[2]*z[i];
	}
/* and copy */
	for(i=0;i<3;i++) {
		x[i] = mx[i];
		y[i] = my[i];
		z[i] = mz[i];
	}
	for(i=0;i<3;i++) z[i] = -z[i];
	for(i=0;i<3;i++) y[i] = -y[i];
#ifdef DEBUG
	VPRINTF("O",o);
	VPRINTF("x",x);
	VPRINTF("y",y);
	VPRINTF("z",z);
#endif
	
	return;
}
void AIR_XFORM(float *o,float *x,float *y,float *z)
{
air_xform_(o,x,y,z);
}

void out_to_fit(double *p)
{
	double	zoom;

/* move to Half pixel origin based system */
	zoom = AIR_params.out.is/AIR_params.out.ip;
        p[0]=2*(p[0]-((AIR_params.out.dx+1.0)/2));
        p[1]=2*(p[1]-((AIR_params.out.dy+1.0)/2));
        p[2]=2*(p[2]-((((AIR_params.out.count-1)*zoom+1.0)+1.0)/2));

/*Rescale to pixel size of the fit file*/
	p[0] = p[0] * (AIR_params.out.ip/AIR_params.fit.ip);
	p[1] = p[1] * (AIR_params.out.ip/AIR_params.fit.ip);
	p[2] = p[2] * (AIR_params.out.ip/AIR_params.fit.ip);

/* get the point position in the FIT file */
	air_forward(p,AIR_params.pitch,AIR_params.roll,AIR_params.yaw,
		AIR_params.tx,AIR_params.ty,AIR_params.tz);
	
/* leave it in FIT.ip spacing units (origin at center) */
	p[0] = p[0]/2;
	p[1] = p[1]/2;
	p[2] = p[2]/2;
/* convert to indexed system of FIT file */
/*
	zoom = AIR_params.fit.is/AIR_params.fit.ip;
        p[0]=(p[0]/2+((AIR_params.fit.dx+1.0)/2));
        p[1]=(p[1]/2+((AIR_params.fit.dy+1.0)/2));
        p[2]=(p[2]/2+((((AIR_params.fit.count-1)*zoom+1.0)+1.0)/2));
        p[2]=(p[2]-1)/zoom+1; 
*/

	return;
}

void air_load_(char *filename,float *ipixel,float *islice,long int *dx,
	long int *dy,long int *count)
{
	FILE 	*fp;
	double	d,ip;

	AIR_params.valid = 0;
	if (filename == 0L) return;

	fp = fopen(filename,"r");
	if (fp != 0) {
		strcpy(AIR_params.filename,filename);
		ip = *ipixel;
/* values in radians and in Half-Pixel coords */
		fscanf(fp,"%lf",&d);
		AIR_params.pitch = d*DEG_TO_RAD;
		fscanf(fp,"%lf",&d);
		AIR_params.roll = d*DEG_TO_RAD;
		fscanf(fp,"%lf",&d);
		AIR_params.yaw = d*DEG_TO_RAD;
		fscanf(fp,"%lf",&d);
		AIR_params.tx = d;
		fscanf(fp,"%lf",&d);
		AIR_params.ty = d;
		fscanf(fp,"%lf",&d);
		AIR_params.tz = d;

/* get the fit volume params */
		fscanf(fp,"%ld %ld %ld %lf %lf",&(AIR_params.fit.dx),
			&(AIR_params.fit.dy),&(AIR_params.fit.count),
			&(AIR_params.fit.ip),&(AIR_params.fit.is));
/* get the out volume params */
		fscanf(fp,"%ld %ld %ld %lf %lf",&(AIR_params.out.dx),
			&(AIR_params.out.dy),&(AIR_params.out.count),
			&(AIR_params.out.ip),&(AIR_params.out.is));

		fclose(fp);

/* convert from MM to half pixels */
		AIR_params.tx *= 2.0/AIR_params.fit.ip;
		AIR_params.ty *= 2.0/AIR_params.fit.ip;
		AIR_params.tz *= 2.0/AIR_params.fit.ip;

		AIR_params.valid = 1;

		AIR_params.bv.ip = ip;
		AIR_params.bv.is = *islice;
		AIR_params.bv.dx = *dx;
		AIR_params.bv.dy = *dy;
		AIR_params.bv.count = *count;
	}
	return;
}
void AIR_LOAD(char *filename,float *ipixel,float *islice,long int *dx,
	long int *dy,long int *count)
{
air_load_(filename,ipixel,islice,dx,dy,count);
}

void air_reverse(double *p,double pitch,double roll,double yaw,
	double tx,double ty,double tz)
{
        double x;
        double x1;
        double y;
        double y1;
        double z;
        double z1;

        x=p[0];
        y=p[1];
        z=p[2];

        z=z+tz;
        y=y+ty;
        x=x+tx;
        x1=x*cos(-roll)+z*sin(-roll);
        z1=z*cos(-roll)-x*sin(-roll);
        x=x1;
        z=z1;
        y1=y*cos(-pitch)+z*sin(-pitch);
        z1=z*cos(-pitch)-y*sin(-pitch);
        y=y1;
        z=z1;
        x1=x*cos(-yaw)+y*sin(-yaw);
        y1=y*cos(-yaw)-x*sin(-yaw);
        x=x1;
        y=y1;

        p[0]=x;
        p[1]=y;
        p[2]=z;

	return;
}

void air_forward(double *p,double pitch,double roll,double yaw,
	double tx,double ty,double tz)
{
        double x;
        double x1;
        double y;
        double y1;
        double z;
        double z1;

        x=p[0];
        y=p[1];
        z=p[2];

        x1=x*cos(yaw)+y*sin(yaw);
        y1=y*cos(yaw)-x*sin(yaw);
        x=x1;
        y=y1;
        y1=y*cos(pitch)+z*sin(pitch);
        z1=z*cos(pitch)-y*sin(pitch);
        y=y1;
        z=z1;
        x1=x*cos(roll)+z*sin(roll);
        z1=z*cos(roll)-x*sin(roll);
        x=x1;
        z=z1;
        x=x-tx;
        y=y-ty;
        z=z-tz;


        p[0]=x;
        p[1]=y;
        p[2]=z;

	return;
}


void air_get_params_(long int *isvalid,float *vec,char *file)
{
	*isvalid = AIR_params.valid;
	if (AIR_params.valid) {
		strcpy(file,AIR_params.filename);
		vec[0] = AIR_params.pitch*RAD_TO_DEG;
		vec[1] = AIR_params.roll*RAD_TO_DEG;
		vec[2] = AIR_params.yaw*RAD_TO_DEG;
		vec[3] = AIR_params.tx*(AIR_params.fit.ip/2.0);
		vec[4] = AIR_params.ty*(AIR_params.fit.ip/2.0);
		vec[5] = AIR_params.tz*(AIR_params.fit.ip/2.0);
	}
	return;
}
void AIR_GET_PARAMS(long int *isvalid,float *vec,char *file)
{
air_get_params_(isvalid,vec,file);
}
