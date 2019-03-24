#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void brainvox_reaper_(long int shm_ipc,long int shm_data,long int shm_images,
	long int shm_pts, long int shm_texture)
{
#ifndef WIN32
	long int	status;

	while (getppid() != 1) sleep(1);

	sleep(5);

/*
	fprintf(stderr,"My parent process has died...\n");
	fprintf(stderr,"I will try to reap the shared memory...\n");
*/

	delete_shared_mem_(shm_ipc,&status);
	delete_shared_mem_(shm_data,&status);
	delete_shared_mem_(shm_images,&status);
	delete_shared_mem_(shm_pts,&status);
	delete_shared_mem_(shm_texture,&status);

	exit(0);
#endif
}
void BRAINVOX_REAPER(long int shm_ipc,long int shm_data,long int shm_images,
	long int shm_pts, long int shm_texture)
{
}
