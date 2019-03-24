/* Copyright 1993-2001 Roger P. Woods, M.D. */
/* Modified 5/5/01 */

/* const char *file_from_path()
 *
 * divides a full path name into a directory and returns a pointer to the file portion
 *
 * Note that calling routine must assure that directory is large enough
 */

#include "AIR.h"

const char *AIR_file_from_path(const char *input)

{
	/* Everything after the last path separator is file name */
	const char *thefile=strrchr(input,AIR_CONFIG_PATH_SEP);

	if(thefile) thefile++;
	else thefile=input;
	
	return thefile;
}
		
