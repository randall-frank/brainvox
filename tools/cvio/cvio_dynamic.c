/*
 * Copyright (c), 1990-2005 Randall Frank and Thomas Grabowski
 *
 * $Id $
 * $HeadURL $
 *
 *                University of Iowa, Department of Neurology,
 *                Human Neuroanatomy and Neuroimaging Lab
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by Randall Frank, Thomas
 *      Grabowski and other unpaid contributors.
 * 5. The names of Randall Frank or Thomas Grabowski may not be used to
 *    endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY RANDALL FRANK AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL RANDALL FRANK OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include "cvio_dynamic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef MAC
#include <mach-o/dyld.h>
#else
#include <dlfcn.h>
#endif 

shlib_handle shlib_open(const char *f, int flags)
{
  shlib_handle handle;

#ifdef WIN32
  handle=LoadLibrary(f);

#elif defined MAC
  NSObjectFileImage image;
  NSObjectFileImageReturnCode dsoerr;

  handle = (void*)NULL;

  dsoerr = NSCreateObjectFileImageFromFile(f, &image);
  if (dsoerr == NSObjectFileImageSuccess) {
    handle = NSLinkModule(image, f, 
			   NSLINKMODULE_OPTION_RETURN_ON_ERROR |
			   NSLINKMODULE_OPTION_PRIVATE);
    NSDestroyObjectFileImage(image);
  }

#else
  if (flags & SHLIB_FLAG_LAZY)
    handle=dlopen(f, RTLD_LAZY);
  else
    handle=dlopen(f, RTLD_NOW);
  if (!handle) fprintf(stderr,"dlopen: %s\n",dlerror());
#endif

  return(handle);
}

void shlib_close(shlib_handle handle) 
{
  if (!handle) return;

#ifdef WIN32
  FreeLibrary(handle);

#elif defined MAC
  NSUnLinkModule(handle, FALSE);

#else
  dlclose(handle);
#endif

  return;
}


void *shlib_getsym(shlib_handle handle, const char *name) 
{
  void *p = (void *)0;

  if (!handle) return(p);
  
#ifdef WIN32
  p = (void *)GetProcAddress(handle, name);

#elif defined MAC
{
  NSSymbol symbol;
  char *_symbol = (char *)malloc(strlen(name)+2);
  strcpy(_symbol, "_");
  strcat(_symbol, name);  

  symbol = NSLookupSymbolInModule((NSModule)handle, _symbol);
  if (!symbol) {
    symbol = NSLookupSymbolInModule((NSModule)handle, name);
    if (!symbol) return(p);
  }

  p = (void *)NSAddressOfSymbol(symbol);
}
#else
  p = (void *)dlsym(handle, name);
#endif

  return(p);
}
