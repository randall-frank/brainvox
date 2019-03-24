/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_ls.c 1599 2006-01-22 03:45:10Z rjfrank $
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

#ifdef BV_SHMEM_SUPPORT
#include "../../../brainvox/trunk/libs/bv_shmem.h"
#endif

#ifdef VOXNET_SUPPORT
#include "voxnet.h"
#include "vn_dictionary.h"
void  do_vn_get(vn_server *s,long int *t,char *var,void **data,long int *dtype);
#endif

#ifdef SHMEM_SUPPORT
#include "tal_shm_interface.h"
#endif

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options]\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -n(n) Brainvox index number default:all\n");
	fprintf(stderr,"	 -h(host) Voxnet host default:localhost\n");
	fprintf(stderr,"	 -p(port) Voxnet port default:-1\n");
	fprintf(stderr,"	 -s(volnum) Shared memory volnum default:all\n");
	fprintf(stderr,"Note: If 'n' is used, only brainvox servers will be located.\n");
	fprintf(stderr,"If 'h' or 'p' are used, only voxnet servers will be located.\n");
	fprintf(stderr,"If 's' is used, only shared memory blocks will be located.\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
/* options */
	long int	n = -1;
	long int	volnum = -1;
	int		port = -1;
	char		*host = 0L;
	long int	bvonly = 0;
	long int	vnonly = 0;
	long int	shonly = 0;

	long int	i;

#ifdef BV_SHMEM_SUPPORT
	Brainvox_Globals vars;
	long int	done;
#endif
#ifdef VOXNET_SUPPORT
        int             err;
        vn_server       server;
        long int        type,datatype;
        char            var[256],*data;
#endif

	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
		switch (argv[i][1]) {
			case 'n':
				n = atoi(&(argv[i][2]));
				bvonly = 1;
				vnonly = 0;
				shonly = 0;
				break;
			case 's':
				volnum = atoi(&(argv[i][2]));
				shonly = 1;
				bvonly = 0;
				vnonly = 0;
				break;
			case 'p':
				port = atoi(&(argv[i][2]));
				vnonly = 1;
				bvonly = 0;
				shonly = 0;
				break;
			case 'h':
				host = &(argv[i][2]);
				vnonly = 1;
				bvonly = 0;
				shonly = 0;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 0) cmd_err(argv[0]);

	if (vnonly) goto search_vn;
	if (shonly) goto search_sh;

#ifndef BV_SHMEM_SUPPORT
	fprintf(stderr,"Brainvox does not run on this machine.\n");
#else
	i = 0;
	if (n >= 0) i = n;
	done = 0;
	while (!done) {
		done = bv_index_open_(i,&vars);
		if (done == 0) {
			fprintf(stderr,"\nBrainvox server:%ld\n",i);
			printf("%d = number of images\n",vars.realimages);
			printf("%d %d = image sizes\n",vars.imagesize,
				vars.imagesize);
			printf("%d = number of interpolated images\n",
				vars.interpimages);
			printf("%d = number of regions\n",vars.numrgns);
			printf("%s = imagename template\n",vars.imagename);
			printf("%f = interpixel\n",vars.interpixel);
			printf("%f = interslice\n",vars.interslice);
			printf("%f = squeezefactor\n",vars.squeezefactor);
			printf("Patient info:\n");
			printf("%s\n",vars.patientinfo[0]);
			printf("%s\n",vars.patientinfo[1]);
			printf("%s\n",vars.patientinfo[2]);
	
/* brainvox references are of the form:
        :X_Y_%%%_{options}
        
        X = brainvox invocation number (0=first)
        Y = volume number (0=image 1=texture)
        %%% = filename template (for slice indexing)
        options:
                r=replicated interpolation (default is linear)
                d=deferred interpolation (default is immediate)
*/

			fprintf(stderr,"Access as:   :%ld_Y_%%%%%%_{r}{d}\n",i);
			bv_shmem_close_(&vars);
		}
		i++;
		if (n >= 0) done = 1;
	}
#endif
	if (bvonly) tal_exit(0);


search_vn:

#ifndef VOXNET_SUPPORT
	fprintf(stderr,"Voxnet does not run on this machine.\n");
#else
/* attach to the server */
        err=vn_attach_server(host,port,&server);
        if (err != VN_NOERR) {
                fprintf(stderr,"Unable to connect to voxnet server\n"); 
		goto search_sh;
        }
	fprintf(stderr,"\nVoxnet: Host:%s Port:%d\n",server.hostname,
		server.port);

        strcpy(var,VN_D_NSLICES);
        do_vn_get(&server,&type,var,(void **)&data,&datatype);
	printf("%ld = number of images\n",*((long int *)data));
        vn_free_data(data,datatype);

        strcpy(var,VN_d_VEXTENTS);
        do_vn_get(&server,&type,var,(void **)&data,&datatype);
	printf("%ld %ld = image sizes\n",((vn_iarray *)data)->data[0],
		((vn_iarray *)data)->data[1]);
        vn_free_data(data,datatype);

        strcpy(var,VN_D_NINTERP);
        do_vn_get(&server,&type,var,(void **)&data,&datatype);
	printf("%ld = number of interpolated images\n",*((long int *)data));
        vn_free_data(data,datatype);

        strcpy(var,VN_S_VNAME);
        do_vn_get(&server,&type,var,(void **)&data,&datatype);
	printf("%s = dataset definition file\n",data);
        vn_free_data(data,datatype);

        strcpy(var,VN_F_IPIXEL);
        do_vn_get(&server,&type,var,(void **)&data,&datatype);
	printf("%f = interpixel\n",*((double *)data));
        vn_free_data(data,datatype);

        strcpy(var,VN_F_ISLICE);
        do_vn_get(&server,&type,var,(void **)&data,&datatype);
	printf("%f = interslice\n",*((double *)data));
        vn_free_data(data,datatype);

        strcpy(var,VN_F_SQUEEZE);
        do_vn_get(&server,&type,var,(void **)&data,&datatype);
	printf("%f = squeezefactor\n",*((double *)data));
        vn_free_data(data,datatype);

        strcpy(var,VN_d_AUXVOLS);
	data = 0;
        do_vn_get(&server,&type,var,(void **)&data,&datatype);
	if (type != VN_MSG_PUT1) {
	    fprintf(stderr,"This is an older voxnet server, please update.\n");
	} else {
	printf("Loaded volume status:\n");
		for(i=0;i<((vn_iarray *)data)->num;i++) {
			printf("%ld : %ld\n",i,((vn_iarray *)data)->data[i]);
		}
	}
        vn_free_data(data,datatype);

/* voxnet references are of the form:
        ::Y_%%%{_host{_port{_options}}}
        
        Y = volume number (0=base 1+=aux)
        %%% = filename template (for slice indexing)
	host = hostname of server (default:localhost)
	port = port number of server (deafult:-1)
        options:
                r=replicated interpolation (default is linear)
*/
	fprintf(stderr,"Access as:  ::Y_%%%%%%_%s_%d_{r}\n",server.hostname,
		server.port);

	vn_close_server(&server);
#endif

	if (vnonly) tal_exit(0);

search_sh:

#ifndef SHMEM_SUPPORT
	fprintf(stderr,"Shared memory blocks are not supported.\n");
#else
	if (volnum == -1) {
		for(i=0;i<MAX_SHM_BLOCKS;i++) {
			int	size,shmid;
			size = tal_shmblk_exists(i,&shmid);
			if (size > 0) {
				printf(":%.2ld_%%%%%%\t%d\n",i,size);
			}
		}
	} else {
		int	size,shmid;
		size = tal_shmblk_exists(volnum,&shmid);
		if (size > 0) {
			printf(":%.2ld_%%%%%%\t%d\n",volnum,size);
		} else {
			printf("Memory block %ld does not exist\n",volnum);
		}
	}
#endif
	if (shonly) tal_exit(0);

	tal_exit(0);

	exit(0);
}

#ifdef VOXNET_SUPPORT
void  do_vn_get(vn_server *s,long int *t,char *var,void **data,long int *dtype)
{
        long int ref,aux;
        long int err;

        err=vn_make_and_send_msg(s,VN_MSG_GET1,VN_NULL_DATASET,0L,var,0L);
        if (err != VN_NOERR) {
                fprintf(stderr,"Unable to send a message\n");
                vn_close_server(s);
                tal_exit(1);
        }
/* recover the reply */
        err=vn_read_and_parse_msg(s,t,&ref,&aux,var,data,dtype);
        if (err != VN_NOERR) {
                fprintf(stderr,"Unable to read a message\n");
                vn_close_server(s);
                tal_exit(1);
        }
        return;
}
#endif
