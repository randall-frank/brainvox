/* Copyright 1998-2002 Roger P. Woods, M.D. */
/* Modified 1/16/02 */

/*
 * Tests whether a file can be read from filename
 *
 * returns an error number (see fprob_err() for decoding) if 
 *  there will be a problem reading the specified input file

 * returns 0 if no problems are anticipated
 */

#include "AIR.h"

#if(AIR_CONFIG_REQ_PERMS!=0)

#include <sys/types.h>	/* required for geteuid, getegid, stat and lstat */
#include <unistd.h>		/* required for geteuid ang getegid */

#include <sys/stat.h>	/* required for stat and lstat */

#include <errno.h>


static int permissions(const struct stat *buf)

{
	static uid_t user=0;
	static gid_t group=0;

	if(!user) user=geteuid();
	if(!group) group=getegid();

	/* Are we forbidden access by the filesystem? */
	if(user==buf->st_uid){
		if(buf->st_mode & S_IRUSR) return 0;
		return AIR_OWNER_CANT_READ_ERROR;
	}
	if(group==buf->st_gid){
			if(buf->st_mode & S_IRGRP) return 0;
			return AIR_GROUP_CANT_READ_ERROR;
	}
	/* We must be non-user, non-group member */
	if(buf->st_mode & S_IROTH) return 0;
	return AIR_WORLD_CANT_READ_ERROR;
}

#endif


AIR_Error AIR_fprobr(const char *filename)

{

#if(AIR_CONFIG_REQ_PERMS!=0)

	struct stat	buf;

	if(stat(filename,&buf)){
		/* stat can't find the file */
#if(AIR_CONFIG_REQ_PERMS==2)
		if(lstat(filename,&buf)){
#endif
			/* Neither stat nor lstat could find the file */

#ifdef ENOENT
			if(errno==ENOENT) return AIR_NO_FILE_READ_ERROR;
#else
			return AIR_NO_FILE_READ_ERROR;
#endif

#ifdef EACCES
			if(errno==EACCES) return AIR_ACCESS_DENIED_READ_ERROR;
#endif

#ifdef EFAULT
			if(errno==EFAULT) return AIR_ACCESS_CODING_READ_ERROR;
#endif

#ifdef EINTR
			if(errno==EINTR) return AIR_ACCESS_SIGNAL_READ_ERROR;
#endif

#ifdef ELOOP
			if(errno==ELOOP) return AIR_ACCESS_LINKS_READ_ERROR;
#endif

#ifdef EMULTIHOP
			if(errno==EMULTIHOP) return AIR_ACCESS_HOPS_READ_ERROR;
#endif

#ifdef ENAMETOOLONG
			if(errno==ENAMETOOLONG) return AIR_NAME_TOO_LONG_READ_ERROR;
#endif

#ifdef ENOLINK
			if(errno==ENOLINK) return AIR_FILE_UNMOUNTED_READ_ERROR;
#endif

#ifdef ETIMEDOUT
			if(errno==ETIMEDOUT) return AIR_FILE_UNAVAILABLE_READ_ERROR;
#endif

#ifdef ENOTDIR
			if(errno==ENOTDIR) return AIR_INVALID_PATH_READ_ERROR;
#endif

#ifdef EOVERFLOW
			if(errno==EOVERFLOW) return AIR_STRUCT_READ_ERROR;
#endif
			return AIR_UNUSUAL_FILE_READ_ERROR;

#if(AIR_CONFIG_REQ_PERMS==2)
		}
		else{
			/* lstat was able to find the file */
			if(buf.st_mode&S_IFLNK) return AIR_BAD_LINK_READ_ERROR;
			return AIR_ATYPICAL_FILE_READ_ERROR; /* This is not a regular file or unsatisfied symbolic link */
		}
#endif
	}
	else{
		/* stat was able to find the file */
		if(buf.st_mode&S_IFREG) return permissions(&buf);
		return AIR_ATYPICAL_FILE_READ_ERROR; /* This is not a regular file or unsatisfied symbolic link */
	}

#else
	/* Try to open the file for reading */
	FILE *fp=fopen(filename,"rb");
	
	if(!fp) return AIR_NO_STAT_FILE_READ_ERROR;
	else{
		/* File does exist */
		if(fclose(fp)) return AIR_CANT_CLOSE_READ_ERROR;
		return 0;
	}
#endif
}
