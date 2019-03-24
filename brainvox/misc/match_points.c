#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "proto.h"

double          ipixel = 1.0;
double          islice = 1.0;
double		squf = 1.0;
long int	interp = 1;
long int	verb = 0;

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s) Usage: %s [options] Gray White CSF inpointset outpointset\n",__DATE__,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:1\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -r(color) random point color default:1\n");
	fprintf(stderr,"         -p(interpixel) interpixel spacing default:1.0\n");
	fprintf(stderr,"         -s(interslice) interslice spacing default:1.0\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -v enable verbose mode default:quiet\n");
	exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 1;
	long int	istart = 1;
	long int	iend = 119;
	long int	istep = 1;
	long int	swab = 0;
	long int	rand_color = 1;
	char		*Gtemp,*Wtemp,*Ctemp,tstr[256],*setname,*outname;
	long int	i,err,j,z,k;
	unsigned char	*Gd,*Wd,*Cd;
	Pset		set;
	float		G,W,C,S;

	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
		switch (argv[i][1]) {
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
			case 'r':
				rand_color = atoi(&(argv[i][2]));
				break;
                        case 'p':
                                ipixel = atof(&(argv[i][2]));
                                break;
                        case 's':
                                islice = atof(&(argv[i][2]));
                                break;
			case 'b':
				swab = 1;
				break;
			case 'v':
				verb = 1;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 5) cmd_err(argv[0]);
	Gtemp = argv[i+0];
	Wtemp = argv[i+1];
	Ctemp = argv[i+2];
	setname = argv[i+3];
	outname = argv[i+4];
	if (read_pset(setname,&set)) ex_err("Unable to read pointset");

/* compute squeeze factor 
 Compute the number of interpolated slices in the resampled dataset
*/
        squf = 10.0;
	k = iend-istart+1;
        j = -1;
        while ((squf > 1.0) && (j < 20)) {
		j = j + 1;
                squf = ((double)(k)*islice)/((double)(k + (j*(k-1)))*ipixel);
	}
	interp = j + 1;

#ifdef DEBUG
	printf("sqf,ip,is: %f %f %f %d\n",squf,ipixel,islice,interp);
#endif

/* get the image memory */
	Gd = malloc(dz*dx*dy);
	if (Gd == 0L) ex_err("Unable to allocate image memory.");
	Wd = malloc(dz*dx*dy);
	if (Wd == 0L) ex_err("Unable to allocate image memory.");
	Cd = malloc(dz*dx*dy);
	if (Cd == 0L) ex_err("Unable to allocate image memory.");

/* read the images */
	z = 0;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(Gtemp,i,&err,tstr);
		fprintf(stderr,"Reading the image file:%s\n",tstr);
		bin_io(tstr,'r',Gd,dx,dy,dz,1,swab,0L);
		name_changer(Wtemp,i,&err,tstr);
		fprintf(stderr,"Reading the image file:%s\n",tstr);
		bin_io(tstr,'r',Wd,dx,dy,dz,1,swab,0L);
		name_changer(Ctemp,i,&err,tstr);
		fprintf(stderr,"Reading the image file:%s\n",tstr);
		bin_io(tstr,'r',Cd,dx,dy,dz,1,swab,0L);

/* classify the points on this slice */
		for(j=0;j<(set.n);j++) {
			if ((set.c[j] == rand_color) && 
			    (z == (set.z[j]/interp))) {
				k = (set.y[j]*dx)+set.x[j];
				switch(dz) {
					case 1:
						G = ((unsigned char *)Gd)[k];
						W = ((unsigned char *)Wd)[k];
						C = ((unsigned char *)Cd)[k];
						break;
					case 2:
						G = ((unsigned short *)Gd)[k];
						W = ((unsigned short *)Wd)[k];
						C = ((unsigned short *)Cd)[k];
						break;
					case 4:
						G = ((float *)Gd)[k];
						W = ((float *)Wd)[k];
						C = ((float *)Cd)[k];
						break;
					default:
						ex_err("Unknown dz size");
						break;
				}
#ifdef DEBUG
	printf("Found: %ld %ld %ld %ld -> %f %f %f\n",
		set.x[j],set.y[j],set.z[j],set.c[j],G,W,C);
#endif

/* classify the point */
				set.c[j] = 0;
				S = G+W+C;
				G = G/S; W = W/S; C = C/S;

#ifdef ORIG_RULES
				if (G > 0.50) {
					set.c[j] = 1; /* red */
				}
				if (W > 0.50) {
					set.c[j] = 3; /* yellow */
				} 
				if (C > 0.50) {
					set.c[j] = 4; /* blue */
				}
				if (((G+W) > 0.85) && (fabs(G-W) < 0.15)) {
					set.c[j] = 2; /* green */
				} 
/* alt rules 
				if (G > (C+W)) set.c[j] = 1;
				if (W > (C+G)) set.c[j] = 3;
				if (C > (G+W)) set.c[j] = 4;
				if (((G+W)>0.80)&&(fabs(G-W)<0.20)) set.c[j]=2;
*/
#else
				if ((G > C) && (G > W)) set.c[j] = 2;
				if ((W > G) && (W > C)) set.c[j] = 4;
				if ((C > G) && (C > W)) set.c[j] = 3;
#endif

#ifdef DEBUG
	printf("Class: %ld %ld %ld %ld -> %f %f %f\n",
		set.x[j],set.y[j],set.z[j],set.c[j],G,W,C);
#endif
			}
		}

		z += 1;
	}

/* write the results */
	write_pset(outname,&set);

/* done */
	free(Gd);
	free(Wd);
	free(Cd);
	exit(0);
}

