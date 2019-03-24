#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

/*
interpixel
interslice
startslice
endslice
#
plusrois
#
minusrois
#
obj1roi1
obj1roi2
#objectname1
obj2roi1
obj2roi2
#objectname2
*/

#define MNUM 20

void	strip(char *s)
{
	long int	i;

	i = strlen(s);
	if (s[i-1] == '\n') s[i-1] = '\0';
	
	return;
}

int main(int argc,char **argv)
{
	FILE 	*fp,*pd,*od;
	char	tstr[256];

	double		is,ip;
	long int	start,end,i;
	long int	radius = 1;

	long int	prois,mrois,orois;
	char		prois_n[MNUM][256];
	char		mrois_n[MNUM][256];
	char		orois_n[MNUM][256];
	char		mvol[256],ivol[256],prog[256];
	time_t		tt;

	if (strstr(argv[0],"fast") != 0) radius = 0;

	if (argc != 3) {
		printf("Usage: %s inputfile outputfile\n",argv[0]);
		printf("Input file format:\n");
		printf("interpixel spacing \n");
		printf("interslice spacing \n");
		printf("first slice number \n");
		printf("last slice number \n");
		printf("#\n");
		printf("List of ROI templates for positive surface\n");
		printf("#\n");
		printf("List of ROI templates for holes in positive surface\n");
		printf("#\n");
		printf("List of ROI templates for object1\n");
		printf("#name of object1\n");
		printf("List of ROI templates for object2\n");
		printf("#name of object2\n");
		printf("List of ROI templates for object3\n");
		printf("#name of object3\n");
		printf("...\n");
		printf("List of ROI templates for last object\n");
		printf("#name of last object\n");
		exit(1);
	}

	od = fopen(argv[2],"w");
	if (od == 0L) {
		printf("Unable to open file:%s\n",argv[2]);
	}
	getcwd(tstr,256);
	fprintf(od,"Dir:%s\n",tstr);
	fprintf(od,"Input:%s Output:%s\n",argv[1],argv[2]);
	tt = time(0L);
	fprintf(od,"Start:%s\n",ctime(&tt));
	fflush(od);

	fp = fopen(argv[1],"r");
	if (fp == 0L) {
		printf("Unable to open file:%s\n",argv[1]);
	}

/* read header */
	fgets(tstr,256,fp);
	sscanf(tstr,"%lf",&ip);
	fgets(tstr,256,fp);
	sscanf(tstr,"%lf",&is);
	fgets(tstr,256,fp);
	sscanf(tstr,"%ld",&start);
	fgets(tstr,256,fp);
	sscanf(tstr,"%ld",&end);
	
	printf("is=%lf ip=%lf start=%ld end=%ld\n",is,ip,start,end);

	while(fgets(tstr,256,fp)) {
		if (tstr[0] == '#') break;
	}

/* read plus rois */
	prois = 0;
	while(fgets(tstr,256,fp)) {
		if (tstr[0] == '#') break;
		strip(tstr);
		strcpy(prois_n[prois],tstr);
		prois++;
	}

/* read minus rois */
	mrois = 0;
	while(fgets(tstr,256,fp)) {
		if (tstr[0] == '#') break;
		strip(tstr);
		strcpy(mrois_n[mrois],tstr);
		mrois++;
	}

/* build the image file... */
	strcpy(ivol,"base_vol_%%%.pic.Z");
	strcpy(mvol,"mask_vol_%%%.pic.Z");
	
/* plus */
	for(i=0;i<prois;i++) {
		printf("Adding Plus ROI:%s\n",prois_n[i]);
		if (i == 0) {
		sprintf(prog,"tal_cutting -f%ld -l%ld -z1 -o -v255 '%s' %s %s",
			start,end,prois_n[i],"-",ivol);
		} else {
		sprintf(prog,"tal_cutting -f%ld -l%ld -z1 -o -v255 '%s' %s %s",
			start,end,prois_n[i],ivol,ivol);
		}
		printf("Running:%s\n",prog);
		system(prog);
	}
/* minus */
	for(i=0;i<mrois;i++) {
		printf("Removing Minus ROI:%s\n",mrois_n[i]);
		sprintf(prog,"tal_cutting -f%ld -l%ld -z1 -o -v0 '%s' %s %s",
			start,end,mrois_n[i],ivol,ivol);
		printf("Running:%s\n",prog);
		system(prog);
	}

/* handle objects */
	while (1) {
/* read object rois */
		orois = 0;
		while(1) {
			if (fgets(tstr,256,fp) == 0) goto out;
			if (tstr[0] == '#') break;
			strip(tstr);
			strcpy(orois_n[orois],tstr);
			orois++;
		}
		printf("Computing object:%s\n",&(tstr[1]));
/* build mask volume */
		for(i=0;i<orois;i++) {
			printf("Adding Object ROI:%s\n",orois_n[i]);
			if (i == 0) {
		sprintf(prog,"tal_cutting -f%ld -l%ld -z1 -o -v255 '%s' %s %s",
				start,end,orois_n[i],"-",mvol);
			} else {
		sprintf(prog,"tal_cutting -f%ld -l%ld -z1 -o -v255 '%s' %s %s",
				start,end,orois_n[i],mvol,mvol);
			}
		printf("Running:%s\n",prog);
			system(prog);
		}
		fprintf(od,"Object:%s\n",&(tstr[1]));
		fflush(od);

/* compute the surface area (and volume) */
		sprintf(prog,"tal_surface -r%ld -f%ld -l%ld -p%lf -s%lf -m%s %s",
			radius,start,end,ip,is,mvol,ivol);
		printf("Running:%s\n",prog);
		pd = popen(prog,"r");
		if (pd == 0L) exit(1);

		while (fgets(tstr,256,pd) != 0) fprintf(od,tstr);

		pclose(pd);

		fprintf(od,"\n");
		fflush(od);

	}

out:
	tt = time(0L);
	fprintf(od,"End:%s\n",ctime(&tt));

	fclose(od);
	fclose(fp);
	exit(0);
}
