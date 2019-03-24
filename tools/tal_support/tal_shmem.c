/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_shmem.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include "tal_shm_interface.h"

int	cmd_rmshm = 0;

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
if (cmd_rmshm) {
	fprintf(stderr,"(%s:%s) Usage: %s shmtemplate\n",__DATE__,TAL_VERSION,s);
} else {
	fprintf(stderr,"(%s:%s) Usage: %s [options] nslices\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:1\n");
	fprintf(stderr,"Output:stdout = shmtemplate (slices=1-nslices)\n");
}
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 1;
	long int	nslices;

	long int	i;
	int 		status,shmid;
	char		tmp[256];

#ifdef SHMEM_SUPPORT
/* which command do we have */
	if (strstr(argv[0],"rmshm")) cmd_rmshm = 1;

/* handle the remove shmem block command */
if (cmd_rmshm) {
	if (argc != 2) cmd_err(argv[0]);

	if (!tal_shmblk_is_file(argv[1])) {
		ex_err("Template is not a shared memory volume.");
	}

/* extract the volume number */
        tmp[0] = argv[1][1];
        tmp[1] = argv[1][2];
        tmp[2] = '\0';
        i = atoi(tmp);

/* check for existence */
	if (tal_shmblk_exists(i,&shmid) == -1) {
		fprintf(stderr,"Unable to find shared memory volume %ld\n",i);
	} else {
/* remove the block */
		if (tal_shmblk_delete(i) == -1) {
		    	fprintf(stderr,
			    "Unable to delete shared memory volume %ld\n",i);
		}
	}

	tal_exit(0);

	exit(0);
} 

/* create new shmem block command */
	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
		switch (argv[i][1]) {
			case 'x':
				dx = atoi(&(argv[i][2]));
				break;
			case 'y':
				dy = atoi(&(argv[i][2]));
				break;
			case 'z':
				dz = atoi(&(argv[i][2]));
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 1) cmd_err(argv[0]);

/* get the number of slices */
	nslices = atoi(argv[i]);

/* the block is now: dz*dx*dy*nslices in bytes */
	i = 0;
	while(i < MAX_SHM_BLOCKS) {

/* look for the first unused block */
		status = tal_shmblk_exists(i,&shmid);
		if (status == -1) {
/* create the block */
			status = tal_shmblk_create(i,dz*dx*dy*nslices);
			if (status != -1) {
				fprintf(stdout,":%.2ld_%%%%%%\n",i);
				tal_exit(0);
			} else {
	fprintf(stderr,"Unable to create shared memory block %ld of size %ld\n",
				i,dz*dx*dy*nslices);
				tal_exit(1);
			}
		}
		i += 1;
	}

/* all blocks in use */
	ex_err("All shared memory blocks in use.");

#else
	ex_err("Shared memory blocks are not supported.");
#endif

	tal_exit(0);

	exit(0);
}
