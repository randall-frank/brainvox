#include <stdio.h>
#include <gl.h>

#define CBASE 64

/* in color index mode select the color closest to the passed cpack value */
/* this is basically a wrapper arounf rgbi */

static long int  cols[14] = {
	0x0000000, 0x00000ff, 0x000ff00, 0x000ffff, 0x0ff0000, 0x0ff00ff,
	0x0ffff00, 0x0ffffff, 0x00080ff, 0x000ff80, 0x080ff00, 0x0ff8000,
	0x0ff0080, 0x08000ff
};

void set_color_index_(long int *it)
{
    	long int mycol;
    	RGBvalue r,g,b;
	Colorindex	i;

    	mycol = *it;

	for(i=0;i<14;i++) {
		if (cols[i] == (mycol & 0x00ffffff)) {
			color(i+CBASE);
			return;
		}
	}

	r = mycol & 0x0000ff;
	g = (mycol & 0x00ff00) >> 8;
	b = (mycol & 0xff0000) >> 16;

/*    	rgbi(r,g,b); */
	printf("Warning: rgbi() not supported\n");
}
void SET_COLOR_INDEX(long int *it)
{
set_color_index_(it);
}


void set_cindex_table_()
{
	Colorindex 	i;
	short		r,g,b;

	for(i=0;i<14;i++) {
		r = cols[i] & 0xff;
		g = ((cols[i] & 0xff00) >> 8);
		b = ((cols[i] & 0xff0000) >> 16);
		mapcolor(i+CBASE,r,g,b);
	}
	return;
}
void SET_CINDEX_TABLE()
{
set_cindex_table_();
}
