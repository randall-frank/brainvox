
extern TW_INT16	AcqFlag;

#define VALID_HANDLE    32      // valid windows handle SB >= 32

void SetMessageLevel (int Level);
int MessageLevel(VOID);
void LogMessage(char *str);
void ShowRC_CC (HWND        hWndofApp,
                TW_UINT16   DisplayRC,     
                TW_UINT16   uiReturnCode,
                TW_UINT16   Dest,          
                char *      Text,           
                char *      Title);

void ShowTW_ID (HWND hWndofApp, TW_IDENTITY twID, char *text);

TW_BOOL MatchTwainInt(pTABLEENTRY pTable, TW_UINT32 uiTableSize,
	TW_INT32 uiCapId, LPSTR pString);

#define ML_NONE 	0
#define ML_ERROR	1
#define ML_INFO		2
#define ML_FULL 	3

/* prototypes for idl_twain_glue.c */
VOID TWInitialize (pTW_IDENTITY pIdentity, HWND hMainWnd);
BOOL TWOpenDSM (VOID);
BOOL TWCloseDSM (VOID);
BOOL TWIsDSMOpen (VOID);
BOOL TWOpenDS (VOID);
BOOL TWCloseDS (VOID);
BOOL TWEnableDS (TW_BOOL Show);
BOOL TWDisableDS (VOID);
BOOL TWIsDSOpen (VOID);
BOOL TWIsDSEnabled (VOID);
BOOL TWSelectDS (VOID);
VOID TWDefaultDS (VOID);
void CloseConnection(VOID);
BOOL ProcessTWMessage(LPMSG lpMsg, HWND hWnd, TW_UINT16 *finished, HGLOBAL *hGbl,
	TW_IMAGEINFO *info, float dpi[2],TW_INT16 *iPal,unsigned char *pal);
TW_INT16 TWXferMech(HWND hWnd,TW_INT16 use_mem);
HGLOBAL TWDoNativeTransfer(HWND hWnd);
TW_UINT16 CallDSMEntry(pTW_IDENTITY pApp, pTW_IDENTITY pSrc,
	TW_UINT32 DG, TW_UINT16 DAT, TW_UINT16 MSG, TW_MEMREF pData);
HGLOBAL TWDoMemTransfer(HWND hWnd,TW_IMAGEINFO *info,
			float dpi[2],TW_INT16 *iPal,unsigned char *pal);
float FIX32ToFloat (TW_FIX32 fix32);