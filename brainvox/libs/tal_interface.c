#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "bv_shmem.h"

int tal_shm_is_file(char *file);
int tal_shm_IO(char *file,char type,long int dx,long int dy,long int dz,
        unsigned char *data);
void tal_shm_cleanup(void);

/* brainvox references are of the form:
	:X_Y_%%%_{options}
	
	X = brainvox invocation number (0=first)
	Y = volume number (0=image 1=texture)
	%%% = filename template (for slice indexing)
	options:
		r=replicated interpolation (default is linear)
		d=deferred interpolation (default is immediate)
*/

/* global data */
#define MAX_SHMEM_VOLS	4

int	tal_bv_copy[MAX_SHMEM_VOLS] = 	{0,0,0,0};  /* brainvox indexes 1-4 */
int	tal_bv_chan[MAX_SHMEM_VOLS][2] = {{0,0},{0,0},{0,0},{0,0}};

Brainvox_Globals	tal_bv_gbls[MAX_SHMEM_VOLS];


int tal_shm_is_file(char *file)
{
	if ((file[0] == ':') && (file[2] == '_') && (file[4] == '_') &&
		(isdigit(file[1])) && (isdigit(file[3]))) {
		if (strlen(file) >= 8) return(1);
	}
	return(0);
}

void tal_shm_cleanup()
{
	long int	i;

/* for all slots */
	for(i=0;i<MAX_SHMEM_VOLS;i++) {
		if (tal_bv_copy[i]) {
/* perform any late interpolation */
			if (tal_bv_chan[i][0]) {
				bv_interpolate_(&(tal_bv_gbls[i]),-1,0);
			}
			if (tal_bv_chan[i][1]) {
				bv_interpolate_(&(tal_bv_gbls[i]),-1,1);
			}
/* detach all shared memory */
			bv_shmem_close_(&(tal_bv_gbls[i]));
/* clean up globals */
			tal_bv_copy[i] = 0;
			tal_bv_chan[i][0] = 0;
			tal_bv_chan[i][1] = 0;
		}
	}

	return;
}

int tal_shm_IO(char *file,char type,long int dx,long int dy,long int dz,
	unsigned char *data)
{
	long int	i,chan,slice,status,j;
	char		tmp[256];

/* Should check for IPC_QUIT on any open connection 
    (but currently cannot as the mesg routines use a single global block) */

#ifdef DEBUG
	printf("in tal_shm_IO:%s,%c,%ld%ld,%ld\n",file,type,dx,dy,dz);
#endif
	
/* valid file */
	if (!tal_shm_is_file(file)) return(1);

/* extract the index and channel number */
	i = file[1] - '0';
	chan = file[3] - '0';
	if ((i < 0) || (i > MAX_SHMEM_VOLS-1)) return(1);
	if ((chan < 0) || (chan > 1)) return(1);

#ifdef DEBUG
	printf("Copy=%ld,channel=%ld\n",i,chan);
#endif

/* are they open? */
	if (tal_bv_copy[i] == 0) {
		if (bv_index_open_(i,&(tal_bv_gbls[i]))) {
		fprintf(stderr,"Unable to attach to Brainvox:%ld\n",i);
			return(1);
		}
		tal_bv_copy[i] = 1;
	}
/* mark the channel as well */
	tal_bv_chan[i][chan] = 1;

/* parse the options (interpolations) */
	if (strlen(file) >= 10) {
		j = 9;
		while(file[j] != '\0') {
			switch(file[j]) {
				case 'r':
			tal_bv_gbls[i].interpolate &= (~(BV_INTERP_LINEAR));
					break;
				case 'd':
			tal_bv_gbls[i].interpolate &= (~(BV_INTERP_IMMEDIATE));
					break;
				default:
	fprintf(stderr,"Unknown volume option:%c\n",file[j]);
					return(1);
					break;
			}
			j++;
		}
	}

/* extract the slice number */
	tmp[0] = file[5];
	tmp[1] = file[6];
	tmp[2] = file[7];
	tmp[3] = '\0';
	slice = atoi(tmp);

#ifdef DEBUG
	printf("slice= %ld interp=%ld\n",slice,tal_bv_gbls[i].interpolate);
#endif

/* check the image sizes */
	if ((dx != tal_bv_gbls[i].imagesize) || 
		(dy != tal_bv_gbls[i].imagesize) || (dz != 1)) {
		fprintf(stderr,"Volume sizes/depths do not match\n");
		return(1);
	}

/* service the request */
	switch(type) {
		case 'r':
			status=bv_realimage_IO_(&(tal_bv_gbls[i]),slice,chan,
				IPC_READ,data);
			break;
		case 'w':
			status=bv_realimage_IO_(&(tal_bv_gbls[i]),slice,chan,
				IPC_WRITE,data);
			break;
		default:
			return(1);
			break;
	}

#ifdef DEBUG
	printf("status=%ld\n",status);
#endif

	return(status);
}
