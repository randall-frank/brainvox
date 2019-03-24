/*
 * This file contains code covered by the following notice.
 * The actual interfaces have been changed for use by Brainvox
 * by RJF (2005) to allow Brainvox to read .Z files without
 * invoking an external program.
 */

/*
 * Copyright (c) 1985, 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * James A. Woods, derived from original work by Spencer Thomas
 * and Joseph Orost.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "z_compress.h"

/* 
 * Compress - data compression program 
 *
 * compress.c - File compression ala IEEE Computer, June 1984.
 *
 * Authors:	Spencer W. Thomas	(decvax!utah-cs!thomas)
 *		Jim McKie		(decvax!mcvax!jim)
 *		Steve Davies		(decvax!vax135!petsd!peora!srd)
 *		Ken Turkowski		(decvax!decwrl!turtlevax!ken)
 *		James A. Woods		(decvax!ihnp4!ames!jaw)
 *		Joe Orost		(decvax!vax135!petsd!joe)
 */

#define	min(a,b)	((a>b) ? b : a)

/*
 * Set USERMEM to the maximum amount of physical user memory available
 * in bytes.  USERMEM is used to determine the maximum BITS that can be used
 * for compression.
 *
 * SACREDMEM is the amount of physical memory saved for others; compress
 * will hog the rest.
 */
#ifndef SACREDMEM
#define SACREDMEM	0
#endif

#ifndef USERMEM
# define USERMEM 	450000	/* default user memory */
#endif

#ifdef USERMEM
# if USERMEM >= (433484+SACREDMEM)
#  define PBITS	16
# else
#  if USERMEM >= (229600+SACREDMEM)
#   define PBITS	15
#  else
#   if USERMEM >= (127536+SACREDMEM)
#    define PBITS	14
#   else
#    if USERMEM >= (73464+SACREDMEM)
#     define PBITS	13
#    else
#     define PBITS	12
#    endif
#   endif
#  endif
# endif
# undef USERMEM
#endif /* USERMEM */

#ifdef PBITS		/* Preferred BITS for this memory size */
# ifndef BITS
#  define BITS PBITS
# endif /* BITS */
#endif /* PBITS */

#if BITS == 16
# define HSIZE	69001		/* 95% occupancy */
#endif
#if BITS == 15
# define HSIZE	35023		/* 94% occupancy */
#endif
#if BITS == 14
# define HSIZE	18013		/* 91% occupancy */
#endif
#if BITS == 13
# define HSIZE	9001		/* 91% occupancy */
#endif
#if BITS <= 12
# define HSIZE	5003		/* 80% occupancy */
#endif

#ifdef M_XENIX			/* Stupid compiler can't handle arrays with */
# if BITS == 16			/* more than 65535 bytes - so we fake it */
#  define XENIX_16
# else
#  if BITS > 13			/* Code only handles BITS = 12, 13, or 16 */
#   define BITS	13
#  endif
# endif
#endif

/*
 * a code_int must be able to hold 2**BITS values of type int, and also -1
 */
#if BITS > 15
typedef long int	code_int;
#else
typedef int		code_int;
#endif
typedef long int	  count_int;

typedef	unsigned char	char_type;

char_type magic_header[] = { "\037\235" };	/* 1F 9D */

/* Defines for third byte of header */
#define BIT_MASK	0x1f
#define BLOCK_MASK	0x80
/* Masks 0x40 and 0x20 are free.  I think 0x20 should mean that there is
   a fourth header byte (for expansion).
*/
#define INIT_BITS 9			/* initial number of bits/code */

static int n_bits;			/* number of bits/code */
static int maxbits = BITS;		/* user settable max # bits/code */
static code_int maxcode;		/* maximum code, given n_bits */
static code_int maxmaxcode = 1 << BITS;	/* should NEVER generate this code */
static int block_compress = BLOCK_MASK;

#define MAXCODE(n_bits)	((1 << (n_bits)) - 1)

static count_int htab [HSIZE];
static unsigned short codetab [HSIZE];

#define htabof(i)	htab[i]
#define codetabof(i)	codetab[i]

static code_int hsize = HSIZE;			/* for dynamic table sizing */

/*
 * To save much memory, we overlay the table used by compress() with those
 * used by decompress().  The tab_prefix table is the same size and type
 * as the codetab.  The tab_suffix table needs 2**BITS characters.  We
 * get this from the beginning of htab.  The output stack uses the rest
 * of htab, and contains characters.  There is plenty of room for any
 * possible stack (stack used to be 8000 characters).
 */

#define tab_prefixof(i)	codetabof(i)
#define tab_suffixof(i)	((char_type *)(htab))[i]
#define de_stack	((char_type *)&tab_suffixof(1<<BITS))

/* forward prototypes */
static int output( code_int code, FILE *fp);
static int compress_int(FILE *fp,int len, unsigned char *b);
static int decompress_int(FILE *fp,int len, unsigned char *b);
static code_int getcode(FILE *fp);
static void cl_hash(count_int hsize);
static void cl_block(FILE *fp);

/*
 * the next two codes should not be changed lightly, as they must not
 * lie within the contiguous general code space.
 */ 
#define FIRST	257	/* first free entry */
#define	CLEAR	256	/* table clear output code */

#define CHECK_GAP 10000	/* ratio check interval */
static int clear_flg;
static int offset,in_count,checkpoint,bytes_out;
static long int ratio;
static int size;
static code_int free_ent;			/* first unused entry */

int read_compress_file(char *filename, int length, unsigned char *buffer)
{
    FILE *fp;
    int len = -1;

    fp = fopen(filename,"rb");
    if (!fp) return(len);

    len = decompress_int(fp,length,buffer);

    fclose(fp);

    return(len);
}

int write_compress_file(char *filename, int length, unsigned char *buffer)
{
    FILE *fp;
    int len = -1;

    fp = fopen(filename,"wb");
    if (!fp) return(len);

    len = compress_int(fp,length,buffer);

    fclose(fp);

    return(len);
}

int size_compress_file(char *filename)
{
    FILE *fp;
    int len = -1;

    fp = fopen(filename,"rb");
    if (!fp) return(len);

    len = decompress_int(fp,0,NULL);

    fclose(fp);

    return(len);
}

/*
 * compress buffer to file
 *
 * Algorithm:  use open addressing double hashing (no chaining) on the 
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */

static int compress_int(FILE *fp, int len, unsigned char *in_buf) 
{
    int pos = 0;

    long fcode;
    code_int i = 0;
    int c;
    code_int ent;
    int disp;
    code_int hsize_reg;
    int hshift;

    maxbits = BITS;		/* user settable max # bits/code */
    maxmaxcode = 1 << BITS;	/* should NEVER generate this code */
    block_compress = BLOCK_MASK;

    hsize = HSIZE;
    if ( len < (1 << 12) )
        hsize = min ( 5003, HSIZE );
    else if ( len < (1 << 13) )
        hsize = min ( 9001, HSIZE );
    else if ( len < (1 << 14) )
        hsize = min ( 18013, HSIZE );
    else if ( len < (1 << 15) )
        hsize = min ( 35023, HSIZE );
    else if ( len < 47000 )
        hsize = min ( 50021, HSIZE );

    fputc(magic_header[0],fp); fputc(magic_header[1],fp);
    fputc((char)(maxbits | block_compress),fp);
    if(ferror(fp)) return(-1);

    offset = 0;
    size = 0;
    bytes_out = 3;		/* includes 3-byte header mojo */
    clear_flg = 0;
    in_count = 1;
    ratio = 0;
    checkpoint = CHECK_GAP;
    maxcode = MAXCODE(n_bits = INIT_BITS);
    free_ent = ((block_compress) ? FIRST : 256 );

    ent = in_buf[pos++];

    hshift = 0;
    for ( fcode = (long) hsize;  fcode < 65536L; fcode *= 2L ) hshift++;
    hshift = 8 - hshift;		/* set hash code range bound */

    hsize_reg = hsize;
    cl_hash( (count_int) hsize_reg);		/* clear hash table */

    while ( pos < len ) {
        c = in_buf[pos++];

	in_count++;
	fcode = (long) (((long) c << maxbits) + ent);
 	i = ((c << hshift) ^ ent);	/* xor hashing */

	if ( htabof (i) == fcode ) {
	    ent = codetabof (i);
	    continue;
	} else if ( (long)htabof (i) < 0 )	/* empty slot */
	    goto nomatch;
 	disp = hsize_reg - i;		/* secondary hash (after G. Knott) */
	if ( i == 0 )
	    disp = 1;
probe:
	if ( (i -= disp) < 0 )
	    i += hsize_reg;

	if ( htabof (i) == fcode ) {
	    ent = codetabof (i);
	    continue;
	}
	if ( (long)htabof (i) > 0 ) 
	    goto probe;
nomatch:
	output ( (code_int) ent, fp );
 	ent = c;
	if ( free_ent < maxmaxcode ) {
 	    codetabof (i) = free_ent++;	/* code -> hashtable */
	    htabof (i) = fcode;
	}
	else if ( (count_int)in_count >= checkpoint && block_compress )
	    cl_block (fp);
    }
    /*
     * Put out the final code.
     */
    output( (code_int)ent, fp );
    output( (code_int)-1, fp );

    return(pos);
}

/*****************************************************************
 * TAG( output )
 *
 * Output the given code.
 * Inputs:
 * 	code:	A n_bits-bit integer.  If == -1, then EOF.  This assumes
 *		that n_bits =< (long)wordsize - 1.
 * Outputs:
 * 	Outputs code to the file.
 * Assumptions:
 *	Chars are 8 bits long.
 * Algorithm:
 * 	Maintain a BITS character long buffer (so that 8 codes will
 * fit in it exactly).  Use the VAX insv instruction to insert each
 * code in turn.  When the buffer fills up empty it and start over.
 */

static char buf[BITS];
static char_type lmask[9]={0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};
static char_type rmask[9]={0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

int output( code_int code, FILE *fp )
{
    int r_off = offset, bits= n_bits;
    char *bp = buf;

    if ( code >= 0 ) {
	/*
	 * Get to the first byte.
	 */
	bp += (r_off >> 3);
	r_off &= 7;
	/*
	 * Since code is always >= 8 bits, only need to mask the first
	 * hunk on the left.
	 */
	*bp = (*bp & rmask[r_off]) | (code << r_off) & lmask[r_off];
	bp++;
	bits -= (8 - r_off);
	code >>= 8 - r_off;
	/* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
	if ( bits >= 8 ) {
	    *bp++ = code;
	    code >>= 8;
	    bits -= 8;
	}
	/* Last bits. */
	if(bits) *bp = code;

	offset += n_bits;
	if ( offset == (n_bits << 3) ) {
	    bp = buf;
	    bits = n_bits;
	    bytes_out += bits;
	    do
		fputc(*bp++,fp);
	    while(--bits);
	    offset = 0;
	}

	/*
	 * If the next entry is going to be too big for the code size,
	 * then increase it, if possible.
	 */
	if ( free_ent > maxcode || (clear_flg > 0))
	{
	    /*
	     * Write the whole buffer, because the input side won't
	     * discover the size increase until after it has read it.
	     */
	    if ( offset > 0 ) {
		if( fwrite( buf, 1, n_bits, fp ) != n_bits) return(-1);
		bytes_out += n_bits;
	    }
	    offset = 0;

	    if ( clear_flg ) {
    	        maxcode = MAXCODE (n_bits = INIT_BITS);
	        clear_flg = 0;
	    }
	    else {
	    	n_bits++;
	    	if ( n_bits == maxbits )
		    maxcode = maxmaxcode;
	    	else
		    maxcode = MAXCODE(n_bits);
	    }
	}
    } else {
	/*
	 * At EOF, write the rest of the buffer.
	 */
	if ( offset > 0 ) fwrite( buf, 1, (offset + 7) / 8, fp );
	bytes_out += (offset + 7) / 8;
	offset = 0;
	fflush( fp );
	if( ferror( fp ) ) return(-1);
    }

    return(0);
}

/*
 * Decompress file to buffer.  This routine adapts to the codes in the
 * file building the "string" table on-the-fly; requiring no table to
 * be stored in the compressed file.  The tables used herein are shared
 * with those of the compress() routine.  See the definitions above.
 */

static int decompress_int(FILE *fp,int len, unsigned char *out_buf)
{
    int pos = 0;

    char_type *stackp;
    int finchar;
    code_int code, oldcode, incode;

    clear_flg = 0;
    size = 0;
    offset = 0;
    free_ent = 0;

    if ((fgetc(fp)!=(magic_header[0] & 0xFF))
	 || (fgetc(fp)!=(magic_header[1] & 0xFF))) return(-1);

    maxbits = fgetc(fp);
    block_compress = maxbits & BLOCK_MASK;
    maxbits &= BIT_MASK;
    maxmaxcode = 1 << maxbits;
    if(maxbits > BITS) return(-1);
    hsize = HSIZE;

    /*
     * As above, initialize the first 256 entries in the table.
     */
    maxcode = MAXCODE(n_bits = INIT_BITS);
    for ( code = 255; code >= 0; code-- ) {
	tab_prefixof(code) = 0;
	tab_suffixof(code) = (char_type)code;
    }
    free_ent = ((block_compress) ? FIRST : 256 );

    finchar = oldcode = getcode(fp);
    if(oldcode == -1) return(pos); /* EOF already? */

    if (out_buf) {
      out_buf[pos++] = (unsigned char)finchar;
      if (pos >= len) return(pos);
    } else pos++;

    stackp = de_stack;

    while ( (code = getcode(fp)) > -1 ) {

	if ( (code == CLEAR) && block_compress ) {
	    for ( code = 255; code >= 0; code-- )
		tab_prefixof(code) = 0;
	    clear_flg = 1;
	    free_ent = FIRST - 1;
	    if ( (code = getcode (fp)) == -1 )	/* O, untimely death! */
		break;
	}
	incode = code;
	/*
	 * Special case for KwKwK string.
	 */
	if ( code >= free_ent ) {
            *stackp++ = finchar;
	    code = oldcode;
	}

	/*
	 * Generate output characters in reverse order
	 */
	while ( code >= 256 ) {
	    *stackp++ = tab_suffixof(code);
	    code = tab_prefixof(code);
	}
	*stackp++ = finchar = tab_suffixof(code);

	/*
	 * And put them out in forward order
	 */
	do {
          if (out_buf) {
            out_buf[pos++] = *--stackp;
            if (pos >= len) return(pos);
          } else {
            --stackp;
            pos++;
          }
	} while ( stackp > de_stack );

	/*
	 * Generate the new entry.
	 */
	if ( (code=free_ent) < maxmaxcode ) {
	    tab_prefixof(code) = (unsigned short)oldcode;
	    tab_suffixof(code) = finchar;
	    free_ent = code+1;
	} 
	/*
	 * Remember previous code.
	 */
	oldcode = incode;
    }

    return(pos);
}

/*****************************************************************
 * TAG( getcode )
 *
 * Read one code from the standard input.  If EOF, return -1.
 * Inputs:
 * 	fp
 * Outputs:
 * 	code or -1 is returned.
 */

static char_type get_buf[BITS];

code_int getcode(FILE *fp) 
{
    code_int code;
    int r_off, bits;
    char_type *bp = get_buf;

    if ( clear_flg > 0 || offset >= size || free_ent > maxcode ) {
	/*
	 * If the next entry will be too big for the current code
	 * size, then we must increase the size.  This implies reading
	 * a new buffer full, too.
	 */
	if ( free_ent > maxcode ) {
	    n_bits++;
	    if ( n_bits == maxbits )
		maxcode = maxmaxcode;	/* won't get any bigger now */
	    else
		maxcode = MAXCODE(n_bits);
	}
	if ( clear_flg > 0) {
    	    maxcode = MAXCODE (n_bits = INIT_BITS);
	    clear_flg = 0;
	}
	size = fread( get_buf, 1, n_bits, fp );
	if ( size <= 0 ) return -1;	/* end of file */
	offset = 0;
	/* Round size down to integral number of codes */
	size = (size << 3) - (n_bits - 1);
    }
    r_off = offset;
    bits = n_bits;

	/*
	 * Get to the first byte.
	 */
	bp += (r_off >> 3);
	r_off &= 7;
	/* Get first part (low order bits) */
	code = (*bp++ >> r_off);
	bits -= (8 - r_off);
	r_off = 8 - r_off;		/* now, offset into code word */
	/* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
	if ( bits >= 8 ) {
	    code |= *bp++ << r_off;
	    r_off += 8;
	    bits -= 8;
	}
	/* high order bits. */
	code |= (*bp & rmask[bits]) << r_off;

    offset += n_bits;

    return code;
}

static void cl_block(FILE *fp)		/* table clear for block compress */
{
    long int rat;

    checkpoint = in_count + CHECK_GAP;

    if(in_count > 0x007fffff) {	/* shift will overflow */
	rat = bytes_out >> 8;
	if(rat == 0) {		/* Don't divide by zero */
	    rat = 0x7fffffff;
	} else {
	    rat = in_count / rat;
	}
    } else {
	rat = (in_count << 8) / bytes_out;	/* 8 fractional bits */
    }
    if ( rat > ratio ) {
	ratio = rat;
    } else {
	ratio = 0;
 	cl_hash ( (count_int) hsize );
	free_ent = FIRST;
	clear_flg = 1;
	output ( (code_int) CLEAR, fp );
    }
}

static void cl_hash(count_int hsize)		/* reset code table */
{
	count_int *htab_p = htab+hsize;
	long i;
	long m1 = -1;

	i = hsize - 16;
 	do {				/* might use Sys V memset(3) here */
		*(htab_p-16) = m1;
		*(htab_p-15) = m1;
		*(htab_p-14) = m1;
		*(htab_p-13) = m1;
		*(htab_p-12) = m1;
		*(htab_p-11) = m1;
		*(htab_p-10) = m1;
		*(htab_p-9) = m1;
		*(htab_p-8) = m1;
		*(htab_p-7) = m1;
		*(htab_p-6) = m1;
		*(htab_p-5) = m1;
		*(htab_p-4) = m1;
		*(htab_p-3) = m1;
		*(htab_p-2) = m1;
		*(htab_p-1) = m1;
		htab_p -= 16;
	} while ((i -= 16) >= 0);
    	for ( i += 16; i > 0; i-- )
		*--htab_p = m1;
}

#ifdef MAIN
int main(int argc,char **argv)
{
   int i;
   unsigned char buf[1024];

   for(i=0;i<sizeof(buf);i++) buf[i] = i & 0xff;
   
   printf("write=%d\n",write_compress_file("test.Z",sizeof(buf),buf));
   printf("size=%d\n",size_compress_file("test.Z"));
   printf("read=%d\n",read_compress_file("test.Z",sizeof(buf),buf));

   for(i=0;i<sizeof(buf);i++) if (buf[i] != (i & 0xff)) {
      printf("Error %d %d %d\n",i,buf[i],i & 0xff);
   }

   exit(0);
}
#endif
