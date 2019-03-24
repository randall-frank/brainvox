/*
 * test-all.c: test of 2D real FFT
 *
 * Usage: test-all [xsize [ysize [nruns] ] ]
 *
 * -----------------------------------------------------------------------
 * This test/example program does a 2-dimensional (two-variate) FFT and
 * then an inverse FFT on a matrix of varying size.
 *
 *   - 31 July 95 mjo
 */

#include <stdio.h>
#include <stdlib.h>
#include "fftn.h"

#define PLIM	30	/* array size above which it's too large to print */

#undef REAL
#ifdef TEST_FLOAT
# define REAL	float
# define fftn	fftnf
#else
# define REAL	double
#endif

/* define some convenient macros for printing out values */
#ifdef COMPLEX_STORAGE
static REAL *data = NULL;
/* element of a complex array stored [real0,imag0,real1,imag1, ...] */
# define Real_El(x,y)	data [2*((y)*xsize + (x))]
# define Imag_El(x,y)	data [2*((y)*xsize + (x)) + 1]
#else	/* COMPLEX_STORAGE */
static REAL *Re_data = NULL, *Im_data = NULL;
# define Real_El(x,y)	Re_data [(y)*xsize + (x)]
# define Imag_El(x,y)	Im_data [(y)*xsize + (x)]
#endif	/* COMPLEX_STORAGE */

void
fill_array (int xsize, int ysize)
{
   int i, j, index;

   index = 0;
   for (j = 0; j < ysize; j++) {   /* fill array with a sequence of #s */
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
   int xsize, ysize, runs;	/* matrix dimensions, number of matrices */

   xsize = ysize = runs = 4;

   if (argc > 1) xsize = ysize = atoi (argv[1]);	/* square */
   if (argc > 2) ysize = atoi (argv[2]);
   if (argc > 3) runs = atoi (argv[3]);

   while (runs-- > 0) {
      /* malloc first, to avoid broken realloc() on certain compilers (Sun) */
#ifdef COMPLEX_STORAGE
      if (data == NULL) {
	 data = (REAL *) malloc (2 * xsize * ysize * sizeof(REAL));
      } else {
	 data = (REAL *) realloc (data, 2 * xsize * ysize * sizeof(REAL));
      }

      if (data == NULL) {
	 fprintf (stderr, "Unable to allocate memory.\n");
	 return 1;
      }
#else	/* COMPLEX_STORAGE */
      if (Re_data == NULL) {
	 Re_data = (REAL *) malloc (xsize * ysize * sizeof(REAL));
	 Im_data = (REAL *) malloc (xsize * ysize * sizeof(REAL));
      } else {
	 Re_data = (REAL *) realloc (Re_data, xsize * ysize * sizeof(REAL));
	 Im_data = (REAL *) realloc (Im_data, xsize * ysize * sizeof(REAL));
      }

      if (Re_data == NULL || Im_data == NULL) {
	 fprintf (stderr, "Unable to allocate memory.\n");
	 return 1;
      }
#endif	/* COMPLEX_STORAGE */

      printf ("Example 2D fft: filling data array [%dx%d].\n", xsize, ysize);
      fill_array (xsize, ysize);

      printf ("Starting fft...");
      fflush (stdout);

      /* 2D forward fft */
      dims [0] = xsize;
      dims [1] = ysize;

      ret = fftn (2, dims,
#ifdef COMPLEX_STORAGE
		  &Real_El (0,0), &Imag_El (0,0),
		  2,		/* complex storage */
#else
		  Re_data, Im_data,
		  1,		/* separate real/imaginary arrays */
#endif
		  0.0);
      if (ret) return 1;

      printf ("done.\n");	/* 2D forward FFT is now completed */
      print_array ("Transformed data:", xsize, ysize);

      /* -------------now do the inverse----------------- */

      printf ("Starting inverse fft...");
      fflush (stdout);

      /* 2D reverse fft */
      ret = fftn (2, dims,
#ifdef COMPLEX_STORAGE
		  &Real_El (0,0), &Imag_El (0,0),
		  -2,	/* inverse, complex storage */
#else
		  Re_data, Im_data,
		  -1,	/* inverse, separate real/imaginary arrays */
#endif
		  (double) xsize * (double) ysize /* normalize */
		  );
      if (ret) return 1;

      printf ("done.\n");	/* 2D forward FFT is now completed */
      /* ---------------------  Array should be un-transformed now ---- */
      print_array ("Re-transformed data:", xsize, ysize);
      if (ret) return 1;

      /* next size */
      xsize++;
      ysize++;
   }

   /* done, free up dynamically allocated memory */
   printf ("Freeing dynamically allocated memory\n");
   fft_free ();

#ifdef COMPLEX_STORAGE
   free (data);
#else
   free (Im_data);
   free (Re_data);
#endif

   return 0;
}
/* ---------------------- end-of-file (c source) ---------------------- */
