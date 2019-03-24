#define COLORIN(v2, MSA, RBcolor, Gcolor, image, zbuf)	\
{							\
	register long int RB;					\
	register long int b2;					\
	register long int ptr;					\
	register long int im_val;				\
	ptr = ((v2)[0] + ((v2)[1]*imagex) );	\
	im_val = (image)[ptr];				\
	b2 = im_val & 0xff00ff;				\
	RB = ((RBcolor) + b2*(MSA)) & 0xff00ff00;	\
	b2 = im_val & 0x00ff00;				\
	RB = RB | (((Gcolor) + b2*(MSA)) & 0x00ff0000);		\
	(image)[ptr] = (RB) >> 8;			\
	(zbuf)[ptr] = (v2)[2];				\
}

#define ZCOLORIN(v2, MSA, RBcolor, Gcolor, image, zbuf)       \
{                                                     \
      register long int RB;                                   \
      register long int b2;                                   \
      register long int ptr;                                  \
      register long int im_val;                               \
      ptr = ((v2)[0] + ((v2)[1]*imagex) );    \
      if ((zbuf)[ptr] < (v2)[2]) {                    \
      im_val = (image)[ptr];                          \
      b2 = im_val & 0xff00ff;                         \
      RB = ((RBcolor) + b2*(MSA)) & 0xff00ff00;       \
      b2 = im_val & 0x00ff00;                         \
      RB = RB | (((Gcolor) + b2*(MSA)) & 0x00ff0000);         \
      (image)[ptr] = (RB) >> 8;                       \
      (zbuf)[ptr] = (v2)[2];          }               \
}


#define FCOLORIN(v2, MSA, RBcolor, Gcolor, addtab, image, iy2)	\
{							\
	long int RB,G;					\
	long int b2;					\
	long int ptr;					\
	long int im_val;				\
	ptr = ((v2)[0] + (addtab)[((v2)[1] + (iy2))] );	\
	im_val = (image)[ptr];				\
	b2 = im_val & 0xff00ff;				\
	RB = ((RBcolor) + b2*(MSA)) & 0xff00ff00;	\
	b2 = im_val & 0x00ff00;				\
	G = ((Gcolor) + b2*(MSA)) & 0x00ff0000;		\
	(image)[ptr] = (RB | G) >> 8;			\
}

#define ZFCOLORIN(v2, MSA, RBcolor, Gcolor, addtab, image, iy2, zbuf)	\
{							\
	register long int RB;					\
	register long int b2;					\
	register long int ptr;					\
	register long int im_val;				\
	ptr = ((v2)[0] + (addtab)[((v2)[1] + (iy2))] );	\
	im_val = (image)[ptr];				\
	b2 = im_val & 0xff00ff;				\
	RB = ((RBcolor) + b2*(MSA)) & 0xff00ff00;	\
	b2 = im_val & 0x00ff00;				\
	RB = RB | (((Gcolor) + b2*(MSA)) & 0x00ff0000);		\
	(image)[ptr] = (RB) >> 8;			\
	(zbuf)[ptr] = (v2)[2];				\
}
