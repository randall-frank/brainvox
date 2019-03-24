#define COLORIN(v2, MSA, RBcolor, Gcolor, image, zbuf)	\
{							\
	register VLINT32 RB;					\
	register VLINT32 b2;					\
	register VLINT32 ptr;					\
	register VLINT32 im_val;				\
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
      register VLINT32 RB;                                   \
      register VLINT32 b2;                                   \
      register VLINT32 ptr;                                  \
      register VLINT32 im_val;                               \
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

#define OLD_ZCOLORMIP(v2, MSA, RBcolor, Gcolor, image, zbuf)       \
{                                                     \
      register VLINT32 RB;                                   \
      register VLINT32 b2;                                   \
      register VLINT32 ptr;                                  \
      register VLINT32 im_val;                               \
      ptr = ((v2)[0] + ((v2)[1]*imagex) );    \
      if ((zbuf)[ptr] < (v2)[2]) {                    \
      im_val = (image)[ptr];                          \
      b2 = im_val & 0x0000ff;                         \
      RB = RBcolor & 0x0000ff;       				  \
      if (RB > b2) {            	                  \
      (image)[ptr] = (RBcolor | Gcolor);              \
      (zbuf)[ptr] = (v2)[2];        };}               \
}

#define ZCOLORMIP(v2, Alpha, color, image, zbuf)       \
{                                                     \
      register VLINT32 b2;                                   \
      register VLINT32 ptr;                                  \
      register VLINT32 im_val;                               \
      ptr = ((v2)[0] + ((v2)[1]*imagex) );    \
      if ((zbuf)[ptr] < (v2)[2]) {                    \
      im_val = (image)[ptr];                          \
      b2 = (im_val & 0xff000000) >> 24;                         \
      if ((Alpha) > b2) {            	                  \
      (image)[ptr] = (color) | (Alpha << 24);              \
      (zbuf)[ptr] = (v2)[2];        };}               \
}

#define ZCOLORMNIP(v2, MSA, RBcolor, Gcolor, image, zbuf)       \
{                                                     \
      register VLINT32 RB;                                   \
      register VLINT32 b2;                                   \
      register VLINT32 ptr;                                  \
      register VLINT32 im_val;                               \
      ptr = ((v2)[0] + ((v2)[1]*imagex) );    \
      if ((zbuf)[ptr] < (v2)[2]) {                    \
      im_val = (image)[ptr];                          \
      b2 = im_val & 0x0000ff;                         \
      RB = RBcolor & 0x0000ff;       				  \
      if (RB < b2) {            	                  \
      (image)[ptr] = (RBcolor | Gcolor);              \
      (zbuf)[ptr] = (v2)[2];        };}               \
}


#define ZCOLORMIP24(v2, R, G, B, image, zbuf)       \
{                                                     \
      register VLINT32 b2;                                   \
      register VLINT32 ptr;                                  \
      register VLINT32 accum;			             \
      ptr = ((v2)[0] + ((v2)[1]*imagex) );    			\
      if ((zbuf)[ptr] < (v2)[2]) {                    		\
      	b2 = (image)[ptr] & 0x0000ff;                           \
     	( R > b2 ) ? ( accum = R ): ( accum = b2 );             \
		b2 = (image)[ptr] & 0x00ff00L;			\
		( G > b2 ) ? ( accum |= G) : ( accum |= b2 );   \
		b2 = (image)[ptr] & 0xff0000L;			\
		( B > b2 ) ? ( accum |= B ): ( accum |= b2 ); 	\
      	(image)[ptr] = accum;              			\
      	(zbuf)[ptr] = (v2)[2];        				\
      }               						\
}

#define ZCOLORYIQ(v2, R, G, B, image, zbuf)       \
{                                                     \
      register VLINT32 b2;                             \
      register VLINT32 ptr;                            \
      register VLINT32 accum;			       \
      ptr = ((v2)[0] + ((v2)[1]*imagex) );    			\
      if ((zbuf)[ptr] < (v2)[2]) {                    		\
      	b2 = (image)[ptr] & 0x0000ff;                           \
     	( R > b2 ) ? ( accum = R ): ( accum = b2 );             \
		b2 = (image)[ptr] & 0x00ff00L;			\
		( G > b2 ) ? ( accum |= G) : ( accum |= b2 );   \
		b2 = (image)[ptr] & 0xff0000L;			\
		( B > b2 ) ? ( accum |= B ): ( accum |= b2 ); 	\
      	(image)[ptr] = accum;              			\

#define ZCOLORIN24(v2, R, G, B, RO, GO, BO, image, zbuf)     \
{                                                     \
      register VLINT32 RB;                                   \
      register VLINT32 b2;                                   \
      register VLINT32 ptr;                                  \
      register VLINT32 im_val;                               \
      ptr = ((v2)[0] + ((v2)[1]*imagex) );    \
      if ((zbuf)[ptr] < (v2)[2]) {                    \
      im_val = (image)[ptr];                          \
      b2 = im_val & 0x000000ff;                       \
      b2 = ((R)*(RO)) + (b2*(255-(RO))); 	      \
      RB = (b2 & 0x0000ff00);                         \
      b2 = (im_val & 0x0000ff00) >> 8;                \
      b2 = ((G)*(GO)) + (b2*(255-(GO))); 	      \
      RB |= ((b2 & 0x0000ff00) << 8);                 \
      b2 = (im_val & 0x00ff0000) >> 16;               \
      b2 = ((B)*(BO)) + (b2*(255-(BO))); 	      \
      RB |= ((b2 & 0x0000ff00) << 16);                \
      (image)[ptr] = (RB) >> 8;                       \
      (zbuf)[ptr] = (v2)[2];          }               \
}


/* faster versions???  not fast enough for now */

#define FCOLORIN(v2, MSA, RBcolor, Gcolor, addtab, image, iy2)	\
{							\
	VLINT32 RB,G;					\
	VLINT32 b2;					\
	VLINT32 ptr;					\
	VLINT32 im_val;				\
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
	register VLINT32 RB;					\
	register VLINT32 b2;					\
	register VLINT32 ptr;					\
	register VLINT32 im_val;				\
	ptr = ((v2)[0] + (addtab)[((v2)[1] + (iy2))] );	\
	im_val = (image)[ptr];				\
	b2 = im_val & 0xff00ff;				\
	RB = ((RBcolor) + b2*(MSA)) & 0xff00ff00;	\
	b2 = im_val & 0x00ff00;				\
	RB = RB | (((Gcolor) + b2*(MSA)) & 0x00ff0000);		\
	(image)[ptr] = (RB) >> 8;			\
	(zbuf)[ptr] = (v2)[2];				\
}
