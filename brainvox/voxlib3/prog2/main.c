#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

main(argc,argv)
int argc;
char **argv;
{
    int dx,dy,start,end,step,i,count;
    int interp;
    char *template;
    unsigned char *data;
    char *opacity,*lut,*opacity2,*lut2;
    char nfile[2];
    double squf;

    nfile[0]='-';
    nfile[1]='\0';

    if( argc<9 ) {
	fprintf(stderr,
	"usage: %s template dx dy start end step interp squf {{{{palfile1} opacityfile1} palfile2} opacityfile2}\n",
		argv[0]);
	exit(1);
    } 

    template = argv[1];
    dx = atoi(argv[2]);
    dy = atoi(argv[3]);
    start = atoi(argv[4]);
    end = atoi(argv[5]);
    step = atoi(argv[6]);
    interp = atoi(argv[7]);
    squf = atof(argv[8]);

    opacity = nfile;
    lut = nfile;
    opacity2 = nfile;
    lut2 = nfile;

/* read colors/opacity */
    if (argc >= 10 ) {
	lut = argv[9];
        lut2 = lut;
	};
    if (argc >= 11 ) {
	opacity = argv[10];
        opacity2 = opacity;
	};
    if (argc >= 12 ) {
	lut2 = argv[11];
	}
    if (argc >= 13 ) {
	opacity2 = argv[12];
	}

	count =0;
	for(i=start; i<=end; i = i + step) count=count+1; 

/* handle interpolations */
	count = count + (interp * (count-1));

/* allocate memory for the data x*y*count */
    (void) printf("Allocating %d bytes of memory.\n",count*dx*dy);
    data = malloc((count*dx*dy));
    if (data == NULL) {
	(void) printf("Unable to allocate image memory.\n");
	exit(1);
	}

    (void) vl_read_images_(template,dx,dy,start,end,step,interp,data);
    (void) render(dx,dy,count,lut,opacity,lut2,opacity2,data,squf);

    (void) free(data);
    exit(0);
}

