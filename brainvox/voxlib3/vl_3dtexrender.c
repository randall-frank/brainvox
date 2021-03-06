#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#include "voxel.h"
#include "vl_util.h"
#include "vl_private.h"
#include "vl_parallel.h"
#include "vtkmarchingcubescases.h"
#include "vl_postlight.h"

static void CalcNewEdges(float fCurCoords[12][3],float fCurTexture[12][3],
                         float fDeltaCoords[12][3],float fDeltaTexture[12][3],
                         int iActive[12],int iIndex[18],int *nIndex,
                         float fDeltaZ,float fDepth[8],int mask,float z, 
                         float fDataCoords[8][4],float fTexCoords[8][4]);
static void CalcSlicePosOrder(float *fDeltaZ, float *fDepth, float *m, 
            int *n, float *start,float fThickness, float fDataCoords[8][4]);
static void SetSquare(float *min,float *max,float var[8][4]);

int vl_3dtexrender(par_quant *par)
{
        int i,d2;
        float a,b,sf,p[3],q[3],toff[3];

        vset *set = par->set;
        sf = set->squeeze_factor;
        d2 = 0; for(i=0;i<set->end[2];i+=(set->num_interp+1)) d2 += 1;

	/* can we do this? */
        if (!set->hw) return(VL_CANNOT_3DTEX);

        /* we only do 8bit/16bit {over} {lighting} */
        if (par->flag & VL_RENDERPLANE) return(VL_CANNOT_3DTEX);
        if (par->flag & VL_RENDERMIP) return(VL_CANNOT_3DTEX);
        if (par->flag & VL_RENDERMNIP) return(VL_CANNOT_3DTEX);
        if (par->flag & VL_RENDER24BIT) return(VL_CANNOT_3DTEX);
        if (par->flag & VL_RENDERYIQ) return(VL_CANNOT_3DTEX);

	/* do we need new pbuffers, etc? */
        if (set->dirty_flags & VL_DIRTY_IMAGE) _vl_rebuild_pbuffer(set);

	/* download textures again? */
        if (set->dirty_flags & VL_DIRTY_VOLUME) _vl_reload_textures(0,set);
        if (set->dirty_flags & VL_DIRTY_PALETTE) _vl_reload_textures(1,set);
        if (!set->hw) return(VL_CANNOT_3DTEX);

        pbActivate(VSET_HW(set)->pb);

        /* various status bits */
        glDisable(GL_BLEND);
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_DITHER);
        glDisable(GL_FOG);
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
        glDisable(GL_TEXTURE_2D);

        /* set up the frustum */
        a = set->imagex;
        b = set->imagey;

        glViewport(0,0,a,b);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glOrtho(-a*0.5,a*0.5,-b*0.5,b*0.5,-a*0.5,a*0.5);

        /* prime the buffers if overlay mode */
        if (par->flag & VL_RENDEROVER) {
           float *tmp;

           glPixelZoom(1.0,1.0);
           glRasterPos3f(-a*0.50,-b*0.50,0.);

           _glActiveTextureARB(GL_TEXTURE0_ARB);
           glDisable(GL_TEXTURE_3D);
           _glActiveTextureARB(GL_TEXTURE1_ARB);
           glDisable(GL_TEXTURE_2D);

           glDrawPixels(set->imagex,set->imagey,GL_RGBA,
                        GL_UNSIGNED_BYTE,set->image);

           tmp = (float *)MALLOC(set->imagex*set->imagey*sizeof(float));
           if (tmp) {
              /* -a*8 to a*8  maps to 0-32767 */
              a = set->imagex*0.5;
              for(i=0;i<set->imagex*set->imagey;i++) {
                 if (set->zbuffer[i] <= VL_INFINITY) {
                    b = -1.0;
                 } else {
                    b = (float)(set->zbuffer[i]) / -8.0*a;  /* -1 to 1 */
#ifdef NEVER
/* glDrawPixels (DEPTH_COMPONENT, GL_SHORT) seems broken!!! */
                    /* finally 0-32767 */
                    b = (b * 16383) + 16384;
#endif
                 }
                 tmp[i] = b;
              }
              glDrawPixels(set->imagex,set->imagey,GL_DEPTH_COMPONENT,
                        GL_FLOAT,tmp);
           }
           free(tmp);

           _glActiveTextureARB(GL_TEXTURE0_ARB);
           glEnable(GL_TEXTURE_3D);
           _glActiveTextureARB(GL_TEXTURE1_ARB);
           glEnable(GL_TEXTURE_2D);

        } else {
           glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        }

	/* render, slicers parallel to the screen */
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glAlphaFunc(GL_GREATER, 0.0);
        glEnable(GL_ALPHA_TEST);
        glEnable(GL_DEPTH_TEST);

        /* bind the fragment program(s) */
        i = 0;
        if (par->flag & VL_RENDER16BIT) i += 1;
        if (par->flag & VL_RENDERLIGHT) i += 2;
        _glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,VSET_HW(set)->fragprogs[i]);
        glEnable(GL_FRAGMENT_PROGRAM_ARB);

        /* set up the lighting params for the "LIT" frag progs */
        if (par->flag & VL_RENDERLIGHT) {
           int k,j;
           VLFLOAT64 d,temp[3][3],nin[4],nout[4];
           /* normalize light vector */
           d = (vl_light_globals.rl_x*vl_light_globals.rl_x);
           d += (vl_light_globals.rl_y*vl_light_globals.rl_y);
           d += (vl_light_globals.rl_z*vl_light_globals.rl_z);
           if (d == 0.0) d = 1.0;
           d = sqrt(d);
           nin[0] = -vl_light_globals.rl_x/d;
           nin[1] = -vl_light_globals.rl_y/d;
           nin[2] = -vl_light_globals.rl_z/d;
           /* rotate */
           for(k=0;k<3;k++) for(j=0;j<3;j++) temp[k][j] = (set->xform)[k][j];
           invert3x3(temp);
           vl_matmult(nin,nout,temp);
           /* store these for the Fragment program */
           nout[3] = 1.0;
           _glProgramEnvParameter4dvARB(GL_FRAGMENT_PROGRAM_ARB,0,nout);
           nin[0] = ((float)vl_light_globals.rl_inten)/256.;
           nin[1] = ((float)vl_light_globals.rl_srcinten)/256.;
           nin[2] = 0;
           nin[3] = 1;
           _glProgramEnvParameter4dvARB(GL_FRAGMENT_PROGRAM_ARB,1,nin);
#if 0
            /* increment in texture coords equal to one voxel... */
           nin[0] = 1.0/(double)(VSET_HW(set)->tsize[0]);
           nin[1] = 1.0/(double)(VSET_HW(set)->tsize[1]);
           nin[2] = 1.0/(double)(VSET_HW(set)->tsize[2]);
           nin[3] = 1;
           _glProgramEnvParameter4dvARB(GL_FRAGMENT_PROGRAM_ARB,2,nin);
#endif
           toff[0] = 1.0/(double)(VSET_HW(set)->tsize[0]);
           toff[1] = 1.0/(double)(VSET_HW(set)->tsize[1]);
           toff[2] = 1.0/(double)(VSET_HW(set)->tsize[2]);
        }

        /* the view */
	glTranslatef(set->trans[0],set->trans[1],set->trans[2]);
        glRotatef(set->rots[2],0.,0.,1.);
        glRotatef(set->rots[1],0.,1.,0.);
        glRotatef(set->rots[0],1.,0.,0.);

        /* center of volume transformation */
	glTranslatef(-(set->d[0])*0.5,-(set->d[1])*0.5,
                     -((set->d[2])*0.5)*sf);

        /* render the slices */
{
   int     nSlices;
   float   fCurrent;
   float   mat[16],*m;
   float   fCurCoords[12][3];
   float   fCurTexture[12][3];
   float   fDeltaCoords[12][3];
   float   fDeltaTexture[12][3];
   int     iActive[12];
   int     iIndex[3*6]; /* 6 triangles maximum */
   int     nIndex;
   float   fDeltaZ,fThick;
   float   fDepth[8];
   int     omask;
   float   fDataCoords[8][4];
   float   fTexCoords[8][4];

   glGetFloatv(GL_MODELVIEW_MATRIX, mat);
   m = mat;

   fDepth[0] = (float)(VSET_HW(set)->tsize[0]);
   fDepth[1] = (float)(VSET_HW(set)->tsize[1]);
   fDepth[2] = (float)(VSET_HW(set)->tsize[2]);

   p[0] = set->start[0]; p[1] = set->start[1]; p[2] = set->start[2]*sf;
   q[0] = set->end[0];   q[1] = set->end[1];   q[2] = set->end[2]*sf;
   SetSquare(p,q,fDataCoords);

   p[0]= set->start[0]+0.5; p[1]= set->start[1]+0.5; p[2]= set->start[2]+0.5;
   q[0]= set->end[0]+0.5;   q[1]= set->end[1]+0.5;   q[2]= d2+0.5; 
                                                     //q[2]= set->end[2]+0.5;
   p[0] /= fDepth[0];    p[1] /= fDepth[1];    p[2] /= fDepth[2];
   q[0] /= fDepth[0];    q[1] /= fDepth[1];    q[2] /= fDepth[2];
   SetSquare(p,q,fTexCoords);

   fThick = 1.0*(set->speed & 0xff);
   CalcSlicePosOrder(&fDeltaZ,fDepth,m,&nSlices,&fCurrent,fThick,fDataCoords);

   glBegin(GL_TRIANGLES);

   omask = -1;
   for(i=0;i<nSlices;i++) {
      int j,mask;

      /* see if the table has changed */
      mask = 0;
      for(j=0;j<8;j++) if (fDepth[j] > fCurrent) mask |= (1<<j);

      /* Compute new edge deltas */
      if (mask != omask) {
         CalcNewEdges(fCurCoords,fCurTexture,fDeltaCoords,fDeltaTexture,
                      iActive,iIndex,&nIndex,fDeltaZ,fDepth,mask,fCurrent,
                      fDataCoords,fTexCoords);
      }

      /* output some tris? */
      if ((mask != 0) && (mask != 255)) {
         if (par->flag & VL_RENDERLIGHT) {
            for(j=0;j<nIndex;j+=3) {
               glTexCoord3fv(fCurTexture[iIndex[j+0]]);
               _glMultiTexCoord3fARB(GL_TEXTURE1_ARB,
                  fCurTexture[iIndex[j+0]][0]+toff[0],
                  fCurTexture[iIndex[j+0]][1],
                  fCurTexture[iIndex[j+0]][2]);
               _glMultiTexCoord3fARB(GL_TEXTURE2_ARB,
                  fCurTexture[iIndex[j+0]][0],
                  fCurTexture[iIndex[j+0]][1]+toff[1],
                  fCurTexture[iIndex[j+0]][2]);
               _glMultiTexCoord3fARB(GL_TEXTURE3_ARB,
                  fCurTexture[iIndex[j+0]][0],
                  fCurTexture[iIndex[j+0]][1],
                  fCurTexture[iIndex[j+0]][2]+toff[2]);
               glVertex3fv(fCurCoords[iIndex[j+0]]);
               glTexCoord3fv(fCurTexture[iIndex[j+1]]);
               _glMultiTexCoord3fARB(GL_TEXTURE1_ARB,
                  fCurTexture[iIndex[j+1]][0]+toff[0],
                  fCurTexture[iIndex[j+1]][1],
                  fCurTexture[iIndex[j+1]][2]);
               _glMultiTexCoord3fARB(GL_TEXTURE2_ARB,
                  fCurTexture[iIndex[j+1]][0],
                  fCurTexture[iIndex[j+1]][1]+toff[1],
                  fCurTexture[iIndex[j+1]][2]);
               _glMultiTexCoord3fARB(GL_TEXTURE3_ARB,
                  fCurTexture[iIndex[j+1]][0],
                  fCurTexture[iIndex[j+1]][1],
                  fCurTexture[iIndex[j+1]][2]+toff[2]);
               glVertex3fv(fCurCoords[iIndex[j+1]]);
               glTexCoord3fv(fCurTexture[iIndex[j+2]]);
               _glMultiTexCoord3fARB(GL_TEXTURE1_ARB,
                  fCurTexture[iIndex[j+2]][0]+toff[0],
                  fCurTexture[iIndex[j+2]][1],
                  fCurTexture[iIndex[j+2]][2]);
               _glMultiTexCoord3fARB(GL_TEXTURE2_ARB,
                  fCurTexture[iIndex[j+2]][0],
                  fCurTexture[iIndex[j+2]][1]+toff[1],
                  fCurTexture[iIndex[j+2]][2]);
               _glMultiTexCoord3fARB(GL_TEXTURE3_ARB,
                  fCurTexture[iIndex[j+2]][0],
                  fCurTexture[iIndex[j+2]][1],
                  fCurTexture[iIndex[j+2]][2]+toff[2]);
               glVertex3fv(fCurCoords[iIndex[j+2]]);
            }
         } else {
            for(j=0;j<nIndex;j+=3) {
               glTexCoord3fv(fCurTexture[iIndex[j+0]]);
               glVertex3fv(fCurCoords[iIndex[j+0]]);
               glTexCoord3fv(fCurTexture[iIndex[j+1]]);
               glVertex3fv(fCurCoords[iIndex[j+1]]);
               glTexCoord3fv(fCurTexture[iIndex[j+2]]);
               glVertex3fv(fCurCoords[iIndex[j+2]]);
            }
         }
         for(j=0;j<12;j++) {
            if (iActive[j]) {
               fCurCoords[j][0] += fDeltaCoords[j][0];
               fCurCoords[j][1] += fDeltaCoords[j][1];
               fCurCoords[j][2] += fDeltaCoords[j][2];
               fCurTexture[j][0]+= fDeltaTexture[j][0];
               fCurTexture[j][1]+= fDeltaTexture[j][1];
               fCurTexture[j][2]+= fDeltaTexture[j][2];
            }
         }
      }

      /* Next slice */
      fCurrent += fDeltaZ;
      omask = mask;
   }

   glEnd();
}

	/* place the resulting bits somewhere useful */
        glReadPixels(0,0,set->imagex,set->imagey,GL_RGBA,GL_UNSIGNED_BYTE,
                     set->image);
        glReadPixels(0,0,set->imagex,set->imagey,GL_DEPTH_COMPONENT,
                     GL_SHORT,set->zbuffer);
        /* -a*8 to a*8  maps to 0-32767 */
        a = set->imagex*0.5;
        for(i=0;i<set->imagex*set->imagey;i++) {
           if (set->zbuffer[i] > -VL_INFINITY) {
              b = VL_INFINITY-1;
           } else {
              b = (float)(set->zbuffer[i]-16384)/16383.0;  /* -1 to 1 */
              /* we want 8*a */
              b *= -8.0*a;
           }
           set->zbuffer[i] = b;
        }
        vl_colorswap4(set->image,set->imagex*set->imagey);
#if 0
        for(i=0;i<set->imagex;i++)   {
           int *image = (int *)set->image;
           printf("%x %d ",image[i+(set->imagey/2)*set->imagex],
                   set->zbuffer[i+(set->imagey/2)*set->imagex]);
        }
        printf("\n");
#endif
        pbDeactivate(VSET_HW(set)->pb);

        par->ret = VL_NOERR;

	return(VL_NOERR);
}

static void XForm4(float *in,float *m,float *out)
{
float tmp[4];
#define M(row,col)  m[col*4+row]
tmp[0] = M(0,0) * in[0] + M(0,1) * in[1] + M(0,2) * in[2] + M(0,3) * in[3];
tmp[1] = M(1,0) * in[0] + M(1,1) * in[1] + M(1,2) * in[2] + M(1,3) * in[3];
tmp[2] = M(2,0) * in[0] + M(2,1) * in[1] + M(2,2) * in[2] + M(2,3) * in[3];
tmp[3] = M(3,0) * in[0] + M(3,1) * in[1] + M(3,2) * in[2] + M(3,3) * in[3];
#undef M
memcpy(out,tmp,sizeof(tmp));
return;
}

static void XForm3(float *in,float *m,float *out)
{
float tmp[3];
#define M(row,col)  m[col*4+row]
tmp[0] = M(0,0) * in[0] + M(0,1) * in[1] + M(0,2) * in[2] + M(0,3) * in[3];
tmp[1] = M(1,0) * in[0] + M(1,1) * in[1] + M(1,2) * in[2] + M(1,3) * in[3];
tmp[2] = M(2,0) * in[0] + M(2,1) * in[1] + M(2,2) * in[2] + M(2,3) * in[3];
#undef M
memcpy(out,tmp,sizeof(tmp));
return;
}

static float XFormZ(float *in,float *m)
{
float tmp;
#define M(row,col)  m[col*4+row]
tmp = M(2,0) * in[0] + M(2,1) * in[1] + M(2,2) * in[2] + M(2,3) * in[3];
#undef M
return(tmp);
}

/*
*        Z^ 7-----6
*         |/|    /|
*         4-----5 |
*         | 3---|-2
*         |/    |/
*         0-----1--> X
*/
static void SetSquare(float *min,float *max,float var[8][4])
{
        int i;
        var[0][0] = min[0]; var[0][1] = min[1]; var[0][2] = min[2];
        var[1][0] = max[0]; var[1][1] = min[1]; var[1][2] = min[2];
        var[2][0] = max[0]; var[2][1] = max[1]; var[2][2] = min[2];
        var[3][0] = min[0]; var[3][1] = max[1]; var[3][2] = min[2];
        var[4][0] = min[0]; var[4][1] = min[1]; var[4][2] = max[2];
        var[5][0] = max[0]; var[5][1] = min[1]; var[5][2] = max[2];
        var[6][0] = max[0]; var[6][1] = max[1]; var[6][2] = max[2];
        var[7][0] = min[0]; var[7][1] = max[1]; var[7][2] = max[2];
        for(i=0;i<8;i++) var[i][3] = 1.0;
}

static void CalcNewEdges(float fCurCoords[12][3],float fCurTexture[12][3],
                         float fDeltaCoords[12][3],float fDeltaTexture[12][3],
                         int iActive[12],int iIndex[18],int *nIndex,
                         float fDeltaZ,float fDepth[8], int mask,float z,
                         float fDataCoords[8][4],float fTexCoords[8][4])
{
   int e,i;

   memset(iActive,0,sizeof(iActive));

   /* Which are active */
   *nIndex = 0;
   for(e=0;triCases[mask].edges[e]!=-1;e++) {
      iActive[triCases[mask].edges[e]] = 1;
      iIndex[*nIndex] = triCases[mask].edges[e];
      *nIndex += 1;
   }

   /* Set them up */
   for(e=0;e<12;e++) {
      if (iActive[e]) {
         int v1 = edgeinfo[e].d1;
         int v2 = edgeinfo[e].d2;
         float w = (z-fDepth[v1])/(fDepth[v2]-fDepth[v1]);
         float dw = (z+fDeltaZ-fDepth[v1])/(fDepth[v2]-fDepth[v1]);
         dw = dw - w;
         for(i=0;i<3;i++) {
            float d;

            d = fDataCoords[v2][i]-fDataCoords[v1][i];
            fCurCoords[e][i] = fDataCoords[v1][i] + w*d;
            fDeltaCoords[e][i] = dw*d;

            d = fTexCoords[v2][i]-fTexCoords[v1][i];
            fCurTexture[e][i] = fTexCoords[v1][i] + w*d;
            fDeltaTexture[e][i] = dw*d;
         }
      }
   }
   return;
}

static void CalcSlicePosOrder(float *fDeltaZ, float *fDepth, float *m, 
            int *n, float *start,float fThickness, float fDataCoords[8][4])
{
   int     i;
   float   fMin,fMax;
   float   z,v1[4],v2[4],v[4];

   /* Get the "virtual" block start point in screen space */
   fDepth[0] = XFormZ(fDataCoords[0],m);
   fMin = fDepth[0];
   fMax = fDepth[0];
   for(i=1;i<8;i++) {
      fDepth[i] = XFormZ(fDataCoords[i],m);
      if (fDepth[i] < fMin) fMin = fDepth[i];
      if (fDepth[i] > fMax) fMax = fDepth[i];
   }
   *start = fMax;
   v[0] = 0.f; v[1] = 0.f; v[2] = 0.f;  v[3] = 1.f;
   XForm4(v,m,v1);
   v[2] = fThickness;
   XForm4(v,m,v2);
   z = ((v1[0]-v2[0])*(v1[0]-v2[0]));
   z += ((v1[1]-v2[1])*(v1[1]-v2[1]));
   z += ((v1[2]-v2[2])*(v1[2]-v2[2]));
   if (z <= 0.) { *n = 0; return; };
   z = (float)sqrt(z);
   *fDeltaZ = -z;
   *n = (int)(((fMax-fMin)/z)+1);

   return;
}
