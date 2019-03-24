#ifndef __IAF_DEFS__
#define __IAF_DEFS__


#if defined (WIN32) || defined ( _WinNT )
	#ifndef FREE
	#include <malloc.h>
	/*  free memory to the OS */
	#define FREE(A) free(A);_heapmin()
	#endif
#else
	#ifndef FREE
	#define FREE(A) free(A)
	#endif
#endif

#ifndef MALLOC
#define MALLOC(A) malloc(A)
#endif
#ifndef REALLOC
#define REALLOC(a,b) realloc(a,b)
#endif

/* IAF portable types */
#if defined (WIN32) || defined ( _WinNT )
typedef __int64 VLINT64;
typedef __int64 *VLPINT64;
typedef unsigned __int64 VLUINT64;
typedef unsigned __int64 *VLPUINT64;
 
#endif

typedef signed long VLINT32;
typedef signed long * VLPINT32;
typedef unsigned long VLUINT32;
typedef unsigned long * VLPUINT32;

typedef signed short VLINT16;
typedef signed short * VLPINT16;
typedef unsigned short VLUINT16;
typedef unsigned short * VLPUINT16;

typedef signed char VLINT8;
typedef signed char * VLPINT8;
typedef unsigned char * VLPUINT8;
typedef unsigned char VLUINT8;

typedef char VLCHAR;
typedef char * VLPCHAR;
typedef signed char VLSCHAR;
typedef signed char * VLPSCHAR;
typedef unsigned char  VLUCHAR;
typedef unsigned char *VLPUCHAR;


typedef float VLFLOAT32;
typedef float * VLPFLOAT32;

typedef double VLFLOAT64;
typedef double * VLPFLOAT64;

/*
typedef long double VLFLOAT80;
typedef long double *VLPFLOAT80;
*/


typedef void VLVOID;
typedef void *VLPVOID;



#ifndef TRUE
#define TRUE 1L
#endif
#ifndef FALSE
#define FALSE 0L
#endif


#endif
