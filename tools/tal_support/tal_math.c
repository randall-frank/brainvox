/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_math.c 1833 2006-07-23 22:42:41Z rjfrank $
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
#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif
#include "proto.h"
#include "polyfill.h"
#include "macroprotos.h"

#define M_PTS 10000
#define MAX_VOLUMES 23

#ifdef WIN32
#define TAL_SUPPORT_DIR "C:\\Program Files\\U of Iowa LCN\\Brainvox\\bin"
#else
#ifdef __APPLE__
#define TAL_SUPPORT_DIR "/Applications/Brainvox.app/lib"
#else
#define TAL_SUPPORT_DIR "/usr/local/lib"
#endif
#endif

void 	read_one(char *intemp,long int sl,unsigned char *data,
		long int dx,long int dy,long int dz,long int swab);
double	get_one_point(long int x,long int y,long int which);
void 	set_one_point(long int x,long int y,double data,long int which);
long int eval_expression(char *expr,long int z);
void	tal_math_setup(void);
void build_compiled_exp(char *expr,char *exe,char *comment);

long int (*user_proc)(char *,long int ) = eval_expression;
char	*user_expression = NULL;
char	*user_comment = NULL;
int	compiled_expression = 0;

double		roi_scale = 0.5;
long int	dx = 256;
long int	dy = 256;
long int	yfliproi = 1;
double		background = 0.0;

/* use arrays instead of these fixed defines */
long int	Adz[MAX_VOLUMES+1];
unsigned char	*Abd[MAX_VOLUMES+1];
unsigned short	*Asd[MAX_VOLUMES+1];
float		*Afd[MAX_VOLUMES+1];
char		Aintemp[MAX_VOLUMES+1][256];
char		*maskslice = NULL;


double	get_one_point(long int x,long int y,long int which)
{
	double		d = 0.0;
	long int 	i,j;
	unsigned short	*ss;
	unsigned char	*cc;
	float		*ff;

	i = x+(dx*y);

	j = Adz[which]; ss = Asd[which]; cc = Abd[which]; ff = Afd[which];
	if ((cc == 0L) || (ss == 0L) || (ff == 0L)) return(0.0);
	if (j == 1) {
		d = cc[i];
	} else if (j == 2) {
		d = ss[i];
	} else if (j == 4) {
		d = ff[i];
	}
	return(d);
}

void set_one_point(long int x,long int y,double data,long int which)
{
	double		d = data;
	long int 	i,j;
	unsigned short	*ss;
	unsigned char	*cc;
	float		*ff;

	i = x+(dx*y);

	j = Adz[which]; ss = Asd[which]; cc = Abd[which]; ff = Afd[which];
	if (j == 1) {
		if (d > 255) d = 255;
		if (d < 0) d = 0;
		cc[i] = d;
	} else if (j == 2) {
		if (d > 65535) d = 65535;
		if (d < 0) d = 0;
		ss[i] = d;
	} else if (j == 4) {
		ff[i] = d;
	}
	return;
}

void read_one(char *intemp,long int sl,unsigned char *data,
	long int dx,long int dy,long int dz,long int swab)
{
	FILE	*fp;
	char		tstr[256];
	long int 	err,j,i,x,y,en,st;
        long int        list[M_PTS],strips[M_PTS];
        long int        ptr,num;
	unsigned short	*sdata = (unsigned short *)data;
	float		*fdata = (float *)data;

	if (data == 0L) return;

	strcpy(tstr,intemp);
	name_changer(intemp,sl,&err,tstr);
	printf("Reading the file:%s\n",tstr);
	if (strstr(tstr,".roi") != 0L) {
		if (dz == 4) {
			for(j=0;j<dx*dy;j++) fdata[j] = 0;
		} else {
			for(j=0;j<dx*dy*dz;j++) data[j] = 0;
		}
/* paint roi into the image */
		num = 0;
		fp = fopen(tstr,"r");
		if (fp != 0L) {
			fscanf(fp,"%ld",&num);
			for(j=0;j<num;j++) {
				fscanf(fp,"%ld",&(list[j]));
				list[j] = list[j]*roi_scale;
			}
			if (num < 6) num = 0;
			fclose(fp);
		}
		if (num == 0) return;
		if (yfliproi) {
			for(j=1;j<num;j+=2) {
				list[j] = (dy-1) - list[j];
			}
		}
/* perform the polygon fill on the list */
        	ptr = M_PTS;
        	polyfill_c_(list,num,strips,&ptr);
        	if (ptr == -1) {
                printf("Warning:overflow encountered in polygon filling.\n");
			return;
		}
/* fill the interior of the ROI */
        	for(i=0;i<ptr;i=i+3) {
                	y = strips[i+2];
/* clip the strip to the current image */
                	st = strips[i];
                	en = strips[i+1];
                	if (st < 0) st = 0;
                	if (en >= dx) en = dx -1;
                	if ((st <= en) && (y >= 0) && (y < dy)) {
/* fill the current strip */
                        j = st+(y*dx);  /* start of line */
                        for(x=st;x<=en;x++) {
				if (dz == 2) {
					sdata[j] = 1;
				} else if (dz == 1) {
					data[j] = 1;
				} else if (dz == 4) {
					fdata[j] = 1;
				}
                                j++;
                        }
			}
                }

	} else {
/* read image file */
		bin_io(tstr,'r',data,dx,dy,dz,0,swab,0L);
	}
	
	return;
}

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	if (compiled_expression) {
	fprintf(stderr,"(%s:%s) Usage: %s [options] outputtemp inputA inputB inputC inputD\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"This is a custom tal_math program.\n");
		if (user_comment) fprintf(stderr,"Comment:%s\n",user_comment);
	} else {
	fprintf(stderr,"(%s:%s) Usage: %s [options] expression outputtemp inputA inputB ... inputW\n",__DATE__,TAL_VERSION,s);
	}
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:15\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -s(scale) ROI scaling factor default:0.5\n");
	fprintf(stderr,"         -m(masktemp) filename template for a mask volume default:none\n");
	fprintf(stderr,"         -d(back) background value output outside of the mask default:0.0\n");
	fprintf(stderr,"         -noflip do not flip ROIs over the X axis default:flip them\n");
	fprintf(stderr,"         -help print help on expressions\n");
	if (!compiled_expression) {
		fprintf(stderr,"         -c(exename) Create a custom executable to execute the expression.\n");
		fprintf(stderr,"         -C(comment) Supply a user comment for a custom executable.\n");
		fprintf(stderr,"         outputtemp and inputs(A-W) are ignored with this option.\n");
	} else {
		fprintf(stderr,"Output=%s\n",user_expression);
	}
	fprintf(stderr,"Image depths are 1=unsigned char,2=unsigned short,4=float\n");
	fprintf(stderr,"Mask volumes are 8bit\n");
	tal_exit(1);
}

int real_tal_math_main(int argc,char **argv)
{
	long int	dz = 2;
	long int	istart = 1;
	long int	iend = 15;
	long int	istep = 1;
	long int	swab = 0;

	char		expr[256],outtemp[256];
	char		tstr[256];
	char		*exename = 0;
	char		*comment = 0;
	long int	i,err,j;
	char		*masktemp = NULL;

	j = 0;
	for(i=0;i<=MAX_VOLUMES;i++) {
		Adz[i] = -1;
		Aintemp[i][0] = '\0';
	}

	bin_roi_scale(roi_scale);
	bin_roi_flip(yfliproi);

/* get the setup */
	tal_math_setup();

/* no user expression in a custom app */
	if (user_expression != 0) j = 1;

/* parse command line flags */
	i = 1;
	while (i < argc) {
		if (argv[i][0] != '-') {
		    if (j == 0) {
			strcpy(expr,argv[i]);
			j++;
			if (exename != 0L) {
			     build_compiled_exp(expr,exename,comment);
			     tal_exit(0);
			}
		    } else if (j < MAX_VOLUMES) {
			if (j == 1) strcpy(outtemp,argv[i]);
			strcpy(Aintemp[j-1],argv[i]);
			Adz[j-1] = dz;
			j++;
		    } else {
			cmd_err(argv[0]);
		    }
		} else {
		    switch (argv[i][1]) {
			case 'c':
				exename = &(argv[i][2]);
				break;
			case 'C':
				comment = &(argv[i][2]);
				break;
			case 'f':
				istart = atoi(&(argv[i][2]));
				break;
			case 'l':
				iend = atoi(&(argv[i][2]));
				break;
			case 'i':
				istep = atoi(&(argv[i][2]));
				break;
			case 'x':
				dx = atoi(&(argv[i][2]));
				break;
			case 'y':
				dy = atoi(&(argv[i][2]));
				break;
			case 'z':
				dz = atoi(&(argv[i][2]));
				break;
			case 's':
				roi_scale = atof(&(argv[i][2]));
				bin_roi_scale(roi_scale);
				break;
			case 'b':
				swab = 1;
				break;
			case 'n':  /* noflip */
				yfliproi = 0;
				bin_roi_flip(yfliproi);
				break;
			case 'm':
				masktemp = &(argv[i][2]);
				break;
			case 'd':
				background = atof(&(argv[i][2]));
				break;
			default:	
				if (strcmp(argv[i],"-help") == 0) {
					print_exp_help(argv[0],1);
					tal_exit(0);
				} else {
					cmd_err(argv[0]);
				}
				break;
		    }
		}
		i++;
	}
	if (j < 2) cmd_err(argv[0]);
/* get the image memory */
	for(i=0;i<=MAX_VOLUMES;i++) {
		if (Adz[i] != -1) {
			Abd[i] =  malloc(Adz[i]*dx*dy);
			if (Abd[i] == 0L) ex_err("Unable to allocate image memory.");
		} else {
			Abd[i] = 0L;
		}
		Asd[i] = (unsigned short *)Abd[i];
		Afd[i] = (float *)Abd[i];
	}
	if (Abd[0] == 0L) ex_err("Unable to allocate image memory.");
	if (masktemp) {
		maskslice = malloc(dx*dy);
		if (!maskslice) ex_err("Unable to allocate image memory.");
	}
/* read the images */
	for(i=istart;i<=iend;i=i+istep) {
		for(j=1;j<=MAX_VOLUMES;j++) {
			read_one(Aintemp[j],i,Abd[j],dx,dy,Adz[j],swab);
		}
		if (masktemp) {
			name_changer(masktemp,i,&err,tstr);
			printf("Reading mask file:%s\n",tstr);
			bin_io(tstr,'r',maskslice,dx,dy,1,0,swab,0L);
		}
/* compute the resultant image */
		user_proc(expr,i-1);
/* write the output image */
		name_changer(outtemp,i,&err,tstr);
		printf("Writing sampled file:%s\n",tstr);
		bin_io(tstr,'w',Abd[0],dx,dy,Adz[0],0,swab,0L);
	}
	for(i=0;i<=MAX_VOLUMES;i++) {
		if (Abd[i]) free(Abd[i]);
	}
	if (masktemp) free(maskslice);
	tal_exit(0);
	exit(0);
}

/* expression code... */

long int User_Identifiers(char *name, double *value);

typedef struct {
	char *name;
	double value;
	} Identifiers;

#define X_ID 0
#define Y_ID 1
#define Z_ID 2
#define A_ID 3
#define B_ID 4
#define C_ID 5
#define D_ID 6
#define E_ID 7
#define F_ID 8
#define G_ID 9
#define H_ID 10
#define I_ID 11
#define J_ID 12
#define K_ID 13
#define L_ID 14
#define M_ID 15
#define N_ID 16
#define O_ID 17
#define P_ID 18
#define Q_ID 19
#define R_ID 20
#define S_ID 21
#define T_ID 22
#define U_ID 23
#define V_ID 24
#define W_ID 25

Identifiers	ident_list[] = { 	{"x",0.0},
					{"y",0.0},
					{"z",0.0},
					{"a",0.0},
					{"b",0.0},
					{"c",0.0},
					{"d",0.0},
					{"e",0.0},
					{"f",0.0},
					{"g",0.0},
					{"h",0.0},
					{"i",0.0},
					{"j",0.0},
					{"k",0.0},
					{"l",0.0},
					{"m",0.0},
					{"n",0.0},
					{"o",0.0},
					{"p",0.0},
					{"q",0.0},
					{"r",0.0},
					{"s",0.0},
					{"t",0.0},
					{"u",0.0},
					{"v",0.0},
					{"w",0.0},
					{0L,0.0}
				};

long int User_Identifiers(char *name, double *value)
{
	long int i;

	i = 0;
	while (ident_list[i].name != 0L) {
		if (strcmp(ident_list[i].name,name) == 0L) {
			*value = ident_list[i].value;
			return(1);
		}
		i++;
	}
	return(0);
}

long int eval_expression(char *expr,long int z)
{

	long int 	y,x,i;
	char		out[256];
	double		value;

/* setup user defined IDENTIFIER search */
	set_user_identifier(User_Identifiers);
	if (init_parser(expr,1L)) goto err_out;

	ident_list[Z_ID].value = z;
	for(y=0;y<dy;y++) {
		ident_list[Y_ID].value = y;
#ifdef DEBUG
	printf("\nWorking on line %ld :",y);
#endif
		for(x=0;x<dx;x++) {
			ident_list[X_ID].value = x;

			if ((maskslice) && (maskslice[x+(y*dx)] == 0)) {
				value = background;
			} else {
				for(i=1;i<MAX_VOLUMES;i++) {
					ident_list[A_ID+i-1].value = 
						get_one_point(x,y,i);
				}

				if (parse_math(out,&value) != 0L) goto err_out;
				if (reset_tokenlist_ptr()) goto err_out;
			}

			set_one_point(x,y,value,0);
		}
	}
/* remove user defined IDENTIFIER search */
	set_user_identifier(0L);
	return(0);

err_out:
/* remove user defined IDENTIFIER search */
	set_user_identifier(0L);
	return(1);
}

void build_compiled_exp(char *expr,char *exe,char *comment) 
{
	FILE 	*fp;
	char 	file[L_tmpnam+10];
	char	comp[4096];
	char	*env;
	long int i;

	if (init_parser(expr,1)) {
		fprintf(stderr,"Unable to parse:%s\n",expr);
		tal_exit(1);
	}
	if (C_parse_math(comp)) { 
		fprintf(stderr,"Unable to parse:%s\n",expr);
		tal_exit(1);
	}

	tmpnam(file);
	strcat(file,".c");

	fp = fopen(file,"w");
	if (fp == 0L) {
		fprintf(stderr,"Unable to create temp .c file\n");
		tal_exit(1);
	}
	fprintf(fp,"#include <stdio.h>\n");
	fprintf(fp,"#include <stdlib.h>\n");
	fprintf(fp,"#include <math.h>\n");
	fprintf(fp,"#include <string.h>\n");
	fprintf(fp,"extern long int (*user_proc)(char *,long int );\n");
	fprintf(fp,"extern char     *user_expression;\n");
	fprintf(fp,"extern char     *user_comment;\n");
	fprintf(fp,"extern int      compiled_expression;\n");
	fprintf(fp,"extern long int dx,dy;\n");
	fprintf(fp,"extern char *maskslice;\n");
	fprintf(fp,"extern double background;\n");
	fprintf(fp,"int real_tal_math_main(int argc,char **argv);\n");
	fprintf(fp,"long int comp_expression(char *expr,long int zin);\n");
	fprintf(fp,"double	get_one_point(long int x,long int y,long int which);\n");
	fprintf(fp,"void 	set_one_point(long int x,long int y,double data,long int which);\n");
	fprintf(fp,"void tal_math_setup()\n");
	fprintf(fp,"{\n");
        	fprintf(fp,"user_proc = comp_expression;\n");
        	fprintf(fp,"user_expression = \"%s\";\n",expr);
		if (comment) {
        		fprintf(fp,"user_comment = \"%s\";\n",comment);
		} else {
        		fprintf(fp,"user_comment = 0L;\n");
		}
        	fprintf(fp,"compiled_expression = 1;\n");
	fprintf(fp,"}\n");
	fprintf(fp,"long int comp_expression(char *expr,long int zin)\n");
	fprintf(fp,"{\n");
        	fprintf(fp,"long int        ij,ii;\n");
        fprintf(fp,"double          value;\n");
        fprintf(fp,"double          x,y,z;\n");
        fprintf(fp,"double          a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w;\n");
        fprintf(fp,"z = zin;\n");
        fprintf(fp,"for(ij=0;ij<dy;ij++) {\n");
                fprintf(fp,"y = ij;\n");
                fprintf(fp,"for(ii=0;ii<dx;ii++) {\n");
                        fprintf(fp,"x = ii;\n");

			fprintf(fp,"if ((maskslice) && (maskslice[ii+(ij*dx)] == 0)) {\n");
                        fprintf(fp,"value = background;\n");
			fprintf(fp,"} else {\n");

			for(i='a';i<='w';i++) {
				fprintf(fp,"%c = get_one_point(ii,ij,%ld);\n",
					(char)i,i-'a'+1L);
			}
                        fprintf(fp,"value = %s\n",comp);

			fprintf(fp,"}\n");

                        fprintf(fp,"set_one_point(ii,ij,value,0);\n");

                fprintf(fp,"}\n");
        fprintf(fp,"}\n");
        fprintf(fp,"return(0);\n");
	fprintf(fp,"}\n");
	fprintf(fp,"int main(int argc,char **argv)\n");
	fprintf(fp,"{\n");
		 fprintf(fp,"	return(real_tal_math_main(argc,argv));\n");
 	fprintf(fp,"}\n");
	fclose(fp);

	env=getenv("TAL_SUPPORT_DIR");
	if (env == 0L) {
#ifdef WIN32
		char path[256],*p0;
		GetModuleFileName(NULL,path,sizeof(path));
		p0 = strrchr(path,'\\');
		if (p0) p0[1] = '\0';
		env = strdup(p0);
#else
		env = TAL_SUPPORT_DIR;
#endif
	}
#ifdef WIN32
	sprintf(comp,"cl -DWIN32 -Oait -Gs -nologo %s \"%s\\libtal_math.lib\" /Fe%s",
		file,env,exe);
#else
	sprintf(comp,"cc -s -O %s -L%s -ltal_math -o %s -lm",
		file,env,exe);
#endif

/* not the safest way to do this, I know! */
	system(comp);

#ifdef DEBUG
	sprintf(comp,"cp %s .",file);
	system(comp);
#endif

	unlink(file);

	tal_exit(0);
}
