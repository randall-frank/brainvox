/*
* File: Table.h
* company: J.F.L. Peripherals Inc.
* Date: Mai 19/98
* Description:
*			This header file contain the definition for all the Tables Entry.
*
*	Copyright � 1998 TWAIN Working Group: Adobe Systems Incorporated, 
*	Canon Information Systems, Eastman Kodak Company, 
*	Fujitsu Computer Products of America, Genoa Technology, 
*	Hewlett-Packard Company, Intel Corporation, Kofax Image Products, 
*	JFL Peripheral Solutions Inc., Ricoh Corporation, and Xerox Corporation.  
*	All rights reserved.
*/

#ifndef _inc_table_h
#define _inc_table_h

typedef struct
{
	const char* pszItemName;
	TW_UINT16 ItemId;
} TABLEENTRY, *pTABLEENTRY;

typedef struct
{
	TABLEENTRY* Table;
	int Max;
} TABLEMSG, *pTABLEMSG;


typedef struct
{
	const char* pszDataType;
	char **gszDataStructure;
	TW_UINT16 Max;	
} TABLEDATSTRUCT, *pTABLEDATSTRUCT;


typedef struct
{
	TABLEENTRY EntryHeader;
	TABLEENTRY *pItemTable;
	TW_UINT32 ItemTableSize;
	TW_UINT16 ExpectedType;
} TABLECAP, *pTABLECAP;


/*
* Maximum for each table
*/
#define MAX_DEST 2
#define MAX_DG 2
#define MAX_DAT 23
#define MAX_MSG 23
#define MAX_CAP 73
#define MAX_CONTYPE 5
#define MAX_DGSM 1
#define MAX_DGS 2
#define MAX_DSMDAT 3
#define MAX_DSCONTROL 9
#define MAX_DSIMAGE 11
#define MAX_CAPABILITY 6
#define MAX_CIECOLOR 1
#define MAX_CUSTOMDSDATA 2
#define MAX_EVENT 1
#define MAX_EXTIMAGEINFO 1
#define MAX_GRAYRESPONSE 2
#define MAX_IDENTITY 6
#define MAX_IMAGEFILEXFER 1
#define MAX_IMAGEINFO 1
#define MAX_IMAGELAYOUT 4
#define MAX_IMAGEMEMXFER 1
#define MAX_IMAGENATIVEXFER 1
#define MAX_JPEGCOMPRESSION 4
#define MAX_NULL 3
#define MAX_PALETTE8 4
#define MAX_PARENT 2
#define MAX_PENDINGXFERS 3
#define MAX_RGBRESPONSE 2
#define MAX_SETUPFILEXFER 4
#define MAX_SETUPMEMXFER 1
#define MAX_STATUS 1
#define MAX_USERINTERFACE 3
#define MAX_XFERGROUP 1
#define MAX_PIXELTYPE 9
#define MAX_PALETTETYPE 3
#define MAX_FORMATTYPE 5
#define MAX_COMPRESSIONTYPE 9
#define MAX_VERLANGUAGETYPE 14
#define MAX_VERCONTRYTYPE 210
#define MAX_BITORDERTYPE 2
#define MAX_FILTERTYPE 9
#define MAX_LIGHTPATHTYPE 2
#define MAX_LIGHTSOURCETYPE 7
#define MAX_ORIENTATIONTYPE 6
#define MAX_PLANARCHUNKYTYPE 2
#define MAX_PIXELFLAVORTYPE 2
#define MAX_SETUPXFERTYPE 3
#define MAX_UNITSTYPE 6
#define MAX_SUPPORTEDSIZETYPE 16
#define MAX_BITDEPTHREDUCTIONTYPE 4
#define MAX_BOOLTYPE 2
#define MAX_DUPLEXTYPE 3
#define MAX_JOBCONTROLTYPE 5
#define MAX_TYPES 13
#define MAX_TYPESRANGE 8
#define MAX_RETURNCODE 10
#define MAX_CONDITIONCODE 14

#define ONEVALUEFIELDS 2
#define FRAMEFIELDS 5
#define RANGEFIELDS 6
#define ENUMERATIONFIELDS 5
#define ARRAYFIELDS 3
#define MESSAGEFIELDS 2
#define DAT_CAPABILITYFIELDS 3
#define DAT_CUSTOMDSDATAFIELDS 2
#define DAT_CIECOLORFIELDS 35
#define DAT_EVENTFIELDS 2
#define DAT_EXTIMAGEINFOFIELDS 2
#define DAT_GRAYRESPONSEFIELDS 1
#define DAT_IDENTITYFIELDS 12
#define DAT_IMAGEINFOFIELDS 17
#define DAT_IMAGELAYOUTFIELDS 7
#define DAT_IMAGEMEMXFERFIELDS 10
#define DAT_IMAGENATIVEXFERFIELDS 1
#define DAT_JPEGCOMPRESSIONFIELDS 9
#define DAT_PALETTE8FIELDS 3
#define DAT_PENDINGXFERSFIELDS 2
#define DAT_RGBRESPONSEFIELDS 1
#define DAT_SETUPFILEXFERFIELDS 3
#define DAT_SETUPMEMXFERFIELDS 3
#define DAT_STATUSFIELDS 2
#define DAT_USERINTERFACEFIELDS 2
#define DAT_XFERGROUPFIELDS 1
#define MAX_DATASTRUCTURE 29
extern TABLEDATSTRUCT DataStructure[MAX_DATASTRUCTURE];
extern char *gszFrameStructure[FRAMEFIELDS];

/* 
* Variable for the intialization of the combo box
*/
extern TABLEENTRY Destination[MAX_DEST];
extern TABLEENTRY DataGroup[MAX_DG];
extern TABLEENTRY DataType[MAX_DAT];
extern TABLEENTRY Message[MAX_MSG];
extern TABLEENTRY Capability[MAX_CAP];
extern TABLEENTRY ConType[MAX_CONTYPE];
extern TABLEENTRY DG_SourceManager[MAX_DGSM];
extern TABLEENTRY DG_Source[MAX_DGS];
extern TABLEENTRY DSM_DGControl[MAX_DSMDAT];
extern TABLEENTRY DS_DGControl[MAX_DSCONTROL];
extern TABLEENTRY DS_DGImage[MAX_DSIMAGE];
extern TABLEMSG MsgTable[MAX_DAT];
/*
* Variable for the initialization of the edit box
*/
extern TABLEENTRY PixelType[MAX_PIXELTYPE];
extern TABLEENTRY PaletteType[MAX_PALETTETYPE];
extern TABLEENTRY FormatType[MAX_FORMATTYPE];
extern TABLEENTRY CompressionType[MAX_COMPRESSIONTYPE];
extern TABLEENTRY VerLanguageType[MAX_VERLANGUAGETYPE];
extern TABLEENTRY VerContryType[MAX_VERCONTRYTYPE];
extern TABLEENTRY BitOrderType[MAX_BITORDERTYPE];
extern TABLEENTRY FilterType[MAX_FILTERTYPE];
extern TABLEENTRY LightPathType[MAX_LIGHTPATHTYPE];
extern TABLEENTRY LightSourceType[MAX_LIGHTSOURCETYPE];
extern TABLEENTRY OrientationType[MAX_ORIENTATIONTYPE];
extern TABLEENTRY PlanarChunkyType[MAX_PLANARCHUNKYTYPE];
extern TABLEENTRY  PixelFlavorType[MAX_PIXELFLAVORTYPE];
extern TABLEENTRY SetupXferType[MAX_SETUPXFERTYPE];
extern TABLEENTRY UnitsType[MAX_UNITSTYPE];
extern TABLEENTRY SupportedSizeType[MAX_SUPPORTEDSIZETYPE];
extern TABLECAP Caps[MAX_CAP];
extern TABLEENTRY Types[MAX_TYPES];
extern TABLEENTRY TypesRange[MAX_TYPESRANGE];
extern TABLEENTRY ReturnCode[MAX_RETURNCODE];
extern TABLEENTRY ConditionCode[MAX_CONDITIONCODE];
extern TABLEENTRY MSG_Identity[MAX_IDENTITY];

#endif //_inc_table_h
