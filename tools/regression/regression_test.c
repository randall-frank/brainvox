
/*
 * Copyright (c), 1990-2000 Randall Frank and Thomas Grabowski
 *
 * $Id: regression_test.c 260 2001-05-18 22:23:14Z rjfrank $
 *                University of Iowa, Department of Neurology,
 *                Human Neuroanatomy and Neuroimaging Lab
 *                University of Iowa, Image Analysis Facility
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by Randall Frank, Thomas
 *      Grabowski and other unpaid contributors.
 * 5. The names of Randall Frank or Thomas Grabowski may not be used to
 *    endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY RANDALL FRANK AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL RANDALL FRANK OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "regression.h"

#define N_ALPHA 3
#define N_REG 6

main(int argc, char **argv)
{
	Regression *reg;
	long int vtype[N_ALPHA] = {-1,0,0};
	char mask[N_REG] = {0,1,1,1,1,0};
	int status;
	int x,j,i,pass;
	double y[N_REG];
	double v1[N_REG], v2[N_REG], v3[N_REG];
	double *obs[3];
	double coef[N_REG*N_ALPHA];
	double sst[N_REG],sse[N_REG],ssv[N_REG*N_ALPHA];

	reg = reg_create(N_ALPHA, vtype, N_REG, mask);
	if (!reg) exit(1);

for(pass=0;pass<3;pass++) {

	printf("Pass: %d\n",pass);
	srand(1000);
	obs[0] = v1;
	obs[1] = v2;
	obs[2] = v3;
	for (x=0; x<10; x++) {
		for (j=0; j<N_REG; j++) {
			y[j] = j + x*(j+0.4) + x*x*(j);
			if (j & 1) {
				double	r = ((double)rand()/(RAND_MAX*0.5))-1.0;
				y[j] += (r*0.1*(double)j);
			}
			v1[j] = 1.0;
			v2[j] = x;
			v3[j] = x*x;
			if (pass > 0) y[j] += 5.5 + 5.5*x + 5.5*x*x;
		}
 		if (pass < 2) {
			status = reg_add_obs(reg,y,1.0,obs);
		} else {
			status = reg_remove_obs(reg,y,1.0,obs);
		}
		if (status != 0) exit(1);
	}

	status = reg_calc_ss(reg, mask, 0, sst, sse, ssv);
	printf("SS:\n");
	for(j=0;j<N_REG;j++) {
		printf("%d-sst=%10.3f sse=%10.3f: %10.3f %10.3f %10.3f\n",
			j,sst[j],sse[j],
			ssv[j*N_ALPHA+0],ssv[j*N_ALPHA+1],ssv[j*N_ALPHA+2]);
	}

	status = reg_calc_ss(reg, mask, 1, sst, sse, ssv);
	printf("PSS:\n");
	for(j=0;j<N_REG;j++) {
		printf("%d-sst=%10.3f sse=%10.3f: %10.3f %10.3f %10.3f\n",
			j,sst[j],sse[j],
			ssv[j*N_ALPHA+0],ssv[j*N_ALPHA+1],ssv[j*N_ALPHA+2]);
	}

	status = reg_calc_coef(reg, mask, coef,NULL);
	printf("COEF:\n");
	for(j=0; j<N_REG; j++) {
		printf("%d-y = %f + x*%f + x*x*%f\n", j, 
			coef[j*N_ALPHA], coef[j*N_ALPHA+1],coef[j*N_ALPHA+2]);
	}
}

	reg_destroy(reg);

	exit(0);
}
	
