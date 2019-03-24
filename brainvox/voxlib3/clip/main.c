#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "2dpolygon.h"

long int main(long int argc,char **argv)
{
	char		str[256];
	long int	x1,y1,x2,y2,l,t,r,b;
	long int	j;
	long int	out[2],nloops;
	long int	vidd[2],vid[2];

	l = (0 << 16);
	r = (255 << 16);
	t = (255 << 16);
	b = (0 << 16);
	while(fgets(str,256,stdin) != 0) {
		if (str[0] == 'R') {
			sscanf(&(str[1]),"%ld %ld %ld %ld\n",&l,&t,&r,&b);
			l = l << 16;
			r = r << 16;
			t = t << 16;
			b = b << 16;
		} else {
		if (sscanf(str,"%ld %ld %ld %ld\n",&x1,&y1,&x2,&y2) == 4) {
			nloops = sqrt(((x1-x2)*(x1-x2)) + ((y1-y2)*(y1-y2)));
			nloops += 1;
			vid[0] = (x1<<16)+ 0x00008000;
			vid[1] = (y1<<16)+ 0x00008000;
			vidd[0] = ((x2 - x1)<<16)/(nloops-1);
			vidd[1] = ((y2 - y1)<<16)/(nloops-1);
			x2 = (vid[0] + (vidd[0]*(nloops-1))) >> 16;
			y2 = (vid[1] + (vidd[1]*(nloops-1))) >> 16;
			printf("Clipping: %ld,%ld %ld,%ld to %ld,%ld %ld,%ld\n",
				x1,y1,x2,y2,l>>16,t>>16,r>>16,b>>16);
			j = vl_clipline(vid,vidd,nloops,l,b,r,t,out);
			if (j == 0) {
				printf("Line outside the rect\n");
			} else {
				x1 = (vid[0] + (vidd[0]*out[0])) >> 16;
				y1 = (vid[1] + (vidd[1]*out[0])) >> 16;
				x2 = (vid[0] + (vidd[0]*out[1])) >> 16;
				y2 = (vid[1] + (vidd[1]*out[1])) >> 16;
				printf("Line clipped to:%ld,%ld %ld,%ld\n",
					x1,y1,x2,y2);
			}
		}
		}
	}

	exit(0);
}
