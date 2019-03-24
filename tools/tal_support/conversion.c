/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: conversion.c 1250 2005-09-16 15:51:42Z dforeman $
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
#include <math.h>
#include <string.h>
#include "proto.h"

/* routines to convert to/from tal space */
void tal_2_mm(double *tin,double *pmm,tal_conv *tal)
{
	double	tmp[3];
	long int i;
/* rescale each Tal MM into volume MM */
/* expand Z,Y,X independently in individual quadrants */
/* Z (top,bottom) */
	if (tin[2] >= 0.0) {
		tmp[2] = (tin[2]/TAL_TOP)*(tal->CA_top);
	} else {
		tmp[2] = (tin[2]/TAL_BOTTOM)*(tal->CA_bottom);
	}
/* X (right,left) */
	if (tin[0] >= 0.0) {
		tmp[0] = (tin[0]/TAL_RIGHT)*(tal->CA_right);
	} else {
		tmp[0] = (tin[0]/TAL_LEFT)*(tal->CA_left);
	}
/* Y (front,middle,back) */
	if (tin[1] >= 0.0) {
		tmp[1] = (tin[1]/TAL_FRONT)*(tal->CA_front);
	} else if (fabs(tin[1]) <= (TAL_MIDDLE)) {
		tmp[1] = (tin[1]/TAL_MIDDLE)*(tal->CA_CP);
	} else {
		tmp[1] = (tin[1]+TAL_MIDDLE)/TAL_BACK;
		tmp[1] = (tmp[1]*(tal->CP_back))-(tal->CA_CP);
	}
/* now place along the vectors */
	for(i=0;i<3;i++) {
		pmm[i] = (tmp[0]*(tal->xaxis[i])) +
				(tmp[1]*(tal->yaxis[i])) +
				(tmp[2]*(tal->zaxis[i]));
	}
/* add the centroid */
	for(i=0;i<3;i++) {
		pmm[i] = pmm[i] + (tal->points[i][PT_CA]);
	}
	return;
}
