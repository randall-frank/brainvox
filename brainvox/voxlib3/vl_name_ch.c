#include <stdio.h>
#include <string.h>
#include "voxel.h"

 void vl_name_changer(VLCHAR *in, VLINT32 num, VLINT32 *error, VLCHAR *out)
{
	VLINT32 per2ptr,astptr,per1ptr;
	VLINT32 i,j,x;
	VLCHAR	temp[30];

/*	assume error */
	*error = 1;
	strcpy(out,in);
/* 	check for valid num */
	if (num < 0) {
		return;
	}
	per1ptr = -1;
	per2ptr = -1;
	astptr = -1;
	for(i = 0; in[i] != '\0'; i++) {
		if (in[i] == '*') astptr = i;
		/* find first % */
		if ((in[i] == '%') && (per1ptr == -1)) per1ptr = i;
		/* find last % */
		if (in[i] == '%') per2ptr = i;
		}

/* error out if more than one field is specified or no fields are specified */
	if ((astptr != -1) && (per1ptr != -1)) return; 
	if ((astptr == -1) && (per1ptr == -1)) return;

/* convert num into a string */
	if ( sprintf(temp,"%020ld STOP",num) < 0) return;

/* now there are two cases: * and % */
	if (astptr != -1) {

/* this is the * code */
		for(j=0; temp[j] == '0'; j++);
		if (num == 0) j = 19;

		x = 0;
/* copy to the * */
		for(i=0; i < astptr; i++) {
			out[x] = in[i];
			x++;
		}
/* paste in the number string */
		for(i=j; i < 20; i++) {
			out[x] = temp[i];
			x++;
		}
/* and the rest of the in sting... */
		for(i=astptr+1;in[i] != '\0' ; i++) {
			out[x] = in[i];
			x++;
		}
		out[x] = '\0';
	} else {

/* this is the % code */

/* copy to the in string completely  */
		for(i=0; in[i] != '\0'; i++) {
			out[i] = in[i];
		}
		out[i] = in[i];

/* paste in the number string */
		x=19-(per2ptr-per1ptr);
		for(i=per1ptr; i <= per2ptr; i++) {
			out[i] = temp[x];
			x++;
		}
/* check for error when output field is too short */
		x=19-(per2ptr-per1ptr);
		for(i=0;i<x;i++) {
			if (temp[i] != '0') return;
		}
	}
/*	ok to return */
	*error = 0;
	return;
}
