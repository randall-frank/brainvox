#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "export.h"
#include "idl_tools.h"

/* function protos */
extern IDL_VPTR IDL_CDECL sharr_map(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sharr_test(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sharr_flush(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sharr_alloc_mem(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sharr_free_mem(int argc, IDL_VPTR argv[], char *argk);

/* define the SHMEM functions */
static IDL_SYSFUN_DEF sharr_functions[] = {
    { sharr_map,		"SHARR_MAP",	3, 3, IDL_SYSFUN_DEF_F_KEYWORDS},
    { sharr_test,		"SHARR_TEST",	1, 1, 0},
    { sharr_flush,		"SHARR_FLUSH",	1, 1, 0},
    { sharr_flush,		"SHARR_ALLOC_MEM",	3, 3, 0},
    { sharr_flush,		"SHARR_FREE_MEM",	1, 1, 0},
};

#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

typedef struct _arrmap {
	struct _arrmap	*next;
	void		*addr;
	IDL_LONG	iLen;
	IDL_ULONG64	iOffset;
	IDL_LONG	iRead;
#ifdef WIN32
	HANDLE		hFile;
	HANDLE		hMapping;
#else
	int		fd;
#endif
} arrmap;

static	arrmap	*arrlist = NULL;

static void free_addr(unsigned char *addr);
static arrmap *find_map(void *addr,arrmap **prev);

/* startup call when DLM is loaded */
int sharr_startup(void)
{
	if (!IDL_AddSystemRoutine(sharr_functions, TRUE, 
		ARRLEN(sharr_functions))) {
		return IDL_FALSE;
	}
	IDL_ExitRegister(sharr_exit_handler);

	return(IDL_TRUE);
}

/* called when IDL is shutdown */
void sharr_exit_handler(void)
{
	while(arrlist) free_addr(arrlist->addr);
	return;
}

/* free up the object corresponding to the addr */
static void free_addr(unsigned char *addr)
{
	arrmap *prev,*p = find_map(addr,&prev);
	if (p) {
#ifdef WIN32
		if (!(p->iRead)) FlushViewOfFile(p->addr,p->iLen);
		UnmapViewOfFile(p->addr);
		CloseHandle(p->hMapping);
		CloseHandle(p->hFile);
#else
		if (!(p->iRead)) msync(p->addr,p->iLen,MS_SYNC|MS_INVALIDATE);
		munmap(p->addr,p->iLen);
		close(p->fd);
#endif
		if (prev) {
			prev->next = p->next;
		} else {
			arrlist = p->next;
		}
		IDL_MemFree( p, "", IDL_MSG_RET);
	}
}

/* find the object corresponding to the addr */
static arrmap *find_map(void *addr,arrmap **prev)
{
	arrmap *p = arrlist;
	if (prev) *prev = NULL;
	while(p) {
		if (p->addr == addr) {
			return(p);
		}
		if (prev) *prev = p;
		p=p->next;
	}
	return(NULL);
}

/*
 *  name = SHARR_ALLOC_MEM(ID,type,dims)
 */
IDL_VPTR IDL_CDECL sharr_alloc_mem(int argc, IDL_VPTR inargv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

/*
 *  error = SHARR_FREE_MEM(name)
 */
IDL_VPTR IDL_CDECL sharr_free_mem(int argc, IDL_VPTR inargv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

 /*
 *  var = SHARR_MAP("name",type,dims[,OFFSET=off][,/CREATE][,/READONLY])
 */
IDL_VPTR IDL_CDECL sharr_map(int argc, IDL_VPTR inargv[], char *argk)
{
	IDL_LONG	pLegalTypes[] = { IDL_TYP_BYTE,IDL_TYP_INT,IDL_TYP_LONG,
		IDL_TYP_FLOAT,IDL_TYP_DOUBLE,IDL_TYP_COMPLEX,IDL_TYP_UINT,
		IDL_TYP_ULONG,IDL_TYP_LONG64,IDL_TYP_ULONG64, -1};
	IDL_VPTR	argv[3],vpTmp = NULL;
	IDL_LONG	iType;
	IDL_LONG	*pDims;
	int			nDims;
	IDL_LONG	iLen,i;
	IDL_VPTR	vpDim = NULL;
	char		*name;
	IDL_ULONG64	iTotal;
	arrmap		*p;

static	IDL_LONG		iCreate;
static	IDL_ULONG64		iOffset;
static	IDL_LONG		iRead;
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
	{"CREATE",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iCreate)},
	{"OFFSET",IDL_TYP_ULONG64,1,IDL_KW_ZERO,0,IDL_CHARA(iOffset)},
	{"READONLY",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iRead)},
    {NULL}
};

	/* parse keywords */
	IDL_KWCleanup(IDL_KW_MARK);
	IDL_KWGetParams(argc,inargv,argk,kw_pars,argv,1);

	/* ensure input types */
	IDL_ENSURE_STRING(argv[0]);
	IDL_ENSURE_ARRAY(argv[2]);

	/* check the type */
	iType = IDL_LongScalar(argv[1]);
	i = 0;
	while(1) {
		if (iType == pLegalTypes[i]) break;
		if (pLegalTypes[i] == -1) {
			if (iType < 0) iType = 0;
			if (iType > IDL_MAX_TYPE) iType = 0;
			IDL_MessageFromBlock(msg_block, M_TOOLS_BADTYPE, 
				IDL_MSG_LONGJMP,IDL_TypeNameFunc(iType));
		}
		i++;
	}

	/* get the dimensions */
	vpDim = IDL_CvtLng(1, &(argv[2]));
	pDims = (IDL_LONG *)vpDim->value.arr->data;
	nDims = vpDim->value.arr->n_elts;
	iLen = pDims[0];
	for(i=1;i<nDims;i++) iLen *= pDims[i];
	iLen *= IDL_TypeSizeFunc(iType);

	/* and the "name" */
	name = IDL_STRING_STR(&(argv[0]->value.str));

	/* allocate holder */
	p = (arrmap *)IDL_MemAlloc(sizeof(arrmap),"",IDL_MSG_RET);
	p->iLen = iLen;
	p->iOffset = iOffset;
	p->iRead = iRead;

#ifdef WIN32
	{
		DWORD iAccess = iRead ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);
		DWORD iStatus = iCreate ? OPEN_ALWAYS : OPEN_EXISTING;
		DWORD iProtect = iRead ? PAGE_READONLY : PAGE_READWRITE;
		DWORD iMapAccess = iRead ? FILE_MAP_READ : FILE_MAP_WRITE;

		/* TO DO: temporary files w/
			FILE_SHARE_DELETE
			FILE_ATTRIBUTE_NORMAL : FILE_FLAG_DELETE_ON_CLOSE

			Add pure shmem support: Named memory and Unix IDs
		*/

		p->hFile = NULL;
		p->hMapping = NULL;
		p->addr = NULL;
		iTotal = p->iOffset + p->iLen;

		p->hFile = CreateFile(name,iAccess,
			FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,iStatus,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if (!p->hFile) {
			IDL_MemFree( p, "", IDL_MSG_RET);
			IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
				IDL_MSG_LONGJMP,"Invalid CreateFile()");
		}
		p->hMapping = CreateFileMapping(p->hFile, NULL,iProtect,
			(iTotal >> 32) & 0xffffffff,iTotal & 0xffffffff,NULL);
		if (!p->hMapping) {
			CloseHandle(p->hFile);
			IDL_MemFree( p, "", IDL_MSG_RET);
			IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
				IDL_MSG_LONGJMP,"Invalid CreateFileMapping()");
		}
		p->addr = MapViewOfFile(p->hMapping,iMapAccess,
			(p->iOffset >> 32) & 0xffffffff,
			p->iOffset & 0xffffffff, p->iLen);
		if (!p->addr) {
			CloseHandle(p->hMapping);
			CloseHandle(p->hFile);
			IDL_MemFree( p, "", IDL_MSG_RET);
			IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
				IDL_MSG_LONGJMP,"Invalid MapViewOfFile()");
		}
	}
#else
	{
		struct stat	buf;
		int flags = iRead ? O_RDONLY : O_RDWR;
		if (iCreate) flags |= O_CREAT;

		p->fd = open(name,flags,0666);
		if (p->fd == -1) {
			IDL_MemFree( p, "", IDL_MSG_RET);
			IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
				IDL_MSG_LONGJMP,"Invalid open()");
		}
		stat(name,&buf);
		if (buf.st_size < p->iLen+p->iOffset) {
			if (truncate(name,p->iLen+p->iOffset) == -1) {
				close(p->fd);
				IDL_MemFree( p, "", IDL_MSG_RET);
				IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
					IDL_MSG_LONGJMP,"Invalid file size");
			}
		}
		flags = iRead ? MAP_PRIVATE : MAP_SHARED;
		p->addr = mmap(NULL,p->iLen,PROT_READ|PROT_WRITE,
			flags,p->fd,p->iOffset);
		if (p->addr == MAP_FAILED) {
			close(p->fd);
			IDL_MemFree( p, "", IDL_MSG_RET);
			IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
				IDL_MSG_LONGJMP,"Invalid mmap()");
		}
	}
#endif

	/* link it in... */
	p->next = arrlist;
	arrlist = p;

	/* tell IDL about it */
	vpTmp = IDL_ImportArray(nDims, pDims, iType, p->addr, 
		free_addr, NULL);

	/* cleanup */
	if (vpDim && (vpDim != argv[2])) IDL_Deltmp(vpDim);
	IDL_KWCleanup(IDL_KW_CLEAN);

	return(vpTmp);
}

/*
 *  status = SHARR_TEST(var)
 */
IDL_VPTR IDL_CDECL sharr_test(int argc, IDL_VPTR argv[], char *argk)
{
	arrmap	*p;

	IDL_ENSURE_ARRAY(argv[0]);

	p = find_map(argv[0]->value.arr->data,NULL);

	return(IDL_GettmpLong(p ? 1 : 0));
}
/*
 * error = SHARR_FLUSH(var)
 */
IDL_VPTR IDL_CDECL sharr_flush(int argc, IDL_VPTR argv[], char *argk)
{
	arrmap	*p;

	IDL_ENSURE_ARRAY(argv[0]);

	p = find_map(argv[0]->value.arr->data,NULL);
	if (!p) return(IDL_GettmpLong(-1));
	if (p->iRead) return(IDL_GettmpLong(1));

#ifdef WIN32
	FlushViewOfFile(p->addr,p->iLen);
#else
	msync(p->addr,p->iLen,MS_SYNC|MS_INVALIDATE);
#endif

	return(IDL_GettmpLong(0));
}
