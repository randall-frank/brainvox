/* AIR.h --header file for Automated Image Registration Subroutines AIR 5.x
 * Copyright 1995-2001 Roger P. Woods, M.D.
 * Modified 1/16/02
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/* Lines that can be configured are labelled: USER CONFIGURABLE LINE */
/* The following information is site or platform specific */

#define AIR_CONFIG_CONFIG 1	/* USER CONFIGURABLE LINE */

		/* If AIR_CONFIG_CONFIG is 0, config.h and config_PC.h files are ignored, 
		 * thereby assuring that the values specified within this file are used 
		 * (unless explicitly overridden by compiler macros)
		 *
		 * If AIR_CONFIG_CONFIG is 1, you should run ./configure from the main AIR
		 * directory before compiling--this can only be done on UNIX systems. Any additional
		 * configuration should be made in src/config.h after ./configure has been run.
		 *
		 * If AIR_CONFIG_CONFIG is 2, the configuration file "config_PC.h" will be
		 * used. Any additional configuration should be made within that file. 
		 * Note also that the files AIRmain.h and HEADERmain.h in the main AIR
		 * directory must be modified before compiling. This option should be used for
		 * PC's running under Windows, Windows NT or MS-DOS. PC's using Linux should
		 * set AIR_CONFIG_CONFIG to 1 as described above.
		 */
		 
#if(AIR_CONFIG_CONFIG==1)
#include "config.h"
#elif(AIR_CONFIG_CONFIG==2)
#include "config_PC.h"
#define AIR_CONFIG_PATHS 2
#endif
		

#ifndef AIR_CONFIG_PATHS
#define AIR_CONFIG_PATHS 1	/* USER CONFIGURABLE LINE */
#endif
		/* AIR_CONFIG_PATHS will determine the separators used in pathnames
		 * Currently supported:
		 *	1 UNIX style paths, including Mac OS X (forward slashes)
		 *	2 PC style paths (backslashes)
		 * If you change or add to these options, you must also manually make 
		 * corresponding changes in the files AIRmain.h, and HEADERmain.h, 
		 * which are in the main AIR directory
		 */

#ifndef AIR_CONFIG_AUTO_BYTESWAP
#define AIR_CONFIG_AUTO_BYTESWAP 0	/* USER CONFIGURABLE LINE */
#endif
		/* If AIR_CONFIG_AUTO_BYTESWAP is non-zero, 
		 * opposite endian header and image files
		 * will be byte-swapped automatically allowing
		 * easy portability between PC's, Macs, Suns, IRIX, etc.
		 *
		 * If your machine doesn't support the IEEE Standard for
		 * Binary Floating-Point Arithmetic (ISO/IEEE Std 754-1985)
		 * you should probably set this to zero
		 */

#ifndef AIR_CONFIG_OUTBITS
#define AIR_CONFIG_OUTBITS 16	/* USER CONFIGURABLE LINE */
#endif
		/* Options for AIR_CONFIG_OUTBITS are 8 and 16
		 * Controls internal representation of data
		 * Controls bits/pixel of output data
		 * Data with a different number of bits/pixel can be input
		 * but will be converted to this number of bits/pixel internally				
		 */

#if(AIR_CONFIG_OUTBITS==16)
#ifndef AIR_CONFIG_REP16
#define AIR_CONFIG_REP16 1	/* USER CONFIGURABLE LINE */
#endif	
#endif
		/* AIR_CONFIG_REP16 is only relevant (and need only 
		 * be defined) when AIR_CONFIG_OUTBITS==16
		 * If 1, 16 bit data will be written to disk as
		 * 	unsigned short ints (NOT ANALYZE compatible)
		 *	header minimum will be 0, maximum 65535
		 * If 2, 16 bit data will be written to disk as
		 * 	short ints w/o using negative values
		 *	This effectively reduces storage to 15 bits.
		 *	The header minimum will be 0, maximum 32767.
		 * If 3, 16 bit data will be written to disk as
		 * 	short ints using negative values
		 *	an actual value of zero will be mapped to
		 * 	-32768 in short int representation
		 *	header minimum will be -32767
		 *	header maximum will be 32767 			
		 */

#if(AIR_CONFIG_OUTBITS==8)

#ifndef AIR_CONFIG_THRESHOLD1
#define AIR_CONFIG_THRESHOLD1 55	/* USER CONFIGURABLE LINE */
#endif

#ifndef AIR_CONFIG_THRESHOLD2
#define AIR_CONFIG_THRESHOLD2 55	/* USER CONFIGURABLE LINE */
#endif

#elif(AIR_CONFIG_OUTBITS==16)

#ifndef AIR_CONFIG_THRESHOLD1
#define AIR_CONFIG_THRESHOLD1 7000	/* USER CONFIGURABLE LINE */
#endif

#ifndef AIR_CONFIG_THRESHOLD2
#define AIR_CONFIG_THRESHOLD2 7000	/* USER CONFIGURABLE LINE */
#endif

#endif
		/* THRESHOLD1 and THRESHOLD2 are the default thresholds
		 * that AIR will offer when you register images.
		 *
		 * You probably want to use the same value for THRESHOLD1 and THRESHOLD2.
		 *
		 * You can always override the defaults, but it's nice to compile
		 * in some reasonable values if you can.
		 *
		 * Ideally, you should look at some typical data, and pick a default
		 * value that will generally separate brain from non-brain structures.
		 *
		 * In other words, pick a threshold such that values outside the structure
		 * of interest have pixel values below that threshold.
		 *
		 * An 8 bit value of 55 is good for PET data, but you'll probably 
		 * want something closer to 10 for 8 bit MRI data
		 *
		 * For 16 bit data, it's hard to guess what value will be reasonable
		 * since the full range is often not utilized. A value around
		 * 7000 is reasonable for PET data which is often effectively 15 positive bits.
		 * For MRI data, as little as 12 bits is commonly used, in which case a
		 * reasonable default threshold is probably in the 100-200 range. On the other
		 * hand, if all 16 bits are effectively utilized, 16 bit values in the 1000-2000 range
		 * may be more appropriate.
		 *
		 * Best bet--look at some data before choosing a default and caution users
		 * not to rely on the default values unless your site is very consistent
		 * about data formats and image intensities.
		 */

#ifndef AIR_CONFIG_VERBOSITY
#define AIR_CONFIG_VERBOSITY 0	/* USER CONFIGURABLE LINE */
#endif
		/* For AIR_CONFIG_VERBOSITY, 0 turns off nonessential screen printing
		 */

#ifndef AIR_CONFIG_PIX_SIZE_ERR
#define AIR_CONFIG_PIX_SIZE_ERR .0001	/* USER CONFIGURABLE LINE */
#endif
		/* Voxel sizes that differ by less than AIR_CONFIG_PIX_SIZE_ERR 
		 * are assumed identical
		 */

/* This section will depend on your compiler if you are not using a UNIX machine */

#ifndef AIR_CONFIG_REQ_PERMS
#define AIR_CONFIG_REQ_PERMS 0	/* USER CONFIGURABLE LINE */
#endif
		/* For AIR_CONFIG_REQ_PERMS:
		 * Use 2 if your compiler can use or fake use of UNIX stat() and lstat() functions
		 * Use 1 if your compiler can use or fake use of UNIX stat() function
		 * Use 0 if your compiler can't even fake UNIX stat() function
		 * Symptoms of not being able to use or fake such permissions would include
		 *  complaints about:
		 *		geteuid(),getegid(),stat(),lstat(),
		 *		sys/types.h, sys/stat.h, unistd.h,
		 *		ENOENT, ENOTDIR, EACCES, ELOOP, EMULTIHOP, ENOLINK
		 *		S_IFREG, S_IFDIR, S_IWUSR, S_IWGRP, S_IWOTH
		 *		buf, buf.st_mode, buf.st_uid, buf.st_gid
		 *
		 * It is always ok to use 0, but you will get more timely and more accurate
		 *  error messages if you can use 1 or 2.
		 */
#ifndef AIR_CONFIG_PIPES
#define AIR_CONFIG_PIPES 0	/* USER CONFIGURABLE LINE */
#endif
		/* A non-zero value for AIR_CONFIG_PIPES requires a system that supports
		 * non-ANSI/ISO features that are only available
		 * on certain UNIX systems. 
		 *
		 * Specifically, the C functions:
		 * 	FILE *popen(const char *command, const char *type)
		 * and
		 * 	int pclose(FILE *stream)
		 * must be supported.
		 */
		 
#if(AIR_CONFIG_PIPES==0)
#undef AIR_CONFIG_DECOMPRESS	/* DON'T CHANGE THIS--CAN'T DECOMPRESS WITHOUT PIPES */
#define AIR_CONFIG_DECOMPRESS 0 /* DON'T CHANGE THIS--CAN'T DECOMPRESS WITHOUT PIPES */
#else
#ifndef AIR_CONFIG_DECOMPRESS
#define AIR_CONFIG_DECOMPRESS 1	/* USER CONFIGURABLE LINE */
#endif
		/* AIR_CONFIG_DECOMPRESS is set to 0 if pipes are not available 
		 * (see AIR_CONFIG_PIPES above)
		 *
		 * A non-zero value indicates that a UNIX decompression utility
		 * exists that is capable of writing the decompressed result to
		 * standard out (which will then be piped into AIR)
		 *
		 * A value of zero disables decompression
		 */
#endif	/* end of if(AIR_CONFIG_PIPES==0) */

#if AIR_CONFIG_DECOMPRESS

#ifndef AIR_CONFIG_COMPRESSED_SUFFIX
#define AIR_CONFIG_COMPRESSED_SUFFIX ".gz"	/* USER CONFIGURABLE LINE */
#endif
		/* AIR_CONFIG_COMPRESSED_SUFFIX is ignored if pipes 
		 * are not available (see AIR_CONFIG_PIPES above)
		 *  or if decompression is disabled
		 *  (see AIR_CONFIG_DECOMPRESS above)
		 *
		 * default is ".gz", but other decompression utilities
		 * may use other suffixes (e.g., ".Z" is used by UNIX compress)
		 */
#ifndef AIR_CONFIG_DECOMPRESS_COMMAND
#define AIR_CONFIG_DECOMPRESS_COMMAND "gunzip -c "	/* USER CONFIGURABLE LINE */
#endif
		/* AIR_CONFIG_DECOMPRESS_COMMAND is ignored if pipes 
		 * are not available (see AIR_CONFIG_PIPES above) 
		 * or if decompression is disabled
		 * (see AIR_CONFIG_DECOMPRESS above)
		 *
		 * default is "gunzip -c " (note trailing space), but
		 * other decompression utilities can be substituted
		 * (e.g., "decompress -c ") as desired, making sure to
		 * configure AIR_CONFIG_COMPRESSED_SUFFIX appropriately
		 */
#endif	/* end of if AIR_CONFIG_DECOMPRESS */
                                


/* This section should be ok for all machines */

#if(AIR_CONFIG_OUTBITS==8)

typedef unsigned char AIR_Pixels;
#define AIR_CONFIG_MAX_POSS_VALUE UCHAR_MAX

/* The following assumes that sizeof(unsigned short int)==2 bytes on your machine */

#elif(AIR_CONFIG_OUTBITS==16)

typedef unsigned short int AIR_Pixels;
#define AIR_CONFIG_MAX_POSS_VALUE USHRT_MAX	/* This is true regardless of the value of REP16 above
				 * 16-bit data is always represented internally as
				 * unsigned short ints, even when it is written to
				 * output files in other formats
				 */
#endif


/* The remaining information should not generally require modification */

typedef int AIR_Boolean;
typedef int AIR_Error;

#define AIR_CONFIG_IMPOSSIBLE_THRESHOLD -32768	/* This must be kept negative, never positive */


#define AIR_CONFIG_HDR_SUFF ".hdr"		/* This suffix will be used in trying to open an image header*/
#define AIR_CONFIG_IMG_SUFF ".img"		/* This suffix will be used in trying to open an image file*/
#define AIR_CONFIG_NRM_SUFF ".nrm"		/* This suffix will be used in trying to open a normalization file */

/* Set up path separators appropriate for this platform */

#if(AIR_CONFIG_PATHS==1)
/* UNIX type paths */
#define AIR_CONFIG_PATH_SEP		'/'
#define AIR_CONFIG_PATH_SEP_NULL		"/\0"
#define AIR_CONFIG_PATH_REL_BASE		"./"

#elif(AIR_CONFIG_PATHS==2)
/* PC type paths */
#define AIR_CONFIG_PATH_SEP		'\\'
#define AIR_CONFIG_PATH_SEP_NULL		"\\\0"
#define AIR_CONFIG_PATH_REL_BASE		".\\"

#else
/* Unknown type paths */
#define AIR_CONFIG_PATH_SEP		'/'
#define AIR_CONFIG_PATH_SEP_NULL		"/\0"
#define AIR_CONFIG_PATH_REL_BASE		"./"
#endif


/* These are the internal structs
 * The external struct (for image headers as stored on disk)
 * is stored separately in HEADER.h 
 */

struct AIR_Fptrs{
	FILE *fp_img;
	FILE *fp_hdr;
	AIR_Error errcode;
	AIR_Boolean header_was_compressed;
	AIR_Boolean image_was_compressed;
};

struct AIR_Key_info{
        unsigned int bits;
        unsigned int x_dim;
        unsigned int y_dim;
        unsigned int z_dim;
        double x_size;
        double y_size;
        double z_size;
};

/* Note that changing the maximum lengths below will change the size of various
 *  structs producing files that are not compatible with those created using other values
 *
 * Also note that it is generally assumed throughout the code that r_file and s_file
 *  lengths are uniform across various structs. Changing the length in one struct independently
 *  of another will probably lead to bad problems since values are often copied from one 
 *  type of struct to another on the assumption that adequate space is guaranteed
 *
 */

#define AIR_CONFIG_MAX_PATH_LENGTH 128			/* Default is 128--changing this value makes .air and .warp files */
#define AIR_CONFIG_MAX_COMMENT_LENGTH 128		/* incompatible with versions that use the default		  */
#define AIR_CONFIG_RESERVED_LENGTH 116			/* Default is 116 */

struct AIR_Air16{
	double  			e[4][4];
	char    			s_file[AIR_CONFIG_MAX_PATH_LENGTH];
	struct AIR_Key_info	s;
	char    			r_file[AIR_CONFIG_MAX_PATH_LENGTH];
	struct AIR_Key_info	r;
	char    			comment[AIR_CONFIG_MAX_COMMENT_LENGTH];
	unsigned long int	s_hash;
	unsigned long int	r_hash;
	unsigned short		s_volume;	/* Not used in this version of AIR */
	unsigned short		r_volume;	/* Not used in this version of AIR */
	char				reserved[AIR_CONFIG_RESERVED_LENGTH];
};

struct AIR_Oldair{					/* Allows AIR 2.0 and later to read AIR 1.0 .air files*/
	double                  e[4][3];
	char                    s_file[128];
	struct AIR_Key_info		s;
	char                    r_file[128];
	struct AIR_Key_info		r;
	char                    comment[128];
	unsigned long int       s_hash;
	unsigned long int       r_hash;
	unsigned short			s_volume;	/* Not used in this version of AIR */
	unsigned short			r_volume;	/* Not used in this version of AIR */
	char                    reserved[116];
};

#define AIR_CONFIG_MAXORDER 12			/* Maximum order for nonlinear warps */

struct AIR_Field{
	unsigned int			coord;
	char                    s_file[AIR_CONFIG_MAX_PATH_LENGTH];
	char					pad[4];		/* Added to improve cross platform consistency of struct alignment */
	struct AIR_Key_info		s;
	char                    r_file[AIR_CONFIG_MAX_PATH_LENGTH];
	struct AIR_Key_info		r;
	char                    comment[AIR_CONFIG_MAX_COMMENT_LENGTH];
	unsigned long int       s_hash;
	unsigned long int       r_hash;
	unsigned short			s_volume;	/* Not used in this version of AIR */
	unsigned short			r_volume;	/* Not used in this version of AIR */
	char                    reserved[AIR_CONFIG_RESERVED_LENGTH];
};

struct AIR_Warp{
	unsigned int			order;
	unsigned int			coeffp;
	unsigned int			coord;
	char                    s_file[AIR_CONFIG_MAX_PATH_LENGTH];
	char					pad[4];		/* Added to improve cross platform consistency of struct alignment */
	struct AIR_Key_info		s;
	char                    r_file[AIR_CONFIG_MAX_PATH_LENGTH];
	struct AIR_Key_info		r;
	char                    comment[AIR_CONFIG_MAX_COMMENT_LENGTH];
	unsigned long int       s_hash;
	unsigned long int       r_hash;
	unsigned short			s_volume;	/* Not used in this version of AIR */
	unsigned short			r_volume;	/* Not used in this version of AIR */
	char                    reserved[AIR_CONFIG_RESERVED_LENGTH];
};

struct AIR_Air168{					/* Allows AIR 4.0 and later to read AIR 3.0 .warp files */
	double                  e[56][4];
	char                    s_file[128];
	struct AIR_Key_info		s;
	char                    r_file[128];
	struct AIR_Key_info		r;
	char                    comment[128];
	unsigned long int       s_hash;
	unsigned long int       r_hash;
	unsigned short			s_volume;	/* Not used in this version of AIR */
	unsigned short			r_volume;	/* Not used in this version of AIR */
	char                    reserved[116];
};

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

/* Function prototypes */

double **AIR_align(const unsigned int, double *, void (*uvfN )(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean ), const unsigned int, double (*uvderivsN )(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *), AIR_Pixels ***, /*@null@*/ AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, AIR_Pixels ***, /*@null@*/ AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, unsigned int, const unsigned int, unsigned int, const double, const unsigned int, const unsigned int, const AIR_Boolean, const unsigned int, const unsigned int, const AIR_Boolean, const AIR_Boolean, const unsigned int, const unsigned int, const unsigned int, const AIR_Boolean, const AIR_Boolean, const AIR_Boolean, /*@out@*/AIR_Error *);

double **AIR_alignwarp(double *, double (*)(const unsigned int, double **, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, double *, const double, unsigned int *), AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, unsigned int, const unsigned int, unsigned int, const double, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const AIR_Boolean, const AIR_Boolean, /*@out@*/ AIR_Error *);

AIR_Error AIR_antiwarp2D(double **, const double, const double, double *, double *, const unsigned int, double *, double *, double *, const double);

AIR_Error AIR_antiwarp3D(double **, const double, const double, const double, double *, double *, double *, const unsigned int, double *, double *, double *, double *, const double);

void AIR_balanc(const unsigned int, double **, /*@out@*/ unsigned int *, /*@out@*/ unsigned int *, /*@out@*/ double *);

void AIR_balbak(const unsigned int, const unsigned int, const unsigned int, const double *, const unsigned int, double **);

AIR_Error AIR_blat40(double **, double **, const unsigned int);

AIR_Error AIR_blat100(double **, double **, const unsigned int);

AIR_Error AIR_blat400(double **, double **, const unsigned int);

AIR_Error AIR_blat1000(double **, double **, const unsigned int);

void AIR_caxpy(const unsigned int, const double, const double, const double *, const double *, double *, double *);

AIR_Error AIR_cdiver(double, double, double, double, /*@out@*/ double *, /*@out@*/ double *);

void AIR_cdotc(const unsigned int, const double *, const double *, const double *, const double *, /*@out@*/ double *, /*@out@*/ double *);

AIR_Error AIR_cdvasb(const unsigned int, const unsigned int, double **, double **, double **, double **, double *, double *, unsigned int *);

unsigned int AIR_cgefa(double **, double **, const unsigned int, unsigned int *);

void AIR_cexp(const double, const double, double *, double *);

void AIR_cffti1(const unsigned int, double *, unsigned int *);

void AIR_cfftb1(const unsigned int, double *, double *, const double *, const unsigned int *);

void AIR_cfftf1(const unsigned int, double *, double *, const double *, const unsigned int *);

void AIR_cgemm(const char, const char, const unsigned int, const unsigned int, const unsigned int, const double, const double, double **, double **, double **, double **, const double, const double, double **, double **);

void AIR_cgesl(double **, double **, const unsigned int, const unsigned int *, double *, double *, const AIR_Boolean);

void AIR_chirpterp(const unsigned int, double *, const unsigned int, double *, const unsigned int, double *, double *, double *, double *, double *, double *, unsigned int *, unsigned int *, const double, const double, const unsigned int, AIR_Boolean *);

/*@null@*/ AIR_Pixels ***AIR_classify(AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, /*@out@*/ AIR_Error *);

void AIR_clip_suffix(char *);

AIR_Error AIR_clog(double, double, double *, double *);

void AIR_close_header(struct AIR_Fptrs *);

AIR_Error AIR_combineair(const struct AIR_Air16 *, const struct AIR_Air16 *, struct AIR_Air16 *);

AIR_Error AIR_combinewarp_left(const struct AIR_Warp, double **, struct AIR_Warp *, double ***);

int AIR_combinewarp_right(struct AIR_Warp *, double ***, const struct AIR_Warp, double **);

AIR_Error AIR_comloger(const AIR_Boolean, double ***, const unsigned int, const double *, const double *, double ***, double ***);

AIR_Error AIR_comtloger(const AIR_Boolean, double ***, const unsigned int, double **, const double, const unsigned int, double *, const double *, const double *, double ***, double ***);

float AIR_consult_header(const char *, struct AIR_Fptrs *, struct AIR_Key_info *, const unsigned int, int *);

void AIR_cpow(double, double, const double, double *, double *);

void AIR_cscal(const unsigned int, const double, const double, double *, double *);

double AIR_csqfield(const double, const double, const double, const double, const double, const double , const double , const double , /*@out@*/ AIR_Error *);

void AIR_csqrt(const double, const double, double *, double *);

double AIR_d1mach(const unsigned int);

void AIR_daxpy(const unsigned int, const double, const double *, double *);

double AIR_dbetai(const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_dcsevl(const double, const double *, const unsigned int, /*@out@*/ AIR_Error *);

double AIR_dcsqfield(const double, const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_dcsq0field(const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_ddet(double **, const unsigned int, unsigned int *);

double AIR_ddot(const unsigned int, const double *, const double *);

double AIR_dffield(const double, const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_df0field(const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_dgamic(const double, const double, int *);

AIR_Error AIR_dgamlm(double *, double *);

double AIR_dgamma(const double, /*@out@*/ AIR_Error *);

double AIR_dgamr(const double, /*@out@*/ AIR_Error *);

void AIR_dgedi(double **, const unsigned int, const unsigned int *, double *);

unsigned int AIR_dgefa(double **, const unsigned int, unsigned int *);

void AIR_dgemm(const char, const char, const unsigned int, const unsigned int, const unsigned int, const double, double **, double **, const double, /*@out@*/ double **);

void AIR_dgesl(double **, const unsigned int, const unsigned int *, double *, const AIR_Boolean);

AIR_Error AIR_diffg2D(double **, const double, const double, double **, const unsigned int, double *, double *);

AIR_Error AIR_diffg3D(double **, const double, const double, const double, double **, const unsigned int, double *, double *, double *);

double AIR_dlbeta(const double, const double, /*@out@*/ AIR_Error *);

double AIR_dlfp2c(const double, const double, const double, /*@out@*/ AIR_Error *);

AIR_Error AIR_dlgams(const double, double *, double *);

double AIR_dlngam(const double, int *);

double AIR_dlnrel(const double, /*@out@*/ AIR_Error *);

AIR_Error AIR_dlogm(const unsigned int, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, const double *, const double *, double ***, double ***, double**);

double AIR_dltp2c(const double, /*@out@*/ AIR_Error *);

void AIR_dmodposl(double **, const unsigned int, double *, const unsigned int *);

double AIR_dnrm2(const unsigned int, const double *);

AIR_Error AIR_do_alignlinear(const char *, const unsigned int, const char *, const AIR_Boolean, const char *, long int, const float, const float, const float, const unsigned int, const AIR_Boolean, /*@null@*/ const char *, const char *, long int, const float, const float, const float, const unsigned int, const AIR_Boolean, /*@null@*/ const char *, /*@null@*/ const char *, /*@null@*/ const char *, /*@null@*/ const char *, const AIR_Boolean, /*@null@*/ const char *, const AIR_Boolean, const unsigned int, const unsigned int, const unsigned int, const float, const unsigned int, const unsigned int, const AIR_Boolean, const AIR_Boolean, const AIR_Boolean, const unsigned int, const AIR_Boolean);

AIR_Error AIR_do_align_warp(const char *, const unsigned int, const unsigned int, const char *, const AIR_Boolean, const char *, long int, const float, const float, const float, /*@null@*/ const char *, const char *, long int, const float, const float, const float, /*@null@*/ const char *, /*@null@*/ const char *, /*@null@*/ const char *, /*@null@*/ const char *, const AIR_Boolean, const unsigned int, const unsigned int, const unsigned int, const float, const unsigned int, const unsigned int, const AIR_Boolean, const AIR_Boolean, const AIR_Boolean, const AIR_Boolean);

AIR_Error AIR_do_binarize(const char *, const char *, const char *, const AIR_Boolean, long int, AIR_Boolean, long int, const AIR_Boolean);

AIR_Error AIR_do_binarymask(const char *, const char *, const char *, const char *, const AIR_Boolean);

AIR_Error AIR_do_binarymath(const char *, const char *, const char *, const char *, const char *, const AIR_Boolean);

AIR_Error AIR_do_cd_air(const char *, const char *, const char *);

AIR_Error AIR_do_cd_warp(const char *, const char *, const char *);

AIR_Error AIR_do_combine_air(const char *, const unsigned int, char **, const char *, const AIR_Boolean);

AIR_Error AIR_do_combine_warp(const char *, const char *, const unsigned int, char **, const AIR_Boolean);

AIR_Error AIR_do_crop(const char *, const char *, const char *, /*@null@*/ const char *, const unsigned int, const AIR_Boolean);

AIR_Error AIR_do_definecommon_air(const char *, const char *, const char *, const char *, const char *, const unsigned int, char **, const AIR_Boolean);

AIR_Error AIR_do_definecommon_scale(const char *, const char *, const char *, const char *, const unsigned int, char **, const AIR_Boolean);

AIR_Error AIR_do_determinant_mask(const char *, const char *, const char *, const AIR_Boolean);

AIR_Error AIR_do_determinant_warp(const char *, const char *, const char *, const double, const double, const AIR_Boolean);

AIR_Error AIR_do_extractmovements(const char *, const char *, const char *, const char *, const unsigned int, char **, const unsigned int, const AIR_Boolean);

AIR_Error AIR_do_fixheader(const char *, const AIR_Boolean, const double, const double, const double);

AIR_Error AIR_do_fuse(const char *, const char *, const unsigned int, const unsigned int, const unsigned int, char **, const AIR_Boolean);

AIR_Error AIR_do_gsmooth(const char *, const char *, const char *, const AIR_Boolean, const AIR_Boolean, const float, const float, const float, /*@null@*/ const char *, const AIR_Boolean, const AIR_Boolean, const AIR_Boolean);

AIR_Error AIR_do_identify(const char *);

AIR_Error AIR_do_invert_air(const char *, const char *, const char *, const AIR_Boolean);

AIR_Error AIR_do_layout(const char *, const char *, const char *, const char, const unsigned int, const unsigned int, const unsigned int, const signed int, const AIR_Boolean);

AIR_Error AIR_do_magnify(const char *, const char *, const char *, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const AIR_Boolean);

AIR_Error AIR_do_makeaheader(const char *, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const double, const double, const double, const AIR_Boolean);

AIR_Error AIR_do_mv_air(const char *, const char *);

AIR_Error AIR_do_mv_warp(const char *, const char *);

AIR_Error AIR_do_overlay_mask(const char *, const char *, const char *, const char *, const long int, const AIR_Boolean);

AIR_Error AIR_do_reconcile_air(const char *, const unsigned int, const char *, const char *, const char *, const char *, const char *, const unsigned int, char **, const AIR_Boolean);

AIR_Error AIR_do_removeconfounds(const char *, const unsigned int, char **, const char *, const char *, const char *, const char *, const unsigned int, char **, const AIR_Boolean);

AIR_Error AIR_do_reorient(const char *, const char *, const char *, const unsigned int, const unsigned int *, /*@null@*/ const char *, const AIR_Boolean);

AIR_Error AIR_do_resize(const char *, const char *, const char *, /*@null@*/ const char *, const unsigned int, const unsigned int, const unsigned int, const signed int, const signed int, const signed int, const AIR_Boolean);

AIR_Error AIR_do_reslice(const char *, const char *, const char *, /*@null@*/ const char *, const unsigned int, const unsigned int *, const AIR_Boolean, double, /*@null@*/ const char *, /*@null@*/ const char *, const AIR_Boolean, const unsigned int, const float, const float, const unsigned int, const float, const float, const unsigned int, const float, const float);

AIR_Error AIR_do_reslice_ucf(const char *, const char *, const char *, AIR_Boolean);

AIR_Error AIR_do_reslice_unwarp(const char *, const char *, /*@null@*/ const char *, const char *, const unsigned int, const unsigned int *, double, /*@null@*/ const char *, /*@null@*/ const char *, const signed int, double, /*@null@*/ const char *, const AIR_Boolean);

AIR_Error AIR_do_reslice_unwarp_ucf(const char *, const char *, const char *, const signed int, AIR_Boolean, AIR_Boolean);

AIR_Error AIR_do_reslice_vector(const char *, const char *, const char *, /*@null@*/ const char *, /*@null@*/ const char *, const unsigned int, const unsigned int *, double, /*@null@*/ const char *, /*@null@*/ const char *, const AIR_Boolean);

AIR_Error AIR_do_reslice_warp(const char *, const char *, /*@null@*/ const char *, const char *, const unsigned int, const unsigned int *, double, /*@null@*/ const char *, /*@null@*/ const char *, const AIR_Boolean);

AIR_Error AIR_do_reslice_warp_ucf(const char *, const char *, const char *, const signed int, double, AIR_Boolean, const char *, AIR_Boolean);

AIR_Error AIR_do_reunite(const char *, const char *, const unsigned int, char **, const AIR_Boolean);

AIR_Error AIR_do_separate(const char *, const char *, const char *, const AIR_Boolean);

AIR_Error AIR_do_shade(const char *, const char *, const char *, const double, const double, const double, const AIR_Boolean);

AIR_Error AIR_do_softmean(const char *, const char *, const unsigned int, char **, /*@null@*/ const char *, const AIR_Boolean);

AIR_Error AIR_do_softmeannorescale(const char *, const char *, const unsigned int, char **, /*@null@*/ const char *, const AIR_Boolean);

AIR_Error AIR_do_stereoproject(const char *, const char *, /*@null@*/ const char *, /*@null@*/ const char *, const char *, const long int, const double, const AIR_Boolean);

AIR_Error AIR_do_straightmean(const char *, const char *, const unsigned int, char **, /*@null@*/ const char *, const AIR_Boolean);

AIR_Error AIR_do_straightmeannorescale(const char *, const char *, const unsigned int, char **, /*@null@*/ const char *, const AIR_Boolean);

AIR_Error AIR_do_strictmean(const char *, const char *, const unsigned int, char **, /*@null@*/ const char *, const AIR_Boolean);

AIR_Error AIR_do_strictmeannorescale(const char *, const char *, const unsigned int, char **, /*@null@*/ const char *, const AIR_Boolean);

AIR_Error AIR_do_zoomer(/*@unique@*/ const char *, /*@unique@*/ const char *, /*@unique@*/ const char *, /*@null@*/ const char *, const AIR_Boolean, const AIR_Boolean, const AIR_Boolean);

unsigned int AIR_dpofa(double **, const unsigned int);

void AIR_dposl(double **, const unsigned int, double *);

void AIR_dqk15(double (*)(const double), const double, const double, double *, double *, double *, double *);

void AIR_dqpsrt(const unsigned int, const unsigned int, unsigned int *, double *, const double *, unsigned int *, unsigned int *);

double AIR_dqage(double (*)(const double), const double, const double, const double, const double, const unsigned int, double *, unsigned int *, unsigned int *, double *, double *, double *, double *, unsigned int *, unsigned int *);

void AIR_drot(const unsigned int, double *, double *, const double, const double);

void AIR_drotg(double *, double *, /*@out@*/ double *, /*@out@*/ double *);

void AIR_dscal(const unsigned int, const double, double *);

void AIR_dsvdc(double **, const unsigned int, const unsigned int, double *, double *, double **, double **, double *, const unsigned int, const AIR_Boolean, /*@out@*/ unsigned int *);

void AIR_dswap(const unsigned int, double *, double *);

double AIR_dtfield(const double, const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_dt0field(const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_dt2field(const double, const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_dt20field(const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

AIR_Error AIR_dvasb(const unsigned int, const unsigned int, double **, double **, double *, unsigned int *);

double AIR_dzfield(const double, const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_dz0field(const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_d9gmic(const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_d9gmit(const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_d9lgic(const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_d9lgmc(const double, /*@out@*/ AIR_Error *);

double AIR_d9lgit(const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_ecsqfield(const double, const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_ecsq0field(const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

AIR_Error AIR_eexper4(const unsigned int, double **, const double *, const double *, double ***, double ***);

double AIR_effield(const double, const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_ef0field(const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

AIR_Error AIR_eloger4(const unsigned int, double **, const double *, const double *, double ***, double ***);

AIR_Error AIR_eloger5(double **, const double *, const double *, double ***, double ***);

double AIR_etfield(const double, const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_et0field(const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_et2field(const double, const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_et20field(const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

AIR_Error AIR_exchng(double **, double **, const unsigned int, const unsigned int, const unsigned int, const unsigned int);

AIR_Error AIR_expderivut(const unsigned int, const unsigned int, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, const double *, const double *, double ***);

AIR_Error AIR_exput(const unsigned int, double **, double **, double **, double **, const double *, const double *, double ***);

AIR_Error AIR_extract_global_shape(const unsigned int, const unsigned int, const unsigned int, const unsigned int, /*@out@*/ unsigned int *, struct AIR_Warp *, double ***, double ***, double **, unsigned int *, unsigned int *, unsigned int *, double *, double *, double ***, double ***);

double AIR_ezfield(const double, const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_ez0field(const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

AIR_Error AIR_fclose_decompress(FILE *, const AIR_Boolean);

double AIR_ffield(const double, const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

const char *AIR_file_from_path(/*@returned@*/ const char *);

double AIR_findcrit(double (*)(), double (*)(), double (*)(), double (*)(), double (*)(), const double, const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

AIR_Pixels ***AIR_flip_x(AIR_Pixels ***, const struct AIR_Key_info *, /*@out@*/ AIR_Error *);

AIR_Pixels ***AIR_flip_y(AIR_Pixels ***, const struct AIR_Key_info *, /*@out@*/ AIR_Error *);

AIR_Pixels ***AIR_flip_z(AIR_Pixels ***, const struct AIR_Key_info *, /*@out@*/ AIR_Error *);

FILE *AIR_fopen_decompress(const char *, AIR_Boolean *, /*@out@*/ AIR_Error *);

void AIR_fprob_errs(const AIR_Error, char *, char *);

AIR_Error AIR_fprobr(const char *);

AIR_Error AIR_fprobw(const char *, const AIR_Boolean);

AIR_Error AIR_gael(double **);

AIR_Pixels ***AIR_gausser(AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, float, float, float, const AIR_Boolean, float *, const AIR_Boolean, const AIR_Boolean, const AIR_Boolean, /*@out@*/ AIR_Error *);

AIR_Error AIR_gausssmaller(AIR_Pixels ***, const struct AIR_Key_info *, float, float, float);

double AIR_genrand(unsigned long mt[]); /* Size is 624 */

unsigned long int AIR_hash(AIR_Pixels ***, const struct AIR_Key_info *);

double AIR_hintbell(const double, /*@out@*/ AIR_Error *);

void AIR_hqr(const unsigned int, const unsigned int, const unsigned int, double **, double *, double *, /*@out@*/ unsigned int *);

void AIR_hqr2(const unsigned int, const unsigned int, const unsigned int, double **, double *, double *, double **, /*@out@*/ unsigned int *);

void AIR_hqr3(const unsigned int, const unsigned int, const unsigned int, double **, double *, double *, double **, /*@out@*/ unsigned int *);

unsigned int AIR_icamax(const unsigned int, const double *, const double *);

unsigned int AIR_idamax(const unsigned int, const double *);

AIR_Boolean AIR_identical(AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *);

unsigned int AIR_initds(const double *, const unsigned int, const double);

AIR_Pixels AIR_interp_lin_2D(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const unsigned int, const double, const unsigned int *, const double);

AIR_Pixels AIR_interp_lin_3D(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const double, const double, const unsigned int *, const double);

AIR_Pixels AIR_interp_nn_2D(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const unsigned int, const double, const unsigned int *, const double);

AIR_Pixels AIR_interp_nn_3D(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const double, const double, const unsigned int *, const double);

AIR_Pixels AIR_interp_wsinc_2D(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const unsigned int, const double, const unsigned int *, const double);

AIR_Pixels AIR_interp_wsinc_3D(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const double, const double, const unsigned int *, const double);

AIR_Pixels AIR_interp_wsinc_xy(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const double, const double, const unsigned int *, const double);

AIR_Pixels AIR_interp_wsinc_xz(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const double, const double, const unsigned int *, const double);

AIR_Pixels AIR_interp_wsinc_yz(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const double, const double, const unsigned int *, const double);

AIR_Error AIR_inverter(double **, double **, const AIR_Boolean, const struct AIR_Key_info *, const struct AIR_Key_info *);

AIR_Pixels ***AIR_layerout(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, const unsigned int, const unsigned int, const unsigned int, const signed int, /*@out@*/ AIR_Error *);

AIR_Pixels ***AIR_load(const char *, /*@out@*/ struct AIR_Key_info *, const AIR_Boolean, /*@out@*/ AIR_Error *);

AIR_Pixels ***AIR_load1(const char *, struct AIR_Key_info *, /*@out@*/ AIR_Error *);

unsigned char *AIR_load1as1(const char *, struct AIR_Key_info *, /*@out@*/ AIR_Error *);

AIR_Error AIR_load_probr(const char *, const AIR_Boolean);

AIR_Error AIR_logderivut(const unsigned int, const unsigned int, double **, double **, double **, double **, double **, double **, double **, double **, double **, double **, const double *, const double *, double ***);

int AIR_logut(const unsigned int, double **, double **, double **, double **, const double *, const double *, double ***);

AIR_Pixels ***AIR_magnifier(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, /*@out@*/ AIR_Error *);

AIR_Pixels AIR_map_value(const char *, long int, /*@out@*/ AIR_Error *);

AIR_Error AIR_mask(AIR_Pixels ***, struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Boolean);

void AIR_matmul(double **, double **, /*@out@*/ double **);

AIR_Error AIR_matrixmean(double ***, double ***, const unsigned int, double **, const double, const unsigned int, const double *, const double *, double ***, double ***, double ***, double ***);

AIR_Pixels AIR_maxvoxel(AIR_Pixels ***, const struct AIR_Key_info *);

AIR_Error AIR_meancalc(AIR_Pixels **, AIR_Pixels ***, const double *, const unsigned int, const signed int, const AIR_Boolean, const unsigned int, const unsigned int, const double, double *);

AIR_Pixels ***AIR_meaner(struct AIR_Key_info *, double *, const signed int, const AIR_Boolean, const char *, const AIR_Boolean, char **, const unsigned int, /*@out@*/ AIR_Error *);

unsigned int AIR_modchol(double **, const unsigned int, /*@out@*/ unsigned int *, /*@out@*/ double *);

void AIR_mulmat(double **, const unsigned int, const unsigned int, double **, const unsigned int, double **);

float AIR_open_header(const char *, /*@out@*/ struct AIR_Fptrs *, /*@out@*/ struct AIR_Key_info *, int *);

void AIR_orthes(const unsigned int, const unsigned int, const unsigned int, double **, double *);

void AIR_ortran(const unsigned int, const unsigned int, const unsigned int, double **, double *, double **);

AIR_Error AIR_padetanh(double *, double *);

signed char AIR_parse_schar(const char *, /*@out@*/ AIR_Error *);

double AIR_parse_double(const char *, /*@out@*/ AIR_Error *);

char **AIR_parse_file_list(const char *, /*@out@*/ unsigned int *, /*@out@*/ AIR_Error *);

float AIR_parse_float(const char *, /*@out@*/ AIR_Error *);

int AIR_parse_int(const char *, /*@out@*/ AIR_Error *);

long int AIR_parse_long(const char *, /*@out@*/ AIR_Error *);

short int AIR_parse_short(const char *, /*@out@*/ AIR_Error *);

unsigned char AIR_parse_uchar(const char *, /*@out@*/ AIR_Error *);

unsigned int AIR_parse_uint(const char *, /*@out@*/ AIR_Error *);

unsigned long int AIR_parse_ulong(const char *, /*@out@*/ AIR_Error *);

unsigned short int AIR_parse_ushort(const char *, /*@out@*/ AIR_Error *);

void AIR_passb2(const unsigned int, const unsigned int, double *, double *, const double *);

void AIR_passb3(const unsigned int, const unsigned int, double *, double *, const double *, const double *);

void AIR_passb4(const unsigned int, const unsigned int, double *, double *, const double *, const double *, const double *);

void AIR_passb5(const unsigned int, const unsigned int, double *, double *, const double *, const double *, const double *, const double *);

void AIR_passb(/*@out@*/ unsigned int *, const unsigned int, const unsigned int, const unsigned int, const unsigned int, double *, double *, double *, double *, double *, const double *);

void AIR_passf2(const unsigned int, const unsigned int, double *, double *, const double *);

void AIR_passf3(const unsigned int, const unsigned int, double *, double *, const double *, const double *);

void AIR_passf4(const unsigned int, const unsigned int, double *, double *, const double *, const double *, const double *);

void AIR_passf5(const unsigned int, const unsigned int, double *, double *, const double *, const double *, const double *, const double *);

void AIR_passf(/*@out@*/ unsigned int *, const unsigned int, const unsigned int, const unsigned int, const unsigned int, double *, double *, double *, double *, double *, const double *);

void AIR_permute(const unsigned int, unsigned int *, unsigned long mt[]); /* Size is 624 */

unsigned int AIR_pfactor(const unsigned int);

AIR_Pixels ***AIR_projslicex(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const AIR_Pixels, AIR_Pixels ***, /*@out@*/ AIR_Error *);

AIR_Error AIR_proj2comm(const unsigned int, double ***, double **, struct AIR_Warp *, double, double, double, double *, double *, double *, double **, double ***, double ***, double *, double *, double *, double *, double *, double *, double ***, double ***, double ***, double ***);

double **AIR_promote(const unsigned int, double **, /*@out@*/ AIR_Error *);

void AIR_qrstep(double **, double **, double, double, double, const unsigned int, const unsigned int, const unsigned int);

AIR_Error AIR_quickread(AIR_Pixels *, const char *, const unsigned int);

AIR_Error AIR_quickwrite(AIR_Pixels **, const struct AIR_Key_info *, const char *, const AIR_Boolean, const char *, const unsigned int);

double AIR_qvderiv2Dwarp(const unsigned int, double **, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, double *, const double, unsigned int *);

double AIR_qvderiv3Dwarp(const unsigned int, double **, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, double *, const double, unsigned int *);

double AIR_qvderivsLS12(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_qvderivsLS15(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_qvderivsLS6(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_qvderivsLS8(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_qvderivsN12(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_qvderivsN15(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_qvderivsN6(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_qvderivsN8(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_qvderivsRS12(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_qvderivsRS15(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_qvderivsRS6(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_qvderivsRS8(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

void AIR_radb2(const unsigned int, const unsigned int, double *, double *, const double *);

void AIR_radb3(const unsigned int, const unsigned int, double *, double *, const double *, const double *);

void AIR_radb4(const unsigned int, const unsigned int, double *, double *, const double *, const double *, const double *);

void AIR_radb5(const unsigned int, const unsigned int, double *, double *, const double *, const double *, const double *, const double *);

void AIR_radbg(const unsigned int, const unsigned int, const unsigned int, const unsigned int, double *, double *, double *, double *, double *, const double *);

void AIR_radf2(const unsigned int, const unsigned int, double *, double *, const double *);

void AIR_radf3(const unsigned int, const unsigned int, double *, double *, const double *, const double *);

void AIR_radf4(const unsigned int, const unsigned int, double *, double *, const double *, const double *, const double *);

void AIR_radf5(const unsigned int, const unsigned int, double *, double *, const double *, const double *, const double *, const double *);

void AIR_radfg(const unsigned int, const unsigned int, const unsigned int, const unsigned int, double *, double *, double *, double *, double *, const double *);

void AIR_rank(const unsigned long int n, double *values, unsigned long int *rank);

AIR_Error AIR_read_air16(const char *, /*@out@*/ struct AIR_Air16 *);

AIR_Error AIR_read_air16via_airw(const char *, /*@out@*/ struct AIR_Air16 *);

AIR_Error AIR_read_airfield(const char *, struct AIR_Field *, float *****, float ****);

double **AIR_read_airw(const char *, /*@out@*/ struct AIR_Warp *, /*@out@*/ AIR_Error *);

AIR_Error AIR_read_image(AIR_Pixels *, FILE **, const struct AIR_Key_info *, const int *, const float);

AIR_Error AIR_read_nrm(const char *, /*@out@*/ unsigned long int *, /*@out@*/ unsigned long int *, /*@out@*/ unsigned long int *);

double AIR_read_value(const char *, const char *, /*@out@*/ AIR_Error *);

AIR_Pixels ***AIR_resizer(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, const unsigned int, const unsigned int, const unsigned int, const signed int, const signed int, const signed int, /*@out@*/ AIR_Error *);

void AIR_report_error(const AIR_Error);

void AIR_rfftb1(const unsigned int, double *, double *, const double *, const unsigned int *);

void AIR_rfftf1(const unsigned int, double *, double *, const double *, const unsigned int *);

void AIR_rffti1(const unsigned int, double *, unsigned int *);

void AIR_rg(const unsigned int, double **, double *, double *, double **, double *, double *, /*@out@*/ unsigned int *);

void AIR_rgschur(const unsigned int, double **, double *, double *, double **, double *, /*@out@*/ unsigned int *);

void AIR_rsftocsf(const unsigned int, double **, double **, double **);

/*@null@*/ AIR_Pixels ***AIR_r_affine_lin(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_persp_lin(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_persp_nn(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_persp_wsinc(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, const unsigned int, const unsigned int, const unsigned int, /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_persp_wsinc_xy(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, const unsigned int, const unsigned int, /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_persp_wsinc_xz(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, const unsigned int, const unsigned int, /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_persp_wsinc_yz(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, const unsigned int, const unsigned int, /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_scan_chirp(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_scan_chirp_xy(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_scan_chirp_xz(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_scan_chirp_yz(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_scan_sinc(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_scan_wsinc(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, const unsigned int, const unsigned int, const unsigned int, /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_warp_2D(AIR_Pixels ***, const struct AIR_Key_info *, const struct AIR_Key_info *, double **, const double, const unsigned int, const unsigned int *, AIR_Pixels (*)(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const unsigned int, const double, const unsigned int *, const double), /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_warp_3D(AIR_Pixels ***, const struct AIR_Key_info *, const struct AIR_Key_info *, double **, const double, const unsigned int, const unsigned int *, AIR_Pixels (*)(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const double, const double, const unsigned int *, const double), /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_uwrp_2D(AIR_Pixels ***, const struct AIR_Key_info *, const struct AIR_Key_info *, double **, const double, const unsigned int, double **, const signed int, const double, const unsigned int *, AIR_Pixels (*)(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const unsigned int, const double, const unsigned int *, const double), /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_uwrp_3D(AIR_Pixels ***, const struct AIR_Key_info *, const struct AIR_Key_info *, double **, const double, const unsigned int, double **, const signed int, const double, const unsigned int *, AIR_Pixels (*)(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const double, const double, const unsigned int *, const double), /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_vec_warp_2D(AIR_Pixels ***, const struct AIR_Key_info *, const struct AIR_Key_info *, float ***, double **, const double, const unsigned int, const unsigned int *, AIR_Pixels (*)(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const unsigned int, const double, const unsigned int *, const double), /*@out@*/ AIR_Error *);

/*@null@*/ AIR_Pixels ***AIR_r_vec_warp_3D(AIR_Pixels ***, const struct AIR_Key_info *, const struct AIR_Key_info *, float ****, double **, const double, const unsigned int, const unsigned int *, AIR_Pixels (*)(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const double, const double, const unsigned int *, const double), /*@out@*/ AIR_Error *);

AIR_Error AIR_same_dim(const struct AIR_Key_info *, const struct AIR_Key_info *);

AIR_Error AIR_same_size(const struct AIR_Key_info *, const struct AIR_Key_info *);

AIR_Error AIR_save(AIR_Pixels ***, const struct AIR_Key_info *, const char *, const AIR_Boolean, const char *);

AIR_Error AIR_saveas1(AIR_Pixels ***, const struct AIR_Key_info *, const char *, const AIR_Boolean, const AIR_Pixels, const AIR_Pixels, const char *);

AIR_Error AIR_save1as1(const unsigned char *, const struct AIR_Key_info *, const char *, const AIR_Boolean, const char *);

AIR_Error AIR_save_probw(const char *, const AIR_Boolean);

double AIR_sdpred(const double *, const double *, double **, const unsigned int);

void AIR_sgenrand(unsigned long, unsigned long mt[]); /* Size is 624 */

AIR_Error AIR_shader(AIR_Pixels ***, const struct AIR_Key_info *, const double, const double, const double);

AIR_Error AIR_sizeregion(AIR_Pixels ***, struct AIR_Key_info *, double *, double *, double *, double *, const double, const double, const double);

void AIR_sqrtut(const unsigned int, double **, double **, double **, double **);

void AIR_sumroi(AIR_Pixels ***, const struct AIR_Key_info *, unsigned long int *, unsigned long int *, unsigned long int *, AIR_Pixels ***, const AIR_Pixels);

AIR_Pixels ***AIR_swap_x_and_y(AIR_Pixels ***,const struct AIR_Key_info *, struct AIR_Key_info *, /*@out@*/ AIR_Error *);

AIR_Pixels ***AIR_swap_x_and_z(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, /*@out@*/ AIR_Error *);

AIR_Pixels ***AIR_swap_y_and_z(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, /*@out@*/ AIR_Error *);

void AIR_swapbytes(void *, const size_t, const size_t);

AIR_Error AIR_sylv(const unsigned int, double **, double **, const unsigned int, double **, double **, double **, double **, double **, double **);

double AIR_tfield(const double, const double, const double, const double, const double, const double, const double, const double, /*@out@*/ AIR_Error *);

double AIR_t2field(const double, const double, const double, const double, const double, const double, const double, const double, int *);


double **AIR_tritrix2(const unsigned int);
double ***AIR_tritrix3(const unsigned int, const unsigned int);
double ****AIR_tritrix4(const unsigned int, const unsigned int, const unsigned int);
double *****AIR_tritrix5(const unsigned int, const unsigned int, const unsigned int, const unsigned int);
double ******AIR_tritrix6(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
double *******AIR_tritrix7(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);

double AIR_uvderiv2Dwarp(const unsigned int, double **, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, double *, const double, unsigned int *);

double AIR_uvderiv3Dwarp(const unsigned int, double **, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, double *, const double, unsigned int *);

double AIR_uvderivsLS12(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_uvderivsLS15(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_uvderivsLS6(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_uvderivsLS8(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_uvderivsN12(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_uvderivsN15(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_uvderivsN6(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_uvderivsN8(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_uvderivsRS12(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_uvderivsRS15(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_uvderivsRS6(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

double AIR_uvderivsRS8(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *);

AIR_Error AIR_uvrm(const unsigned int, double **, double **, double ***, double ***, double ****, double ****, const AIR_Boolean, const struct AIR_Key_info *, const struct AIR_Key_info *);

void AIR_uv2D3(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean);

void AIR_uv2D4(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean);

void AIR_uv2D5(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean);

void AIR_uv2D6(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean);

void AIR_uv2D8(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean);

void AIR_uv3D12(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean);

void AIR_uv3D15(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean);

void AIR_uv3D6(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean);

void AIR_uv3D7(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean);

void AIR_uv3D9(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean);

AIR_Error AIR_warp2D(double **, const double, const double, /*@out@*/ double *, /*@out@*/ double *, const unsigned int, double *);

AIR_Error AIR_warp3D(double **, const double, const double, const double, /*@out@*/ double *, /*@out@*/ double *, /*@out@*/ double *, const unsigned int, double *);

AIR_Error AIR_write_airw(const char *, const AIR_Boolean, double **, const struct AIR_Warp *);

AIR_Error AIR_write_air16(const char *, const AIR_Boolean, double **, const AIR_Boolean, const struct AIR_Air16 *);

AIR_Error AIR_write_header(const char *, const struct AIR_Key_info *, const char *, const int *);

AIR_Error AIR_write_nrm(const char *, const unsigned long int, const unsigned long int, const unsigned long int, const AIR_Boolean);

AIR_Error AIR_write_value(const double, const char *, const char *, const AIR_Boolean);

void AIR_zeros(AIR_Pixels ***, const struct AIR_Key_info *);

double AIR_zfield(/*@unused@*/ const double, /*@unused@*/ const double, const double, const double, const double, const double, const double, const double, int *);

/*@null@*/ AIR_Pixels ***AIR_zoom(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, /*@out@*/ AIR_Error *);


AIR_Pixels *AIR_create_vol1(const unsigned int);
AIR_Pixels **AIR_create_vol2(const unsigned int, const unsigned int);
AIR_Pixels ***AIR_create_vol3(const unsigned int, const unsigned int, const unsigned int);
AIR_Pixels ****AIR_create_vol4(const unsigned int, const unsigned int, const unsigned int, const unsigned int);
AIR_Pixels *****AIR_create_vol5(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
AIR_Pixels ******AIR_create_vol6(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
AIR_Pixels *******AIR_create_vol7(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
AIR_Pixels ********AIR_create_vol8(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
AIR_Pixels *********AIR_create_vol9(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);

double *AIR_matrix1(const unsigned int);
double **AIR_matrix2(const unsigned int, const unsigned int);
double ***AIR_matrix3(const unsigned int, const unsigned int, const unsigned int);
double ****AIR_matrix4(const unsigned int, const unsigned int, const unsigned int, const unsigned int);
double *****AIR_matrix5(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
double ******AIR_matrix6(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
double *******AIR_matrix7(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);

char *AIR_matrix1c(const unsigned int);
char **AIR_matrix2c(const unsigned int, const unsigned int);
char ***AIR_matrix3c(const unsigned int, const unsigned int, const unsigned int);
char ****AIR_matrix4c(const unsigned int, const unsigned int, const unsigned int, const unsigned int);
char *****AIR_matrix5c(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
char ******AIR_matrix6c(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
char *******AIR_matrix7c(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
char ********AIR_matrix8c(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);

float *AIR_matrix1f(const unsigned int);
float **AIR_matrix2f(const unsigned int, const unsigned int);
float ***AIR_matrix3f(const unsigned int, const unsigned int, const unsigned int);
float ****AIR_matrix4f(const unsigned int, const unsigned int, const unsigned int, const unsigned int);
float *****AIR_matrix5f(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
float ******AIR_matrix6f(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
float *******AIR_matrix7f(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);

int *AIR_matrix1i(const unsigned int);
int **AIR_matrix2i(const unsigned int, const unsigned int);
int ***AIR_matrix3i(const unsigned int, const unsigned int, const unsigned int);
int ****AIR_matrix4i(const unsigned int, const unsigned int, const unsigned int, const unsigned int);
int *****AIR_matrix5i(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
int ******AIR_matrix6i(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
int *******AIR_matrix7i(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);

void AIR_free_1(/*@null@*/ /*@out@*/ /*@only@*/ double *);
void AIR_free_2(/*@null@*/ /*@out@*/ /*@only@*/ double **);
void AIR_free_3(/*@null@*/ /*@out@*/ /*@only@*/ double ***);
void AIR_free_4(/*@null@*/ /*@out@*/ /*@only@*/ double ****);
void AIR_free_5(/*@null@*/ /*@out@*/ /*@only@*/ double *****);
void AIR_free_6(/*@null@*/ /*@out@*/ /*@only@*/ double ******);
void AIR_free_7(/*@null@*/ /*@out@*/ /*@only@*/ double *******);

void AIR_free_vol1(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels *);
void AIR_free_vol2(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels **);
void AIR_free_vol3(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***);
void AIR_free_vol4(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ****);
void AIR_free_vol5(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels *****);
void AIR_free_vol6(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ******);
void AIR_free_vol7(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels *******);
void AIR_free_vol8(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ********);
void AIR_free_vol9(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels *********);


void AIR_free_1c(/*@null@*/ /*@out@*/ /*@only@*/ char *);
void AIR_free_2c(/*@null@*/ /*@out@*/ /*@only@*/ char **);
void AIR_free_3c(/*@null@*/ /*@out@*/ /*@only@*/ char ***);
void AIR_free_4c(/*@null@*/ /*@out@*/ /*@only@*/ char ****);
void AIR_free_5c(/*@null@*/ /*@out@*/ /*@only@*/ char *****);
void AIR_free_6c(/*@null@*/ /*@out@*/ /*@only@*/ char ******);
void AIR_free_7c(/*@null@*/ /*@out@*/ /*@only@*/ char *******);
void AIR_free_8c(/*@null@*/ /*@out@*/ /*@only@*/ char ********);

void AIR_free_1f(/*@null@*/ /*@out@*/ /*@only@*/ float *);
void AIR_free_2f(/*@null@*/ /*@out@*/ /*@only@*/ float **);
void AIR_free_3f(/*@null@*/ /*@out@*/ /*@only@*/ float ***);
void AIR_free_4f(/*@null@*/ /*@out@*/ /*@only@*/ float ****);
void AIR_free_5f(/*@null@*/ /*@out@*/ /*@only@*/ float *****);
void AIR_free_6f(/*@null@*/ /*@out@*/ /*@only@*/ float ******);
void AIR_free_7f(/*@null@*/ /*@out@*/ /*@only@*/ float *******);

void AIR_free_1i(/*@null@*/ /*@out@*/ /*@only@*/ int *);
void AIR_free_2i(/*@null@*/ /*@out@*/ /*@only@*/ int **);
void AIR_free_3i(/*@null@*/ /*@out@*/ /*@only@*/ int ***);
void AIR_free_4i(/*@null@*/ /*@out@*/ /*@only@*/ int ****);
void AIR_free_5i(/*@null@*/ /*@out@*/ /*@only@*/ int *****);
void AIR_free_6i(/*@null@*/ /*@out@*/ /*@only@*/ int ******);
void AIR_free_7i(/*@null@*/ /*@out@*/ /*@only@*/ int *******);

double AIR_align_last_costfunction(void);

/* Error code */
/* These codes are arbitrary, but must be unique */

/* All error codes must match AIR_*_ERROR */

/* Generic file read errors match the reserved pattern AIR_*_READ_ERROR */

enum {

	/* Common global errors */
	AIR_MEMORY_ALLOCATION_ERROR=1,
    AIR_STRING_NOT_DOUBLE_ERROR,
    AIR_STRING_NOT_FLOAT_ERROR,
    AIR_STRING_NOT_INT_ERROR,
    AIR_STRING_NOT_LONG_ERROR,
    AIR_STRING_NOT_SCHAR_ERROR,
    AIR_STRING_NOT_SHORT_ERROR,
    AIR_STRING_NOT_UCHAR_ERROR,
    AIR_STRING_NOT_UINT_ERROR,
    AIR_STRING_NOT_ULONG_ERROR,
    AIR_STRING_NOT_USHORT_ERROR,
    AIR_USER_INTERFACE_ERROR,

	
	
	/* General error messages related to input files */
	AIR_ACCESS_CODING_READ_ERROR=100,
	AIR_ACCESS_DENIED_READ_ERROR,
	AIR_ACCESS_HOPS_READ_ERROR,
    AIR_ACCESS_LINKS_READ_ERROR,
    AIR_ACCESS_SIGNAL_READ_ERROR,
    AIR_ATYPICAL_FILE_READ_ERROR,
    AIR_BAD_LINK_READ_ERROR,
    AIR_CANT_CLOSE_READ_ERROR,
    AIR_FILE_UNAVAILABLE_READ_ERROR,
    AIR_FILE_UNMOUNTED_READ_ERROR,
    AIR_GROUP_CANT_READ_ERROR,
    AIR_INVALID_PATH_READ_ERROR,
    AIR_NAME_TOO_LONG_READ_ERROR,
    AIR_NO_FILE_READ_ERROR,
    AIR_NO_STAT_FILE_READ_ERROR,
    AIR_OWNER_CANT_READ_ERROR,
    AIR_STRUCT_READ_ERROR,
	AIR_UNSPECIFIED_FILE_READ_ERROR,
    AIR_UNUSUAL_FILE_READ_ERROR,
    AIR_WORLD_CANT_READ_ERROR,

	/* Error messages related to input image files */
    AIR_BAD_CODE_READ_ERROR=200,
    AIR_DECOMPRESS_READ_ERROR,
	AIR_FSEEK_READ_ERROR,
    AIR_HEADER_INIT_CODING_ERROR,
	AIR_INFO_BITS_ERROR,
    AIR_INFO_DIM_ERROR,
    AIR_INFO_HLENGTHFIELD_ERROR,
    AIR_INFO_LIMIT_ERROR,
    AIR_INFO_SIZE_ERROR,
    AIR_PLANEWISE_DECOMPRESS_READ_ERROR,
    AIR_READ_BINARYASNON_FILE_ERROR,
    AIR_READ_HEADER_FILE_ERROR,
    AIR_READ_IMAGE_FILE_ERROR,
    AIR_READ_NONASBINARY_FILE_ERROR,
    AIR_READIMAGE_CODING_ERROR,
    
    /* Error messages related to non-image input files */
    AIR_READ_AIR_FILE_ERROR=300,
    AIR_READ_ASCII_FILE_ERROR,
    AIR_READ_BINARY_FILE_ERROR,
    AIR_READ_INIT_FILE_ERROR,
    AIR_READ_NORM_FILE_ERROR,
    AIR_READ_OLDWARP_FILE_ERROR,
    AIR_READ_PERSPWARP_FILE_ERROR,
    AIR_READ_SCALING_FILE_ERROR, /* Note that this pertains to both scaling init files and scaling files used in reslicing */
    AIR_READ_SUFFIX_FILE_ERROR,
    AIR_READ_UCF_FILE_ERROR,
    AIR_READ_VECTOR_FILE_ERROR,
    AIR_READ_WARP_FILE_ERROR,
    
    /* General error messages related to output files */
    AIR_ACCESS_CODING_WRITE_ERROR=400,
	AIR_ACCESS_DENIED_WRITE_ERROR,
	AIR_ACCESS_HOPS_WRITE_ERROR,
	AIR_ACCESS_LINKS_WRITE_ERROR,
	AIR_ACCESS_SIGNAL_WRITE_ERROR,
	AIR_ATYPICAL_FILE_WRITE_ERROR,
    AIR_CANT_CLOSE_WRITE_ERROR,
	AIR_FILE_UNAVAILABLE_WRITE_ERROR,
	AIR_FILE_UNMOUNTED_WRITE_ERROR,
	AIR_GROUP_CANT_WRITE_ERROR,
	AIR_INVALID_PATH_WRITE_ERROR,
	AIR_NAME_TOO_LONG_WRITE_ERROR,
	AIR_NO_DIRECTORY_WRITE_ERROR,
	AIR_NO_PERMISSION_WRITE_ERROR,
	AIR_OWNER_CANT_WRITE_ERROR,
	AIR_STRUCT_WRITE_ERROR,
    AIR_UNSPECIFIED_FILE_WRITE_ERROR,
	AIR_UNUSUAL_FILE_WRITE_ERROR,
	AIR_WORLD_CANT_WRITE_ERROR,
	
	/* Error messages related to output image files */
	AIR_FSEEK_WRITE_ERROR=500,
	AIR_WRITE_BAD_DIM_ERROR,
	AIR_WRITE_BADBITS_CODING_ERROR,
	AIR_WRITE_HEADER_FILE_ERROR,
	AIR_WRITE_IMAGE_FILE_ERROR,
	AIR_WRITE_NONASBINARY_CODING_ERROR,
	
	/* Error messages related to non-image output files */
	

    AIR_WRITE_AIR_FILE_ERROR=600,
    AIR_WRITE_ASCII_FILE_ERROR,
    AIR_WRITE_BINARY_FILE_ERROR,
    AIR_WRITE_INIT_FILE_ERROR,
    AIR_WRITE_NORM_FILE_ERROR,
    AIR_WRITE_SCALING_FILE_ERROR, /* Note that this pertains to both scaling init files and scaling files used in reslicing */
    AIR_WRITE_SUFFIX_FILE_ERROR,
    AIR_WRITE_UCF_FILE_ERROR,
    AIR_WRITE_WARP_FILE_ERROR,



	/* Other error messages */

    AIR_INFO_DIM_MISMATCH_ERROR=700,
    AIR_INFO_SIZE_MISMATCH_ERROR,

    AIR_PATH_TOO_LONG_ERROR,
    AIR_MULTI_PLANE_IMAGE_ERROR,
    AIR_WARP_NOT_FIRST_ORDER_ERROR,
    AIR_VECTOR_FIELD_MISMATCH_ERROR,
    AIR_INTERP_CANT_2D_ERROR,
    AIR_NO_2D_VECTOR_UNITE_ERROR,
    AIR_VECTOR_FIELD_MULTIPLANE_ERROR,
    AIR_NORM_FILE_MISMATCH_ERROR,
    AIR_NORM_FILE_ZERO_ERROR,
    AIR_EMPTY_ROI_ERROR,
 
    AIR_TRANSFORM_BREAKS_SCANLINE_ERROR,
    AIR_NO_PERSPECTIVE_ERROR,
    AIR_POLYNOMIAL_ORDER_ERROR,
    AIR_POLYNOMIAL_DIMENSIONS_ERROR,

    AIR_DIMENSION_MISMATCH_ERROR,
    AIR_SVD_FAILURE_ERROR,

    AIR_CANT_2D_UNMATCHED_ERROR,
    AIR_CANT_3D_SINGLE_PLANE_ERROR,
    AIR_THRESHOLD_TOO_LARGE_ERROR,
    AIR_THRESHOLD_TOO_SMALL_ERROR,

    AIR_ALIGN_BAD_PARAMETER_ERROR,

    AIR_ALIGNWARP_BAD_PARAMETER_ERROR,

    AIR_ANTIWARP_ERROR,

    AIR_COMPLEX_ZERO_DIVIDE_ERROR,

    AIR_SINGULAR_MATRIX_ERROR,	/* Library routines may implement their own macro */
    AIR_NON_POSITIVE_DETERMINANT_ERROR, /* Library routines may implement their own macro */

    AIR_SINGULAR_CDVASB_ERROR,
    AIR_COMPLEX_LOG_UNDEFINED_ERROR,
    AIR_SINGULAR_COMLOGER_ERROR,
    AIR_COMLOGER_CONVERGE_ERROR,
    AIR_MATRIXMEAN_CONVERGE_ERROR,

    AIR_SCHUR_FAILURE_ERROR,
    AIR_EIGEN_VECTOR_FAILURE,
    AIR_SINGULAR_EEXPER_ERROR,
    AIR_SINGULAR_ELOGER_ERROR,
    AIR_NONPOSITIVE_ELOGER_ERROR,
    AIR_SINGULAR_MATRIXMEAN_ERROR,
    AIR_SPAT_INVERT_MATRIXMEAN_ERROR,
    AIR_SINGULAR_PROJCOMM_ERROR,
    AIR_SPAT_INVERT_PROJCOMM_ERROR,
    AIR_HQR_FAILURE_ERROR,

    AIR_SINGULAR_DVASB_ERROR,

    AIR_EXCHNG_CONVERGENCE_ERROR,

    AIR_SINGULAR_GAEL_ERROR,
    AIR_BAD_CONTRASTS_ERROR,
    AIR_INVALID_CONTRASTER_ERROR,


    AIR_DQAGE_INTEGRATION_ERROR,
    AIR_NEGATIVE_SMOOTHING_ERROR,
    AIR_INVALID_LAYOUT_ERROR,
    AIR_INVALID_MAGNIFICATION_ERROR,
    AIR_INVALID_MASK_ERROR,
    AIR_INVALID_MEANCALC_ERROR,
    AIR_INVALID_RESIZING_ERROR,
    AIR_INVALID_SHADING_ERROR,
    AIR_INVALID_UT_ERROR,
    AIR_DCSEVL_BAD_N_ERROR,
    AIR_DCSEVL_BAD_X_ERROR,
    AIR_D9LGMC_CANT_INIT_ERROR,
    AIR_D9LGMC_SMALL_X_ERROR,
    AIR_DGAMLM_CANT_MIN_ERROR,
    AIR_DGAMLM_CANT_MAX_ERROR,
    AIR_DGAMMA_X_IS_ZERO_ERROR,
    AIR_GAMMA_OF_NEGATIVE_ERROR,
    AIR_GAMMA_OVERFLOW_ERROR,
    AIR_DLNREL_X_TOO_SMALL_ERROR,
    AIR_DLBETA_NEGATIVE_ARGS_ERROR,
    AIR_DBETAI_BAD_X_ERROR,
    AIR_DBETAI_BAD_PQ_ERROR,
    AIR_DLTP2C_BAD_Q_ERROR,
    AIR_DLFP2C_BAD_P_ERROR,
    AIR_D9GMIC_BAD_A_ERROR,
    AIR_D9GMIC_NEGATIVE_X_ERROR,
    AIR_D9GMIC_CONVERGE_ERROR,
    AIR_D9GMIT_BAD_X_ERROR,
    AIR_D9GMIT_CONVERGE_ERROR,
    AIR_D9LGIC_CONVERGE_ERROR,
    AIR_D9LGIT_BAD_X_ERROR,
    AIR_D9LGIT_CONVERGE_ERROR,
    AIR_DGAMIC_BAD_X_ERROR,
    AIR_DGAMIC_BAD_A_X_ERROR,
    AIR_HOTELL_3D_ONLY_ERROR,
    AIR_MAGNIFY_MOD_ERROR,
    AIR_SHAPE_NO_SUCCESSES_ERROR,
    AIR_ILLEGAL_FWHM_ERROR,
    AIR_NOT_IN_DISTRIBUTION_TAIL_ERROR,
    AIR_SHAPE_CONVERGENCE_ERROR,
    AIR_ANOVA_BAD_MODEL_ERROR,

    AIR_REQUIRES_16_BITS_ERROR



};

