/*
 * 
 * Subtraction filter for speech acquired during a fMRI experiment.  Sound 
 * profile from the acquisition of the second volume of EPI slices is saved
 * to a memory buffer and then subtracted from the sound profile acquired
 * during all subsequent volume acquisitions.  Reading and filtering from raw
 * sound file is done on large (4000 element) buffers.  Output is written to a 
 * separate CVIO format file, which can be filtered to detect speech events
 * (currently done by cvio_stim_pacer - 12/04/02).
 * 
 * Input: # of slices, TR and the names of the sound and image files.
 *
 * Options:	-v		verbose
 *		-r(rate)	rate (Hz) of sound acquisition (default 16000)
 *		-a(abort)	abort time in seconds for no samples
 * 
 * 01-13-03	BLE
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include "cvio_lib.h"
#include <assert.h>
#include <getopt.h>

/* set the size for single-read buffer
  The size is significant compared to its previous value because
  it not longer dictates a lower bound for latency.
  larger buffer should speed offline processing with no effect on
  realtime processing.
*/
#define BUFSIZE	800

int tstext = 0;

void exit_cvio_error(int32_t err)
{
    char    tstr[256];
    int32_t   tlen = 256;
    
    cvio_get_error_message(err, tstr, &tlen);
    cvio_log(tstext, "(speech_subtract2) Fatal error: %s\n", tstr);
    
    cvio_cleanup(0);
    exit(1);
}

void info(char *app)
{
	fprintf(stderr, "(%s) Usage: %s v2.2 [options]\n", __DATE__, app);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-v\t\tverbose\n");
	fprintf(stderr, "\t-r (rate)\tsound acquisition rate (def. 16000)\n");
	fprintf(stderr, "\t-T\tTimestamp all textual output\n");
	fprintf(stderr, "\t-d\tdon't create output file(it already exists)\n");
	fprintf(stderr, "\t-i (input stream)\traw sound input stream\n");
	fprintf(stderr, "\t-o (output stream)\tfiltered sound output stream\n");
	fprintf(stderr, "\t-u (unblank stream)\timage acquisition time stream\n");
	fprintf(stderr, "\t-t (tr)\ttime to repeat\n");
	fprintf(stderr, "\t-s (slices)\tslices per volume\n");
	exit(1);
}

/* utility functions called from main */
void freebufs(uint16_t *ref, uint16_t *snd, uint16_t *out, CVIO_DT_TIME *tim) {

	free(ref);
	free(snd);
	free(out);
	free(tim);

}

void closestrms(uint32_t snd, uint32_t img, uint32_t dst, FILE *file, int verbal) {

	cvio_close(snd);
	cvio_close(img);
	cvio_close(dst);
	if (verbal) fclose(file);

}

int main(int argc, char **argv)
{
	int ch, dontcreate = 0;
	uint32_t dummy, img_start_sample;
	int32_t count;
    int32_t slices, block, err, i, steps, counter, loc;
    int32_t rate=16000;
	int verbose = 0;
    float tr;
    char *img = NULL, *snd = NULL, *dst = NULL;
    uint32_t img_s, snd_s, dst_s;
    CVIO_DT_TIME img_t, snd_start_time, old_last_time;
	CVIO_DT_TIME *snd_t;
    int32_t img_type, img_ndims, img_dims[3];
    int32_t snd_type, snd_ndims, snd_dims[3];
    uint32_t max_img, max_snd;
    CVIO_DT_TIME cTime, abort_t=200000;
    uint16_t *ref_b, *snd_b, *out_b;
    FILE *fp;
	int currslice = 0, refptr = 0, start = 0;
	
	/* parse the command line */
    i=1;
    while ((ch = getopt(argc, argv, "vTt:s:r:a:i:o:u:d")) != -1)
    	switch (ch) {
		case 'v' : verbose = 1; break;
		case 'r' : rate = (int32_t)atol(optarg); break;
		case 'a' : abort_t = (CVIO_DT_TIME)(atol(optarg)*20000); break;
		case 'i' : snd = strdup(optarg); break;
		case 'o' : dst = strdup(optarg); break;
		case 'u' : img = strdup(optarg); break;
		case 't' : tr = atof(optarg); break;
		case 's' : slices = (int32_t)atol(optarg); break;
		case 'T' : tstext = 1; break;
		case 'd' : dontcreate = 1; break;
		case '?' :
		default : info(argv[0]); break;
	}

	if (snd == NULL || dst == NULL || img == NULL)
		info(argv[0]);

    if (cvio_init()) exit(1);
    
    if (verbose) fp = fopen("speech_filt.txt", "w");
    
    /* open the input files */
	cTime = cvio_current_time();
	err = CVIO_ERR_EOF;
	while (err) {
//printf("trying to open %s\n", img);
    	err = cvio_open(img, CVIO_ACCESS_READ, &img_s);
    	if ((cvio_current_time() - cTime) > abort_t) exit_cvio_error(err);
	}
	err = CVIO_ERR_EOF;
	while (err) {
//printf("trying to open %s\n", snd);
    	err = cvio_open(snd, CVIO_ACCESS_READ, &snd_s);
    	if ((cvio_current_time() - cTime) > abort_t) exit_cvio_error(err);
	}
    if (verbose) cvio_log(tstext,"Input files for subtraction opened\n");
    
    /* check datatypes, dims and maximum samples */
    err = cvio_datatype(img_s,&img_type,&img_ndims,img_dims);
    if (err) exit_cvio_error(err);
    err = cvio_datatype(snd_s,&snd_type,&snd_ndims,snd_dims);
    if (err) exit_cvio_error(err);
    
    if (snd_ndims != 1)
	{
		cvio_log(tstext,"(%s): Input file type error\n", argv[0]);
		cvio_cleanup(0);
		info(argv[0]);
    }
    
    /* get a couple of important sizes */
    err = cvio_max_length(img_s, &max_img);
    if (err) exit_cvio_error(err);
    err = cvio_max_length(snd_s, &max_snd);
    if (err) exit_cvio_error(err);
    
    /* create and open the output file */
	if (!dontcreate) {
		err = cvio_create(dst, max_snd, CVIO_TYP_USHORT,snd_ndims,snd_dims,0);
		if (err) exit_cvio_error(err);
	}
cvio_log(tstext, "trying to open %s\n",dst);
    err = cvio_open(dst, CVIO_ACCESS_APPEND, &dst_s);
    if (err) exit_cvio_error(err);
    
    /* Calculate the number of volumes to expect */
    steps = (int32_t)(max_img/slices);
    if (verbose) cvio_log(tstext,"Steps: %d\n", steps);
    
    /* find size of reference block and allocate memory */
    block = (int32_t)(rate*tr * 1.005);
    if (verbose) cvio_log(tstext,"Block size: %d\n", block);
    
    ref_b = (uint16_t *)malloc(block*(snd_type/8));
    out_b = (uint16_t *)malloc(BUFSIZE*(snd_type/8));
    snd_b = (uint16_t *)malloc(BUFSIZE*(snd_type/8));
    snd_t = (CVIO_DT_TIME *)malloc(BUFSIZE*sizeof(CVIO_DT_TIME));
    for (i=0; i<BUFSIZE; i++) out_b[i] = 0;
    
	/* read to image number SLICES+1 */
	for (i=0; i<(slices+1); i++) {
		int sampcount = 1;
		err = cvio_read_next_samples_blocking(img_s,&img_t,NULL,&sampcount, 0, 0);
		if (err != CVIO_ERR_OK) {
			cvio_cleanup(0);
			exit(1);
		}
	}

    /* img_t holds the timestamp of the SLICES+1st image.  Find that timestamp 
     * in the raw sound file.
     */

	if (verbose) cvio_log(tstext,"Reference buffer starts at timestamp %d\n",img_t);

	cvio_tell(img_s, &img_start_sample, &dummy, &dummy);
	count = 0;
	do {
		old_last_time = snd_t[count];
		count = BUFSIZE;
		err = cvio_read_next_samples_blocking(snd_s,snd_t,snd_b, &count, abort_t, CVIO_TIMEOUT_LOCAL);
		if (err != CVIO_ERR_OK || count == 0) {
			freebufs(ref_b,out_b,snd_b,snd_t);
			closestrms(snd_s,img_s,dst_s,fp,verbose);
			exit_cvio_error(err);
		}
		for (loc = 0; loc < count && snd_t[loc] < img_t; loc++);
	} while (snd_t[loc] < img_t);
	/* loc should now be the index of the first sample during scan */

	if (loc == 0)
		snd_start_time = old_last_time;
	else
		snd_start_time = snd_t[loc - 1];

	/* Copy next BLOCK samples into reference buffer */
	counter = 0;
	while (counter < block) {
		for (i = loc; i < count; i++)
			ref_b[counter++] = snd_b[i];

		loc = 0;
		count = block - counter;
		if (count > BUFSIZE)
			count = BUFSIZE;
		err = cvio_read_next_samples_blocking(snd_s, snd_t, snd_b, &count, 0, 0);
		if (err) {
			freebufs(ref_b,out_b,snd_b,snd_t);
			closestrms(snd_s,img_s,dst_s,fp,verbose);
			exit_cvio_error(err);
		}
	}
	
	/* reference buffer is filled, rewind streams to start of valid data */
	err = cvio_seek(img_s, 0, img_start_sample);
	if (err != CVIO_ERR_OK)
		cvio_log(tstext, "error %d\n", err);
	assert(err == CVIO_ERR_OK);
	err = cvio_read_samples(snd_s, &snd_start_time, NULL, 1);
	if (err != CVIO_ERR_OK)
		cvio_log(tstext, "error %d\n", err);
	assert(err == CVIO_ERR_OK);

	/* remaining volumes processed in a loop
		output should start with 1 TR's worth of perfect silence
		since it exactly matches refbuf
	*/
	steps = steps - 2; /* first and last step may be incomplete*/

	count = 0;
	loc = 0;

	while (steps > 0)
	{
		int32_t sampcount = 1;
		/* check image file
			inc currslice if there's an image.
			if currslice is divisible by slices, we have a volume start time
			we end up with 1 slice worth of latency.  hopefully it's not too bad.
		*/
		err = cvio_read_next_samples_blocking(img_s,&img_t,NULL,&sampcount, 0, 0);
		if (err != CVIO_ERR_OK) {
			cvio_cleanup(0);
			exit(1);
		}

		currslice++;
		if (!(currslice % slices)) {
			currslice = 0;
			--steps;
		}
		if (currslice == 1) {
			refptr = 0;
		}
		if (verbose) cvio_log(tstext,"Found image %d: time %d at real time %d\n",
		 steps, img_t, cvio_current_time());

		/* process the sound buffer until you reach the image time */
		do {
			if (loc >= count) {
				loc = 0;
				count = 0;
			}
			if (count == 0) {
				count = BUFSIZE;
				err = cvio_read_next_samples_blocking(snd_s, snd_t, snd_b, &count, abort_t, CVIO_TIMEOUT_LOCAL);
				if (err != CVIO_ERR_OK || count == 0) {
cvio_log(tstext, "sound stream EOF\n");
					freebufs(ref_b,out_b,snd_b,snd_t);
					closestrms(snd_s,img_s,dst_s,fp,verbose);
					exit_cvio_error(err);
				}
			}
if (img_t - snd_t[loc] > 2200 || refptr > block) {
	cvio_log(tstext,"trouble: snd_t[loc] = %d, img_t = %d : diff = %d\n", snd_t[loc], img_t, img_t - snd_t[loc]);
	cvio_log(tstext, "loc = %d, refptr = %d\n", loc, refptr);
}
			for (start = loc; loc < count && snd_t[loc] < img_t; loc++, refptr++) {
				if (refptr < block) {
					if (snd_type == CVIO_TYP_USHORT) {
						int32_t temp;
						temp = snd_b[loc] - 32768;
						temp -= (ref_b[refptr] - 32768);
						temp += 32768;
						snd_b[loc] = temp;
					} else {
						int32_t temp;
						temp = snd_b[loc] - ref_b[refptr];
						temp += 32768;
						snd_b[loc] = temp;
					}
				} else {
					snd_b[loc] = 0;
//					printf("%d > %d\n", refptr, block);
				}
			}
			if (loc - start == 0) {
//0 byte writes mess things up now that they mean something...
//				printf("skipping 0 byte write\n");
				continue;
			}
			err = cvio_add_samples(dst_s,snd_t + start,snd_b + start, loc - start);
			if (err) {
				freebufs(ref_b,out_b,snd_b,snd_t);
				closestrms(snd_s,img_s,dst_s,fp,verbose);
				exit_cvio_error(err);
			}
		} while (loc >= count || snd_t[loc] < img_t);
	} /* that's it...start again with first image in next volume */
        
    /* cleanup and exit */
    freebufs(ref_b,out_b,snd_b,snd_t);
    closestrms(snd_s,img_s,dst_s,fp,verbose);
    
    err = cvio_cleanup(0);
    return 0; /* exit with zero error status */
}
