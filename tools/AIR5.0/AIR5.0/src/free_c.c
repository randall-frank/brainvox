/* Copyright 1999-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * These routines free memory
 *
 * Note check for null pointer before proceding to next lower level
 */

#include "AIR.h"

void AIR_free_8c(char ********pixels)
{
	if(*pixels) AIR_free_7c(*pixels);
	free(pixels);
}
void AIR_free_7c(char *******pixels)
{
	if(*pixels) AIR_free_6c(*pixels);
	free(pixels);
}
void AIR_free_6c(char ******pixels)
{
	if(*pixels) AIR_free_5c(*pixels);
	free(pixels);
}
void AIR_free_5c(char *****pixels)
{
	if(*pixels) AIR_free_4c(*pixels);
	free(pixels);
}
void AIR_free_4c(char ****pixels)
{
	if(*pixels) AIR_free_3c(*pixels);
	free(pixels);
}
void AIR_free_3c(char ***pixels)
{
	if(*pixels) AIR_free_2c(*pixels);
	free(pixels);
}
void AIR_free_2c(char **pixels)
{
	if(*pixels) AIR_free_1c(*pixels);
	free(pixels);
}
void AIR_free_1c(char *pixels)
{
	free(pixels);
}
