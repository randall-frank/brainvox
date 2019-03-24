#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "export.h"
#include "idl_tools.h"

/*
 * Define message codes and their corresponding printf(3) format
 * strings. Note that message codes start at zero and each one is
 * one less that the previous one. Codes must be monotonic and
 * contiguous.
 */
static IDL_MSG_DEF msg_arr[] =
{
  {  "M_TOOLS_ERROR",		"%NError: %s." },
  {  "M_TOOLS_BADTYPE",		"%NUnsupproted data type: %s." },
  {  "M_TOOLS_BADAUX",		"%NNumber of AUX vectors must match the number of vertices." },
  {  "M_TOOLS_BADWEIGHTS",	"%NNumber of weights must match the length of the AUX vector." },
  {  "M_TOOLS_BADSND",		"%NSound data must be a 1D or a [2,n] array." },
  {  "M_TOOLS_BADDIBFORMAT",	"%N%s bitmap formats not supported." },
  {  "M_TOOLS_BADDIMS",		"%NThe %s values are invalid." },
  {  "M_TOOLS_BADRANGE",	"%NThe block bounds fall outside the src or dst." },
  {  "M_TOOLS_BADKWDS",		"%NPASTE_ARRAY is required for the PASTE_ keywords." },
  {  "M_TOOLS_BADPASTE",	"%NArrays must have the same type and dimensionality for /PASTE." },
  {  "M_TOOLS_BADPROCNAME", "%NInternal error, function definition for %s could not be found." },
  {  "M_TOOLS_BADSIG",		"%NInvalid function signature." },
  {  "M_TOOLS_EXCLKEY",		"%NThe specified keywords are exclusive." },
};

/*
 * The load function fills in this message block handle with the
 * opaque handle to the message block used for this module. The other
 * routines can then use it to throw errors from this block.
 */
IDL_MSG_BLOCK msg_block;

int IDL_Load(void)
{

  if (!(msg_block = IDL_MessageDefineBlock("idl_tools", ARRLEN(msg_arr),
	   msg_arr))) {
	return IDL_FALSE;
  }

  if (!sock_startup()) {
	IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
		IDL_MSG_RET,"Unable to initialize socket libraries");
  }

  if (!sharr_startup()) {
	IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
		IDL_MSG_RET,"Unable to initialize shared memory libraries");
  }

  if (!snd_startup()) {
 	IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
		IDL_MSG_RET,"Unable to initialize sound libraries");
  }

  if (!gzip_startup()) {
 	IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
		IDL_MSG_RET,"Unable to initialize compression libraries");
  }

  if (!mesh_startup()) {
 	IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
		IDL_MSG_RET,"Unable to initialize decimation libraries");
  }

  if (!twain_startup()) {
 	IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
		IDL_MSG_RET,"Unable to initialize TWAIN libraries");
  }

  if (!extproc_startup()) {
 	IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
		IDL_MSG_RET,"Unable to initialize external procedure libraries");
  }

  if (!vfw_startup()) {
 	IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
		IDL_MSG_RET,"Unable to initialize VFW libraries");
  }

  return IDL_TRUE;
}
