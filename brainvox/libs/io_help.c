#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <time.h>
#include <fcntl.h>
#ifndef WIN32
#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>
#else
#include <windows.h>
#include <io.h>
#include <direct.h>
#endif

#include "prefs.h"
#include "match.h"

#ifdef WIN32

#define R_OK 4
#define W_OK 2
#define X_OK 4
#define F_OK 0

#define SUFFIX	"*"
#define	SLASH	"\\"
#define streq(a,b)	(strcmp(a,b)==0)

#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)

struct dirent
{
  long			d_ino;		/* Always zero. */ 
  unsigned short	d_reclen;	/* Always zero. */ 
  unsigned short	d_namlen;	/* Length of name in d_name. */ 
  char			d_name[1];	/* File name. */
  /* NOTE: dirent structures are variable in length, big enough  */
  /*       to hold the full length of d_name[]  */
  /*       Thus, sizeof(dirent) is misleading... */
};

#define DIRBLKSIZ       512
#define MAXNAMLEN	255	/* maximum filename length */	

typedef struct
{
  /* dd_path_dummy extends "dirent" to provide a place for the filename */
  struct dirent		dd_dir;
  char			dd_path_dummy[MAXNAMLEN];

  struct _finddata_t	dd_dta;    /* disk transfer area */
  intptr_t		dd_handle; /* _findnext handle  */

  /* Status of search: */
  /*   0 = not started yet (next entry to read is first entry) */
  /*  -1 = off the end */
  /*   positive = 0 based index of next entry */
       
  short			dd_stat;          /* Status of search: */

  /* given path for dir with search pattern (struct is extended) */
  char			dd_name[MAXNAMLEN];

} DIR;

DIR*		opendir (const char* szPath);
struct dirent*	readdir (DIR* dir);
int		closedir (DIR* dir);

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
        	if (strlen(t) > 3) {
        	if (t[strlen(t)-1] == '/') t[strlen(t)-1] ='\0';
        	if (t[strlen(t)-1] == '\\') t[strlen(t)-1] ='\0';
        }
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
closedir(DIR* dirp)
{
	int	rc;

	errno = 0;
	rc = 0;

	if (!dirp) return -1;

	if (dirp->dd_handle != -1) {
		rc = _findclose(dirp->dd_handle);
	}

        free(dirp);

	return rc;
}

#define access _access
#define unlink _unlink
#define getpid _getpid
#define getcwd _getcwd
#define chdir _chdir
#define mkdir _mkdir
#define rmdir _rmdir
#define open _open
#define close _close
#define read _read
#define write _write
#define stat _stat
#define popen _popen
#define pclose _pclose

#endif

#ifdef CAP_NOUS
#define dir_open_ DIR_OPEN
#define dir_close_ DIR_CLOSE
#define dir_read_ DIR_READ
#define get_ref_ GET_REF
#define get_val_ GET_VAL
#define p_open_ P_OPEN
#define p_close_ P_CLOSE
#define p_read_line_ P_READ_LINE
#define p_write_line_ P_WRITE_LINE
#define bitslice_ BITSLICE
#define data_histo_ DATA_HISTO
#define bv_open_ BV_OPEN
#define bv_read_ BV_READ
#define bv_write_ BV_WRITE
#define bv_close_ BV_CLOSE
#define bv_mkdir_ BV_MKDIR
#define bv_rename_ BV_RENAME
#define bv_rmdir_ BV_RMDIR
#define bv_etime_ BV_ETIME
#define bv_exists_ BV_EXISTS
#define bv_gettempdir_ BV_GETTEMPDIR
#define bv_get_app_path_ BV_GET_APP_PATH
#define bv_prepend_path_ BV_PREPEND_PATH
#define bv_set_app_name_ BV_SET_APP_NAME
#define bv_system_path_ BV_SYSTEM_PATH
#define bv_system_ BV_SYSTEM
#define bv_exit_ BV_EXIT
#define bv_frand_ BV_FRAND
#define bv_seed_frand_ BV_SEED_FRAND
#define bv_fork_ BV_FORK
#define bv_unlink_ BV_UNLINK
#define bv_getpid_ BV_GETPID
#define bv_time_ BV_TIME
#define bv_getcwd_ BV_GETCWD
#define bv_chdir_ BV_CHDIR
#define bv_access_ BV_ACCESS
#define bv_os_ BV_OS
#define bv_drives_ BV_DRIVES
#endif

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

void copy_name(long int *loc,char *output,char *name,long int len,
        long int isdir,long int *num);

void    read_filenames_(char *dir,char *output,long int len,char *filter,
        long int *num,long int fdir)
{
        DIR             *dfp;
        struct dirent   *dp;
        long int        i;
        struct  stat mystat;
        char    tstr[256],tt[256];
        long int        isdir;

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
void    READ_FILENAMES(char *dir,char *output,long int len,char *filter,
        long int *num,long int fdir)
{
read_filenames_(dir,output,len,filter,num,fdir);
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

/* 	File format:

TAGGED_DATAFILE:note
X_NAME:value
X_NAME:value
X_NAME:value
X_NAME:value
TAG_END

	name is: X_NAME
	where X is:
		F = float
		I = long int
		S = string
		W = word
*/

void	read_tagged_file_(char *file,long int *num,FTag *tags,long int *err)
{
	long int	i;
	FILE		*fp;
	char		tstr[256],*t,tname[256];

/* first none of the data have been found */
	for(i=0;i<(*num);i++) {
		tags[i].found = 0;
	}

	fp = fopen(file,"r");
	if (fp == 0L) {
		*err = 1;
		return;
	}
	if (fgets(tstr,256,fp) == 0) {
		fclose(fp);
		*err = 1;
		return;
	}
	if (strstr(tstr,"TAGGED_DATAFILE:") == 0) {
		fclose(fp);
		*err = 1;
		return;
	}

	while (fgets(tstr,256,fp)) {

		if (strstr(tstr,"TAG_END") == tstr) break;


		for(i=0;i<(*num);i++) {
			strcpy(tname,tags[i].name);
			strcat(tname,":");
			if (strstr(tstr,tname) == tstr) {
/* remove new line */
				t = strstr(tstr,"\n");
				if (t != 0L) *t = '\0';
/* find colon */
				t = strstr(tstr,":");
				if (t == 0L) break;
				t += 1;
				
				tags[i].found = 1;
				switch(tags[i].name[0]) {
					case 'S':
						strcpy((char *)tags[i].data,t);
						break;
					case 'I':
						sscanf(t,"%ld",
						    (long int *)tags[i].data);
						break;
					case 'F':
						sscanf(t,"%f",
							(float *)tags[i].data);
						break;
					case 'W':
						sscanf(t,"%s",
							(char *)tags[i].data);
						break;
				}
			}
		}
	}

	fclose(fp);

	*err = 0;
	return;
}
void	READ_TAGGED_FILE(char *file,long int *num,FTag *tags,long int *err)
{
read_tagged_file_(file,num,tags,err);
}

void	write_tagged_file_(char *file,char *note,long int *update,
		long int *num,FTag *tags,long int *err)
{
	long int	i,matched;
	FILE		*fp,*old;
	char		tmpfile[256],tstr[256],tname[256];

	strcpy(tmpfile,file);
	strcat(tmpfile,"__TEMP__");

	fp = fopen(tmpfile,"w");
	if (fp == 0L) {
		*err = 1;
		return;
	}
	fprintf(fp,"TAGGED_DATAFILE:%s\n",note);

	for(i=0;i<(*num);i++) {
		switch(tags[i].name[0]) {
			case 'S':
			case 'W':
				fprintf(fp,"%s:%s\n",tags[i].name,
					(char *)tags[i].data);
				break;
			case 'I':
				fprintf(fp,"%s:%ld\n",tags[i].name,
					*((long int *)tags[i].data));
				break;
			case 'F':
				fprintf(fp,"%s:%f\n",tags[i].name,
					*((float *)tags[i].data));
				break;
		}
	}

	if ((*update) != 0) {
/* copy any additional entries from the previous file (if any) */
		old = fopen(file,"r");
		if (old == 0L) goto done;

		if (fgets(tstr,256,old) == 0) {
			fclose(old);
			goto done;
		}
		if (strstr(tstr,"TAGGED_DATAFILE:") == 0) {
			fclose(old);
			goto done;
		}

		while (fgets(tstr,256,old)) {

			if (strstr(tstr,"TAG_END") == tstr) {
				fclose(old);
				goto done;
			}
			matched = 0;
			for(i=0;i<(*num);i++) {
/* found a replicate, mark to not copy */
				strcpy(tname,tags[i].name);
				strcat(tname,":");
				if (strstr(tstr,tname) == tstr) matched = 1;
			}
			if (matched == 0) {
/* copy into the new file */
				fprintf(fp,"%s",tstr);
			}
		}
		fclose(old);
	}

done:
	fprintf(fp,"TAG_END\n");
	
	fclose(fp);

/* make the new file it... */
/*
#ifdef WIN32
	sprintf(tstr,"move \"%s\" \"%s\"",tmpfile,file);
#else
	sprintf(tstr,"mv %s %s",tmpfile,file);
#endif
	system(tstr);
*/
        unlink(file);
	if (rename(tmpfile,file)) {
		*err = 2;
	}

	*err = 0;
	return;
}
void	WRITE_TAGGED_FILE(char *file,char *note,long int *update,
		long int *num,FTag *tags,long int *err)
{
write_tagged_file_(file,note,update,num,tags,err);
}


/* dereference functions */
void	get_ref_(long int *data,long int *ptr)
{
	*ptr = (long int)data;

	return;
}

void	get_val_(long int **ptr,long int *val)
{
	*val = (**ptr);

	return;
}

/* popen helper functions */
void	p_open_(char *cmd,char *type,long int *fp)
{

	*fp = (long int)popen(cmd,type);

	return;
}

void	p_close_(long int *fp)
{

	pclose((FILE *)*fp);

	return;
}

void	p_read_line_(char *text,long int *length,long int *fp)
{
	char	*i;

	i = fgets(text,*length,(FILE *)*fp);
	if (i == 0L) {
		*length = 0;
		return;
	}

	if (text[strlen(text)-1] == '\n') text[strlen(text)-1] = '\0';

	*length = strlen(text);

	return;
}

void	p_write_line_(char *text,long int *fp)
{
	fprintf((FILE *)*fp,"%s\n",text);

	return;
	
}

/* routine to aid bitslicing */
/* copy the "bits" high order bits into the top of "out" */
/* 0<bits<8 */
void	bitslice_(unsigned char *in,unsigned char *out,long int *size,
	long int *inmin,long int *inmax,long int *outmin,long int *outmax,
	long int *bits)
{
	long int	i,j,k;
	double		din,dout;
	unsigned char	mask[9]={0x0,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff};

	din = (*inmax)-(*inmin);
	dout = (*outmax)-(*outmin);
	
	for(i=0;i<(*size);i++) {
		j = out[i];
		j = ((j-(*outmin))/dout)*255.0;
		if (j < 0) j = 0;
		if (j > 255) j = 255;
		j = j >> (*bits);
		k = in[i];
		k = ((k-(*inmin))/din)*255.0;
		if (k < 0) k = 0;
		if (k > 255) k = 255;
		j = j | (mask[(*bits)] & k);
		out[i] = j;
	}
	return;
}

void data_histo_(unsigned char *in,long int *size,long int *hist)
{
	long int	i;

	for(i=0;i<(*size);i++) hist[in[i]] += 1;

	return;
}

/* UNIX function wrappers... */
const char *strnchr(const char *s,int c,int l)
{
	int	i;
	for(i=0;i<l;i++) {
		if (!s[i]) return(NULL);
		if (s[i] == c) return(s+i);
	}
	return(NULL);
}

#ifdef CAP_NOUS
void bv_sginap_(unsigned long *ticks);
void BV_SGINAP(unsigned long *ticks) { bv_sginap_(ticks); };
#endif

/* ticks are in miliseconds.  sleep() in seconds, usleep() in microseconds */
void bv_sginap_(unsigned long *ticks)
{
#ifdef WIN32
        Sleep(*ticks);
#else
        unsigned int s,n;
        n = (*ticks)*1000;
        s = (*ticks)/1000;
        if (s) {
           sleep(s);
           n = n - (s*1000000);
        }
        usleep(n);
#endif
}

int bv_open_(const char *p, const char *flags, int *mode,int s1,int s2)
{
	int flg = 0;

	if (strnchr(flags,'r',s2) && strnchr(flags,'w',s2)) {
		flg |= O_RDWR;
	} else if (strnchr(flags,'r',s2)) {
		flg |= O_RDONLY;
	} else if (strnchr(flags,'w',s2)) {
		flg |= O_WRONLY;
	}
	if (strnchr(flags,'c',s2)) flg |= O_CREAT;
	if (strnchr(flags,'x',s2)) flg |= O_EXCL;
	if (strnchr(flags,'t',s2)) flg |= O_TRUNC;
#ifdef WIN32
        flg |= _O_BINARY;
#endif
	return(open(p,flg,*mode));
}
int bv_read_(int *fp, void *buf, int *count)
{
	return(read(*fp,buf,*count));
}
int bv_write_(int *fp, void *buf, int *count)
{
	return(write(*fp,buf,*count));
}
int bv_close_(int *fp)
{
	return(close(*fp));
}
int bv_mkdir_(const char *p, int *mode)
{
#ifdef WIN32
	return(mkdir(p));
#else
	return(mkdir(p,*mode));
#endif
}
int bv_rename_(const char *src, const char *dst)
{
	return(rename(src,dst));
}
int bv_rmdir_(const char *dir)
{
	return(rmdir(dir)); 
}
float bv_etime_(float *arr)
{
	static int first = 0;
	static double fv;
	double	d;

#ifdef WIN32
	d = ((double)GetTickCount())/1000.0;
#else
        struct timeval t;

        gettimeofday(&t,NULL);
	d = (double)t.tv_sec + ((double)t.tv_usec)*1E-6;
#endif
	arr[1] = 0;
	if (!first) {
		first = 1;
		fv = d;
	}
	arr[0] = d - fv;
	return(arr[0]+arr[1]);
/*
	clock_t c = clock();
	arr[0] = ((float)c)/(float)(CLOCKS_PER_SEC);
	arr[1] = 0;
	return(arr[0]+arr[1]);
*/
}

void bv_gettempdir_(char *p,int len)
{
        int i;

        strcpy(p,"/tmp");
        if (getenv("TMP")) strcpy(p,getenv("TMP"));
        if (getenv("TEMP")) strcpy(p,getenv("TEMP"));
        if (getenv("TMPDIR")) strcpy(p,getenv("TMPDIR"));

#ifdef WIN32
	strcat(p,"\\");
#else
	strcat(p,"/");
#endif
	i = strlen(p);
	while(i<len) p[i++] = ' ';

        return;
}

int bv_access_(const char *p,const char *m,int s1,int s2)
{
	int mode = 0;
	if (strnchr(m,'r',s2)) mode |= R_OK;
	if (strnchr(m,'w',s2)) mode |= W_OK;
	if (strnchr(m,'x',s2)) mode |= X_OK;
	if (strnchr(m,'f',s2)) mode |= F_OK;
	return(access(p,mode));
}
int bv_chdir_(const char *p)
{
	return(chdir(p));
}
int bv_getcwd_(char *p,int maxlen)
{
	if (getcwd(p,maxlen-1)) return(1);
	return(0);
}
int bv_time_(void)
{
	return(time(NULL));
}
int bv_getpid_(void)
{
	return(getpid());
}
int bv_unlink_(const char *p)
{
	return(unlink(p));
}
int bv_fork_(void)
{
#ifdef WIN32
	return(-1);
#endif
	return(fork());
}
float bv_frand_(void)
{
	float d = ((float)rand())/(float)(RAND_MAX);
	return(d);
}
void bv_seed_frand_(int *seed)
{
	srand(*seed);
}
void bv_exit_(int *code)
{
	exit(*code);
}

char *app_path(char *argv0);
static char *appl_path = NULL;

void bv_system_(const char *p,int len)
{
      char *t1 = (char *)malloc(len+1);

      strncpy(t1,p,len);
      t1[len] = '\0'; len--;
      while(t1[len] == ' ') { t1[len] = '\0'; len--; }

#ifdef WIN32
      if (strchr(t1,'&')) {
         char tt[1024],*t;
         sprintf(tt,"start /B %s",t1);
         t = strchr(tt,'&');  if (t) *t = '\0';
         system(tt);
         free(t1);
         return;
      }
#endif

      system(t1);
      free(t1);

      return;
}
void bv_system_path_(const char *s,int len)
{
   if (appl_path) {
      char *p,*t0 = NULL,*t1 = NULL;
      t0 = (char *)malloc(len+strlen(appl_path)+1);
      t1 = (char *)malloc(len+1);
      if (t0 && t1) {
         strcpy(t0,appl_path);

         strncpy(t1,s,len);
         t1[len] = '\0'; len--;
         while(t1[len] == ' ') { t1[len] = '\0'; len--; }

         strcat(t0,t1);

#ifdef WIN32
         {
/* Note: the pathname must be quoted, and the title must be there as: "" */
            char tt[1024],*t;
            if (strchr(t1,'&')) {
               sprintf(tt,"start /B \"\" %s%s",appl_path,t1);
            } else {
               sprintf(tt,"start /B /WAIT \"\" %s%s",appl_path,t1);
            }
            t = strchr(tt,'&');  if (t) *t = '\0';
            system(tt);
            free(t0);
            free(t1);
            return;
         }
#endif
         system(t0);

         free(t0);
         free(t1);
         return;
      }
   } 
   bv_system_(s,len);
   return;
}

void bv_set_app_name_(const char *p,int len)
{
   char *argv0 = NULL;

   argv0 = (char *)malloc(len+1);
   if (argv0) {
      strncpy(argv0,p,len);
      len--;
      while(argv0[len] == ' ') {
         argv0[len] = '\0';
         len--;
      }
      app_path(argv0);
      free(argv0);
   }

   return;
}

void bv_prepend_path_(char *p, int len)
{
   int i,j,k;
   j = strlen(appl_path);
   i = len;
   while(p[i-1] == ' ') i--;
   if (i+j+2 <= len) {
      /* slide existing string over */
      for(k=i;k>=0;k--) p[k+j] = p[k];
      /* stuff in the path */
      strncpy(p,appl_path,j);
   }
   return;
}

/* return FORTRAN style */
int bv_get_app_path_(char *p,int len)
{
   int i;

   if (!appl_path) return(-1);

   i = strlen(appl_path);
   if (i+2 > len) return(-1);

   strcpy(p,appl_path);

   p[i++] = ' ';
   p[i++] = ' ';

   return(0);
}

char *app_path(char *argv0) 
{
   char path[1024] = {0},cwd[1024] = {0};
   char *p;

   if (appl_path) return(appl_path);

#ifndef WIN32
   if (getenv("PATH")) strcpy(path,getenv("PATH"));
   getcwd(cwd,sizeof(cwd));

   if (argv0[0] == '/') {
      appl_path = (char *)malloc(strlen(argv0)+1);
      if (appl_path) {
         strcpy(appl_path,argv0);
         p = strrchr(appl_path,'/');
         if (p) p[1] = '\0';
      }
   } else {
      char tmp[1024];
      char *q,*p = path;
      while(p) {
         q = strchr(p,':');
         if (q) *q++ = '\0';

         strcpy(tmp,p);
         strcat(tmp,"/");
         strcat(tmp,argv0);
         if (access(tmp,X_OK|R_OK) == 0) {
            appl_path = (char *)malloc(strlen(tmp)+1);
            if (appl_path) {
               strcpy(appl_path,tmp);
               p = strrchr(appl_path,'/');
               if (p) p[1] = '\0';
            }
            return(appl_path);
         }

         p = q;
      }
      strcpy(tmp,cwd);
      strcat(tmp,"/");
      strcat(tmp,argv0);
      if (access(tmp,X_OK|R_OK) == 0) {
         appl_path = (char *)malloc(strlen(tmp)+1);
         if (appl_path) {
            strcpy(appl_path,tmp);
            p = strrchr(appl_path,'/');
            if (p) p[1] = '\0';
         }
         return(appl_path);
      }
   }
#else
   GetModuleFileName(NULL,path,sizeof(path));
   appl_path = (char *)calloc(strlen(path)+2,1);
   if (!appl_path) return(NULL);
   strcpy(appl_path,"\"");
   strcat(appl_path,path);
   p = strrchr(appl_path,'\\');
   if (p) p[1] = '\0';
   strcat(appl_path,"\"");
#endif

   return(appl_path);
}

int bv_exists_(char *p,int len)
{
      struct stat buf;
      int ret = 0;
      char *t1 = (char *)malloc(len+1);

      strncpy(t1,p,len);
      t1[len] = '\0'; len--;
      while(t1[len] == ' ') { t1[len] = '\0'; len--; }

      if (stat(t1,&buf) == 0) ret = 1;

      free(t1);

      return(ret);
}

int bv_drives_(void)
{
#if defined(WIN32)
   return(_getdrives());
#else
   return(0);
#endif
}

int bv_os_(void)
{
#if defined(WIN32)
   return(0);
#elif defined(osx)
   return(1);
#elif defined(irix)
   return(2);
#else
   return(3);
#endif
}
