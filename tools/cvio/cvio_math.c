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

/* read N streams, apply math function and write another stream */
/* note: timestamps are taken from the "A" file */
/* if a file is a scalar, make it a complete sample by replication */
/* compile option to produce a shared lib of the function.  pass the
 * name of the shared library to the app */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "cvio_lib.h"
#include "macro2protos.h"
#include "cvio_dynamic.h"

typedef double (*ExpProc)(double *in);
typedef void *(*InfoProc)(int what);

void info(char *app)
{
        fprintf(stderr,"(%s) Usage: %s [options] expression dstfile srcAfile [srcBfile ... srcWfile]\n",__DATE__,app);
        fprintf(stderr,"\t or %s -S expression shfile comment\n",app);
        fprintf(stderr,"\t or %s -s [options] shfile dstfile srcAfile [srcBfile ... srcWfile]\n",app);
        fprintf(stderr,"Options:\n");
        fprintf(stderr,"        -v verbose\n");
        fprintf(stderr,"        -c dstfile is in compressed form\n");
	fprintf(stderr,"        -z(outdepth) output depth 2=byte,3=short,4=float,5=unsigned short default:2\n");
        fprintf(stderr,"        -help print help on expessions\n");
	fprintf(stderr,"        -A do not copy unknown attributes\n");
        fprintf(stderr,"        -S create a shared library for the expression\n");
        fprintf(stderr,"        -s use a shared library as the expression\n");
        fprintf(stderr,"        -m(maxsize) max number of samples for shmem (default: # src samples)\n");
	fprintf(stderr,"        -d don't create output file (it already exists)\n");
	fprintf(stderr,"	-b(sample) begin processing at sample\n");
        exit(1);
}

void	exit_cvio_error(int32_t err) 
{
	char	tstr[256];
	int32_t	tlen = 256;

	cvio_get_error_message(err,tstr,&tlen);
	fprintf(stderr,"Fatal error:%s\n",tstr);

	cvio_cleanup(0);
	exit(1);
}

void copy_attribs(uint32_t dst_s,uint32_t src_s)
{
	char    listnames[512],*p;
	int32_t   len,num,i;
	char    value[256];

	len = 512;
	cvio_getattriblist(src_s, listnames, &len, &num);
	
	p = listnames;
	for(i=0;i<num;i++) {
		len = 256;
		if (!strstr(p,"REQ_") && strstr(p,"_ST")) {
			if (!cvio_getattribute(src_s,p,value,&len)) {
				cvio_setattribute(dst_s,p,value);
			}
		}
		p += (strlen(p) + 1);
	}
	return;
}

/* buffers: 0-22 = A-W  23=output */
#define MAX_BUFFERS 24
#define OUT_BUFFER 23

/* Note: each buffer is big enough for two samples.  The current,
 * and the "next" one. */
unsigned char *Aucptr[MAX_BUFFERS] = {0};
unsigned short *Ausptr[MAX_BUFFERS];
short *Asptr[MAX_BUFFERS];
float *Afptr[MAX_BUFFERS];
uint32_t iSize[MAX_BUFFERS];  /* size (in bytes) of a sample */
int32_t datatype[MAX_BUFFERS]; /* type of the various streams */
uint32_t iSamples; /* number of elements in a sample */
CVIO_DT_TIME tNext[MAX_BUFFERS]; /* note: tNext[0] = current time */
int32_t iState[MAX_BUFFERS] = {0}; /* -1=done, 0=initial, 1=one read */
int32_t		ndims,dims[3];

#define MAX_VARS (23+8)

double ID_values[MAX_VARS];
char *ID_names[MAX_VARS+1] = {
	"a","b","c","d","e","f","g","h","i","j","k","l","m",
	"n","o","p","q","r","s","t","u","v","w",
	"x","y","z", "maxx","maxy","maxz", "isamp","fsamp",
	NULL
};

long int User_Identifiers(char *name, double *value)
{
        long int i = 0;
        while (ID_names[i]) {
                if (strcmp(ID_names[i],name) == 0L) {
                        *value = ID_values[i];
                        return(1);
                }
                i++;
        }
        return(0);
}

/* what: 0=sum 1=ss 2=min 3=max 4=N */
/* use mask? */
double get_value(int idx,int z)
{
   double t = 1.0;
   if (idx == -1) return(t);
   switch(datatype[idx]) {
      case CVIO_TYP_BYTE:
         t = Aucptr[idx][z];
         break;
      case CVIO_TYP_SHORT:
         t = Asptr[idx][z];
         break;
      case CVIO_TYP_USHORT:
         t = Ausptr[idx][z];
         break;
      case CVIO_TYP_FLOAT:
         t = Afptr[idx][z];
         break;
   }
   return(t);
}

double stat_function(char *var,char *mask,int what,int dx,int dy,int dz)
{
   int midx,idx,ss[3][2],i,j,k,z;
   double v;

   /* clamp bounds (remember -1 is ok) */
   if (dx < -1) dx = 0;
   if (dy < -1) dy = 0;
   if (dz < -1) dz = 0;
   if (dx >= dims[0]) dx = dims[0]-1;
   if (dy >= dims[1]) dy = dims[1]-1;
   if (dz >= dims[2]) dz = dims[2]-1;

   /* find a variable */
   if ((var[0] >= 'A') && (var[0] <= 'W')) {
      idx = var[0] - 'A';
   } else if ((var[0] >= 'a') && (var[0] <= 'w')) {
      idx = var[0] - 'a';
   } else {
      return(0.0);
   }
   /* invalid variable */
   if (!Aucptr[idx]) return(0.0);

   /* Mask variable */
   midx = -1;
   if (mask) {
      if ((mask[0] >= 'A') && (mask[0] <= 'W')) {
         midx = mask[0] - 'A';
      } else if ((mask[0] >= 'a') && (mask[0] <= 'w')) {
         midx = mask[0] - 'a';
      } else {
         return(0.0);
      }
      if (!Aucptr[midx]) return(0.0);
   }

   /* invalid function */
   if ((what < 0) || (what > 4)) return(0.0);

   /* pick limits */
   ss[0][0] = dx; ss[0][1] = dx+1;
   ss[1][0] = dy; ss[1][1] = dy+1;
   ss[2][0] = dz; ss[2][1] = dz+1;
   if (ss[0][0] == -1) {
      ss[0][0] = 0; ss[0][1] = dims[0];
   }
   if (ss[1][0] == -1) {
      ss[1][0] = 0; ss[1][1] = dims[1];
   }
   if (ss[2][0] == -1) {
      ss[2][0] = 0; ss[2][1] = dims[2];
   }

   /* over the proper domain */
   switch(what) {
      case 0:
         v = 0;
         for(k=ss[2][0];k<ss[2][1];k++) {
         for(j=ss[1][0];j<ss[1][1];j++) {
         z=(k*(dims[0]*dims[1])) + (j*(dims[0])) + ss[0][0];
         for(i=ss[0][0];i<ss[0][1];i++) {
            if (get_value(midx,z)) v = v + 1.0;
            z++;
         }
         }
         }
         break;
      case 1:
         v = 0;
         for(k=ss[2][0];k<ss[2][1];k++) {
         for(j=ss[1][0];j<ss[1][1];j++) {
         z=(k*(dims[0]*dims[1])) + (j*(dims[0])) + ss[0][0];
         for(i=ss[0][0];i<ss[0][1];i++) {
            if (get_value(midx,z)) {
               double t = get_value(idx,z);
               v = v + t;
            }
            z++;
         }
         }
         }
         break;
      case 2:
         v = 0;
         for(k=ss[2][0];k<ss[2][1];k++) {
         for(j=ss[1][0];j<ss[1][1];j++) {
         z=(k*(dims[0]*dims[1])) + (j*(dims[0])) + ss[0][0];
         for(i=ss[0][0];i<ss[0][1];i++) {
            if (get_value(midx,z)) {
               double t = get_value(idx,z);
               v = v + (t*t);
            }
            z++;
         }
         }
         }
         break;
      case 3:
         v = 1e+23;
         for(k=ss[2][0];k<ss[2][1];k++) {
         for(j=ss[1][0];j<ss[1][1];j++) {
         z=(k*(dims[0]*dims[1])) + (j*(dims[0])) + ss[0][0];
         for(i=ss[0][0];i<ss[0][1];i++) {
            if (get_value(midx,z)) {
               double t = get_value(idx,z);
               if (t < v) v = t;
            }
            z++;
         }
         }
         }
         break;
      case 4:
         v = -1e+23;
         for(k=ss[2][0];k<ss[2][1];k++) {
         for(j=ss[1][0];j<ss[1][1];j++) {
         z=(k*(dims[0]*dims[1])) + (j*(dims[0])) + ss[0][0];
         for(i=ss[0][0];i<ss[0][1];i++) {
            if (get_value(midx,z)) {
               double t = get_value(idx,z);
               if (t > v) v = t;
            }
            z++;
         }
         }
         }
         break;
   }
   return(v);
}


double InterpExpression(double *in)
{
	char	out[256];
	double	value = 0.0;
	/* Note: in will be equal to the ID_values global array */
	if (parse_math(out,&value) != 0) {
		fprintf(stderr,"Error in expression evaluation\n");
		cvio_cleanup(0);
		exit(1);
	}
	reset_tokenlist_ptr();
	return(value);
}

int main(int argc, char *argv[])
{
	int32_t		i,j;
	int32_t		err;

	int32_t		dontcreate = 0;
        int32_t 	iVerbose = 0;
        int32_t 	iCompress = 0;
        int32_t 	cp_attribs = 1;
        int32_t		iCompileMode = 0;
	int32_t		iDepth = CVIO_TYP_BYTE;

        uint32_t 	nsrcfiles;
	char		**srcfiles;
	char		*dstfile;
	char		*expression = NULL;

	uint32_t	src_s[MAX_BUFFERS], dst_s = -1;
	int32_t		d_datatype,d_ndims,d_dims[3];
	int32_t		nsamp,max_samp = 0, begin = 0;
	int32_t		iCount;

	ExpProc	        exp_proc = InterpExpression;
	shlib_handle	sh_hdl = NULL;

	uint32_t	c_s;
	CVIO_DT_TIME	c_t;

	for(i=0;i<MAX_BUFFERS;i++) src_s[i] = -1;

	/* parse the options */
        i = 1;
        while((i<argc) && (argv[i][0] == '-')) {
                switch(argv[i][1]) {
                        case 'v' : iVerbose = 1; break;
                        case 'A' : cp_attribs = 0; break;
                        case 'c' : iCompress = CVIO_CREATE_GZIP; break;
                        case 'S' : iCompileMode = 1; break;
                        case 's' : iCompileMode = 2; break;
                        case 'm' : max_samp = atoi(argv[i]+2); break;
			case 'd' : dontcreate = 1; break;
			case 'b' : begin = atoi(argv[i]+2); break;
                        case 'z' :
				j = atoi(argv[i]+2); 
				if (j == 2) {
					iDepth = CVIO_TYP_BYTE;
				} else if (j == 3) {
					iDepth = CVIO_TYP_SHORT;
				} else if (j == 5) {
					iDepth = CVIO_TYP_USHORT;
				} else if (j == 4) {
					iDepth = CVIO_TYP_FLOAT;
				} else {
					info(argv[0]);
				}
				break;
                        default: 
				if (strcmp(argv[i],"-help") == 0) {
					print_exp_help(argv[0],1);
					exit(0);
				}
				info(argv[0]); 
				break;
                }
                i++;
	}

	if (argc-i < 2) info(argv[0]);

        /* get the expression */
        expression = argv[i++];

	/* get the filenames */
	dstfile = argv[i++];
	srcfiles = &(argv[i]);
        nsrcfiles = argc - i;
        if (nsrcfiles < 1) info(argv[0]);
	if (nsrcfiles > ('x' - 'a')) info(argv[0]);  /* only a->w allowed */

        /* various compile modes */
        if (iCompileMode >= 2) {
		void **pp;
		InfoProc ip;
        	/* open a shared library for use */
                /* expression is a shared library */
		sh_hdl = shlib_open(expression,0);
		if (!sh_hdl) {
			fprintf(stderr,"Unable to open shared lib: %s\n",
				expression);
			exit(1);
                }
		ip = (InfoProc)shlib_getsym(sh_hdl,"cvio_math_proc");
                if (!ip) {
			shlib_close(sh_hdl);
			fprintf(stderr,"Unable to find a function in lib:%s\n",
				expression);
			exit(1);
                }
		if (iVerbose) {
                    fprintf(stdout,"Function: %s\n",(char *)ip(1));
                    fprintf(stdout,"Comment: %s\n",(char *)ip(2));
		}
		exp_proc = (ExpProc)ip(3);
		pp = (void **)ip(4);
		*pp = (void *)stat_function;

        } else if (iCompileMode == 1) {
		char	comp[8192],tmp[256],cmd[256];
		FILE 	*fp;
		char	*cc_default = 
#ifdef WIN32
			"cl -DWIN32 %s %s"; /*TODO*/
#else
#ifdef __APPLE__
			"gcc -bundle -o %s %s";
#else
			"gcc -shared -o %s %s";
#endif
#endif

		if (getenv("CVIO_COMP_SHARED")) {
			cc_default = getenv("CVIO_COMP_SHARED");
		}

        	/* compile the expression */
                /* dstfile is output, srcfiles[0] is a comment */
		set_user_identifier(User_Identifiers);
		if (init_parser(expression,1L)) {
			fprintf(stderr,"Invalid expression: %s\n",expression);
			exit(1);
		}
		if (C_parse_math(comp)) {
			fprintf(stderr,"Unable to compile: %s\n",expression);
			exit(1);
		}

		/* write out a file (expression=comp) */
		strcpy(tmp,dstfile);
		strcat(tmp,".c");
		fp = fopen(tmp,"w");
		if (!fp) {
			fprintf(stderr,"Unable to open src file: %s\n",tmp);
		}
		fprintf(fp,"#include <stdio.h>\n");
		fprintf(fp,"#include <stdlib.h>\n");
		fprintf(fp,"#include <string.h>\n");
		fprintf(fp,"#include <math.h>\n");
		fprintf(fp,"#ifndef WIN32\n");
		fprintf(fp,"#define FUNC extern\n");
		fprintf(fp,"#else\n");
		fprintf(fp,"#define FUNC __declspec(dllexport)\n");
		fprintf(fp,"#endif\n");
		fprintf(fp,"double (*stat_func)(char *,char *,int,int,int,int) = NULL;\n");
		fprintf(fp,"static double comp_func(double *in)\n");
		fprintf(fp,"{\n");
		fprintf(fp," double value;\n");
		i = 0;
		while(ID_names[i]) {
			fprintf(fp," double %s;\n",ID_names[i]);
			i++;
		}
		i = 0;
		while(ID_names[i]) {
			fprintf(fp," %s = in[%d];\n",ID_names[i],i);
			i++;
		}
		fprintf(fp," value = %s\n",comp);
		fprintf(fp," return(value);\n");
		fprintf(fp,"}\n");
		fprintf(fp,"FUNC void *cvio_math_proc(int what)\n");
		fprintf(fp,"{\n");
		fprintf(fp," if (what == 1) return((void*)\"%s\");\n",
			expression);
		fprintf(fp," if (what == 2) return((void*)\"%s\");\n",
			srcfiles[0]);
		fprintf(fp," if (what == 3) return((void*)comp_func);\n");
		fprintf(fp," if (what == 4) return((void*)&stat_func);\n");
		fprintf(fp," return((void*)0);\n");
		fprintf(fp,"}\n");
		
		fclose(fp);

		/* compile it */
		sprintf(cmd,cc_default,dstfile,tmp);
		system(cmd);

		/* delete the tmp file */
		if (iVerbose == 0) unlink(tmp); 

		exit(0);
        } else {
		/* parse check/setup the expression (interpreted case) */
		set_user_identifier(User_Identifiers);
		if (init_parser(expression,1L)) {
			fprintf(stderr,"Invalid expression: %s\n",expression);
			exit(1);
		}
        }

        /* ok, we are ready to go... */
	if (cvio_init()) exit(1);

	/* open the source files */
        for(i=0;i<nsrcfiles;i++) {
		err = cvio_open(srcfiles[i],CVIO_ACCESS_READ,&(src_s[i]));
		if (err) exit_cvio_error(err);

		/* get its format and size */
		if (i == 0) {
			err=cvio_datatype(src_s[i],&(datatype[i]),&ndims,dims);
			if (err) exit_cvio_error(err);

			if (ndims < 3) dims[2] = 1;
			if (ndims < 2) dims[1] = 1;
		} else {
			err=cvio_datatype(src_s[i],&d_datatype,&d_ndims,d_dims);
			if (err) exit_cvio_error(err);
			if (d_ndims != ndims) {
				fprintf(stderr,"Number of input file sample dimensions do not match: %s\n",srcfiles[i]);
				if (sh_hdl) shlib_close(sh_hdl);
				cvio_cleanup(0);
				exit(1);
			}
			for(j=0;j<ndims;j++) if (dims[j] != d_dims[j]) {
				fprintf(stderr,"Input file sample dimensions do not match: %s\n",srcfiles[i]);
				if (sh_hdl) shlib_close(sh_hdl);
				cvio_cleanup(0);
				exit(1);
			}
			datatype[i] = d_datatype;
		}
		d_ndims = ndims;
		memcpy(d_dims, dims, sizeof(dims));
                iSamples = 1;
                for(j=0;j<ndims;j++) iSamples *= dims[j];
		iSize[i] = (datatype[i] & CVIO_TYP_SIZE_MASK)*iSamples/8;

		/* create the buffers */
		Aucptr[i] = (unsigned char *)malloc(iSize[i]*2);
		Asptr[i] = (short *)(Aucptr[i]);
		Ausptr[i] = (unsigned short *)(Aucptr[i]);
		Afptr[i] = (float *)(Aucptr[i]);

		if (!max_samp) {
			err = cvio_tell(src_s[i],&c_s,&c_t,(uint32_t *)&nsamp);
			if (err) exit_cvio_error(err);
			if (nsamp > max_samp) max_samp = nsamp;
			err = cvio_max_length(src_s[i],(uint32_t *)&nsamp);
			if (nsamp > max_samp) max_samp = nsamp;
		}
		/* seek to first interesting sample */
		if (begin) {
			CVIO_DT_TIME ts[10000];
			int32_t count = begin, toread, err = 0;
			do {
				toread = count > 10000 ? 10000 : count;
				err = cvio_read_next_samples_blocking(src_s[i], ts, NULL, &toread, 0, 0);
				count -= toread;
			} while (count && err == CVIO_ERR_OK);
		}
		/* intial state (empty buffers) */
		iState[i] = 0;
	}

	if (!dontcreate) {
		/* create the destination file */
		err = cvio_create(dstfile,max_samp,iDepth,ndims,dims,iCompress);
		if (err) exit_cvio_error(err);
	}

	/* open the destination file */
	err = cvio_open(dstfile,CVIO_ACCESS_APPEND,&dst_s);
	if (err) exit_cvio_error(err);

	if (dontcreate) {
/*
	If the output stream already exists, its dimensions are allowed
	to be smaller than that of the input streams.
	In this case, we only do computation over its dimensions, but we
	still need the original dimensions so the stat functions can work
	over the entire input volume.
*/
		err=cvio_datatype(dst_s,&iDepth,&d_ndims,d_dims);
		if (err) exit_cvio_error(err);

		if (d_ndims < 3) d_dims[2] = 1;
		if (d_ndims < 2) d_dims[1] = 1;

	}

	/* set up the output buffers */
	datatype[OUT_BUFFER] = iDepth;
	iSize[OUT_BUFFER] = (datatype[OUT_BUFFER] & CVIO_TYP_SIZE_MASK)*
				iSamples/8;
	Aucptr[OUT_BUFFER] = (unsigned char *)malloc(iSize[OUT_BUFFER]);
	Asptr[OUT_BUFFER] = (short *)(Aucptr[OUT_BUFFER]);
	Ausptr[OUT_BUFFER] = (unsigned short *)(Aucptr[OUT_BUFFER]);
	Afptr[OUT_BUFFER] = (float *)(Aucptr[OUT_BUFFER]);

	/* copy from the first file */
	if (cp_attribs) copy_attribs(dst_s,src_s[0]);

	/* read initial sample(s) */
        for(i=0;i<nsrcfiles;i++) {
		int32_t num = 1;
		CVIO_DT_TIME t[2];

		if (i > 0) num = 2;
		err = cvio_read_next_samples_blocking(src_s[i],t,Aucptr[i],
							&num,0, 0);
		if (err && (err != CVIO_ERR_EOF)) exit_cvio_error(err);
		tNext[i] = t[1];
		if (i == 0) tNext[i] = t[0];
		iState[i] = num;
		if (num == 0) {
			fprintf(stderr,
				"Unable to read samples from input files\n");
			goto byebye;
		} else if ((num == 1) && (i != 0)) {
			/* big time value, this sample will last */
			/* usually "EOF" condition */
			iState[i] = -1;
			tNext[i] = 0xfffffffe;  
		}
	}

	/* until the timesource closes */
	iCount = 0;
	while(iState[0] != -1 && iCount < max_samp) {
		int ready = nsrcfiles-1;
		/* read a sample from every file if needed */
		if (iVerbose) printf("Computing sample %d @ %d\n",
					iCount,tNext[0]);
		for(i=1;i<nsrcfiles;i++) {
			if (tNext[i] > tNext[0]) {
				ready -= 1;
			} else {
				/* slide one sample over and read one */
				int32_t num = 1;
				memcpy(Aucptr[i],Aucptr[i]+iSize[i],iSize[i]);
				err = cvio_read_next_samples_blocking(src_s[i],
					&(tNext[i]),Aucptr[i]+iSize[i],&num,0,0);
				if (err == CVIO_ERR_EOF) {
					/* handle EOF case */
					err = CVIO_ERR_OK;
					/* big time value, sample will last */
					/* usually "EOF" condition */
					iState[i] = -1;
					tNext[i] = 0xfffffffe;  
				} else if (err) {
					exit_cvio_error(err);
				}
				if (tNext[i] >= tNext[0]) ready -= 1;
			}
                }
		if (!ready) {
		    int32_t num,x,y,z,idx;

		    /* the work for each "sample" */
		    ID_values[26] = d_dims[0];
		    ID_values[27] = d_dims[1];
		    ID_values[28] = d_dims[2];
		    ID_values[29] = iCount;
		    ID_values[30] = tNext[0];

		    idx = 0;
		    for(z=0;z<d_dims[2];z++) {
			ID_values[25] = z;
		    for(y=0;y<d_dims[1];y++) {
			ID_values[24] = y;
		    for(x=0;x<d_dims[0];x++) {
			int j;
			double v;
			ID_values[23] = x;
			for(j=0;j<nsrcfiles;j++) {
			    switch(datatype[j]) {
			        case CVIO_TYP_BYTE:
	    		            ID_values[j] = Aucptr[j][idx];
				    break;
			        case CVIO_TYP_SHORT:
				    ID_values[j] = Asptr[j][idx];
				    break;
			        case CVIO_TYP_USHORT:
				    ID_values[j] = Ausptr[j][idx];
				    break;
			        case CVIO_TYP_FLOAT:
				    ID_values[j] = Afptr[j][idx];
				    break;
			    }
			}
			v = exp_proc(ID_values);
			switch(datatype[OUT_BUFFER]) {
			    case CVIO_TYP_BYTE:
				Aucptr[OUT_BUFFER][idx] = v;
				break;
			    case CVIO_TYP_SHORT:
				Asptr[OUT_BUFFER][idx] = v;
				break;
			    case CVIO_TYP_USHORT:
				Ausptr[OUT_BUFFER][idx] = v;
				break;
			    case CVIO_TYP_FLOAT:
				Afptr[OUT_BUFFER][idx] = v;
				break;
			}
			idx += 1;
		    }
		    }
		    }

		    /* send it out */
		    err = cvio_add_samples(dst_s,&(tNext[0]),
						Aucptr[OUT_BUFFER],1);
		    if (err) exit_cvio_error(err);
		    iCount += 1;

		    /* read the next output time */
		    num = 1;
		    err = cvio_read_next_samples_blocking(src_s[0],
					&(tNext[0]),Aucptr[0],&num,0,0);
		    if (err == CVIO_ERR_EOF) {
			iState[0] = -1;
			err = CVIO_ERR_OK;
		    } else if (err) {
			exit_cvio_error(err);
		    }
		}
	}

	/* report any errors */
	if (err) exit_cvio_error(err);

	/* cleanup and exit */
byebye:
	for(i=0;i<nsrcfiles;i++) {
		if (src_s[i] != -1) err = cvio_close(src_s[i]);
		if (Aucptr[i]) free(Aucptr[i]);
	}
	err = cvio_close(dst_s);
	free(Aucptr[OUT_BUFFER]);

	if (sh_hdl) shlib_close(sh_hdl);

	cvio_cleanup(0);

	exit(0);
}
