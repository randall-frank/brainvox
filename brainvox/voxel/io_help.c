#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <sys/types.h>
#include <dirent.h>


void	dir_open_(long int *ret,char *dir)
{
        DIR             *dfp;

	dfp = opendir(dir);
	*ret = (long int)(dfp);

	return;
}

void	dir_close_(long int *val)
{
	DIR		*dfp;

	dfp = (DIR *)(*val);
	closedir(dfp);

	return;
}

void	dir_read_(long int *val,char *file)
{
	DIR		*dfp;
        struct dirent   *dp;

	dfp = (DIR *)(*val);
	dp = readdir(dfp);
	if (dp == 0L) {	
		file[0] = 0;
	} else {
		strcpy(file,dp->d_name);
	}

	return;
}
