/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/8/01 */

/*
 * will calculate first and second derivatives with
 * respect to the paramaters for a full 15
 * parameter model.
 *
 * Note that a full 4 x 4 matrix is assumed.		
 */

#include "AIR.h"

#define PARAMETERS 15

void AIR_uv3D15(const double *i, double **ef, double ***def, double ****eef, const struct AIR_Key_info *reslice_info, const struct AIR_Key_info *standard_info, const AIR_Boolean zooming)

{
	/*Define forward transform*/
	
	const double *ii=i;

	ef[0][0]=*ii++;
	ef[1][0]=*ii++;
	ef[2][0]=*ii++;
	ef[3][0]=*ii++;
	ef[0][1]=*ii++;
	ef[1][1]=*ii++;
	ef[2][1]=*ii++;
	ef[3][1]=*ii++;
	ef[0][2]=*ii++;
	ef[1][2]=*ii++;
	ef[2][2]=*ii++;
	ef[3][2]=*ii++;
	ef[0][3]=*ii++;
	ef[1][3]=*ii++;
	ef[2][3]=*ii++;
	ef[3][3]=1;		/*16th parameter is redundant*/


	/*All second partial derivatives are zero for forward transform*/
	{
		unsigned int m;
		double *defmno=**def;
		double *eefmnop=***eef;
		
		for (m=0;m<4;m++){
			
			unsigned int n;
			
			for (n=0;n<4;n++){
			
				unsigned int o;
				
				for (o=0;o<PARAMETERS;o++, defmno++){
				
					*defmno=0;
					{
						unsigned int p;
						
						for (p=0; p<=o; p++, eefmnop++){
						
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
	def[2][0][2]=1;
	def[3][0][3]=1;
	def[0][1][4]=1;
	def[1][1][5]=1;
	def[2][1][6]=1;
	def[3][1][7]=1;
	def[0][2][8]=1;
	def[1][2][9]=1;
	def[2][2][10]=1;
	def[3][2][11]=1;
	def[0][3][12]=1;
	def[1][3][13]=1;
	def[2][3][14]=1;

}

	
