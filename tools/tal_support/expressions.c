/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: expressions.c 1250 2005-09-16 15:51:42Z dforeman $
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

#include "macroprotos.h"

long int eval_expression(char *exp);
long int User_Identifiers(char *name, double *value);

typedef struct {
	char *name;
	double value;
	} Identifiers;

#define X_ID 0
#define Y_ID 1
#define Z_ID 2
#define A_ID 3
#define B_ID 4
#define C_ID 5
#define D_ID 6

Identifiers	ident_list[] = { 	{"x",0.0},
					{"y",0.0},
					{"Z",0.0},
					{"a",0.0},
					{"b",0.0},
					{"c",0.0},
					{"d",0.0},
					{"r",0.0},
					{0L,0.0}
				};

long int User_Identifiers(char *name, double *value)
{
	long int i;

	i = 0;
	while (ident_list[i].name != 0L) {
		if (strcmp(ident_list[i].name,name) == 0L) {
			*value = ident_list[i].value;
			return(1);
		}
		i++;
	}
	return(0);
}

#define M_PTS 10000

long int eval_expression(char *exp)
{
	char		out[256];
	unsigned char	*tmp;
	long int	x,y,j,i;
	double		value,v,ssr,ssi;
	double		nr,ni;
	int             list[M_PTS],strips[M_PTS];
	int             ptr,num;
	int		st,en;
	Trace		*roi;

	tmp = malloc(imagex*imagey);
	if (tmp == 0L) return;
	for(i=0;i<imagex*imagey;i++) tmp[i] = 0;

/* prep for the polyfill */
        num = 0;
	if (cur_trace < 0) goto skipfill;
	roi = thetraces[cur_trace];
	if (roi->num_points < 3) goto skipfill;
        for(i=0;i<roi->num_points;i++) {
                list[num] = roi->thepnts[i].x;
                list[num+1] = roi->thepnts[i].y;
                num = num + 2;
        }

/* perform the polygon fill on the list */
        ptr = M_PTS;
        polyfill_c_(list,num,strips,&ptr);
        if (ptr == -1) {
                printf("Warning:overflow encountered in polygon filling.\n");
		goto skipfill;
        }
/* fill the interior of the ROI */
        for(i=0;i<ptr;i=i+3) {
                y = strips[i+2];
/* clip the strip to the current image */
		st = strips[i];
                en = strips[i+1];
                if (st < 0) st = 0;
                if (en >= imagex) en = imagex -1;
                if ((st <= en) && (y >= 0) && (y < imagey)) {
/* fill the current strip */
                        j = st+(y*imagex);  /* start of line */
			for(x=st;x<=en;x++) {
				tmp[j] = 1;
				j++;
			}
		}
	}

skipfill:
#ifdef DEBUG
	i = 0;
	while(ident_list[i].name != 0) {
		printf("%s = %lf\n",ident_list[i].name,ident_list[i].value);
		i++;
	}
#endif
/* setup user defined IDENTIFIER search */
	set_user_identifier(User_Identifiers);
	if (init_parser(exp,1L)) goto err_out;
	j = 0;
	for(y=0;y<imagey;y++) {
		ident_list[Y_ID].value = y;
#ifdef DEBUG
	printf("\nWorking on line %ld :",y);
#endif
		for(x=0;x<imagex;x++) {
			ident_list[X_ID].value = x;
			if (parse_math(out,&value) != 0L) goto err_out;
			if (reset_tokenlist_ptr()) goto err_out;
			if (value < 0.0) value = 0.0;
			if (value > 255.0) value = 255.0;
			image_data[j] = value;
			j++; 
		}
	}
/* remove user defined IDENTIFIER search */
	set_user_identifier(0L);
	free(tmp);
	return(0);

err_out:
/* remove user defined IDENTIFIER search */
	set_user_identifier(0L);
	free(tmp);
	return(1);
}
