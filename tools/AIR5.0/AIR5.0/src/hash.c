/* Copyright 1993-2001 Roger P. Woods, M.D. */
/* Modified 5/9/01 */


/* unsigned int hash()
 *
 * This routine creates a semiunique identifier based on voxel values
 *
 * Note that it is required that data be written contiguously to avoid
 *  crashing this program
 *
 *
 * If x_dim is divisible by 4 (2 for 16 bit),
 * scrambling of y and z indices will not alter the hash value, but
 *  inversion of the x index will unless sizeof(AIR_Pixels)>=sizeof(int)	
 */

#include "AIR.h"

unsigned long int AIR_hash(AIR_Pixels ***volume, const struct AIR_Key_info *stats)

{
	const unsigned int x_dim=stats->x_dim;
	const unsigned int y_dim=stats->y_dim;
	const unsigned int z_dim=stats->z_dim;
	
	unsigned int *start=(unsigned int *)**volume; /* Volume must be a value returned by malloc */
	unsigned int *end=start+(unsigned long int)x_dim*y_dim*z_dim*sizeof(AIR_Pixels)/sizeof(unsigned int);
	
	unsigned int *current;
	
	unsigned int value=0;
	
	for(current=start;current<end;current++){
		value=(value^*current);
	}
	return value;
}
