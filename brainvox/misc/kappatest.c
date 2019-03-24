#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DLOOK(n,col)  ((n*nraters)+col)
#define TLOOK(n,col)  ((n*n_levels)+col)

#define MAX_LEVELS	100

void ex_err(char *s);
void cmd_err(char *s);
long int get_level(char *level,char levels[MAX_LEVELS][256],long int *n_levels);

void ex_err(char *s)
{
        fprintf(stderr,"Fatal error:%s\n",s);
        exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s) Usage: %s [options] inputfile\n",__DATE__,s);
	fprintf(stderr,"Options:  -s summary output only\n");
	fprintf(stderr,"If inputfile is '-' the program will read from stdin\n");
	fprintf(stdout,"summary:kappa stdev consper nraters levels subjects\n");
	exit(1);
}

long int get_level(char *level,char levels[MAX_LEVELS][256],long int *n_levels)
{
	long int	i;
	
	for(i=0;i<(*n_levels);i++) {
		if (strcmp(level,levels[i]) == 0) {
			return(i);
		}
	}
	i = (*n_levels);

	if (i >= MAX_LEVELS-1) return(-1);

	strcpy(levels[i],level);
	*n_levels = i + 1;

	return(i);
}

int main(int argc, char **argv)
{
	FILE		*in = stdin;
	long int	i,j;
	char		text[256],t1[256],*t2,t3[256];
	long int	nraters = -1;
	long int	summary = 0;
	long 		*data;
	long int	npts;
	long int	allocpts;

	long int	n_levels = 0;
	char		levels[MAX_LEVELS][256];

	double		*Pj,p,pe;
	long 		*tab;
	double		n,K,R,N,sum;
	double		kappa,stdev;


/* read command line options */
	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
		switch (argv[i][1]) {
			case 's':
				summary = 1;
				break;
			default:
				cmd_err(argv[0]);
		}
		i++;
	}
/* get command line parameters */
	if ((argc-i) != 1) cmd_err(argv[0]);

/* get the input file */
	if (strcmp(argv[i],"-") != 0) {
		in = fopen(argv[i],"r");
		if (in == 0L) ex_err("Unable to open input file.");
	}

/* Read the input file */
        while (fgets(text,256,in) != NULL) {
		t2 = strchr(text,'\n');
		if (t2) *t2 = '\0';
                if (nraters == -1) {
/* search text for number of columns */
                        strcpy(t1,text);
                        t2 = strtok(t1,"\t ");
                        nraters = 0;
                        while (t2 != NULL) {
                                nraters++;
                                t2 = strtok(NULL,"\t ");
                        }
#ifdef DEBUG
                        fprintf(stderr,"Found %ld columns\n",nraters);
#endif
/* allocate initial memory */
                        allocpts = 500;
                        npts = 0;
                        data=(long *)malloc(sizeof(long)*allocpts*nraters);
                        if (data == 0L) ex_err("Unable to allocate memory.");
                }
                if (npts+10 > allocpts) {
                        allocpts += 500;  /* space for 500 more lines */
                        data=(long *)realloc(data,sizeof(long)*allocpts*
				nraters);
                        if (data == 0L) ex_err("Unable to allocate memory.");
#ifdef DEBUG
                        fprintf(stderr,"Allocated %ld rows\n",allocpts);
#endif
                }
                strcpy(t1,text);
                t2 = strtok(t1,"\t ");
                i = 0;
                while ((t2 != NULL) && (i < nraters)) {
			sscanf(t2,"%s",t3);
			j = get_level(t3,levels,&n_levels);
                        data[DLOOK(npts,i)] = j;
#ifdef DEBUG
	printf("%ld ",j);
#endif
			if (j == -1) ex_err("Only 100 levels can be used");
                        i++;
                        t2 = strtok(NULL,"\t ");
                }
#ifdef DEBUG
	printf("\n");
#endif
                npts++;
        }
#ifdef DEBUG
        fprintf(stderr,"Read %ld rows %ld columns\n",npts,nraters);
#endif
	if (in != stdin) fclose(in);

/* validity check */
	if (n_levels <= 1) {
		ex_err("Numbers of levels must be greater than one.");
	}
	if (npts <= 1) {
		ex_err("Numbers of observations must be greater than one.");
	}
	if (nraters <= 1) {
		ex_err("Numbers of raters must be greater than one.");
	}

/* time to compute the kappa statistics */
/* first the new table */
	tab = (long int *)malloc(sizeof(long)*npts*n_levels);
	if (tab == 0L) ex_err("Unable to allocate additonal memory");
/* clear the table */
	for(i=0;i<npts;i++) {
	for(j=0;j<n_levels;j++) {
		tab[TLOOK(i,j)] = 0;
	}
	}
/* count up the values */
	sum = 0;
	for(i=0;i<npts;i++) {
	for(j=0;j<nraters;j++) {
		tab[TLOOK(i,data[DLOOK(i,j)])] += 1;
		sum += 1.0;
	}
	}
	K = nraters;
	R = n_levels;
	N = npts;
/* now compute the Pj values */
	Pj = (double *)malloc(sizeof(double)*n_levels);
	if (Pj == 0L) ex_err("Unable to allocate additonal memory");

/* compute Pj */
	for(j=0;j<n_levels;j++) {
		Pj[j] = 0;
		for(i=0;i<npts;i++) {
			n = tab[TLOOK(i,j)];
			Pj[j] +=  n;
		}
		Pj[j] /= sum;
	}

/* compute p */
	p = 0.0;
	for(i=0;i<npts;i++) {
		for(j=0;j<n_levels;j++) {
			n = tab[TLOOK(i,j)];
			p += (n*n);
		}
	}
	p = (1.0/(N*K*(K-1)))*(p - (N*K));

/* compute pe */
	pe = 0;
	sum = 0.0;
	for(j=0;j<n_levels;j++) {
		pe += (Pj[j]*Pj[j]);
		sum += (Pj[j]*Pj[j]*Pj[j]);
	}

/* finally Kappa! */
	kappa = (p - pe)/(1.0 - pe);
	stdev = pe - ((2.0*K) - 3.0)*pe*pe + 2.0*(K-2.0)*sum;
	stdev = stdev/((1.0 - pe)*(1.0 - pe));
	stdev = (2.0*stdev)/(N*K*(K-1.0));
	stdev = sqrt(stdev);

	if (summary) {
		fprintf(stdout,"%lf\t%lf\t%lf\t%lf\t%lf\t%lf\n",
			kappa,stdev,p*100.0,K,R,N);
	} else {
		fprintf(stdout,"Kappa\t%lf\tStdev\t%lf\tZ\t%lf\n",
			kappa,stdev,kappa/stdev);
		fprintf(stdout,"ConsensusPercent\t%lf\n",p*100.0);
		fprintf(stdout,"Raters\t%lf\tLevels\t%lf\tSubjects\t%lf\n",
			K,R,N);
	}

/* done ! */
	free(data);
	free(tab);
	free(Pj);

	exit(0);
}
