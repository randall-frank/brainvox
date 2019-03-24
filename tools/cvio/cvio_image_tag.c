/*
 * Copyright (c), 1990-2005 Randall Frank and Thomas Grabowski
 *
 * $Id $
 * $HeadURL $
 *
 *                University of Iowa, Department of Neurology,
 *                Human Neuroanatomy and Neuroimaging Lab
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
 * REVISION HISTORY:
 *	Updated 05/03/2001 by Brent Eaton:  changed type of variable value
 *					    from USHORT to SHORT.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include "cvio_lib.h"

void info(char *app)
{
        fprintf(stderr,"(%s) Usage: %s [options] srcfile dstfile dirname startnum numimages\n",__DATE__,app);
        fprintf(stderr,"Options:\n");
        fprintf(stderr,"        -v verbose\n");
        fprintf(stderr,"        -m(matrix) image size (default:128)\n");
        fprintf(stderr,"        -h(header) image header size in bytes (default:8432)\n");
        fprintf(stderr,"        -a(wait)  abort if no samples for wait seconds. (default: 300)\n");
        fprintf(stderr,"        -s(str) file template (sprintf string) (default: none)\n");
	fprintf(stderr,"	-e(ext) file extension\n");
	fprintf(stderr,"	-n no timestamp stream present\n");
	fprintf(stderr,"\nNotes:\n");
	fprintf(stderr,"\tif no -s, the images are named: DIRNAME/iX.MRGE.Y\n");
	fprintf(stderr,"\tX=startnum+inum  Y=inum (starting at 0)\n");
	fprintf(stderr,"\tif -s, the image are named: DIRNAME/STRX\n");
	fprintf(stderr,"\tSTRX=sprintf(STRX,STR,startnum+inum) (inum starts at 0)\n");
        exit(1);
}

void	exit_cvio_error(int32_t err) 
{
	char	tstr[256];
	int32_t	tlen = 256;

	cvio_get_error_message(err,tstr,&tlen);
	fprintf(stderr,"Fatal error:%s\n",tstr);
	fprintf(stderr,"cvio_image_tag\n");

	cvio_cleanup(0);
	exit(1);
}

int main(int argc, char *argv[])
{
	int32_t count;
        int32_t 	verbose = 0;
	int32_t 	nostamp = 0;
        int32_t 	hdr_size = 8432;
	int32_t	abort_time = 300;
	int32_t   matrix = 128;
	char	*template = NULL;
	time_t	mytime;
	char	*srcfile = NULL;
	char	*dstfile;
	char	*directory;
	char	*extension;
	int32_t	baseimagenum;
        uint32_t  src_s,dst_s;
        int32_t   err;
	int32_t	numimages;
	int32_t		i;
	CVIO_DT_TIME	ts;
	CVIO_DT_USHORT 	*myimage;
	CVIO_DT_SHORT	value;
	char	filename[1024];
	FILE	*fp;
	int32_t   datatype,ndims,dims[3],imgsize;
	int kludge = 0;

	/* parse the options */
        i = 1;
        while((i<argc) && (argv[i][0] == '-')) {
                switch(argv[i][1]) {
                        case 'v' : verbose = 1; break;
                        case 'h' : 
				hdr_size = atoi(argv[i]+2);
				break;
                        case 'a' : 
				abort_time = atoi(argv[i]+2);
				break;
                        case 'm' : 
				matrix = atoi(argv[i]+2);
				break;
			case 'e' :
				extension = argv[i]+2;
				break;
                        case 's' : 
				template = argv[i]+2;
				break;
			case 'n' :
				nostamp = 1;
				break;
                        default: info(argv[0]); break;
                }
                i++;
	}

	if (!nostamp) {
		if (argc-i != 5) info(argv[0]);
	} else {
		if (argc-i != 4) info(argv[0]);
	}

	/* get the filenames */
	kludge = i;
	if (!nostamp)
		srcfile = argv[kludge++];
	dstfile = argv[kludge++];
	directory = argv[kludge++];
	baseimagenum = atoi(argv[kludge++]);
	numimages = atoi(argv[kludge++]);

	if (cvio_init()) exit(1);

	/* open the source file */
	if (!nostamp) {
		err = cvio_open(srcfile,CVIO_ACCESS_READ,&src_s);
		if (err) exit_cvio_error(err);

		/* get its format and size */
		err = cvio_datatype(src_s,&datatype,&ndims,dims);
		if (err) exit_cvio_error(err);

		/* 16bit, scalars */
		if ((datatype != CVIO_TYP_IMPULSE) || 
		    (ndims != 1) || (dims[0] != 1)) {
			fprintf(stderr,"Unexpected input CVIO format.");
			cvio_cleanup(0);
			exit(1);
		}
	}

	/* create the destination file */
	dims[0] = matrix;
	dims[1] = matrix;
	err = cvio_create(dstfile,numimages,CVIO_TYP_SHORT,2,dims,0);
	if (err) exit_cvio_error(err);

	/* open the destination file */
	err = cvio_open(dstfile,CVIO_ACCESS_APPEND,&dst_s);
	if (err) exit_cvio_error(err);

	/* calculate the buffer memory */
	imgsize = matrix*matrix*sizeof(CVIO_DT_USHORT)+hdr_size;
	myimage = (CVIO_DT_USHORT *)malloc(imgsize);
	if (!myimage) {
		fprintf(stderr,"Unable to allocate buffer memory.\n");
		cvio_cleanup(0);
		exit(1);
	}

	for(i=0;i<numimages;i++) {

		if (verbose) printf("Looking for image:%d\n",i);

		if (!nostamp) {
			/* find the next signal */
			count = 1;
	//XXX use the timeout feature when available
			err = cvio_read_next_samples_blocking(src_s,&ts,&value, &count, 0, CVIO_TIMEOUT_LOCAL);
			if (err != CVIO_ERR_OK) {
				printf("End of acquisition\n");
				cvio_cleanup(0);
				exit(1);
			}
		} else {
			ts = CVIO_TIME_NEXT;
		}
		/* look/wait for image to arrive */
		if (template) {
			char	tmp[1024];
			strcpy(tmp,directory);
			strcat(tmp,"/");
			strcat(tmp,template);
			sprintf(filename,tmp,baseimagenum+i);
		} else {
			sprintf(filename,"%s/i%d.MRGE.%d",directory,
				baseimagenum+i,i+1);
		}
		if (verbose) printf("looking for: %d %s\n",ts,filename);

		/* read the image */
		fp = NULL;
		mytime = time(NULL);
		while(!fp) {
			struct stat sbuf;
			fp = fopen(filename,"rb");
			stat(filename,&sbuf);
			if ((!fp) || (sbuf.st_size < imgsize)) {
				if (fp) fclose(fp);
				fp = NULL;
				usleep(100000);
				if ((time(NULL) - mytime) > abort_time) {
					fprintf(stderr,"Timeout: No samples for %d seconds.\n",abort_time);
					cvio_cleanup(0);
					exit(1);
				}
			}
		}

		fread(myimage,hdr_size,1,fp);
		fread(myimage,matrix*matrix*sizeof(CVIO_DT_USHORT),1,fp);

		fclose(fp);

		/* write to CVIO file */
		err = cvio_add_samples(dst_s,&ts,myimage,1);
		if (err) break;
	}
	
	/* free the buffer */
	free(myimage);

	/* report any errors */
	if (err) exit_cvio_error(err);

	/* cleanup and exit */
	err = cvio_close(src_s);
	if (!nostamp)
		err = cvio_close(dst_s);

	cvio_cleanup(0);

	exit(0);
}
