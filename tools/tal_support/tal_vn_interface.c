/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_vn_interface.c 1250 2005-09-16 15:51:42Z dforeman $
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

#include "voxnet.h"
#include "vn_dictionary.h"

int tal_voxnet_is_file(char *file);
int tal_voxnet_IO(char *file,char type,long int dx,long int dy,long int dz,
        unsigned char *data);
void tal_voxnet_cleanup(void);


/* local */
int Parse_vn_File(char *file,int *port,char *host,long int *slice,
	long int *chan,char *opts);

#define MAX_CXN 4

long int	n_cxn = 0;

typedef struct {
	vn_server	server;
	int		port;
	char		host[80];
} VoxNet_CXN;

VoxNet_CXN       s_list[MAX_CXN];

/* voxnet references are of the form:
        ::Y_%%%{_host{_port{_options}}}

        Y = volume number (0=base 1+=aux)
        %%% = filename template (for slice indexing)
        host = hostname of server (default:localhost)
        port = port number of server (deafult:-1)
        options:
                r=replicated interpolation (default is linear)
*/

int Parse_vn_File(char *file,int *port,char *host,long int *slice,
	long int *chan,char *opts)
{
	long int	i,j;
	char		tmp[256];

/* defaults */
	host[0] = 0;
	*port = -1;
	*slice = 1;
	*chan = 0;
	opts[0] = '\0';

/* check validity */
	if ((file[0] != ':')||(file[1] != ':')||(file[3] != '_')) return(0);
	if (strlen(file) < 7) return(0);

/* get the channel */	
	*chan = file[2] - '0';
	if ((*chan < 0) || (*chan > 3)) return(0);

/* get the slice number */
	tmp[0] = file[4];
	tmp[1] = file[5];
	tmp[2] = file[6];
	tmp[3] = '\0';
	*slice = atoi(tmp); 

/* get the host (if any) */
	if (strlen(file) < 9) return(1);
	i = 8;
	j = 0;
	while ((file[i] != '\0') && (file[i] != '_')) {
		host[j++] = file[i++];
	}
	host[j] = '\0'; /* terminate */
	if (file[i] == '\0') return(1); /* no more chars */
	i++;  /* bump over '_' */
	if (file[i] == '\0') return(1);

/* get the port (if any) */
	j = 0;
	while ((file[i] != '\0') && (file[i] != '_')) {
		tmp[j++] = file[i++];
	}
	tmp[j] = '\0';
	*port = atoi(tmp);
	if (file[i] == '\0') return(1); /* no more chars */
	i++;  /* bump over '_' */
	if (file[i] == '\0') return(1);

/* get any opts */
	strcpy(opts,&(file[i]));

	return(1);
}

int tal_voxnet_is_file(char *file)
{
	char		host[80],opts[256];
	long int	slice,chan;
	int		port,i;

	i = Parse_vn_File(file,&port,host,&slice,&chan,opts);

	return(i);
}

int tal_voxnet_IO(char *file,char cmd,long int dx,long int dy,long int dz,
        unsigned char *img)
{
	long int	err,server;
	char		host[80],opts[256];
	long int	islice,chan;
	int		port,i;
	vn_server	*mysvr;
	unsigned char	*data;
	long int        aux,type,ref,datatype;
	char		var[256];
	vn_slice        slice,*pslice;
	long int	interp = 0;  /* linear */


/* valid filename (and parse the server vars) */
	err = Parse_vn_File(file,&port,host,&islice,&chan,opts);
	if (!err) return(1);

/* parse the options */
	i = 0;
	while(opts[i] != '\0') {
		switch(opts[i]) {
			case 'r':
				interp = 1;
				break;
			default:
				fprintf(stderr,"Unknown option: %c\n",opts[i]);
				break;
		}
		i++;
	}

/* has this server been hit before? */
	server = -1;
	for(i=0;i<n_cxn;i++) {
/* A match?? */
		if ((s_list[i].port == port) && 
			(strcmp(s_list[i].host,host) == 0)) {
			server = i;
			break;
		}
	}
/* attach to new server */
	if (server == -1) {
/* and space for it */
		if (n_cxn >= MAX_CXN) return(1);
		server = n_cxn;
/* try to open */
		/* attach to the server */
        	err=vn_attach_server(host,port,&(s_list[server].server));
        	if (err != VN_NOERR) return(1);
/* add it on */
		s_list[server].port = port;
		strcpy(s_list[server].host,host);
		n_cxn += 1;
	}

/* server is: s_list[server].server */
	mysvr = &(s_list[server].server);
/* set the channel for I/O */
	err=vn_make_and_send_msg(mysvr,VN_MSG_PUT1,VN_NULL_DATASET,0L,
		VN_D_AUXSELECT,(char *)&chan);
	if (err != VN_NOERR) return(1);
        err=vn_read_and_parse_msg(mysvr,&type,&ref,&aux,var,
		(void **)&data,&datatype);
	if (err != VN_NOERR) return(1);
	vn_free_data(data,datatype);  /* necessary after a put??? */
/* set the interpolation option */
	err=vn_make_and_send_msg(mysvr,VN_MSG_PUT1,VN_NULL_DATASET,0L,
		VN_D_INTSLICE,(char *)&interp);
	if (err != VN_NOERR) return(1);
        err=vn_read_and_parse_msg(mysvr,&type,&ref,&aux,var,
		(void **)&data,&datatype);
	if (err != VN_NOERR) return(1);
	vn_free_data(data,datatype);  /* necessary after a put??? */

/* do the real slice I/O */

	switch(cmd) {
		case 'r':
			slice.dx = 0;
			slice.dy = 0;
			slice.slice = islice-1;
			slice.data = 0L;
			err=vn_make_and_send_msg(mysvr,VN_MSG_GET1,
				VN_NULL_DATASET,0L,VN_R_SLICE,(char *)&slice);
			if (err != VN_NOERR) return(1);
			err=vn_read_and_parse_msg(mysvr,&type,&ref,
				&aux,var,(void **)&data,&datatype);
			if (err != VN_NOERR) return(1);
			if (type != VN_MSG_PUT1) {
		fprintf(stderr,"Unable to retrieve image %ld\n",islice);
				vn_free_data(data,datatype);
				return(1);
			}
			pslice = (vn_slice *)data;
			if ((dx != pslice->dx)||(dy != pslice->dy)||(dz != 1)) {
				vn_free_data(data,datatype);
		fprintf(stderr,"Invalid image size/depth.\n");
				return(1);
			}
			for(i=0;i<dx*dy;i++) {
				img[i] = pslice->data[i];
			}
			vn_free_data(data,datatype);
			break;
		case 'w':
			if (dz != 1) {
		fprintf(stderr,"Invalid image size/depth.\n");
				return(1);
			}
			slice.dx = dx;
			slice.dy = dy;
			slice.slice = islice-1;
			slice.data = img;
			err=vn_make_and_send_msg(mysvr,VN_MSG_PUT1,
				VN_NULL_DATASET,0L,VN_R_SLICE,(char *)&slice);
			if (err != VN_NOERR) return(1);
			err=vn_read_and_parse_msg(mysvr,&type,&ref,
				&aux,var,(void **)&data,&datatype);
			if (err != VN_NOERR) return(1);
			vn_free_data(data,datatype);
			break;
		default:
			return(1);
			break;
	}

	return(0);
}

void tal_voxnet_cleanup()
{
	long int	i;

/* close em up */
	for(i=0;i<n_cxn;i++) {
		vn_close_server(&(s_list[i].server));
	}
	n_cxn = 0;

	return;
}

