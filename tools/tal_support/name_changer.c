/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: name_changer.c 1250 2005-09-16 15:51:42Z dforeman $
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
#include <string.h>

void name_changer(char *in, long int num, long int *error, char *out)
{
	long int per2ptr,per1ptr;
	long int i,j,x,st,k;
	char	temp[30];

/*	assume error */
	*error = 1;
	strcpy(out,in);
/* 	check for valid num */
	if (num < 0) {
		return;
	}
/* convert num into a string */
	if ( sprintf(temp,"%020ld STOP",num) < 0) return;
/* this is the start point for the * code */
	for(st=0; temp[st] == '0'; st++);
	if (num == 0) st = 19;

	per1ptr = -1;
	per2ptr = 0;
	x = 0;
	for(i=0;i<strlen(in);i++) {
		if (in[i] == '*') {
/* insert number */
			for(j=st; j < 20; j++) out[x++] = temp[j];
		} else if (in[i] == '%') {
/* catch first % and copy */
			if (per1ptr == -1) per1ptr = x;
			per2ptr = x;
			out[x++] = in[i];
		} else {
			if (per1ptr != -1) {
/* replace with number */
				j=19-(per2ptr-per1ptr);
				for(k=per1ptr; k <= per2ptr; k++) {
					out[k] = temp[j++];
				}
				per1ptr = -1;
			}
/* copy */
			out[x++] = in[i];
		}
	}
	if (per1ptr != -1) {
/* replace with number */
		j=19-(per2ptr-per1ptr);
		for(k=per1ptr; k <= per2ptr; k++) {
			out[k] = temp[j++];
		}
		per1ptr = -1;
	}
/* terminate */
	out[x] = '\0';
	*error = 0;
	return;
}
