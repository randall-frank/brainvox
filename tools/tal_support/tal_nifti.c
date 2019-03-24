/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_nifti.c 1599 2006-01-22 03:45:10Z rjfrank $
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
#include "nifti1.h"

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] nifti-header\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -xy Output -x -y values in command line form\n");
	fprintf(stderr,"	 -z  Output -z value in command line form\n");
	fprintf(stderr,"	 -f  Output -f -l slice limits in command line form\n");
	fprintf(stderr,"	 -ip Output interpixel value\n");
	fprintf(stderr,"	 -is Output interslice value\n");
	fprintf(stderr,"	 -hp(hdr) Output header for interpixel value (default:-p)\n");
	fprintf(stderr,"	 -hs(hdr) Output header for interslice value (default:-s)\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	int i,iSingle,iOffset,iSwap;
	FILE *fp;
	char *file;
        nifti_1_header hdr;
	char *fmt = "unknown";
        char *hPixel = "-p";
        char *hSlice = "-s";

	/* command line options */
	long int	cmdline = 0;

	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
		switch (argv[i][1]) {
			case 'x':
				cmdline |= 1;
				break;
			case 'z':
				cmdline |= 2;
				break;
			case 'f':
				cmdline |= 4;
				break;
			case 'i':
                                if (argv[i][2] == 'p') {
					cmdline |= 8;
                                } else if (argv[i][2] == 's') {
					cmdline |= 16;
                                }
				break;
			case 'h':
                                if (argv[i][2] == 'p') {
					hPixel = argv[i]+3;
					cmdline |= 8;
                                } else if (argv[i][2] == 's') {
					hSlice = argv[i]+3;
                                }
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 1) cmd_err(argv[0]);
	file = argv[i];
 
	/* read the header file */
	fp = fopen(file,"rb");
        if (!fp) {
		fprintf(stderr,"Unable to open file: %s\n",file);
		tal_exit(1);
        }
	fread(&hdr,sizeof(hdr),1,fp);
	fclose(fp);

	/* verify and convert it as needed */
	iSwap = NIFTI_hdrcheck(&hdr,&iSingle,&iOffset);
	if (iSwap == -1) {
		fprintf(stderr,"%s is not in Analyze/NIFTI-1 format\n",file);
		tal_exit(1);
	}

	/* "cmdline" options */
	if (cmdline & 1) {
		printf(" -x%d -y%d",hdr.dim[1],hdr.dim[2]);
	}
	if (cmdline & 2) {
		int v = 1;
		if (hdr.datatype == DT_INT16) v = 2;
		if (hdr.datatype == DT_FLOAT32) v = 4;
		printf(" -z%d",v);
	}
	if (cmdline & 4) {
		printf(" -f%d -l%d",1,hdr.dim[3]);
	}
	if (cmdline & 8) {
		printf(" %s%f",hPixel,hdr.pixdim[1]);
	}
	if (cmdline & 16) {
		printf(" %s%f",hSlice,hdr.pixdim[3]);
	}
	if (cmdline) {
		printf("\n");
		tal_exit(0);
		exit(0);
	}

	/* print out the whole header */
	printf("Filename: %s\n",file);
	if (iSingle) {
		printf("Single file format. Offset:%d\n",iOffset);
	}
	printf("Dimensions:");
	for(i=0;i<hdr.dim[0];i++) printf(" %d",hdr.dim[i+1]);
	printf("\n");
	printf("Scales:");
	for(i=0;i<hdr.dim[0];i++) printf(" %f",hdr.pixdim[i+1]);
	printf("\n");
	if (hdr.datatype == DT_UINT8) fmt = "unsigned byte";
	if (hdr.datatype == DT_INT16) fmt = "short";
	if (hdr.datatype == DT_UINT16) fmt = "unsigned short";
	if (hdr.datatype == DT_FLOAT32) fmt = "float";
	if (hdr.datatype == DT_FLOAT64) fmt = "double";
	if (hdr.datatype == DT_INT32) fmt = "int";
	if (hdr.datatype == DT_UINT32) fmt = "unsigned int";
	printf("Dataformat: %s\n",fmt);
	if (iSwap) {
		printf("Endian: Non-native\n");
	} else {
		printf("Endian: Native\n");
	}
	hdr.descrip[79]='\0';
	printf("Description: %s\n",hdr.descrip);
	printf("Intent: %s\n",hdr.intent_name);
	if ((hdr.scl_slope == 0.) && (hdr.scl_inter == 0.)) hdr.scl_slope=1.;
	printf("Pixel transform: %f*v + %f\n",hdr.scl_slope,hdr.scl_inter);

	tal_exit(0);
	exit(0);
}
