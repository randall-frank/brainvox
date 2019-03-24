/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/27/01 */

/*
 * void uv3D9()
 *
 * will take pitch,roll,yaw,p,q,r,talax,talay, and talaz
 * and convert them to the corresponding forward unit vectors
 * & derivatives
 */

#include "AIR.h"

#define PARAMETERS 9


static void tala_rotatef(double *e, double **de, double ***ee, const double *i, const double xoom1, const double xoom2, const double yoom1, const double yoom2, const double zoom1, const double zoom2, const double pixel_size1, const double pixel_size2, const unsigned int x_dim1, const unsigned int x_dim2, const unsigned int y_dim1, const unsigned int y_dim2, const unsigned int z_dim1, const unsigned int z_dim2, const AIR_Boolean zooming)

{

	double x[5],y[5],z[5];
	double dx[5][9];
	double dy[5][9];
	double dz[5][9];
	double ex[5][9][9];
	double ey[5][9][9];
	double ez[5][9][9];

	const double
		pitch=i[0],
		roll=i[1],
		yaw=i[2],
		p=i[3],
		q=i[4],
		r=i[5],
		talx=i[6],
		taly=i[7],
		talz=i[8];

	const double
		sinyaw=sin(yaw),
		cosyaw=cos(yaw),
		sinpitch=sin(pitch),
		cospitch=cos(pitch),
		sinroll=sin(roll),
		cosroll=cos(roll);

	if(!zooming){
		x[0]=(2*e[0]+1-x_dim1)*xoom1*(pixel_size1/pixel_size2);
		y[0]=(2*e[1]+1-y_dim1)*yoom1*(pixel_size1/pixel_size2);
		z[0]=(2*e[2]+1-z_dim1)*zoom1*(pixel_size1/pixel_size2);
	}
	else{
		x[0]=(2*e[0]+(1-x_dim1)*xoom1)*(pixel_size1/pixel_size2);
		y[0]=(2*e[1]+(1-y_dim1)*yoom1)*(pixel_size1/pixel_size2);
		z[0]=(2*e[2]+(1-z_dim1)*zoom1)*(pixel_size1/pixel_size2);
	}

	/*Apply scaling*/

	x[1]=x[0]*talx;
		dx[1][6]=x[0];

	y[1]=y[0]*taly;
		dy[1][7]=y[0];

	z[1]=z[0]*talz;
		dz[1][8]=z[0];

	/*Apply yaw*/

	x[2]=x[1]*cosyaw+y[1]*sinyaw;
		dx[2][2]=-x[1]*sinyaw+y[1]*cosyaw;
			ex[2][2][2]=-x[1]*cosyaw-y[1]*sinyaw;

		dx[2][6]=dx[1][6]*cosyaw /*+dy[1][6]*sinyaw*/;
			ex[2][6][2]=-dx[1][6]*sinyaw;
			/*ex[2][6][6]=ex[1][6][6]*cosyaw;*/

		dx[2][7]=/*dx[1][7]*cosyaw+*/ dy[1][7]*sinyaw;
			ex[2][7][2]=dy[1][7]*cosyaw;
			/*ex[2][7][6]=ey[1][7][6]*sinyaw;*/
			/*ex[2][7][7]=ey[1][7][7]*sinyaw;*/

		/*dx[2][8]=0;*/
			/*ex[2][8][]=0;*/



	y[2]=y[1]*cosyaw-x[1]*sinyaw;

		dy[2][2]=-x[1]*cosyaw-y[1]*sinyaw;
			ey[2][2][2]=x[1]*sinyaw-y[1]*cosyaw;

		dy[2][6]=/*dy[1][6]*cosyaw*/ -dx[1][6]*sinyaw;
			ey[2][6][2]=-dx[1][6]*cosyaw;
			/*ey[2][6][6]=-ex[1][6][6]*sinyaw;*/

		dy[2][7]=dy[1][7]*cosyaw /*-dx[1][7]*sinyaw*/;
			ey[2][7][2]=-dy[1][7]*sinyaw;
			/*ey[2][7][6]=ey[1][7][6]*cosyaw;*/
			/*ey[2][7][7]=ey[1][7][7]*cosyaw;*/
			/*ey[2][7][8]=ey[1][7][8]*cosyaw;*/

		/*dy[2][8]=0*/
			/*ey[2][8][]=0*/

	/*z[2]=z[1]*/

	/*Apply pitch*/

        /*x[3]=x[2];*/
	y[3]=y[2]*cospitch+z[1]*sinpitch;

		dy[3][0]=-y[2]*sinpitch+z[1]*cospitch;

			ey[3][0][0]=-y[2]*cospitch-z[1]*sinpitch;

		dy[3][1]=0; /*parameter 1 not yet applied*/
			ey[3][1][0]=0;
			ey[3][1][1]=0;

		dy[3][2]=dy[2][2]*cospitch /*+dz[1][2]*sinpitch*/;

			ey[3][2][0]=-dy[2][2]*sinpitch;
			ey[3][2][1]=0; /*parameter 1 not yet applied*/
			ey[3][2][2]=ey[2][2][2]*cospitch;

		dy[3][6]=dy[2][6]*cospitch /*+dz[1][6]*sinpitch*/;

			ey[3][6][0]=-dy[2][6]*sinpitch;
			ey[3][6][1]=0; /*parameter 1 not yet applied*/
			ey[3][6][2]=ey[2][6][2]*cospitch;
			ey[3][6][6]=0; /*ey[2][6][6]*cospitch*/

		dy[3][7]=dy[2][7]*cospitch /*+dz[1][7]*sinpitch*/;

			ey[3][7][0]=-dy[2][7]*sinpitch;
			ey[3][7][1]=0; /*paramter 1 not yet applied*/
			ey[3][7][2]=ey[2][7][2]*cospitch;
			ey[3][7][6]=0; /*ey[2][7][6]*cospitch*/
			ey[3][7][7]=0; /*ey[2][7][7]*cospitch*/

		dy[3][8]=/*dy[2][8]*cospitch+*/ dz[1][8]*sinpitch;
			ey[3][8][0]=dz[1][8]*cospitch;
			ey[3][8][1]=0; /*ez[1][8][1]*sinpitch*/
			ey[3][8][2]=0; /*ez[1][8][2]*sinpitch*/
			ey[3][8][6]=0; /*ez[1][8][6]*sinpitch*/
			ey[3][8][7]=0; /*ez[1][8][7]*sinpitch*/
			ey[3][8][8]=0; /*ez[1][8][8]*sinpitch*/
			

	z[3]=z[1]*cospitch-y[2]*sinpitch;

		dz[3][0]=-z[1]*sinpitch-y[2]*cospitch;

			ez[3][0][0]=-z[1]*cospitch+y[2]*sinpitch;

		dz[3][2]=/*dz[1][2]*cospitch*/ -dy[2][2]*sinpitch;

			ez[3][2][0]=-dy[2][2]*cospitch;
			ez[3][2][2]=-ey[2][2][2]*sinpitch;

		dz[3][6]=/*dz[1][6]*cospitch*/ -dy[2][6]*sinpitch;

			ez[3][6][0]=-dy[2][6]*cospitch;
			ez[3][6][2]=-ey[2][6][2]*sinpitch;
			/*ez[3][6][6]=-ey[2][6][6]*sinpitch;*/

		dz[3][7]=/*dz[1][7]*cospitch*/ -dy[2][7]*sinpitch;

			ez[3][7][0]=-dy[2][7]*cospitch;
			ez[3][7][2]=-ey[2][7][2]*sinpitch;
			/*ez[3][7][6]=-ey[2][7][6]*sinpitch;*/
			/*ez[3][7][7]=-ey[2][7][7]*sinpitch;*/

		dz[3][8]=dz[1][8]*cospitch /*-dy[2][8]*sinpitch*/;

			ez[3][8][0]=-dz[1][8]*sinpitch;
			/*ez[3][8][1]=-ez[1][8][1]*cospitch;*/
			/*ez[3][8][2]=-ez[1][8][2]*cospitch;*/
			/*ez[3][8][6]=-ez[1][8][6]*cospitch;*/
			/*ez[3][8][7]=-ez[1][8][7]*cospitch;*/
			/*ez[3][8][8]=-ez[1][8][8]*cospitch;*/
			

	/* Apply roll */

	x[4]=x[2]*cosroll+z[3]*sinroll;
		dx[4][0]=/*dx[2][0]*cosroll+*/ dz[3][0]*sinroll;
			ex[4][0][0]=ez[3][0][0]*sinroll;
		dx[4][1]=-x[2]*sinroll+z[3]*cosroll;
			ex[4][1][0]=/*-dx[2][0]*sinroll+*/ dz[3][0]*cosroll;
			ex[4][1][1]=-x[2]*cosroll-z[3]*sinroll;
		dx[4][2]=dx[2][2]*cosroll+dz[3][2]*sinroll;
			ex[4][2][0]=/*ex[2][2][0]*cosroll+*/ ez[3][2][0]*sinroll;
			ex[4][2][1]=-dx[2][2]*sinroll+dz[3][2]*cosroll;
			ex[4][2][2]=ex[2][2][2]*cosroll+ez[3][2][2]*sinroll;
		dx[4][6]=dx[2][6]*cosroll+dz[3][6]*sinroll;
			ex[4][6][0]=/*ex[2][6][0]*cosroll+*/ ez[3][6][0]*sinroll;
			ex[4][6][1]=-dx[2][6]*sinroll+dz[3][6]*cosroll;
			ex[4][6][2]=ex[2][6][2]*cosroll+ez[3][6][2]*sinroll;
			ex[4][6][6]=0; /*ex[2][6][6]*cosroll+ez[3][6][6]*sinroll;*/
		dx[4][7]=dx[2][7]*cosroll+dz[3][7]*sinroll;
			ex[4][7][0]=/*ex[2][7][0]*cosroll+*/ ez[3][7][0]*sinroll;
			ex[4][7][1]=-dx[2][7]*sinroll+dz[3][7]*cosroll;
			ex[4][7][2]=ex[2][7][2]*cosroll+ez[3][7][2]*sinroll;
			ex[4][7][6]=0; /*ex[2][7][6]*cosroll+ez[3][7][6]*sinroll*/
			ex[4][7][7]=0; /*ex[2][7][7]*cosroll+ez[3][7][7]*sinroll*/
		dx[4][8]=/*dx[2][8]*cosroll+*/ dz[3][8]*sinroll;
			ex[4][8][0]=ez[3][8][0]*sinroll;
			ex[4][8][1]=dz[3][8]*cosroll;
			ex[4][8][2]=0; /*ez[3][8][2]*sinroll*/
			ex[4][8][6]=0; /*ez[3][8][6]*sinroll*/
			ex[4][8][7]=0; /*ez[3][8][7]*sinroll*/
			ex[4][8][8]=0; /*ez[3][8][8]*sinroll*/

        /*y[4]=y[3];*/

	z[4]=z[3]*cosroll-x[2]*sinroll; 
		dz[4][0]=dz[3][0]*cosroll /*-dx[2][0]*sinroll*/;
			ez[4][0][0]=ez[3][0][0]*cosroll;
		dz[4][1]=-z[3]*sinroll-x[2]*cosroll;
			ez[4][1][0]=-dz[3][0]*sinroll /*-dx[2][0]*cosroll*/;
			ez[4][1][1]=-z[3]*cosroll+x[2]*sinroll;
		dz[4][2]=dz[3][2]*cosroll-dx[2][2]*sinroll;
			ez[4][2][0]=ez[3][2][0]*cosroll /*-ex[2][2][0]*sinroll*/;
			ez[4][2][1]=-dz[3][2]*sinroll-dx[2][2]*cosroll;
			ez[4][2][2]=ez[3][2][2]*cosroll-ex[2][2][2]*sinroll;
		dz[4][6]=dz[3][6]*cosroll-dx[2][6]*sinroll;
			ez[4][6][0]=ez[3][6][0]*cosroll /*-ex[2][6][0]*sinroll*/;
			ez[4][6][1]=-dz[3][6]*sinroll-dx[2][6]*cosroll;
			ez[4][6][2]=ez[3][6][2]*cosroll-ex[2][6][2]*sinroll;
			ez[4][6][6]=0; /*ez[3][6][6]*cosroll-ex[2][6][6]*sinroll;*/
		dz[4][7]=dz[3][7]*cosroll-dx[2][7]*sinroll;
			ez[4][7][0]=ez[3][7][0]*cosroll /*-ex[2][7][0]*sinroll*/;
			ez[4][7][1]=-dz[3][7]*sinroll-dx[2][7]*cosroll;
			ez[4][7][2]=ez[3][7][2]*cosroll-ex[2][7][2]*sinroll;
			ez[4][7][6]=0; /*ez[3][7][6]*cosroll-ex[2][7][6]*sinroll;*/
			ez[4][7][7]=0; /*ez[3][7][7]*cosroll-ex[2][7][7]*sinroll;*/
		dz[4][8]=dz[3][8]*cosroll /*-dx[2][8]*sinroll*/;
			ez[4][8][0]=ez[3][8][0]*cosroll;
			ez[4][8][1]=-dz[3][8]*sinroll;
			ez[4][8][2]=0; /*ez[3][8][2]*cosroll;*/
			ez[4][8][6]=0; /*ez[3][8][6]*cosroll;*/
			ez[4][8][7]=0; /*ez[3][8][7]*cosroll;*/
			ez[4][8][8]=0; /*ez[3][8][8]*cosroll;*/

	e[0]=(x[4]-p+(x_dim2-1)*xoom2)/(2*xoom2);

		de[0][0]=dx[4][0]/(2*xoom2);
			ee[0][0][0]=ex[4][0][0]/(2*xoom2);
		de[0][1]=dx[4][1]/(2*xoom2);
			ee[0][1][0]=ex[4][1][0]/(2*xoom2);
			ee[0][1][1]=ex[4][1][1]/(2*xoom2);
		de[0][2]=dx[4][2]/(2*xoom2);
			ee[0][2][0]=ex[4][2][0]/(2*xoom2);
			ee[0][2][1]=ex[4][2][1]/(2*xoom2);
			ee[0][2][2]=ex[4][2][2]/(2*xoom2);
		de[0][3]=-1.0/(2*xoom2);
			ee[0][3][0]=0;
			ee[0][3][1]=0;
			ee[0][3][2]=0;
			ee[0][3][3]=0;
		de[0][4]=0;
			ee[0][4][0]=0;
			ee[0][4][1]=0;
			ee[0][4][2]=0;
			ee[0][4][3]=0;
			ee[0][4][4]=0;
		de[0][5]=0;
			ee[0][5][0]=0;
			ee[0][5][1]=0;
			ee[0][5][2]=0;
			ee[0][5][3]=0;
			ee[0][5][4]=0;
			ee[0][5][5]=0;
		de[0][6]=dx[4][6]/(2*xoom2);
			ee[0][6][0]=ex[4][6][0]/(2*xoom2);
			ee[0][6][1]=ex[4][6][1]/(2*xoom2);
			ee[0][6][2]=ex[4][6][2]/(2*xoom2);
			ee[0][6][3]=0;
			ee[0][6][4]=0;
			ee[0][6][5]=0;
			ee[0][6][6]=ex[4][6][6]/(2*xoom2);
		de[0][7]=dx[4][7]/(2*xoom2);
			ee[0][7][0]=ex[4][7][0]/(2*xoom2);
			ee[0][7][1]=ex[4][7][1]/(2*xoom2);
			ee[0][7][2]=ex[4][7][2]/(2*xoom2);
			ee[0][7][3]=0;
			ee[0][7][4]=0;
			ee[0][7][5]=0;
			ee[0][7][6]=ex[4][7][6]/(2*xoom2);
			ee[0][7][7]=ex[4][7][7]/(2*xoom2);
		de[0][8]=dx[4][8]/(2*xoom2);
			ee[0][8][0]=ex[4][8][0]/(2*xoom2);
			ee[0][8][1]=ex[4][8][1]/(2*xoom2);
			ee[0][8][2]=ex[4][8][2]/(2*xoom2);
			ee[0][8][3]=0;
			ee[0][8][4]=0;
			ee[0][8][5]=0;
			ee[0][8][6]=ex[4][8][6]/(2*xoom2);
			ee[0][8][7]=ex[4][8][7]/(2*xoom2);
			ee[0][8][8]=ex[4][8][8]/(2*xoom2);


	e[1]=(y[3]-q+(y_dim2-1)*yoom2)/(2*yoom2);


		de[1][0]=dy[3][0]/(2*yoom2);
			ee[1][0][0]=ey[3][0][0]/(2*yoom2);
		de[1][1]=dy[3][1]/(2*yoom2);
			ee[1][1][0]=ey[3][1][0]/(2*yoom2);
			ee[1][1][1]=ey[3][1][1]/(2*yoom2);
		de[1][2]=dy[3][2]/(2*yoom2);
			ee[1][2][0]=ey[3][2][0]/(2*yoom2);
			ee[1][2][1]=ey[3][2][1]/(2*yoom2);
			ee[1][2][2]=ey[3][2][2]/(2*yoom2);
		de[1][3]=0;
			ee[1][3][0]=0;
			ee[1][3][1]=0;
			ee[1][3][2]=0;
			ee[1][3][3]=0;
		de[1][4]=-1.0/(2*yoom2);
			ee[1][4][0]=0;
			ee[1][4][1]=0;
			ee[1][4][2]=0;
			ee[1][4][3]=0;
			ee[1][4][4]=0;
		de[1][5]=0;
			ee[1][5][0]=0;
			ee[1][5][1]=0;
			ee[1][5][2]=0;
			ee[1][5][3]=0;
			ee[1][5][4]=0;
			ee[1][5][5]=0;
		de[1][6]=dy[3][6]/(2*yoom2);
			ee[1][6][0]=ey[3][6][0]/(2*yoom2);
			ee[1][6][1]=ey[3][6][1]/(2*yoom2);
			ee[1][6][2]=ey[3][6][2]/(2*yoom2);
			ee[1][6][3]=0;
			ee[1][6][4]=0;
			ee[1][6][5]=0;
			ee[1][6][6]=ey[3][6][6]/(2*yoom2);
		de[1][7]=dy[3][7]/(2*yoom2);
			ee[1][7][0]=ey[3][7][0]/(2*yoom2);
			ee[1][7][1]=ey[3][7][1]/(2*yoom2);
			ee[1][7][2]=ey[3][7][2]/(2*yoom2);
			ee[1][7][3]=0;
			ee[1][7][4]=0;
			ee[1][7][5]=0;
			ee[1][7][6]=ey[3][7][6]/(2*yoom2);
			ee[1][7][7]=ey[3][7][7]/(2*yoom2);
		de[1][8]=dy[3][8]/(2*yoom2);
			ee[1][8][0]=ey[3][8][0]/(2*yoom2);
			ee[1][8][1]=ey[3][8][1]/(2*yoom2);
			ee[1][8][2]=ey[3][8][2]/(2*yoom2);
			ee[1][8][3]=0;
			ee[1][8][4]=0;
			ee[1][8][5]=0;
			ee[1][8][6]=ey[3][8][6]/(2*yoom2);
			ee[1][8][7]=ey[3][8][7]/(2*yoom2);
			ee[1][8][8]=ey[3][8][8]/(2*yoom2);

	e[2]=(z[4]-r+(z_dim2-1)*zoom2)/(2*zoom2);

		de[2][0]=dz[4][0]/(2*zoom2);
			ee[2][0][0]=ez[4][0][0]/(2*zoom2);
		de[2][1]=dz[4][1]/(2*zoom2);
			ee[2][1][0]=ez[4][1][0]/(2*zoom2);
			ee[2][1][1]=ez[4][1][1]/(2*zoom2);
		de[2][2]=dz[4][2]/(2*zoom2);
			ee[2][2][0]=ez[4][2][0]/(2*zoom2);
			ee[2][2][1]=ez[4][2][1]/(2*zoom2);
			ee[2][2][2]=ez[4][2][2]/(2*zoom2);
		de[2][3]=0;
			ee[2][3][0]=0;
			ee[2][3][1]=0;
			ee[2][3][2]=0;
			ee[2][3][3]=0;
		de[2][4]=0;
			ee[2][4][0]=0;
			ee[2][4][1]=0;
			ee[2][4][2]=0;
			ee[2][4][3]=0;
			ee[2][4][4]=0;
		de[2][5]=-1.0/(2*zoom2);
			ee[2][5][0]=0;
			ee[2][5][1]=0;
			ee[2][5][2]=0;
			ee[2][5][3]=0;
			ee[2][5][4]=0;
			ee[2][5][5]=0;
		de[2][6]=dz[4][6]/(2*zoom2);
			ee[2][6][0]=ez[4][6][0]/(2*zoom2);
			ee[2][6][1]=ez[4][6][1]/(2*zoom2);
			ee[2][6][2]=ez[4][6][2]/(2*zoom2);
			ee[2][6][3]=0;
			ee[2][6][4]=0;
			ee[2][6][5]=0;
			ee[2][6][6]=ez[4][6][6]/(2*zoom2);
		de[2][7]=dz[4][7]/(2*zoom2);
			ee[2][7][0]=ez[4][7][0]/(2*zoom2);
			ee[2][7][1]=ez[4][7][1]/(2*zoom2);
			ee[2][7][2]=ez[4][7][2]/(2*zoom2);
			ee[2][7][3]=0;
			ee[2][7][4]=0;
			ee[2][7][5]=0;
			ee[2][7][6]=ez[4][7][6]/(2*zoom2);
			ee[2][7][7]=ez[4][7][7]/(2*zoom2);
		de[2][8]=dz[4][8]/(2*zoom2);
			ee[2][8][0]=ez[4][8][0]/(2*zoom2);
			ee[2][8][1]=ez[4][8][1]/(2*zoom2);
			ee[2][8][2]=ez[4][8][2]/(2*zoom2);
			ee[2][8][3]=0;
			ee[2][8][4]=0;
			ee[2][8][5]=0;
			ee[2][8][6]=ez[4][8][6]/(2*zoom2);
			ee[2][8][7]=ez[4][8][7]/(2*zoom2);
			ee[2][8][8]=ez[4][8][8]/(2*zoom2);

        return;
}

void AIR_uv3D9(const double *i, double **e, double ***de, double ****ee, const struct AIR_Key_info *reslice_info, const struct AIR_Key_info *standard_info, const AIR_Boolean zooming)

{
	const unsigned int
		x_dim1=standard_info->x_dim,
		y_dim1=standard_info->y_dim,
		z_dim1=standard_info->z_dim;
	
	const unsigned int
		x_dim2=reslice_info->x_dim,
		y_dim2=reslice_info->y_dim,
		z_dim2=reslice_info->z_dim;
	
	double
		xoom1,xoom2,
		yoom1,yoom2,
		zoom1,zoom2;
	double 	pixel_size1,pixel_size2;

	pixel_size1=standard_info->x_size;
	if(standard_info->y_size<pixel_size1) pixel_size1=standard_info->y_size;
	if(standard_info->z_size<pixel_size1) pixel_size1=standard_info->z_size;
	xoom1=standard_info->x_size/pixel_size1;
	yoom1=standard_info->y_size/pixel_size1;
	zoom1=standard_info->z_size/pixel_size1;

	pixel_size2=reslice_info->x_size;
	if(reslice_info->y_size<pixel_size2) pixel_size2=reslice_info->y_size;
	if(reslice_info->z_size<pixel_size2) pixel_size2=reslice_info->z_size;
	xoom2=reslice_info->x_size/pixel_size2;
	yoom2=reslice_info->y_size/pixel_size2;
	zoom2=reslice_info->z_size/pixel_size2;

        e[0][0]=1;
        e[0][1]=0;
        e[0][2]=0;

        e[1][0]=0;
        e[1][1]=1;
        e[1][2]=0;

        e[2][0]=0;
        e[2][1]=0;
        e[2][2]=1;

        e[3][0]=0;
        e[3][1]=0;
        e[3][2]=0;

	/*And the constant row*/
	e[0][3]=0;
	e[1][3]=0;
	e[2][3]=0;
	e[3][3]=1;

	tala_rotatef(e[0],de[0],ee[0],i,xoom1,xoom2,yoom1,yoom2,zoom1,zoom2,pixel_size1,pixel_size2,x_dim1,x_dim2,y_dim1,y_dim2,z_dim1,z_dim2,zooming);
	tala_rotatef(e[1],de[1],ee[1],i,xoom1,xoom2,yoom1,yoom2,zoom1,zoom2,pixel_size1,pixel_size2,x_dim1,x_dim2,y_dim1,y_dim2,z_dim1,z_dim2,zooming);
	tala_rotatef(e[2],de[2],ee[2],i,xoom1,xoom2,yoom1,yoom2,zoom1,zoom2,pixel_size1,pixel_size2,x_dim1,x_dim2,y_dim1,y_dim2,z_dim1,z_dim2,zooming);
	tala_rotatef(e[3],de[3],ee[3],i,xoom1,xoom2,yoom1,yoom2,zoom1,zoom2,pixel_size1,pixel_size2,x_dim1,x_dim2,y_dim1,y_dim2,z_dim1,z_dim2,zooming);


        e[2][0]-=e[3][0];
        e[1][0]-=e[3][0];
        e[0][0]-=e[3][0];

        e[2][1]-=e[3][1];
        e[1][1]-=e[3][1];
        e[0][1]-=e[3][1];

        e[2][2]-=e[3][2];
        e[1][2]-=e[3][2];
        e[0][2]-=e[3][2];
	{
		unsigned int t;
		
		for (t=0;t<9;t++){
	        	de[2][0][t]-=de[3][0][t];
	        	de[1][0][t]-=de[3][0][t];
	        	de[0][0][t]-=de[3][0][t];

	        	de[2][1][t]-=de[3][1][t];
	        	de[1][1][t]-=de[3][1][t];
	       		de[0][1][t]-=de[3][1][t];

	        	de[2][2][t]-=de[3][2][t];
	        	de[1][2][t]-=de[3][2][t];
	        	de[0][2][t]-=de[3][2][t];

			de[0][3][t]=0;
			de[1][3][t]=0;
			de[2][3][t]=0;
			de[3][3][t]=0;
			{
				unsigned int s;
				
				for(s=0;s<=t;s++){

		        		ee[2][0][t][s]-=ee[3][0][t][s];
		        		ee[1][0][t][s]-=ee[3][0][t][s];
		        		ee[0][0][t][s]-=ee[3][0][t][s];

		        		ee[2][1][t][s]-=ee[3][1][t][s];
		        		ee[1][1][t][s]-=ee[3][1][t][s];
		       			ee[0][1][t][s]-=ee[3][1][t][s];

		        		ee[2][2][t][s]-=ee[3][2][t][s];
		        		ee[1][2][t][s]-=ee[3][2][t][s];
		        		ee[0][2][t][s]-=ee[3][2][t][s];

					ee[0][3][t][s]=0;
					ee[1][3][t][s]=0;
					ee[2][3][t][s]=0;
					ee[3][3][t][s]=0;
				}
			}
		}
	}
	return;
}
