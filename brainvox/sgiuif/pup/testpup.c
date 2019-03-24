#include <gl/gl.h>
#include <device.h>

goofball(n) {
    printf("n = %d\n", n);
    return 27;
}

main() {
    long m, m1, m2, which;
    int col = 0;

    foreground();
    prefposition(0, 511, 0, 511);
    winopen("testpup");
    color(BLACK);
    clear();

    m = defpup("submenu %t%s|item 0|item 1|item 2 %l|item 3");
    m1 = defpup("main %t%s|goofball %f|zork %m|gerk %x11|blark %i%x33", goofball, m);
    m2 = defpup("rmenu %t%s|radio 1%r0|radio 2%r0%d|radio 3%r0%l%x119|radio 4%r1|radio 5%r1|radio 6%r1%x55");
    which = m;
    qdevice(RIGHTMOUSE);
    qdevice(LEFTMOUSE);
    qdevice(MIDDLEMOUSE);
    while(1) {
	short d;
	switch(qread(&d)) {
	case REDRAW:
		reshapeviewport();
		clear();
	   break;
	case RIGHTMOUSE:
		printf("%d\n", dopup(which));
	    break;
	case LEFTMOUSE:
	    if (d == 1) {
		which = which == m ? m1 : which == m1 ? m2 : m;
		while(getbutton(LEFTMOUSE)) sginap(2);
	    }
	    break;
	case MIDDLEMOUSE:
	    if (d == 1) {
		col ^= 1; color(col ? BLACK : WHITE);
		clear();
	    }
	    break;
	}
    }
}
