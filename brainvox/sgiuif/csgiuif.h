/*----------------------------------------------*/
/* C Include file for the SGIUIF package        */
/* Copyright (C) 1989 Image Analysis Facility   */
/*               71 HBRF University of Iowa     */
/*----------------------------------------------*/


/*	version number				*/

#define	DLG_VERSION = 1.10

/*	The dialog item structure...		*/

typedef struct ditemS
{
int	dtype;
int	tlength;
int	tpos;
char    text[200];
int	rect[4];
int	aux;
}  ditem; /* End Structure				*/

/*	Define the item types...		*/

#define DLG_HEADER 1
#define DLG_COLOR  2
#define DLG_END    3
#define DLG_LINE   4
#define DLG_BOX    5
#define DLG_TEXT   6
#define LG_EDIT    7
#define DLG_SBAR   8
#define	DLG_MENU   9
#define	DLG_CHECK  10
#define	DLG_BUTTON 11
#define DLG_NOP    12
#define	DLG_RADIO  13


/*	define the event types...		*/

#define DLG_CHKEVNT 3000
#define	DLG_BUTEVNT 3001
#define	DLG_BAREVNT 3002
#define	DLG_EDTEVNT 3003
#define	DLG_POPEVNT 3004
#define	DLG_TXTEVNT 3005
#define	DLG_NOPEVNT 3006
#define	DLG_RADEVNT 3007

/******* end of SGIUIF includes ****************/
