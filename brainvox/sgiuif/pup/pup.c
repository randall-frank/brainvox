/*
 *  david blythe  jan 1991  drb@clsc.utoronto.ca
 *  ontario centre for large scale computation
 *  copyright notice:
 *	  copyright 1991 OCLSC
 *        anyone is free to alter or distribute this code as long as this
 *	  entire header remains unaltered.
 */

static char vers[] = "$Header: /resdata/realtime/NeuroCVSArchive/cvsroot/brainvox/sgiuif/pup/pup.c,v 1.1.1.1 2001/05/18 22:24:09 rjfrank Exp $";

#include <gl.h>
#include <device.h>
#include <varargs.h>

#include "pup.h"
#include "font.h"

void addtopup(Menu pup,char *str,long arg);
Menu defpup(char *str, void *d);

Menu
newpup() {
    struct menu *menu;
    char *malloc();

    if ((menu = (struct menu *)malloc(sizeof(struct menu))) == NULL) return -1;
    bzero(menu, sizeof(struct menu));
#ifdef BUTTON
    menu->button = RIGHTMOUSE;
#endif
#ifdef SAVEPOS
    menu->savepos = S_NOSAVE;
#endif
    return (Menu) menu;
}

Menu
defpup(str, va_alist)
char *str;
va_dcl
{
    struct menu *menu;
    Menu m;
    char *malloc();
    va_list vp;
    char *p, *savestr();
    struct element *el = 0;

    if ((m = newpup()) < 0) return -1;
    menu = (struct menu *)m;

    va_start(vp);

    /* now parse the argument list */
    for (p = str;;) {
	int flags = 0;
	char *func, *text;
	int retval, tflag = 0;
#ifdef F_RADIO
	int group;
#endif

	/* skip blanks */
	while(*str == ' ') str++;

	while(*p != '\0' && *p != '|' && *p != '%') *p++;
	if (p == str) break;
	text = savestr(str, p-str);
	/* parse options */
	while(*p == '%') {
	    switch(*++p) {
#ifdef SAVEPOS
	    case 's': menu->savepos = S_UNSET; p++; break;
#endif
	    case 't':
		menu->title = text; p++; tflag++; break;
	    case 'x':
		retval = atoi(++p);
		flags |= F_RETVAL;
		while(*p >= '0' && *p <= '9') p++; break;
	    case 'F':
		menu->mfunc = va_arg(vp, char *); ++p; break;
	    case 'd':
		flags |= F_DISBLE; ++p; break;
	    case 'f':
		func = va_arg(vp, char *); flags |= F_FUNC; ++p;
		flags &= ~(F_NFUNC|F_SUBMEN); break;
	    case 'n':
		func = va_arg(vp, char *); flags |= F_NFUNC; ++p;
		flags &= ~(F_FUNC|F_SUBMEN); break;
	    case 'm':
		func = va_arg(vp, char *); flags |= F_SUBMEN; ++p;
#ifdef F_MARK
		flags &= ~(F_FUNC|F_NFUNC|F_MARK|F_UMARK); break;
#else
		flags &= ~(F_FUNC|F_NFUNC); break;
#endif
	    case 'l':
		flags |= F_ULINE; ++p; break;
#ifdef BUTTON
	    case 'b':
		if (p[1] < '1' || p[1] > '3') return -1;
		*p++;
		menu->button = *p == '1' ? LEFTMOUSE : *p == '3' ? RIGHTMOUSE :
				MIDDLEMOUSE;
		++p; break;
#endif
#ifdef F_MARK
	    case 'i':
		flags &= ~(F_MARK|F_SUBMEN); flags |= F_UMARK; ++p; break;
	    case 'I':
		flags &= ~(F_UMARK|F_SUBMEN); flags |= F_MARK; ++p; break;
	    case 'r':
		if (p[1] < '0' || p[1] > '9') return -1; group = *++p - '0';
		flags &= M_RADIO; flags |= F_RADIO;  ++p; break;
#endif
	    default:
		return -1;
	    }
	    while(*p == ' ') p++;
	}

	if (*p != '|' && *p != '\0') return -1;
	if (*p == '|') ++p;
	str = p;

	if (tflag) continue;

	if (!menu->elements)
	    menu->elements = (struct element *)malloc(sizeof(struct element));
	else
	    menu->elements = (struct element *)realloc(menu->elements, sizeof(struct element)*(menu->nel+1));
	if (!menu->elements)
	    return -1;
	el = menu->elements+menu->nel;
	el->func = func;
	if (!(flags & F_RETVAL)) {
	    retval = el-menu->elements+1;
	    flags |= F_RETVAL;
	}
	el->ret = retval;
	el->flags = flags;
	el->name = text;
	el->name = text;
#ifdef F_RADIO
	if (flags&F_RADIO) {
	    if (!menu->radio[group])
		menu->radio[group] = (int *)malloc(sizeof(int));
	    else
		menu->radio[group] = (int *)realloc(menu->radio[group], sizeof(int)*(menu->nradio[group]+1));
	    menu->radio[group][menu->nradio[group]] = menu->nel;
	    menu->nradio[group]++;
	    el->flags |= group << RGROUP_SHIFT;
	}
#endif
	menu->nel++;
    }
    va_end(vp);
    return (Menu) menu;
}

char *
savestr(s, l)
char *s;
{
    char *p, *malloc();
    if (!l) l = strlen(s);
    p = malloc(l+1);
    if (p) {
	bcopy(s, p, l);
	p[l] = '\0';
    }
    return p;
}

void
addtopup(pup, str, arg)
Menu pup;
char *str;
long arg;
{
    struct menu *menu = (struct menu *)pup;
    char *p, *savestr();
    struct element *el = 0;
    int used_arg = 0;
#ifdef F_RADIO
    int group;
#endif

    if (pup == 0 || pup == -1 || str == 0) return;

    /* now parse the argument list */
    for (p = str;;) {
	int flags = 0;
	char *func, *text;
	int retval, tflag = 0;

	/* skip blanks */
	while(*str == ' ') str++;

	while(*p != '\0' && *p != '|' && *p != '%') *p++;
	if (p == str) break;
	text = savestr(str, p-str);
	/* parse options */
	while(*p == '%') {
	    switch(*++p) {
#ifdef BUTTON
	    case 'b':
		if (p[1] < '1' || p[1] > '3') return /*-1*/;
		*p++;
		menu->button = *p == '1' ? LEFTMOUSE : *p == '3' ? RIGHTMOUSE :
				MIDDLEMOUSE;
		++p; break;
#endif
#ifdef SAVEPOS
	    case 's': menu->savepos = S_UNSET; p++; break;
#endif
	    case 't':
		menu->title = text; p++; tflag++; break;
	    case 'x':
		retval = atoi(++p);
		flags |= F_RETVAL;
		while(*p >= '0' && *p <= '9') p++; break;
	    case 'F':
		if (used_arg) return /*-1*/;
		used_arg++;
		menu->mfunc = (char *) arg; ++p; break;
	    case 'd':
		flags |= F_DISBLE; ++p; break;
	    case 'f':
		if (used_arg) return /*-1*/;
		used_arg++;
		func = (char *)arg; flags |= F_FUNC; ++p; break;
	    case 'n':
		if (used_arg) return /*-1*/;
		used_arg++;
		func = (char *)arg; flags |= F_NFUNC; ++p; break;
	    case 'm':
		if (used_arg) return /*-1*/;
		used_arg++;
#ifdef F_MARK
		flags &= ~(F_MARK|F_UMARK);
#endif
		func = (char *)arg; flags |= F_SUBMEN; ++p; break;
	    case 'l':
		flags |= F_ULINE; ++p; break;
#ifdef F_MARK
	    case 'i':
		flags &= ~(F_MARK|F_SUBMEN); flags |= F_UMARK; ++p; break;
	    case 'I':
		flags &= ~(F_UMARK|F_SUBMEN); flags |= F_MARK; ++p; break;
	    case 'r':
		if (p[1] < '0' || p[1] > '9') return; group = *++p - '0';
		flags &= M_RADIO; flags |= F_RADIO;  ++p; break;
#endif
	    default:
		return /*-1*/;
	    }
	    while(*p == ' ') p++;
	}
	if (*p != '|' && *p != '\0') return /*-1*/;
	if (*p == '|') ++p;
	str = p;

	if (tflag) continue;

	if (!menu->elements)
	    menu->elements = (struct element *)malloc(sizeof(struct element));
	else
	    menu->elements = (struct element *)realloc(menu->elements, sizeof(struct element)*(menu->nel+1));
	if (!menu->elements)
	    return /*-1*/;
	el = menu->elements+menu->nel;
	el->func = func;
	if (!(flags & F_RETVAL)) {
	    retval = el-menu->elements+1;
	    flags |= F_RETVAL;
	}
	el->ret = retval;
	el->flags = flags;
	el->name = text;
#ifdef F_RADIO
	if (flags&F_RADIO) {
	    if (!menu->radio[group])
		menu->radio[group] = (int *)malloc(sizeof(int));
	    else
		menu->radio[group] = (int *)realloc(menu->radio[group], sizeof(int)*(menu->nradio[group]+1));
	    menu->radio[group][menu->nradio[group]] = menu->nel;
	    menu->nradio[group]++;
	    el->flags |= group << RGROUP_SHIFT;
	}
#endif
	menu->nel++;
    }
    return /*0*/;
}

void
freepup(pup)
Menu pup;
{
    struct menu *menu = (struct menu *)pup;
    struct element *e;

    for(e = menu->elements; menu->nel; e++, menu->nel-- ) {
	if (e->name) free(e->name);
    }
    free((char *)menu->elements);
    if (menu->title) free(menu->title);
    free((char *)menu);
}

void
changepup(pup, item, str, arg)
Menu pup;
long item;
char *str;
long arg;
{
    struct menu *menu = (struct menu *)pup;
    char *p, *savestr();
    struct element *el = 0;
    int used_arg = 0;
    int oflags;
#ifdef F_RADIO
    int group;
#endif
    int flags = 0;
    char *func, *text;
    int retval, tflag = 0;

    if (pup == 0 || pup == -1 || str == 0) return;
    item--;
    if (item < -1 || item >= menu->nel) return;

    oflags = menu->elements[item].flags;

    /* now parse the argument list */
    p = str;

    /* skip blanks */
    while(*str == ' ') str++;

    while(*p != '\0' && *p != '%') *p++;
    text = savestr(str, p-str);
    /* parse options */
    while(*p == '%') {
	if (item == -1) {	/* title stuff only */
	    switch(*++p) {
	    case 't':
		if (menu->title && menu->title != text) free(text);
		menu->title = text; p++; break;
	    case 'F':
		menu->mfunc = (char *)arg; ++p; break;
#ifdef BUTTON
	    case 'b':
		if (p[1] < '1' || p[1] > '3') return /*-1*/;
		*p++;
		menu->button = *p == '1' ? LEFTMOUSE : *p == '3' ? RIGHTMOUSE :
				MIDDLEMOUSE;
		++p; break;
#endif
#ifdef SAVEPOS
	    case 's':
		if (menu->savepos == S_NOSAVE)
		    menu->savepos = S_UNSET;
		else
		    menu->savepos = S_NOSAVE;
		p++; break;
#endif
	    }
	} else {
	    switch(*++p) {	/* anything else */
	    case 'x':
		retval = atoi(++p);
		flags |= F_RETVAL;
		while(*p >= '0' && *p <= '9') p++; break;
	    case 'd':
		if (oflags&F_DISBLE) flags &= ~F_DISBLE;
		else flags |= F_DISBLE; ++p; break;
	    case 'f':
		if (used_arg) return /*-1*/;
		used_arg++;
		func = (char *)arg; flags |= F_FUNC; ++p; break;
	    case 'n':
		if (used_arg) return /*-1*/;
		used_arg++;
		func = (char *)arg; flags |= F_NFUNC; ++p; break;
	    case 'm':
		if (used_arg) return /*-1*/;
		used_arg++;
#ifdef F_MARK
		flags &= ~(F_MARK|F_UMARK);
#endif
		func = (char *)arg; flags |= F_SUBMEN; ++p; break;
	    case 'l':
		flags |= F_ULINE; ++p; break;
#ifdef F_MARK
	    case 'i':
		flags &= ~(F_MARK|F_SUBMEN); flags |= F_UMARK; ++p; break;
	    case 'I':
		flags &= ~(F_UMARK|F_SUBMEN); flags |= F_MARK; ++p; break;
	    case 'r':
		if (p[1] < '0' || p[1] > '9') return; group = *++p - '0';
		flags &= M_RADIO; flags |= F_RADIO;  ++p; break;
#endif
	    default:
		return /*-1*/;
	    }
	}
	while(*p == ' ') p++;
    }
    if (*p != '\0') return /*-1*/;
    if (item == -1) return;

    /* tidy up */
    el = menu->elements+item;
    if (el->name) free(el->name);
#ifdef F_RADIO
    if (el->flags & F_RADIO) {
	int j, k, group;
	/* delete from the radio group */
	group = (el->flags&M_RGROUP) >> RGROUP_SHIFT;
	for(k = j = 0; j < menu->nradio[group]; j++) {
	    if(menu->radio[group][j] != item)
		menu->radio[group][k++] = menu->radio[group][j];
	}
	if (--menu->nradio[group] == 0) {
	    free((char *)menu->radio[group]);
	    menu->radio[group] = 0;
	}
    }
#endif

    el->func = func;
    if (!(flags & F_RETVAL)) {
	retval = el-menu->elements+1;
	flags |= F_RETVAL;
    }
    el->ret = retval;
    el->flags = flags;
    el->name = text;
#ifdef F_RADIO
    if (flags&F_RADIO) {
	if (!menu->radio[group])
	    menu->radio[group] = (int *)malloc(sizeof(int));
	else
	    menu->radio[group] = (int *)realloc(menu->radio[group], sizeof(int)*(menu->nradio[group]+1));
	menu->radio[group][menu->nradio[group]] = el - menu->elements;
	menu->nradio[group]++;
	el->flags |= group << RGROUP_SHIFT;
    }
#endif
    return /*0*/;
}

void
setpup(pup, item, mode)
Menu pup;
long item, mode;
{
    struct menu *menu = (struct menu *)pup;

    if (pup == 0 | pup == -1) return;

    if (item < 1 || item > menu->nel) return;
    --item;
    if (mode == PUP_NONE)
	menu->elements[item].flags &= ~F_DISBLE;
    else if (mode == PUP_GREY)
	menu->elements[item].flags |= F_DISBLE;
}

long
dopup(pup)
Menu pup;
{
    long val;
    short cursor;
    Colorindex dummy, dummy2;
    Matrix mp, mv;
    Boolean vis, zmode;
    long matmode;

    if ((matmode = getmmode()) == MVIEWING) {
	getmatrix(mv); mmode(MPROJECTION); getmatrix(mp);
    } else if (matmode == MPROJECTION) {
	getmatrix(mp);
    } else {
	pushmatrix();
    }
    pushviewport(); pushattributes(); fullscrn();
    zmode = getzbuffer(); zbuffer(FALSE);
    drawmode(PUPDRAW);
    getcursor(&cursor, &dummy, &dummy2, &vis);
    curson(); setpattern(0); font(0); linewidth(1);
    mapcolor(PUP_COLOR,(short)128,(short)128,(short)128);
/* rjf Patch for 8bit Indigo */
    mapcolor(PUP_BLACK,(short)0,(short)0,(short)0);
    mapcolor(PUP_WHITE,(short)255,(short)255,(short)255);
    val = _dopup((struct menu *)pup, (struct menu *)0);
    setcursor(cursor,0,0);
    if (!vis) cursoff();
    drawmode(NORMALDRAW);	/* seem to need this */
    zbuffer(zmode);
    endfullscrn(); popattributes(); popviewport();
    if (matmode == MVIEWING) {
	mmode(MPROJECTION); loadmatrix(mp);
	mmode(MVIEWING); loadmatrix(mv);
    } else if (matmode == MPROJECTION) {
	mmode(MPROJECTION);
	loadmatrix(mp);
    } else {
	popmatrix();
    }
    if (pup != 0 && pup != -1)
#ifdef BUTTON
	qfilter(((struct menu *)pup)->button);
#else
	qfilter(RIGHTMOUSE);
#endif

    return val;
}

#define EL_HEIGHT	20
#define M_SLOP_Y	5
#define EL_SLOP		2	/* menu text baseline offset */
#define M_SLOP_X	5
#define HILIGHT_FUDGE	6
#define BORDER		3
#define SO		9	/* shadow offset */
#define HEADER_HEIGHT	13	/* header height */
#define TITLE_HEIGHT	22	/* header with title height */
#define HEADER_OFFSET	(5+2*BORDER)
#define TITLE_SLOP	7	/* title text baseline offset */
#define FUDGE_X		16	/* how far menu is offset from the cursor */
#define ARROW_FUDGE	(M_SLOP_X+10)	/* offset for submenu glyph */


#define CURSOR_PAT	(2768)
#define WHITE_PAT	(2768)
#define BLACK_PAT	(2767)
#define GLYPHS		(2768)
#ifdef FONT_HEIGHT
#define FONT		(2767)
#else
#define FONT		0
#endif

static
_dopup(menu, parent)
struct menu *menu, *parent;
{
    int i, n, lit = -1;
    struct element *el;
    int sub = 0;
    int omx, omy;
    int off = -1;

    if ((int)menu == -1 || menu == 0) return -1; /* trust no one */
    if (!menu->nel) return -1;

    setup_menu(menu);

    /* wait for button down */
#ifdef BUTTON
    while (!getbutton(menu->button)) ;
#else
    while (!getbutton(RIGHTMOUSE)) ;
#endif

#ifdef BUTTON
    while(getbutton(menu->button)) {
#else
    while(getbutton(RIGHTMOUSE)) {
#endif
	int mx, my;
	sginap(2);
	mx = getvaluator(MOUSEX);
	my = getvaluator(MOUSEY);
#define abs(a) ((a) < 0 ? -(a) : (a))
	/* spasticness */
	if (abs(omx-mx) < 2 && abs(omy-my) < 2) continue;
	if (mx > menu->ox && mx < menu->cx &&
	    my > menu->oy && my < menu->cy-M_SLOP_Y) {
	    n = menu->nel - (my - menu->oy)/EL_HEIGHT - 1;
	    if (n != lit) {
		if (lit != -1) unhilight(menu, menu->elements+lit, lit);
		if (menu->elements[n].flags&F_DISBLE) {
		    lit = -1; goto next;
		}
		lit = n;
		el = menu->elements+lit;
		hilight(menu, el, lit);
		if (off != -1 && lit != off) off = -1;
	    }
	    if (el->flags&F_SUBMEN) {
		/* if we are within 1/2 of the right edge */
		if (off == -1  && mx > menu->cx - (menu->cx - menu->ox)/2) {
		    off = lit;
		    lit = _dopup((struct menu *)el->func, menu);
		    if (lit != -1) {
#ifdef SAVEPOS
			if (menu->savepos > S_NOSAVE) menu->savepos = off;
#endif
			sub = 1; break;
		    } else {
			/* repair the damaged menu */
			draw_menu(menu);
		    }
		} else if (off != -1 && mx < menu->cx - (menu->cx - menu->ox)/2)
		    off = -1;
	    }
	} else {
	    if (lit != -1) unhilight(menu, menu->elements+lit, lit);
	    lit = -1;
	    if (parent) {
		/* check if we have moved out of the submenu into the
		 * parent memory
		 */
		if ( (mx > parent->box && mx < parent->bcx &&
		      my > parent->boy && my < parent->bcy) &&
		    !(mx > menu->box && mx < menu->bcx &&
		      my > menu->boy && my < menu->bcy) ) break;
	    }
	}
next:
	omx = mx; omy = my;
    }
    clear_menu(menu);

    if (lit != -1 && !sub) {
	int (*fptr)();
	struct element *el = menu->elements+lit;

	if (el->flags&(F_DISBLE|F_SUBMEN)) return -1;
#ifdef SAVEPOS
	if (menu->savepos > S_NOSAVE) menu->savepos = lit;
#endif
	lit = el->ret;

	if (el->flags&(F_FUNC|F_NFUNC)) {
	    fptr = (int (*)())el->func;
	    lit = fptr(lit);
	}
	if (menu->mfunc && !(el->flags&F_NFUNC)) {
	    fptr = (int (*)())menu->mfunc;
	    lit = fptr(lit);
	}
#ifdef F_MARK
	if (el->flags&F_RADIO) {
	    if (!(el->flags&F_MARK)) {
		/* turn off the previous marked element */
		int i, group = (el->flags&M_RGROUP) >> RGROUP_SHIFT;
		for(i = 0; i < menu->nradio[group]; i++) {
		    if (((menu->elements[menu->radio[group][i]].flags)&(F_RADIO|F_MARK)) ==
			(F_RADIO|F_MARK))
			menu->elements[menu->radio[group][i]].flags &= ~F_MARK;
		}
	    }
	    el->flags |= F_MARK;
	}
	else if (el->flags&F_MARK)
	    el->flags = (el->flags & ~F_MARK) | F_UMARK;
	else if (el->flags&F_UMARK)
	    el->flags = (el->flags & ~F_UMARK) | F_MARK;
#endif
    }
    return lit;
}

static
draw_menu(menu)
struct menu *menu;
{
    int i, extra;
    struct element *e;

    extra = HEADER_OFFSET + (menu->title ? TITLE_HEIGHT : HEADER_HEIGHT);

#define SHADOWS
#ifdef SHADOWS
    color(PUP_BLACK); setpattern(BLACK_PAT);
    sboxfi(menu->ox+SO-BORDER, menu->oy-SO-BORDER,
	   menu->cx+SO+BORDER, menu->cy-SO+BORDER);
    sboxfi(menu->ox+SO-BORDER, menu->cy-SO+HEADER_OFFSET-BORDER,
	   menu->cx+SO+BORDER, menu->cy-SO+extra+BORDER);
    setpattern(0);
#endif

    /* borders */
    color(PUP_BLACK);
    sboxfi(menu->ox-BORDER, menu->oy-BORDER,
	   menu->cx+BORDER, menu->cy+BORDER);
    sboxfi(menu->ox-BORDER, menu->cy+HEADER_OFFSET-BORDER,
	   menu->cx+BORDER, menu->cy+extra+BORDER);

    /* menu head */
/*    setpattern(BLACK_PAT);
    sboxfi(menu->ox, menu->cy+HEADER_OFFSET, menu->cx, menu->cy+extra);
    color(PUP_WHITE);
    setpattern(WHITE_PAT);
    sboxfi(menu->ox, menu->cy+HEADER_OFFSET, menu->cx, menu->cy+extra);
    setpattern(0); */

    color(PUP_COLOR);
    sboxfi(menu->ox, menu->cy+HEADER_OFFSET, menu->cx, menu->cy+extra);
    color(PUP_BLACK);

    if (menu->title) {
	color(PUP_BLACK);
	cmov2i(menu->ox+M_SLOP_X, menu->cy+HEADER_OFFSET+TITLE_SLOP);
	charstr(menu->title);
    }

    color(PUP_WHITE);
    sboxfi(menu->ox, menu->oy, menu->cx, menu->cy);
    color(PUP_BLACK);
    for(i = 1, e = menu->elements; i <= menu->nel; e++, i++) {
	cmov2i(menu->ox+M_SLOP_X, menu->cy+EL_SLOP - i*EL_HEIGHT);
	charstr(e->name);
	if (e->flags&F_SUBMEN) {
	    /* draw little arrow at the end */
	    static char abuf[2] = {1, 0};
	    cmov2i(menu->cx-ARROW_FUDGE, menu->cy+EL_SLOP - i*EL_HEIGHT);
	    font(GLYPHS); charstr(abuf); font(FONT);
	}
#ifdef F_MARK
	else if (e->flags&F_MARK) {
	    /* draw little box at the end */
	    static char gbuf[2] = {2, 0};
	    cmov2i(menu->cx-ARROW_FUDGE, menu->cy+EL_SLOP - i*EL_HEIGHT);
	    font(GLYPHS); charstr(gbuf); font(FONT);
	}
#endif
	if (e->flags&F_DISBLE) { /* scary stuff kids */
	    color(PUP_WHITE);
	    setpattern(WHITE_PAT);
	    sboxfi(menu->ox+1, menu->cy+EL_SLOP-i*EL_HEIGHT+1-HILIGHT_FUDGE,
	       menu->cx-1, menu->cy+EL_SLOP-(i-1)*EL_HEIGHT-1-HILIGHT_FUDGE);
	    setpattern(0);
	    color(PUP_BLACK);
	}
	if (e->flags&F_ULINE) {
           int v[2];
           v[0] = menu->ox;
           v[1] = menu->cy+EL_SLOP - i*EL_HEIGHT-HILIGHT_FUDGE;
           bgnline(); v2i(v); v[0] = menu->cx; v2i(v); endline();
	}
    }
}

static
setup_menu(menu)
struct menu *menu;
{
    int sx, sy;
    int cx, cy;
    struct element *e;
    int i, extra;
    static short pat[17] = {
	0xaaaa,0x5555,0xaaaa,0x5555,0xaaaa,0x5555,0xaaaa,0x5555,
	0xaaaa,0x5555,0xaaaa,0x5555,0xaaaa,0x5555,0xaaaa,0x5555,
	0xaaaa
    };
#ifdef F_MARK
    static short arrow[13*2] = {
	0x0100,0x0180,0x01c0,0x01e0,0x3ff0,0x3ff8,0x3ffc,	/* arrow */
	0x3ff8,0x3ff0,0x01e0,0x01c0,0x0180,0x0100,
	0x0000,0x1ff0,0x1ff0,0x1ff0,0x1ff0,0x1ff0,0x1ff0,	/* square */
	0x1ff0,0x1ff0,0x1ff0,0x0000,0x0000,0x0000
    };
    static Fontchar arrowchar[3] = {
	{0,0,0,0,0,0}, {0,16,13,0,0,13}, {13,16,13,0,0,13}
    };
#else
    static short arrow[13] = {
	0x0100,0x0180,0x01c0,0x01e0,0x3ff0,0x3ff8,0x3ffc,
	0x3ff8,0x3ff0,0x01e0,0x01c0,0x0180,0x0100
    };
    static Fontchar arrowchar[2] = {{0,0,0,0,0,0}, {0,16,13,0,0,13}};
#endif
#define OUTLINE
#ifdef OUTLINE
    static short c_arrow[2][16] = {
	0x0000, 0x0000, 0x0000, 0x0020, 0x0030, 0x0038, 0x003c, 0x7ffe, 
	0x7fff, 0x7ffe, 0x003c, 0x0038, 0x0030, 0x0020, 0x0000, 0x0000, 
	0x0000, 0x0040, 0x0060, 0x0050, 0x0048, 0x0044, 0xffc2, 0x8001, 
	0x8000, 0x8001, 0xffc2, 0x0044, 0x0048, 0x0050, 0x0060, 0x0040, 
    };
#else
    static short c_arrow[16] = {
	0x0000, 0x0000, 0x0040, 0x0060, 0x0070, 0x0078, 0xfffc, 0xfffe, 
	0xffff, 0xfffe, 0xfffc, 0x0078, 0x0070, 0x0060, 0x0040, 0x0000, 
    };
#endif

#define min(a,b) (a) < (b) ? (a) : (b)
#define max(a,b) (a) > (b) ? (a) : (b)


#ifdef FONT_HEIGHT
    defrasterfont(FONT, FONT_HEIGHT, FONT_CHARS, fontchar, sizeof fontras, fontras);
    font(FONT);
#endif

    sy = menu->nel*EL_HEIGHT+M_SLOP_Y;
    extra = HEADER_OFFSET + (menu->title ? TITLE_HEIGHT : HEADER_HEIGHT);

    for(i = sx = 0, e = menu->elements; i < menu->nel; i++, e++) {
	int width = strwidth(e->name) + ARROW_FUDGE * ((e->flags&F_STRETCH) != 0);
	sx = max(sx, width);
    }
    if (menu->title)
	sx = max(sx, strwidth(menu->title));
    sx += 2*M_SLOP_X;

    menu->box = 0; menu->bcx = sx+2*BORDER;
    menu->boy = 0; menu->bcy = sy+extra+2*BORDER;

    cx = getvaluator(MOUSEX); cy = getvaluator(MOUSEY);

    cx = min(cx, XMAXSCREEN-menu->bcx-FUDGE_X); cx = max(cx, FUDGE_X);
#ifdef SAVEPOS
    if (menu->savepos > S_UNSET)
	i = extra+menu->savepos*EL_HEIGHT + 2*EL_HEIGHT/3 + EL_SLOP;
    else
#endif
    i = (extra-(HEADER_OFFSET-BORDER))/2;  /* FUDGE_Y */
    cy = min(cy, YMAXSCREEN-i); cy = max(cy, menu->bcy-i);
    setvaluator(MOUSEX, cx, 0, XMAXSCREEN);
    setvaluator(MOUSEY, cy, 0, YMAXSCREEN);
    cx -= FUDGE_X; cy += i;

    cy -= sy+extra;

    menu->box += cx; menu->bcx += cx;
    menu->boy += cy; menu->bcy += cy;

    menu->ox = cx; menu->oy = cy;
    menu->cx = cx+sx-1; menu->cy = cy+sy-1;

    /* arrow cursor */
#ifdef OUTLINE
    curstype(C16X2);
#else
    curstype(C16X1);
#endif
    defcursor(CURSOR_PAT, c_arrow);
    curorigin(CURSOR_PAT, 15, 8);
    setcursor(CURSOR_PAT,0,0);
    /* arrow pattern for submenus */
#ifdef F_MARK
    defrasterfont(GLYPHS, 13, 3, arrowchar, sizeof arrow, arrow);
#else
    defrasterfont(GLYPHS, 13, 2, arrowchar, sizeof arrow, arrow);
#endif

    defpattern(BLACK_PAT, 16, pat);	/* BLACK pattern */
    defpattern(WHITE_PAT, 16, pat+1);	/* WHITE pattern */

    draw_menu(menu);
}

static
hilight(menu, el, i)
struct menu *menu;
struct element *el;
{
    color(PUP_BLACK);
    sboxfi(menu->ox+1, menu->cy+EL_SLOP-(i+1)*EL_HEIGHT+1-HILIGHT_FUDGE,
	   menu->cx-1, menu->cy+EL_SLOP-i*EL_HEIGHT-1-HILIGHT_FUDGE);
    cmov2i(menu->ox+M_SLOP_X, menu->cy+EL_SLOP- (i+1)*EL_HEIGHT);
    color(PUP_WHITE);
    charstr(el->name);
    if (el->flags&F_SUBMEN) {
	/* draw little arrow at the end */
	static char abuf[2] = {1, 0};
	cmov2i(menu->cx-ARROW_FUDGE, menu->cy+EL_SLOP-(i+1)*EL_HEIGHT);
	font(GLYPHS); charstr(abuf); font(FONT);
    }
#ifdef F_MARK
    else if (el->flags&F_MARK) {
	/* draw little box at the end */
	static char gbuf[2] = {2, 0};
	cmov2i(menu->cx-ARROW_FUDGE, menu->cy+EL_SLOP-(i+1)*EL_HEIGHT);
	font(GLYPHS); charstr(gbuf); font(FONT);
    }
#endif
}

static
unhilight(menu, el, i)
struct menu *menu;
struct element *el;
{
    color(PUP_WHITE);
    sboxfi(menu->ox+1, menu->cy+EL_SLOP-(i+1)*EL_HEIGHT+1-HILIGHT_FUDGE,
	   menu->cx-1, menu->cy+EL_SLOP-i*EL_HEIGHT-1-HILIGHT_FUDGE);
    cmov2i(menu->ox+M_SLOP_X, menu->cy+EL_SLOP - (i+1)*EL_HEIGHT);
    color(PUP_BLACK);
    charstr(el->name);
    if (el->flags&F_SUBMEN) {
	/* draw little arrow at the end */
	static char abuf[2] = {1, 0};
	cmov2i(menu->cx-ARROW_FUDGE, menu->cy+EL_SLOP-(i+1)*EL_HEIGHT);
	font(GLYPHS); charstr(abuf); font(FONT);
    }
#ifdef F_MARK
    else if (el->flags&F_MARK) {
	/* draw little box at the end */
	static char gbuf[2] = {2, 0};
	cmov2i(menu->cx-ARROW_FUDGE, menu->cy+EL_SLOP-(i+1)*EL_HEIGHT);
	font(GLYPHS); charstr(gbuf); font(FONT);
    }
#endif
}

static
clear_menu(menu)
struct menu *menu;
{
    int extra;

    extra = HEADER_OFFSET + (menu->title ? TITLE_HEIGHT : HEADER_HEIGHT);

    color(PUP_CLEAR);
    sboxfi(menu->ox-BORDER, menu->oy-BORDER,
	   menu->cx+BORDER, menu->cy+BORDER);
    sboxfi(menu->ox-BORDER, menu->cy+HEADER_OFFSET-BORDER,
	   menu->cx+BORDER, menu->cy+extra+BORDER);
#ifdef SHADOWS
    sboxfi(menu->ox+SO-BORDER, menu->oy-SO-BORDER,
	   menu->cx+SO+BORDER, menu->cy-SO+BORDER);
    sboxfi(menu->ox+SO-BORDER, menu->cy-SO+HEADER_OFFSET-BORDER,
	   menu->cx+SO+BORDER, menu->cy-SO+extra+BORDER);
#endif
}

qfilter(but) {
    /* throw away mouse button events we don't want to see */
#define BSIZE 20
    short buf[BSIZE];
    int i, k, n = blkqread(buf, BSIZE);
    for(k = i = 0; i < n; i+=2) {
	if (buf[i] == but || buf[i] == MOUSEX || buf[i] == MOUSEY) continue;
	buf[k++] = buf[i];
	buf[k++] = buf[i+1];
    }
    /* read some more */
    if (n == BSIZE) qfilter(but);
    /* reenter the ones we want */
    for(i = 0; i < k; i+=2)
	qenter(buf[i], buf[i+1]);
}
#define FORTRAN
#ifdef FORTRAN
/*
 * fortran bindings
 */

void
freepu_(pup)
Menu *pup;
{
    freepup(*pup);
}

Menu
newpup_() {
    return newpup();
}

void
addtop_(pup, str, len, arg)
Menu *pup;
char *str;
int *len, *arg;
{
    char *s = 0, *malloc();
    if (*len > 0) {
	s = malloc(*len+1); bcopy(str, s, *len); s[*len] = '\0';
    }
    addtopup(*pup, s, *arg);
    if (s) free(s);
}

void
change_(pup, item, str, len, arg)
Menu *pup;
char *str;
long *item, *len, *arg;
{
    char *s = 0, *malloc();
    if (*len > 0) {
	s = malloc(*len+1); bcopy(str, s, *len); s[*len] = '\0';
    }
    changepup(*pup, *item, s, *arg);
    if (s) free(s);
}

void
setpup_(pup, item, mode)
Menu *pup;
long *item, *mode;
{
    setpup(*pup, *item, *mode);
}

long
dopup_(pup)
Menu *pup;
{
    return dopup(*pup);
}
#endif /* FORTRAN */
