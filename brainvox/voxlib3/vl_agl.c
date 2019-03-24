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

#ifdef osx

#include <Carbon/Carbon.h>
#include <AGL/agl.h>

/* wrappers to allow X11 and AGL OpenGL in the same app */

/* define these differently here */
typedef AGLPbuffer osxPbuffer;
typedef AGLPixelFormat osxPixelFormat;
typedef AGLContext osxContext;
typedef AGLDevice osxDevice;

/* wrappers */
osxPixelFormat osxChoosePixelFormat(const osxDevice *a,GLint b,const GLint *c)
{
#ifdef AGL_DEBUG
printf("agl CPF\n");
#endif
   return(aglChoosePixelFormat(a,b,c));
}
osxContext osxCreateContext(osxPixelFormat a,osxContext b)
{
#ifdef AGL_DEBUG
printf("agl CC\n");
#endif
   return(aglCreateContext(a,b));
}
GLboolean osxCreatePBuffer(GLint a,GLint b,GLenum c,GLenum d,long e,
                           osxPbuffer *f)
{
#ifdef AGL_DEBUG
printf("agl CPB\n");
#endif
   return(aglCreatePBuffer(a,b,c,d,e,f));
}
void osxDestroyPixelFormat(osxPixelFormat a) 
{
#ifdef AGL_DEBUG
printf("agl DPF\n");
#endif
   aglDestroyPixelFormat(a);
}
GLboolean osxDestroyContext(osxContext a)
{
#ifdef AGL_DEBUG
printf("agl DC\n");
#endif
   return(aglDestroyContext(a));
}
GLboolean osxDestroyPBuffer(osxPbuffer a)
{
#ifdef AGL_DEBUG
printf("agl DPB\n");
#endif
   return(aglDestroyPBuffer(a));
}
GLboolean osxSetCurrentContext(osxContext a) 
{
#ifdef AGL_DEBUG
printf("agl SCC\n");
#endif
   return(aglSetCurrentContext(a));
}
GLboolean osxSetPBuffer(osxContext a,osxPbuffer b,GLint c,GLint d,GLint e)
{
#ifdef AGL_DEBUG
printf("agl SPB\n");
#endif
   return(aglSetPBuffer(a,b,c,d,e));
}
GLint osxGetVirtualScreen(osxContext a)
{
#ifdef AGL_DEBUG
printf("agl GVS\n");
#endif
   return(aglGetVirtualScreen(a));
}

CFBundleRef gBundleRefOpenGL = NULL;

OSStatus aglInitEntryPoints(void)
{
    OSStatus err = noErr;
    Str255 frameworkName;
    FSRefParam fileRefParam;
    FSRef fileRef;
    CFURLRef bundleURLOpenGL;

    if (gBundleRefOpenGL) return(noErr);

    frameworkName[0] = strlen("OpenGL.framework");
    strcpy(frameworkName+1,"OpenGL.framework");

    memset(&fileRefParam, 0, sizeof(fileRefParam));

    /* Frameworks directory/folder */
    err = FindFolder (kSystemDomain, kFrameworksFolderType, false,
                      &fileRefParam.ioVRefNum, &fileRefParam.ioDirID);
    if (noErr != err) {
        DebugStr ("\pCould not find frameworks folder");
        return err;
    }

    memset(&fileRef, 0, sizeof(fileRef));

    fileRefParam.ioNamePtr  = frameworkName;
    fileRefParam.newRef = &fileRef;

    err = PBMakeFSRefSync (&fileRefParam); /* make FSRef for folder */
    if (noErr != err) {
        DebugStr ("\pCould not make FSref to frameworks folder");
        return err;
    }
    /* create URL to folder */
    bundleURLOpenGL = CFURLCreateFromFSRef (kCFAllocatorDefault,
                                            &fileRef);
    if (!bundleURLOpenGL) {
        DebugStr ("\pCould create OpenGL Framework bundle URL");
        return paramErr;
    }
    /* create ref to GL's bundle */
    gBundleRefOpenGL = CFBundleCreate (kCFAllocatorDefault,
                                       bundleURLOpenGL);
    if (!gBundleRefOpenGL) {
        DebugStr ("\pCould not create OpenGL Framework bundle");
        return paramErr;
    }
    CFRelease (bundleURLOpenGL); /* release created bundle */
    /* if the code was successfully loaded, look for our function. */
    if (!CFBundleLoadExecutable (gBundleRefOpenGL)) {
        DebugStr ("\pCould not load MachO executable");
        return paramErr;
    }
    return err;
}

void aglDellocEntryPoints(void)
{
    if (gBundleRefOpenGL != NULL) {
        CFBundleUnloadExecutable (gBundleRefOpenGL);
        CFRelease (gBundleRefOpenGL);
        gBundleRefOpenGL = NULL;
    }
}

void *aglGetProcAddress(char * pszProc)
{
    aglInitEntryPoints();
    if (!gBundleRefOpenGL) return(NULL);
    return CFBundleGetFunctionPointerForName (gBundleRefOpenGL,
                CFStringCreateWithCStringNoCopy (NULL,
                     pszProc, CFStringGetSystemEncoding (), NULL));
}

/* for some reason, OSX seems to get the colors swapped... */
void vl_colorswap4(void *in,int num)
{
   unsigned char *p = (unsigned char *)in;
   unsigned char t;
   int i;
   for(i=0;i<num;i++) {
      t = p[0]; p[0] = p[3]; p[3] = t;
      t = p[1]; p[1] = p[2]; p[2] = t;
      p += 4;
   }
}
void vl_colorswap2(void *in,int num)
{
   unsigned char *p = (unsigned char *)in;
   unsigned char t;
   int i;
   for(i=0;i<num;i++) {
      t = p[0]; p[0] = p[1]; p[1] = t;
      p += 2;
   }
}

#else

void vl_colorswap2(void *in,int num)
{
   return;
}
void vl_colorswap4(void *in,int num)
{
   return;
}

#endif


