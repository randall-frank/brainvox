
/* this operation is only supported under IRIX  w/digital media options */
/* the Makefile defines "irix" if this is IRIX and the digital media libs
 *  have been installed */
#ifdef irix

/* include the necessary libraries */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <audio.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/signal.h>
#include <sched.h>
#include <dmedia/dm_params.h>
#include <dmedia/dm_audio.h>
#include <math.h>
#include "cvio_lib.h"

/*local prototypes */
void audio_port_setup(double time, int32 channel,int32 *channelnum, double *rate, char **file_name);
void exit_cvio_error(int32 err);	
void exit_cmd(char *s);

void exit_cvio_error(int32 err) 
{
	char	tstr[256];
	int32	tlen = 256;

	cvio_get_error_message(err,tstr,&tlen);
	fprintf(stderr,"Fatal acquire error:%s\n",tstr);

	cvio_cleanup(0);
	exit(1);
}

void audio_port_setup(double time, int32 channel, int32 *channelnum,double *rate, char **file_name)
{
	/* declare the necessary variables */
	ALpv globalbuffer[2];
	float big_rate;
	double time_inc;
	int32 result, global_rate, i, k;
	ALconfig portconfig;
	ALport port1;
	int32 b, h, total_frames, short_frames, count;
	int32 check1, ret;
	float correct_sample[8], absolute_sample[8];
	int16	*final_values1, *short_segment;
	CVIO_DT_TIME	*absolute_time,*time_data1;
	CVIO_DT_TIME basetime,temp,temp2;
	int32 t, count1;
	uint32 file_stream[8];
	int32	itype,ndims,dims[3];
	FILE *fp;

	double	pass_time = 0.5;  /* time for a single pass */

	global_rate = 48000;	/* minimum global sampling rate allowed */
	big_rate = 48000.0;

	time_inc = CVIO_TIME_TICKS_PER_SEC/big_rate;
	
	/* change global parameters to desired values */
	globalbuffer[0].param = AL_MASTER_CLOCK;
	globalbuffer[0].value.i = AL_CRYSTAL_MCLK_TYPE;
	globalbuffer[1].param = AL_RATE;
	globalbuffer[1].value.ll = alDoubleToFixed(big_rate);
	result = alSetParams(AL_DEFAULT_INPUT, globalbuffer, 2);

	/* turn off standard error handler */		
	alSetErrorHandler(0);

	/* desired number of samples to be collected */
	total_frames = global_rate*time;

	/* number of samples in one half second */
	short_frames = global_rate*pass_time;

	short_segment=(int16 *)malloc(sizeof(int16)*8*short_frames);
	final_values1=(int16 *)malloc(sizeof(int16)*short_frames);
	absolute_time=(CVIO_DT_TIME *)malloc(sizeof(CVIO_DT_TIME)*short_frames);
	time_data1=(CVIO_DT_TIME *)malloc(sizeof(CVIO_DT_TIME)*short_frames);

	/* determine sample interval to collect at desired rate */
	for(i=0;i<channel;i++) {
		correct_sample[i] = big_rate/rate[i];
		absolute_sample[i] = 0.0;
	}
		
	/* set up audio ports configuration with desired specifications */
	/* default ALconfig has two's complement format and 16-bit width */
	portconfig = alNewConfig();
	ret=alSetQueueSize(portconfig, short_frames*2.0);
	ret=alSetChannels(portconfig, 8);

	/* open one port up */
	port1 = alOpenPort("porta", "r", portconfig);

	for(i=0;i<channel;i++) {
		/* create a shared memory segment */
		check1 = cvio_open(file_name[i], CVIO_ACCESS_APPEND, &file_stream[i]);
printf("open: %ld\n",check1);
		if (check1) exit_cvio_error(check1);

		check1 = cvio_datatype(file_stream[i],&itype,&ndims,dims);
printf("datatype: %ld: %ld: %ld: %ld: %ld: %ld\n",check1,itype,ndims,dims[0],dims[1],dims[2]);
		if (check1) exit_cvio_error(check1);

		if ((ndims != 1) || (dims[0] != 1) || 
			(itype != CVIO_TYP_SHORT)) {
			exit_cvio_error(CVIO_ERR_BADDATATYPE);
		}
	}

	for (i=0; i<time/pass_time; i++) {
		/* get samples from port 1 */
		temp2 = cvio_current_time();

		alReadFrames(port1,short_segment,short_frames);

		/* get raw data and time values */
		if (i == 0) basetime = cvio_current_time();
		temp = basetime + ((short_frames*i)*time_inc);
		if (i != 0) temp = temp + 1;

		for (t=0; t<short_frames; t++) {
			absolute_time[t] = temp - 
				((double)(short_frames-t))*time_inc;
		} /* end for t */

		for(k=0;k<channel;k++) {
			h=0; count1=0;
			for (b=short_frames*i; b<short_frames*(i+1); b++) {
				if (b == (int)(absolute_sample[k]+0.5)) {
					final_values1[h] = short_segment[count1*8+channelnum[k]];
					time_data1[h] = absolute_time[count1];
					absolute_sample[k] += correct_sample[k];
					h++;
				} /* end if */
				count1++;
			} /* end for b */

			/* write data to shared memory files */
			check1 = cvio_add_samples(file_stream[k],time_data1,final_values1,h);
			if (check1) exit_cvio_error(check1);
		}
	} /* end for i */
		
	for(i=0;i<channel;i++) {
		check1 = cvio_close(file_stream[i]);
		if (check1) exit_cvio_error(check1);
	}
	 
	/* close audio ports */
	alClosePort(port1);
	alFreeConfig(portconfig); 

	/* free memory */
	free(short_segment);
	free(final_values1);
	free(absolute_time);
	free(time_data1);

	printf("done\n");

	return;
}

int main(int argc, char **argv)
{
	double duration, rate[8];
	int32  channel[8];
	char *filename[8];
	int32 num_channels, i;

	if (argc < 5) exit_cmd(argv[0]);

	num_channels = (argc-2)/3;

	if (num_channels > 8) num_channels = 8;

	for (i=0; i<num_channels; i++) {
		channel[i] = atoi(argv[i*3+2]);
		rate[i] = atof(argv[i*3+3]);
		filename[i] = argv[i*3+4];
	}

 	duration = atof(argv[1]);
	
	audio_port_setup(duration,num_channels,channel,rate,filename);
	
	exit(0);
}

void exit_cmd(char *s)
{
	fprintf(stderr,"(%s) %s duration [channel rate filename] ...\n",
		__DATE__,s);
	exit(1);
}

#else

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	fprintf(stderr,"%s is only supported under irix.\n",argv[0]);
	exit(1);
}

#endif
