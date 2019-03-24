/* Copyright 1999-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * These routines free memory
 *
 * Note check for null pointer before proceding to next lower level
 */

#include "AIR.h"

void AIR_free_7(double *******pixels)
{
	if(*pixels) AIR_free_6(*pixels);
	free(pixels);
}
void AIR_free_6(double ******pixels)
{
	if(*pixels) AIR_free_5(*pixels);
	free(pixels);
}
void AIR_free_5(double *****pixels)
{
	if(*pixels) AIR_free_4(*pixels);
	free(pixels);
}
void AIR_free_4(double ****pixels)
{
	if(*pixels) AIR_free_3(*pixels);
	free(pixels);
}
void AIR_free_3(double ***pixels)
{
	if(*pixels) AIR_free_2(*pixels);
	free(pixels);
}
void AIR_free_2(double **pixels)
{
	if(*pixels) AIR_free_1(*pixels);
	free(pixels);
}
void AIR_free_1(double *pixels)
{
	free(pixels);
}
