/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/8/01 */

/*
 * void uv2D6()
 *
 * will calculate first and second derivatives with
 * respect to the parameters for an in-plane affine
 * 6 parameter model.
 *
 * Note that a full 4 x 4 matrix is assumed.		
 */


#include "AIR.h"

#define PARAMETERS 6

void AIR_uv2D6(const double *i, double **ef, double ***def, double ****eef, const struct AIR_Key_info *reslice_info, const struct AIR_Key_info *standard_info, const AIR_Boolean zooming)

{
	const double *ii=i;

	/*Define forward transform*/

	ef[0][0]=*ii++;
	ef[1][0]=*ii++;
	ef[2][0]=0;
	ef[3][0]=*ii++;
	ef[0][1]=*ii++;
	ef[1][1]=*ii++;
	ef[2][1]=0;
	ef[3][1]=*ii++;
	ef[0][2]=0;
	ef[1][2]=0;
	ef[2][2]=1;
	ef[3][2]=0;
	ef[0][3]=0;
	ef[1][3]=0;
	ef[2][3]=0;
	ef[3][3]=1;

	/*All second partial derivatives are zero for forward transform*/
	{
		unsigned int m;
		double *defmno=**def;
		double *eefmnop=***eef;
		
		for (m=0;m<4;m++){
		
			unsigned int n;
			
			for (n=0;n<4;n++){
			
				unsigned int o;
				
				for (o=0; o<PARAMETERS; o++, defmno++){
				
					*defmno=0;
					{
						unsigned int p;
						
						for (p=0;p<=o;p++, eefmnop++){
						
							*eefmnop=0;
						}
					}
				}
			}
		}
	}
	
	/*First partial derivatives are zero except where the parameter*/
	/* is the factor being differentiated in which case it is one*/
	def[0][0][0]=1;
	def[1][0][1]=1;

	def[3][0][2]=1;
	def[0][1][3]=1;
	def[1][1][4]=1;

	def[3][1][5]=1;

}

	
