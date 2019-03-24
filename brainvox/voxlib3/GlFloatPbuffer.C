///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2003, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GlFloatPbuffer.h"

//----------------------------------------------------------------------
//  Vendor specific headers, here to make cross platform cleaner...
//----------------------------------------------------------------------
#ifdef WIN32

// From wglext.h:  WGL_ARB_pbuffer
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

// From: wglati.h
#ifndef WGL_ATI_pixel_format_float
#define WGL_ATI_pixel_format_float  1

#define WGL_TYPE_RGBA_FLOAT_ATI             0x21A0
#define GL_TYPE_RGBA_FLOAT_ATI              0x8820
#define GL_COLOR_CLEAR_UNCLAMPED_VALUE_ATI  0x8835
#endif

// function prototypes...

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

// dispatch table...
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

// From:  glxATI.h
#ifndef GLX_ATI_pixel_format_float
#define GLX_ATI_pixel_format_float  1
                                                                                
#define GLX_RGBA_FLOAT_ATI_BIT          0x00000100
                                                                                
#endif // GLX_ATI_pixel_format_float


// From:  glxtokens.h (nVidia's version)
#ifndef GLX_NV_float_buffer
#define GLX_NV_float_buffer 1

#define GLX_FLOAT_COMPONENTS_NV         0x20B0

#endif // GLX_NV_float_buffer

#endif

#ifdef WIN32

HWND wnd = NULL;
HDC dc = NULL;
HGLRC rc = NULL;

bool WindowsCreateCtx(void) 
{
    int pixelformat;

    WNDCLASS wc;
    PIXELFORMATDESCRIPTOR pfd;
    /* register window class */
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpfnWndProc = DefWindowProc;
    wc.lpszClassName = "Pbuffer";
    if (!RegisterClass(&wc)) return(false);
    /* create window */
    wnd = CreateWindow("Pbuffer", "pBuffer", 0, CW_USEDEFAULT, CW_USEDEFAULT, 
	CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandle(NULL), NULL);
    if (!wnd) return(false);
    /* get the device context */
    dc = GetDC(wnd);
    if (!dc) return(false);
    /* find pixel format */
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pixelformat = ChoosePixelFormat(dc, &pfd);
    if (pixelformat == 0) return(false);

    /* set the pixel format for the dc */
    if (!SetPixelFormat(dc, pixelformat, &pfd)) return(false);
    /* create rendering context */
    rc = wglCreateContext(dc);
    if (!rc) return(false);
    if (!wglMakeCurrent(dc, rc)) return(false);
	return(true);
}
void WindowsDestroyCtx(void)
{
    if (rc) wglMakeCurrent(NULL, NULL);
    if (rc) wglDeleteContext(wglGetCurrentContext());
    if (wnd && NULL != dc) ReleaseDC(wnd, dc);
    if (wnd) DestroyWindow(wnd);
    UnregisterClass("Pbuffer", GetModuleHandle(NULL));
}

GlFloatPbuffer::GlFloatPbuffer (HWND win, int *dim, int depth, 
	int zdepth, int sdepth) :
    _prevDC (0),
    _prevContext (0),
    _context (0),
    _pbuffer (0),
    _pbufdc (0)
{
    _depth = depth;
    _zdepth = zdepth;
    _sdepth = sdepth;
    _dim[0] = dim[0];
    _dim[1] = dim[1];
    _hw_detect = PBUFFER_HW_NONE;

    // get the device
    HDC hdc = GetDC(win);

    // grab the previous
    _prevContext = wglGetCurrentContext();
    _prevDC = wglGetCurrentDC();

    // Get the proc addresses...
    if (!_wglGetExtensionsStringARB) {
    	if (!WindowsCreateCtx()) return;

    	_wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    	_wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC)wglGetProcAddress("wglCreatePbufferARB");
    	_wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC)wglGetProcAddress("wglGetPbufferDCARB");
    	_wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)wglGetProcAddress("wglReleasePbufferDCARB");
    	_wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC)wglGetProcAddress("wglDestroyPbufferARB");
    	_wglQueryPbufferARB = (PFNWGLQUERYPBUFFERARBPROC)wglGetProcAddress("wglQueryPbufferARB");
    	_wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
    	_wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribfvARB");
    	_wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

	WindowsDestroyCtx();

    	if (!_wglGetExtensionsStringARB) return;
    	if (!_wglCreatePbufferARB) return;
    }

    // Check HW
    const char *ext = _wglGetExtensionsStringARB(hdc);
    if (strstr(ext,"WGL_ATI_pixel_format_float")) _hw_detect=PBUFFER_HW_ATI;
    if (strstr(ext,"WGL_NV_float_buffer")) _hw_detect=PBUFFER_HW_NVIDIA;

    // Attribute lists
    int fbAttrList[100] = {0}, pbAttrList[100] = {0};
    float flist[10] = {0};
    int i = 0;
    fbAttrList[i++] = WGL_DOUBLE_BUFFER_ARB;	fbAttrList[i++] = 0;
    fbAttrList[i++] = WGL_DRAW_TO_PBUFFER_ARB;	fbAttrList[i++] = 1;
    fbAttrList[i++] = WGL_SUPPORT_OPENGL_ARB;	fbAttrList[i++] = 1;    
    fbAttrList[i++] = WGL_RED_BITS_ARB;		fbAttrList[i++] = _depth;
    fbAttrList[i++] = WGL_GREEN_BITS_ARB;	fbAttrList[i++] = _depth;
    fbAttrList[i++] = WGL_BLUE_BITS_ARB;	fbAttrList[i++] = _depth;
    fbAttrList[i++] = WGL_ALPHA_BITS_ARB;	fbAttrList[i++] = _depth;
    fbAttrList[i++] = WGL_STENCIL_BITS_ARB;	fbAttrList[i++] = _sdepth;
    fbAttrList[i++] = WGL_DEPTH_BITS_ARB;	fbAttrList[i++] = _zdepth;

    if ((_hw_detect == PBUFFER_HW_ATI) && (_depth > 8)) {
	fbAttrList[i++] = WGL_PIXEL_TYPE_ARB;
	fbAttrList[i++] = WGL_TYPE_RGBA_FLOAT_ATI;
    } else if ((_hw_detect == PBUFFER_HW_NVIDIA) && (_depth > 8)) {
	fbAttrList[i++] = WGL_PIXEL_TYPE_ARB;
	fbAttrList[i++] = WGL_TYPE_RGBA_ARB;
	fbAttrList[i++] = WGL_FLOAT_COMPONENTS_NV;
       	fbAttrList[i++] = 1;
    } else {
	fbAttrList[i++] = WGL_PIXEL_TYPE_ARB;
	fbAttrList[i++] = WGL_TYPE_RGBA_ARB;
    }
    fbAttrList[i++] = 0;

    // set up the Pbuffer config list
    i = 0;
//    pbAttrList[i++] = WGL_PBUFFER_LARGEST_ARB;    pbAttrList[i++] = 1;
    pbAttrList[i++] = 0;

    int pformats[10];
    unsigned int num;

    // Pick the pixel format
    if (!_wglChoosePixelFormatARB(hdc, fbAttrList, flist, 
	sizeof(pformats)/sizeof(pformats[0]), pformats, &num)) {
	ReleaseDC(win,hdc);
	return;
    }

    // Create the pbuffer object
    _pbuffer = _wglCreatePbufferARB(hdc, pformats[0], _dim[0], _dim[1], 
		pbAttrList);
    if (!_pbuffer) {
	ReleaseDC(win,hdc);
	return;
    }

    // We don't need this anymore
    ReleaseDC(win,hdc);

    // Get the device context for the pbuffer
    _pbufdc = _wglGetPbufferDCARB(_pbuffer);
    if (!_pbufdc) {
	return;
    }

    // Create an OpenGL context
    _context = wglCreateContext(_pbufdc);
    if (!_context) {
	return;
    }

    // Share display lists/etc with the current (if any)
    if (_prevContext) wglShareLists(_context,_prevContext);

    // Make us current
    wglMakeCurrent(_pbufdc,_context);
}
#else

GlFloatPbuffer::GlFloatPbuffer(Display *display, int *dim, int depth, 
	int zdepth, int sdepth) 
{
    // Deal with the Display *
    _display = display;
    _needCloseDisplay = false;
    if (!_display) {
        _display = XOpenDisplay(NULL);
        _needCloseDisplay = true;
    }
    int screen = DefaultScreen (_display);

    // Init some stuff...
    _context = NULL;
    _pbuffer = 0;
    _prevDisplay = NULL;
    _prevContext = NULL;
    _prevDrawable  = None;
    _depth = depth;
    _zdepth = zdepth;
    _sdepth = sdepth;
    _dim[0] = dim[0];
    _dim[1] = dim[1];

    // get the display + current GLX products
    _prevDisplay  = glXGetCurrentDisplay ();
    if (!_prevDisplay) _prevDisplay = _display;
    _prevDrawable = glXGetCurrentDrawable ();
    _prevContext  = glXGetCurrentContext ();

    // Check HW
    _hw_detect = PBUFFER_HW_NONE;
    const char *ext = glXGetClientString(_display, GLX_EXTENSIONS);
    if (strstr(ext,"GLX_ATI_pixel_format_float")) _hw_detect = PBUFFER_HW_ATI;
    if (strstr(ext,"GLX_NV_float_buffer")) _hw_detect = PBUFFER_HW_NVIDIA;

    // set up the FBConfig list
    int fbAttrList[100],pbAttrList[100];
    int i = 0;
    fbAttrList[i++] = GLX_RED_SIZE;      fbAttrList[i++] = _depth;
    fbAttrList[i++] = GLX_GREEN_SIZE;    fbAttrList[i++] = _depth;
    fbAttrList[i++] = GLX_BLUE_SIZE;     fbAttrList[i++] = _depth;
    fbAttrList[i++] = GLX_ALPHA_SIZE;    fbAttrList[i++] = _depth;
    fbAttrList[i++] = GLX_STENCIL_SIZE;  fbAttrList[i++] = _sdepth;
    fbAttrList[i++] = GLX_DEPTH_SIZE;    fbAttrList[i++] = _zdepth;
    fbAttrList[i++] = GLX_DOUBLEBUFFER;  fbAttrList[i++] = false;
    fbAttrList[i++] = GLX_DRAWABLE_TYPE; fbAttrList[i++] = GLX_PBUFFER_BIT;
    if ((_hw_detect == PBUFFER_HW_ATI) && (_depth > 8)) {
        fbAttrList[i++] = GLX_RENDER_TYPE; 
        fbAttrList[i++] = GLX_RGBA_FLOAT_ATI_BIT;
    } else if ((_hw_detect == PBUFFER_HW_NVIDIA) && (_depth > 8)) {
        fbAttrList[i++] = GLX_RENDER_TYPE;
        fbAttrList[i++] = GLX_RGBA_BIT;
        fbAttrList[i++] = GLX_FLOAT_COMPONENTS_NV;
        fbAttrList[i++] = true;
    } else {
        fbAttrList[i++] = GLX_RENDER_TYPE;
        fbAttrList[i++] = GLX_RGBA_BIT;
    }
    fbAttrList[i++] = None;

    // set up the Pbuffer config list
    i = 0;
    pbAttrList[i++] = GLX_PBUFFER_WIDTH;      pbAttrList[i++] = _dim[0];
    pbAttrList[i++] = GLX_PBUFFER_HEIGHT;     pbAttrList[i++] = _dim[1];
//    pbAttrList[i++] = GLX_LARGEST_PBUFFER;    pbAttrList[i++] = true;
    pbAttrList[i++] = GLX_PRESERVED_CONTENTS; pbAttrList[i++] = true;
    pbAttrList[i++] = None;

    // Choose a configuration
    int num;
    GLXFBConfig *config = glXChooseFBConfig(_display,screen,fbAttrList,&num);
    if (!config) return;

    // create the Pbuffer
    _pbuffer = glXCreatePbuffer(_display, config[0], pbAttrList);
    if (!_pbuffer) {
	XFree(config);
	return;
    }

    // Create the context
    _context = glXCreateNewContext(_display,config[0],GLX_RGBA_TYPE,
	glXGetCurrentContext(),true);
    if (!_context) {
	XFree(config);
	return;
    }

    XFree(config);
    return;
}
#endif

GlFloatPbuffer::~GlFloatPbuffer ()
{
#ifdef WIN32
    wglMakeCurrent(NULL,NULL);

    if (_context) wglDeleteContext(_context);
    if (_pbufdc) _wglReleasePbufferDCARB(_pbuffer,_pbufdc);
    if (_pbuffer) _wglDestroyPbufferARB(_pbuffer);

    if (_prevContext && _prevDC) {
	wglMakeCurrent(_prevDC, _prevContext);
    }
#else
    glXMakeCurrent(_display, None, None);

    if (_context && _display) glXDestroyContext(_display, _context);
    if (_pbuffer && _display) glXDestroyPbuffer(_display, _pbuffer);

    //
    // Restore previous context, if valid.
    //
    
    if (_prevDisplay && (_prevDrawable != None) && _prevContext) {
	glXMakeCurrent (_prevDisplay, _prevDrawable, _prevContext);
    }
    if (_needCloseDisplay) XCloseDisplay(_display);
#endif
}


void
GlFloatPbuffer::activate ()
{
    //
    // Save current context.
    //
#ifdef WIN32
    _prevContext = wglGetCurrentContext();
    _prevDC = wglGetCurrentDC();

    wglMakeCurrent(_pbufdc, _context);
#else
    _prevDisplay  = glXGetCurrentDisplay ();
    _prevDrawable = glXGetCurrentDrawable ();
    _prevContext  = glXGetCurrentContext ();

    glXMakeCurrent (_display, _pbuffer, _context);
#endif
}


void
GlFloatPbuffer::deactivate ()
{
    //
    // Restore previous context, if valid.
    //
#ifdef WIN32
    if (_prevContext && _prevDC) {
	wglMakeCurrent(_prevDC, _prevContext);
    }
#else
    if (_prevDisplay && (_prevDrawable != None) && _prevContext) {
	glXMakeCurrent (_prevDisplay, _prevDrawable, _prevContext);
    }
#endif
}

#ifdef MAIN

int main(int argc,char **argv)
{
	GlFloatPbuffer *pb;
	int dims[2] = {100,100};
        char *hw_table[3] = {"None","ATI","nVidia"};

	// simple 8bit case
	pb = new GlFloatPbuffer(NULL,dims,8,0,0);
	if (!pb->valid()) {
		printf("Unable to create 8bit pbuffer\n");
	} else {
        printf("Detected Hardware: %s\n",hw_table[pb->hardware()]);
        printf("Successful 8bit pbuffer\n");
    }
	delete pb;

	// simple 8bit+Z case
	pb = new GlFloatPbuffer(NULL,dims,8,16,0);
	if (!pb->valid()) {
		printf("Unable to create 8bit pbuffer\n");
	} else {
        printf("Successful 8bit + 16Z pbuffer\n");
    }
	delete pb;

	// 16bit float case (some nvidia)
	pb = new GlFloatPbuffer(NULL,dims,16,0,0);
	if (!pb->valid()) {
		printf("Unable to create 16bit float pbuffer\n");
	} else {
        printf("Successful 16bit float pbuffer\n");
    }
	delete pb;

	// 32bit float case (some ATI and some nvidia)
	pb = new GlFloatPbuffer(NULL,dims,32,0,0);
	if (!pb->valid()) {
		printf("Unable to create 32bit float pbuffer\n");
	} else {
        printf("Successful 32bit float pbuffer\n");
    }
	delete pb;

	exit(0);

	return(0);
}

#endif
