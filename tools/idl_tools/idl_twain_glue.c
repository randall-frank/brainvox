
/*
 * Much of this code comes from the TWAIN Developers Toolkit
 *	See http://www.twain.org for details
 */

#include <windows.h>    // Note: twain.h also REQUIRES windows defs
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "twain.h"         // for TW data type defines
#include "twndebug.h"
#include "table.h"
#include "idl_twain_glue.h"

// Globals to this module ONLY
static BOOL TWDSMOpen = FALSE;    // glue code flag for an Open Source Manager
static HANDLE hDSMDLL = NULL;     // handle to Source Manager for explicit load
static HWND hWnd = NULL;                 // global for window handle
static BOOL TWDSOpen  = FALSE;    // glue code flag for an Open Source
static BOOL TWDSEnabled  = FALSE; // glue code flag for an Open Source
static TW_USERINTERFACE  twUI;	  // Fix as per Courisimault: 940518 - bd
static int MessageLevelVAR = ML_NONE;

// Globals
TW_IDENTITY appID, dsID;          // storage for App and DS (Source) states
DSMENTRYPROC lpDSM_Entry;         // entry point to the SM
TW_STATUS gGlobalStatus = {0, 0};

// local protos 
static HGLOBAL ConvertImage(HGLOBAL in,TW_INT32 *dim,TW_INT16 bpp,TW_IMAGEINFO info);


////////////////////////////////////////////////////////////////////////////
// FUNCTION: MessageLevel
//
// ARGS:    none
//
// RETURNS: current state of Source Manager (open/closed)
//
// NOTES:   Just a way to reduce the number of global vars
//
int MessageLevel (VOID)
{
	return (MessageLevelVAR);
}

////////////////////////////////////////////////////////////////////////////
// FUNCTION: SetMessageLevel
//
// ARGS:    none
//
// RETURNS: current state of Source Manager (open/closed)
//
// NOTES:   Just a way to reduce the number of global vars
//
void SetMessageLevel (int Level)
{
	MessageLevelVAR = Level;
	return;
}

/* Dummy function for now */

// Routine to reset for the default data source
VOID TWDefaultDS() 
{
	if (!TWIsDSOpen()) {
		dsID.Id = 0;
		dsID.ProductName[0] = 0;
		appID.Id = 0;   
	}
	return;
}

////////////////////////////////////////////////////////////////////////////
// FUNCTION: TWInitialize
//
// ARGS:    pIdentity information about the App
//          hMainWnd  handle to App main window
//
// RETURNS: none
//
// NOTES:   This simple copy to a static structure, appID, does not support
//          multiple connections. TODO, upgrade.
//
VOID TWInitialize (pTW_IDENTITY pIdentity, HWND hMainWnd)
{
	appID = *pIdentity;    // get copy of app info
	hWnd = hMainWnd;       // get copy of app window handle
	return;
} 

////////////////////////////////////////////////////////////////////////////
// FUNCTION: TWOpenDSM
//
// ARGS:    none
//
// RETURNS: current state of the Source Manager
//
// NOTES:     1). be sure SM is not already open
//            2). explicitly load the .DLL for the Source Manager
//            3). call Source Manager to:
//                - opens/loads the SM
//                - pass the handle to the app's window to the SM
//                - get the SM assigned appID.id field
//
BOOL TWOpenDSM (VOID)
{
	TW_UINT16     twRC = TWRC_FAILURE;
	OFSTRUCT      OpenFiles;
	#define       WINDIRPATHSIZE 160
	char          WinDir[WINDIRPATHSIZE];
	TW_STR32      DSMName;

	memset(&OpenFiles, 0, sizeof(OFSTRUCT));
	memset(WinDir, 0, sizeof(char[WINDIRPATHSIZE]));
	memset(DSMName, 0, sizeof(TW_STR32));

	// debug only
	if (TWDSMOpen == TRUE)
	{
		LogMessage("DSM already open\r\n");
	}

	// Only open SM if currently closed
	if (TWDSMOpen!=TRUE)
	{
		// Open the SM, Refer explicitly to the library so we can post a nice
		// message to the the user in place of the "Insert TWAIN.DLL in drive A:"
		// posted by Windows if the SM is not found.

		GetWindowsDirectory (WinDir, WINDIRPATHSIZE);

		lstrcpy (DSMName, "TWAIN_32.DLL");

		if (WinDir[strlen(WinDir)-1] != '\\')
		{
			lstrcat (WinDir, "\\");
		}
		lstrcat (WinDir, DSMName);

		if ((OpenFile(WinDir, &OpenFiles, OF_EXIST) != -1) &&
				(hDSMDLL =     LoadLibrary(DSMName)) != NULL &&
				(hDSMDLL >= (HANDLE)VALID_HANDLE) &&
				(lpDSM_Entry = (DSMENTRYPROC)GetProcAddress(hDSMDLL, MAKEINTRESOURCE (1))) != NULL)
		{
			// This call performs four important functions:
			//  	- opens/loads the SM
			//    	- passes the handle to the app's window to the SM
			//    	- returns the SM assigned appID.id field
			//    	- be sure to test the return code for SUCCESSful open of SM


			twRC = CallDSMEntry(&appID,
								NULL,
								DG_CONTROL,
								DAT_PARENT,
								MSG_OPENDSM,
								(TW_MEMREF)&hWnd);

			switch (twRC)
			{
				case TWRC_SUCCESS:
					// Needed for house keeping.  Do single open and do not
					// close SM which is not already open ....
					TWDSMOpen = TRUE;
					if (MessageLevel() >= ML_FULL)
					{
						ShowRC_CC(hWnd, 0, 0, 0, 
									"Source Manager was Opened successfully", 
									"TWAIN Information");
					}
					break;

				case TWRC_FAILURE:
					LogMessage("OpenDSM failure\r\n");
				default:
					// Trouble opening the SM, inform the user
					TWDSMOpen = FALSE;
					if (MessageLevel() >= ML_ERROR)                
					{
						ShowRC_CC(hWnd, 1, twRC, 0,	//Source Manager
									"",
									"DG_CONTROL/DAT_PARENT/MSG_OPENDSM");
					}
					break;
			}
		}
		else
		{
			if (MessageLevel() >= ML_ERROR)
			{
				ShowRC_CC(hWnd, 0, 0, 0,
							"Error in Open, LoadLibrary, or GetProcAddress.\nTwain DLL may not exist.",
							#ifdef WIN32
								"TWAIN_32.DLL");
							#else
								"TWAIN.DLL");
							#endif
			}
		}
	}  
	// Let the caller know what happened
	return (TWDSMOpen);
} 

////////////////////////////////////////////////////////////////////
// FUNCTION: TWCloseDSM
//
// ARGS:    none
//
// RETURNS: current state of Source Manager
//
// NOTES:    1). be sure SM is already open
//           2). call Source Manager to:
//               - request closure of the Source identified by appID info
//
//
BOOL TWCloseDSM ()
{
	TW_UINT16 twRC = TWRC_FAILURE;
	char buffer[80];

	memset(buffer, 0, sizeof(char[80]));

	if (!TWDSMOpen)
	{
		if (MessageLevel()  >= ML_ERROR)
		{
			ShowRC_CC(hWnd, 0, 0, 0,
						"Cannot Close Source Manager\nSource Manager Not Open", 
						"Sequence Error");
		}
	}
	else
	{
		if (TWDSOpen==TRUE)
		{
			if (MessageLevel()  >= ML_ERROR)
			{
				ShowRC_CC(hWnd, 0, 0, 0,
							"A Source is Currently Open", "Cannot Close Source Manager");
			}
		}
		else
		{
			// Only close something which is already open
			if (TWDSMOpen==TRUE)
			{
				// This call performs one important function:
				// - tells the SM which application, appID.id, is requesting SM to close
				// - be sure to test return code, failure indicates SM did not close !!

			twRC = CallDSMEntry(&appID,
								NULL,
								DG_CONTROL,
								DAT_PARENT,
								MSG_CLOSEDSM,
								&hWnd);

				if (twRC != TWRC_SUCCESS)
				{
					// Trouble closing the SM, inform the user
					if (MessageLevel() >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC, 0,
									"",
									"DG_CONTROL/DAT_PARENT/MSG_CLOSEDSM");
					}

					sprintf(buffer,"CloseDSM failure -- twRC = %d\r\n",twRC);
					LogMessage(buffer);
				}
				else
				{
					TWDSMOpen = FALSE;
					// Explicitly free the SM library
					if (hDSMDLL)
					{        
						FreeLibrary (hDSMDLL);
						hDSMDLL=NULL;
						// the data source id will no longer be valid after
						// twain is killed.  If the id is left around the
						// data source can not be found or opened
						dsID.Id = 0;  
					}
					if (MessageLevel() >= ML_FULL)
					{
						ShowRC_CC(hWnd, 0, 0, 0,
									"Source Manager was Closed successfully", 
									"TWAIN Information");
					}
				}
			}
		}
	}
	// Let the caller know what happened
	return (twRC==TWRC_SUCCESS);
} // TWCloseDSM

////////////////////////////////////////////////////////////////////////////
// FUNCTION: TWIsDSMOpen
//
// ARGS:    none
//
// RETURNS: current state of Source Manager (open/closed)
//
// NOTES:   Just a way to reduce the number of global vars and keep the
//          state of SM information local to this module.  Let the caller,
//          app, know current state of the SM.
//
BOOL TWIsDSMOpen (VOID)
{
	return (TWDSMOpen);
}

/////////////////////////////////////////////////////////////////////////
// FUNCTION: TWOpenDS
//
// ARGS:    none
//
// RETURNS: current state of select Source
//
// NOTES:    1). only attempt to open a source if it is currently closed
//           2). call Source Manager to:
//                - open the Source indicated by info in dsID
//                - SM will fill in the unique .Id field
//
BOOL TWOpenDS (VOID)
{
	TW_UINT16 twRC = TWRC_FAILURE;

	if (TWDSMOpen==FALSE)
	{
		LogMessage("DSM not open - cannot open DS\r\n");

		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(hWnd, 0, 0, 0,
						"Cannot Open Source\nSource Manager not Open",
						"TWAIN Error");
		}
	}
	else
	{
		//Source Manager is open
		if (TWDSOpen==TRUE)
		{
			LogMessage("Source already open\r\n");

			if (MessageLevel() >= ML_FULL)
			{
				ShowRC_CC(hWnd, 0, 0, 0,
							"Source is already open", "TWAIN Information");
			}
		}
		else
		{
			// This will open the Source.
			twRC = CallDSMEntry(&appID,
								NULL,
							DG_CONTROL,
							DAT_IDENTITY,
							MSG_OPENDS,
							&dsID);

			switch (twRC)
			{
				case TWRC_SUCCESS:
					LogMessage("OpenDS success\r\n");

					// do not change flag unless we successfully open
					TWDSOpen = TRUE;
					if (MessageLevel() >= ML_INFO)
					{
						ShowTW_ID(hWnd, dsID,
									"DG_CONTROL/DAT_IDENTITY/MSG_OPENDS TWRC_SUCCESS");
					}
					break;

				case TWRC_FAILURE:
					LogMessage("OpenDS failure\r\n");

					// Trouble opening the Source
					//Determine Condition Code
					if (MessageLevel()  >= ML_ERROR)                    
					{
						ShowRC_CC(hWnd, 1, twRC, 0, //Source Manager RC
									"",
									"DG_CONTROL/DAT_IDENTITY/MSG_OPENDS");
					}

					// this is a patch required in the event that TWAcquire
					// fails due to bad or non-existant during run of a
					// autotest. The tests are contnued via the WM_PAINT msg
					//InvalidateRect(NULL,NULL,TRUE);
					break;

				default:
					if (MessageLevel()  >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 0, 0, 0,
									"Unknown Return Code",
									"DG_CONTROL/DAT_IDENTITY/MSG_OPENDS");
					}
					break;
			}
		}
	}
	return TWDSOpen;
} 

///////////////////////////////////////////////////////////////////////////
// FUNCTION: TWCloseDS
//
// ARGS:    none
//
// RETURNS: none
//
// NOTES:    1). only attempt to close an open Source
//           2). call Source Manager to:
//                - ask that Source identified by info in dsID close itself
//
BOOL TWCloseDS (VOID)
{
	TW_UINT16 twRC = TWRC_FAILURE;

	if (!TWDSOpen)
	{
		if (MessageLevel()  >= ML_ERROR)
		{
			ShowRC_CC(hWnd, 0, 0, 0,
						"Cannot Close Source\nSource Not Open", 
						"Sequence Error");
		}
	}
	else
	{
		if (TWDSEnabled == TRUE)
		{
			if (MessageLevel()  >= ML_ERROR)
			{
				ShowRC_CC(hWnd, 0, 0, 0,
							"Source is Currently Enabled", "Cannot Close Source");
			}
		}
		else
		{
			if (TWDSOpen==TRUE)
			{
				// Close an open Source
				twRC = CallDSMEntry(&appID,
								NULL,
								DG_CONTROL,
								DAT_IDENTITY,
								MSG_CLOSEDS,
								&dsID);

				LogMessage("CloseDS\r\n");

				// show error on close
				if (twRC!= TWRC_SUCCESS) 
				{
					if (MessageLevel() >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC, 0,
									"",
									"DG_CONTROL/DAT_IDENTITY/MSG_CLOSEDS");
					}
				} 
				else 
				{
					TWDSOpen = FALSE;
					dsID.Id = 0;
					dsID.ProductName[0] = 0;
					if (MessageLevel() >= ML_FULL)
					{
						ShowRC_CC(hWnd, 0, 0, 0,
									"Source was Closed successfully", 
									"TWAIN Information");
					}
				}

			}
		}
	}
	return(twRC==TWRC_SUCCESS);
} // TWCloseDS

////////////////////////////////////////////////////////////////////////
// FUNCTION: TWEnableDS
//
// ARGS:    none
//
// RETURNS: BOOL for TRUE=open; FALSE=not open/fail
//
// NOTES:    1). only enable an open Source
//           2). call the Source Manager to:
//                - bring up the Source's User Interface
//
BOOL TWEnableDS (TW_BOOL Show)
{
	BOOL Result = FALSE;
	TW_UINT16 twRC = TWRC_FAILURE;

	if (TWDSOpen==FALSE)
	{
		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(hWnd, 0, 0, 0,
						"Cannot Enable Source\nNo Source Open", 
						"TWAIN Error");
		}
	}
	else
	{	
		// only enable open Source's
		if (TWDSEnabled==TRUE)	//Source is alredy enabled
		{
			if (MessageLevel() >= ML_ERROR)
			{
				ShowRC_CC(hWnd, 0, 0, 0,
							"Cannot Enable Source, already enabled", 
							"TWAIN Error");
			}
		}
		else
		{

			// This will display the Source User Interface. The Source should only display
			// a user interface that is compatible with the group defined
			// by appID.SupportedGroups (in our case DG_IMAGE | DG_CONTROL)
			memset(&twUI, 0, sizeof(TW_USERINTERFACE));
			twUI.hParent = hWnd;
			twUI.ShowUI  = Show;

			twRC = CallDSMEntry(&appID,
						&dsID,
						DG_CONTROL,
						DAT_USERINTERFACE,
						MSG_ENABLEDS,
						(TW_MEMREF)&twUI);

			if (twRC!=TWRC_SUCCESS)
			{
				if (MessageLevel() >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 1, twRC, 1,
								"",
								"DG_CONTROL/DAT_USERINTERFACE/MSG_ENABLEDS");
				}
			}
			else
			{
				Result = TRUE;
				TWDSEnabled = TRUE;
				if (MessageLevel() >= ML_FULL)
				{
					if(twUI.ShowUI == TRUE)
					{
						ShowRC_CC(hWnd, 0, 0, 0,
									"Source Enabled",
									"DG_CONTROL/DAT_USERINTERFACE/MSG_ENABLEDS");
					}
				}
			}
		}
	}
	return (Result);
} 

////////////////////////////////////////////////////////////////////////////
// FUNCTION: TWDisableDS
//
// ARGS:    none
//
// RETURNS: twRC
//
// NOTES:    1). only disable an open Source
//           2). call Source Manager to:
//                - ask Source to hide it's User Interface
//
BOOL TWDisableDS (VOID)
{
	TW_UINT16 twRC = TWRC_FAILURE;
	TW_USERINTERFACE twUI;

	memset(&twUI, 0, sizeof(TW_USERINTERFACE));

	LogMessage("Disable DS\r\n");

	// only disable enabled Source's
	if (TWDSEnabled!=TRUE)
	{
		if (MessageLevel()  >= ML_ERROR)
		{
			ShowRC_CC(hWnd, 0, 0, 0,
						"Cannot Disable Source\nSource Not Enabled", 
						"Sequence Error");
		}
	}
	else
	{
		twUI.hParent = hWnd;
		twUI.ShowUI = TWON_DONTCARE8;

		twRC = CallDSMEntry(&appID,
					&dsID,
					DG_CONTROL,
					DAT_USERINTERFACE,
					MSG_DISABLEDS,
					(TW_MEMREF)&twUI);

		if (twRC == TWRC_SUCCESS)
		{   
			LogMessage("DS Disabled\r\n");

			TWDSEnabled = FALSE;
			if (MessageLevel() >= ML_FULL)
			{
				ShowRC_CC(hWnd, 0, 0, 0,
							"Source Disabled",
							"DG_CONTROL/DAT_USERINTERFACE/MSG_DISABLEDS");
			}
		}
		else
		{
			if (MessageLevel() >= ML_ERROR)
			{
				ShowRC_CC(hWnd, 1, twRC, 1,
							"",
							"DG_CONTROL/DAT_USERINTERFACE/MSG_DISABLEDS");
			}
		}
	}    	
	return (twRC==TWRC_SUCCESS);
} 

/////////////////////////////////////////////////////////////////////////
// FUNCTION: TWIsDSOpen
//
// ARGS:    none
//
// RETURNS: current state of the Source
//
// NOTES:    Just a way to reduce the number of global vars and keep the
//           state of Source information local to this module.  Let the caller,
//           app, know current state of the Source.
//
BOOL TWIsDSOpen (VOID)
{
	return (TWDSOpen);
} 

///////////////////////////////////////////////////////////////////////////
// FUNCTION: TWIsDSEnabled
//
// ARGS:    none
//
// RETURNS: current state of Source (Enabled?)
//
// NOTES:   Just a way to reduce the number of global vars and keep the
//          state of SM information local to this module.  Let the caller,
//          app, know current state of the SM.
//
BOOL TWIsDSEnabled (VOID)
{
	return (TWDSEnabled);
}

//////////////////////////////////////////////////////////////////////////
// FUNCTION: TWSelectDS
//
// ARGS:    none
//
// RETURNS: twRC TWAIN status return code
//
// NOTES:   1). call the Source Manager to:
//              - have the SM put up a list of the available Sources
//              - get information about the user selected Source from
//                NewDSIdentity, filled by Source
//
BOOL TWSelectDS (VOID)
{
	TW_UINT16 twRC = TWRC_FAILURE;
	TW_IDENTITY NewDSIdentity;

	memset(&NewDSIdentity, 0, sizeof(TW_IDENTITY));

	if (TWDSOpen)
	{
		LogMessage("TWSelectDS -- source already open\r\n");

		//A Source is already open
		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(hWnd, 0, 0, 0,
						"A Source is already open\nClose Source before Selecting a New Source",
						"DG_CONTROL/DAT_IDENTITY/MSG_USERSELECT");
		}
		twRC = TWRC_FAILURE;
	}
	else
	{
		// I will settle for the system default.  Shouldn't I get a highlight
		// on system default without this call?

		twRC = CallDSMEntry(&appID,
					NULL,
					DG_CONTROL,
					DAT_IDENTITY,
					MSG_GETDEFAULT,
					(TW_MEMREF)&NewDSIdentity);

		// This call performs one important function:
		// - should cause SM to put up dialog box of available Source's
		// - tells the SM which application, appID.id, is requesting, REQUIRED
		// - returns the SM assigned NewDSIdentity.id field, you check if changed
		//  (needed to talk to a particular Data Source)
		// - be sure to test return code, failure indicates SM did not close !!
		//
		twRC = CallDSMEntry(&appID,
						NULL,
						DG_CONTROL,
						DAT_IDENTITY,
						MSG_USERSELECT,
						(TW_MEMREF)&NewDSIdentity);

		// Check if the user changed the Source and react as apporpriate.
		// - TWRC_SUCCESS, log in new Source
		// - TWRC_CANCEL,  keep the current Source
		// - default,      check down the codes in a status message, display result
		//

		switch (twRC)
		{
			case TWRC_SUCCESS:
				dsID = NewDSIdentity; 
				if (MessageLevel() >= ML_INFO)
				{
					ShowTW_ID(hWnd, dsID,
								"DG_CONTROL/DAT_IDENTITY/MSG_USERSELECT");
				}
				break;
			case TWRC_CANCEL:
				if (MessageLevel() >= ML_INFO)
				{
					ShowRC_CC(hWnd, 1, twRC, 0,
								"",
								"DG_CONTROL/DAT_IDENTITY/MSG_USERSELECT");
				}
				break;
			case TWRC_FAILURE:	        
			default:
				if (MessageLevel() >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 1, twRC, 0,
								"",
								"DG_CONTROL/DAT_IDENTITY/MSG_USERSELECT");
				}
				break;
		}
	}
	return (twRC);
}  // TWSelectDS

/*
* Fucntion: CallDSMEntry
* Author:	Nancy Letourneau / J.F.L. Peripherals Inc.
* Input:  
*		Function - 
*		pApp - 
*		pSrc - 
*		DG -
*		DAT -
*		MSG -
*		pData -
* Output: 
*		TW_UINT16 - Value of Item field of container. 
* Comments:
*
*/
TW_UINT16 CallDSMEntry(pTW_IDENTITY pApp, pTW_IDENTITY pSrc,
	TW_UINT32 DG, TW_UINT16 DAT, TW_UINT16 MSG, TW_MEMREF pData)
{
	TW_UINT16 twRC = (*lpDSM_Entry)(pApp, pSrc, DG, DAT, MSG, pData);

	if((twRC != TWRC_SUCCESS)&&(DAT!=DAT_EVENT))
	{
		VERIFY((*lpDSM_Entry)(pApp, pSrc, DG_CONTROL, DAT_STATUS, MSG_GET, 
					(TW_MEMREF)&gGlobalStatus) == TWRC_SUCCESS);
		TRACE("CallDSMEntry function: call failed with RC = %d, CC = %d.\n", 
					twRC, gGlobalStatus.ConditionCode);
	}
	return twRC;
}

///////////////////////////////////////////////////////////////////////////
// function: ShowRC_CC -- 
// Author: TWAIN Working Group
// Input:
//		hWndofApp - handle to window
//		DisplayRC -
//		ReturnCode - 
//		Dest - 
//		Text - 
//		Title - 
// Output: none
// Comments:
//		Main message notification routine.  Displays the failure, condition
// codes and message associated with the problem.  Additionally, all log messages
// pass thru this function.
//
void ShowRC_CC (HWND        hWndofApp,
                TW_UINT16   DisplayRC,      //0-No, 1-Yes
                TW_UINT16   uiReturnCode,
                TW_UINT16   Dest,           //0-Source Manager or 1-Source 
                char *      Text,           //Additional Information        
                char *      Title)          //Triplet or Cap
{
	char            Details[255];
	char            Details2[1024];
	char outstring[1024];
	char outtitle[255];

	memset(Details, 0, sizeof(char[255]));
	memset(Details2, 0, sizeof(char[1024]));
	memset(outstring, 0, sizeof(char[1024]));
	memset(outtitle, 0, sizeof(char[255]));

	strcpy(Details, "");
	strcpy(Details2, "");                         

	lstrcpy(outstring,Text);
	lstrcpy(outtitle,Title);

	// if display check and display condition code
	if(DisplayRC == 1)  
	{
		//Get Return Code to be displayed, place in Details2
		MatchTwainInt(ReturnCode, MAX_RETURNCODE, (TW_INT32)uiReturnCode, 
									Details2);

		MatchTwainInt(ConditionCode,MAX_CONDITIONCODE, 
									(TW_INT32)gGlobalStatus.ConditionCode, 
									Details);

		lstrcpy(outtitle,"Operation Status");

		lstrcpy(outstring,"Operation: ");
		lstrcat(outstring,Title);
		lstrcat(outstring,"\r\n");
		lstrcat(outstring,"Return Code: ");
		lstrcat(outstring,Details2);
		lstrcat(outstring,"\r\n");
		lstrcat(outstring,"Condition Code: ");
		lstrcat(outstring,Details);
	}   

	MessageBox (hWndofApp, outstring, outtitle, MB_OK);
	return;
} 

///////////////////////////////////////////////////////////////////////////
// Function: ShowTW_ID --
// Author: TWAIN Working Group
// Input:
//		hWndofApp - handle to window
//		twID - 
//		text - pointer on
// Output: none
// Comments:
//
void ShowTW_ID (HWND hWndofApp, TW_IDENTITY twID, char *text)
{
	char Details [255];
	char Details2 [1024];

	memset(Details, 0, sizeof(char[255]));
	memset(Details2, 0, sizeof(char[1024]));

	ASSERT(hWndofApp);
	ASSERT(text);

	sprintf(Details2, "\n%s %d", "Id:", twID.Id);
	sprintf (Details, "\n%s %d", "TW_VERSION.MajorNum:", twID.Version.MajorNum);
	strcat(Details2, Details);
	sprintf (Details, "\n%s %d", "TW_VERSION.MinorNum:", twID.Version.MinorNum);
	strcat(Details2, Details);
	sprintf (Details, "\n%s %d", "TW_VERSION.Language:", twID.Version.Language);
	strcat(Details2, Details);
	sprintf (Details, "\n%s %d", "TW_VERSION.Country:", twID.Version.Country);
	strcat(Details2, Details);
	sprintf (Details, "\n%s %s", "TW_VERSION.Info:", twID.Version.Info);
	strcat(Details2, Details);
	sprintf (Details, "\n%s %d", "ProtocolMajor:", twID.ProtocolMajor);
	strcat(Details2, Details);
	sprintf (Details, "\n%s %d", "ProtocolMinor:", twID.ProtocolMinor);
	strcat(Details2, Details);
	sprintf (Details, "\n%s %d", "SupportedGroups:", twID.SupportedGroups);
	strcat(Details2, Details);
	sprintf (Details, "\n%s %s", "Manufacturer:", twID.Manufacturer);
	strcat(Details2, Details);
	sprintf (Details, "\n%s %s", "ProductFamily:", twID.ProductFamily);
	strcat(Details2, Details);
	sprintf (Details, "\n%s %s", "ProductName:", twID.ProductName);
	strcat(Details2, Details);

	ShowRC_CC(hWndofApp,0,0,0,Details2,text);
	return;
}

/*
* Function: MatchTwainInt
* Author: TWAIN Working Group
* Input:
*		pTable - Pointer to a Table entry that contain the value for the initialization
*		uiTableSize - Maximum of item in table 
*		uiCap -	ID for the current capability 
*		pString - 
* Output:
*		TW_BOOL -	TRUE is successful
* Comments:
*/
TW_BOOL MatchTwainInt(pTABLEENTRY pTable, TW_UINT32 uiTableSize,
	TW_INT32 uiCapId, LPSTR pString)

{
	TW_BOOL result = FALSE;
	TW_UINT16 i = 0;

	ASSERT(pTable);
	ASSERT(pString);

	for(i = 0; i < uiTableSize; i++)
	{
		if (pTable[i].ItemId == uiCapId)
		{
			lstrcpy(pString, pTable[i].pszItemName);
			result = TRUE;
			break;
		}   
	}   

	return  result;
}   


/*
* Function: TWXferMech -- 
* Author: TWAIN Working Group
* Input:
*		hWnd - handle to window
* Output:
*		TW_INT16
* Comments:
*		Set the current transfer mechanism for Twain based on the param
*/
TW_INT16 TWXferMech(HWND hWnd,TW_INT16 use_mem)

{
	TW_CAPABILITY   cap;
	pTW_ONEVALUE    pval = NULL;
	TW_INT16        status = TWRC_FAILURE;  

	/*
	*	Initialize all structures
	*/
	memset(&cap, 0, sizeof(TW_CAPABILITY));

	ASSERT(hWnd);

	LogMessage("TWXferMech\r\n");

	cap.Cap = ICAP_XFERMECH;
	cap.ConType = TWON_ONEVALUE;

	/*
	* alloc the container
	*/
	if (cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE)))
	{
		pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
		pval->ItemType = TWTY_UINT16;
		pval->Item = TWSX_NATIVE;
		if (use_mem) pval->Item = TWSX_MEMORY;

		GlobalUnlock(cap.hContainer);

		status = CallDSMEntry(&appID,
						&dsID,
						DG_CONTROL, 
						DAT_CAPABILITY, 
						MSG_SET,
						(TW_MEMREF)&cap);

		GlobalFree((HANDLE)cap.hContainer);

		if (status != TWRC_SUCCESS)
		{
			if (MessageLevel() >= ML_ERROR)
			{
				ShowRC_CC(hWnd, 1, status, 1, "","MSG_SET of ICAP_XFERMECH"); 
			}
		} 
	}
	else
	{
		status = TWRC_FAILURE;
		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(hWnd, 0, 0, 0, "Memory Allocation Failed","MSG_SET of ICAP_XFERMECH"); 
		}
	}
	return status;
}  

////////////////////////////////////////////////////////////////////////////
// FUNCTION: ProcessTWMessage
//
//
// ARGS:     lpMsg  Pointer to Windows msg retrieved by GetMessage
//           hWnd   Application's main window handle
//
// RETURNS: TRUE  if application should process message as usual
//              FALSE if application should skip processing of this message
//
// NOTES:   1). be sure both Source Manager and Source are open
//              2). two way message traffic:
//                  - relay windows messages down to Source's modeless dialog
//                  - retrieve TWAIN messages from the Source
//
// COMMENT: ProcessSourceMessage is designed for applications that can only
// have one Source open at a time.  If you wish your application to have more
// than one Source open at a time please consult the TWAIN ToolKit for
// event handling instructions.
//

BOOL ProcessTWMessage(LPMSG lpMsg, HWND hWnd, TW_UINT16 *finished, HGLOBAL *hGbl,
	TW_IMAGEINFO *info, float dpi[2],TW_INT16 *iPal,unsigned char *pal)
{
	TW_UINT16  twRC = TWRC_NOTDSEVENT;
	TW_EVENT   twEvent;

	*finished = 0;

	/* standard event processing */
	memset(&twEvent, 0, sizeof(TW_EVENT));

	ASSERT(lpMsg);
	ASSERT(hWnd);
	// Only ask Source Manager to process event if there is a Source connected.

	if ((TWIsDSMOpen()) && (TWIsDSOpen()))
	{
		// A Source provides a modeless dialog box as its user interface.
		// The following call relays Windows messages down to the Source's
		// UI that were intended for its dialog box.  It also retrieves TWAIN
		// messages sent from the Source to our  Application.
		//

		twEvent.pEvent = (TW_MEMREF)lpMsg;

		twRC = CallDSMEntry(&appID,
						&dsID, 
						DG_CONTROL, 
						DAT_EVENT,
						MSG_PROCESSEVENT, 
						(TW_MEMREF)&twEvent);

		switch (twEvent.TWMessage)
		{
			case MSG_XFERREADY:
				//If AcqFlag >0 then we are in state 5
				if (AcqFlag) {
					if (info) {
						*hGbl = TWDoMemTransfer(hWnd,info,dpi,iPal,pal);
					} else {
						*hGbl = TWDoNativeTransfer(hWnd);
					}
					*finished = 1; 
				} else if (MessageLevel() >= ML_ERROR) {
					ShowRC_CC(hWnd, 0, 0, 0, 
								"Received while not in state 5", 
								"MSG_XFERREADY");
					*finished = -1;
				}
				break;

			case MSG_CLOSEDSREQ:
				*finished = 3;
				break;

			case MSG_CLOSEDSOK:
				*finished = 2;
				break;
			
		// No message from the Source to the App break;
		// possible new message
			case MSG_NULL:
			default:
				break;
		}   
	} 
	// tell the caller what happened
	return (twRC==TWRC_DSEVENT);           // returns TRUE or FALSE
} 

/*
* Function: DoNativeTransfer -- 
* Author: TWAIN Working Group
* Input:     
*		hWnd - Handle to the Window
* Output: none
* Comment: 
*/
HGLOBAL TWDoNativeTransfer(HWND hWnd)
{
	TW_PENDINGXFERS     twPendingXfer;
	TW_UINT16           twRC = TWRC_FAILURE;
	TW_UINT16           twRC2 = TWRC_FAILURE;
	TW_UINT32           hBitMap = 0;
	HGLOBAL             hbm_acq = NULL;     // handle to bit map from Source to ret to App
	char buffer[2048];

	LPBITMAPINFOHEADER lpdib = NULL;

	memset(&twPendingXfer, 0, sizeof(TW_PENDINGXFERS));
	memset(buffer, 0, sizeof(char[2048]));

	ASSERT(hWnd);

	/*
	* Do until there are no more pending transfers
	* explicitly initialize the our flags
	*/
	twPendingXfer.Count = 0;
	do 
	{
		/*
		* Initiate Native Transfer
		*/
		twRC = CallDSMEntry(&appID,
						&dsID, 
						DG_IMAGE,
						DAT_IMAGENATIVEXFER, 
						MSG_GET, 
						(TW_MEMREF)&hBitMap);

		switch (twRC)
		{
			case TWRC_XFERDONE:  // Session is in State 7
				if (MessageLevel() >= ML_FULL)
				{
					ShowRC_CC(hWnd, 0, 0, 0, "TWRC_XFERDONE", 
							"DG_IMAGE/DAT_IMAGENATIVEXFER/MSG_GET");
				}                
				hbm_acq = (HBITMAP)hBitMap;

				/*
				* Acknowledge the end of the transfer 
				* and transition to state 6/5
				*/
				twRC2 = CallDSMEntry(&appID,
									&dsID, 
									DG_CONTROL,
									DAT_PENDINGXFERS, 
									MSG_ENDXFER,
									(TW_MEMREF)&twPendingXfer);

				if (twRC2 != TWRC_SUCCESS)
				{
					if (MessageLevel()  >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC2, 1, 
									"", 
									"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");
					}
				}

				sprintf(buffer,"Pending Xfers = %d\r\n",twPendingXfer.Count);
				LogMessage(buffer);

				/* drop all but the last one on the floor (for now) */
				if (twPendingXfer.Count != 0) 
				{
					GlobalUnlock(hbm_acq);
					GlobalFree(hbm_acq);
					hbm_acq = NULL;
				}

				break;

			/*
			* the user canceled or wants to rescan the image
			* something wrong, abort the transfer and delete the image
			* pass a null ptr back to App
			*/
			case TWRC_CANCEL:   // Session is in State 7
				if (MessageLevel()  >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 0, 0, 0, "TWRC_CANCEL", 
								"DG_IMAGE/DAT_IMAGENATIVEXFER/MSG_GET");                
				}

				/*
				* Source (or User) Canceled Transfer
				* transistion to state 6/5
				*/
				twRC2 = CallDSMEntry(&appID,
								&dsID, 
								DG_CONTROL,
								DAT_PENDINGXFERS, 
								MSG_ENDXFER,
								(TW_MEMREF)&twPendingXfer);

				if (twRC2 != TWRC_SUCCESS)
				{
					if (MessageLevel()  >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC2, 1, 
									"", 
									"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
					}
				}
/*
				if (twPendingXfer.Count == 0)
				{
					 again!! CloseConnection(); 
				}
*/
				break;

			case TWRC_FAILURE:  //Session is in State 6
				/*
				* The transfer failed
				*/
				if (MessageLevel()  >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 1, TWRC_FAILURE, 1, 
								"", "DG_IMAGE/DAT_IMAGENATIVEXFER/MSG_GET");                
				}

				/*
				* Abort the image
				* Enhancement: Check Condition Code and attempt recovery
				*/
				twRC2 = CallDSMEntry(&appID,
								&dsID, 
								DG_CONTROL,
								DAT_PENDINGXFERS, 
								MSG_ENDXFER,
								(TW_MEMREF)&twPendingXfer); 

				if (twRC2 != TWRC_SUCCESS)
				{
					if (MessageLevel()  >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC2, 1, 
									"", 
									"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
					}
				}
/*
				if (twPendingXfer.Count == 0)
				{
					 CloseConnection();  
				}
*/
				break;

			default:    //Sources should never return any other RC
				if (MessageLevel()  >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 0, 0, 0, "Unknown Return Code", 
								"DG_IMAGE/DAT_IMAGENATIVEXFER/MSG_GET");
				}

				/*
				* Abort the image
				* Enhancement: Check Condition Code and attempt recovery instead
				*/
				twRC2 = CallDSMEntry(&appID,
								&dsID, 
								DG_CONTROL,
								DAT_PENDINGXFERS, 
								MSG_ENDXFER,
								(TW_MEMREF)&twPendingXfer);

				if (twRC2 != TWRC_SUCCESS)
				{
					if (MessageLevel()  >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC2, 1, 
									"", 
									"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");
					}
				}
/*
				if (twPendingXfer.Count == 0)
				{
				    CloseConnection(); 
				}
*/
				break;
		}   

	} while (twPendingXfer.Count != 0);

	/* AcqFlag = 0; why? */

	return(hbm_acq);
}   

void CloseConnection()
{
	if (TWIsDSEnabled()) TWDisableDS();
	if (TWIsDSOpen()) TWCloseDS();
	if (TWIsDSMOpen()) TWCloseDSM();

	AcqFlag = 0;

	return;
}

/*
* Function: DoMemTransfer -- 
* Author: TWAIN Working Group
* Input:     
*		hWnd - Handle to the Window
* Output: none
* Comment: 
*/
HGLOBAL TWDoMemTransfer(HWND hWnd,TW_IMAGEINFO *info,
			float dpi[2],TW_INT16 *iPal,unsigned char *pal)
{
	TW_PENDINGXFERS		twPendingXfer;
	TW_UINT16           twRC2 = TWRC_FAILURE;
	TW_UINT16           twRC = TWRC_FAILURE;
	HANDLE              hbm_acq = NULL;     // handle to raw data from Source to ret to App
	TW_IMAGEMEMXFER     xfer;
	TW_SETUPMEMXFER     setup;
	TW_PALETTE8         tpal;
	unsigned char TW_HUGE  *ptr = NULL;
	TW_UINT16           index = 0;
	TW_UINT32           size = 0;
	TW_CAPABILITY       cap;
	TW_UINT16           PixelFlavor = 0;   
	pTW_ONEVALUE        pOneV = NULL;
	TW_UINT16           Units = 0;
	float               XRes = 0, YRes = 0;
	char                buffer[2048];
	int                 blocks = 0;

	memset(&twPendingXfer, 0, sizeof(TW_PENDINGXFERS));
	memset(&xfer, 0, sizeof(TW_IMAGEMEMXFER));
	memset(&setup, 0, sizeof(TW_SETUPMEMXFER));
	memset(info, 0, sizeof(TW_IMAGEINFO));
	memset(&tpal, 0, sizeof(TW_PALETTE8));
	memset(&cap, 0, sizeof(TW_CAPABILITY));
	memset(buffer, 0, sizeof(char[2048]));

	ASSERT(hWnd);

   /*
	* Do until there are no more pending transfers
	* explicitly initialize the our flags
	*/
	twPendingXfer.Count = 0;
	do
	{
        
		twRC = CallDSMEntry(&appID,
						&dsID, 
						DG_IMAGE, 
						DAT_IMAGEINFO,
						MSG_GET, 
						(TW_MEMREF)info);

		if (twRC != TWRC_SUCCESS)
		{  
			if (MessageLevel() >= ML_ERROR)
			{
				ShowRC_CC(hWnd, 1, twRC, 1,
							"Memory Transfer",
							"DG_IMAGE/DAT_IMAGEINFO/MSG_GET");              
			}
		}
		else
		{  
			*iPal = 0;
			size = (((((TW_INT32)info->ImageWidth*info->BitsPerPixel+31)/32)*4)
							* info->ImageLength);

			/*
			* make the size an integral of the preferred transfer size
			*/
			twRC = CallDSMEntry(&appID,
							&dsID, 
							DG_CONTROL,
							DAT_SETUPMEMXFER, 
							MSG_GET, 
							(TW_MEMREF)&setup);

			blocks = (int)(size / setup.Preferred);
			size = (blocks +1) * setup.Preferred;

			hbm_acq = GlobalAlloc(GHND,size);

			if (hbm_acq == NULL)
			{   
				/*
				* GlobalAlloc Failed
				*/
				if (MessageLevel() >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 0, 0, 0,
								"GlobalAlloc Failed in DoMemTransfer",
								"Memory Error");
				}
			}
			else
			{
	    		/*
				* Get Units and calculate PelsPerMeter
				*/
				cap.Cap = ICAP_UNITS;
				cap.ConType = TWON_DONTCARE16;
				cap.hContainer = NULL;
                
				twRC = CallDSMEntry(&appID,
								&dsID, 
								DG_CONTROL, 
								DAT_CAPABILITY, 
								MSG_GETCURRENT, 
								(TW_MEMREF)&cap);

				if (twRC != TWRC_SUCCESS)
				{
					if (MessageLevel() >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC, 1,
									"ICAP_UNITS",
									"DG_CONTROL/DAT_CAPABILITY/MSG_GETCURRENT");
					}
					dpi[0] = 99;
					dpi[1] = 99;
				}
				else
				{
					pOneV = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
					Units = (TW_UINT16)(pOneV->Item);
					GlobalUnlock(cap.hContainer);
					GlobalFree((HANDLE)cap.hContainer);

					XRes = FIX32ToFloat(info->XResolution);
					YRes = FIX32ToFloat(info->YResolution);
                    
					switch(Units)
					{
						case TWUN_INCHES:
							dpi[0] = XRes;
							dpi[1] = YRes;
							break;
						case TWUN_CENTIMETERS:
							dpi[0] = XRes*2.54f;
							dpi[1] = YRes*2.54f;
							break;
						case TWUN_PICAS:
						case TWUN_POINTS:
						case TWUN_TWIPS:
						case TWUN_PIXELS:
						default:
							dpi[0] = 99;
							dpi[1] = 99;
							break;
					}
				}
                
				/*
				* Setup Palette -- if the palettes are B/W or shades of gray, the color
				* table is built here.  If the image is 8 bit color, a call to the
				* source is made to retrieve the correct set of colors.  If the call 
				* fails, the color table is constructed with 256 shades of gray inorder
				* to provide some image reference
				*/
				switch (info->PixelType)
				{
					case TWPT_BW:                                                       
						/*
						* Get CAP_PIXELFLAVOR to determine colors
						* fill in the palette information
						*/
						cap.Cap = ICAP_PIXELFLAVOR;
						cap.ConType = TWON_DONTCARE16;
						cap.hContainer = NULL;

						twRC = CallDSMEntry(&appID,
										&dsID, 
										DG_CONTROL, 
										DAT_CAPABILITY, 
										MSG_GETCURRENT, 
										(TW_MEMREF)&cap);

						if (twRC != TWRC_SUCCESS)
						{   
							if (MessageLevel() >= ML_INFO)
							{
								ShowRC_CC(hWnd, 1, twRC, 1,
											"MSG_GET",
											"ICAP_PIXELFLAVOR");
							}
							PixelFlavor = TWPF_CHOCOLATE;
						}
						else
						{
							if (cap.ConType != TWON_ONEVALUE)
							{
								PixelFlavor = TWPF_CHOCOLATE;
							}
							else
							{                            
								pOneV = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
								PixelFlavor = (TW_UINT16)(pOneV->Item);
								GlobalUnlock(cap.hContainer);
							}                       
							GlobalFree((HANDLE)cap.hContainer);
						}
						if (PixelFlavor == 0)
						{
							/*
							* 0=Black
							*/
							pal[0] = 0;
							pal[256] = 0;
							pal[512] = 0;

							pal[1] = 255;
							pal[257] = 255;
							pal[513] = 255;
						}
						else
						{
							/*
							* ICAP_PIXELFLAVOR == 1
							* 0=White
							*/
							pal[1] = 0;
							pal[257] = 0;
							pal[513] = 0;

							pal[0] = 255;
							pal[256] = 255;
							pal[512] = 255;
						}
						*iPal = 2;
						break;
                    
					case TWPT_GRAY:
						for (index=0; index<256; index++)
						{
							pal[index] = (BYTE)index;
							pal[index+256] = (BYTE)index;
							pal[index+512] = (BYTE)index;
						}
						*iPal = 256;
						break;

					case TWPT_RGB:
						*iPal = 0;
						break;
                    
					case TWPT_PALETTE:
					case TWPT_CMY:
					case TWPT_CMYK:
					case TWPT_YUV:
					case TWPT_YUVK:
					case TWPT_CIEXYZ:
					default:
						/*
						* fill in the palette information
						*/
						twRC = CallDSMEntry(&appID,
										&dsID, 
										DG_IMAGE, 
										DAT_PALETTE8, 
										MSG_GET, 
										(TW_MEMREF)&tpal);

						if (twRC != TWRC_SUCCESS)
						{
							if (MessageLevel() >= ML_ERROR)
							{
								ShowRC_CC(hWnd, 1, twRC, 1,
											"",
											"DG_IMAGE/DAT_PALETTE8/MSG_GET -- defaulting to 256 gray image palette");
							}
							*iPal = 256;
							for (index=0; index<256; index++)
							{
								pal[index] = (BYTE)index;
								pal[index+256] = (BYTE)index;
								pal[index+512] = (BYTE)index;
							}   
						}
						else
						{
							*iPal = tpal.NumColors;
							for (index=0; index<tpal.NumColors; index++)
							{
								pal[index] = tpal.Colors[index].Channel1;
								pal[index+256] = tpal.Colors[index].Channel2;
								pal[index+512] = tpal.Colors[index].Channel3;
							}   
						}
						break;              
				}   //end switch(PixelType)

				/*
				* locate the start of the buffer 
				*/
				ptr = (unsigned char TW_HUGE *)GlobalLock(hbm_acq);

				/*
				* determine the buffer size 
				*/
				twRC = CallDSMEntry(&appID,
								&dsID, 
								DG_CONTROL,
								DAT_SETUPMEMXFER, 
								MSG_GET, 
								(TW_MEMREF)&setup);

				if (twRC != TWRC_SUCCESS)
				{
					if (MessageLevel() >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC, 1,
									"",
									"DG_CONTROL/DAT_SETUPMEMXFER/MSG_GET");
					}
				}
				else
				{
					/*
					* we will use a pointer to shared memory
					*/
					xfer.Memory.Flags = TWMF_APPOWNS | TWMF_POINTER;
					xfer.Memory.Length = setup.Preferred;
					xfer.Memory.TheMem = ptr;

					/*
					* transfer the data -- loop until done or canceled 
					*/
					do
					{
						twRC = CallDSMEntry(&appID,
										&dsID, 
										DG_IMAGE,
										DAT_IMAGEMEMXFER, 
										MSG_GET, 
										(TW_MEMREF)&xfer);

						switch (twRC)
						{
							case TWRC_SUCCESS:
								ptr += xfer.BytesWritten;
								xfer.Memory.TheMem = ptr;
								break;
    
							case TWRC_XFERDONE:
								/*
								* Successful Transfer
								*/
								if (MessageLevel() >= ML_FULL)
								{
									ShowRC_CC(hWnd, 0, 0, 0,
												"TWRC_XFERDONE",
												"DG_IMAGE/DAT_IMAGEMEMXFER/MSG_GET");               
								}
								GlobalUnlock(hbm_acq);

								/*
								* Acknowledge the end of the transfer
								* and transition to state 6/5
								*/
								twRC2 = CallDSMEntry(&appID,
												&dsID, 
												DG_CONTROL,
												DAT_PENDINGXFERS, 
												MSG_ENDXFER,
												(TW_MEMREF)&twPendingXfer);

								if (twRC2 != TWRC_SUCCESS)
								{
									if (MessageLevel()  >= ML_ERROR)
									{
										ShowRC_CC(hWnd, 1, twRC2, 1, 
													"", 
													"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
									}
								}

								/* drop all but the last one on the floor (for now) */
								if (twPendingXfer.Count != 0) 
								{
									GlobalFree(hbm_acq);
									hbm_acq = NULL;
								}

								/*
								* showRC_CC is a safe operation here since there will be no triplet
								* calls generated 
								*/
								if (MessageLevel() >= ML_INFO)
								{
									wsprintf(buffer,"Images = %d",twPendingXfer.Count);
									ShowRC_CC(NULL,0,0,0,buffer,"Pending Transfers");
								}
								break;
    
							case TWRC_CANCEL:

								/*
								* The Source is in state 7
								* transistion to state 6/5
								*/
								if (MessageLevel() >= ML_ERROR)
								{
									ShowRC_CC(hWnd, 0, 0, 0,
												"TWRC_CANCEL",
												"DG_IMAGE/DAT_IMAGEMEMXFER/MSG_GET");               
								}

								twRC2 = CallDSMEntry(&appID,
											&dsID, 
											DG_CONTROL,
											DAT_PENDINGXFERS, 
											MSG_ENDXFER,
											(TW_MEMREF)&twPendingXfer);

								if (twRC2 != TWRC_SUCCESS)
								{
									if (MessageLevel()  >= ML_ERROR)
									{
										ShowRC_CC(hWnd, 1, twRC2, 1, 
													"", 
													"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
									}
								}
        
								GlobalUnlock(hbm_acq);
								GlobalFree(hbm_acq);
								hbm_acq = NULL;
								break;
                            
							case TWRC_FAILURE:

								/*
								* The transfer failed
								* Enhancement: Check Condition Code and attempt recovery
								*/
								if (MessageLevel() >= ML_ERROR)
								{
									ShowRC_CC(hWnd, 1, TWRC_FAILURE, 1,
												"",
												"DG_IMAGE/DAT_IMAGEMEMXFER/MSG_GET");               
								}

								twRC2 = CallDSMEntry(&appID,
												&dsID, 
												DG_CONTROL,
												DAT_PENDINGXFERS, 
												MSG_ENDXFER,
												(TW_MEMREF)&twPendingXfer);

								if (twRC2 != TWRC_SUCCESS)
								{
									if (MessageLevel()  >= ML_ERROR)
									{
										ShowRC_CC(hWnd, 1, twRC2, 1, 
													"", 
													"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
									}
								}

								GlobalUnlock(hbm_acq);
								GlobalFree(hbm_acq);
								hbm_acq = NULL;
								break;
                            
							default:
								if (MessageLevel() >= ML_ERROR)
								{
									ShowRC_CC(hWnd, 0, 0, 0,
												"Unknown Return Code",
												"DG_IMAGE/DAT_IMAGEMEMXFER/MSG_GET");               
								}
								/*
								* Abort the image
								*/
								twRC2 = CallDSMEntry(&appID,
												&dsID, 
												DG_CONTROL,
												DAT_PENDINGXFERS, 
												MSG_ENDXFER,
												(TW_MEMREF)&twPendingXfer);

								if (twRC2 != TWRC_SUCCESS)
								{
									if (MessageLevel()  >= ML_ERROR)
									{
										ShowRC_CC(hWnd, 1, twRC2, 1, 
													"", 
													"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
									}
								}

								GlobalUnlock(hbm_acq);
								GlobalFree(hbm_acq);
								hbm_acq = NULL;

								/*
								* showRC_CC is a safe operation here since there will be no triplet
								* calls generated 
								*/
								if (MessageLevel() >= ML_INFO)
								{
									wsprintf(buffer,"Images = %d",twPendingXfer.Count);
									ShowRC_CC(NULL,0,0,0,buffer,"Pending Transfers");
								}
								break;
						} // switch
					} while (twRC == TWRC_SUCCESS);
				} 
			} 
		}  
	} while (twPendingXfer.Count != 0);

	return(hbm_acq);
}

///////////////////////////////////////////////////////////////////////////
// Function: FIX32ToFloat -- 
// Author: TWAIN Working Group
// Input:
//		fix32
// Output:
//		float
// Comments:
//		Convert a FIX32 value into a floating point value
//
float FIX32ToFloat (TW_FIX32 fix32)
{
	float   floater = 0;

	floater = (float) fix32.Whole + (float) (fix32.Frac / 65536.0);
	return(floater);
}
   
