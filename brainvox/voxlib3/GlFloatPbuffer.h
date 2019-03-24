#ifndef _GL_FLOAT_PBUFFER_H_
#define _GL_FLOAT_PBUFFER_H_

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

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>

#ifndef WGL_ARB_pbuffer
DECLARE_HANDLE(HPBUFFERARB);
#endif

#else
#include <GL/gl.h>
#include <GL/glx.h>
#endif

//-------------------------------------------------------------------
//
//    A C++ wrapper around OpenGL floating-point pbuffers.  It
//    creates a pbuffer with no stencil and no Z, so it's really
//    only useful for 2D rendering.  
//
//    Based on the simple_float_pbuffer OpenGL demo in NVIDIA's Cg
//    SDK.
//
//    Currently only works with glx.
//
//-------------------------------------------------------------------

#define PBUFFER_HW_NONE		0
#define PBUFFER_HW_ATI		1
#define PBUFFER_HW_NVIDIA	2

class GlFloatPbuffer
{
  public:

    //--------------------------------------------------------------
    // Constructor.
    //
    // Creates an N-bit floating point pbuffer with the specified
    // dimensions.
    //
    //--------------------------------------------------------------
#ifdef WIN32
    GlFloatPbuffer (HWND win,int *dims,int depth,
		int zdepth=0,int sdepth=0);
#else
    GlFloatPbuffer (Display *display,int *dims,int depth,
		int zdepth=0,int sdepth=0);
#endif
    virtual ~GlFloatPbuffer ();


    //---------------------------
    // The pbuffer's dimensions.
    //---------------------------
    virtual int *getdims(void) { return(_dim); };
    virtual int getdepth(void) { return(_depth); };
    virtual int getsdepth(void) { return(_sdepth); };
    virtual int getzdepth(void) { return(_zdepth); };

    //--------------------------------------------------------------
    // Make the pbuffer the current GL context and drawable.
    // Subsequent OpenGL calls will apply to this pbuffer.
    //--------------------------------------------------------------
    virtual void       activate (void);

    //-----------------------------------------------------------
    // Restore the GL context and drawable that were in effect
    // prior to the last activate () call.
    //-----------------------------------------------------------
    virtual void       deactivate(void);

    //-----------------------------------------------------------
    // True if the pBuffer could be successfully created.
    //-----------------------------------------------------------
    virtual bool       valid(void) { 
                          if (_context) return(true);
                          return(false); 
                          };

    //-----------------------------------------------------------
    // Hardware identification
    //-----------------------------------------------------------
    virtual int         hardware(void) { return(_hw_detect); };

  protected:

    int		     _dim[2];
    int		     _depth;
    int		     _sdepth;
    int		     _zdepth;
    int              _hw_detect;

#ifndef WIN32
    Display *        _display;
    GLXPbuffer       _pbuffer;
    GLXContext       _context;
    bool	     _needCloseDisplay;

    Display *        _prevDisplay;
    GLXPbuffer       _prevDrawable;
    GLXContext       _prevContext;
#else
    HPBUFFERARB      _pbuffer;
    HDC              _pbufdc;
    HGLRC	     _context;

    HDC		     _prevDC;
    HGLRC	     _prevContext;
#endif
};

#endif // _GL_FLOAT_PBUFFER_H_
