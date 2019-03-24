/*
 * test-fftn.c: a small test of 2D real FFT
 *
 * Usage: test-fftn [xsize [ysize]]
 *
 * -----------------------------------------------------------------------
 * This is a simple program to demonstrate the usage of the Singleton
 * split-radix n-dimensional FFT routine. For N dimensions, the fft routine
 * must be called N times (with appropriate arguments each time) -- the
 * function fftn() does that.
 *
 * This routine demonstrates fwd and inverse transform of a small floating-
 * point array. You could use double-precision, and there are other options-
 * for instance using a combined real/imag data array instead of separate.
 * for a more extensive example, and more notes, see the test-all.c program.
 *
 *   - 7/31/95 jpb
 */

#include <stdio.h>
#include <stdlib.h>
#include "fftn.h"

#define PLIM	30	/* array size above which it's too large to print */

#undef REAL
#define REAL	float
#define fftn	fftnf

/* define some convenient macros for printing out values */

static REAL *Re_data = NULL, *Im_data = NULL;
#define Real_El(x,y)	Re_data [(y)*xsize + (x)]
#define Imag_El(x,y)	Im_data [(y)*xsize + (x)]

void
fill_array (int xsize, int ysize)
{
   int i, j, index;

   index = 0;
   for (j = 0; j < ysize; j++) {	/* fill array with a sequence of #s */
      for (i = 0; i < xsize; i++) {
	 Real_El (i,j) = index++;
	 Imag_El (i,j) = 0.0;
	 if (xsize < PLIM) printf ("%.1f ", Real_El (i,j));
      }
      if (xsize < PLIM) printf ("\n");
   }
}

void
print_array (const char msg[], int xsize, int ysize)
{
   int i, j;

   /* print out small arrays */
   if (xsize < PLIM) {
      if (msg && *msg) printf ("%s\n", msg);
      for (j = 0; j < ysize; j++) {
	 for (i = 0; i < xsize; i++)
	   printf ("%.1f,%.1f ", Real_El (i,j), Imag_El (i,j));
	 printf ("\n");
      }
   }
}

int
main (int argc, char **argv)
{
   int ret;
   int dims [2];		/* pass fft dimensions */
   int xsize, ysize;		/* matrix dimensions */

   xsize = 4;
   ysize = 5;

   if (argc > 1) xsize = ysize = atoi (argv[1]);	/* square */
   if (argc > 2) ysize = atoi (argv[2]);

   dims [0] = xsize;	/* x,y array dimensions to pass */
   dims [1] = ysize;

   Re_data = (REAL *) calloc (xsize * ysize, sizeof(REAL));
   Im_data = (REAL *) calloc (xsize * ysize, sizeof(REAL));
   if (Re_data == NULL || Im_data == NULL) {
      fprintf (stderr, "Unable to allocate memory for data storage.\n");
      return 1;
   }

   printf ("Example 2D fft: filling data array [%dx%d].\n", xsize, ysize);
   fill_array (xsize, ysize);

   printf ("Starting fft...");
   fflush (stdout);

   /* 2D forward fft */
/*
   ret = fftn (2, dims, Re_data, Im_data, 1, 0.0);
*/
   ret = fftn (2, dims, Re_data, Im_data, 1, (double) xsize * (double) ysize);
   if (ret) return 1;

   printf ("done.\n");	/* 2D forward FFT is now completed */
   print_array ("Transformed data:", xsize, ysize);

   /* -------------now do the inverse----------------- */

   printf ("Starting inverse fft...");
   fflush (stdout);
   /* 2D reverse fft */
/*
   ret = fftn (2, dims, Re_data, Im_data, -1,
	       (double) xsize * (double) ysize);
*/
   ret = fftn (2, dims, Re_data, Im_data, -1, 0.0);
   if (ret) return 1;

   /* ---------------------  Array should be un-transformed now ---- */
   print_array ("Re-transformed data:", xsize, ysize);

   printf ("done.\nFreeing dynamically allocated memory\n");
   fft_free ();

   free (Im_data);
   free (Re_data);
   return 0;
}
/* ---------------------- end-of-file (c source) ---------------------- */
