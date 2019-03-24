#include "gl.h"
#include "device.h"
#include "fmclient.h"

/* 
   fmclient FORTRAN wrapper

   These routine act as wrappers to the fmxxx routines for FORTRAN.
   Not all are supported here, but enough to get by.
   The biggest note is that these routines expect to be passed
   valid C strings in many cases.  I generally add a "char(0)" to
   the end of all the needed strings and these routines work ok.

	Revision history:
	8 Mar 89 rjf
		First version of the wrappers.

*/
#ifdef CAP_NOUS
#define fminit_  FMINIT
#define fmprstr_ FMPRSTR
#define fmfreefont_ FMFREEFONT
#define fmgetstrwidth_ FMGETSTRWIDTH
#define fmsetfont_ FMSETFONT
#define fmfindfont_ FMFINDFONT
#define fmgetfontinfo_ FMGETFONTINFO
#define fmsetpath_ FMSETPATH
#define fmscalefont_ FMSCALEFONT
#define fmfreefont_ FMFREEFONT
#endif

/* Call fminit */
void fminit_(void )
{
#ifndef GL_ONLY
	fminit();
#else
	return;
#endif
}
/* call fmprstr("asas"//char(0)) */
void fmprstr_(char *str)
{
#ifndef GL_ONLY
	fmprstr(str);
#else
	charstr(str);
#endif
}
/* call fmfindfont("sadfd"//char(0),fh) */
void fmfindfont_(char *name, fmfonthandle *fh)
{
#ifndef GL_ONLY
	*fh =fmfindfont(name);
#else
	*fh = (fmfonthandle)1;
#endif
}
/* call fmsetfont(fh) */
void fmsetfont_(fmfonthandle *fh)
{
#ifndef GL_ONLY
	fmsetfont(*fh);
#else
	return;
#endif
}
/* call fmgetstrwidth(fh,"asds"//char(0),width) */
void fmgetstrwidth_(fmfonthandle *fh,char *str,long *width)
{
#ifndef GL_ONLY
	*width=fmgetstrwidth(*fh,str);
#else
	*width=strwidth(str);
#endif
}
/* call fmfreefont(fh) */
void fmfreefont_(fmfonthandle *fh)
{
#ifndef GL_ONLY
	fmfreefont(*fh);
#else
	return;
#endif
}
/* call fmscalefont(fh1,R*4,fh2) */
void fmscalefont_(fmfonthandle *fh,float *size,fmfonthandle *fh2)
{
#ifndef GL_ONLY
	double  size2;

	size2=(*size);
	*fh2=fmscalefont(*fh,size2);
#else
	return;
#endif
}
/* fmgetfontinfo(fh,array) */
void fmgetfontinfo_(fmfonthandle *fh,long *l1,long *l2,long *l3,
	long *l4,long *l5)
{
#ifndef GL_ONLY
	long junk;
	fmfontinfo cinfo;

	junk=fmgetfontinfo(*fh,&cinfo);
	*l1=cinfo.xorig;
	*l2=cinfo.yorig;
	*l3=cinfo.xsize;
	*l4=cinfo.ysize;
	*l5=cinfo.height;
#else
	*l1 = 0;
	*l2 = getdescender();
	*l3=strwidth("W");
	*l4=getheight();
	*l5=getheight()+getdescender();
#endif
}
/* call fmsetpath("/usr/lib/fmfonts"//char(0)) */
void fmsetpath_(char *str)
{
#ifndef GL_ONLY
	fmsetpath(str);
#else
	return;
#endif
}
