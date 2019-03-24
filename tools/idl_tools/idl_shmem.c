#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "export.h"
#include "idl_tools.h"

/* function protos */
extern IDL_VPTR IDL_CDECL shmem_alloc(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL shmem_allocvar(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL shmem_free(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL shmem_attach(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL shmem_attachvar(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL shmem_num_attached(int argc, IDL_VPTR argv[], char *argk);

/* define the SHMEM functions */
static IDL_SYSFUN_DEF snd_functions[] = {

    { shmem_alloc,		"SHMEM_ALLOC",		1, 1, 0},
    { shmem_allocvar,		"SHMEM_ALLOCVAR",	2, 2, 0},
    { shmem_free,		"SHMEM_FREE",		1, 1, 0},
    { shmem_attach,		"SHMEM_ATTACH",		3, 3, 0},
    { shmem_attachvar,		"SHMEM_ATTACHVAR",	1, 1, 0},
    { shmem_num_attached,	"SHMEM_NUM_ATTACHED",	1, 1, 0},
};

/* startup call when DLM is loaded */
int shmem_startup(void)
{
	return(IDL_TRUE);
}

/* called when IDL is shutdown */
void shmem_exit_handler(void)
{
	return;
}

#ifdef WIN32

/*
 *  key = SHMEM_ALLOCVAR(type,dims)
 */
IDL_VPTR IDL_CDECL shmem_allocvar(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_LONG	typ;
	i_var		var;
	IDL_VPTR        vpTmp;
	IDL_LONG	i;

	typ = IDL_LongScalar(argv[0]);
	IDL_ENSURE_ARRAY(argv[1]);
	vpTmp = IDL_CvtLng(1, &(argv[1]));

	if ((typ <= IDL_TYP_UNDEF) || (typ > IDL_MAX_TYPE)) {
		return(IDL_GettmpLong(-1));
	}
	if ((typ == IDL_TYP_PTR) || (typ == IDL_TYP_OBJREF) || 
	    (typ == IDL_TYP_STRUCT)) return(IDL_GettmpLong(-1));

	var.token = TOKEN;
	var.type = typ;
	var.ndims = IDL_MIN(IDL_MAX_ARRAY_DIM,vpTmp->value.arr->n_elts);
	var.nelts = vpTmp->value.arr->dim[0];
	for(i=1;i<var.ndims;i++) var.nelts *= vpTmp->value.arr->dim[i];
	var.len = var.nelts*IDL_TypeSizeFunc(typ);

	if (vpTmp != argv[1]) IDL_Deltmp(vpTmp);

	return(IDL_GettmpLong(-1));
}

/*
 *  key = SHMEM_ALLOC(length)
 */
IDL_VPTR IDL_CDECL shmem_alloc(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_LONG	num;

	num = IDL_LongScalar(argv[0]);

	return(IDL_GettmpLong(-1));
}
/*
 * error = SHMEM_FREE(key)
 */
IDL_VPTR IDL_CDECL shmem_free(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}
/*
 *  var = SHMEM_ATTACHVAR(key)
 */
IDL_VPTR IDL_CDECL shmem_attachvar(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}
/*
 *  var = SHMEM_ATTACH(key,type,dims)
 */
IDL_VPTR IDL_CDECL shmem_attach(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}
/*
 *  num = SHMEM_NUM_ATTACHED(key)
 */
IDL_VPTR IDL_CDECL shmem_num_attached(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

#else

IDL_VPTR IDL_CDECL shmem_allocvar(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}
IDL_VPTR IDL_CDECL shmem_alloc(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

IDL_VPTR IDL_CDECL shmem_free(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

IDL_VPTR IDL_CDECL shmem_attachvar(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}
IDL_VPTR IDL_CDECL shmem_attach(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

IDL_VPTR IDL_CDECL shmem_num_attached(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

#endif
