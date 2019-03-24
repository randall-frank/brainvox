#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <stdlib.h>

#include "vl_parallel.h"

#if (defined(IRIX4) || defined(IRIX5))
#include <sys/types.h>
#include <sys/prctl.h>
void vl_m_ncpus_(VLINT32 *ncpus,VLINT32 *err)
{
	*ncpus = prctl(PR_MAXPPROCS);
	*err = 0;

	return;
}
#elif defined(SOLARIS)
#include <unistd.h>
void vl_m_ncpus_(VLINT32 *ncpus,VLINT32 *err)
{
	*ncpus = sysconf(_SC_NPROCESSORS_ONLN);
	*err = 0;

	return;
}
#elif (defined(WIN32) || defined(__WIN32__))
#include <windows.h>
#include <windowsx.h>
void vl_m_ncpus_(VLINT32 *ncpus,VLINT32 *err)
{
SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );
	*ncpus = sysinfo.dwNumberOfProcessors;
	*err = 0;

	return;
}
#else
void vl_m_ncpus_(VLINT32 *ncpus,VLINT32 *err)
{
	*ncpus = 1;
	*err = 1;

	return;
}
#endif
