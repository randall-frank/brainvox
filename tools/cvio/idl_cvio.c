/*
 * Copyright (c), 1990-2005 Randall Frank and Thomas Grabowski
 *
 * $Id $
 * $HeadURL $
 *
 *                University of Iowa, Department of Neurology,
 *                Human Neuroanatomy and Neuroimaging Lab
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by Randall Frank, Thomas
 *      Grabowski and other unpaid contributors.
 * 5. The names of Randall Frank or Thomas Grabowski may not be used to
 *    endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY RANDALL FRANK AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL RANDALL FRANK OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "export.h"
#include "cvio_lib.h"

/*
 * TODO: 
 */

#define ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))

/*
 * Define message codes and their corresponding printf(3) format
 * strings. Note that message codes start at zero and each one is
 * one less that the previous one. Codes must be monotonic and
 * contiguous.
 */

#define M_CVIO_ERROR 0
#define M_CVIO_BADDIMS -1
#define M_CVIO_SHORTARR -2

static IDL_MSG_DEF msg_arr[] =
{
  {  "M_CVIO_ERROR",	"%NError: %s." },
  {  "M_CVIO_BADDIMS",	"%NDimensions must match stream sample dimensions." },
  {  "M_CVIO_SHORTARR",	"%NInsufficient number of samples presented." },
};

/*
 * The load function fills in this message block handle with the
 * opaque handle to the message block used for this module. The other
 * routines can then use it to throw errors from this block.
 */
IDL_MSG_BLOCK msg_block;


IDL_LONG cviostr2idl(char *str);
IDL_LONG cvio2idl(int32 type);

void cvio_exit_handler(void)
{
	cvio_cleanup(0);
}

IDL_VPTR IDL_CDECL idl_cvio_get_error_msg(int argc, IDL_VPTR argv[], char *argk)
{
	int32 	err;
	int32	blen = 256;
	char	buff[256];
	IDL_VPTR	vpTmp;

	IDL_EXCLUDE_EXPR(argv[1]);

	err = cvio_get_error_message(IDL_LongScalar(argv[0]),buff,&blen);
	if (!err) {
		vpTmp = IDL_StrToSTRING(buff);
		IDL_VarCopy(vpTmp,argv[1]);
	}

	return(IDL_GettmpLong(err));
}
IDL_VPTR IDL_CDECL idl_cvio_listshmem(int argc, IDL_VPTR argv[], char *argk)
{
	int32 	err;
	int32	blen = 1024;
	int32	n,i;
	char	buff[1024],*p;
	IDL_VPTR	vpTmp;
	IDL_STRING	*s;

	IDL_EXCLUDE_EXPR(argv[0]);
	IDL_EXCLUDE_EXPR(argv[1]);

	err = cvio_listshmem(buff,&blen,&n);
	if (!err) {
		IDL_VarCopy(IDL_GettmpLong(n),argv[1]);
		if (n != 0) {
			s = (IDL_STRING *)IDL_MakeTempVector(IDL_TYP_STRING,
				n, IDL_BARR_INI_ZERO, &vpTmp);
			p = buff;
			for(i=0;i<n;i++) {
				IDL_StrStore(s,p);
				while(*p++);
				s++;
			}
			IDL_VarCopy(vpTmp,argv[0]);
		}
	}

	return(IDL_GettmpLong(err));
}

IDL_VPTR IDL_CDECL idl_cvio_create(int argc, IDL_VPTR argv[], char *argk)
{
	int32		err,i;
	int32		dims[3];
	IDL_VPTR	vpTmp;
	int32 		type;

	IDL_ENSURE_STRING(argv[0]);
	IDL_ENSURE_SCALAR(argv[0]);
	IDL_ENSURE_ARRAY(argv[4]);
	
	vpTmp = IDL_CvtLng(1,&(argv[4]));

	for(i=0;i<3;i++) {
		if (i < vpTmp->value.arr->n_elts) {
			dims[i] = ((IDL_LONG *)vpTmp->value.arr->data)[i];
		} else {
			dims[i] = 1;
		}
	}

	if (vpTmp != argv[4]) IDL_Deltmp(vpTmp);

	type = IDL_LongScalar(argv[2]);
	if (type == 0) type = CVIO_TYP_STRING;

	err = cvio_create(IDL_STRING_STR(&(argv[0]->value.str)),
			  IDL_LongScalar(argv[1]),
			  type,
			  IDL_LongScalar(argv[3]), 
			  dims,
			  IDL_LongScalar(argv[5]));

	return(IDL_GettmpLong(err));
}
IDL_VPTR IDL_CDECL idl_cvio_delete(int argc, IDL_VPTR argv[], char *argk)
{
	int32           err;

	IDL_ENSURE_STRING(argv[0]);
	IDL_ENSURE_SCALAR(argv[0]);
	err = cvio_delete(IDL_STRING_STR(&(argv[0]->value.str)));

	return(IDL_GettmpLong(err));
}
IDL_VPTR IDL_CDECL idl_open(int argc, IDL_VPTR argv[], char *argk)
{
	int32           err;
	uint32		n;

	IDL_ENSURE_STRING(argv[0]);
	IDL_ENSURE_SCALAR(argv[0]);
	IDL_EXCLUDE_EXPR(argv[2]);

	err = cvio_open(IDL_STRING_STR(&(argv[0]->value.str)),
			IDL_LongScalar(argv[1]),&n);
	if (!err) IDL_VarCopy(IDL_GettmpLong(n),argv[2]);

	return(IDL_GettmpLong(err));
}
IDL_VPTR IDL_CDECL idl_close(int argc, IDL_VPTR argv[], char *argk)
{
	int32           err;

	err = cvio_close(IDL_LongScalar(argv[0]));

	return(IDL_GettmpLong(err));
}
IDL_VPTR IDL_CDECL idl_has_writer(int argc, IDL_VPTR argv[], char *argk)
{
	int32           err;
	int32		n;

	IDL_EXCLUDE_EXPR(argv[1]);

	err = cvio_has_writer(IDL_LongScalar(argv[0]),&n);
	if (!err) IDL_VarCopy(IDL_GettmpLong(n),argv[1]);

	return(IDL_GettmpLong(err));
}
IDL_VPTR IDL_CDECL idl_max_length(int argc, IDL_VPTR argv[], char *argk)
{
	int32           err;
	int32		n;

	IDL_EXCLUDE_EXPR(argv[1]);

	err = cvio_max_length(IDL_LongScalar(argv[0]),&n);
	if (!err) IDL_VarCopy(IDL_GettmpLong(n),argv[1]);

	return(IDL_GettmpLong(err));
}

IDL_LONG cvio2idl(int32 type)
{
	switch(type) {
		case CVIO_TYP_STRING: return(IDL_TYP_STRING);
		case CVIO_TYP_BYTE: return(IDL_TYP_BYTE);
		case CVIO_TYP_USHORT: return(IDL_TYP_UINT);
		case CVIO_TYP_SHORT: return(IDL_TYP_INT);
		case CVIO_TYP_FLOAT: return(IDL_TYP_FLOAT);
	}
	return(IDL_TYP_LONG);
}

IDL_VPTR IDL_CDECL idl_add_samples(int argc, IDL_VPTR argv[], char *argk)
{
	uint32		stream;
	int32		n,err,i;
	CVIO_DT_TIME	*tp;
	void		*dp;
	IDL_VPTR	vpData,vpTime;
	int32		type,ndim,dims[3];
	IDL_MEMINT	n_elts,cnt;
	char		*pTime,*pData;

	n = IDL_LongScalar(argv[3]);
	stream = IDL_ULongScalar(argv[0]);

	/* get the stream data format */
	err = cvio_datatype(stream,&type,&ndim,dims);
	if (err) return(IDL_GettmpLong(err));

	/* how many data values in a sample */
	cnt = dims[0];
	for(i=1;i<ndim;i++) cnt *= dims[i];

	/* make sure time and/or data are arrays */
	if (n > 1) IDL_ENSURE_ARRAY(argv[1]);
	if (n*cnt > 1) IDL_ENSURE_ARRAY(argv[2]);

	vpData = IDL_BasicTypeConversion(1,&(argv[2]),cvio2idl(type));
	vpTime = IDL_CvtULng(1,&(argv[1]));

	/* time check */
	IDL_VarGetData(vpTime,&n_elts,&pTime,IDL_FALSE);
	if (n_elts < n) {
		IDL_MessageFromBlock(msg_block, M_CVIO_SHORTARR,
			IDL_MSG_LONGJMP);
	}

	/* data check */
	IDL_VarGetData(vpData,&n_elts,&pData,IDL_FALSE);
	if (type == CVIO_TYP_STRING) {
		/* MUST be a 1D array or a scalar */
		if (n_elts < n) {
			IDL_MessageFromBlock(msg_block, M_CVIO_SHORTARR,
				IDL_MSG_LONGJMP);
		}
		if (n > 1) {
			if (vpData->value.arr->n_dim != 1) {
				IDL_MessageFromBlock(msg_block, M_CVIO_BADDIMS,
					IDL_MSG_LONGJMP);
			}
		}
	} else {
		/* are there enough values? */
		if (n_elts < n*cnt) {
			IDL_MessageFromBlock(msg_block, M_CVIO_SHORTARR,
				IDL_MSG_LONGJMP);
		}
		/* if a sample is more than a scalar, we compare the
                 * dimensionality of the sample to the input array */
		if (cnt > 1) {
			/* must be at least ndim dimensions */
			if (vpData->value.arr->n_dim < ndim) {
				IDL_MessageFromBlock(msg_block, 
					M_CVIO_BADDIMS,IDL_MSG_LONGJMP);
			}
			/* and they must match */
			for(i=0;i<ndim;i++) {
			    	if (dims[i] != vpData->value.arr->dim[i]) {
					IDL_MessageFromBlock(msg_block, 
						M_CVIO_BADDIMS,IDL_MSG_LONGJMP);
			    	}
			}
		}
	}
	
	/* handle strings, one sample at a time */
	if (type == CVIO_TYP_STRING) {
		char		*tstr;
		IDL_STRING	*s;
		IDL_VPTR	vpTmp;

		tp = (CVIO_DT_TIME *)pTime;
		s = (IDL_STRING *)pData;

		tstr = (char *)IDL_GetScratch(&vpTmp,dims[0]+1,1);
		tstr[dims[0]] = '\0';

		for(i=0;i<n;i++) {
			strcpy(tstr,IDL_STRING_STR(s));
			s++;
			err = cvio_add_samples(stream,tp+i,tstr,1);
			if (err) break;
		}
		IDL_Deltmp(vpTmp);
	} else {
		tp = (CVIO_DT_TIME *)pTime;
		dp = (void *)pData;

		err = cvio_add_samples(stream,tp,dp,n);
	}

	if (vpTime != argv[1]) IDL_Deltmp(vpTime);
	if (vpData != argv[2]) IDL_Deltmp(vpData);

	return(IDL_GettmpLong(err));
}
IDL_VPTR IDL_CDECL idl_rd_samples(int argc, IDL_VPTR argv[], char *argk,
	int32 iNext, int32 iNumArg) 
{
	uint32		stream;
	int32		n,err,i;
	CVIO_DT_TIME	*tp;
	void		*dp;
	IDL_VPTR	vpData,vpTime;
	int32		type,ndim,dims[3];
	IDL_MEMINT	dd[8];

	IDL_ENSURE_ARRAY(argv[1]);
	IDL_EXCLUDE_EXPR(argv[1]);
	if (iNumArg != 2) IDL_EXCLUDE_EXPR(argv[2]);
	n = IDL_LongScalar(argv[iNumArg]);
	stream = IDL_ULongScalar(argv[0]);

	err = cvio_datatype(stream,&type,&ndim,dims);
	if (err) return(IDL_GettmpLong(err));

	if (argv[1]->value.arr->n_elts < n) {
		IDL_MessageFromBlock(msg_block, M_CVIO_SHORTARR,
			IDL_MSG_LONGJMP);
	}

	vpTime = IDL_CvtULng(1,&(argv[1]));
	if (vpTime != argv[1]) IDL_VarCopy(vpTime,argv[1]);

	tp = (CVIO_DT_TIME *)vpTime->value.arr->data;
	if (iNext) for(i=0;i<n;i++) tp[i] = CVIO_TIME_NEXT;

	if (iNumArg == 2) {
		/* no output buffer variable specified */
		err = cvio_read_samples(stream,tp,NULL,n); 
	} else {
	    if (type == CVIO_TYP_STRING) {
	        /* return a string (array) */
		IDL_STRING	*s;
		char		*tstr;
		IDL_VPTR	vpTmp;

		tstr = (char *)IDL_GetScratch(&vpTmp,dims[0]+1,1);
		tstr[dims[0]] = '\0';

		s = (IDL_STRING *)IDL_MakeTempVector(IDL_TYP_STRING,
			n, IDL_BARR_INI_ZERO, &vpData);

		for(i=0;i<n;i++) {
			err = cvio_read_samples(stream,tp+i,tstr,1); 
			if (err) break;
			IDL_StrStore(s,tstr);
			s++;
		}
		IDL_Deltmp(vpTmp);

	    } else {
		/* return a "normal" array */
		if ((ndim == 1) && (dims[0] == 1)) {
			dp = (void *)IDL_MakeTempVector(cvio2idl(type),n,
				IDL_ARR_INI_NOP,&vpData);
		} else {
			for(i=0;i<ndim;i++) dd[i] = dims[i];
			dd[ndim] = n;
			dp = (void *)IDL_MakeTempArray(cvio2idl(type),ndim+1,
				dd,IDL_ARR_INI_NOP,&vpData);
		}
		err = cvio_read_samples(stream,tp,dp,n); 
	    }
	    IDL_VarCopy(vpData,argv[2]);
	}

	return(IDL_GettmpLong(err));
}
IDL_VPTR IDL_CDECL idl_read_samples(int argc, IDL_VPTR argv[], char *argk)
{
	return(idl_rd_samples(argc,argv,argk,0,argc-1));
}

IDL_VPTR IDL_CDECL idl_cvio_read_next_samples(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_VPTR	vpTmp;

	IDL_EXCLUDE_EXPR(argv[1]);
/* if not an array, make one */
	if ((!(argv[1]->flags & IDL_V_ARR)) || 
	    (argv[1]->type == IDL_TYP_UNDEF) ||
	    (argv[1]->type == IDL_TYP_STRUCT) ||
	    (argv[1]->type == IDL_TYP_PTR) ||
	    (argv[1]->type == IDL_TYP_OBJREF)) {

		IDL_MakeTempVector(IDL_TYP_ULONG,IDL_LongScalar(argv[3]),
			IDL_ARR_INI_NOP,&vpTmp);
		IDL_VarCopy(vpTmp,argv[1]);
		
	} else {
		IDL_LONG n;
		n = IDL_LongScalar(argv[argc-1]);
		if (n != argv[1]->value.arr->n_elts) {
			IDL_MakeTempVector(IDL_TYP_ULONG,n,IDL_ARR_INI_NOP,&vpTmp);
			IDL_VarCopy(vpTmp,argv[1]);
		}
	}

	return(idl_rd_samples(argc,argv,argk,1,argc-1));
}

IDL_LONG	cviostr2idl(char *str)
{
	IDL_LONG	i;

	if (strcmp(str,"REQ_ATTR_DIMS_US") == 0) return(999);
	i = strlen(str);
	if (i < 2) return(-1);
	if ((str[i-2] == 'U') && (str[i-1] == 'S')) return(IDL_TYP_UINT);
	if ((str[i-2] == 'U') && (str[i-1] == 'L')) return(IDL_TYP_ULONG);
	if ((str[i-2] == 'T') && (str[i-1] == 'I')) return(IDL_TYP_ULONG);
	if ((str[i-2] == 'S') && (str[i-1] == 'T')) return(IDL_TYP_STRING);
	return(-1);
}

IDL_VPTR IDL_CDECL idl_cvio_getattribute(int argc, IDL_VPTR argv[], char *argk)
{
	int32		err,blen=256;
	char		buff[256];
	char		*attr;
	IDL_VPTR	vpTmp;
	IDL_UINT	*p;

	IDL_ENSURE_SCALAR(argv[1]);
	IDL_ENSURE_STRING(argv[1]);
	IDL_EXCLUDE_EXPR(argv[2]);

	attr = IDL_STRING_STR(&(argv[1]->value.str));
	err = cvio_getattribute(IDL_ULongScalar(argv[0]),attr,buff,&blen);
	if (!err) {
		switch(cviostr2idl(attr)) {
			case IDL_TYP_STRING:
				vpTmp = IDL_StrToSTRING(buff);
				break;
			case IDL_TYP_UINT:
				vpTmp = IDL_GettmpUInt(*((unsigned short *)buff));
				break;
			case IDL_TYP_ULONG:
				vpTmp = IDL_GettmpULong(*((IDL_LONG *)buff));
				break;
			case 999:
				p = (IDL_UINT *)IDL_MakeTempVector(
					IDL_TYP_UINT,3,IDL_ARR_INI_NOP,&vpTmp);
				memcpy(p,buff,3*sizeof(IDL_UINT));
				break;
			default:
				vpTmp = IDL_GettmpULong(-1);
				break;
		}
		IDL_VarCopy(vpTmp,argv[2]);
	}

	return(IDL_GettmpLong(err));
}
IDL_VPTR IDL_CDECL idl_cvio_setattribute(int argc, IDL_VPTR argv[], char *argk)
{
	int32		err;
	IDL_VPTR	vpTmp;
	IDL_LONG	type;
	IDL_LONG	count = 1;
	char		*attr;
	void		*buff;

	IDL_ENSURE_SCALAR(argv[1]);
	IDL_ENSURE_STRING(argv[1]);

	attr = IDL_STRING_STR(&(argv[1]->value.str));

	if (argc > 2) {
		type = cviostr2idl(attr);
		if (type == 999) {
			type = IDL_TYP_UINT;
			count = 3;
			IDL_ENSURE_ARRAY(argv[2]);
		} else {
			IDL_ENSURE_SCALAR(argv[2]);
		}

		if (type < 1) return(IDL_GettmpLong(CVIO_ERR_UNKNOWN_ATTR));

		vpTmp = IDL_BasicTypeConversion(1,&(argv[2]),type);
		if (type == IDL_TYP_STRING) {
			buff = IDL_STRING_STR(&(vpTmp->value.str));
		} else if (count == 3) {
			buff = vpTmp->value.arr->data;
		} else {
			buff = &(vpTmp->value.c);
		}

		err = cvio_setattribute(IDL_ULongScalar(argv[0]),attr,buff);

		if (vpTmp != argv[2]) IDL_Deltmp(vpTmp);

	} else {
		err = cvio_setattribute(IDL_ULongScalar(argv[0]),attr,NULL);
	}

	return(IDL_GettmpLong(err));
}

IDL_VPTR IDL_CDECL idl_cvio_getattriblist(int argc, IDL_VPTR argv[], char *argk)
{
	int32 	err;
	int32	blen = 1024;
	int32	n,i;
	char	buff[1024],*p;
	IDL_VPTR	vpTmp;
	IDL_STRING	*s;

	IDL_EXCLUDE_EXPR(argv[1]);
	IDL_EXCLUDE_EXPR(argv[2]);

	err = cvio_getattriblist(IDL_ULongScalar(argv[0]),buff,&blen,&n);
	if (!err) {
		IDL_VarCopy(IDL_GettmpLong(n),argv[2]);
		if (n != 0) {
			s = (IDL_STRING *)IDL_MakeTempVector(IDL_TYP_STRING,
				n, IDL_BARR_INI_ZERO, &vpTmp);
			p = buff;
			for(i=0;i<n;i++) {
				IDL_StrStore(s,p);
				while(*p++);
				s++;
			}
			IDL_VarCopy(vpTmp,argv[1]);
		}
	}

	return(IDL_GettmpLong(err));
}
IDL_VPTR IDL_CDECL idl_cvio_version(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_StrToSTRING(CVIO_LIBRARY_VERSION));
}
IDL_VPTR IDL_CDECL idl_cvio_current_time(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpULong(cvio_current_time()));
}
IDL_VPTR IDL_CDECL idl_cvio_sleep(int argc, IDL_VPTR argv[], char *argk)
{
	int32           err;

	err = cvio_sleep(IDL_DoubleScalar(argv[0]));

	return(IDL_GettmpLong(err));
}
IDL_VPTR IDL_CDECL idl_cvio_tell(int argc, IDL_VPTR argv[], char *argk)
{
	int32	err;
	uint32	c_samp,n_samp;
	CVIO_DT_TIME	c_time;

	IDL_EXCLUDE_EXPR(argv[1]);
	IDL_EXCLUDE_EXPR(argv[2]);
	IDL_EXCLUDE_EXPR(argv[3]);

	err = cvio_tell(IDL_ULongScalar(argv[0]),&c_samp,&c_time,&n_samp);

	if (!err) {
		IDL_VarCopy(IDL_GettmpULong(c_samp),argv[1]);
		IDL_VarCopy(IDL_GettmpULong(c_time),argv[2]);
		IDL_VarCopy(IDL_GettmpULong(n_samp),argv[3]);
	}

	return(IDL_GettmpLong(err));
}
IDL_VPTR IDL_CDECL idl_cvio_seek(int argc, IDL_VPTR argv[], char *argk)
{
	int32   err;

	err = cvio_seek(IDL_ULongScalar(argv[0]),IDL_LongScalar(argv[1]),
		IDL_ULongScalar(argv[2]));

	return(IDL_GettmpLong(err));
}
IDL_VPTR IDL_CDECL idl_cvio_datatype(int argc, IDL_VPTR argv[], char *argk)
{
	int32		err;
	int32		type,ndim,dims[3];
	IDL_LONG	*d;
	IDL_VPTR	vpTmp;

	IDL_EXCLUDE_EXPR(argv[1]);
	IDL_EXCLUDE_EXPR(argv[2]);
	IDL_EXCLUDE_EXPR(argv[3]);

	err = cvio_datatype(IDL_ULongScalar(argv[0]),&type,&ndim,dims);
	if (!err) {
		IDL_VarCopy(IDL_GettmpLong(type),argv[1]);
		IDL_VarCopy(IDL_GettmpLong(ndim),argv[2]);
		d = (IDL_LONG *)IDL_MakeTempVector(IDL_TYP_LONG,3,
			IDL_ARR_INI_NOP,&vpTmp);
		d[0] = dims[0];
		d[1] = dims[1];
		d[2] = dims[2];
		IDL_VarCopy(vpTmp,argv[3]);
	}

	return(IDL_GettmpLong(err));
}

IDL_VPTR IDL_CDECL idl_air_align(int iargc, IDL_VPTR inargv[], char *argk);

int IDL_Load(void)
{
  static IDL_SYSFUN_DEF function_addr[] = {
    { idl_cvio_get_error_msg, "CVIO_GET_ERROR_MESSAGE", 2, 2, 0},
    { idl_cvio_create, "CVIO_CREATE", 6, 6, 0},
    { idl_cvio_listshmem, "CVIO_LISTSHMEM", 2, 2, 0},
    { idl_cvio_delete, "CVIO_DELETE", 1, 1, 0},
    { idl_open, "CVIO_OPEN", 3, 3, 0},
    { idl_close, "CVIO_CLOSE", 1, 1, 0},
    { idl_has_writer, "CVIO_HAS_WRITER", 2, 2, 0},
    { idl_max_length, "CVIO_MAX_LENGTH", 2, 2, 0},
    { idl_add_samples, "CVIO_ADD_SAMPLES", 4, 4, 0},
    { idl_read_samples, "CVIO_READ_SAMPLES", 4, 4, 0},
    { idl_cvio_getattribute, "CVIO_GETATTRIBUTE", 3, 3, 0},
    { idl_cvio_setattribute, "CVIO_SETATTRIBUTE", 2, 3, 0},
    { idl_cvio_getattriblist, "CVIO_GETATTRIBUTELIST", 3, 3, 0},
    { idl_cvio_current_time, "CVIO_CURRENT_TIME", 0, 0, 0},
    { idl_cvio_read_next_samples, "CVIO_READ_NEXT_SAMPLES", 4, 4, 0},
    { idl_cvio_tell, "CVIO_TELL", 4, 4, 0},
    { idl_cvio_seek, "CVIO_SEEK", 3, 3, 0},
    { idl_cvio_datatype, "CVIO_DATATYPE", 4, 4, 0},
    { idl_cvio_sleep, "CVIO_SLEEP", 1, 1, 0},
    { idl_cvio_version, "CVIO_VERSION", 0, 0, 0},
    { idl_air_align, "AIR_ALIGN", 2, 2, IDL_SYSFUN_DEF_F_KEYWORDS},
  };

  if (!(msg_block = IDL_MessageDefineBlock("idl_cvio", ARRLEN(msg_arr),
	   msg_arr))) {
	return IDL_FALSE;
  }

  if (cvio_init()) {
	IDL_MessageFromBlock(msg_block, M_CVIO_ERROR,
		IDL_MSG_RET,"Unable to initialize cvio library");
	return IDL_FALSE;
  }

  if (!IDL_AddSystemRoutine(function_addr, TRUE, ARRLEN(function_addr))) {
	return IDL_FALSE;
  }

  IDL_ExitRegister(cvio_exit_handler);

  return IDL_TRUE;
}
