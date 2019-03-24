/*
 *  david blythe  jan 1991  drb@clsc.utoronto.ca
 *  ontario centre for large scale computation
 *  copyright notice:
 *	  copyright 1991 OCLSC
 *        anyone is free to alter or distribute this code as long as this
 *	  entire header remains unaltered.
 */

typedef long Menu;

#define F_RETVAL	0x001	/* %x */	/* basic sgi */
#define F_SUBMEN	0x002	/* %m */
#define F_FUNC		0x004	/* %f */
#define F_NFUNC		0x008	/* %n */
#define F_ULINE		0x010	/* %l */

#define F_MARK		0x020	/* %i */	/* extensions */
#define F_UMARK		0x040	/* %I */
#define F_RADIO		0x080	/* %r */
#define F_GENER		0x100	/* %g */

#define F_DISBLE	0x200	/* %d */	/* disabled entry */

/* MASKS */
#define M_SUBMEN	~(F_MARK|F_UMARK|F_GENER|F_RADIO)
#define M_RADIO		~(F_SUBMEN|F_UMARK|F_GENER)
#define M_MARK		~(F_SUBMEN|F_UMARK|F_GENER)
#define M_UMARK		~(F_SUBMEN|F_MARK|F_RADIO|F_GENER)
#define M_GENER		~(F_SUBMEN|F_MARK|F_UMARK|F_RADIO)

#define M_RGROUP	0xf000	/* bits where radio group number is stored */
#define RGROUP_SHIFT	12	/* # bits to shift to get there */

#define F_STRETCH	(F_SUBMEN|F_MARK|F_UMARK|F_RADIO)

struct element {
    char *name;			/* element name */
    int flags;			/* element flags */
    int  ret;			/* retrun value from %x */
    char *func;			/* menu function from %n or %f */
};

#define BUTTON			/* allow mouse button specification */
#define N_RADIO		10	/* maximum number of radio groups */
#define SAVEPOS			/* remember last selection */

#ifdef SAVEPOS
#define S_NOSAVE	-2	/* don't save the last pick */
#define S_UNSET		-1	/* save it, but we haven't had one yet */
#endif

struct menu {
    char *title;		/* pointer to menu title %t */
    int  nel;			/* number of elements */
    struct element *elements;	/* pointer to first menu element */
    char *mfunc;		/* menu function  %F */
    int  ox, oy, cx, cy;	/* corners of menu */
    int  box, boy, bcx, bcy;	/* bounding box */
#ifdef BUTTON
    int button;			/* which button  %b */
#endif
#ifdef N_RADIO
    int nradio[N_RADIO];	/* # elements in radio group */
    int *radio[N_RADIO];	/* lists of elements in a radio group */
#endif
#ifdef SAVEPOS
    int savepos;		/* last menu item picked */
#endif
};
