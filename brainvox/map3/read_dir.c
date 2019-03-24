#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <string.h>
#ifndef WIN32
#include <dirent.h>
#else
#include <windows.h>
#include <io.h>
#include <direct.h>
#endif
#include <sys/stat.h> 

#include "match.h"

#ifdef WIN32

#define R_OK 4
#define W_OK 2
#define X_OK 4
#define F_OK 0

#define SUFFIX  "*"
#define SLASH   "\\"
#define streq(a,b)      (strcmp(a,b)==0)

#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)

struct dirent
{
  long                  d_ino;          /* Always zero. */
  unsigned short        d_reclen;       /* Always zero. */
  unsigned short        d_namlen;       /* Length of name in d_name. */
  char                  d_name[1];      /* File name. */
  /* NOTE: dirent structures are variable in length, big enough  */
  /*       to hold the full length of d_name[]  */
  /*       Thus, sizeof(dirent) is misleading... */
};

#define DIRBLKSIZ       512
#define MAXNAMLEN       255     /* maximum filename length */

typedef struct
{
  /* dd_path_dummy extends "dirent" to provide a place for the filename */
  struct dirent         dd_dir;
  char                  dd_path_dummy[MAXNAMLEN];

  struct _finddata_t    dd_dta;    /* disk transfer area */
  intptr_t              dd_handle; /* _findnext handle  */

  /* Status of search: */
  /*   0 = not started yet (next entry to read is first entry) */
  /*  -1 = off the end */
  /*   positive = 0 based index of next entry */

  short                 dd_stat;          /* Status of search: */

  /* given path for dir with search pattern (struct is extended) */  char                  dd_name[MAXNAMLEN];

} DIR;

DIR*            opendir (const char* szPath);
struct dirent*  readdir (DIR* dir);
int             closedir (DIR* dir);

struct dirent *d, *p;

/*
 * opendir
 *
 * Returns a pointer to a DIR structure appropriately filled in to begin
 * searching a directory.
 */
DIR*
opendir(const char* szPath)
{
        DIR *dir;
        struct _stat statDir;
        char t[MAXNAMLEN];

        errno = 0;

        if (!szPath) return (DIR*) 0;
        if (szPath[0] == '\0') return (DIR*) 0;

        /* Attempt to determine if the given path really is a directory. */
        strcpy(t,szPath);
        if (t[strlen(t)-1] == '/') t[strlen(t)-1] ='\0';
        if (t[strlen(t)-1] == '\\') t[strlen(t)-1] ='\0';
        if (_stat(t, &statDir)) {
                /* Error, stat should have set an error value. */
                return (DIR*) 0;
        }

        if (!S_ISDIR(statDir.st_mode)) return (DIR*) 0;

        dir = (DIR *)calloc(1,sizeof(DIR));
        if (!dir) return (DIR*) 0;

        /* Create the search expression. */
        strcpy(dir->dd_name, t);
        strcat(dir->dd_name, SLASH);

        /* Add on the search pattern */
        strcat(dir->dd_name, SUFFIX);

        /* Initialize handle to -1 so that a premature closedir doesn't try
         * to call _findclose on it. */
        dir->dd_handle = -1;

        /* Initialize the status. */
        dir->dd_stat = 0;

        return dir;
}

/*
 * readdir
 *
 * Return a pointer to a dirent structure filled with the information on the
 * next entry in the directory.
 */
struct dirent *
readdir( DIR *dirp )
{
        errno = 0;

        /* Check for valid DIR struct. */
        if (!dirp) return (struct dirent*) 0;

        if (dirp->dd_stat < 0) {
                // We have already returned all files in the directory
                // (or the structure has an invalid dd_stat).
                return (struct dirent *) 0;

        } else if (dirp->dd_stat == 0) {
                // We haven't started the search yet. so start it
                dirp->dd_handle = _findfirst(dirp->dd_name, &(dirp->dd_dta));

                if (dirp->dd_handle == -1) {
                        // no files in this directory.
                        dirp->dd_stat = -1;
                } else {
                        dirp->dd_stat = 1;
                }
        } else {
                // Get the next search entry.
                if (_findnext(dirp->dd_handle, &(dirp->dd_dta))) {
                        // We are off the end or otherwise in error.
                        _findclose (dirp->dd_handle);
                        dirp->dd_handle = -1;
                        dirp->dd_stat = -1;
                } else {
                        // Update the status to indicate the correct number.
                        dirp->dd_stat++;
                }
        }

        if (dirp->dd_stat > 0) {
                // Successfully got an entry.
                dirp->dd_dir.d_ino = 0;
                dirp->dd_dir.d_reclen = 0;
                strcpy(dirp->dd_dir.d_name,dirp->dd_dta.name);
                dirp->dd_dir.d_namlen = strlen(dirp->dd_dir.d_name);
                return &dirp->dd_dir;
        }

        return (struct dirent*) 0;
}

/*
 * closedir
 *
 * Frees up resources allocated by opendir.
 */
int
closedir (DIR* dirp)
{
        int     rc;

        errno = 0;
        rc = 0;

        if (!dirp) return -1;

        if (dirp->dd_handle != -1) {
                rc = _findclose(dirp->dd_handle);
        }

        free(dirp);

        return rc;
}

#endif

void copy_name(long int *loc,char *output,char *name,long int len,
	long int isdir,long int *num);

void	read_filenames_(char *dir,char *output,long int len,char *filter,
	long int *num,long int fdir)
{
        DIR             *dfp;
        struct dirent   *dp;
	long int	i;
	struct  stat mystat;
	char	tstr[256],tt[256];
	long int	isdir;

	i = 0;
	*num = 0;
	dfp = opendir(dir);
	if (dfp == 0L) return;

	strcpy(tstr,dir);
	strcat(tstr,"/");
	while (1) {
		dp = readdir(dfp);
		if (dp == 0L) break;
		strcpy(tt,tstr);	
		strcat(tt,dp->d_name);
		if (stat(tt,&mystat) == 0) {
			isdir = ((mystat.st_mode & S_IFDIR) == S_IFDIR);
/* filter must be defined, and filtering of dirs selected or it is a file */
			if (filter != 0L) {
				if ((fdir) || (!isdir)) {
#ifdef USE_REGEX
					re_comp(filter);
					if (re_exec(dp->d_name) == 1) {
#else
					if (match(filter,dp->d_name)) {
#endif
				copy_name(&i,output,dp->d_name,len,isdir,num);
					}
				} else {
				copy_name(&i,output,dp->d_name,len,isdir,num);
				}
			} else {
				copy_name(&i,output,dp->d_name,len,isdir,num);
			}
		}
	}
	closedir(dfp);
	return;
}

void copy_name(long int *loc,char *output,char *name,long int len,
	long int isdir,long int *num)
{
	long int j,k,i;

	i = *loc;

	j = strlen(name) + 1;
	if (isdir) j = j + 1;
	if ((i+j) >= len) return;
	
	if (isdir) {
		output[i++] = '/';
		j--;
	}
	for(k=0;k<j;k++) {
		output[i++] = name[k];
	}
	*num += 1;
	*loc = i;
	return;
}
