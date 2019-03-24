#ifdef WIN32
#include <windows.h>
#endif

/*   this routine is made to be called from FORTRAN
     Integer*4	PTR,SIZE
     CALL MALLOC(PTR,SIZE)
	allocates SIZE bytes in heap and returns the pointer
	to them in PTR	

     CALL FREE(PTR)
	deallocates the memory pointed to by PTR

	this memory can be accessed by subroutines as follows:
	...
	CALL MALLOC(PTR,400)  !4*100
	If (PTR .EQ. 0) CALL EXIT(1) !Failed memory allocation...
	CALL DUMMY(%VAL(PTR))
	CALL FREE(PTR)
	...
	SUBROUTINE 	DUMMY(ARRAY)
	INTEGER*4	ARRAY(100)
	RETURN
	END

	Note: this causes the calling procedure to pass the value
		of the pointer on the stack instead of the address
		of the pointer.  the called procedure does not know
		this and assumes the value on the stack is the address
		of the array.  the array may be referenced as normal
		but the data goes into the dynamically allocated memory.

	Not a prescribed technique but it works until FORTRAN 8X
	Randy Frank, Image Analysis Facility, University of Iowa
*/

void fmalloc_(int *tran, int *size)
{
	*tran = (int)malloc((unsigned int)*size);
}
void ffree_(int *tran)
{
	free((void *)(*tran));
}
void FMALLOC(int *tran, int *size)
{
	*tran = (int)malloc((unsigned int)*size);
}
void FFREE(int *tran)
{
	free((void *)(*tran));
}


void clip2text_(char *text, int *ret, int max)
{
#ifdef WIN32
   extern HWND get_current_hwnd(void);

   HGLOBAL   hglb; 
   LPTSTR    lptstr; 

   *ret = 0;

   if (!IsClipboardFormatAvailable(CF_TEXT)) return; 
   if (!OpenClipboard(get_current_hwnd())) return; 
 
   hglb = GetClipboardData(CF_TEXT); 
   if (hglb) { 
      lptstr = GlobalLock(hglb); 
      if (lptstr) {
         int n = GlobalSize(hglb);
         if (n >= max) n = max-1;
         memcpy(text,lptstr,n);
         text[max-1] = ' ';
         *ret = n;
         GlobalUnlock(hglb); 
      } 
   } 
   CloseClipboard(); 
#endif
   return;
}

void CLIP2TEXT(char *text, int *ret, int max)
{
   clip2text_(text,ret,max);
}

void text2clip_(char *text, int max)
{
#ifdef WIN32
   extern HWND get_current_hwnd(void);

   HGLOBAL   hglb; 
   LPTSTR    lptstr; 

   if (max == 0) return;
   if (!OpenClipboard(get_current_hwnd())) return;

   EmptyClipboard(); 
 
   hglb = GlobalAlloc(GMEM_MOVEABLE,max + 1); 
   if (!hglb) { 
      CloseClipboard(); 
      return; 
   } 
 
   lptstr = GlobalLock(hglb); 
   memcpy(lptstr, text, max);
   lptstr[max-1] = '\0';
   GlobalUnlock(hglb); 

   SetClipboardData(CF_TEXT, hglb); 

   CloseClipboard(); 
#endif
   return; 
}

void TEXT2CLIP(char *text, int max)
{
   text2clip_(text,max);
}

