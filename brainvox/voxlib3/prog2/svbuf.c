#include <stdio.h>

void savebuffers(unsigned long *image, unsigned long *zbuf, int dx, int dy)
{

	FILE *f1;

	f1 = fopen("zimage","w");

/* header */
	fwrite(&dx,sizeof(int),1,f1);
	fwrite(&dy,sizeof(int),1,f1);
/* image */
	fwrite(image,sizeof(int),dx*dy,f1);
/* zbuffer */
	fwrite(zbuf,sizeof(int),dx*dy,f1);

	fclose(f1);

}
