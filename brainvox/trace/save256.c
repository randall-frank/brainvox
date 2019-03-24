#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void save256_(short *array,int s,char *name)
{
   FILE *fp;
   unsigned char *tmp;
   int i,j,x,y;

   fp = fopen(name,"wb");
   if (!fp) return;

   tmp = (unsigned char *)malloc(s*s);

   i = 0;
   for(y=0;y<s/2;y++) {
      j = ((s/2) - y - 1)*(s/2);
      for(x=0;x<s/2;x++) {
         tmp[j++] = array[i++] - 256; i++;
      }
      i = i + s;
   }

   fwrite(tmp,s*s/4,1,fp);
   
   free(tmp);

   fclose(fp);

   return;
}
void SAVE256(short *array,int s,char *name)
{
save256_(array,s,name);
}
