/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/27/01 */

/*
 * will take pitch,roll,yaw,p,q, and r
 * and convert them to the corresponding forward unit vectors
 * & derivatives
 */

#include "AIR.h"

#define PARAMETERS 6

static void rigid_rotatef(double *e, double **de, double ***ee, const double *i, const double xoom1, const double xoom2, const double yoom1, const double yoom2, const double zoom1, const double zoom2, const double pixel_size1, const double pixel_size2, const unsigned int x_dim1, const unsigned int x_dim2, const unsigned int y_dim1, const unsigned int y_dim2, const unsigned int z_dim1, const unsigned int z_dim2, const AIR_Boolean zooming)

{

	double x[5],y[5],z[5];
	double dx[5][6];
	double dy[5][6];
	double dz[5][6];
	double ex[5][6][6];
	double ey[5][6][6];
	double ez[5][6][6];
	
	const double
		pitch=i[0],
		roll=i[1],
		yaw=i[2],
		p=i[3],
		q=i[4],
		r=i[5];
	
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


	/*This model doesn't apply scaling*/

	x[1]=x[0];
	y[1]=y[0];
	z[1]=z[0];

	/*Apply yaw*/

	x[2]=x[1]*cosyaw+y[1]*sinyaw;
	dx[2][2]=-x[1]*sinyaw+y[1]*cosyaw;
	ex[2][2][2]=-x[1]*cosyaw-y[1]*sinyaw;


	y[2]=y[1]*cosyaw-x[1]*sinyaw;

	dy[2][2]=-x[1]*cosyaw-y[1]*sinyaw;
	ey[2][2][2]=x[1]*sinyaw-y[1]*cosyaw;


	/*z[2]=z[1]*/

	/*Apply pitch*/

	/*x[3]=x[2];*/

	/*derivatives of y[3] with respect to parameter 1 which is not yet applied are*/
	/* computed as zero since these values are references subsequently*/
	/* This is not done for x[3] and z[3] because they are futher modified*/
	/* to generate x[4] and z[4] which have nonzero derivatives with respect to*/
	/* parameter 1 */

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

			

	z[3]=z[1]*cospitch-y[2]*sinpitch;

		dz[3][0]=-z[1]*sinpitch-y[2]*cospitch;

			ez[3][0][0]=-z[1]*cospitch+y[2]*sinpitch;

		dz[3][2]=/*dz[1][2]*cospitch*/ -dy[2][2]*sinpitch;

			ez[3][2][0]=-dy[2][2]*cospitch;
			ez[3][2][2]=-ey[2][2][2]*sinpitch;

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

        return;
}


void AIR_uv3D6(const double *i, double **e, double ***de, double ****ee, const struct AIR_Key_info *reslice_info, const struct AIR_Key_info *standard_info, const AIR_Boolean zooming)

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

	rigid_rotatef(e[0],de[0],ee[0],i,xoom1,xoom2,yoom1,yoom2,zoom1,zoom2,pixel_size1,pixel_size2,x_dim1,x_dim2,y_dim1,y_dim2,z_dim1,z_dim2,zooming);
	rigid_rotatef(e[1],de[1],ee[1],i,xoom1,xoom2,yoom1,yoom2,zoom1,zoom2,pixel_size1,pixel_size2,x_dim1,x_dim2,y_dim1,y_dim2,z_dim1,z_dim2,zooming);
	rigid_rotatef(e[2],de[2],ee[2],i,xoom1,xoom2,yoom1,yoom2,zoom1,zoom2,pixel_size1,pixel_size2,x_dim1,x_dim2,y_dim1,y_dim2,z_dim1,z_dim2,zooming);
	rigid_rotatef(e[3],de[3],ee[3],i,xoom1,xoom2,yoom1,yoom2,zoom1,zoom2,pixel_size1,pixel_size2,x_dim1,x_dim2,y_dim1,y_dim2,z_dim1,z_dim2,zooming);


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
		
		for (t=0;t<6;t++){
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
