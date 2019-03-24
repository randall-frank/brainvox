/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_vbin.c 1213 2005-08-27 20:51:21Z rjfrank $
 *                University of Iowa, Department of Neurology,
 *                Human Neuroanatomy and Neuroimaging Lab
 *                University of Iowa, Image Analysis Facility
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <float.h>
#ifndef WIN32
#include <unistd.h>
#else
FILE *popen(char *a, char *b);
int pclose(FILE *f);
#endif
#include <limits.h>

#include <GL/glut.h>
#include <GL/gl.h>

#include "proto.h"

/* TODO: menu: reset, zoom, pan */

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] binfile [xsize ysize [palettefile]]\n",__DATE__,TAL_VERSION,s);
        fprintf(stderr,"Options:   -pid  : echo process ID to stdout\n");
        fprintf(stderr,"          -wait  : run in foreground\n"); 
        fprintf(stderr,"        -head h  : header size in bytes\n");
        fprintf(stderr,"       -pos x y  : position LL of window at x,y\n");
/*        fprintf(stderr,"        -nobord  : use no border around window\n"); */
        fprintf(stderr,"         -z1|-8  : force as 8 bit image\n");
        fprintf(stderr,"        -z2|-16  : force as 16 bit image\n");
        fprintf(stderr,"        -z3|-24  : force as 24 bit rgb image\n");
        fprintf(stderr,"     -z4|-float  : force as binary floating point image\n");
        fprintf(stderr,"       -max max  : select max value\n");
        fprintf(stderr,"       -min min  : select min value\n");
        fprintf(stderr,"          -swap  : force image swap bytes\n");
        fprintf(stderr,"        -noswap  : force no image swap bytes\n");
        fprintf(stderr,"        -noflip  : no image flipping\n");
        fprintf(stderr,"        -pal palfile  : option form of palettefile argument\n");

	tal_exit(1);
}

#define MAX_ARGS 50

/* globals used by the display functions */
unsigned long	*image;
unsigned char	*raw_image;
int		width,height,xsize,ysize;
int		zsize = 0;  /* 0=unknown 1=8bit 2=16bit 3=24bit 4=float */
GLfloat		Xzoom,Yzoom;
int		bPixval = 0;
int		Pixloc[2];
#if defined(MESA)
int		isMesa = 1;
#else
int		isMesa = 0;
#endif

#define MENU_QUIT	0
#define MENU_ZOOM1	1
#define MENU_ZOOM2	2
#define MENU_ZOOM3	3
#define MENU_ZOOM4	4
#define MENU_FLIP	5

void menu_selected(int entry)
{
        switch(entry)
        {
		case MENU_FLIP:
			yflip(raw_image,xsize,ysize,zsize);
			yflip((unsigned char *)image,xsize,ysize,4);
			glutPostRedisplay();
			break;
		case MENU_ZOOM1:
		case MENU_ZOOM2:
		case MENU_ZOOM3:
		case MENU_ZOOM4:	
			glutReshapeWindow(entry*xsize,entry*ysize);
			break;
                case MENU_QUIT:
			free(image);
			free(raw_image);
			tal_exit(0);
                        break;
        }
}

void init_menu()
{
	int	menu;

        menu=glutCreateMenu(menu_selected);
        glutAddMenuEntry("Zoom 1.0",MENU_ZOOM1);
        glutAddMenuEntry("Zoom 2.0",MENU_ZOOM2);
        glutAddMenuEntry("Zoom 3.0",MENU_ZOOM3);
        glutAddMenuEntry("Zoom 4.0",MENU_ZOOM4);
        glutAddMenuEntry("Flip",MENU_FLIP);
        glutAddMenuEntry("Quit",MENU_QUIT);
        glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void mouse_click(int button,int state,int x,int y)
{
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			bPixval = 1;
			Pixloc[0] = x/Xzoom;
			Pixloc[1] = ysize-y/Yzoom;
			if (Pixloc[0] < 0) Pixloc[0] = 0;
			if (Pixloc[1] < 0) Pixloc[1] = 0;
			if (Pixloc[0] >= xsize) Pixloc[0] = xsize-1;
			if (Pixloc[1] >= ysize) Pixloc[1] = ysize-1;
		} else {
			bPixval = 0;
		}
		glutPostRedisplay();
	}
}

void mouse_move(int x,int y)
{
	if (bPixval) {
		Pixloc[0] = x/Xzoom;
		Pixloc[1] = ysize-y/Yzoom;
		if (Pixloc[0] < 0) Pixloc[0] = 0;
		if (Pixloc[1] < 0) Pixloc[1] = 0;
		if (Pixloc[0] >= xsize) Pixloc[0] = xsize-1;
		if (Pixloc[1] >= ysize) Pixloc[1] = ysize-1;
		glutPostRedisplay();
	}
}

void key(unsigned char c, int x, int y)
{
  	switch (c) {
  		case 27:
			free(image);
			free(raw_image);
			tal_exit(0);
			break;
  	}
	return;
}

void reshape(int wid, int hei)
{
	width = wid;
	height = hei;

	glViewport(0,0,width,height);

    	glMatrixMode(GL_PROJECTION);
   	glLoadIdentity();
	glOrtho(0,width-1,0,height-1,-1,1);
    	glMatrixMode(GL_MODELVIEW);
   	glLoadIdentity();
	
	Xzoom = ((float)width)/((float)xsize);
	Yzoom = ((float)height)/((float)ysize);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glRasterPos2i(0, 0);
	glPixelZoom( Xzoom, Yzoom );

	glColor3ub(0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);

	return;
}

void redraw(void)
{	
	glColor3ub(0,0,0);
	if (isMesa) glClear(GL_COLOR_BUFFER_BIT);

	glRasterPos2i(0, 0);
	glDrawPixels(xsize, ysize, GL_RGBA, GL_UNSIGNED_BYTE, image);

	if (bPixval) {
		long int	i,idx;
		char		str[256];

		idx = Pixloc[0]+xsize*Pixloc[1];
		switch(zsize) {
			case 1:
				sprintf(str,"%d %d - %d",Pixloc[0],Pixloc[1],
					raw_image[idx]);
				break;
			case 2:
				sprintf(str,"%d %d - %d",Pixloc[0],Pixloc[1],
					((unsigned short *)raw_image)[idx]);
				break;
			case 3:
				sprintf(str,"%d %d - %d %d %d",
					Pixloc[0],Pixloc[1], 
					raw_image[idx*3+0],
					raw_image[idx*3+1],
					raw_image[idx*3+2]);
				break;
			case 4:
				sprintf(str,"%d %d - %f",Pixloc[0],Pixloc[1],
					((float *)raw_image)[idx]);
				break;
		}
		if (Pixloc[1] > ysize/2) {
			glRecti(0,2,strlen(str)*9+1,5+12);
			glColor3ub(255,255,255);
			glRasterPos2i(1,5);
		} else {
			glRecti(0,height-20-2,strlen(str)*9+1,height-20+12);
			glColor3ub(255,255,255);
			glRasterPos2i(1,height-20);
		}
		for(i=0;str[i];i++) {
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15,str[i]);
		}
	}
	if (isMesa) {
		glutSwapBuffers();
	} else {
		glFlush();
	}

	return;
}

int main(int argc,char **argv)
{
	int	pid = 0;
	int	bord = 1;
	int	pos = 0;
	int	head = 0;
	int	xpos = 0;
	int	ypos = 0;
	int	swap = 0;  /* 0=unknown 1=yes 2=no */
	int	flip = 1;
	int	comp = 0;
	char	*palfile = 0;  /* 0=unknown/none  */
	char	*ifile;
	char	*exec_args[MAX_ARGS];
	unsigned char	pal[768];
	unsigned short	*simage;
	char	*title;
	float		*fimage;
	int		lut[256];
	unsigned char	*clut;
	int 	i,j,k,high,low;
	FILE	*fp;
	struct	stat mystat;
	char	tmp[256];
	double	mult;
	double	imin = -99999;
	double  imax = -99999;
	double	fhigh,flow;

	xsize = 0;  /* 0=unknown */
	ysize = 0;  /* 0=unknown */
	
/* check command line for proper number of args */
	if (argc < 2) cmd_err(argv[0]);

  	glutInit(&argc, argv);
	if (isMesa) {
  		glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE );
	} else {
  		glutInitDisplayMode( GLUT_RGB | GLUT_SINGLE );
	}

	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (strcmp(argv[i],"-pid") == 0) {
/* this option should fork off a child with the -pid replaced by -wait,
	print the pid of the child to stdout and exit */
			pid = 1;
		} else if (strcmp(argv[i],"-wait") == 0) {
			pid = 2;
		} else if (strcmp(argv[i],"-pos") == 0) {
			if ((i+2) >= argc) cmd_err(argv[0]);
			i++;
			xpos = atoi(argv[i]);
			i++;
			ypos = atoi(argv[i]);
			pos = 1;
		} else if (strcmp(argv[i],"-head") == 0) {
			if ((i+1) >= argc) cmd_err(argv[0]);
			i++;
			head = atoi(argv[i]);
		} else if (strcmp(argv[i],"-min") == 0) {
			if ((i+1) >= argc) cmd_err(argv[0]);
			i++;
			imin = atof(argv[i]);
		} else if (strcmp(argv[i],"-max") == 0) {
			if ((i+1) >= argc) cmd_err(argv[0]);
			i++;
			imax = atof(argv[i]);
		} else if (strcmp(argv[i],"-pal") == 0) {
			if ((i+1) >= argc) cmd_err(argv[0]);
			i++;
			palfile = argv[i];
		} else if (strcmp(argv[i],"-nobord") == 0) {
			bord = 0;
		} else if (strcmp(argv[i],"-noflip") == 0) {
			flip = 0;
		} else if (strcmp(argv[i],"-8") == 0) {
			zsize = 1;
		} else if (strcmp(argv[i],"-z1") == 0) {
			zsize = 1;
		} else if (strcmp(argv[i],"-16") == 0) {
			zsize = 2;
		} else if (strcmp(argv[i],"-z2") == 0) {
			zsize = 2;
		} else if (strcmp(argv[i],"-24") == 0) {
			zsize = 3;
		} else if (strcmp(argv[i],"-z3") == 0) {
			zsize = 3;
		} else if (strcmp(argv[i],"-float") == 0) {
			zsize = 4;
		} else if (strcmp(argv[i],"-z4") == 0) {
			zsize = 4;
		} else if (strcmp(argv[i],"-swap") == 0) {
			swap = 1;
		} else if (strcmp(argv[i],"-noswap") == 0) {
			swap = 2;
		} else {
			cmd_err(argv[0]);
		}
		i++;
	}
/* now look for image name */
	if (i < argc) {
		ifile = argv[i];
		i++;
	} else {
		cmd_err(argv[0]);
	}
/* now look for image size */
	if ((i+1) < argc) {
		xsize = atoi(argv[i]);
		i++;
		ysize = atoi(argv[i]);
		i++;
	}
/* initialize the palette */
	clut = (unsigned char *)lut;
	for(j=0;j<256;j++) {
		clut[j*4+0] = j;
		clut[j*4+1] = j;
		clut[j*4+2] = j;
		clut[j*4+3] = 0;
	}
/* now look for .pal filename */
	if (i < argc)  palfile = argv[i];
/* read the palette */
	if (palfile) {
		fp = fopen(palfile,"rb");
		if (fp == 0L) {
			fprintf(stderr,"Unable to read palette %s\n",palfile);
		} else {
			fread(pal,1,768,fp);
			for(j=0;j<256;j++) {
				clut[j*4+0] = pal[j];
				clut[j*4+1] = pal[j+256];
				clut[j*4+2] = pal[j+512];
			}
			fclose(fp);
		}
		i++;
	}

#ifndef WIN32
/* finished parsing cmdline */
	if ((pid == 1) || (pid == 0)) {
		i = fork();
		if (i == -1)   {
			ex_err("Unable to fork off child process");
		} else if (i == 0) {
/* child */
			int arg = 0;

			exec_args[arg++] = argv[0];
			if (pid == 0) {
				exec_args[arg++] = "-wait";
			}

			for(j=1;((j<argc)&&(j<MAX_ARGS));j++) {
				if (strcmp(argv[j],"-pid") == 0) {
					exec_args[arg++] = "-wait";
				} else {
					exec_args[arg++] = argv[j];
				}
			}

			exec_args[arg++] = '\0';
			execvp(argv[0], exec_args);
			tal_exit(0);
		} else {
/* parent */
			if (pid == 1) printf("%d\n",i);
			tal_exit(0);
		}
	}
#endif

/* begin task of picking up undefined args */

        comp = 0;    /* assume no compression */
        if ((ifile[strlen(ifile)-2] == '.') &&
                                (ifile[strlen(ifile)-1] == 'Z')) comp = 1;
        if ((ifile[strlen(ifile)-3] == '.') && (ifile[strlen(ifile)-2] == 'g')
        		&& (ifile[strlen(ifile)-1] == 'z')) comp = 2;

/* start by retrieving the file size in bytes */
	if ((zsize == 0) || (xsize == 0) || (ysize == 0)) {
	    if (comp) {
                j = 0;
		if (comp == 1) {
                	sprintf(tmp,"zcat %s",ifile);
		} else {
                	sprintf(tmp,"gunzip -dc %s",ifile);
		}
                fp = popen(tmp,"r");
                if (fp != 0) {
                        while (fgetc(fp) != EOF) j++;
                        pclose(fp);
                } else {
			ex_err("Unable to access the input file");
		}
                mystat.st_size = j;
	    } else {
		if (stat(ifile,&mystat) == -1) {
			ex_err("Unable to access the input file");
		}
	    }
	}
/* if the pixel size and image size is unknown then try to find it */
	if ((zsize == 0) && (xsize == 0) && (ysize == 0)) {
/* First:assume 8bit */
		zsize = 1;
		j = (mystat.st_size-head);
		xsize = (int)sqrt((double)(j));
		ysize = xsize;
/* try 16 bit */
		j = ((mystat.st_size-head) / 2);
		i = (int)sqrt((double)(j));
/* if it is a match, use it */
		if (((mystat.st_size-head) - (2*i*i)) == 0) {
			xsize = i;
			ysize = i;
			zsize = 2;
		}
/* try 24 bit */
		j = ((mystat.st_size-head) / 3);
		i = (int)sqrt((double)(j));
/* if it is a match, use it */
		if (((mystat.st_size-head) - (3*i*i)) == 0) {
			xsize = i;
			ysize = i;
			zsize = 3;
		}
/* if the pixel size is unknown then try to find it */
	} else if (zsize == 0) {
		zsize = (mystat.st_size-head)/(xsize*ysize);
		if (zsize < 1) zsize = 1;
		if (zsize > 3) zsize = 3;
/* if image size is unknown then try to find it */
	} else if ((xsize == 0) && (ysize == 0)) {
		j = ((mystat.st_size-head) / zsize);
		xsize = (int)sqrt((double)(j));
		ysize = xsize;
	} 
/* read the image from disk */
	raw_image = malloc(xsize*ysize*zsize);
	if (raw_image == 0L) {
                ex_err("Unable to allocate image memory");
	}
	if (bin_io(ifile,'r',raw_image,xsize,ysize,zsize,0,0,head)) {
                ex_err("Unable to read image file");
	}

/* convert to visual form */
	image = (unsigned long *)malloc(xsize*ysize*sizeof(long));
	if (image == 0L) {
		free(raw_image);
               	ex_err("Unable to allocate image memory");
	}
/* different based on type */
	switch(zsize) {
		case 1:
			for(i=0;i<xsize*ysize;i++) {
				image[i] = lut[raw_image[i]];
			}
			break;
		case 2:
			simage = (unsigned short *)raw_image;
/* check swap if undefined */
			if (swap == 0) {
/* add the high bytes, and the low bytes */
				j = 0;
				k = 0;
				for(i=0;i<xsize*ysize*zsize;i = i + 2) {
					j = j + raw_image[i];
					k = k + raw_image[i+1];
				}
/* if either is 0 then swap to the other */
				if ((j != 0) && (k != 0)) {
					if (j > k) swap = 1;
				}
			}
			if (swap == 1) bswap(raw_image,xsize,ysize,2);
/* now convert 16 to 8 bits */
/* find max/min */
			high = 0;
			low = 65536;
			for(i=0;i<xsize*ysize;i++) {
				if (simage[i] < low) low = simage[i];
				if (simage[i] > high) high = simage[i];
			}
			if (imax != -99999) high = imax;
			if (imin != -99999) low = imin;
/* scale into 0-255 and map through the LUT */
			mult = 255.0/(double)(high - low);
			for(i=0;i<xsize*ysize;i++) {
				j = (int)((double)(simage[i]-low)*mult);
				if (j < 0) j = 0;
				if (j > 255) j = 255;
				image[i] = lut[j];
			}
			break;
		case 3:  /* RGB image */
			j = 0;
			for(i=0;i<xsize*ysize;i++) {
				image[i] = (raw_image[j]) | 
					(raw_image[j+1] << 8) | 
					(raw_image[j+2] << 16);
				j = j + 3;
			}
			break;
		case 4:  /* floating point image */
			if (swap == 1) bswap(raw_image,xsize,ysize,4);
/* now convert float to 8 bits */
			fimage = (float *)raw_image;
/* find max/min */
			fhigh = -DBL_MAX;
			flow = DBL_MAX;
			for(i=0;i<xsize*ysize;i++) {
				if (fimage[i] < flow) flow = fimage[i];
				if (fimage[i] > fhigh) fhigh = fimage[i];
			}
			if (imax != -99999) fhigh = imax;
			if (imin != -99999) flow = imin;
/* scale into 0-255 and map through the LUT */
			mult = 255.0/(fhigh - flow);
			for(i=0;i<xsize*ysize;i++) {
				j = (int)((fimage[i]-flow)*mult);
				if (j < 0) j = 0;
				if (j > 255) j = 255;
				image[i] = lut[j];
			}
			break;
	}
	if (flip) {
		yflip(raw_image,xsize,ysize,zsize);
		yflip((unsigned char *)image,xsize,ysize,4);
	}

/* open the window */

/* GLUT is always in foreground	
	if (pid != 0) {
		foreground();   
	}
*/
/* unsupported by GLUT 
	if (bord == 0) noborder();
*/

	if (pos) {
		glutInitWindowPosition(xpos,ypos);
	}
	glutInitWindowSize(xsize,ysize);

	title = (unsigned char *)strrchr(ifile,'/');
	if (title == 0) title = (unsigned char *)ifile;
	if (swap == 1) {
		sprintf(tmp,"%dx%dx%d-bs-%s",xsize,ysize,zsize,title);
	} else {
		sprintf(tmp,"%dx%dx%d-%s",xsize,ysize,zsize,title);
	}
  	glutCreateWindow(tmp);
  	glutDisplayFunc(redraw);
  	glutReshapeFunc(reshape);
  	glutKeyboardFunc(key);
	glutMouseFunc(mouse_click);
	glutMotionFunc(mouse_move);

	init_menu();

/* unsupported by GLUT 
	if (bord == 0) noborder();
*/

/* do the event loop */
	glutMainLoop();

	free(image);
	free(raw_image);
	tal_exit(0);
	exit(0);
}

