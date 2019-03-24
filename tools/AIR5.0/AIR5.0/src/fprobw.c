/* Copyright 1998-2002 Roger P. Woods, M.D. */
/* Modified 1/16/02 */

/*
 * int fprobw()
 *
 * Tests whether a file can be written to filename 
 *
 * returns an error number (see fprob_err() for decoding) if there will be a problem creating the specified output file
 * returns 0 if no problems are anticipated
 *
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
	if(buf->st_mode & S_IWUSR) return 0;
		return AIR_OWNER_CANT_WRITE_ERROR;
	}
	if(group==buf->st_gid){
		if(buf->st_mode & S_IWGRP) return 0;
			return AIR_GROUP_CANT_WRITE_ERROR;
		}
	/* We must be non-user, non-group member */
	if(buf->st_mode & S_IWOTH) return 0;
	return AIR_WORLD_CANT_WRITE_ERROR;
}
#endif



AIR_Error AIR_fprobw(const char *filename, const AIR_Boolean ow)

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
			if(errno==ENOENT){
#endif
				/* The file does not exist, does the directory? */
				
				{
					/* Figure out how long the directory name will be */
					const char *fileportion=strrchr(filename,AIR_CONFIG_PATH_SEP);
					if(fileportion) fileportion++;
					else fileportion=filename;
					{
						size_t length=fileportion-filename;
						
						if(length<128){
					
							char dirname[128];
							{
								size_t i;
								for(i=0;i<length;i++){
									dirname[i]=filename[i];
								}
								dirname[i]='\0';
							}
							if(dirname[0]=='\0') strcpy(dirname,AIR_CONFIG_PATH_REL_BASE);

							if(stat(dirname,&buf)) return AIR_NO_DIRECTORY_WRITE_ERROR; /* The proposed directory does not exist */
						}
						else{
							
							char *dirname=malloc(length+1);
							if(!dirname) return AIR_MEMORY_ALLOCATION_ERROR;
							{
								size_t i;
								for(i=0;i<length;i++){
									dirname[i]=filename[i];
								}
								dirname[i]='\0';
							}
							if(stat(dirname,&buf)){
								free(dirname);
								return AIR_NO_DIRECTORY_WRITE_ERROR; /* The proposed directory does not exist */
							}
							free(dirname);
						}
					}
				}
				return permissions(&buf);
#ifdef ENOENT
			}
#endif

#ifdef EACCES
			if(errno==EACCES) return AIR_ACCESS_DENIED_WRITE_ERROR;
#endif

#ifdef EFAULT
			if(errno==EFAULT) return AIR_ACCESS_CODING_WRITE_ERROR;
#endif

#ifdef EINTR
			if(errno==EINTR) return AIR_ACCESS_SIGNAL_WRITE_ERROR;
#endif

#ifdef ELOOP
			if(errno==ELOOP) return AIR_ACCESS_LINKS_WRITE_ERROR;

#endif

#ifdef EMULTIHOP
			if(errno==EMULTIHOP) return AIR_ACCESS_HOPS_WRITE_ERROR;
#endif

#ifdef ENAMETOOLONG
			if(errno==ENAMETOOLONG) return AIR_NAME_TOO_LONG_WRITE_ERROR;
#endif

#ifdef ENOLINK
			if(errno==ENOLINK) return AIR_FILE_UNMOUNTED_WRITE_ERROR;
#endif

#ifdef ETIMEDOUT
			if(errno==ETIMEDOUT) return AIR_FILE_UNAVAILABLE_WRITE_ERROR;
#endif

#ifdef ENOTDIR
			if(errno==ENOTDIR) return AIR_INVALID_PATH_WRITE_ERROR;
#endif

#ifdef EOVERFLOW
			if(errno==EOVERFLOW) return AIR_STRUCT_WRITE_ERROR;
#endif
			return AIR_UNUSUAL_FILE_WRITE_ERROR;

#if(AIR_CONFIG_REQ_PERMS==2)
		}
		else{
			/* lstat was able to find the file */
			if(!ow) return AIR_NO_PERMISSION_WRITE_ERROR; /*file exists, no overwrite permission */
			if(buf.st_mode&S_IFLNK) return permissions(&buf);
			return AIR_ATYPICAL_FILE_WRITE_ERROR; /* This is not a regular file or unsatisfied symbolic link */
		}
#endif
	}
	else{
		/* stat was able to find the file */
		if(!ow) return AIR_NO_PERMISSION_WRITE_ERROR; /* file exists, no overwrite permission */
		if(buf.st_mode&S_IFREG) return permissions(&buf);
		return AIR_ATYPICAL_FILE_WRITE_ERROR; /* This is not a regular file or unsatisfied symbolic link */
	}

#else
	/* Existence of file is irrelevant if we have overwrite permission */
	if(ow) return 0;
	{
		FILE *fp=fopen(filename,"rb");
		if(!fp){
			/* Presumably, file does not exist--can't easily deal with special files without stat() */
			/* We won't try to create file now--if the path is defective, this will lead to a failure later */
			return 0;
		}
		else{
			/* File does exist */
			(void)fclose(fp);
			return AIR_NO_PERMISSION_WRITE_ERROR;
		}
	}
#endif
}
