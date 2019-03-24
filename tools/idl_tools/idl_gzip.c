#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "export.h"
#include "idl_tools.h"
#include "zlib.h"

/* function protos */
extern IDL_VPTR IDL_CDECL gzip_zip(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL gzip_unzip(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL gzip_packvar(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL gzip_unpackvar(int argc, IDL_VPTR argv[], char *argk);

#define BLOCK_SIZE	65536
#define DUMMY_SIZE	4096

/* some external stuff from idl_gzio.c */
typedef voidp mzFile;
mzFile mzopen(char *buf,int buflen,const char *mode);
int mzread(mzFile file,voidp buf,unsigned len);
int mzwrite(mzFile file,voidp buf,unsigned len);
int mzclose(mzFile file);

/* define the GZIP functions */
static IDL_SYSFUN_DEF gzip_functions[] = {

    { gzip_zip,			"GZIP",			2, 2, 0},
    { gzip_unzip,		"GUNZIP",		2, 2, IDL_SYSFUN_DEF_F_KEYWORDS},
    { gzip_packvar,		"PACKVAR",		2, 2, 0},
    { gzip_unpackvar,	"UNPACKVAR",	2, 2, 0},

};

/*
 *	err = GZIP('file','file')
 *  err = GZIP('file',var)
 *	err = GZIP( var, 'file')
 *	err = GZIP( var, var)
 *	err = GUNZIP('file','file' [,LENGTH=l][,OFFSET=o])
 *	err = GUNZIP('file', var [,LENGTH=l][,OFFSET=o])
 *	err = GUNZIP(var, var [,LENGTH=l][,OFFSET=o])
 *	err = GUNZIP(var, 'file' [,LENGTH=l][,OFFSET=o])
 *	
 *	err = PACKVAR( var, var)
 *	err = UNPACKVAR( var, var)
 */

/* startup call when DLM is loaded */
int gzip_startup(void)
{
	if (!IDL_AddSystemRoutine(gzip_functions, TRUE, 
		ARRLEN(gzip_functions))) {
		return IDL_FALSE;
	}
	IDL_ExitRegister(gzip_exit_handler);

	return(IDL_TRUE);
}

/* called when IDL is shutdown */
void gzip_exit_handler(void)
{
}

IDL_VPTR IDL_CDECL gzip_zip(int argc, IDL_VPTR argv[], char *argk)
{
	FILE		*in = NULL;
	gzFile		*out = NULL;
	mzFile      *mout = NULL;
	char		*buffer,*mbuf;
	IDL_VPTR	vpTmp = NULL, vpOut = NULL;
	IDL_LONG	iNum;
	IDL_LONG	iCount;

	IDL_EXCLUDE_UNDEF(argv[0]);

	if (argv[0]->type == IDL_TYP_STRING) {
		if (argv[1]->type == IDL_TYP_STRING) {
			/* file to file */
			IDL_ENSURE_SCALAR(argv[1]);
			IDL_ENSURE_SCALAR(argv[0]);
			buffer = IDL_GetScratch(&vpTmp, 1, BLOCK_SIZE);
			in = fopen(IDL_STRING_STR(&(argv[0]->value.str)),"rb");
			if (!in) goto err_out;
			out = gzopen(IDL_STRING_STR(&(argv[1]->value.str)),"wb");
			if (!out) goto err_out;
			iCount = BLOCK_SIZE;
			iNum = 0;
			while(iCount == BLOCK_SIZE) {
				iCount = fread(buffer,1,BLOCK_SIZE,in);
				if (iCount > 0) {
					gzwrite(out,buffer,iCount);
					iNum += iCount;
				}
			}
			fclose(in);
			gzclose(out);
			IDL_Deltmp(vpTmp);
		} else {
			/* file to var */
			IDL_ENSURE_SCALAR(argv[0]);
			IDL_EXCLUDE_EXPR(argv[1]);
			in = fopen(IDL_STRING_STR(&(argv[0]->value.str)),"rb");
			if (!in) goto err_out;
		    fseek(in,0,SEEK_END);
			iCount = ftell(in);
			fseek(in,0,SEEK_SET);
			iNum = (iCount*1.1)+18+12;
			buffer = IDL_GetScratch(&vpTmp, 1, BLOCK_SIZE+iNum);
			mbuf = buffer + BLOCK_SIZE;
			mout = mzopen(mbuf,iNum,"wb");
			if (!mout) goto err_out;

			iCount = BLOCK_SIZE;
			while(iCount == BLOCK_SIZE) {
				iCount = fread(buffer,1,BLOCK_SIZE,in);
				if (iCount > 0) mzwrite(mout,buffer,iCount);
			}
			fclose(in);
			iNum = mzclose(mout);
			memcpy(IDL_MakeTempVector(IDL_TYP_BYTE,iNum,IDL_ARR_INI_NOP,
				&vpOut),mbuf,iNum);
			IDL_VarCopy(vpOut,argv[1]);			
			IDL_Deltmp(vpTmp);
		}
	} else {
		if (argv[1]->type == IDL_TYP_STRING) {
			/* var to file */
			IDL_ENSURE_SCALAR(argv[1]);
			IDL_VarGetData(argv[0], &iNum, &buffer, 1);
			iNum = iNum * IDL_TypeSizeFunc(argv[0]->type);
			out = gzopen(IDL_STRING_STR(&(argv[1]->value.str)),"wb");
			if (!out) goto err_out;
			gzwrite(out,buffer,iNum);
			gzclose(out);
		} else {
			/* var to var */
			IDL_EXCLUDE_EXPR(argv[1]);
			IDL_VarGetData(argv[0], &iNum, &buffer, 1);
			iNum = iNum * IDL_TypeSizeFunc(argv[0]->type);
			iCount = (iNum*1.1)+18+12;
			mbuf = IDL_GetScratch(&vpTmp, 1, iCount);
			mout = mzopen(mbuf,iCount,"wb");
			if (!mout) goto err_out;
			mzwrite(mout,buffer,iNum);
			iNum = mzclose(mout);
			memcpy(IDL_MakeTempVector(IDL_TYP_BYTE,iNum,IDL_ARR_INI_NOP,
				&vpOut),mbuf,iNum);
			IDL_VarCopy(vpOut,argv[1]);			
			IDL_Deltmp(vpTmp);
		}
	}

	return(IDL_GettmpLong(iNum));

err_out:
	if (in) fclose(in);
	if (out) gzclose(out);
	if (mout) mzclose(mout);
	if (vpTmp) IDL_Deltmp(vpTmp);

	return(IDL_GettmpLong(-1));
}

IDL_VPTR IDL_CDECL gzip_unzip(int argc, IDL_VPTR inargv[], char *argk)
{
	gzFile		*in = NULL;
	mzFile      *min = NULL;
	FILE		*out = NULL;
	char		*buffer;
	IDL_VPTR	argv[2],vpTmp = NULL;
	IDL_LONG	iNum;
	IDL_LONG	iCount;

static	IDL_LONG	iOffset;
static	IDL_LONG	iLength;
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
	{"LENGTH",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iLength)},
	{"OFFSET",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iOffset)},
        {NULL}
        };

	IDL_KWCleanup(IDL_KW_MARK);
	IDL_KWGetParams(argc,inargv,argk,kw_pars,argv,1);

	if (argv[1]->type == IDL_TYP_STRING) {
		if (argv[0]->type == IDL_TYP_STRING) {
			/* file to file */
			IDL_ENSURE_SCALAR(argv[0]);
			buffer = IDL_GetScratch(&vpTmp, 1, BLOCK_SIZE);
			in = gzopen(IDL_STRING_STR(&(argv[0]->value.str)),"rb");
			if (!in) goto err_out;
			out = fopen(IDL_STRING_STR(&(argv[1]->value.str)),"wb");
			if (!out) goto err_out;
			if (iOffset > 0) {
				UCHAR	dummy[DUMMY_SIZE];
				while(iOffset > 0) {
					gzread(in,dummy,iOffset > DUMMY_SIZE ? DUMMY_SIZE : iOffset);
					iOffset -= DUMMY_SIZE;
				}
			}
			iCount = BLOCK_SIZE;
			iNum = 0;
			while(iCount == BLOCK_SIZE) {
				if (iLength > 0) {
					if (iNum + BLOCK_SIZE > iLength) {
						iCount = iLength - iNum;
					}
				}
				iCount = gzread(in,buffer,iCount);
				if (iCount > 0) {
					fwrite(buffer,1,iCount,out);
					iNum += iCount;
				}
			}
			gzclose(in);
			fclose(out);
			IDL_Deltmp(vpTmp);
		} else {
			/* var to file */
			IDL_ENSURE_SCALAR(argv[1]);
			IDL_VarGetData(argv[0], &iNum, &buffer, 1);
			iNum = iNum * IDL_TypeSizeFunc(argv[0]->type);
			min = mzopen(buffer,iNum,"rb");
			if (!min) goto err_out;
			out = fopen(IDL_STRING_STR(&(argv[1]->value.str)),"wb");
			if (!out) goto err_out;
			buffer = IDL_GetScratch(&vpTmp, 1, BLOCK_SIZE);
			if (iOffset > 0) {
				UCHAR	dummy[DUMMY_SIZE];
				while(iOffset > 0) {
					mzread(min,dummy,iOffset > DUMMY_SIZE ? DUMMY_SIZE : iOffset);
					iOffset -= DUMMY_SIZE;
				}
			}
			iCount = BLOCK_SIZE;
			iNum = 0;
			while(iCount == BLOCK_SIZE) {
				if (iLength > 0) {
					if (iNum + BLOCK_SIZE > iLength) {
						iCount = iLength - iNum;
					}
				}
				iCount = mzread(min,buffer,iCount);
				if (iCount > 0) {
					fwrite(buffer,1,iCount,out);
					iNum += iCount;
				}
			}
			mzclose(min);
			fclose(out);
			IDL_Deltmp(vpTmp);
		}
	} else {
		if (argv[0]->type == IDL_TYP_STRING) {
			/* file to var */
			UCHAR	filelen[4];
			IDL_ENSURE_SCALAR(argv[0]);
			IDL_EXCLUDE_EXPR(argv[1]);
			out = fopen(IDL_STRING_STR(&(argv[0]->value.str)),"rb");
			if (!out) goto err_out;
			fseek(out,-4,SEEK_END);
			fread(filelen,sizeof(filelen),1,out);
			fclose(out);

			iNum  = filelen[0];
			iNum += filelen[1]*256L;
			iNum += filelen[2]*256L*256L;
			iNum += filelen[3]*256L*256L*256L;

			if (iOffset > 0) iNum = iNum - iOffset;
			if (iNum < 0) goto err_out;
			if (iLength > 0) iNum = IDL_MIN(iNum,iLength);
			
			buffer = IDL_MakeTempVector(IDL_TYP_BYTE,iNum,
				IDL_ARR_INI_NOP,&vpTmp);
			IDL_VarCopy(vpTmp,argv[1]);
			in = gzopen(IDL_STRING_STR(&(argv[0]->value.str)),"rb");
			if (!in) goto err_out;
			if (iOffset > 0) {
				UCHAR	dummy[DUMMY_SIZE];
				while(iOffset > 0) {
					gzread(in,dummy,iOffset > DUMMY_SIZE ? DUMMY_SIZE : iOffset);
					iOffset -= DUMMY_SIZE;
				}
			}
			gzread(in,buffer,iNum);
			gzclose(in);
		} else {
			/* var to var */
			UCHAR	filelen[4];
			IDL_EXCLUDE_EXPR(argv[1]);
			IDL_VarGetData(argv[0], &iCount, &buffer, 1);
			iCount = iCount * IDL_TypeSizeFunc(argv[0]->type);
			memcpy(filelen,buffer+iCount-sizeof(filelen),sizeof(filelen));

			iNum  = filelen[0];
			iNum += filelen[1]*256L;
			iNum += filelen[2]*256L*256L;
			iNum += filelen[3]*256L*256L*256L;

			if (iOffset > 0) iNum = iNum - iOffset;
			if (iNum < 0) goto err_out;
			if (iLength > 0) iNum = IDL_MIN(iNum,iLength);

			min = mzopen(buffer,iCount,"rb");
			if (!min) goto err_out;

			buffer = IDL_MakeTempVector(IDL_TYP_BYTE,iNum,
				IDL_ARR_INI_NOP,&vpTmp);
			IDL_VarCopy(vpTmp,argv[1]);

			if (iOffset > 0) {
				UCHAR	dummy[DUMMY_SIZE];
				while(iOffset > 0) {
					mzread(min,dummy,iOffset > DUMMY_SIZE ? DUMMY_SIZE : iOffset);
					iOffset -= DUMMY_SIZE;
				}
			}
			mzread(min,buffer,iNum);
			mzclose(min);
		}
	}

	IDL_KWCleanup(IDL_KW_CLEAN);

	return(IDL_GettmpLong(iNum));

err_out:
	IDL_KWCleanup(IDL_KW_CLEAN);

	if (in) gzclose(in);
	if (min) mzclose(min);
	if (out) fclose(out);
	if (vpTmp) IDL_Deltmp(vpTmp);

	return(IDL_GettmpLong(-1));
}

IDL_VPTR IDL_CDECL gzip_packvar(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_VPTR	vpTmp,vpOut;
	i_var		var;
    char        *pbuffer,*dest;
	int			err;
	IDL_LONG	iCompLen,iOffset;

	IDL_ENSURE_SIMPLE(argv[0]);
	IDL_EXCLUDE_EXPR(argv[1]);
	vpTmp = argv[0];

	var.token = TOKEN;
	var.type = vpTmp->type;
	if ((var.type == IDL_TYP_STRUCT) ||
	    (var.type == IDL_TYP_PTR) ||
	    (var.type == IDL_TYP_OBJREF) ||
	    (var.type == IDL_TYP_UNDEF)) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADTYPE, 
			IDL_MSG_LONGJMP,IDL_TypeNameFunc(var.type));
	}
	if (vpTmp->type == IDL_TYP_STRING) {
		if (vpTmp->flags & IDL_V_ARR) return(IDL_GettmpLong(-1));
		pbuffer = IDL_STRING_STR(&(vpTmp->value.str));
		var.ndims = 0;
		var.len = vpTmp->value.str.slen+1;
		var.nelts = var.len;
	} else if (vpTmp->flags & IDL_V_ARR) {
		pbuffer = vpTmp->value.arr->data;
		var.ndims = vpTmp->value.arr->n_dim;
		var.len = vpTmp->value.arr->arr_len;
		var.nelts = vpTmp->value.arr->n_elts;
		memcpy(var.dims,vpTmp->value.arr->dim,
			IDL_MAX_ARRAY_DIM*sizeof(IDL_LONG));
	} else {
		pbuffer = &(vpTmp->value.c);
		var.ndims = 0;
		var.len = IDL_TypeSizeFunc(var.type);
		var.nelts = 1;
	}

	iCompLen = (IDL_LONG)(1.01*(double)var.len) + 12;
	dest = IDL_GetScratch(&vpTmp, 1, iCompLen);
	err = compress(dest,&iCompLen,pbuffer,var.len);
	if (err != Z_OK) {
		IDL_Deltmp(vpTmp);
		return(IDL_GettmpLong(-1));
	}

	iOffset = sizeof(i_var);

	pbuffer = IDL_MakeTempVector(IDL_TYP_BYTE,iCompLen+iOffset,
			IDL_ARR_INI_NOP,&vpOut);
	IDL_VarCopy(vpOut,argv[1]);
	memcpy(pbuffer,&var,sizeof(i_var));
	memcpy(pbuffer+iOffset,dest,iCompLen);

	IDL_Deltmp(vpTmp);

	return(IDL_GettmpLong(var.len));
}

IDL_VPTR IDL_CDECL gzip_unpackvar(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_LONG	swab=0,iOffset=0;
	i_var 		var;
	IDL_VPTR	vpTmp;
	char		*pbuffer;
	int			err;

	IDL_ENSURE_SIMPLE(argv[0]);
	IDL_ENSURE_ARRAY(argv[0]);
	IDL_EXCLUDE_EXPR(argv[1]);

	if (argv[0]->type != IDL_TYP_BYTE) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADTYPE, 
			IDL_MSG_LONGJMP,IDL_TypeNameFunc(argv[0]->type));
	}
		memcpy(&var,argv[0]->value.arr->data,sizeof(i_var));
		if (var.token == SWAPTOKEN) {
			byteswap(&var,sizeof(i_var),sizeof(IDL_LONG));
			swab = 1;
		}
		if (var.token != TOKEN) return(IDL_GettmpLong(-1));

	/* allocate the variable */
		if (var.type == IDL_TYP_STRING) {
			vpTmp = IDL_StrToSTRING("");
			IDL_StrEnsureLength(&(vpTmp->value.str),var.len);
			vpTmp->value.str.slen = var.len - 1;
			pbuffer = vpTmp->value.str.s;
			memset(pbuffer,0x20,var.len-1);
			pbuffer[var.len] = '\0';
			IDL_VarCopy(vpTmp,argv[1]);
		} else if (var.ndims != 0) {
			pbuffer = IDL_MakeTempArray(var.type, var.ndims,
				var.dims, IDL_BARR_INI_NOP, &vpTmp);
			IDL_VarCopy(vpTmp,argv[1]);
		} else {
			vpTmp = IDL_GettmpLong(0);
			IDL_VarCopy(vpTmp,argv[1]);
			IDL_StoreScalarZero(argv[1],var.type);
			pbuffer = &(argv[1]->value.c);
		}
		iOffset = sizeof(i_var);

	err = uncompress(pbuffer, &var.len, 
		argv[0]->value.arr->data+iOffset,
		argv[0]->value.arr->n_elts);
	if (err != Z_OK) return(IDL_GettmpLong(-1));

	if (swab) {
		int	swapsize = var.len/var.nelts;
		if ((var.type == IDL_TYP_COMPLEX) ||
		    (var.type == IDL_TYP_DCOMPLEX)) {
			swapsize /= 2;
		}
		byteswap(pbuffer,var.len,swapsize);
	}

	return(IDL_GettmpLong(var.len));
}
