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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "voxel.h"
#include "vl_private.h"

/*
 *----------------------------------------------------------------------
 *  Vendor specific headers, here to make cross platform cleaner...
 *---------------------------------------------------------------------- 
 */

#ifdef WIN32

/* From wglext.h:  WGL_ARB_pbuffer */
#ifndef WGL_ARB_pbuffer
#define WGL_DRAW_TO_PBUFFER_ARB        0x202D
#define WGL_MAX_PBUFFER_PIXELS_ARB     0x202E
#define WGL_MAX_PBUFFER_WIDTH_ARB      0x202F
#define WGL_MAX_PBUFFER_HEIGHT_ARB     0x2030
#define WGL_PBUFFER_LARGEST_ARB        0x2033
#define WGL_PBUFFER_WIDTH_ARB          0x2034
#define WGL_PBUFFER_HEIGHT_ARB         0x2035
#define WGL_PBUFFER_LOST_ARB           0x2036
#endif

#ifndef WGL_ARB_pixel_format
#define WGL_NUMBER_PIXEL_FORMATS_ARB   0x2000
#define WGL_DRAW_TO_WINDOW_ARB         0x2001
#define WGL_DRAW_TO_BITMAP_ARB         0x2002
#define WGL_ACCELERATION_ARB           0x2003
#define WGL_NEED_PALETTE_ARB           0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB    0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB     0x2006
#define WGL_SWAP_METHOD_ARB            0x2007
#define WGL_NUMBER_OVERLAYS_ARB        0x2008
#define WGL_NUMBER_UNDERLAYS_ARB       0x2009
#define WGL_TRANSPARENT_ARB            0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB  0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define WGL_SHARE_DEPTH_ARB            0x200C
#define WGL_SHARE_STENCIL_ARB          0x200D
#define WGL_SHARE_ACCUM_ARB            0x200E
#define WGL_SUPPORT_GDI_ARB            0x200F
#define WGL_SUPPORT_OPENGL_ARB         0x2010
#define WGL_DOUBLE_BUFFER_ARB          0x2011
#define WGL_STEREO_ARB                 0x2012
#define WGL_PIXEL_TYPE_ARB             0x2013
#define WGL_COLOR_BITS_ARB             0x2014
#define WGL_RED_BITS_ARB               0x2015
#define WGL_RED_SHIFT_ARB              0x2016
#define WGL_GREEN_BITS_ARB             0x2017
#define WGL_GREEN_SHIFT_ARB            0x2018
#define WGL_BLUE_BITS_ARB              0x2019
#define WGL_BLUE_SHIFT_ARB             0x201A
#define WGL_ALPHA_BITS_ARB             0x201B
#define WGL_ALPHA_SHIFT_ARB            0x201C
#define WGL_ACCUM_BITS_ARB             0x201D
#define WGL_ACCUM_RED_BITS_ARB         0x201E
#define WGL_ACCUM_GREEN_BITS_ARB       0x201F
#define WGL_ACCUM_BLUE_BITS_ARB        0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB       0x2021
#define WGL_DEPTH_BITS_ARB             0x2022
#define WGL_STENCIL_BITS_ARB           0x2023
#define WGL_AUX_BUFFERS_ARB            0x2024
#define WGL_NO_ACCELERATION_ARB        0x2025
#define WGL_GENERIC_ACCELERATION_ARB   0x2026
#define WGL_FULL_ACCELERATION_ARB      0x2027
#define WGL_SWAP_EXCHANGE_ARB          0x2028
#define WGL_SWAP_COPY_ARB              0x2029
#define WGL_SWAP_UNDEFINED_ARB         0x202A
#define WGL_TYPE_RGBA_ARB              0x202B
#define WGL_TYPE_COLORINDEX_ARB        0x202C
#endif

#ifndef WGL_NV_float_buffer
#define WGL_FLOAT_COMPONENTS_NV        0x20B0
#define WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_R_NV 0x20B1
#define WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RG_NV 0x20B2
#define WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGB_NV 0x20B3
#define WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGBA_NV 0x20B4
#define WGL_TEXTURE_FLOAT_R_NV         0x20B5
#define WGL_TEXTURE_FLOAT_RG_NV        0x20B6
#define WGL_TEXTURE_FLOAT_RGB_NV       0x20B7
#define WGL_TEXTURE_FLOAT_RGBA_NV      0x20B8
#endif

/* From: wglati.h */
#ifndef WGL_ATI_pixel_format_float
#define WGL_ATI_pixel_format_float  1

#define WGL_TYPE_RGBA_FLOAT_ATI             0x21A0
#define GL_TYPE_RGBA_FLOAT_ATI              0x8820
#define GL_COLOR_CLEAR_UNCLAMPED_VALUE_ATI  0x8835
#endif

/* function prototypes... */

#ifndef WGL_ARB_extensions_string
#define WGL_ARB_extensions_string 1
typedef const char * (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC) (HDC hDC);
#endif /* WGL_ARB_extensions_string */

#ifndef WGL_ARB_pbuffer
#define WGL_ARB_pbuffer 1
typedef HPBUFFERARB (WINAPI * PFNWGLCREATEPBUFFERARBPROC) (HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int *piAttribList);
typedef HDC (WINAPI * PFNWGLGETPBUFFERDCARBPROC) (HPBUFFERARB hPbuffer);
typedef int (WINAPI * PFNWGLRELEASEPBUFFERDCARBPROC) (HPBUFFERARB hPbuffer, HDC hDC);
typedef BOOL (WINAPI * PFNWGLDESTROYPBUFFERARBPROC) (HPBUFFERARB hPbuffer);
typedef BOOL (WINAPI * PFNWGLQUERYPBUFFERARBPROC) (HPBUFFERARB hPbuffer, int iAttribute, int *piValue);
#endif

#ifndef WGL_ARB_pixel_format
#define WGL_ARB_pixel_format 1
typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBIVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBFVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);
typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
#endif

/* dispatch table... */
PFNWGLGETEXTENSIONSSTRINGARBPROC _wglGetExtensionsStringARB = NULL;
PFNWGLCREATEPBUFFERARBPROC _wglCreatePbufferARB = NULL;
PFNWGLGETPBUFFERDCARBPROC _wglGetPbufferDCARB = NULL;
PFNWGLRELEASEPBUFFERDCARBPROC _wglReleasePbufferDCARB = NULL;
PFNWGLDESTROYPBUFFERARBPROC _wglDestroyPbufferARB = NULL;
PFNWGLQUERYPBUFFERARBPROC _wglQueryPbufferARB = NULL;
PFNWGLGETPIXELFORMATATTRIBIVARBPROC _wglGetPixelFormatAttribivARB = NULL;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC _wglGetPixelFormatAttribfvARB = NULL;
PFNWGLCHOOSEPIXELFORMATARBPROC _wglChoosePixelFormatARB = NULL;

#else

/* From:  glxATI.h */
#ifndef GLX_ATI_pixel_format_float
#define GLX_ATI_pixel_format_float  1
                                                                                
#define GLX_RGBA_FLOAT_ATI_BIT          0x00000100
                                                                                
#endif /* GLX_ATI_pixel_format_float */


/* From:  glxtokens.h (nVidia's version) */
#ifndef GLX_NV_float_buffer
#define GLX_NV_float_buffer 1

#define GLX_FLOAT_COMPONENTS_NV         0x20B0

#endif /* GLX_NV_float_buffer */

#endif

#ifdef WIN32

HWND wnd = NULL;
HDC dc = NULL;
HGLRC rc = NULL;

static int WindowsCreateCtx(void) 
{
    int pixelformat;

    WNDCLASS wc;
    PIXELFORMATDESCRIPTOR pfd;
    /* register window class */
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpfnWndProc = DefWindowProc;
    wc.lpszClassName = "Pbuffer";
    if (!RegisterClass(&wc)) return(GL_FALSE);
    /* create window */
    wnd = CreateWindow("Pbuffer", "pBuffer", 0, CW_USEDEFAULT, CW_USEDEFAULT, 
	CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandle(NULL), NULL);
    if (!wnd) return(GL_FALSE);
    /* get the device context */
    dc = GetDC(wnd);
    if (!dc) return(GL_FALSE);
    /* find pixel format */
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pixelformat = ChoosePixelFormat(dc, &pfd);
    if (pixelformat == 0) return(GL_FALSE);

    /* set the pixel format for the dc */
    if (!SetPixelFormat(dc, pixelformat, &pfd)) return(GL_FALSE);
    /* create rendering context */
    rc = wglCreateContext(dc);
    if (!rc) return(GL_FALSE);
    if (!wglMakeCurrent(dc, rc)) return(GL_FALSE);
    return(GL_TRUE);
}
static void WindowsDestroyCtx(void)
{
    if (rc) wglMakeCurrent(NULL, NULL);
    if (rc) wglDeleteContext(wglGetCurrentContext());
    if (wnd && NULL != dc) ReleaseDC(wnd, dc);
    if (wnd) DestroyWindow(wnd);
    UnregisterClass("Pbuffer", GetModuleHandle(NULL));
}
static int WindowsInit(void)
{
    /* Get the proc addresses... */
    if (!_wglGetExtensionsStringARB) {
    	if (!WindowsCreateCtx()) return(-1);

    	_wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    	_wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC)wglGetProcAddress("wglCreatePbufferARB");
    	_wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC)wglGetProcAddress("wglGetPbufferDCARB");
    	_wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)wglGetProcAddress("wglReleasePbufferDCARB");
    	_wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC)wglGetProcAddress("wglDestroyPbufferARB");
    	_wglQueryPbufferARB = (PFNWGLQUERYPBUFFERARBPROC)wglGetProcAddress("wglQueryPbufferARB");
    	_wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
    	_wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribfvARB");
    	_wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

    	if (!_wglGetExtensionsStringARB) return(-1);
    }
    return(0);
}
#endif

/******************** OpenGL extensions */

/* bindings for the extensions */
PFNGLACTIVETEXTUREARBPROC _glActiveTextureARB = NULL;
PFNGLMULTITEXCOORD3FARBPROC _glMultiTexCoord3fARB = NULL;
PFNGLDELETEPROGRAMSARBPROC _glDeleteProgramsARB = NULL;
PFNGLGENPROGRAMSARBPROC _glGenProgramsARB = NULL;
PFNGLBINDPROGRAMARBPROC _glBindProgramARB = NULL;
PFNGLPROGRAMSTRINGARBPROC _glProgramStringARB = NULL;
PFNGLPROGRAMENVPARAMETER4FVARBPROC _glProgramEnvParameter4fvARB = NULL;
PFNGLPROGRAMENVPARAMETER4DVARBPROC _glProgramEnvParameter4dvARB = NULL;
#ifdef WIN32
PFNGLTEXIMAGE3DPROC _glTexImage3D = NULL;
PFNGLTEXSUBIMAGE3DPROC _glTexSubImage3D = NULL;
#endif

#ifdef osx
extern void *aglGetProcAddress(char * pszProc);
#define GPA aglGetProcAddress
#elif WIN32
#define GPA wglGetProcAddress
#else
#define GPA glXGetProcAddressARB
#endif

static int BindGLFuncs(void)
{
    /* have we bound things yet? */
    if (!_glActiveTextureARB) {
      _glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)GPA("glActiveTextureARB");
      _glMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC)GPA("glMultiTexCoord3fARB");
      _glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)GPA("glDeleteProgramsARB");
      _glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)GPA("glGenProgramsARB");
      _glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)GPA("glBindProgramARB");
      _glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)GPA("glProgramStringARB");
      _glProgramEnvParameter4fvARB = (PFNGLPROGRAMENVPARAMETER4FVARBPROC)GPA("glProgramEnvParameter4fvARB");
      _glProgramEnvParameter4dvARB = (PFNGLPROGRAMENVPARAMETER4DVARBPROC)GPA("glProgramEnvParameter4dvARB");
#ifdef WIN32
      _glTexImage3D = (PFNGLTEXIMAGE3DPROC)GPA("glTexImage3D");
      _glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)GPA("glTexSubImage3D");
#endif
    }
    return(0);
}


#ifdef osx

PBuffer *pbCreate(int *dim, int depth, int zdepth, int sdepth) 
{
/*
    GLint attrib[] = { AGL_RGBA, AGL_DEPTH_SIZE, 24, AGL_CLOSEST_POLICY, 
                       AGL_NONE };
*/
    GLint attrib[] = { AGL_RGBA, AGL_RED_SIZE, 8, AGL_ALPHA_SIZE, 8, 
                       AGL_DEPTH_SIZE, 24, AGL_CLOSEST_POLICY, AGL_ACCELERATED,
                       AGL_NO_RECOVERY, AGL_NONE };

    const char *ext;

    PBuffer *p = (PBuffer *)calloc(1,sizeof(PBuffer));
    if (!p) return(NULL);

    p->_depth = depth;
    p->_zdepth = zdepth;
    p->_sdepth = sdepth;
    p->_dim[0] = dim[0];
    p->_dim[1] = dim[1];
    p->_hw_detect = PBUFFER_HW_NONE;
    p->_pbuffer = NULL;

#ifdef GLDEBUG
   printf("RJF OSX pixelformat\n");
#endif
    p->_osxPixFmt = osxChoosePixelFormat(NULL, 0, attrib);
    if (!p->_osxPixFmt) {
       free(p);
       return(NULL);
    }
#ifdef GLDEBUG
   printf("RJF OSX createcontext\n");
#endif
    p->_osxContext = osxCreateContext(p->_osxPixFmt, NULL);
    if (!p->_osxContext) {
       osxDestroyPixelFormat(p->_osxPixFmt);
       free(p);
       return(NULL);
    }
#ifdef GLDEBUG
   printf("RJF OSX createpbuffer\n");
#endif
    if (!osxCreatePBuffer(p->_dim[0], p->_dim[1], GL_TEXTURE_RECTANGLE_EXT, 
                    GL_RGBA, 0, &(p->_pbuffer))) {
       osxDestroyPixelFormat(p->_osxPixFmt);
       osxDestroyContext(p->_osxContext);
       free(p);
       return(NULL);
    }

    pbActivate(p);

    /* get our extensions */
    BindGLFuncs();

    ext = (const char *)glGetString(GL_EXTENSIONS);
#ifdef GLDEBUG
   printf("RJF OSX extensions %s\n",ext);
#endif
    if (!strstr(ext,"GL_ARB_fragment_program")) {
       fprintf(stderr,"Missing fragment program extension\n%s\n",ext);
       pbDestroy(p);
       return(NULL);
    }

#ifdef GLDEBUG
   printf("RJF OSX ActiveTexture %X\n",_glActiveTextureARB);
#endif
    if (!_glActiveTextureARB) {
       pbDestroy(p);
       return(NULL);
    }

    return(p);
}

#elif WIN32

PBuffer *pbCreate(HWND win, int *dim, int depth, int zdepth, int sdepth) 
{
    HDC hdc;
    const char *ext;
    int fbAttrList[100] = {0}, pbAttrList[100] = {0};
    float flist[10] = {0};
    int pformats[50],i;
    unsigned int num;

    PBuffer *p = (PBuffer *)calloc(1,sizeof(PBuffer));
    if (!p) return(NULL);

    p->_depth = depth;
    p->_zdepth = zdepth;
    p->_sdepth = sdepth;
    p->_dim[0] = dim[0];
    p->_dim[1] = dim[1];
    p->_hw_detect = PBUFFER_HW_NONE;
    p->_pbuffer = NULL;

#ifdef GLDEBUG
printf("RJF - pbCreate win=%d\n",win);
#endif

    /* get the device */
    if (!win) win = wnd;
    hdc = GetDC(win);
#ifdef GLDEBUG
printf("RJF HDC = %d\n",hdc);
#endif

    /* do the getprocaddress bits */
    if (WindowsInit()) {
#ifdef GLDEBUG
printf("RJF - WindowsInit failed\n");
#endif
       free(p);
       return(NULL);
    }

    /* grab the previous */
    p->_prevContext = wglGetCurrentContext();
    p->_prevDC = wglGetCurrentDC();

#ifdef GLDEBUG
printf("RJF - ctx dc %d %d\n",p->_prevContext,p->_prevDC);
#endif

    /* can we get a pbuffer? */
    if (!_wglCreatePbufferARB) {
#ifdef GLDEBUG
printf("RJF - no _wglCreatePbufferARB\n");
#endif
       free(p);
       return(NULL);
    }

    /* Check HW */
    ext = _wglGetExtensionsStringARB(hdc);
    if (ext) {
       if (strstr(ext,"WGL_ATI_pixel_format_float")) p->_hw_detect=PBUFFER_HW_ATI;
       if (strstr(ext,"WGL_NV_float_buffer")) p->_hw_detect=PBUFFER_HW_NVIDIA;
    }

#ifdef GLDEBUG
printf("RJF - extensions: %s\n",ext);
printf("RJF - depths: %d %d\n",p->_depth,p->_zdepth);
#endif

    /* Attribute lists */
    i = 0;
    fbAttrList[i++] = WGL_DOUBLE_BUFFER_ARB;	fbAttrList[i++] = GL_FALSE;
    fbAttrList[i++] = WGL_DRAW_TO_PBUFFER_ARB;	fbAttrList[i++] = GL_TRUE;
    fbAttrList[i++] = WGL_DRAW_TO_WINDOW_ARB;	fbAttrList[i++] = GL_FALSE;
    fbAttrList[i++] = WGL_SUPPORT_OPENGL_ARB;	fbAttrList[i++] = GL_TRUE;    
    fbAttrList[i++] = WGL_RED_BITS_ARB;		fbAttrList[i++] = p->_depth;
    fbAttrList[i++] = WGL_GREEN_BITS_ARB;	fbAttrList[i++] = p->_depth;
    fbAttrList[i++] = WGL_BLUE_BITS_ARB;	fbAttrList[i++] = p->_depth;
#if 0
    fbAttrList[i++] = WGL_ALPHA_BITS_ARB;	fbAttrList[i++] = p->_depth;
    fbAttrList[i++] = WGL_STENCIL_BITS_ARB;	fbAttrList[i++] = p->_sdepth;
#endif
    if (p->_zdepth) {
        fbAttrList[i++] = WGL_DEPTH_BITS_ARB;	fbAttrList[i++] = p->_zdepth;
    }
    if ((p->_hw_detect == PBUFFER_HW_ATI) && (p->_depth > 8)) {
	fbAttrList[i++] = WGL_PIXEL_TYPE_ARB;
	fbAttrList[i++] = WGL_TYPE_RGBA_FLOAT_ATI;
    } else if ((p->_hw_detect == PBUFFER_HW_NVIDIA) && (p->_depth > 8)) {
	fbAttrList[i++] = WGL_PIXEL_TYPE_ARB;
	fbAttrList[i++] = WGL_TYPE_RGBA_ARB;
	fbAttrList[i++] = WGL_FLOAT_COMPONENTS_NV;
       	fbAttrList[i++] = GL_TRUE;
    } else {
	fbAttrList[i++] = WGL_PIXEL_TYPE_ARB;
	fbAttrList[i++] = WGL_TYPE_RGBA_ARB;
    }
    fbAttrList[i++] = 0;

    /* set up the Pbuffer config list */
    i = 0;
#if 0
    pbAttrList[i++] = WGL_PBUFFER_LARGEST_ARB;    pbAttrList[i++] = 1;
#endif
    pbAttrList[i++] = 0;

    /* Pick the pixel format */
    if (!_wglChoosePixelFormatARB(hdc, fbAttrList, flist, 
	sizeof(pformats)/sizeof(pformats[0]), pformats, &num)) {
	ReleaseDC(win,hdc);
        free(p);
	return(NULL);
    }

#ifdef GLDEBUG
printf("RJF - format: %d\n",pformats[0]);
#endif

    /* Create the pbuffer object */
    p->_pbuffer = _wglCreatePbufferARB(hdc, pformats[0], 
                                       p->_dim[0], p->_dim[1], pbAttrList);
    if (!p->_pbuffer) {
	ReleaseDC(win,hdc);
        free(p);
	return(NULL);
    }

    /* We don't need this anymore */
    ReleaseDC(win,hdc);

    /* Get the device context for the pbuffer */
    p->_pbufdc = _wglGetPbufferDCARB(p->_pbuffer);
    if (!p->_pbufdc) {
        free(p);
	return(NULL);
    }

    /* Create an OpenGL context */
    p->_context = wglCreateContext(p->_pbufdc);
    if (!p->_context) {
        free(p);
	return(NULL);
    }

    /* Share display lists/etc with the current (if any) */
    if (p->_prevContext) wglShareLists(p->_context,p->_prevContext);

    /* Make us current */
    if (!wglMakeCurrent(p->_pbufdc,p->_context)) {
       fprintf(stderr,"Unable to make pbuffer current\n");
    }

    /* bind all the other gl funcs */
    BindGLFuncs();
    
    /* do not use 3d texturing (if we cannot bind the funcs) */
    if (!_glActiveTextureARB) {
       pbDestroy(p);
       p = NULL;
    }

    return(p);
}
#else

PBuffer *pbCreate(Display *display, int *dim, int depth, int zdepth, 
                  int sdepth) 
{
    const char *ext;
    int screen,num,mjr,mnr;
    int fbAttrList[100],pbAttrList[100];
    int i;

    PBuffer *p = (PBuffer *)calloc(1,sizeof(PBuffer));
    if (!p) return(NULL);

    /* Deal with the Display */
    p->_display = display;
    p->_needCloseDisplay = GL_FALSE;
    if (!p->_display) {
        p->_display = XOpenDisplay(NULL);
        p->_needCloseDisplay = GL_TRUE;
    }
    screen = DefaultScreen(p->_display);

    /* Init some stuff... */
    p->_context = NULL;
    p->_pbuffer = 0;
    p->_prevDisplay = NULL;
    p->_prevContext = NULL;
    p->_prevDrawable  = None;
    p->_depth = depth;
    p->_zdepth = zdepth;
    p->_sdepth = sdepth;
    p->_dim[0] = dim[0];
    p->_dim[1] = dim[1];
    p->_pbuffer = NULL;

    /* get the display + current GLX products */
    p->_prevDisplay  = glXGetCurrentDisplay();
    if (!p->_prevDisplay) p->_prevDisplay = p->_display;
    p->_prevDrawable = glXGetCurrentDrawable();
    p->_prevContext  = glXGetCurrentContext();

    /* Check HW */
    p->_hw_detect = PBUFFER_HW_NONE;
    ext = glXGetClientString(p->_display, GLX_EXTENSIONS);
    if (strstr(ext,"GLX_ATI_pixel_format_float")) p->_hw_detect=PBUFFER_HW_ATI;
    if (strstr(ext,"GLX_NV_float_buffer")) p->_hw_detect=PBUFFER_HW_NVIDIA;

    /* set up the FBConfig list */
    i = 0;
    fbAttrList[i++] = GLX_RED_SIZE;      fbAttrList[i++] = p->_depth;
    fbAttrList[i++] = GLX_GREEN_SIZE;    fbAttrList[i++] = p->_depth;
    fbAttrList[i++] = GLX_BLUE_SIZE;     fbAttrList[i++] = p->_depth;
    fbAttrList[i++] = GLX_ALPHA_SIZE;    fbAttrList[i++] = p->_depth;
    fbAttrList[i++] = GLX_STENCIL_SIZE;  fbAttrList[i++] = p->_sdepth;
    fbAttrList[i++] = GLX_DEPTH_SIZE;    fbAttrList[i++] = p->_zdepth;
    fbAttrList[i++] = GLX_DOUBLEBUFFER;  fbAttrList[i++] = GL_FALSE;
    fbAttrList[i++] = GLX_DRAWABLE_TYPE; fbAttrList[i++] = GLX_PBUFFER_BIT;
    if ((p->_hw_detect == PBUFFER_HW_ATI) && (p->_depth > 8)) {
        fbAttrList[i++] = GLX_RENDER_TYPE; 
        fbAttrList[i++] = GLX_RGBA_FLOAT_ATI_BIT;
    } else if ((p->_hw_detect == PBUFFER_HW_NVIDIA) && (p->_depth > 8)) {
        fbAttrList[i++] = GLX_RENDER_TYPE;
        fbAttrList[i++] = GLX_RGBA_BIT;
        fbAttrList[i++] = GLX_FLOAT_COMPONENTS_NV;
        fbAttrList[i++] = GL_TRUE;
    } else {
        fbAttrList[i++] = GLX_RENDER_TYPE;
        fbAttrList[i++] = GLX_RGBA_BIT;
    }
    fbAttrList[i++] = None;

    /* set up the Pbuffer config list */
    i = 0;
    pbAttrList[i++] = GLX_PBUFFER_WIDTH;      pbAttrList[i++] = p->_dim[0];
    pbAttrList[i++] = GLX_PBUFFER_HEIGHT;     pbAttrList[i++] = p->_dim[1];
#if 0
    pbAttrList[i++] = GLX_LARGEST_PBUFFER;    pbAttrList[i++] = GL_TRUE;
#endif
    pbAttrList[i++] = GLX_PRESERVED_CONTENTS; pbAttrList[i++] = GL_TRUE;
    pbAttrList[i++] = None;

    /* Choose a configuration */
    if (!glXQueryVersion(p->_display,&mjr,&mnr)) {
       mjr = 1;
       mnr = 2;
    }

    /* check for version 1.3 (and fbconfig) */
    if ((mjr > 1) || ((mjr == 1) && (mnr >= 3))) {
       /* use the 1.3 fbconfig functions */
       GLXFBConfig *config=glXChooseFBConfig(
                             p->_display,screen,fbAttrList,&num);
       if (!config) {
          if (p->_needCloseDisplay) XCloseDisplay(p->_display);
          free(p);
          return(NULL);
       }

       /* create the Pbuffer */
       p->_pbuffer = glXCreatePbuffer(p->_display, config[0], pbAttrList);
       if (!p->_pbuffer) {
          if (p->_needCloseDisplay) XCloseDisplay(p->_display);
   	  XFree(config);
          free(p);
	  return(NULL);
       }

       /* Create the context */
       p->_context = glXCreateNewContext(p->_display,config[0],GLX_RGBA_TYPE,
	   glXGetCurrentContext(),GL_TRUE);
       if (!p->_context) {
          if (p->_needCloseDisplay) XCloseDisplay(p->_display);
	  XFree(config);
	  free(p);
	  return(NULL);
       }

       XFree(config);
    } else {
       if (p->_needCloseDisplay) XCloseDisplay(p->_display);
       free(p);
       return(NULL);
    }

    /* bind all the other gl funcs */
    BindGLFuncs();
   
    /* do not use 3d texturing (if we cannot bind the funcs) */
    if (!_glActiveTextureARB) {
       pbDestroy(p);
       p = NULL;
    }

    return(p);
}
#endif

void pbDestroy(PBuffer *p)
{
    if (!p) return;
#ifdef osx
    /* Restore previous context, if valid. */
    osxSetCurrentContext(NULL);
    if (p->_pbuffer) osxDestroyPBuffer(p->_pbuffer);
    if (p->_osxContext) osxDestroyContext(p->_osxContext);
    if (p->_osxPixFmt) osxDestroyPixelFormat(p->_osxPixFmt);
    if (p->_prevDisplay && (p->_prevDrawable != None) && p->_prevContext) {
	glXMakeCurrent(p->_prevDisplay, p->_prevDrawable, p->_prevContext);
    }
#elif WIN32
    wglMakeCurrent(NULL,NULL);

    if (p->_context) wglDeleteContext(p->_context);
    if (p->_pbufdc) _wglReleasePbufferDCARB(p->_pbuffer,p->_pbufdc);
    if (p->_pbuffer) _wglDestroyPbufferARB(p->_pbuffer);

    if (p->_prevContext && p->_prevDC) {
	wglMakeCurrent(p->_prevDC, p->_prevContext);
    }
#else
    glXMakeCurrent(p->_display, None, None);

    if (p->_context && p->_display) glXDestroyContext(p->_display,p->_context);
    if (p->_pbuffer && p->_display) glXDestroyPbuffer(p->_display,p->_pbuffer);

    /* Restore previous context, if valid. */
    if (p->_prevDisplay && (p->_prevDrawable != None) && p->_prevContext) {
	glXMakeCurrent(p->_prevDisplay, p->_prevDrawable, p->_prevContext);
    }
    if (p->_needCloseDisplay) XCloseDisplay(p->_display);
#endif
    free(p);
}


void pbActivate(PBuffer *p)
{
#ifdef GLDEBUG
   printf("RJF Activate\n");
#endif
    /* Save current context. */
#ifdef WIN32
    p->_prevContext = wglGetCurrentContext();
    p->_prevDC = wglGetCurrentDC();
    if (!wglMakeCurrent(p->_pbufdc, p->_context)) {
       fprintf(stderr,"Unable to make current...\n");
    }
#else
    p->_prevDisplay  = glXGetCurrentDisplay();
    p->_prevDrawable = glXGetCurrentDrawable();
    p->_prevContext  = glXGetCurrentContext();
#ifndef osx
    glXMakeCurrent(p->_display, p->_pbuffer, p->_context);
#else
    osxSetCurrentContext(p->_osxContext);
    osxSetPBuffer(p->_osxContext, p->_pbuffer, 0, 0, 
                  osxGetVirtualScreen(p->_osxContext));
#endif
#endif
}


void pbDeactivate(PBuffer *p)
{
#ifdef GLDEBUG
   printf("RJF Deactivate\n");
#endif
    /* Restore previous context, if valid. */
#ifdef WIN32
    if (p->_prevContext && p->_prevDC) {
	wglMakeCurrent(p->_prevDC, p->_prevContext);
    }
#else
    if (p->_prevDisplay && (p->_prevDrawable != None) && p->_prevContext) {
	glXMakeCurrent(p->_prevDisplay, p->_prevDrawable, p->_prevContext);
    }
#endif
}

int *pbGetdims(PBuffer *p) { return(p->_dim); }
int pbGetdepth(PBuffer *p) { return(p->_depth); }
int pbGetsdepth(PBuffer *p) { return(p->_sdepth); }
int pbGetzdepth(PBuffer *p) { return(p->_zdepth); }
int pbHardware(PBuffer *p) { return(p->_hw_detect); }
int pbValid(PBuffer *p)
{
    if (!p) return(0);
    if (p->_pbuffer) return(1);
    return(0);
}

/* texture layouts for N channels */
static GLuint fmt_table[5] = {
   0, 
   GL_LUMINANCE, 
   GL_LUMINANCE_ALPHA, 
   GL_RGB, 
   GL_RGBA
};

static GLuint int_fmt_table[5] = {
   0, 
   GL_LUMINANCE8, 
   GL_LUMINANCE8_ALPHA8, 
   GL_RGB8, 
   GL_RGBA8
};

/* fragment programs for 8bit,16bit,8bit+lighting,16bit+lighting */
/*   "MOV result.color, tmp.r;\n" */
/*   program.env[x] = glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB,x,p); */
/*   program.env[0] = computed normal, 1.0 */
/*   program.env[1] = inten, srcinten, 0, 1.0 */
/*   program.env[2] = delta in "tex coords" for gradient */

/* get grad
 * normalize grad
 * grad DOT env[0]
 * clamp v >= 0.0
 * scale = env[1].x + v * env[1].y
 * color = color * scale (clamped)
 */
/*
   "OPTION ARB_precision_hint_fastest;\n"
   Remember: L and LA textures in => RGB & RGBA fetches...
 */
static char *frag_strings[4] = {
   "!!ARBfp1.0\n"
   "OPTION ARB_precision_hint_fastest;\n"
   "TEMP tmp;\n"
   "TEMP tmp2;\n"
   "TEX tmp, fragment.texcoord[0], texture[0], 3D;\n"
   "MAD tmp2, tmp, {0.9960,0.0,0.0,0.0}, {0.0039,0.6,0.0,1.0};\n"
   "TEX result.color, tmp2, texture[1], 2D;\n"
   "END\n"
   ,
   "!!ARBfp1.0\n"
   "OPTION ARB_precision_hint_fastest;\n"
   "TEMP tmp;\n"
   "TEMP tmp2;\n"
   "TEX tmp, fragment.texcoord[0], texture[0], 3D;\n"
   "MAD tmp2, tmp.raaa, {0.9960,0.4980,0.0,0.0}, {0.0039,0.0019,0.0,1.0};\n"
   "TEX result.color, tmp2, texture[1], 2D;\n"
   "END\n"
   ,
   "!!ARBfp1.0\n"
   "OPTION ARB_precision_hint_fastest;\n"
   "TEMP tmp;\n"
   "TEMP tmp2;\n"
   "TEMP tmp3;\n"
   "TEMP tmp4;\n"
   "TEMP tmp5;\n"
   "TEX tmp, fragment.texcoord[0], texture[0], 3D;\n"
   "MAD tmp2, tmp, {0.9960,0.0,0.0,0.0}, {0.0039,0.6,0.0,1.0};\n"
   "TEX tmp5, tmp2, texture[1], 2D;\n"
   "TEX tmp4.x, fragment.texcoord[1], texture[0], 3D;\n"
   "TEX tmp4.y, fragment.texcoord[2], texture[0], 3D;\n"
   "TEX tmp4.z, fragment.texcoord[3], texture[0], 3D;\n"
   "SUB tmp3, tmp4, tmp.r;\n"
   "DP3 tmp2, tmp3, tmp3;\n"
   "SUB tmp2.z, tmp2.x, 0.000000001;\n"
   "CMP tmp2.x, tmp2.z, 1.0, tmp2.x;\n"
   "RSQ tmp2.x, tmp2.x;\n"
   "MUL tmp3, tmp3, tmp2.x;\n"
   "DP3 tmp2, tmp3, program.env[0];\n"
   "MAX tmp2.x, tmp2.x, 0.0;\n"
   "MAD tmp2.x, tmp2.x, program.env[1].y, program.env[1].x;\n"
   "MAX tmp2.x, tmp2.x, 0.0;\n"
   "MUL_SAT result.color, tmp5, tmp2.x;\n"
   "MOV result.color.a, tmp5.a;\n"
   "END\n"
   ,
   "!!ARBfp1.0\n"
   "OPTION ARB_precision_hint_fastest;\n"
   "TEMP tmp;\n"
   "TEMP tmp2;\n"
   "TEMP tmp3;\n"
   "TEMP tmp4;\n"
   "TEMP tmp5;\n"
   "TEX tmp, fragment.texcoord[0], texture[0], 3D;\n"
   "MAD tmp2, tmp.raaa, {0.9960,0.4980,0.0,0.0}, {0.0039,0.0019,0.0,1.0};\n"
   "TEX tmp5, tmp2, texture[1], 2D;\n"
   "TEX tmp4.x, fragment.texcoord[1], texture[0], 3D;\n"
   "TEX tmp4.y, fragment.texcoord[2], texture[0], 3D;\n"
   "TEX tmp4.z, fragment.texcoord[3], texture[0], 3D;\n"
   "SUB tmp3, tmp4, tmp.r;\n"
   "DP3 tmp2, tmp3, tmp3;\n"
   "SUB tmp2.z, tmp2.x, 0.000000001;\n"
   "CMP tmp2.x, tmp2.z, 1.0, tmp2.x;\n"
   "RSQ tmp2.x, tmp2.x;\n"
   "MUL tmp3, tmp3, tmp2.x;\n"
   "DP3 tmp2, tmp3, program.env[0];\n"
   "MAX tmp2.x, tmp2.x, 0.0;\n"
   "MAD tmp2.x, tmp2.x, program.env[1].y, program.env[1].x;\n"
   "MAX tmp2.x, tmp2.x, 0.0;\n"
   "MUL_SAT result.color, tmp5, tmp2.x;\n"
   "MOV result.color.a, tmp5.a;\n"
   "END\n"
};

int err_check(char *s)
{
   GLenum err = glGetError();
   if (err == GL_NO_ERROR) return(0);
   if (!s) {
      fprintf(stderr,"Informational note: clear code set (%x).\n",err);
      return(0);
   }
   fprintf(stderr,"OpenGL error: %s : %d (%x)\n",s,err,err);
   return(-1);
}

#ifndef MAIN

void vl_init_hw_(void *datum, int flags, vset *set)
{
   vl_init_(set);

#ifdef NO_PBUFFER
   return;
#endif

   if (flags & VL_PBUFFER) {
      set->hw = (void *)MALLOC(sizeof(hw_vset));
      if (set->hw) {
         VSET_HW(set)->hasTex = 0;
         VSET_HW(set)->hasProg = 0;
         VSET_HW(set)->datum = datum;
         VSET_HW(set)->pb = NULL;
         _vl_rebuild_pbuffer(set);
         if (set->hw && (!VSET_HW(set)->pb)) {
            vl_free_(VL_PBUFFER,set);
            if (set->hw) FREE(set->hw);
            set->hw = NULL;
         }
      }
   }

   return;
}

void _vl_reload_textures(int palette, vset *set)
{
   if (!set->hw) return;
   if (!VSET_HW(set)->hasTex) return;

   pbActivate(VSET_HW(set)->pb);

#ifdef GLDEBUG
   printf("RJF reloading textures... %d\n",palette); 
#endif

   if (palette) {
      GLclampf r,g,b,a;
      unsigned int *tmp;

      tmp = (unsigned int *)malloc(256*512*4);
      if (tmp) {
         int i,j,k;
         k = 0;
         for(i=0;i<256;i++) {
         for(j=0;j<256;j++) {
            unsigned int u,v = 0;
            u = ((set->rlut[j] >>  0) & 0xff)*((set->llut[i] >>  0) & 0xff);
            v = v | (u/255);
            u = ((set->rlut[j] >>  8) & 0xff)*((set->llut[i] >>  8) & 0xff);
            v = v | ((u/255) << 8);
            u = ((set->rlut[j] >> 16) & 0xff)*((set->llut[i] >> 16) & 0xff);
            v = v | ((u/255) << 16);
            u = ((set->rlut[j] >> 24) & 0xff)*((set->llut[i] >> 24) & 0xff);
            v = v | ((u/255) << 24);
            tmp[k++] = v;
         }
         }
         for(i=0;i<128;i++) {
            for(j=0;j<256;j++) tmp[k++] = set->rlut[j];
         }
         for(i=0;i<128;i++) {
            for(j=0;j<256;j++) tmp[k++] = set->llut[j];
         }
         err_check(NULL);
         _glActiveTextureARB(GL_TEXTURE1_ARB);
         err_check("ActiveTextureARB");
         vl_colorswap4(tmp,256*512);
         glTexSubImage2D(GL_TEXTURE_2D,0,0,0,256,512,
                         GL_RGBA,GL_UNSIGNED_BYTE,tmp);
         err_check("TexSubImage2D");

         free(tmp);
      }

      /* and the background color */
      r = ((float)((set->backpack >>  0) & 0xff)) / 255;
      g = ((float)((set->backpack >>  8) & 0xff)) / 255;
      b = ((float)((set->backpack >> 16) & 0xff)) / 255;
      a = 0.0;
      glClearColor(r,g,b,a);

      set->dirty_flags &= ~VL_DIRTY_PALETTE;

   } else {
      unsigned char *tmp;
      int num,d2,i,j,k,count,size[3];

      d2 = 0;
      for(i=0;i<set->d[2];i+=(set->num_interp+1)) d2 += 1;
      num = set->d[0]*set->d[1];

      /* note:if volume size/format changed, may need _vl_rebuild_textures() */
      count = 1;
      if (set->aux_data[1]) count++;
      if (set->aux_data[2]) count++;
      if (set->aux_data[3]) count++;
      size[0] = 1; while(size[0] < set->d[0]) size[0] *= 2;
      size[1] = 1; while(size[1] < set->d[1]) size[1] *= 2;
      size[2] = 1; while(size[2] < d2) size[2] *= 2;
      if ((count != VSET_HW(set)->nchan) || 
          (size[0] != VSET_HW(set)->tsize[0]) ||
          (size[1] != VSET_HW(set)->tsize[1]) || 
          (size[2] != VSET_HW(set)->tsize[2])) {
         _vl_rebuild_textures(1,set);
         _vl_rebuild_textures(0,set);
      }

      /* interleave the datasets */
      tmp = (unsigned char *)malloc(num*d2*count);
      if (tmp) {
         for(i=0;i<count;i++) {
            unsigned char *dst = tmp+i;
            unsigned char *src = set->aux_data[i];
            for(k=0;k<d2;k++) {
               for(j=0;j<num;j++) {
                  *dst = *src++;
                  dst += count;
               }
               src += (num*(set->num_interp));
            }
         }
         err_check(NULL);
         _glActiveTextureARB(GL_TEXTURE0_ARB);
         err_check("ActiveTextureARB");
#ifdef WIN32
         _glTexSubImage3D(GL_TEXTURE_3D,0,0,0,0,set->d[0],set->d[1],d2,
                         fmt_table[VSET_HW(set)->nchan],GL_UNSIGNED_BYTE,tmp);
#else
         glTexSubImage3D(GL_TEXTURE_3D,0,0,0,0,set->d[0],set->d[1],d2,
                         fmt_table[VSET_HW(set)->nchan],GL_UNSIGNED_BYTE,tmp);
#endif
         err_check("TexSubImage3D");
         free(tmp);
      }

      set->dirty_flags &= ~VL_DIRTY_VOLUME;
   }
   pbDeactivate(VSET_HW(set)->pb);
   return;
}

int _vl_rebuild_fragprog(int del, vset *set)
{
   int i;
#ifdef GLDEBUG
   printf("RJF rebuild_fragprog\n");
#endif
   if (!set->hw) return(0);
   if (del) {
      if (!VSET_HW(set)->hasProg) return(0);
      pbActivate(VSET_HW(set)->pb);
      _glDeleteProgramsARB(4,VSET_HW(set)->fragprogs);
      pbDeactivate(VSET_HW(set)->pb);
      VSET_HW(set)->hasProg = 0;
   } else {
      pbActivate(VSET_HW(set)->pb);
      err_check(NULL);
      _glGenProgramsARB(4,VSET_HW(set)->fragprogs);
      err_check("GenProgramsARB");
      for(i=0;i<4;i++) {
         _glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,
                         VSET_HW(set)->fragprogs[i]);
         err_check("BindProgramARB");
         _glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,
                         GL_PROGRAM_FORMAT_ASCII_ARB,
                         strlen(frag_strings[i]),frag_strings[i]);
         if (err_check("ProgramStringARB")) {
            printf("Error: %s\n",glGetString(GL_PROGRAM_ERROR_STRING_ARB));
         }
      }
      pbDeactivate(VSET_HW(set)->pb);
      VSET_HW(set)->hasProg = 1;
   }
   return(0);
}

void _vl_rebuild_textures(int del, vset *set)
{
   if (!set->hw) return;

   if (del) {
      if (!VSET_HW(set)->hasTex) return;
      pbActivate(VSET_HW(set)->pb);
      glDeleteTextures(2,VSET_HW(set)->textures);
      pbDeactivate(VSET_HW(set)->pb);
      VSET_HW(set)->hasTex = 0;

   } else {
      int d2,i;

      d2 = 0;
      for(i=0;i<set->d[2];i+=(set->num_interp+1)) d2 += 1;

      VSET_HW(set)->tsize[0] = 1;
      VSET_HW(set)->tsize[1] = 1;
      VSET_HW(set)->tsize[2] = 1;
      while(VSET_HW(set)->tsize[0] < set->d[0]) VSET_HW(set)->tsize[0] *= 2;
      while(VSET_HW(set)->tsize[1] < set->d[1]) VSET_HW(set)->tsize[1] *= 2;
      while(VSET_HW(set)->tsize[2] < d2) VSET_HW(set)->tsize[2] *= 2;

      pbActivate(VSET_HW(set)->pb);
      glGenTextures(2,VSET_HW(set)->textures);

      VSET_HW(set)->nchan = 1;
      if (set->aux_data[1]) VSET_HW(set)->nchan++;
      if (set->aux_data[2]) VSET_HW(set)->nchan++;
      if (set->aux_data[3]) VSET_HW(set)->nchan++;

#ifdef GLDEBUG
printf("RJF - rebuilding textures... %d %d %d %d\n",VSET_HW(set)->nchan,
       VSET_HW(set)->tsize[0],VSET_HW(set)->tsize[1],VSET_HW(set)->tsize[2]);
#endif

      err_check(NULL);
      _glActiveTextureARB(GL_TEXTURE0_ARB);
      err_check("glActiveTextureARB");
      glBindTexture(GL_TEXTURE_3D, VSET_HW(set)->textures[0]);
      err_check("glBindTexture");
#ifdef WIN32
      _glTexImage3D(
#else
      glTexImage3D(
#endif
                   GL_TEXTURE_3D, 0, int_fmt_table[VSET_HW(set)->nchan], 
                   VSET_HW(set)->tsize[0], VSET_HW(set)->tsize[1], 
                   VSET_HW(set)->tsize[2],
                   0, fmt_table[VSET_HW(set)->nchan], GL_UNSIGNED_BYTE, NULL);
      err_check("glTexImage3D");
      if (set->num_interp == 0) {
         glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
         glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      } else {
         glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      }
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glEnable(GL_TEXTURE_3D);
      glPixelStorei(GL_UNPACK_ALIGNMENT,1);
      glPixelStorei(GL_PACK_ALIGNMENT,1);
      glDisable(GL_COLOR_TABLE);

      _glActiveTextureARB(GL_TEXTURE1_ARB);
      glBindTexture(GL_TEXTURE_2D,VSET_HW(set)->textures[1]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 512, 0, GL_RGBA, 
                   GL_UNSIGNED_BYTE, NULL );
      err_check("glTexImage2D");
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glEnable(GL_TEXTURE_2D);
      glPixelStorei(GL_UNPACK_ALIGNMENT,1);
      glPixelStorei(GL_PACK_ALIGNMENT,1);
      glDisable(GL_COLOR_TABLE);

      set->dirty_flags = VL_DIRTY_PALETTE|VL_DIRTY_VOLUME;
      VSET_HW(set)->hasTex = 1;
      pbDeactivate(VSET_HW(set)->pb);
   }
   return;
}

void _vl_rebuild_pbuffer(vset *set)
{
   if (!set->hw) return;

#ifdef GLDEBUG
   printf("RJF rebuild_pbuffer\n");
#endif

   if (VSET_HW(set)->pb) {
      int *d;
      d = pbGetdims(VSET_HW(set)->pb);
      if ((d[0] != set->imagex) || (d[1] != set->imagey)) {
         _vl_rebuild_textures(1,set);
         _vl_rebuild_fragprog(1,set);
         pbDestroy(VSET_HW(set)->pb);
         VSET_HW(set)->pb = NULL;
      }
   }
   if (!VSET_HW(set)->pb) {
      int dims[2];
      dims[0] = set->imagex;
      dims[1] = set->imagey;
      if (dims[0] <= 0) dims[0] = 256;
      if (dims[1] <= 0) dims[1] = 256;

#ifdef osx
      VSET_HW(set)->pb=pbCreate(dims,8,16,0);
#elif WIN32
      VSET_HW(set)->pb=pbCreate((HWND)(VSET_HW(set)->datum),dims,8,24,0);
      if (!VSET_HW(set)->pb) {
         VSET_HW(set)->pb=pbCreate((HWND)(VSET_HW(set)->datum),dims,8,16,0);
      }
#else
      VSET_HW(set)->pb=pbCreate((Display *)(VSET_HW(set)->datum),dims,8,16,0);
#endif
      if (!VSET_HW(set)->pb) {
         vl_free_(VL_PBUFFER,set);
      } else {
         _vl_rebuild_textures(0,set);
         if (_vl_rebuild_fragprog(0,set)) {
            vl_free_(VL_PBUFFER,set);
            return;
         }
         pbActivate(VSET_HW(set)->pb);
         glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
         pbDeactivate(VSET_HW(set)->pb);
      }
   }
   return;
}

void vl_free_(int flags, vset *set)
{
   if ((flags & VL_PBUFFER) && set->hw) {
      _vl_rebuild_textures(1,set);
      _vl_rebuild_fragprog(1,set);

      if (VSET_HW(set)->pb) pbDestroy(VSET_HW(set)->pb);
      free(set->hw);
      set->hw = NULL;
   }
   return;
}

void vl_query_(int what, int *value, vset *set)
{
   *value = 0;
   switch(what) {
      case VL_QUERY_HW:
         if (set->hw) *value = 1;
         break;
      case VL_QUERY_HW_TYPE:
         if (set->hw) *value = pbHardware(VSET_HW(set)->pb);
         break;
   
   }
   return;
}
#endif


#ifdef MAIN

int main(int argc,char **argv)
{
    PBuffer *pb;
    int dims[2] = {100,100};
    char *hw_table[3] = {"None","ATI","nVidia"};

    /* simple 8bit case */
    pb = pbCreate(NULL,dims,8,0,0);
    if (!pbValid(pb)) {  
	printf("Unable to create 8bit pbuffer\n");
    } else {
        printf("Detected Hardware: %s\n",hw_table[pbHardware(pb)]);
        printf("Successful 8bit pbuffer\n");
    }
    pbDestroy(pb);

    /* simple 8bit+Z case */
    pb = pbCreate(NULL,dims,8,16,0);
    if (!pbValid(pb)) {  
	printf("Unable to create 8bit + 16Z pbuffer\n");
    } else {
        printf("Successful 8bit + 16Z pbuffer\n");
    }
    pbDestroy(pb);

    /* simple 8bit+24Z case */
    pb = pbCreate(NULL,dims,8,24,0);
    if (!pbValid(pb)) {  
	printf("Unable to create 8bit + 24Z pbuffer\n");
    } else {
        printf("Successful 8bit + 24Z pbuffer\n");
    }
    pbDestroy(pb);

    /* 16bit float case (some nvidia) */
    pb = pbCreate(NULL,dims,16,0,0);
    if (!pbValid(pb)) {  
	printf("Unable to create 16bit float pbuffer\n");
    } else {
        printf("Successful 16bit float pbuffer\n");
    }
    pbDestroy(pb);

    /* 32bit float case (some ATI and some nvidia) */
    pb = pbCreate(NULL,dims,32,0,0);
    if (!pbValid(pb)) {  
	printf("Unable to create 32bit float pbuffer\n");
    } else {
        printf("Successful 32bit float pbuffer\n");
    }
    pbDestroy(pb);

    exit(0);

    return(0);
}

#endif
