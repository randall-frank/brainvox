/********************************************************************************/
/* real_time_stim.c 								*/
/* 	COMPILE: cc real_time_stim.c -o real_time_stim cvio_lib.o  		*/
/* 		 also include all the .o files from zlib 			*/
/*	PURPOSE: filters the cvio stimulus channel file down to its onsets	*/
/*		 and the corresponding timestamp, which are written into 	*/
/*		 another cvio file. The generated cvio file can then be used	*/
/*		 as an input for cvio_impulse, which performs the real-time	*/
/*		 convolution							*/
/*	USAGE:	 real_time_stim [options] srcfile dstfile			*/
/*		 -v: verbose mode						*/
/*		 -t: sets the gating threshold for an event			*/
/*		 -a: sets the abort time in seconds (default 300)		*/
/*		 -s: file template (sprintf string) (default none)		*/
/*		 srcfile: cvio file to get filtered				*/
/*		 dstfile: output file						*/
/*	AUTHOR:	 Chris Smyser	02-09-01					*/
/*		 Brent Eaton	02-13-01 (revisions and comments)		*/
/*		 Brent Eaton	03-23-01 (revisions)				*/
/*	NOTES:	 This routine currently (2/13/01) has two shortcomings, it	*/
/*		 skips any onset events that occur in the first 0.8 secs of 	*/
/*		 run time (a short event can still be included during this	*/
/*		 time to start the acuisition, it just will not be included in	*/
/*		 the events sent to the convolution) and it must be told the	*/
/*		 exact number of events which will occur (otherwise it either	*/
/*		 undercounts or waits 300 secs for another sample before 	*/
/*		 quitting). Neither of these shortcomings is fatal.		*/
/*		 ** (3/23/01) The two shortcomings above have been remedied.	*/
/*		 Program will now detect onsets immediately as they occur after	*/
/*		 filtering begins. Also number of expected stimulus no longer	*/
/*		 has to be entered as cvio_has_writer() function is used to 	*/
/*		 keep track of whether the file being filtered is still being	*/
/*		 written. This does limit the use of this program to filtering	*/
/*		 shared memory files for now because has_writer() does not work	*/
/*		 on disk files. An attempt will be made to remedy this problem	*/
/*		 in the future.							*/
/********************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <cvio_lib.h>

int tstext = 0;

void info(char *app)
{
        fprintf(stderr,"(%s) Usage: %s [options] srcfile dstfile\n",__DATE__,app);
        fprintf(stderr,"Options:\n");
        fprintf(stderr,"        -v verbose\n");
        fprintf(stderr,"        -t(thres) gating threshold\n");
        fprintf(stderr,"        -a(wait)  abort if no samples for wait seconds. (default: 300)\n");
        fprintf(stderr,"        -s(str) file template (sprintf string) (default: none)\n");
	fprintf(stderr,"	-T	Timestamp all textual output\n");
	fprintf(stderr,"	-d	don't create output streams(they already exist)\n");
	fprintf(stderr,"	-o	offset\n");
        exit(1);
}

void	exit_cvio_error(int32_t err) 
{
	char	tstr[256];
	int32_t	tlen = 256;

	cvio_get_error_message(err,tstr,&tlen);
	fprintf(stderr,"real_time_stim: Fatal error:%s\n",tstr);

	cvio_cleanup(0);
	exit(1);
}

int main(int argc, char *argv[])
{
    int32_t	offset = 0;
    int32_t 	verbose = 0;
    int32_t 	threshold = 32000;
    int dontcreate = 0;
	int32_t	abort_time = 300;
	char	*template = NULL;
	char	*srcfile;
	char	*dstfile;
	uint32_t oc;
	int32_t count;
    uint32_t  src_s,dst_s;
    uint32_t  maxsamp, maxevents;
    int32_t   err;
	int32_t		i,j;
	CVIO_DT_TIME	ts[500], outts[500];
	int16_t  	value[500], output[500];
	int32_t   datatype,ndims,dims[3],imgsize;
	FILE *pieceofjunk;

	/* parse the options */
        i = 1;
        while((i<argc) && (argv[i][0] == '-')) {
                switch(argv[i][1]) {
			case 'd' : dontcreate = 1; break;
                        case 'v' : verbose = 1; break;
                        case 't' : 
				threshold = atoi(argv[i]+2);
				break;
                        case 'o' : 
				offset = atoi(argv[i]+2);
				break;
                        case 'a' : 
				abort_time = atoi(argv[i]+2);
				break;
                        case 's' : 
				template = argv[i]+2;
				break;
			case 'T' : tstext = 1; break;
                        default: info(argv[0]); break;
                }
                i++;
	}

	if (argc-i != 2) info(argv[0]);

	/* get the filenames */
	srcfile = argv[i];
	dstfile = argv[i+1];

	if (cvio_init()) exit(1);

	/* open the source file */
	err = cvio_open(srcfile,CVIO_ACCESS_READ,&src_s);
	if (err) exit_cvio_error(err);

	/* get its format and size */
	err = cvio_datatype(src_s,&datatype,&ndims,dims);
	if (err) exit_cvio_error(err);

	err = cvio_max_length(src_s,&maxsamp);
cvio_log(tstext,"err: %ld maxsamp: %u\n",err,maxsamp);
	if (err) exit_cvio_error(err);

	/* 16bit, scalars */
	if ((datatype != CVIO_TYP_SHORT) || 
	    (ndims != 1) || (dims[0] != 1)) {
		cvio_log(tstext,"Unexpected input CVIO format.\n");
		cvio_cleanup(0);
		exit(1);
	}
	if (!dontcreate) {
		/* create the destination file */
		maxevents = maxsamp/1000;
		cvio_log(tstext,"after create\n");
		cvio_create(dstfile, maxevents, CVIO_TYP_BOOL, ndims, dims, 0);
	}

	/* open the destination file */
	err = cvio_open(dstfile,CVIO_ACCESS_APPEND,&dst_s);
	if (err) exit_cvio_error(err);

	memset(output, 1, sizeof(output));
	do {
		int lasthigh = 0;
		/* read up to 500 samples at a time */
		count = 500;
		oc = 0;
		err = cvio_read_next_samples_blocking(src_s,ts,&value,&count, 0, 0);
//printf("real_time_stim got %ld samples, err = %ld\n", count, err);
		if (err == CVIO_ERR_OK) {
			assert(count > 0);
			for (i = 0; i < count; i++) {
				if (value[i] == -32768)  {
					value[i] = -32767;
				}
				value[i] = abs(value[i]);
				if (value[i] - offset > (int16_t)threshold) {
					if (!lasthigh)
						outts[oc++] = ts[i];
					lasthigh = 1;
				} else lasthigh = 0;
			}
			if (oc)
				err = cvio_add_samples(dst_s,outts,output,oc);
			else err = cvio_add_samples(dst_s, &ts[count - 1], NULL, 0);
		}
	} while(err == CVIO_ERR_OK);

	/* cleanup and exit */
	err = cvio_close(src_s);
	err = cvio_close(dst_s);

	cvio_cleanup(0);

	exit(0);
}
