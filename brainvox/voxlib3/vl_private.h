
#ifndef __VOXEL_PRIVATE_H__
#define __VOXEL_PRIVATE_H__

#include "iafdefs.h"
#include "vl_pbuffer.h"
#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN32
#ifndef APIENTRY
#define APIENTRY
#endif
#endif

/***************** Cross Platform OpenGL extensions *******************/
#ifndef GL_EXT_texture_rectangle
#define GL_EXT_texture_rectangle

#define GL_TEXTURE_RECTANGLE_EXT          0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_EXT  0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_EXT    0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT 0x84F8

#endif

#ifndef GL_ARB_fragment_program
#define GL_ARB_fragment_program 1

#define GL_FRAGMENT_PROGRAM_ARB           0x8804
#define GL_PROGRAM_ALU_INSTRUCTIONS_ARB   0x8805
#define GL_PROGRAM_TEX_INSTRUCTIONS_ARB   0x8806
#define GL_PROGRAM_TEX_INDIRECTIONS_ARB   0x8807
#define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x8808
#define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x8809
#define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x880A
#define GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB 0x880B
#define GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB 0x880C
#define GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB 0x880D
#define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x880E
#define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x880F
#define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x8810
#define GL_MAX_TEXTURE_COORDS_ARB         0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB    0x8872

/* Note: this is an incomplete list, only the ones used by voxlib */
#ifndef osx
typedef void (APIENTRYP PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (APIENTRYP PFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (APIENTRYP PFNGLDELETEPROGRAMSARBPROC) (GLsizei n, const GLuint *programs);
typedef void (APIENTRYP PFNGLGENPROGRAMSARBPROC) (GLsizei n, GLuint *programs);
typedef void (APIENTRYP PFNGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
typedef void (APIENTRYP PFNGLPROGRAMSTRINGARBPROC) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef void (APIENTRYP PFNGLPROGRAMENVPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
typedef void (APIENTRYP PFNGLPROGRAMENVPARAMETER4DVARBPROC) (GLenum target, GLuint index, const GLdouble *params);

#ifdef WIN32
typedef void (APIENTRYP PFNGLTEXIMAGE3DPROC)( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLTEXSUBIMAGE3DPROC)( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
#endif

#endif 

#endif



/* hardware bits */
/* pbuffer, textures, setup bits */
typedef struct {
  PBuffer *pb;
  void *datum;
  int tsize[3];
  int nchan;
  GLuint textures[2];
  int hasTex;
  GLuint fragprogs[4]; 
  int hasProg;
} hw_vset;

/* redirection table... */
/* fragment programs */
extern PFNGLACTIVETEXTUREARBPROC _glActiveTextureARB;
extern PFNGLMULTITEXCOORD3FARBPROC _glMultiTexCoord3fARB;
extern PFNGLDELETEPROGRAMSARBPROC _glDeleteProgramsARB;
extern PFNGLGENPROGRAMSARBPROC _glGenProgramsARB;
extern PFNGLBINDPROGRAMARBPROC _glBindProgramARB;
extern PFNGLPROGRAMSTRINGARBPROC _glProgramStringARB;
extern PFNGLPROGRAMENVPARAMETER4FVARBPROC _glProgramEnvParameter4fvARB;
extern PFNGLPROGRAMENVPARAMETER4DVARBPROC _glProgramEnvParameter4dvARB;

/* 
   Setup: depthbuffering
   Fragment progs:
   single|dual alpha lighting

   Future:
   MIP
   MINP
   YIQ
   RGBA
   PLANE?
 */

#define VSET_HW(a) \
	((hw_vset*)(a)->hw)

void _vl_reload_textures(int palette, vset *set);
void _vl_rebuild_textures(int del, vset *set);
void _vl_rebuild_pbuffer(vset *set);

#ifdef __cplusplus
}
#endif

#endif
