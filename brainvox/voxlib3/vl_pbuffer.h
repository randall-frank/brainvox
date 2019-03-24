#ifndef _GL_FLOAT_PBUFFER_H_
#define _GL_FLOAT_PBUFFER_H_

/*
 *
 * Copyright (c) 2003, Industrial Light & Magic, a division of Lucas
 * Digital Ltd. LLC
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * *       Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * *       Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 * *       Neither the name of Industrial Light & Magic nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#ifdef WIN32

#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#ifndef WGL_ARB_pbuffer
DECLARE_HANDLE(HPBUFFERARB);
#endif

#else

#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/X.h> 

#endif

#ifdef osx

/* from agl.h */
#define AGL_NONE                   0
#define AGL_ALL_RENDERERS          1  /* choose from all available renderer*/
#define AGL_BUFFER_SIZE            2  /* depth of the index buffer         */
#define AGL_LEVEL                  3  /* level in plane stacking           */
#define AGL_RGBA                   4  /* choose an RGBA format             */
#define AGL_DOUBLEBUFFER           5  /* double buffering supported        */
#define AGL_STEREO                 6  /* stereo buffering supported        */
#define AGL_AUX_BUFFERS            7  /* number of aux buffers             */
#define AGL_RED_SIZE               8  /* number of red component bits      */
#define AGL_GREEN_SIZE             9  /* number of green component bits    */
#define AGL_BLUE_SIZE             10  /* number of blue component bits     */
#define AGL_ALPHA_SIZE            11  /* number of alpha component bits    */
#define AGL_DEPTH_SIZE            12  /* number of depth bits              */
#define AGL_STENCIL_SIZE          13  /* number of stencil bits            */
#define AGL_ACCUM_RED_SIZE        14  /* number of red accum bits          */
#define AGL_ACCUM_GREEN_SIZE      15  /* number of green accum bits        */
#define AGL_ACCUM_BLUE_SIZE       16  /* number of blue accum bits         */
#define AGL_ACCUM_ALPHA_SIZE      17  /* number of alpha accum bits        */
#define AGL_PIXEL_SIZE            50  /* frame buffer bits per pixel       */
#define AGL_MINIMUM_POLICY        51  /* never smaller bufs than request   */
#define AGL_MAXIMUM_POLICY        52  /* largest buffers of type requested */
#define AGL_OFFSCREEN             53  /* an off-screen capable renderer    */
#define AGL_FULLSCREEN            54  /* a full-screen capable renderer    */
#define AGL_SAMPLE_BUFFERS_ARB    55  /* # of multi sample buffers         */
#define AGL_SAMPLES_ARB           56  /* # of samples/multi sample buffer  */
#define AGL_AUX_DEPTH_STENCIL     57  /* independent depth and/or stencil buffers for the aux buffer */
#define AGL_COLOR_FLOAT           58  /* color buffers store float pixels  */
#define AGL_MULTISAMPLE           59  /* choose multisample                */
#define AGL_SUPERSAMPLE           60  /* choose supersample                */
#define AGL_SAMPLE_ALPHA          61  /* request alpha filtering           */
#define AGL_RENDERER_ID           70  /* request renderer by ID            */
#define AGL_SINGLE_RENDERER       71  /* choose 1 renderer for all screens */
#define AGL_NO_RECOVERY           72  /* disable failure recovery systems  */
#define AGL_ACCELERATED           73  /* hardware accelerated renderer     */
#define AGL_CLOSEST_POLICY        74  /* closest color buffer to request   */
#define AGL_ROBUST                75  /* does not need failure recovery    */
#define AGL_BACKING_STORE         76  /* backbuf contents valid after swap */
#define AGL_MP_SAFE               78  /* renderer is multi-processor safe  */
#define AGL_WINDOW                80  /* can be used to render to a window */
#define AGL_MULTISCREEN           81  /* 1 window can span multiple screens*/
#define AGL_VIRTUAL_SCREEN        82  /* virtual screen number             */
#define AGL_COMPLIANT             83  /* renderer is opengl compliant      */
#define AGL_PBUFFER               90  /* can be used to render to a pbuffer*/
#define AGL_REMOTE_PBUFFER        91  /* render offline to a pbuffer       */

typedef void *osxPbuffer;
typedef void *osxPixelFormat;
typedef void *osxContext;
typedef void *osxDevice;

osxPixelFormat osxChoosePixelFormat(const osxDevice *,GLint,const GLint *);
osxContext osxCreateContext(osxPixelFormat,osxContext);
GLboolean osxCreatePBuffer(GLint,GLint,GLenum,GLenum,long,osxPbuffer *);
void osxDestroyPixelFormat(osxPixelFormat);
GLboolean osxDestroyContext(osxContext);
GLboolean osxDestroyPBuffer(osxPbuffer);
GLboolean osxSetCurrentContext(osxContext);
GLboolean osxSetPBuffer(osxContext,osxPbuffer,GLint,GLint,GLint);
GLint osxGetVirtualScreen(osxContext);
#endif

#define PBUFFER_HW_NONE		0
#define PBUFFER_HW_ATI		1
#define PBUFFER_HW_NVIDIA	2

typedef struct {
    int		     _dim[2];
    int		     _depth;
    int		     _sdepth;
    int		     _zdepth;
    int              _hw_detect;
#ifdef osx
    osxPbuffer       _pbuffer;
    osxPixelFormat   _osxPixFmt;
    osxContext       _osxContext;
    Display *        _prevDisplay;
    GLXPbuffer       _prevDrawable;
    GLXContext       _prevContext;
#elif WIN32
    HPBUFFERARB      _pbuffer;
    HDC              _pbufdc;
    HGLRC	     _context;

    HDC		     _prevDC;
    HGLRC	     _prevContext;
#else
    Display *        _display;
    GLXPbuffer       _pbuffer;
    GLXContext       _context;
    int  	     _needCloseDisplay;

    Display *        _prevDisplay;
    GLXPbuffer       _prevDrawable;
    GLXContext       _prevContext;
#endif
} PBuffer;

/*
 * Creates an N-bit floating point pbuffer with the specified
 * dimensions.
 */
#ifdef osx
PBuffer *pbCreate(int *dims, int depth, int zdepth, int sdepth);
#elif WIN32
PBuffer *pbCreate(HWND win,int *dims, int depth, int zdepth, int sdepth);
#else
PBuffer *pbCreate(Display *display,int *dims, int depth, int zdepth, int sdepth);
#endif
void pbDestroy(PBuffer *);


/* The pbuffer's dimensions. */
int *pbGetdims(PBuffer *);
int pbGetdepth(PBuffer *);
int pbGetsdepth(PBuffer *);
int pbGetzdepth(PBuffer *);

/*
 * Make the pbuffer the current GL context and drawable.
 * Subsequent OpenGL calls will apply to this pbuffer.
 */
void pbActivate(PBuffer *);

/*
 * Restore the GL context and drawable that were in effect
 * prior to the last activate () call.
 */
void pbDeactivate(PBuffer *);

/* True if the pBuffer could be successfully created. */
int pbValid(PBuffer *);

/* Hardware identification */
int pbHardware(PBuffer *);

void vl_colorswap2(void *p,int num);
void vl_colorswap4(void *in,int num);

#endif /* _GL_FLOAT_PBUFFER_H_ */
