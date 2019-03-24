#include <stdio.h>
#include <string.h>
#include <malloc.h>

main(argc,argv)
int argc;
char **argv;
{
	int dx,dy;
	int i,j,j1,j2;
	FILE *fp;
	unsigned char *img,lut[768],temp;
	char outfilename[100];

	if (argc < 6) { 
		printf("usage: %s stubname binfile dx dy lutfile {-x}\n",argv[0]);
		exit(1);
	}
	
	strcpy(outfilename,argv[1]);
	strcat(outfilename,".c");
	
	dx = atoi(argv[3]);
	dy = atoi(argv[4]);

	if (dx*dy <= 0) exit(1);
	
	img = (unsigned char *)malloc(dx*dy);
	if (img == 0) exit(1);

	/* read the image file */
	fp = fopen(argv[2],"r");
	if (fp == 0) exit(1);
	(void) fread(img,1,dx*dy,fp);
	(void) fclose(fp);

/* check for flip over x axis */
	if (argc > 6) {
		if (strcmp(argv[6],"-x") == 0) {
			for(j=0; j < (dy/2); j++) {
				j1 = j*dx;
				j2 = (dy-j-1)*dx;
				for(i=0; i < dx; i++) {
					temp = img[j1+i];
					img[j1+i] = img[j2+i];
					img[j2+i] = temp;
				}
			}
		}
	}
	
	/* read the palette file */
	fp = fopen(argv[5],"r");
	if (fp == 0) exit(1);
	(void) fread(lut,1,768,fp);
	(void) fclose(fp);

/* write out the file */
	fp = fopen(outfilename,"w");
	if (fp == 0) exit(1);
	
	fprintf(fp,"/* file generated from binary file %s \n",argv[2]);
        fprintf(fp,"   and palette %s by mkploticon */\n",argv[5]);
	fprintf(fp,"#include <stdio.h>\n#include <malloc.h>\n#include <device.h>\n#include <gl.h>\n#include <get.h>\n");
	fprintf(fp,"#define DX %d\n#define DY %d\n",dx,dy);
	fprintf(fp,"static unsigned char image_%s[] = {\n",argv[1]);
	j = 0;
	for(i=0; i < dx*dy; i++) {
		fprintf(fp,"%#2x,",(int)img[i]);
		j++;
		if (j == 10) {
			j = 0;
			fprintf(fp,"\n");
			}
	}
	fprintf(fp,"};\n");
	fprintf(fp,"static unsigned char lut_%s[] = {\n",argv[1]);
	j = 0;
	for(i=0; i< 768; i++) {
		fprintf(fp,"%#2x,",(int)lut[i]);
		j++;
		if (j == 10) {
			j = 0;
			fprintf(fp,"\n");
			}
	}
	fprintf(fp,"};\n");
	fprintf(fp,"static int lastm_%s = -1L;\nstatic char *imb_%s = 0L;\n",argv[1],argv[1]);
	fprintf(fp,"void drawicon_%s_(int x, int y)\n{\n",argv[1]);
	fprintf(fp,"   int i,j,newmode;\n");
	fprintf(fp,"   newmode = getdisplaymode();\n");
	fprintf(fp,"   if (newmode == DMRGBDOUBLE) newmode = DMRGB;\n");
	fprintf(fp,"   if (newmode == DMDOUBLE) newmode = DMSINGLE;\n");
	fprintf(fp,"   if ((newmode != lastm_%s) && (newmode == DMRGB)) {\n",argv[1]);
	fprintf(fp,"	  if (imb_%s != 0L) free(imb_%s);\n",argv[1],argv[1]);
	fprintf(fp,"	  imb_%s = (char *)malloc(DX*DY*sizeof(int));\n",argv[1]);
	fprintf(fp,"	  for(i=0; i < DX*DY; i++) {\n");
	fprintf(fp,"		j = image_%s[i];\n",argv[1]);
	fprintf(fp,"		((int *)imb_%s)[i] = lut_%s[j] + (lut_%s[j+256]*256) + (lut_%s[j+512]*65536);\n",argv[1],argv[1],argv[1],argv[1]);
	fprintf(fp,"	  }\n");
	fprintf(fp,"   }\n");
	fprintf(fp,"   if ((newmode != lastm_%s) && (newmode == DMSINGLE)) {\n",argv[1]);
	fprintf(fp,"	  if (imb_%s != 0L) free(imb_%s);\n",argv[1],argv[1]);
	fprintf(fp,"	  imb_%s = (char *)malloc(DX*DY*sizeof(short));\n",argv[1]);
	fprintf(fp,"	  for(i=0; i < DX*DY; i++) {\n");
	fprintf(fp,"		j = image_%s[i];\n",argv[1]);
	fprintf(fp,"		((short *)imb_%s)[i] = rgbi(lut_%s[j],lut_%s[j+256],lut_%s[j+512]);\n",argv[1],argv[1],argv[1],argv[1]);
	fprintf(fp,"	  }\n");
	fprintf(fp,"   }\n");
	fprintf(fp,"   lastm_%s = newmode;\n",argv[1]);
	fprintf(fp,"   if (lastm_%s == DMRGB) lrectwrite(x,y,x+DX-1,y+DY-1,imb_%s);\n",argv[1],argv[1]);
	fprintf(fp,"   if (lastm_%s == DMSINGLE) rectwrite(x,y,x+DX-1,y+DY-1,imb_%s);\n",argv[1],argv[1]);
	fprintf(fp,"}\n");
	
	(void) fclose(fp);

}

