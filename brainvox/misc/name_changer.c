#include <stdio.h>
#include <string.h>

void name_changer(char *in, long int num, long int *error, char *out)
{
	long int per2ptr,per1ptr;
	long int i,j,x,st,k;
	char	temp[30];

/*	assume error */
	*error = 1;
	strcpy(out,in);
/* 	check for valid num */
	if (num < 0) {
		return;
	}
/* convert num into a string */
	if ( sprintf(temp,"%020ld STOP",num) < 0) return;
/* this is the start point for the * code */
	for(st=0; temp[st] == '0'; st++);
	if (num == 0) st = 19;

	per1ptr = -1;
	per2ptr = 0;
	x = 0;
	for(i=0;i<strlen(in);i++) {
		if (in[i] == '*') {
/* insert number */
			for(j=st; j < 20; j++) out[x++] = temp[j];
		} else if (in[i] == '%') {
/* catch first % and copy */
			if (per1ptr == -1) per1ptr = x;
			per2ptr = x;
			out[x++] = in[i];
		} else {
			if (per1ptr != -1) {
/* replace with number */
				j=19-(per2ptr-per1ptr);
				for(k=per1ptr; k <= per2ptr; k++) {
					out[k] = temp[j++];
				}
				per1ptr = -1;
			}
/* copy */
			out[x++] = in[i];
		}
	}
	if (per1ptr != -1) {
/* replace with number */
		j=19-(per2ptr-per1ptr);
		for(k=per1ptr; k <= per2ptr; k++) {
			out[k] = temp[j++];
		}
		per1ptr = -1;
	}
/* terminate */
	out[x] = '\0';
	*error = 0;
	return;
}
