/* Copyright 1999-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * These routines free memory
 *
 * Note check for null pointer before proceding to next lower level
 */

#include "AIR.h"

void AIR_free_7f(float *******pixels)
{
	if(*pixels) AIR_free_6f(*pixels);
	free(pixels);
}
void AIR_free_6f(float ******pixels)
{
	if(*pixels) AIR_free_5f(*pixels);
	free(pixels);
}
void AIR_free_5f(float *****pixels)
{
	if(*pixels) AIR_free_4f(*pixels);
	free(pixels);
}
void AIR_free_4f(float ****pixels)
{
	if(*pixels) AIR_free_3f(*pixels);
	free(pixels);
}
void AIR_free_3f(float ***pixels)
{
	if(*pixels) AIR_free_2f(*pixels);
	free(pixels);
}
void AIR_free_2f(float **pixels)
{
	if(*pixels) AIR_free_1f(*pixels);
	free(pixels);
}
void AIR_free_1f(float *pixels)
{
	free(pixels);
}
