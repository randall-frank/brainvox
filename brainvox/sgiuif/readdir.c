#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>  

/*
        routine to read a directory in FORTRAN.
        FORTRAN syntax: Call readdir(num,list,dir)
        where dir is a 0 terminated string of the dir to read, Num=I*4,
        and list is a byte array (256*256).  On return there
        will be NUM entries (0 terminated) in LIST.  The . and ..
        entries will not have been removed.
*/ 

#ifndef WIN32
#include <dirent.h>
#else

#include <windows.h>

struct dirent {  
    ino_t d_ino;                /* inode (always 1 on WIN32) */  
    char d_name[MAX_PATH + 1];  /* filename (null terminated) */  
};  
   
typedef struct {  
    HANDLE handle;              /* handle for FindFirstFile or FindNextFile */  
    long offset;                /* offset into directory */  
    int finished;               /* 1 if there are not more files */  
    WIN32_FIND_DATA finddata;   /* file data FindFirstFile or FindNextFile  
                                 * returns */  
    char *dir;                  /* the directory path we are reading */  
    struct dirent ent;          /* the dirent to return */  
} DIR;  
   
DIR *opendir(const char *);  
struct dirent *readdir(DIR *);  
int closedir(DIR *);  
 
DIR * opendir(const char *dir) 
{ 
    DIR         *dp; 
    char        *findspec; 
    HANDLE      handle; 
    size_t      dirlen; 
 
    dirlen = strlen(dir); 
    findspec = malloc(dirlen + 2 + 1); 
    if (findspec == NULL) return NULL; 
 
    if (dirlen == 0) {
        strcpy(findspec, "*"); 
    } else if (isalpha(dir[0]) && dir[1] == ':' && dir[2] == '\0') {
        sprintf(findspec, "%s*", dir); 
    } else if (dir[dirlen - 1] == '/' || dir[dirlen - 1] == '\\') {
        sprintf(findspec, "%s*", dir); 
    } else {
        sprintf(findspec, "%s\\*", dir); 
    }
 
    dp = (DIR *)malloc(sizeof(DIR)); 
    if (dp == NULL) { 
        free(findspec); 
        return NULL; 
    } 
 
    dp->offset = 0; 
    dp->finished = 0; 
    dp->dir = strdup(dir); 
    if (dp->dir == NULL) { 
        free(dp); 
        free(findspec); 
        return NULL; 
    } 
 
    handle = FindFirstFile(findspec, &(dp->finddata)); 
    if (handle == INVALID_HANDLE_VALUE) { 
        free(dp->dir); 
        free(dp); 
        free(findspec); 
        return NULL; 
    } 
    dp->handle = handle; 
 
    free(findspec); 
    return dp; 
} 
 
struct dirent *readdir(DIR *dp) 
{ 
    if (dp == NULL || dp->finished) return NULL; 
 
    if (dp->offset != 0) { 
        if (FindNextFile(dp->handle, &(dp->finddata)) == 0) { 
            dp->finished = 1; 
            return NULL; 
        } 
    } 
    dp->offset++; 
 
    strncpy(dp->ent.d_name, dp->finddata.cFileName, MAX_PATH); 
    dp->ent.d_ino = 1; 
 
    return &(dp->ent); 
} 
 
int closedir(DIR *dp) 
{ 
    FindClose(dp->handle); 
    free(dp->dir); 
    free(dp); 
 
    return 0; 
} 
#endif

void readdir_(int *max,char list[256*256], char *dir)

{
	char	*dirptr;
	char	*dirptr2;
	char	buffer[512];
	struct  stat mystat;
	int  	nfiles;
	int 	i,j,k;
	DIR	*drip;
	struct dirent 	*dp;

	dirptr=strcat(dir,"/");
	drip = opendir(dir);
	nfiles=0;
	j=0;
	if (drip != NULL) 
		{
		while ((dp = readdir(drip)) != NULL)
			{
				nfiles=nfiles+1;
				strcpy(buffer,dirptr);
				dirptr2=buffer;
				dirptr2=strcat(dirptr2,dp->d_name);
				k=stat(dirptr2,&mystat);  
				if ((mystat.st_mode & S_IFDIR) == S_IFDIR)
				{
					list[j]='/';
					j=j+1;
				};
				k=0;
				while (dp->d_name[k] !='\0')
				{
					list[j]=dp->d_name[k];
					k=k+1;
					j=j+1;
				};
				list[j]='\0';
				j=j+1;
			};
		closedir(drip);
	};
	*max=nfiles;
}
