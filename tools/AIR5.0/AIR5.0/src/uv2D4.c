/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/27/01 */

/*
 * void uv2D4()
 *
 * In-plane global rescaling model
 *
 * will take yaw,p,q,and gscale
 * and convert them to the corresponding forward unit vectors
 * & derivatives
 */

#include "AIR.h"

#define PARAMETERS 4

static void gscale2D_rotatef(double *e, double **de, double ***ee, const double *i, const double xoom1, const double xoom2, const double yoom1, const double yoom2, const double pixel_size1, const double pixel_size2, const unsigned int x_dim1, const unsigned int x_dim2, const unsigned int y_dim1, const unsigned int y_dim2, const AIR_Boolean zooming)

{

	double x[5],y[5];
	double dx[5][4];
	double dy[5][4];
	double ex[5][4][4];
	double ey[5][4][4];

	const double
		yaw=i[0],
		p=i[1],
		q=i[2],
		gscale=i[3];

	const double
		sinyaw=sin(yaw),
		cosyaw=cos(yaw);

	if(!zooming){
        	x[0]=(2*e[0]+1-x_dim1)*xoom1*(pixel_size1/pixel_size2);
       		y[0]=(2*e[1]+1-y_dim1)*yoom1*(pixel_size1/pixel_size2);
	}
	else{
		x[0]=(2*e[0]+(1-x_dim1)*xoom1)*(pixel_size1/pixel_size2);
		y[0]=(2*e[1]+(1-y_dim1)*yoom1)*(pixel_size1/pixel_size2);
	}

	/*Apply scaling*/

	x[1]=x[0]*gscale;
		dx[1][3]=x[0];
			ex[1][3][3]=0;

	y[1]=y[0]*gscale;
		dy[1][3]=y[0];
			ey[1][3][3]=0;
	/*Apply yaw*/

	x[4]=x[1]*cosyaw+y[1]*sinyaw;
		dx[4][0]=-x[1]*sinyaw+y[1]*cosyaw;
			ex[4][0][0]=-x[1]*cosyaw-y[1]*sinyaw;

		dx[4][3]=dx[1][3]*cosyaw+dy[1][3]*sinyaw;
			ex[4][3][0]=-dx[1][3]*sinyaw+dy[1][3]*cosyaw;
			ex[4][3][3]=0; /*ex[1][3][3]*cosyaw+ey[1][3][3]*sinyaw;*/


	y[3]=y[1]*cosyaw-x[1]*sinyaw;

		dy[3][0]=-x[1]*cosyaw-y[1]*sinyaw;
			ey[3][0][0]=x[1]*sinyaw-y[1]*cosyaw;

		dy[3][3]=dy[1][3]*cosyaw-dx[1][3]*sinyaw;
			ey[3][3][0]=-dy[1][3]*sinyaw-dx[1][3]*cosyaw;
			ey[3][3][3]=0; /*ey[1][3][3]*cosyaw-ex[1][3][3]*sinyaw;*/


	e[0]=(x[4]-p+(x_dim2-1)*xoom2)/(2*xoom2);

		de[0][0]=dx[4][0]/(2*xoom2);
			ee[0][0][0]=ex[4][0][0]/(2*xoom2);
		de[0][1]=-1.0/(2*xoom2);
			ee[0][1][0]=0;
			ee[0][1][1]=0;
		de[0][2]=0;
			ee[0][2][0]=0;
			ee[0][2][1]=0;
			ee[0][2][2]=0;
		de[0][3]=dx[4][3]/(2*xoom2);
			ee[0][3][0]=ex[4][3][0]/(2*xoom2);
			ee[0][3][1]=0;
			ee[0][3][2]=0;
			ee[0][3][3]=ex[4][3][3]/(2*xoom2);


	e[1]=(y[3]-q+(y_dim2-1)*yoom2)/(2*yoom2);


		de[1][0]=dy[3][0]/(2*yoom2);
			ee[1][0][0]=ey[3][0][0]/(2*yoom2);
		de[1][1]=0;
			ee[1][1][0]=0;
			ee[1][1][1]=0;
		de[1][2]=-1.0/(2*yoom2);
			ee[1][2][0]=0;
			ee[1][2][1]=0;
			ee[1][2][2]=0;
		de[1][3]=dy[3][3]/(2*yoom2);
			ee[1][3][0]=ey[3][3][0]/(2*yoom2);
			ee[1][3][1]=0;
			ee[1][3][2]=0;
			ee[1][3][3]=ey[3][3][3]/(2*yoom2);

        /*e[2]=e[2]*/

		de[2][0]=0;
			ee[2][0][0]=0;
		de[2][1]=0;
			ee[2][1][0]=0;
			ee[2][1][1]=0;
		de[2][2]=0;
			ee[2][2][0]=0;
			ee[2][2][1]=0;
			ee[2][2][2]=0;
		de[2][3]=0;
			ee[2][3][0]=0;
			ee[2][3][1]=0;
			ee[2][3][2]=0;
			ee[2][3][3]=0;

        return;
}



void AIR_uv2D4(const double *i, double **e, double ***de, double ****ee, const struct AIR_Key_info *reslice_info, const struct AIR_Key_info *standard_info, const AIR_Boolean zooming)

{
	const unsigned int
		x_dim1=standard_info->x_dim,
		y_dim1=standard_info->y_dim;

	const unsigned int
		x_dim2=reslice_info->x_dim,
		y_dim2=reslice_info->y_dim;

	double
		xoom1,xoom2,
		yoom1,yoom2;
	double 	pixel_size1,pixel_size2;

	pixel_size1=standard_info->x_size;
	if(standard_info->y_size<pixel_size1) pixel_size1=standard_info->y_size;
	if(standard_info->z_size<pixel_size1) pixel_size1=standard_info->z_size;
	xoom1=standard_info->x_size/pixel_size1;
	yoom1=standard_info->y_size/pixel_size1;

	pixel_size2=reslice_info->x_size;
	if(reslice_info->y_size<pixel_size2) pixel_size2=reslice_info->y_size;
	if(reslice_info->z_size<pixel_size2) pixel_size2=reslice_info->z_size;
	xoom2=reslice_info->x_size/pixel_size2;
	yoom2=reslice_info->y_size/pixel_size2;

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

	gscale2D_rotatef(e[0],de[0],ee[0],i,xoom1,xoom2,yoom1,yoom2,pixel_size1,pixel_size2,x_dim1,x_dim2,y_dim1,y_dim2,zooming);
	gscale2D_rotatef(e[1],de[1],ee[1],i,xoom1,xoom2,yoom1,yoom2,pixel_size1,pixel_size2,x_dim1,x_dim2,y_dim1,y_dim2,zooming);
	gscale2D_rotatef(e[2],de[2],ee[2],i,xoom1,xoom2,yoom1,yoom2,pixel_size1,pixel_size2,x_dim1,x_dim2,y_dim1,y_dim2,zooming);
	gscale2D_rotatef(e[3],de[3],ee[3],i,xoom1,xoom2,yoom1,yoom2,pixel_size1,pixel_size2,x_dim1,x_dim2,y_dim1,y_dim2,zooming);




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
		
		for (t=0;t<4;t++){
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
