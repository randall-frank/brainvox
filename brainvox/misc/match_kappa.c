#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "proto.h"

long int	verb = 0;

#define MAX_SETS	20

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s) Usage: %s [options] stdpointset pointset [pointset...]\n",__DATE__,s);
	fprintf(stderr,"Options: \n");
	fprintf(stderr,"         -r(color) random point color default:1\n");
	fprintf(stderr,"         -v enable verbose mode default:quiet\n");
	fprintf(stderr,"         -2 no GW levels allowed\n");
	fprintf(stderr,"         -d(pointset) perform discord check\n");
	fprintf(stderr,"         -e(pointset) pointset of points to exclude\n");
	fprintf(stderr,"         -c(consensus) output consensus pointset\n");
	fprintf(stderr,"         -l output location as first 3 columns\n");
	exit(1);
}

int main(int argc,char **argv)
{
	long int	rand_color = 1;
	long int	noGW = 0;
	char		tstr[256];
	long int	i,err,j,k,nsets,bad_point;
	Pset		set[MAX_SETS];
	Pset		save_set,exclude_set,cons_set;
	long int	p[MAX_SETS];
	long int	stdset,firstset,lastset;
	double		d,dd;
	long int	umatched = 0;
	char 		*exclude = NULL, *cons = NULL, *discord = NULL;
	long int	location = 0;

	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
		switch (argv[i][1]) {
			case 'e':
				exclude = &(argv[i][2]);
				break;
			case 'c':
				cons = &(argv[i][2]);
				break;
			case 'r':
				rand_color = atoi(&(argv[i][2]));
				break;
			case 'v':
				verb = 1;
				break;
			case '2':
				noGW = 1;
				break;
			case 'l':
				location = 1;
				break;
			case 'd':
				discord = &(argv[i][2]);
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) < 2) cmd_err(argv[0]);
	stdset = i;
	firstset = i+1;
	lastset = argc;

/* read the pointsets */
	if (read_pset(argv[stdset],&(set[0]))) {
		fprintf(stderr,"read_pset error:%s\n", argv[stdset]);
		ex_err("Unable to read pointset");
	}
	nsets = 1;
	for(i=firstset;i<lastset;i++) {
		if (read_pset(argv[i],&(set[nsets]))) {
			fprintf(stderr,"read_pset error:%s\n", argv[i]);
			ex_err("Unable to read pointset");
		}
		nsets += 1;
	}
	if (exclude) {
		if (read_pset(exclude,&exclude_set)) {
			fprintf(stderr,"Unable to read exclusion pset:%s\n",
				exclude);
			exclude_set.n = 0;
		}
	} else {
		exclude_set.n = 0;
	}
/* for saving */
	save_set.n = 0;
	cons_set.n = 0;

/* for each point in the stdset */
	for(j=0;j<(set[0].n);j++) {
		bad_point = 0;
/* for every poinset */
		for(i=1;i<nsets;i++) {
			d = 256*256*256;
			p[i] = -1;
/* find the closest point to the stdset */
			for(k=0;k<(set[i].n);k++) {
				if ((set[i].c[k] > 1)  && 
					(set[i].c[k] < 5)) {
					dd  = pow(set[i].x[k]-set[0].x[j],2);
					dd += pow(set[i].y[k]-set[0].y[j],2);
					dd += pow(set[i].z[k]-set[0].z[j],2);
					if (dd < d) {
						p[i] = k;
						d = dd;
					}
				}
			}
/* if no point was found, or the nearest point is too far away, reject */
			if ((p[i] == -1) || (d > 3*3*3)) {
				bad_point = 1;
				break;
			}
			if ((noGW) && (set[i].c[p[i]] == 2)) {
				bad_point = 1;
				break;
			}
		}
/* if the exclude set point is too close, make it a bad point */
/* this check should probably be first */
		for(k=0;k<(exclude_set.n);k++) {
			dd  = pow(exclude_set.x[k]-set[0].x[j],2);
			dd += pow(exclude_set.y[k]-set[0].y[j],2);
			dd += pow(exclude_set.z[k]-set[0].z[j],2);
			if (dd < 3*3*3) {
				bad_point = 1;
				break;
			}
		}
/* we have a match!!!  print the values */
		if (bad_point == 0) {
			if (discord) {
/* do the raters agree??? */
				k = 1;
				for(i=2;i<nsets;i++) {
					if (set[i].c[p[i]] != set[1].c[p[1]]) {
						k = -1;
					}
				}
/* if not, add it to the discord pointset */
				if (k == -1) {
					i = save_set.n;
					save_set.x[i] = set[0].x[j];
					save_set.y[i] = set[0].y[j];
					save_set.z[i] = set[0].z[j];
					save_set.c[i] = 1;
					save_set.n++;
if (verb) {
	fprintf(stderr,"%ld %ld %ld\n",set[1].c[p[1]],set[2].c[p[2]],set[3].c[p[3]]);
}
				}
			}
			if (cons) {
				long int	count[3] = {0,0,0};

				for(i=1;i<nsets;i++) {
					if ((set[i].c[p[i]] > 1) &&
					    (set[i].c[p[i]] < 5)) {
						count[set[i].c[p[i]]-2] += 1;
					}
				}
				i = cons_set.n;
				cons_set.x[i] = set[0].x[j];
				cons_set.y[i] = set[0].y[j];
				cons_set.z[i] = set[0].z[j];
				cons_set.c[i] = 2;
				if (count[1] > count[cons_set.c[i]-2]) {
					cons_set.c[i] = 3;
				}
				if (count[2] > count[cons_set.c[i]-2]) {
					cons_set.c[i] = 4;
				}
				cons_set.n++;
			}
/* report what we all found */
			if (location) {
				fprintf(stdout," %.3ld %.3ld %.3ld",
					set[0].x[j],set[0].y[j],set[0].z[j]);
			}
			for(i=1;i<nsets;i++) {
				fprintf(stdout," %ld",set[i].c[p[i]]);
			}
			fprintf(stdout,"\n");
		} else {
			umatched++;
		}
	}
	if (umatched > 0) {
		fprintf(stderr," %ld unmatched points\n",umatched);
	}
	if (discord) {
		strcpy(save_set.title,":Discord points");
		write_pset(discord,&(save_set));
	}

	if (cons) {
		write_pset(cons,&(cons_set));
	}

/* done ! */
	exit(0);
}

