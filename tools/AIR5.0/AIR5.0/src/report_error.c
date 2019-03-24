/* Copyright 1999-2001 Roger P. Woods, M.D. */
/* Modified 12/20/01 */

/*
 * void report_error()
 *
 * This routine prints out AIR errors
 *
 */

#include "AIR.h"

void AIR_report_error(const AIR_Error errornumber)

{
	char problem[255];
	char shortcode[255];

	AIR_fprob_errs(errornumber, problem, shortcode);
	printf("%s (%s)\n",problem,shortcode);

}
