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
extern IDL_VPTR IDL_CDECL twain_hastwain(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL twain_select(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL twain_acquire(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL twain_open(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL twain_close(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL twain_grab(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL twain_list(int argc, IDL_VPTR argv[], char *argk);

/* define the TWAIN functions */
static IDL_SYSFUN_DEF twain_functions[] = {

    { twain_hastwain,	"TWAIN_PRESENT",    0, 0, 0},
    { twain_select,		"TWAIN_SELECT",	    0, 0, 0},
    { twain_acquire,	"TWAIN_ACQUIRE",    2, 5, IDL_SYSFUN_DEF_F_KEYWORDS},
    { twain_open,		"TWAIN_OPEN",		0, 0, 0},
    { twain_close,		"TWAIN_CLOSE",		0, 0, 0},
    { twain_grab,		"TWAIN_GRAB",		2, 5, 0},
    { twain_list,		"TWAIN_LIST",		1, 1, 0},

};

/*
 * Current:
 *	has_twain = TWAIN_PRESENT()
 *	error = TWAIN_SELECT()
 *	error = TWAIN_ACQUIRE(img,info[,r,g,b][,/HIDE_GUI])
 *
 * Working:
 *  error = TWAIN_OPEN()
 *  error = TWAIN_CLOSE()
 *  error = TWAIN_GRAB(img,info[,r,g,b])
 *
 * TODO:
 *  error = TWAIN_LIST(list) [and TWAIN_SELECT([name])]
 *
 *	Note: <0 = error   0 = ok
 */

#ifdef WIN32

#include <windows.h>
#include "twain.h"
#include "table.h"
#include "idl_twain_glue.h"

/* startup call when DLM is loaded */
int twain_startup(void)
{
	if (!IDL_AddSystemRoutine(twain_functions, TRUE, 
		ARRLEN(twain_functions))) {
		return IDL_FALSE;
	}
	IDL_ExitRegister(twain_exit_handler);

	
	return(IDL_TRUE);
}

/* globals */
static HWND		hwnd = NULL;
TW_IDENTITY	AppIdentity;
TW_INT16	AcqFlag = 0;

/* called when IDL is shutdown */
void twain_exit_handler(void)
{
	CloseConnection();
	if (hwnd) DestroyWindow(hwnd);
}


/* local protos */
static int Setup(void);
static HGLOBAL MessageLoop(TW_UINT16 *why,TW_IMAGEINFO *info,
			  float dpi[2],TW_INT16 *iPal,unsigned char *pal);
static IDL_LONG OpenDefaultSource(TW_BOOL showui,TW_INT16 use_mem);
static IDL_LONG ConvertImage(HGLOBAL in,TW_IMAGEINFO *info,float res[2],
					  TW_INT16 iPal,unsigned char *pal,
					  IDL_LONG argc, IDL_VPTR *argv);

static int Setup()
{
	if (hwnd) return(1);

	hwnd = CreateWindow("STATIC",                     // class
			    "Dummy",                      // title
			    WS_POPUPWINDOW,               // style
			    CW_USEDEFAULT, CW_USEDEFAULT, // x, y
			    CW_USEDEFAULT, CW_USEDEFAULT, // width, height
			    HWND_DESKTOP,                 // parent window
			    NULL,                         // hmenu
			    NULL,		          // hinst
			    NULL);                        // lpvparam

	if (!hwnd) return(0);

	AppIdentity.Id = 0; 		
	AppIdentity.Version.MajorNum = 1;
	AppIdentity.Version.MinorNum = 000;
	AppIdentity.Version.Language = TWLG_USA;
	AppIdentity.Version.Country  = TWCY_USA;
	lstrcpy (AppIdentity.Version.Info,  "IDL_TWAIN 1.0.0.0  04/18/1999");
	lstrcpy (AppIdentity.ProductName,   "IDL_TWAIN");

	AppIdentity.ProtocolMajor = 1;
	AppIdentity.ProtocolMinor = 7;
	AppIdentity.SupportedGroups =  DG_IMAGE | DG_CONTROL;
	lstrcpy (AppIdentity.Manufacturer,  "Anonymous");
	lstrcpy (AppIdentity.ProductFamily, "IDL_TOOL DLM");

	// pass app particulars to glue code
	TWInitialize (&AppIdentity, hwnd);

	AcqFlag = 0;

	return(1);
}

static HGLOBAL MessageLoop(TW_UINT16 *why,TW_IMAGEINFO *info,
			  float dpi[2],TW_INT16 *iPal,unsigned char *pal)
{
	MSG			msg;
	TW_UINT16	finished = 0;
	HGLOBAL		hDIB = NULL;

	while (GetMessage((LPMSG)&msg, NULL, 0, 0))
	{
		if (!ProcessTWMessage ((LPMSG)&msg, hwnd, &finished, &hDIB,
				info,dpi,iPal,pal)) {
			TranslateMessage ((LPMSG)&msg);
			DispatchMessage ((LPMSG)&msg);
		}
		if (finished || hDIB) break;
	}                     

	if (why) *why = finished;

	return(hDIB);
}

void LogMessage(char *str)
{
	DEBUGMSG(str);
	return;
}

IDL_VPTR IDL_CDECL twain_hastwain(int argc, IDL_VPTR argv[], char *argk)
{
	if (!Setup()) IDL_GettmpLong(-1);
	if (AcqFlag) return(IDL_GettmpLong(1));

	if (TWOpenDSM() == TRUE) {
		TWCloseDSM ();
		return(IDL_GettmpLong(1));
	}

	return(IDL_GettmpLong(0));
}

IDL_VPTR IDL_CDECL twain_select(int argc, IDL_VPTR argv[], char *argk)
{
	BOOL	err;

	if (!Setup()) IDL_GettmpLong(-1);
	if (AcqFlag) return(IDL_GettmpLong(-10));

	if (TWOpenDSM() == TRUE) {
		err = TWSelectDS();
		TWCloseDSM ();
		if (err) return(IDL_GettmpLong(-1));

		return(IDL_GettmpLong(0));
	}

	return(IDL_GettmpLong(-1));
}

static IDL_LONG OpenDefaultSource(TW_BOOL ShowUI,TW_INT16 use_mem)
{
	TW_INT16	result = TRUE;
	IDL_LONG	eRet = 0;

	if (AcqFlag) return(-10);

	/* use the default data source */
	TWDefaultDS();

	if (TWOpenDSM() == TRUE) {
		DEBUGMSG("TWOpenDSM");
		if (TWOpenDS() == TRUE) {
			DEBUGMSG("TWOpenDS");
			if (TWXferMech(hwnd,use_mem) == TWRC_SUCCESS) {
				DEBUGMSG("TWXferMech");
				if (!TWIsDSEnabled()) {
						DEBUGMSG("TWIsDSEnabled");
						result = TWEnableDS (ShowUI);
						DEBUGMSG("TWEnableDS");
						if (result == FALSE) {
							eRet = -4;
						} else {
							AcqFlag = 3;
						}
						/*AcqFlag = 0 Do Not Accept MSG_XFERREADY
						 *          1 Disable/CloseDS/CloseDSM
						 *          2 Disable Only
						 *          3 Do Not Disable - only if ShowUI=TRUE
						 */
						/*
						 * if (result) hDIB = MessageLoop();
						 */
				}
			} else {
				eRet = -2;
			}
		} else {
			eRet = -3;
			LogMessage("OpenDS failed -- TWAcquire\r\n");
		}
	} else {
		eRet = -5;
	}
	/* clean up any errors */
	if (eRet != 0) {
		CloseConnection();
		AcqFlag = 0;
	}
	return (eRet);
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

IDL_VPTR IDL_CDECL twain_acquire(int inargc, IDL_VPTR inargv[], char *argk)
{
	TW_BOOL		ShowUI = TRUE;
	IDL_LONG	eRet	= -1;
	HGLOBAL		hDIB;
	IDL_LONG	i,j;
	TW_IMAGEINFO	info;
	float			res[2];
	TW_INT16		iPal;
	unsigned char	pal[768];

	IDL_VPTR	argv[5];
	int		argc;

static  IDL_LONG	iHideGUI;
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
	{"HIDE_GUI",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iHideGUI)},
	{NULL}
};

	if (!Setup()) IDL_GettmpLong(-1);
	if (AcqFlag) return(IDL_GettmpLong(-10));

	/* parse keywords */
	IDL_KWCleanup(IDL_KW_MARK);
	argc = IDL_KWGetParams(inargc,inargv,argk,kw_pars,argv,1);

	for(i=0;i<argc;i++) IDL_EXCLUDE_EXPR(argv[i]);

	if (iHideGUI) ShowUI = FALSE;

	hDIB = NULL;

	eRet = OpenDefaultSource(ShowUI,1);
	if (eRet != 0) return(IDL_GettmpLong(eRet));

	/* Grab an image */
	hDIB = MessageLoop(NULL,&info,res,&iPal,pal);

	/* make extra sure things are closed down properly */
	CloseConnection();
	DEBUGMSG("CloseConnection");

	/* if we have no output, return an error */
	if (!hDIB) return(IDL_GettmpLong(-1));

	DEBUGMSG("Converting");
	eRet = ConvertImage(hDIB,&info,res,iPal,pal,argc,argv);

#ifdef NEVER

{
	LPBITMAPINFOHEADER	lpdib;
	RGBQUAD			*pColor;
	UCHAR			*pData,*pOut,*pSrc,*pDst;
	IDL_StructDefPtr	sdef;
	IDL_LONG		iDims[4];
	IDL_VPTR		vpTmp;
	twain_info		*info;
	IDL_LONG		iBits,iColors,iScanline;

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
#ifdef DEBUG
{
	FILE *fp;
	fp = fopen("junkbmp.bin","wb");
	fwrite(lpdib,GlobalSize(hDIB),1,fp);
	fclose(fp);
}
#endif

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
	info->fDPI[0] = (float)lpdib->biXPelsPerMeter;
	info->fDPI[1] = (float)lpdib->biYPelsPerMeter;
	info->fDPI[0] *= (0.0254f);
	info->fDPI[1] *= (0.0254f);
	if (iColors) info->iHavePalette = 1;
	info->iIndex = 0;
	info->iNum = 1;
	info->iPixelType = IDL_TYP_BYTE;
	IDL_StrStore(&(info->sType),"TWAIN");
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
	} else {
		iDims[0] = info->iChannels;
		iDims[1] = info->iDims[0];
		iDims[2] = info->iDims[1];
		iDims[4] = 3;
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

#endif

	IDL_KWCleanup(IDL_KW_CLEAN);

	return(IDL_GettmpLong(eRet)); 
}

/* classical breakdown...(note: only TWAIN_GRAB is legal between
 * TWAIN_OPEN and TWAIN_CLOSE)
 */
IDL_VPTR IDL_CDECL twain_open(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_LONG	eRet = 0;

	if (!Setup()) IDL_GettmpLong(-1);
	if (AcqFlag) return(IDL_GettmpLong(-10));

#ifdef DEBUG
	SetMessageLevel(ML_FULL);
#endif

	eRet = OpenDefaultSource(FALSE,1);

	return(IDL_GettmpLong(eRet));
}
IDL_VPTR IDL_CDECL twain_close(int argc, IDL_VPTR argv[], char *argk)
{
	/* only if we are in an "open" state */
	if (!AcqFlag) return(IDL_GettmpLong(-10));

	CloseConnection();
	DEBUGMSG("CloseConnection");
	AcqFlag = 0;

	return(IDL_GettmpLong(0));
}

IDL_VPTR IDL_CDECL twain_grab(int argc, IDL_VPTR argv[], char *argk)
{
	HGLOBAL			hDIB = NULL;
	TW_IMAGEINFO	info;
	float			res[2];
	TW_INT16		iPal;
	unsigned char	pal[768];
	IDL_LONG		ret = -1;
	int				i;

	for(i=0;i<argc;i++) IDL_EXCLUDE_EXPR(argv[i]);

	/* only if we are in an "open" state */
	if (!AcqFlag) return(IDL_GettmpLong(-10));

	hDIB = TWDoMemTransfer(hwnd,&info,res,&iPal,pal);
	DEBUGMSG("Image Acquired");
	if (hDIB) {
		DEBUGMSG("Converting");
		ret = ConvertImage(hDIB,&info,res,iPal,pal,argc,argv);
	} else {
		return(IDL_GettmpLong(-1));
	}
/* 
   The Intel TWAIN drivers require the source be closed and
   reopened for each "grab".

	TWDisableDS();
	TWCloseDS();
	TWOpenDS();
	TWXferMech(hwnd,1);
	TWEnableDS(FALSE);
*/
	return(IDL_GettmpLong(ret));
}

static IDL_LONG ConvertImage(HGLOBAL in,TW_IMAGEINFO *info,float res[2],
					  TW_INT16 iPal,unsigned char *pal,
					  IDL_LONG argc, IDL_VPTR *argv)
{
	unsigned char	*pData,*pOut,*pDst,*pSrc;
	IDL_StructDefPtr	sdef;
	IDL_LONG		iDims[5];
	IDL_VPTR		vpTmp;
	twain_info		*inf;
	IDL_LONG		iBits,iScanline,iBytes;
	int				i,j;

	/* build the struct */
	sdef = IDL_MakeStruct(0, s_tags);
	iDims[0] = 1;
	inf = (twain_info *)IDL_MakeTempStruct(sdef, 1, iDims, &vpTmp, TRUE);

	/* lock'em */
	pData = (unsigned char *)GlobalLock(in);

	/* fix an Intel BUG: They report a 24bit image as 24bits/sample */
	if ((info->BitsPerSample[0] == 24) && (info->SamplesPerPixel == 3)) {
		info->BitsPerSample[0] = 8;
		info->BitsPerSample[1] = 8;
		info->BitsPerSample[2] = 8;
	}

	/* reject bad formats */
	/* if any channel is different from others */
	for(i=1;i<info->SamplesPerPixel;i++) {
		if (info->BitsPerSample[i] != info->BitsPerSample[0]) {
			GlobalUnlock(in);
			GlobalFree(in);
			IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIBFORMAT, 
				IDL_MSG_LONGJMP,"Non-uniform channel depth");
		}
	}
	/* pixel interleaved */
	if (info->Planar != TWPC_CHUNKY) {
		GlobalUnlock(in);
		GlobalFree(in);
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIBFORMAT, 
				IDL_MSG_LONGJMP,"Planar interleaved");
	}
	/* color 1bit/channel */
	if ((info->BitsPerSample[0] == 1) &&
		(info->SamplesPerPixel != 1))
	{
		GlobalUnlock(in);
		GlobalFree(in);
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIBFORMAT, 
				IDL_MSG_LONGJMP,"1 bit color");
	}
	/* specific bit depths (TODO: handle 16bits) */
	if ((info->BitsPerSample[0] != 1) &&
		(info->BitsPerSample[0] != 8)) {

#ifdef DEBUG
		char	tstr[4096];
		sprintf(tstr,"bps: %d, bpp: %d, spp: %d",info->BitsPerSample[0],
			info->BitsPerPixel,info->SamplesPerPixel);
		IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
				IDL_MSG_INFO,tstr);
#endif

		GlobalUnlock(in);
		GlobalFree(in);
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIBFORMAT, 
				IDL_MSG_LONGJMP,"Non 1 or 8 bits/channel");
	}

	/* raw data pointers */
	iBits = info->BitsPerPixel;
	iBytes = info->BitsPerPixel/8;
	iScanline = (info->ImageWidth*iBits)/8;
	while(iScanline & 0x3) iScanline++;

	/* Info Struct */
	inf->iDims[0] = info->ImageWidth;
	inf->iDims[1] = info->ImageLength;
	inf->fDPI[0] = res[0];
	inf->fDPI[1] = res[1];
	if (iPal) inf->iHavePalette = 1;
	inf->iIndex = 0;
	inf->iNum = 1;
	inf->iPixelType = IDL_TYP_BYTE;
	if (info->BitsPerSample[0] > 8) inf->iPixelType = IDL_TYP_UINT;

	IDL_StrStore(&(inf->sType),"TWAIN");
	inf->iChannels = info->SamplesPerPixel;
	IDL_VarCopy(vpTmp,argv[1]);

	/* palette */
	if (inf->iHavePalette) {
		if (argc >= 3) {
			pOut = (UCHAR *)IDL_MakeTempVector(IDL_TYP_BYTE,iPal,
				IDL_BARR_INI_NOP,&vpTmp);
			for(i=0;i<iPal;i++) pOut[i] = pal[i];
			IDL_VarCopy(vpTmp,argv[2]);
		}
		if (argc >= 4) {
			pOut = (UCHAR *)IDL_MakeTempVector(IDL_TYP_BYTE,iPal,
				IDL_BARR_INI_NOP,&vpTmp);
			for(i=0;i<iPal;i++) pOut[i] = pal[i+256];
			IDL_VarCopy(vpTmp,argv[3]);
		}
		if (argc >= 5) {
			pOut = (UCHAR *)IDL_MakeTempVector(IDL_TYP_BYTE,iPal,
				IDL_BARR_INI_NOP,&vpTmp);
			for(i=0;i<iPal;i++) pOut[i] = pal[i+512];
			IDL_VarCopy(vpTmp,argv[4]);
		}
	}

	/* destination image */
	if (inf->iChannels == 1) {
		iDims[0] = inf->iDims[0];
		iDims[1] = inf->iDims[1];
		iDims[4] = 2;
	} else {
		iDims[0] = inf->iChannels;
		iDims[1] = inf->iDims[0];
		iDims[2] = inf->iDims[1];
		iDims[4] = 3;
	}
	pOut = (UCHAR *)IDL_MakeTempArray(inf->iPixelType,iDims[4],iDims, 
			IDL_BARR_INI_NOP,&vpTmp);
	IDL_VarCopy(vpTmp,argv[0]);

	/* copy the bits */
	switch(info->BitsPerSample[0]) {
		case 1 : 
			pDst = pOut;
			for(j=0;j<inf->iDims[1];j++) {
				pSrc = pData + (inf->iDims[1]-j-1)*iScanline;
				for(i=0;i<inf->iDims[0];i++) {
					IDL_LONG	by,bi;

					by = i / 8;
					bi = 7 - (i % 8);
					*pDst++ = (pSrc[by] & (1<<bi)) ? 1 : 0;
				}
			}
			break;
		case 8:
			pDst = pOut;
			for(j=0;j<inf->iDims[1];j++) {
				pSrc = pData + (inf->iDims[1]-j-1)*iScanline;
				for(i=0;i<inf->iDims[0];i++) {
					int	k;
					for(k=0;k<inf->iChannels;k++) {
						*pDst++ = pSrc[k];
					}
					pSrc += iBytes;
				}
			}
			break;	
	}

	/* done */
	GlobalUnlock(in);
	if (GlobalFlags(in) & GMEM_LOCKCOUNT) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
				IDL_MSG_INFO,"Bad lock count");
	}
	GlobalFree(in);

	return(0);
}

/* future */
IDL_VPTR IDL_CDECL twain_list(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}


#else

/* startup call when DLM is loaded */
int twain_startup(void)
{
	if (!IDL_AddSystemRoutine(twain_functions, TRUE, 
		ARRLEN(twain_functions))) {
		return IDL_FALSE;
	}
	IDL_ExitRegister(twain_exit_handler);
	return(IDL_TRUE);
}

/* called when IDL is shutdown */
void twain_exit_handler(void)
{
}
IDL_VPTR IDL_CDECL twain_hastwain(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(0));
}
IDL_VPTR IDL_CDECL twain_select(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}
IDL_VPTR IDL_CDECL twain_acquire(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}
IDL_VPTR IDL_CDECL twain_open(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}
IDL_VPTR IDL_CDECL twain_close(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}
IDL_VPTR IDL_CDECL twain_grab(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}
IDL_VPTR IDL_CDECL twain_list(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

#endif

