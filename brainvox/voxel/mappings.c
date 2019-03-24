/* 
	routines for evaluation a user supplied expression for
	image textures
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#include "macroprotos.h"

long int User_Identifiers(char *name, double *value);

typedef struct {
	char *name;
	double value;
} Identifiers;

#define VV_ID 0
#define TV_ID 1
#define VO_ID 2
#define TO_ID 3
#define VL_ID 4
#define TL_ID 5

Identifiers	ident_list[] = { 	{"vv",0.0},
					{"tv",0.0},
					{"vo",0.0},
					{"to",0.0},
					{"vl",0.0},
					{"tl",0.0},
					{0L,0.0}
				};

long int User_Identifiers(char *name, double *value)
{
	long int i;

	i = 0;
	while (ident_list[i].name != 0L) {
		if (strcmp(ident_list[i].name,name) == 0L) {
			*value = ident_list[i].value;
			return(1);
		}
		i++;
	}
	return(0);
}


void eval_texture_expr_(char *exp,long int *buffer,
	long int *v1,long int *v2,long int *a1,long int *a2,
	long int *err)
{
	char		out[256];
	long int	x,y,j,i;
	double		value;

	*err = 0;
/* setup user defined IDENTIFIER search */
	set_user_identifier(User_Identifiers);
	if (init_parser(exp,1L,0)) goto err_out;
	j = 0;
	for(y=0;y<256;y++) {
		ident_list[TV_ID].value = y;
		ident_list[TL_ID].value = v2[y];
		ident_list[TO_ID].value = a2[y];

#ifdef DEBUG
	printf("\nWorking on texture: %ld :",y);
#endif
		for(x=0;x<256;x++) {
			ident_list[VV_ID].value = x;
			ident_list[VL_ID].value = v1[x];
			ident_list[VO_ID].value = a1[x];
	
			if (parse_math(out,&value) != 0L) goto err_out;
			if (reset_tokenlist_ptr()) goto err_out;
			if (value < 0.0) value = 0.0;
			if (value > 255.0) value = 255.0;
			buffer[j] = value;
			j++; 
		}
	}
/* remove user defined IDENTIFIER search */
	set_user_identifier(0L);
	return;

err_out:
/* remove user defined IDENTIFIER search */
	set_user_identifier(0L);
	*err = 1;
	return;
}
