#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "export.h"
#include "idl_tools.h"

#ifdef DEBUG
#define DEBUGMSG(a) IDL_MessageFromBlock(msg_block,M_TOOLS_ERROR,IDL_MSG_INFO,a)
#else
#define DEBUGMSG(a)
#endif

/* function protos */
extern IDL_VPTR IDL_CDECL vfw_hasvfw(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL vfw_connect(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL vfw_list(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL vfw_grab(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL vfw_dialog(int argc, IDL_VPTR argv[], char *argk);

/* define the VFW functions */
static IDL_SYSFUN_DEF vfw_functions[] = {
    { vfw_hasvfw,	"VFW_PRESENT",		0, 0, 0},
    { vfw_dialog,	"VFW_DIALOG",	    1, 1, 0},
    { vfw_grab,		"VFW_GRAB",			2, 5, 0},
    { vfw_connect,	"VFW_CONNECT",		0, 1, 0},
    { vfw_list,		"VFW_LIST",			2, 2, 0},
};

/* API: */
/* err = VFW_DIALOG(which) */
/* exist = VFW_PRESENT() */
/* err=VFW_LIST(names,descs) */
/* err = VFW_GRAB(i,s[,r[,g[,b]]]) */
/* err = VFW_CONNECT([num]) */
/*
 *	Note: <0 = error   0 = ok
 */

#ifdef WIN32

#include <windows.h>
#include <vfw.h>

/* startup call when DLM is loaded */
int vfw_startup(void)
{
	if (!IDL_AddSystemRoutine(vfw_functions, TRUE, 
		ARRLEN(vfw_functions))) {
		return IDL_FALSE;
	}
	IDL_ExitRegister(vfw_exit_handler);

	
	return(IDL_TRUE);
}

/* globals */
static HWND		hwnd = NULL;
static HWND		gwnd = NULL;
static BOOL		bIsConn = FALSE;
static HIC		hic = NULL;
static BITMAPINFOHEADER    bihOut; 
static BITMAPINFOHEADER   *bihIn; 
static HANDLE	hDIB = NULL;

/* local protos */
static int Setup(void);
static BOOL Connect(int i);
static void FindDecompress(void);

/* called when IDL is shutdown */
void vfw_exit_handler(void)
{
	if (hwnd) {
		if (bIsConn) capDriverDisconnect(hwnd);
		bIsConn = FALSE;
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	if (gwnd) DestroyWindow(gwnd);
	gwnd = NULL;
	FindDecompress();
}

static int Setup()
{
	if (gwnd) return(1);

	DEBUGMSG("1");
	gwnd = CreateWindow("STATIC",				// class
			    "Dummy",						// title
			    WS_POPUPWINDOW,					// style
			    CW_USEDEFAULT, CW_USEDEFAULT,	// x, y
			    CW_USEDEFAULT, CW_USEDEFAULT,	// width, height
			    HWND_DESKTOP,					// parent window
			    NULL,							// hmenu
			    NULL,							// hinst
			    NULL);							// lpvparam

	if (!gwnd) return(0);

	DEBUGMSG("2");
	hwnd = capCreateCaptureWindow((LPSTR)"VFW grab",
		WS_CHILD | WS_VISIBLE, 
		0, 0, 160, 120, gwnd, 1);              
 	if (!hwnd) {
		DestroyWindow(gwnd);
		gwnd = NULL;
		return(0);
	}

	DEBUGMSG("3");
	Connect(0);
 
	return(1);
}

static void FindDecompress(void)
{
	int		size;

	DEBUGMSG("D1");
	if (hic) {
		ICClose(hic);
		free(bihIn);
		hic = NULL;
		bihIn = NULL;
	}

	DEBUGMSG("D2");
	if (!bIsConn) return;

	size = capGetVideoFormatSize(hwnd);
	bihIn = (BITMAPINFOHEADER *)malloc(size);
	size = capGetVideoFormat(hwnd,bihIn,size);
 
/* Initialize the output bitmap structure. */
	bihOut.biSize = sizeof(BITMAPINFOHEADER); 
	bihOut.biWidth = bihIn->biWidth;
	bihOut.biHeight = bihIn->biHeight; 
	bihOut.biPlanes = 1; 
	bihOut.biCompression = BI_RGB;
	bihOut.biBitCount = 24; 
	bihOut.biSizeImage = 0; 
    bihOut.biXPelsPerMeter = bihOut.biYPelsPerMeter = 0; 
    bihOut.biClrUsed = bihOut.biClrImportant = 0; 

	hic = ICLocate(ICTYPE_VIDEO, 0L, 
				bihIn, &bihOut, ICMODE_DECOMPRESS);

	DEBUGMSG("D3");
 
	return;
}

static LRESULT PASCAL FrameCallbackProc(HWND hWnd, LPVIDEOHDR lpVHdr) 
{ 
	hDIB = NULL;

	DEBUGMSG("FRAME");
	if (!hic) return(FALSE);

	hDIB = ICImageDecompress(hic,0,bihIn,lpVHdr->lpData,&bihOut);

#ifdef DEBUG
if (hDIB) {
	FILE *fp;
	unsigned char	*tmp;

	DEBUGMSG("JUNK WRITE");
	tmp = (unsigned char *)GlobalLock(hDIB);
	if (!tmp) {
		GlobalUnlock(hDIB);
	} else {
		fp = fopen("junkbmp.bin","wb");
		fwrite(tmp,GlobalSize(hDIB),1,fp);
		fclose(fp);
		GlobalUnlock(hDIB);
	}
}
#endif

    return (LRESULT) TRUE ; 
} 
 
static BOOL Connect(int i)
{

	bIsConn = capDriverConnect(hwnd,i);
	if (bIsConn) {
		DEBUGMSG("Connected");
		capSetCallbackOnFrame(hwnd,FrameCallbackProc);
		FindDecompress();
		if (!hic) {
			DEBUGMSG("No decompresssor");
			capDriverDisconnect(hwnd);
			bIsConn = FALSE;
		}
	}

	return(bIsConn);
}
 
/* exist = VFW_PRESENT() */
IDL_VPTR IDL_CDECL vfw_hasvfw(int argc, IDL_VPTR argv[], char *argk)
{
	if (!Setup()) return(IDL_GettmpLong(-1));

	return(IDL_GettmpLong(1));
}

/* err = VFW_DIALOG(which) */
IDL_VPTR IDL_CDECL vfw_dialog(int argc, IDL_VPTR argv[], char *argk)
{
	BOOL	err;

	if (!Setup()) return(IDL_GettmpLong(-1));
	if (!bIsConn) return(IDL_GettmpLong(-2));

	switch(IDL_LongScalar(argv[0])) {
		case 0:
			err = capDlgVideoSource(hwnd);
			break;
		case 1:
			err = capDlgVideoFormat(hwnd);
			break;
		case 2:
			err = capDlgVideoDisplay(hwnd);
			break;
		case 3:
			err = capDlgVideoCompression(hwnd);
			break;
	}

	if (!err) return(IDL_GettmpLong(-1));

	FindDecompress();
	if (!hic) return(IDL_GettmpLong(-3));

	return(IDL_GettmpLong(0));
}

/* err = VFW_CONNECT([num]) */
IDL_VPTR IDL_CDECL vfw_connect(int argc, IDL_VPTR argv[], char *argk)
{
	if (!Setup()) return(IDL_GettmpLong(-1));

	if (bIsConn) {
		capDriverDisconnect(hwnd);
		bIsConn = FALSE;
	}

	if (argc == 1) {
		Connect(IDL_LongScalar(argv[0]));
		if (!bIsConn) return(IDL_GettmpLong(-1));
	}

	return(IDL_GettmpLong(0));
}

/* Returned Info structure */
typedef struct {
	IDL_LONG	iChannels;
	IDL_LONG	iDims[2];
	float		fDPI[2];
	short		iHavePalette;
	IDL_LONG	iIndex;
	IDL_LONG	iNum;
	short		iPixelType;
	IDL_STRING	sType;
} twain_info;
static IDL_LONG	dim_dims[] = {1, 2};
static IDL_STRUCT_TAG_DEF s_tags[] = {
	{"CHANNELS", 0, (void *) IDL_TYP_LONG},
	{"DIMENSIONS", dim_dims, (void *) IDL_TYP_LONG},
	{"DPI", dim_dims, (void *) IDL_TYP_FLOAT},
	{"HAS_PALETTE", 0, (void *) IDL_TYP_INT},
	{"IMAGE_INDEX", 0, (void *) IDL_TYP_LONG},
	{"NUM_IMAGES", 0, (void *) IDL_TYP_LONG},
	{"PIXEL_TYPE", 0, (void *) IDL_TYP_INT},
	{"TYPE", 0, (void *) IDL_TYP_STRING},
	{ 0 }
};
/* err = VFW_GRAB(i,s[,r[,g[,b]]]) */
IDL_VPTR IDL_CDECL vfw_grab(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_LONG	eRet	= -1;
	IDL_LONG	i,j;

	if (!Setup()) IDL_GettmpLong(-1);
	if (!bIsConn) return(IDL_GettmpLong(-2));

	for(i=0;i<argc;i++) IDL_EXCLUDE_EXPR(argv[i]);

	hDIB = NULL;

	/* Grab an image */
	capGrabFrame(hwnd);

	/* if we have no output, return an error */
	if (!hDIB) return(IDL_GettmpLong(eRet));

{
	LPBITMAPINFOHEADER	lpdib;
	RGBQUAD			*pColor;
	UCHAR			*pData,*pOut,*pSrc,*pDst;
	IDL_StructDefPtr	sdef;
	IDL_LONG		iDims[4];
	IDL_VPTR		vpTmp;
	twain_info		*info;
	IDL_LONG		iBits,iColors,iScanline,iLen;
	HDC	hdc;

	/* build the struct */
	sdef = IDL_MakeStruct(0, s_tags);
	iDims[0] = 1;
	info = (twain_info *)IDL_MakeTempStruct(sdef, 1, iDims, &vpTmp, TRUE);

	/* cook up return values */
	lpdib = (LPBITMAPINFOHEADER)GlobalLock(hDIB);
	if (!lpdib) {
		GlobalFree(hDIB);
		return(IDL_GettmpLong(-1));
	}

	pColor = (RGBQUAD *)((LPSTR)lpdib + (WORD)(lpdib->biSize));

	if (lpdib->biCompression != BI_RGB) {
		GlobalUnlock(hDIB);
		GlobalFree(hDIB);
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIBFORMAT, 
				IDL_MSG_LONGJMP,"Compressed");
	} 
	if ((lpdib->biBitCount == 4) || (lpdib->biBitCount == 16)) {
		GlobalUnlock(hDIB);
		GlobalFree(hDIB);
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIBFORMAT, 
				IDL_MSG_LONGJMP,"4bit and 16 bit");
	}

	/* raw data pointers */
	iBits = lpdib->biBitCount;
	iColors = lpdib->biClrUsed;
	if ((iColors == 0) && (iBits == 8)) iColors = 256;
	if ((iColors == 0) && (iBits == 1)) iColors = 2;
	iScanline = (lpdib->biWidth*iBits)/8;
	while(iScanline & 0x3) iScanline++;
	pData = (UCHAR *)(pColor+iColors);

	/* Info Struct */
	info->iDims[0] = lpdib->biWidth;
	info->iDims[1] = IDL_ABS(lpdib->biHeight);
	/* screen resolution */
	hdc = GetWindowDC(hwnd);
	info->fDPI[0] = GetDeviceCaps(hdc,LOGPIXELSX);
	info->fDPI[1] = GetDeviceCaps(hdc,LOGPIXELSY);
	ReleaseDC(hwnd,hdc);
	if (iColors) info->iHavePalette = 1;
	info->iIndex = 0;
	info->iNum = 1;
	info->iPixelType = IDL_TYP_BYTE;
	IDL_StrStore(&(info->sType),"VFW");
	info->iChannels = 1;
	if (iBits > 8) info->iChannels = 3;
	IDL_VarCopy(vpTmp,argv[1]);

	/* palette */
	if (info->iHavePalette) {
		if (argc >= 3) {
			pOut = (UCHAR *)IDL_MakeTempVector(IDL_TYP_BYTE,iColors,
				IDL_BARR_INI_NOP,&vpTmp);
			for(i=0;i<iColors;i++) pOut[i] = pColor[i].rgbRed;
			IDL_VarCopy(vpTmp,argv[2]);
		}
		if (argc >= 4) {
			pOut = (UCHAR *)IDL_MakeTempVector(IDL_TYP_BYTE,iColors,
				IDL_BARR_INI_NOP,&vpTmp);
			for(i=0;i<iColors;i++) pOut[i] = pColor[i].rgbGreen;
			IDL_VarCopy(vpTmp,argv[3]);
		}
		if (argc >= 5) {
			pOut = (UCHAR *)IDL_MakeTempVector(IDL_TYP_BYTE,iColors,
				IDL_BARR_INI_NOP,&vpTmp);
			for(i=0;i<iColors;i++) pOut[i] = pColor[i].rgbBlue;
			IDL_VarCopy(vpTmp,argv[4]);
		}
	}

	/* destination image */
	if (info->iChannels == 1) {
		iDims[0] = info->iDims[0];
		iDims[1] = info->iDims[1];
		iDims[4] = 2;
		iLen = iDims[0]*iDims[1];
	} else {
		iDims[0] = info->iChannels;
		iDims[1] = info->iDims[0];
		iDims[2] = info->iDims[1];
		iDims[4] = 3;
		iLen = iDims[0]*iDims[1]*iDims[2];
	}
	pOut = (UCHAR *)IDL_MakeTempArray(IDL_TYP_BYTE,iDims[4],iDims, 
			IDL_BARR_INI_NOP,&vpTmp);
	IDL_VarCopy(vpTmp,argv[0]);

	switch(iBits) {
		case 1 : 
			pDst = pOut;
			for(j=0;j<info->iDims[1];j++) {
				pSrc = pData + j*iScanline;
				for(i=0;i<info->iDims[0];i++) {
					IDL_LONG	by,bi;

					by = i / 8;
					bi = 7 - (i % 8);
					*pDst++ = (pSrc[by] & (1<<bi)) ? 1 : 0;
				}
			}
			break;
		case 8:
			pDst = pOut;
			for(j=0;j<info->iDims[1];j++) {
				pSrc = pData + j*iScanline;
				for(i=0;i<info->iDims[0];i++) {
					*pDst++ = *pSrc++;
				}
			}
			break;
		case 24:
			pDst = pOut;
			for(j=0;j<info->iDims[1];j++) {
				pSrc = pData + j*iScanline;
				for(i=0;i<info->iDims[0];i++) {
					pDst[2] = *pSrc++;
					pDst[1] = *pSrc++;
					pDst[0] = *pSrc++;
					pDst += 3;
				}
			}
			break;
		case 32:
			pDst = pOut;
			for(j=0;j<info->iDims[1];j++) {
				pSrc = pData + j*iScanline;
				for(i=0;i<info->iDims[0];i++) {
					pDst[2] = *pSrc++;
					pDst[1] = *pSrc++;
					pDst[0] = *pSrc++;
					pSrc++;
					pDst += 3;
				}
			}
			break;
	}
}
	/* We're out of here... */
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);

	return(IDL_GettmpLong(0)); 
}


/* err=VFW_LIST(names,descs) */
IDL_VPTR IDL_CDECL vfw_list(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_MEMINT	i,cnt=0;
	IDL_STRING	*s0,*s1;
	IDL_VPTR	vtmp;
	for(i=0;i<argc;i++) IDL_EXCLUDE_EXPR(argv[i]);

	for(i=0;i<10;i++) {
		char	pName[256],pVer[256];
		if (capGetDriverDescription(i,pName,256,pVer,256)) cnt++;
	}
	if (cnt < 1) return(IDL_GettmpLong(0));

	s0 = (IDL_STRING *)IDL_MakeTempArray(IDL_TYP_STRING,1,&cnt,
			IDL_ARR_INI_ZERO, &vtmp);
	IDL_VarCopy(vtmp,argv[0]);
	s1 = (IDL_STRING *)IDL_MakeTempArray(IDL_TYP_STRING,1,&cnt,
			IDL_ARR_INI_ZERO, &vtmp);
	IDL_VarCopy(vtmp,argv[1]);

	for(i=0;i<10;i++) {
		char	pName[256],pVer[256];
		if (capGetDriverDescription(i,pName,256,pVer,256)) { 
			IDL_StrStore(s0++, pName);
			IDL_StrStore(s1++, pVer);
		}
	}

	return(IDL_GettmpLong(cnt));
}


#else

/* startup call when DLM is loaded */
int vfw_startup(void)
{
	if (!IDL_AddSystemRoutine(vfw_functions, TRUE, 
		ARRLEN(vfw_functions))) {
		return IDL_FALSE;
	}
	IDL_ExitRegister(vfw_exit_handler);
	return(IDL_TRUE);
}

/* called when IDL is shutdown */
void vfw_exit_handler(void)
{
}
IDL_VPTR IDL_CDECL vfw_hasvfw(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(0));
}
IDL_VPTR IDL_CDECL vfw_list(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}
IDL_VPTR IDL_CDECL vfw_connect(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}
IDL_VPTR IDL_CDECL vfw_grab(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}
IDL_VPTR IDL_CDECL vfw_dialog(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

#endif

