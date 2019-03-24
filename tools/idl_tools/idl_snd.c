#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "export.h"
#include "idl_tools.h"

/* function protos */
extern IDL_VPTR IDL_CDECL snd_play(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL snd_stop(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL snd_query(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL snd_acq_start(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL snd_acq_data(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL snd_acq_stop(int argc, IDL_VPTR argv[], char *argk);

/* define the SND functions */
static IDL_SYSFUN_DEF snd_functions[] = {

    { snd_play,		"SND_PLAY",		2, 2, 0},
    { snd_stop,		"SND_STOP",		0, 1, 0},
    { snd_query,	"SND_QUERY",		0, 1, 0},
    { snd_acq_start,	"SND_ACQ_START",	1, 1, 0},
    { snd_acq_data,	"SND_ACQ_DATA",		3, 3, 0},
    { snd_acq_stop,	"SND_ACQ_STOP",		0, 0, 0},

};

#ifdef WIN32

#include <dsound.h>

typedef struct _soundbuffer {
	struct _soundbuffer	*next;
	LPDIRECTSOUNDBUFFER	lpdsb;
	IDL_LONG		iSoundID;
} soundbuffer;

/* indirect handling of the direct sound APIs */
typedef HRESULT (WINAPI *myDirectSoundCaptureCreate)(LPGUID,                  
		LPDIRECTSOUNDCAPTURE *,LPUNKNOWN); 
typedef HRESULT (WINAPI *myDirectSoundCreate)(LPGUID,         
		LPDIRECTSOUND *,LPUNKNOWN);

/* Globals */
static	HINSTANCE			hDLL = NULL;

/* play globals */
static	LPDIRECTSOUND			lpds = NULL;
static	IDL_LONG			iNextID = 1000;
static	soundbuffer			*sndlist = NULL;
static  myDirectSoundCreate		pDirectSoundCreate = NULL;

/* acquire globals */
static  LPDIRECTSOUNDCAPTURE		lpdsc = NULL;
static  LPDIRECTSOUNDCAPTUREBUFFER	lpdscb = NULL;
static	WAVEFORMATEX			acqwfx;
static	DWORD				nBufferSize;
static	DWORD				dwReadCursor = 0;
static  myDirectSoundCaptureCreate	pDirectSoundCaptureCreate = NULL;

/* prototypes */
static void snd_prune_buffers(IDL_LONG which);

/* startup call when DLM is loaded */
int snd_startup(void)
{
	HWND hWnd;

	/* dynamically load DirectSound for machines without it */
	hDLL = LoadLibrary("dsound.dll");
	if (!hDLL) return(IDL_FALSE);
	
	pDirectSoundCreate = (myDirectSoundCreate)GetProcAddress(hDLL,"DirectSoundCreate");
	if (!pDirectSoundCreate) return(IDL_FALSE);
	if FAILED(pDirectSoundCreate(NULL, &lpds, NULL)) return IDL_FALSE;

/* set the priority, needed w/ DirectX 7 */
	hWnd = GetForegroundWindow();
	if (hWnd == NULL) hWnd = GetDesktopWindow();
	IDirectSound_SetCooperativeLevel(lpds, hWnd, DSSCL_PRIORITY);

/* this call is not supported under Windows NT 4.0 and earlier!!! */
/* so we will allow and expect it to fail... */
	pDirectSoundCaptureCreate = (myDirectSoundCaptureCreate)GetProcAddress(hDLL,"DirectSoundCaptureCreate");

	if (!IDL_AddSystemRoutine(snd_functions, TRUE, 
		ARRLEN(snd_functions))) return IDL_FALSE;

	IDL_ExitRegister(snd_exit_handler);

	return(IDL_TRUE);
}

/* called when IDL is shutdown */
void snd_exit_handler(void)
{
	/* playback */
	snd_prune_buffers(-1);
	if (lpds) IDirectSound_Release(lpds); 
	/* acquire */
	if (lpdscb) IDirectSoundCaptureBuffer_Stop(lpdscb);
	if (lpdscb) IDirectSoundCaptureBuffer_Release(lpdscb);
	if (lpdsc) IDirectSoundCapture_Release(lpdsc);

	if (hDLL) FreeLibrary(hDLL);

	return;
}

static void snd_prune_buffers(IDL_LONG which)
{
	soundbuffer	*buff = sndlist;
	soundbuffer	*next,*prev;
	DWORD		status;

	prev = NULL;

	while(buff) {
		IDL_LONG	bye = 0;
		
		next = buff->next;

/* die if: error, done playing, request to kill all, or matched specific ID */
		if FAILED(IDirectSoundBuffer_GetStatus( buff->lpdsb, 
			&status)) bye = 1;
		if (!(status & DSBSTATUS_PLAYING)) bye = 1;
		if ((which < 0) || (which == buff->iSoundID)) {
			if (bye == 0) {
				IDirectSoundBuffer_Stop( buff->lpdsb );
			}
			bye = 1;
		}

		if (bye) {
			IDirectSoundBuffer_Release( buff->lpdsb );
			if (!prev) {
				sndlist = next;
			} else {
				prev->next = next;
			}
			IDL_MemFree( buff, "", IDL_MSG_RET);
		} else {
			prev = buff;
		}

		buff = next;
	}
	return;
}

/*
 *	Notes:
 *		The system is based on DirectSound.  It will not init
 *		if DirectSound is not installed.
 *		All error codes return -1 on failure.
 *		Sound handles are passively reclaimed whenever any of
 *		the SND_XXX routines are called.  This means that any
 *		sounds that play themselves out will not have their
 *		memory/resources reclaimed until the next SND_XXX call
 *		or IDL shutdown.
 */

/*
 *	handle = SND_PLAY(sounddata, rate)
 *
 *	Plays a sampled sounds stream at the given rate (samples per second).
 *	Sounddata can be a 1D array (mono) or a [2,N] array (stereo).
 *	The data can be in 8bit (unsigned) or 16bit (signed), all other
 *	types are converted to 16bit signed internally.  The sound
 *	plays asynchronously and can be controlled by the returned handle.
 *	Once the sound has finished playing, the handle is no longer valid
 *	(this is a one-shot system).  Note: multiple sounds can be played
 *	simultaneously and all handles will be unique.
 */
IDL_VPTR IDL_CDECL snd_play(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_LONG	iRate;
	IDL_LONG	iChan;
	IDL_LONG	iSamp;
	IDL_LONG	iSize;
	IDL_VPTR	vpTmp;
	DSBUFFERDESC	dsbdesc;
	WAVEFORMATEX	wavefmt;
	LPDIRECTSOUNDBUFFER	lpdsb;
	void		*ptr;
	DWORD		len;
	soundbuffer	*buffer;
	HRESULT ret;

	snd_prune_buffers(0);

	IDL_ENSURE_ARRAY(argv[0]);
	vpTmp = argv[0];
	if (vpTmp->value.arr->n_dim > 2) {
 		IDL_MessageFromBlock(msg_block, M_TOOLS_BADSND, 
			IDL_MSG_LONGJMP);
	}
	if ((vpTmp->value.arr->n_dim == 2) && 
		(vpTmp->value.arr->dim[0] > 2)) {
 		IDL_MessageFromBlock(msg_block, M_TOOLS_BADSND, 
			IDL_MSG_LONGJMP);
	}
	if (vpTmp->value.arr->n_dim == 2) {
		iChan = vpTmp->value.arr->dim[0];
	} else {
		iChan = 1;
	}

	if (vpTmp->type != IDL_TYP_BYTE) {
		vpTmp = IDL_CvtFix(1, &vpTmp);
		iSize = 2;
	} else {
		iSize = 1;
	}
	iRate = IDL_LongScalar(argv[1]);
	iSamp = vpTmp->value.arr->n_elts / iChan;

	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); 
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STATIC
		| DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY; 
	dsbdesc.dwBufferBytes = iSamp * iSize * iChan;  
	dsbdesc.lpwfxFormat = &wavefmt;  
	
	wavefmt.wFormatTag = WAVE_FORMAT_PCM;
	wavefmt.nChannels = (unsigned short)iChan;
	wavefmt.nSamplesPerSec = iRate;
	wavefmt.nAvgBytesPerSec = iRate * iSize * iChan;
	wavefmt.nBlockAlign = (unsigned short)(iSize * iChan);
	wavefmt.wBitsPerSample = iSize * 8;
	wavefmt.cbSize = 0;

	if FAILED(IDirectSound_CreateSoundBuffer(
		lpds, &dsbdesc, &lpdsb, NULL))    {
		if (vpTmp != argv[0]) IDL_Deltmp(vpTmp);
		return(IDL_GettmpLong(-1));   
	}
	if FAILED(ret=IDirectSoundBuffer_Lock( lpdsb, 0, dsbdesc.dwBufferBytes, 
		&ptr, &len, NULL, NULL, 0 )) {
		IDirectSoundBuffer_Release( lpdsb );
		if (vpTmp != argv[0]) IDL_Deltmp(vpTmp);
		return(IDL_GettmpLong(-2));   
	}

	len = IDL_MIN(len, dsbdesc.dwBufferBytes);
	memcpy(ptr, vpTmp->value.arr->data, len);

	if FAILED(IDirectSoundBuffer_Unlock( lpdsb, ptr, len, NULL, 0)) {
		IDirectSoundBuffer_Release( lpdsb );
		if (vpTmp != argv[0]) IDL_Deltmp(vpTmp);
		return(IDL_GettmpLong(-3));   
	}

	if FAILED(IDirectSoundBuffer_Play( lpdsb, 0, 0, 0)) {
		IDirectSoundBuffer_Release( lpdsb );
		if (vpTmp != argv[0]) IDL_Deltmp(vpTmp);
		return(IDL_GettmpLong(-4));   
	}

	buffer = (soundbuffer *)IDL_MemAlloc(sizeof(soundbuffer),
		"",IDL_MSG_RET);
	if (!buffer) { 
		IDirectSoundBuffer_Release( lpdsb );
		if (vpTmp != argv[0]) IDL_Deltmp(vpTmp);
		return(IDL_GettmpLong(-5));
	}

	buffer->next = sndlist;
	buffer->iSoundID = iNextID++;
	buffer->lpdsb = lpdsb;
	sndlist = buffer;

	if (vpTmp != argv[0]) IDL_Deltmp(vpTmp);

	return(IDL_GettmpLong(buffer->iSoundID));
}

/*
 *	err = SND_STOP([handle])
 *
 *	If a handle is specified (and playing), it its playing will
 *	be stopped and its memory reclaimed.  If no handle is specified,
 *	ALL playing sounds will be stopped and reclaimed.
 */
IDL_VPTR IDL_CDECL snd_stop(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_LONG	iID = -1;

	if (argc > 0) iID = IDL_LongScalar(argv[0]);

	snd_prune_buffers(iID);

	return(IDL_GettmpLong(0));
}

/*
 *	err = SND_QUERY([handle])
 *
 *	Returns 1 if the specified handle is still playing.
 *	Returns -1 if the specified handle does not exist (or is done playing).
 *	If no handle is specified, the function returns 1
 *	if any sounds are playing, otherwise it returns 0.
 */
IDL_VPTR IDL_CDECL snd_query(int argc, IDL_VPTR argv[], char *argk)
{
	soundbuffer	*buff = sndlist;
	IDL_LONG	iID;

	snd_prune_buffers(0);

	if (argc == 0) {
		if (sndlist) return(IDL_GettmpLong(1));
		return(IDL_GettmpLong(0));
	}

	iID = IDL_LongScalar(argv[0]);
	while(buff) {
		if (buff->iSoundID == iID) {
			return(IDL_GettmpLong(1));
		}
		buff = buff->next;
	}

	return(IDL_GettmpLong(-1));
}

/*
 *	err = SND_ACQ_START(size)
 *
 *	Starts a sound acquisition process using a
 *	buffer of the size passed (in seconds).
 *	returns 0 on success and -1 on failure
 *	or 1 if an acquistion is already in progress.
 */
IDL_VPTR IDL_CDECL snd_acq_start(int argc, IDL_VPTR argv[], char *argk)
{
	DSCCAPS			    dsccaps;
	DSCBUFFERDESC               dscbDesc;
	double			    dAcqLen;

	if (!pDirectSoundCaptureCreate) goto err_out;

	if (lpdsc) return(IDL_GettmpLong(1));

	dAcqLen = IDL_DoubleScalar(argv[0]);
	if(dAcqLen <= 0.0) goto err_out;

	/* create the device */
	if FAILED(pDirectSoundCaptureCreate(NULL, &lpdsc, NULL)) {
		goto err_out;
	}
	/* figure out what its format is */
	dsccaps.dwSize = sizeof(DSCCAPS);
	if FAILED(IDirectSoundCapture_GetCaps(lpdsc, &dsccaps)) {
		goto err_out;
	}

	if (dsccaps.dwFormats & WAVE_FORMAT_4S16) {
			acqwfx.nChannels = 2;
			acqwfx.wBitsPerSample = 16;
			acqwfx.nSamplesPerSec = 44100;
	} else if (dsccaps.dwFormats & WAVE_FORMAT_4S08) {
			acqwfx.nChannels = 2;
			acqwfx.wBitsPerSample = 8;
			acqwfx.nSamplesPerSec = 44100;
	} else if (dsccaps.dwFormats & WAVE_FORMAT_4M16) {
			acqwfx.nChannels = 1;
			acqwfx.wBitsPerSample = 16;
			acqwfx.nSamplesPerSec = 44100;
	} else if (dsccaps.dwFormats & WAVE_FORMAT_4M08) {
			acqwfx.nChannels = 1;
			acqwfx.wBitsPerSample = 8;
			acqwfx.nSamplesPerSec = 44100;
	} else if (dsccaps.dwFormats & WAVE_FORMAT_2S16) {
			acqwfx.nChannels = 2;
			acqwfx.wBitsPerSample = 16;
			acqwfx.nSamplesPerSec = 22050;
	} else if (dsccaps.dwFormats & WAVE_FORMAT_2S08) {
			acqwfx.nChannels = 12;
			acqwfx.wBitsPerSample = 8;
			acqwfx.nSamplesPerSec = 22050;
	} else if (dsccaps.dwFormats & WAVE_FORMAT_2M16) {
			acqwfx.nChannels = 1;
			acqwfx.wBitsPerSample = 16;
			acqwfx.nSamplesPerSec = 22050;
	} else if (dsccaps.dwFormats & WAVE_FORMAT_2M08) {
			acqwfx.nChannels = 1;
			acqwfx.wBitsPerSample = 8;
			acqwfx.nSamplesPerSec = 22050;
	} else if (dsccaps.dwFormats & WAVE_FORMAT_1S16) {
			acqwfx.nChannels = 2;
			acqwfx.wBitsPerSample = 16;
			acqwfx.nSamplesPerSec = 11025;
	} else if (dsccaps.dwFormats & WAVE_FORMAT_1S08) {
			acqwfx.nChannels = 2;
			acqwfx.wBitsPerSample = 8;
			acqwfx.nSamplesPerSec = 11025;
	} else if (dsccaps.dwFormats & WAVE_FORMAT_1M16) {
			acqwfx.nChannels = 1;
			acqwfx.wBitsPerSample = 16;
			acqwfx.nSamplesPerSec = 11025;
	} else if (dsccaps.dwFormats & WAVE_FORMAT_1M08) {
			acqwfx.nChannels = 1;
			acqwfx.wBitsPerSample = 8;
			acqwfx.nSamplesPerSec = 11025;
	}
	acqwfx.wFormatTag = WAVE_FORMAT_PCM;
	acqwfx.cbSize = 0;
	acqwfx.nBlockAlign = (acqwfx.nChannels*acqwfx.wBitsPerSample)/8;
	acqwfx.nAvgBytesPerSec = acqwfx.nSamplesPerSec*acqwfx.nBlockAlign;

	dscbDesc.dwSize = sizeof(DSCBUFFERDESC);    
	dscbDesc.dwFlags = 0;
	dscbDesc.dwBufferBytes = (unsigned long)(dAcqLen*acqwfx.nAvgBytesPerSec);
	dscbDesc.dwBufferBytes /= acqwfx.nBlockAlign;
	dscbDesc.dwBufferBytes *= acqwfx.nBlockAlign;
	dscbDesc.dwReserved = 0;
	dscbDesc.lpwfxFormat = &acqwfx; 
	if FAILED(IDirectSoundCapture_CreateCaptureBuffer(lpdsc, 
		&dscbDesc, &lpdscb, NULL)) {
		goto err_out;
	}

	if FAILED(IDirectSoundCaptureBuffer_Start(lpdscb,
		DSCBSTART_LOOPING)) {
		goto err_out;
	}

	/* record the buffer size and the current read cursor */
	nBufferSize = dscbDesc.dwBufferBytes;
	dwReadCursor = 0;

	return(IDL_GettmpLong(0));

err_out:
	if (lpdscb) IDirectSoundCaptureBuffer_Release(lpdscb);
	if (lpdsc) IDirectSoundCapture_Release(lpdsc);
	lpdscb = NULL;
	lpdsc = NULL;

	return(IDL_GettmpLong(-1));
}

/*
 *	err = SND_ACQ_STOP()
 *
 *	Shuts down the acquisition process.
 *	returns 0 on success and -1 on failure.
 */
IDL_VPTR IDL_CDECL snd_acq_stop(int argc, IDL_VPTR argv[], char *argk)
{
	if (lpdscb) IDirectSoundCaptureBuffer_Stop(lpdscb);
	if (lpdscb) IDirectSoundCaptureBuffer_Release(lpdscb);
	if (lpdsc) IDirectSoundCapture_Release(lpdsc);

	lpdscb = NULL;
	lpdsc = NULL;

	return(IDL_GettmpLong(0));
}

/*
 *	err = SND_ACQ_DATA(time,data,rate)
 *
 *	Acquires sound for (time) seconds and returns the data
 *	in (data) and the sampling rate in Hz in (rate).
 *	returns 0 on success and -1 on failure.
 */
IDL_VPTR IDL_CDECL snd_acq_data(int argc, IDL_VPTR argv[], char *argk)
{
	double			dLen;
	DWORD			nSize;
	IDL_VPTR		vpTmp;
	char			*buffer;
	DWORD			len1,len2;
	DWORD			dwCapturePos,dwReadPos;
	void			*ptr1,*ptr2;
	int		        type,n_dim;
	IDL_MEMINT	        dim[3];

	if (!lpdsc) return(IDL_GettmpLong(-1));

	dLen = IDL_DoubleScalar(argv[0]);
	if (dLen <= 0.0) return(IDL_GettmpLong(-1));

	IDL_EXCLUDE_EXPR(argv[1]);
	IDL_EXCLUDE_EXPR(argv[2]);

	nSize = (unsigned long)(dLen*acqwfx.nAvgBytesPerSec);
	nSize /= acqwfx.nBlockAlign;
	nSize *= acqwfx.nBlockAlign;
	if (nSize > nBufferSize*0.5) return(IDL_GettmpLong(-1));

	IDL_VarCopy(IDL_GettmpLong(acqwfx.nSamplesPerSec),argv[2]);

	type = IDL_TYP_BYTE;
	if (acqwfx.wBitsPerSample == 16) type = IDL_TYP_INT;
	if (acqwfx.nChannels == 2) {
		n_dim = 2;
		dim[0] = 2;
		dim[1] = nSize/acqwfx.nBlockAlign;
	} else {
		n_dim = 1;
		dim[0] = nSize/acqwfx.nBlockAlign;
	}

	buffer = (char *)IDL_MakeTempArray(type, n_dim, dim,
		IDL_BARR_INI_NOP, &vpTmp);

	while(1) {
		if FAILED(IDirectSoundCaptureBuffer_GetCurrentPosition(
			lpdscb, &dwCapturePos, &dwReadPos)) {
			goto err_out;
		}
		if (dwReadPos < dwReadCursor) dwReadPos += nBufferSize;
		if ((dwReadPos-dwReadCursor) >= nSize) break;
		SleepEx((unsigned long)(dLen*0.1), 0);
	}

        if FAILED(IDirectSoundCaptureBuffer_Lock(lpdscb,dwReadCursor,
		nSize, &ptr1, &len1, &ptr2, &len2, 
		0)) {
		goto err_out;
	}

	/* copy the buffer in 2 blocks (if needed) */
	memcpy(buffer,ptr1,len1);	
	if (ptr2) memcpy(buffer+len1,ptr2,len2);

        if FAILED(IDirectSoundCaptureBuffer_Unlock(lpdscb,
		ptr1,len1,ptr2,len2)) {
		goto err_out;
	}

	/* bump the cursor */
	dwReadCursor += nSize;
	if (dwReadCursor >= nBufferSize) dwReadCursor -= nBufferSize;

	/* return the variable */
	IDL_VarCopy(vpTmp,argv[1]);

	return(IDL_GettmpLong(0));

err_out:
	if (lpdscb) IDirectSoundCaptureBuffer_Stop(lpdscb);
	if (lpdscb) IDirectSoundCaptureBuffer_Release(lpdscb);
	if (lpdsc) IDirectSoundCapture_Release(lpdsc);

	lpdscb = NULL;
	lpdsc = NULL;

	return(IDL_GettmpLong(-1));
}

#else

/* startup call when DLM is loaded */
int snd_startup(void)
{
	if (!IDL_AddSystemRoutine(snd_functions, TRUE, 
		ARRLEN(snd_functions))) {
		return IDL_FALSE;
	}
	IDL_ExitRegister(snd_exit_handler);
	return(IDL_TRUE);
}

/* called when IDL is shutdown */
void snd_exit_handler(void)
{
}

IDL_VPTR IDL_CDECL snd_play(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

IDL_VPTR IDL_CDECL snd_stop(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

IDL_VPTR IDL_CDECL snd_query(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

IDL_VPTR IDL_CDECL snd_acq_start(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

IDL_VPTR IDL_CDECL snd_acq_data(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}

IDL_VPTR IDL_CDECL snd_acq_stop(int argc, IDL_VPTR argv[], char *argk)
{
	return(IDL_GettmpLong(-1));
}


#endif

