#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "igl_common.h"
#include "gl.h"
#include "get.h"

/* drawing interface */

/* TODO:
*/

void viewport(short l,short r,short b, short t)
{
	glViewport(l,b,r-l,t-b);
}
void viewpo_(long *l,long *r,long *b,long *t)
{
	viewport(*l,*r,*b,*t);
}

void pushviewport(void)
{
	glPushAttrib(GL_VIEWPORT_BIT);
}
void pushvi_(void)
{
	pushviewport();
}

void popviewport(void)
{
	glPopAttrib();
}
void popvie_(void)
{
	popviewport();
}

void gl_setupwindow(void)
{	
	glDisable(GL_DEPTH_TEST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
}

void getviewport(short *l,short *r,short *b,short *t)
{
	GLint	dims[4];
	glGetIntegerv( GL_VIEWPORT, dims);
	*l = dims[0];
	*b = dims[1];
	*r = dims[0] + dims[2];
	*t = dims[1] + dims[3];
}

void clear(void)
{
	GLfloat	col[4];
	GLint	dim[4];

	glGetIntegerv( GL_VIEWPORT, dim);
	glGetFloatv( GL_CURRENT_COLOR, col);

	glScissor(dim[0],dim[1],dim[2],dim[3]);
	glEnable(GL_SCISSOR_TEST);
	glClearColor(col[0],col[1],col[2],col[3]);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
}

void czclear( unsigned long back, long depth)
{
	GLfloat	col[4];
	GLint	dim[4];

	cpack(back);

	glGetIntegerv( GL_VIEWPORT, dim);
	glGetFloatv( GL_CURRENT_COLOR, col);

	glScissor(dim[0],dim[1],dim[2],dim[3]);
	glEnable(GL_SCISSOR_TEST);
	glClearColor(col[0],col[1],col[2],col[3]);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
}

long getbuffer(void)
{
	GLint	b;
	int	i = 0;
	glGetIntegerv(GL_DRAW_BUFFER,&b);
	if (b == GL_FRONT_AND_BACK) i = FRNTBUFFER | BCKBUFFER;
	if (b == GL_BACK) i = BCKBUFFER;
	if (b == GL_FRONT) i = FRNTBUFFER;

	return(i);
}

void frontbuffer(long l)
{
	GLenum	m = GL_BACK;
	if (l) {
		m = GL_FRONT_AND_BACK;
	}
	glDrawBuffer(m);
#ifdef DEBUG
printf("Frontbuf: %d\n",l);
#endif
}

void backbuffer(long l)
{
	GLint	b;
	GLenum	m = GL_NONE;
	glGetIntegerv(GL_DRAW_BUFFER,&b);
	if ((b == GL_FRONT) || (b == GL_FRONT_AND_BACK)) {
		m = GL_FRONT_AND_BACK; 
	} else {
		m = GL_BACK; 
	}
	glDrawBuffer(m);
#ifdef DEBUG
printf("Backbuf: %d\n",l);
#endif
}

void zbuffer(long b)
{
	if (b) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}
}

long getzbuffer(void)
{
	GLboolean	b;
	b = glIsEnabled(GL_DEPTH_TEST);
	return(b ? 1 : 0);
}

void swapbuffers(void)
{
	wigl	*w = get_current();
	if (!w) return;
#ifdef DEBUG
printf("Swap: %d\n",w);
#endif
#ifdef WIN32
	SwapBuffers( w->hdc );
#else
	glXSwapBuffers( get_display(), w->win );
#endif
}

int curmmode = MPROJECTION;
void mmode( short m)
{
	switch(m) {
		case MVIEWING:
			glMatrixMode(GL_MODELVIEW);
			curmmode = m;
			break;
		case MPROJECTION:
			glMatrixMode(GL_PROJECTION);
			curmmode = m;
			break;
		default:
			printf("Invalid mmode(%d)\n",m);
			break;
	}
}

void popmatrix( void )
{
	glPopMatrix();
}
void pushmatrix( void )
{
	glPushMatrix();
}

void ortho(float l, float r, float b, float t, float n, float f)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(l,r,b,t,n,f);
	glMatrixMode(GL_MODELVIEW);
}
void ortho2(float l, float r, float b, float t)
{
	ortho(l,r,b,t,-1,1);
}
void ortho_(float *l, float *r, float *b, float *t,float *n,float *f)
{
	ortho(*l,*r,*b,*t,*n,*f);
}
void ortho2_(float *l, float *r, float *b, float *t)
{
	ortho2(*l,*r,*b,*t);
}

void gfflush(void)
{
	GLint	b;
	glGetIntegerv(GL_DRAW_BUFFER,&b);
	if (b == GL_FRONT_AND_BACK) gflush();
	if (b == GL_FRONT) gflush();
}
void gfflush_(void)
{
	gfflush();
}
void gflush(void)
{
	glFinish();
}
void gflush_(void)
{
	gflush();
}

void resetl_(long *b)
{
	return;
}

void rectzoom(float x,float y)
{
	glPixelZoom(x,y);
}
void rectzo_(float *x,float *y)
{
	rectzoom(*x,*y);
}

static int sourcebuffer = SRC_BACK;
static int destz = 0;

void zdraw(long b)
{
	destz = b;
}
void zdraw_(long *b)
{
	zdraw(*b);
}
long lrectread(short a,short b,short c,short d, unsigned long *data)
{
	if (sourcebuffer == SRC_ZBUFFER) {
		glReadPixels(a,b,c-a+1,d-b+1,GL_DEPTH_COMPONENT,GL_INT,data);
	} else {
		glReadPixels(a,b,c-a+1,d-b+1,GL_RGBA,GL_UNSIGNED_BYTE,data);
	}
	if (is_big_endian() && (sourcebuffer != SRC_ZBUFFER)) {
		int swab = 4;
		int num = (c-a+1)*(d-b+1);
		byteswap(data,num*swab,swab);
	}
	return((c-a+1)*(d-b+1));
}
long lrectr_(long int *a,long int *b,long int *c,long int *d,long int *e)
{
	return(lrectread(*a,*b,*c,*d,(unsigned long *)e));
}
void lrectwrite(short a,short b,short c,short d, const unsigned long *indata)
{
	GLvoid	*data;
        int dx,dy,i;
        dx = c-a+1;
        dy = d-b+1;
	data = (void *)indata;
	if (is_big_endian() && (!destz)) {
		static void *tmp = NULL;
		static int tlen = 0;
		int swab = 4;
		int num = dx*dy;
		if (!tmp) {
			tmp = malloc(num*swab);
		} else {
			if (tlen < num*swab) {
				tmp = realloc(tmp,num*swab);
				tlen = num*swab;
			}
		}
		data = (void *)tmp;
		memcpy(data,indata,num*swab);
		byteswap(data,num*swab,swab);
	}
	glRasterPos2i(a,b);
	if (destz) {
               GLboolean isZ;
               GLint glf;
#if 0
		float junk[500*500];
glReadBuffer(GL_BACK);
		glReadPixels(0,0,dx,dy,GL_DEPTH_COMPONENT,GL_FLOAT,junk);
printf("RJF - really:\n");
for(i=50;i<dx-50;i++) printf("%f ",((float*)junk)[dx*(dy/2)+i]);
printf("\n");
#endif
                glGetIntegerv(GL_DEPTH_FUNC,&glf);
                isZ = glIsEnabled(GL_DEPTH_TEST);
		glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
                glEnable(GL_DEPTH_TEST);
	        glDepthFunc(GL_ALWAYS);
		glDrawPixels(dx,dy,GL_DEPTH_COMPONENT,GL_FLOAT,data);
	        glDepthFunc(glf);
                if (!isZ) glDisable(GL_DEPTH_TEST);
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
#if 0
printf("RJF - before:\n");
for(i=50;i<dx-50;i++) printf("%f ",((float*)data)[dx*(dy/2)+i]);
		glReadPixels(0,0,dx,dy,GL_DEPTH_COMPONENT,GL_FLOAT,data);
printf("\n");
printf("RJF - after:\n");
for(i=50;i<dx-50;i++) printf("%f ",((float*)data)[dx*(dy/2)+i]);
printf("\n");
#endif
	} else {
		glDrawPixels(dx,dy,GL_RGBA,GL_UNSIGNED_BYTE,data);
	}
}
void lrectw_(long int *a,long int *b,long int *c,long int *d,long int *e)
{
	lrectwrite(*a,*b,*c,*d,(unsigned long *)e);
}

void readsource(long a)
{
	switch(a) {
		case SRC_FRONT:
			glReadBuffer(GL_FRONT);
			break;
		case SRC_BACK:
			glReadBuffer(GL_BACK);
			break;
		case SRC_ZBUFFER:
			glReadBuffer(GL_BACK);
			break;
		default:
			glReadBuffer(GL_BACK);
			break;
	}
	sourcebuffer = a;
	return;
}
void readso_(long *a)
{
	readsource(*a);
}

void rectre_()
{
	printf("%s: Unimplemented...\n","rectre_()");
}
void rectwr_()
{
	printf("%s: Unimplemented...\n","rectwr_()");
}

void writem_(short *wm)
{
	printf("%s: Unimplemented...\n","writem_()");
}

void zclear(void)
{
	glClear(GL_DEPTH_BUFFER_BIT);
}
void zclear_(void)
{
	zclear();
}

void linewidth(short w)
{
	glLineWidth(w);
}
void linewi_(long *w)
{
	linewidth(*w);
}

void bgnpoint(void)
{
	glBegin(GL_POINTS);
}
void bgnpoi_(void)
{
	bgnpoint();
}
void endpoint(void)
{
	glEnd();
}
void endpoi_(void)
{
	endpoint();
}

void getmco_()
{
	printf("%s: Unimplemented...\n","getmco_()");
}

void gammaramp( const short r[256], const short g[256], const short b[256])
{
	return;
}

void wmpack(unsigned long w)
{
	GLboolean	r,g,b,a;
	r = GL_FALSE;
	g = GL_FALSE;
	b = GL_FALSE;
	a = GL_FALSE;
	if (w & 0x000000ff) r = GL_TRUE;
	if (w & 0x0000ff00) g = GL_TRUE;
	if (w & 0x00ff0000) b = GL_TRUE;
	if (w & 0xff000000) a = GL_TRUE;
	glColorMask(r,g,b,a);
}
void wmpack_(unsigned long *w)
{
	wmpack(*w);
}

void zwritemask(unsigned long mask)
{
	GLboolean	z = GL_FALSE;
	if (mask) z = GL_TRUE;
	glDepthMask(z);
}
void zwrite_(unsigned long *m)
{
	zwritemask(*m);
}

void zfunction(long f)
{
	GLenum	glf = GL_LESS;
	switch(f) {
		case ZF_NEVER:
			glf = GL_NEVER;
			break;
		case ZF_LESS:
			glf = GL_LESS;
			break;
		case ZF_EQUAL:
			glf = GL_EQUAL;
			break;
		case ZF_LEQUAL:
			glf = GL_LEQUAL;
			break;
		case ZF_GREATER:
			glf = GL_GREATER;
			break;
		case ZF_NOTEQUAL:
			glf = GL_NOTEQUAL;
			break;
		case ZF_GEQUAL:
			glf = GL_GEQUAL;
			break;
		case ZF_ALWAYS:
			glf = GL_ALWAYS;
			break;
	}
	glDepthFunc(glf);
}
void zfunct_(long *f)
{
	zfunction(*f);
}

void lsetdepth(long n,long f)
{
	printf("TODO: lsetdepth()\n");
}
void lsetde_(long *n,long *f)
{
	lsetdepth(*n,*f);
}

void stenci_(void )
{
	printf("%s: Unimplemented...\n","stenci_()");
}
void sclear_(void )
{
	printf("%s: Unimplemented...\n","sclear_()");
}
void stensi_(void )
{
	printf("%s: Unimplemented...\n","stensi_()");
}


/* FORTRAN */
#ifdef WIN_IFORT
#define getvie_ GETVIE
#define getbuf_ GETBUF
#define frontb_ FRONTB
#define backbu_ BACKBU
#define zbuffe_ ZBUFFE
#define swapbu_ SWAPBU
#define getzbu_ GETZBU
#endif

void getvie_(short *l,short *r,short *b,short *t)
{
	getviewport(l,r,b,t);
}

long getbuf_(void)
{
	return(getbuffer());
}

void frontb_(long *log)
{
	frontbuffer(*log);
}

void backbu_(long *log)
{
	backbuffer(*log);
}

void zbuffe_(long *log)
{
	zbuffer(*log);
}

void swapbu_(void)
{
	swapbuffers();
}

long getzbu_(void)
{
        return(getzbuffer());
}


/* primitives */

unsigned short	linestyles[32768];

void deflinestyle(short n,unsigned short l)
{
	linestyles[n] = l;
}
void deflin_(long *n,unsigned long *l)
{
	deflinestyle(*n,*l);
}

void setlinestyle(short n)
{
	if (n == 0) {
		glDisable(GL_LINE_STIPPLE);
	} else {
		glLineStipple(1,linestyles[n]);
		glEnable(GL_LINE_STIPPLE);
	}
}
void setlin_(long *n)
{
	setlinestyle(*n);
}

void cmov2i(long x,long y)
{
	glRasterPos2i(x,y);
}

void cmovi(long x, long y, long z)
{
	glRasterPos3i(x,y,z);
}

void cmov2(float x,float y)
{
	glRasterPos2f(x,y);
}

void color(unsigned short ci)
{
	printf("Unsupported color model: color()\n");
}

void circfi(long xo,long yo,long rad)
{
	float	x,y;
	float	a = 0,inc=10.0;
	int	i;

	glBegin(GL_POLYGON);
	for(i=0;i<360;i+=inc) {
		x = sin(a)*(float)(rad) + xo;
		y = cos(a)*(float)(rad) + yo;
		glVertex2f(x,y);
		a += (M_PI/180.)*inc;
	}
	glEnd();
}

void circi(long xo,long yo,long rad)
{
	float	x,y;
	float	a = 0,inc=10.0;
	int	i;

	glBegin(GL_LINE_LOOP);
	for(i=0;i<360;i+=inc) {
		x = sin(a)*(float)(rad) + xo;
		y = cos(a)*(float)(rad) + yo;
		glVertex2f(x,y);
		a += (M_PI/180.)*inc;
	}
	glEnd();
}

void RGBcolor(short rs,short gs,short bs)
{
	GLubyte	r,g,b;
	r = rs;
	g = gs;
	b = bs;
	glColor3ub(r,g,b);
}

void rgbi(unsigned char r,unsigned char g,unsigned char b)
{
	glColor3ub(r,g,b);
}

void c3i(const long *rgb)
{
	GLubyte	r,g,b;
	r = rgb[0];
	g = rgb[1];
	b = rgb[2];
	glColor3ub(r,g,b);
}

void cpack(unsigned long c)
{
	GLubyte r,g,b,a;
	a = (c & 0xff000000) >> 24;
	b = (c & 0x00ff0000) >> 16;
	g = (c & 0x0000ff00) >> 8;
	r = (c & 0x000000ff);
	glColor4ub(r,g,b,a); 
}

void recti(long l,long b,long r,long t)
{
	glBegin(GL_LINE_LOOP);
	glVertex2i(l,b);
	glVertex2i(l,t);
	glVertex2i(r,t);
	glVertex2i(r,b);
	glEnd();
}

void rectfi(long l,long b,long r,long t)
{
	glBegin(GL_POLYGON);
	glVertex2i(l,b);
	glVertex2i(l,t);
	glVertex2i(r,t);
	glVertex2i(r,b);
	glEnd();
}

void sboxfi(long l,long b,long r,long t)
{
	glBegin(GL_POLYGON);
	glVertex2f((float)l-0.5,(float)b-0.5);
	glVertex2f((float)l-0.5,(float)t+0.5);
	glVertex2f((float)r+0.5,(float)t+0.5);
	glVertex2f((float)r+0.5,(float)b-0.5);
	glEnd();
}
void sboxfi_(long *l,long *b,long *r,long *t)
{
	sboxfi(*l,*b,*r,*t);
}


void bgnclosedline(void)
{
	glBegin(GL_LINE_LOOP);
}

void endclosedline(void)
{
	glEnd();
}

void bgnline(void)
{
	glBegin(GL_LINE_STRIP);
}

void endline(void)
{
	glEnd();
}

void bgnpolygon(void)
{
	glBegin(GL_POLYGON);
}

void endpolygon(void)
{
	glEnd();
}

void v2i(const long *pt)
{
	glVertex2i(pt[0],pt[1]);
}

void v2f(const float *pt)
{
	glVertex2f(pt[0],pt[1]);
}

void v2d(const double *pt)
{
	glVertex2f(pt[0],pt[1]);
}

void v3f(const float *pt)
{
	glVertex3f(pt[0],pt[1],pt[2]);
}

void n3f(const float *pt)
{
	glNormal3f(pt[0],pt[1],pt[2]);
}
void c3f(const float *pt)
{
	glColor3f(pt[0],pt[1],pt[2]);
}

void v3d(const double *pt)
{
	glVertex3f(pt[0],pt[1],pt[2]);
}

void v3i(const long *pt)
{
	glVertex3i(pt[0],pt[1],pt[2]);
}

/* FORTRAN */
void cmov2i_(long *x,long *y)
{
	cmov2i(*x,*y);
}

void color_(long *ci)
{
	color(*ci);
}

void circfi_(long *x,long *y,long *rad)
{
	circfi(*x,*y,*rad);
}

void circi_(long *x,long *y,long *rad)
{
	circi(*x,*y,*rad);
}

void c3i_(long *rgb)
{
	c3i(rgb);
}

void recti_(long *l,long *b,long *r,long *t)
{
	recti(*l,*b,*r,*t);
}

void rectfi_(long *l,long *b,long *r,long *t)
{
	rectfi(*l,*b,*r,*t);
}

void bgnclo_(void)
{
	bgnclosedline();
}

void endclo_(void)
{
	endclosedline();
}

void bgnlin_(void)
{
	bgnline();
}

void endlin_(void)
{
	endline();
}

void bgnpol_(void)
{
	bgnpolygon();
}

void endpol_(void)
{
	endpolygon();
}

void v2i_(long *pt)
{
	v2i(pt);
}

void v2f_(float *pt)
{
	v2f(pt);
}

void v2d_(double *pt)
{
	v2d(pt);
}

void v3d_(double *pt)
{
	v3d(pt);
}

void v3i_(long *pt)
{
	v3i(pt);
}

void cpack_(unsigned long *c)
{
	cpack(*c);
}

void clear_(void)
{
	clear();
}

void rgbcol_(long *rs,long *gs,long *bs)
{
	RGBcolor(*rs,*gs,*bs);
}

/* Depth Cue */
void depthc_(long *on)
{
	depthcue(*on);
}

void depthcue(Boolean mode)
{
	if (mode) {
		glEnable(GL_FOG);
	} else {
		glDisable(GL_FOG);
	}
}

void lrgbra_(short *rmin,short *gmin,short *bmin,
	     short *rmax,short *gmax,short *bmax, 
	     long *zn, long *zf)
{
	lRGBrange(*rmin,*gmin,*bmin,*rmax,*gmax,*bmax,*zn,*zf);
}

void lRGBrange(short rmin,short gmin,short bmin,
	       short rmax,short gmax,short bmax, 
	       long zn, long zf)
{
	GLfloat	fcolor[4];
	glFogi(GL_FOG_MODE,GL_LINEAR);
	glFogf(GL_FOG_START,zn/32768);
	glFogf(GL_FOG_START,zf/32768);
	fcolor[0] = rmax/255.;
	fcolor[1] = gmax/255.;
	fcolor[2] = bmax/255.;
	fcolor[3] = 1.0;
	glFogfv(GL_FOG_COLOR,fcolor);
}

/* unfinished work */
void lmdef(short type, short index, short num, const float *prop)
{
}
void lmbind(short target, short index)
{
}

void bgntmesh(void)
{
	printf("bgntmesh() not implemented\n");
}
void endtmesh(void)
{
	printf("endtmesh() not implemented\n");
}
void swaptmesh(void)
{
	printf("swaptmesh() not implemented\n");
}

void blendfunction(long s, long d)
{
	printf("blendfunction() not implemented\n");
}
void subpixel( long onoff)
{
	printf("subpixel() not implemented\n");
}
void smoothline( long onoff)
{
	printf("smoothline() not implemented\n");
}

/* from Mesa glu.c */
void perspective( short ang, float aspect, float zNear, float zFar)
{
   GLdouble xmin, xmax, ymin, ymax, fovy;

   fovy = ((float)ang) * 0.1;

   ymax = zNear * tan( fovy * M_PI / 360.0 );
   ymin = -ymax;

   xmin = ymin * aspect;
   xmax = ymax * aspect;

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}

void getmatrix( float mat[4][4])
{
	if (curmmode == MPROJECTION) {
		glGetFloatv(GL_PROJECTION_MATRIX,(float *)mat);
	} else {
		glGetFloatv(GL_MODELVIEW_MATRIX,(float *)mat);
	}
}

void loadmatrix( const Matrix mat)
{
	glLoadMatrixf((float *)mat); 
}

void multmatrix( const Matrix mat)
{
	glMultMatrixf((float *)mat); 
}

void translate( float x, float y, float z)
{
	glTranslatef(x,y,z);
}
void scale( float x, float y, float z)
{
	glScalef(x,y,z);
}
void rotate( short ang, char a)
{
	switch(a) {
		case 'x':
			glRotatef(ang*0.1,1.0,0.0,0.0);
			break;
		case 'y':
			glRotatef(ang*0.1,0.0,1.0,0.0);
			break;
		case 'z':
			glRotatef(ang*0.1,0.0,0.0,1.0);
			break;
		default:
			printf("Invalid rotate() axis: %c\n",a);
			break;
	}
}


/* function to perform general 2, 4 and 8 byte byteswapping */
void  byteswap(void *buffer,int len,int swapsize)
{
        int     num;
        char    *p = (char *)buffer;
        char    t;

        switch(swapsize) {
                case 2:
                        num = len/swapsize;
                        while(num--) {
                                t = p[0]; p[0] = p[1]; p[1] = t;
                                p += swapsize;
                        }
                        break;
                case 4:
                        num = len/swapsize;
                        while(num--) {
                                t = p[0]; p[0] = p[3]; p[3] = t;
                                t = p[1]; p[1] = p[2]; p[2] = t;
                                p += swapsize;
                        }
                        break;
                case 8:
                        num = len/swapsize;
                        while(num--) {
                                t = p[0]; p[0] = p[7]; p[7] = t;
                                t = p[1]; p[1] = p[6]; p[6] = t;
                                t = p[2]; p[2] = p[5]; p[5] = t;
                                t = p[3]; p[3] = p[4]; p[4] = t;
                                p += swapsize;
                        }
                        break;
                default:
                        break;
        }
        return;
}

