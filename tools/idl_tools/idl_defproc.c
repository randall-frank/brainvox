#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "export.h"
#include "idl_tools.h"

#define MAX_ARGS	20

/* function protos */
extern IDL_VPTR IDL_CDECL extproc_define(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL extproc_query(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL extproc_deref(int argc, IDL_VPTR argv[], char *argk);

/*
	err = EXTPROC_DEFINE("idlprocname","name.dll","pname","signature"[,/CDECL])
	def = EXTPROC_QUERY("idlprocname")
	var = EXTPROC_DEREF(addr[,/STRING][,INT=][,LONG=][,FLOAT=][,DOUBLE=]
				[,UINT=][,ULONG=][,BYTE=])
*/

/* define the External Proc functions */
static IDL_SYSFUN_DEF extproc_functions[] = {

    { extproc_define,	"EXTPROC_DEFINE",		4, 4, IDL_SYSFUN_DEF_F_KEYWORDS},
    { extproc_deref,	"EXTPROC_DEREF",		1, 1, IDL_SYSFUN_DEF_F_KEYWORDS},
    { extproc_query,	"EXTPROC_QUERY",		1, 1, 0},
};

/* at present only under Windows and IDL 5.3 and higher */
#if defined(WIN32) && ((IDL_VERSION_MAJOR > 5) || ((IDL_VERSION_MAJOR == 5) && (IDL_VERSION_MINOR >= 3)))

#include <windows.h>
#include "dynacall.h"

typedef struct _procdef {
	struct _procdef	*next;
	char		name[256];
	DWORD		procptr;
	HINSTANCE	hInst;  /* handle to the DLL to close on exit*/
	long int	sig[MAX_ARGS];
	long int	n_args;
	long int	iCDECL;
} procdef;

/* a list of currently define procs */
static procdef	*proclist;

/* prototypes */
static IDL_VPTR IDL_CDECL execute_proc(int argc, IDL_VPTR argv[], char *argk);
static IDL_VPTR IDL_CDECL function_entry(int argc, IDL_VPTR argv[], char *argk);
static void IDL_CDECL procedure_entry(int argc, IDL_VPTR inargv[], char *argk);
static procdef *find_function(char *name);
static int parse_signature(char *sig,procdef *p);

/* find a given procname in the current list */
static procdef *find_function(char *name)
{
	procdef	*p = proclist;
	char	*t,tmp[256];

	strcpy(tmp,name);
	t = tmp;
	while(*t) {
		*t = toupper(*t);
		t++;
	}
	while(p) {
		if (strcmp(tmp,p->name) == 0) return(p);
		p = p->next;
	}
	return(NULL);
}

/* convert a signature from a string into numbers */
static int parse_signature(char *sig,procdef *p)
{
	char	*s = sig;

	p->n_args = 0;
	p->sig[0] = IDL_TYP_UNDEF;
	while(*s) {
		if ((p->n_args == 0) && (*s == 'v')) {
			p->sig[p->n_args] = IDL_TYP_UNDEF;
		} else {
			switch(*s) {
				case 'c':
					p->sig[p->n_args] = IDL_TYP_BYTE;
					break;
				case 'i':
					p->sig[p->n_args] = IDL_TYP_INT;
					break;
				case 'l':
					p->sig[p->n_args] = IDL_TYP_LONG;
					break;
				case 'I':
					p->sig[p->n_args] = IDL_TYP_UINT;
					break;
				case 'L':
					p->sig[p->n_args] = IDL_TYP_ULONG;
					break;
				case 'f':
					p->sig[p->n_args] = IDL_TYP_FLOAT;
					break;
				case 'd':
					p->sig[p->n_args] = IDL_TYP_DOUBLE;
					break;
				case 'p':
					p->sig[p->n_args] = IDL_TYP_PTR;
					break;
				case '(':
					if (p->n_args) return(0);
				case ',':
					p->n_args += 1;
					if (p->n_args >= MAX_ARGS) return(0);
					p->sig[p->n_args] = IDL_TYP_INT;
					break;
				case ')':
					p->n_args += 1;
					return(p->n_args);
					break;
				case ' ':
					break;
			}
		}
		s++;
	}

	return(0);
}

/* startup call when DLM is loaded */
int extproc_startup(void)
{
	proclist = NULL;

	if (!IDL_AddSystemRoutine(extproc_functions, TRUE, 
		ARRLEN(extproc_functions))) {
		return IDL_FALSE;
	}
 	IDL_ExitRegister(extproc_exit_handler);

	return(IDL_TRUE);
}

/* called when IDL is shutdown */
void extproc_exit_handler(void)
{
	procdef	*p = proclist;

	while(p) {
		procdef *q = p;
		p = p->next;
		FreeLibrary(q->hInst);
		IDL_MemFree(q, "", IDL_MSG_RET);
	}

	return;
}

/* standard interfaces for all routines */
static IDL_VPTR IDL_CDECL function_entry(int argc, IDL_VPTR argv[], char *argk)
{
	return(execute_proc(argc,argv,argk));
}
static void IDL_CDECL procedure_entry(int argc, IDL_VPTR argv[], char *argk)
{
	execute_proc(argc,argv,argk);
	return;
}

/* actual call proc */
static IDL_VPTR IDL_CDECL execute_proc(int argc, IDL_VPTR argv[], char *argk)
{
	procdef		*p;
	DYNAPARM	Parm[MAX_ARGS];
	RESULT		rc;
	IDL_LONG	i;
	IDL_VPTR	vpTmp = NULL;
	int			iFlags = DC_CALL_STD;
	double		dList[MAX_ARGS];

	/* find the function */
	p = find_function(IDL_SysRtnGetCurrentName());
	if (!p) {
		/* This should NEVER happen, it would be a MAJOR internal error!!! */
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADPROCNAME, 
			IDL_MSG_LONGJMP,IDL_SysRtnGetCurrentName());
	}

	/* clear the params */
	memset(Parm, 0, sizeof(DYNAPARM)*MAX_ARGS);

	/* set up the params */
/*
 * The rules:
 * 1) if a scalar, convert the IDL var to the type and pass it by value
 * 2) if pointer
 *    a) if IDL var is a scalar, pass the value of the scalar as an address
 *    b) pass the address of the IDL var data section
 */
	for(i=0;i<p->n_args-1;i++) {
		IDL_EXCLUDE_FILE(argv[i]);
		IDL_EXCLUDE_UNDEF(argv[i]);
		switch(p->sig[i+1]) {
			case IDL_TYP_BYTE:
				IDL_ENSURE_SCALAR(argv[i]);
				vpTmp = IDL_CvtByte(1,&(argv[i]));
				Parm[i].nWidth = sizeof(char);
				Parm[i].dwArg = vpTmp->value.c;
				if (vpTmp != argv[i]) IDL_Deltmp(vpTmp);
				break;
			case IDL_TYP_INT:
				IDL_ENSURE_SCALAR(argv[i]);
				vpTmp = IDL_CvtFix(1,&(argv[i]));
				Parm[i].nWidth = sizeof(short);
				Parm[i].dwArg = vpTmp->value.i;
				if (vpTmp != argv[i]) IDL_Deltmp(vpTmp);
				break;
			case IDL_TYP_UINT:
				IDL_ENSURE_SCALAR(argv[i]);
				vpTmp = IDL_CvtUInt(1,&(argv[i]));
				Parm[i].nWidth = sizeof(unsigned short);
				Parm[i].dwArg = vpTmp->value.ui;
				if (vpTmp != argv[i]) IDL_Deltmp(vpTmp);
				break;
			case IDL_TYP_LONG:
				Parm[i].nWidth = sizeof(IDL_LONG);
				Parm[i].dwArg = IDL_LongScalar(argv[i]);
				break;
			case IDL_TYP_ULONG:
				Parm[i].nWidth = sizeof(IDL_ULONG);
				Parm[i].dwArg = IDL_ULongScalar(argv[i]);
				break;
			case IDL_TYP_FLOAT:
				IDL_ENSURE_SCALAR(argv[i]);
				vpTmp = IDL_CvtFlt(1,&(argv[i]));
				Parm[i].nWidth = sizeof(float);
				memcpy(&(Parm[i].dwArg),&(vpTmp->value.f),Parm[i].nWidth);
				if (vpTmp != argv[i]) IDL_Deltmp(vpTmp);
				break;
			case IDL_TYP_DOUBLE:
				Parm[i].nWidth = sizeof(double);
				dList[i] = IDL_DoubleScalar(argv[i]);
				Parm[i].pArg = &(dList[i]);
				Parm[i].dwFlags = DC_FLAG_ARGPTR;
				break;
			case IDL_TYP_PTR:
				Parm[i].nWidth = sizeof(void *);
				if (argv[i]->type == IDL_TYP_STRING) {
					IDL_ENSURE_SCALAR(argv[i]);
					Parm[i].dwArg = (DWORD)IDL_STRING_STR(&(argv[i]->value.str));
				} else if (argv[i]->flags & IDL_V_ARR) {
					IDL_EXCLUDE_CONST(argv[i]);
					IDL_EXCLUDE_EXPR(argv[i]);
					Parm[i].dwArg = (DWORD)(argv[i]->value.arr->data);
				} else {
					Parm[i].dwArg = IDL_MEMINTScalar(argv[i]);
				}
				break;
		}
	}

	/* call the function */
	if (p->iCDECL) iFlags = DC_CALL_CDECL;
	if (p->sig[0] == IDL_TYP_DOUBLE) iFlags |= DC_RETVAL_MATH8;
	if (p->sig[0] == IDL_TYP_FLOAT) iFlags |= DC_RETVAL_MATH4;
	rc = DynaCall(iFlags, p->procptr, p->n_args-1, Parm, NULL, 0);

	/* handle the return value (if any) */
	if (p->sig[0] != IDL_TYP_UNDEF) {
		switch(p->sig[0]) {
			case IDL_TYP_BYTE:
			case IDL_TYP_INT:
				vpTmp = IDL_GettmpInt((short)rc.Int);
				break;
			case IDL_TYP_UINT:
				vpTmp = IDL_GettmpUInt((unsigned short)rc.Int);
				break;
			case IDL_TYP_LONG:
				vpTmp = IDL_GettmpLong(rc.Long);
				break;
			case IDL_TYP_ULONG:
				vpTmp = IDL_GettmpULong(rc.Long);
				break;
			case IDL_TYP_FLOAT:
				vpTmp = IDL_Gettmp();
				vpTmp->type = IDL_TYP_FLOAT;
				vpTmp->value.f = rc.Float;
				break;
			case IDL_TYP_DOUBLE:
				vpTmp = IDL_Gettmp();
				vpTmp->type = IDL_TYP_DOUBLE;
				vpTmp->value.d = rc.Double;
				break;
			case IDL_TYP_PTR:
				vpTmp = IDL_GettmpMEMINT((IDL_ULONG)rc.Pointer);
				break;
		}
	}

	return(vpTmp);
}


/*
	err = EXTPROC_DEFINE("idlname","name.dll","funcname","signature"[,/CDECL]) 
*/
IDL_VPTR IDL_CDECL extproc_define(int argc, IDL_VPTR inargv[], char *argk)
{
	IDL_VPTR        argv[4];
	char			*pname,*psig,*pdll,*pfname;
	IDL_LONG		ret = -1;
	procdef			*p;
	HINSTANCE		hInst;
	DWORD			lpFunction;
	IDL_SYSFUN_DEF	*idldef;
	int				isfunc;
	char			*t;

static	IDL_LONG	iCDECL;
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
		{"CDECL",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iCDECL)},
        {NULL}
        };

	IDL_KWCleanup(IDL_KW_MARK);
	IDL_KWGetParams(argc,inargv,argk,kw_pars,argv,1);

	IDL_ENSURE_SCALAR(argv[0]);
	IDL_ENSURE_SCALAR(argv[1]);
	IDL_ENSURE_SCALAR(argv[2]);
	IDL_ENSURE_SCALAR(argv[3]);
	pname = IDL_STRING_STR(&(argv[0]->value.str));
	pdll = IDL_STRING_STR(&(argv[1]->value.str));
	pfname = IDL_STRING_STR(&(argv[2]->value.str));
	psig = IDL_STRING_STR(&(argv[3]->value.str));

	/* prior def check */
	p = find_function(pname);
	if (p) {
		ret = 1;
		goto err;
	}

	/* resolve the routine */
	hInst = LoadLibrary(pdll);
	if (!hInst) {
		ret = -1;
		goto err;
	}
	lpFunction = SearchProcAddress(hInst, pfname);
	if (!lpFunction) {
		ret = -2;
		FreeLibrary(hInst);
		goto err;
	}
	
	/* get the procdef */
	p = (procdef *)IDL_MemAlloc(sizeof(procdef), "", IDL_MSG_LONGJMP);
	/* fill it in */
	p->procptr = lpFunction;
	p->hInst = hInst;
	strcpy(p->name,pname);
	t = p->name;
	while(*t) {
		*t = toupper(*t);
		t++;
	}
	if (parse_signature(psig,p) == 0) {
		FreeLibrary(p->hInst);
		IDL_MemFree(p, "", IDL_MSG_RET);
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADSIG, 
			IDL_MSG_LONGJMP);
	}
	p->iCDECL = iCDECL;

	/* tell IDL about it */
	idldef = (IDL_SYSFUN_DEF *)IDL_MemAllocPerm(sizeof(IDL_SYSFUN_DEF),
		"",IDL_MSG_LONGJMP);
	idldef->name = (char *)IDL_MemAllocPerm(strlen(pname)+1,
		"",IDL_MSG_LONGJMP);
	strcpy(idldef->name,p->name);
	idldef->arg_max = p->n_args-1;
	idldef->arg_min = p->n_args-1;
	if (p->sig[0] == IDL_TYP_UNDEF) {
		idldef->funct_addr = (IDL_FUN_RET)procedure_entry;
		isfunc = FALSE;
	} else {
		idldef->funct_addr = function_entry;
		isfunc = TRUE;
	}
	idldef->flags = 0;
	if (!IDL_AddSystemRoutine(idldef, isfunc, 1)) {
		FreeLibrary(p->hInst);
		IDL_MemFree(p, "", IDL_MSG_RET);
		FreeLibrary(hInst);
		ret = -3;
		goto err;
	}

	/* add it to the list */
	p->next = proclist;
	proclist = p;
	ret = 0;

err:
	IDL_KWCleanup(IDL_KW_CLEAN);

	return(IDL_GettmpLong(ret));
}

/*
	def = EXTPROC_QUERY("idlprocname")
*/
IDL_VPTR IDL_CDECL extproc_query(int argc, IDL_VPTR argv[], char *argk)
{
	procdef		*p;
	IDL_VPTR	vpTmp;
	char		*s;
	IDL_LONG	i;

	IDL_ENSURE_SCALAR(argv[0]);
	vpTmp = IDL_StrToSTRING("");
	p = find_function(IDL_STRING_STR(&(argv[0]->value.str)));
	if (p) {
		IDL_StrEnsureLength(&(vpTmp->value.str),p->n_args*3+1+6);
		s = vpTmp->value.str.s;
		s[0] = '\0';
		if (p->iCDECL) strcat(s,"cdecl ");
		for(i=0;i<p->n_args;i++) {
			switch(p->sig[i]) {
				case IDL_TYP_UNDEF:
					strcat(s,"v");
					break;
				case IDL_TYP_PTR:
					strcat(s,"p");
					break;
				case IDL_TYP_BYTE:
					strcat(s,"c");
					break;
				case IDL_TYP_INT:
					strcat(s,"i");
					break;
				case IDL_TYP_UINT:
					strcat(s,"I");
					break;
				case IDL_TYP_LONG:
					strcat(s,"l");
					break;
				case IDL_TYP_ULONG:
					strcat(s,"L");
					break;
				case IDL_TYP_FLOAT:
					strcat(s,"f");
					break;
				case IDL_TYP_DOUBLE:
					strcat(s,"d");
					break;
			}
			if (i == 0) {
				strcat(s,"(");
			} else if (i != p->n_args-1) {
				strcat(s,",");
			} else {
				strcat(s,")");
			}
		}
		vpTmp->value.str.slen = strlen(s);
	}

	return(vpTmp);
}

#else

/* startup call when DLM is loaded */
int extproc_startup(void)
{
	if (!IDL_AddSystemRoutine(extproc_functions, TRUE, 
		ARRLEN(extproc_functions))) {
		return IDL_FALSE;
	}
	IDL_ExitRegister(extproc_exit_handler);
	return(IDL_TRUE);
}

/* called when IDL is shutdown */
void extproc_exit_handler(void)
{
}

IDL_VPTR IDL_CDECL extproc_define(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

IDL_VPTR IDL_CDECL extproc_query(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

#endif

IDL_VPTR IDL_CDECL extproc_deref(int argc, IDL_VPTR inargv[], char *argk)
{
	IDL_VPTR	vpTmp;
	IDL_VPTR	argv[1];
	IDL_LONG	iCnt,iType,iLen;
	char		*pc;

static	IDL_LONG	iSTRING;
static	IDL_LONG	iINT;
static	IDL_LONG	iLONG;
static	IDL_LONG	iUINT;
static	IDL_LONG	iULONG;
static	IDL_LONG	iFLOAT;
static	IDL_LONG	iDOUBLE;
static	IDL_LONG	iBYTE;
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
		{"BYTE",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iBYTE)},
		{"DOUBLE",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iDOUBLE)},
		{"FLOAT",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iFLOAT)},
		{"INT",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iINT)},
		{"LONG",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iLONG)},
		{"STRING",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iSTRING)},
		{"UINT",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iUINT)},
		{"ULONG",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iULONG)},
        {NULL}
        };

	IDL_KWCleanup(IDL_KW_MARK);
	IDL_KWGetParams(argc,inargv,argk,kw_pars,argv,1);

	IDL_ENSURE_SCALAR(argv[0]);	
	pc = (char *)IDL_MEMINTScalar(argv[0]);

	iCnt = 0;
	if (iSTRING) {
		iType = IDL_TYP_STRING;
		iCnt += 1;
	}
	if (iINT) {
		iType = IDL_TYP_INT;
		iCnt += 1;
		iLen = iINT;
	}
	if (iLONG) {
		iType = IDL_TYP_LONG;
		iCnt += 1;
		iLen = iLONG;
	}
	if (iUINT) {
		iType = IDL_TYP_UINT;
		iCnt += 1;
		iLen = iUINT;
	}
	if (iULONG) {
		iType = IDL_TYP_ULONG;
		iCnt += 1;
		iLen = iULONG;
	}
	if (iFLOAT) {
		iType = IDL_TYP_FLOAT;
		iCnt += 1;
		iLen = iFLOAT;
	}
	if (iDOUBLE) {
		iType = IDL_TYP_DOUBLE;
		iCnt += 1;
		iLen = iDOUBLE;
	}
	if (iBYTE) {
		iType = IDL_TYP_BYTE;
		iCnt += 1;
		iLen = iBYTE;
	}
	if (!iCnt) {
		iType = IDL_TYP_BYTE;
		iLen = 1;
		iCnt = 1;
	}
	if (iCnt > 1) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_EXCLKEY, 
			IDL_MSG_LONGJMP);
	}

	if (iType == IDL_TYP_STRING) {
		vpTmp = IDL_StrToSTRING("");
		IDL_StrEnsureLength(&(vpTmp->value.str),strlen(pc));
		strcpy(vpTmp->value.str.s,pc);
		vpTmp->value.str.slen = strlen(vpTmp->value.str.s);
	} else {
		char	*pdst;
		pdst = IDL_MakeTempVector(iType,iLen,IDL_ARR_INI_NOP,&vpTmp);
		memcpy(pdst,pc,iLen*vpTmp->value.arr->elt_len);
	}

	IDL_KWCleanup(IDL_KW_CLEAN);

	return(vpTmp);
}

