/* Copyright 1999-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * These routines free memory
 *
 * Note check for null pointer before proceding to next lower level
 */

#include "AIR.h"

void AIR_free_vol9(AIR_Pixels *********pixels)
{
	if(*pixels) AIR_free_vol8(*pixels);
	free(pixels);
}
void AIR_free_vol8(AIR_Pixels ********pixels)
{
	if(*pixels) AIR_free_vol7(*pixels);
	free(pixels);
}
void AIR_free_vol7(AIR_Pixels *******pixels)
{
	if(*pixels) AIR_free_vol6(*pixels);
	free(pixels);
}
void AIR_free_vol6(AIR_Pixels ******pixels)
{
	if(*pixels) AIR_free_vol5(*pixels);
	free(pixels);
}
void AIR_free_vol5(AIR_Pixels *****pixels)
{
	if(*pixels) AIR_free_vol4(*pixels);
	free(pixels);
}
void AIR_free_vol4(AIR_Pixels ****pixels)
{
	if(*pixels) AIR_free_vol3(*pixels);
	free(pixels);
}
void AIR_free_vol3(AIR_Pixels ***pixels)
{
	if(*pixels) AIR_free_vol2(*pixels);
	free(pixels);
}
void AIR_free_vol2(AIR_Pixels **pixels)
{
	if(*pixels) AIR_free_vol1(*pixels);
	free(pixels);
}
void AIR_free_vol1(AIR_Pixels *pixels)
{
	free(pixels);
}
