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
void audio_port_setup(double time, int32 channel, double *rate, char **file_name);
void exit_cvio_error(int32 err);	
void exit_cmd(char *s);

void exit_cvio_error(int32 err) 
{
	char	tstr[256];
	int32	tlen = 256;

	cvio_get_error_message(err,tstr,&tlen);
	fprintf(stderr,"Fatal error:%s\n",tstr);

	cvio_cleanup(0);
	exit(1);
}

void audio_port_setup(double time, int32 channel, double *rate, char **file_name)
{
	ALpv globalbuffer[2];
	float big_rate;
	double time_inc;
	int32 result, global_rate, i, k;

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

	if (channel == 1) 
	{
		/* declare the necessary variables */
		ALconfig portconfig;
		ALport port1;
		int32 i, b, h, total_frames, short_frames, count;
		int32 check1;
		float correct_sample1, absolute_sample;
		int16 final_values1[(int)rate[0]/2], short_segment[global_rate/2];
		CVIO_DT_TIME absolute_time[global_rate/2], time_data1[(int)rate[0]/2], temp, temp2;
		CVIO_DT_TIME basetime;
		int32 t, count1;
		uint32 file_stream1;
		int32	itype,ndims,dims[3];
		FILE *fp;
		
		/* desired number of samples to be collected */
		total_frames = global_rate*time;
		/* number of samples in one half second */
		short_frames = global_rate*0.5;

		/* determine sample interval to collect at desired rate */
		correct_sample1 = big_rate/rate[0];
		absolute_sample = 0.0;
		
		/* set up audio ports configuration with desired specifications */
		/* default ALconfig has two's complement format and 16-bit width */
		portconfig = alNewConfig();
		alSetQueueSize(portconfig, short_frames*3.0);
		alSetChannels(portconfig, 1);

		/* open one port up */
		port1 = alOpenPort("porta", "r", portconfig);
		
#ifdef DEBUG
		temp2 = cvio_current_time();
		fp = fopen("time","w");
		fprintf(fp,"initial time: %lu\n", temp2);  
#endif

		/* create a shared memory segment */
		check1 = cvio_open(file_name[0], CVIO_ACCESS_APPEND, &file_stream1);
		if (check1) exit_cvio_error(check1);

		check1 = cvio_datatype(file_stream1,&itype,&ndims,dims);
		if (check1) exit_cvio_error(check1);

		if ((ndims != 1) || (dims[0] != 1) || 
			(itype != CVIO_TYP_SHORT)) {
			exit_cvio_error(CVIO_ERR_BADDATATYPE);
		}
	
		for (i=0; i<time*2; i++) {
			/* get samples from port 1 */
			temp2 = cvio_current_time();
#ifdef DEBUG
			fprintf(fp,"before time: %ld\t%lu\n", i, temp2);  
#endif

			alReadFrames(port1,&short_segment,short_frames);

			/* get raw data and time values */
			if (i == 0) {
				basetime = cvio_current_time();
			}
			temp = basetime + ((short_frames*i)*time_inc);
			temp2 = cvio_current_time();
			if (abs(temp - temp2) > time_inc*1.5*correct_sample1) {
				fprintf(stderr,"Warning: timestamp overlap detected (%ul %f).\n",
					abs(temp - temp2),time_inc*1.5*correct_sample1);
			}
#ifdef DEBUG
			fprintf(fp,"after time: %ld\t%lu - %lu\n", i, temp, temp2);
#endif
			for (t=0; t<short_frames; t++) {
				absolute_time[t] = temp - ((double)(short_frames-t))*time_inc;
			} /* end for t */

			h=0; count1=0;
			for (b=short_frames*i; b<short_frames*(i+1); b++) {
				if (b == (int)(absolute_sample+0.5)) {
					final_values1[h] = short_segment[count1];
					time_data1[h] = absolute_time[count1];
					absolute_sample += correct_sample1;
					h++;
				} /* end if */
				count1++;
			} /* end for b */
#ifdef DEBUG	
			for (k=0; k<11; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values1[k], time_data1[k]);
			}
			for (k=h-11; k<h; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values1[k], time_data1[k]);
			}
			
#endif
			/* write data to shared memory files */
			check1 = cvio_add_samples(file_stream1,time_data1,final_values1,h);
			if (check1) exit_cvio_error(check1);
		} /* end for i */
		
		cvio_close(file_stream1);
		if (check1) exit_cvio_error(check1);

#ifdef DEBUG
		fclose(fp);		
#endif
	 
		/* close audio ports */
		alClosePort(port1);
		alFreeConfig(portconfig); 

		printf("done\n");
	}

#if 0
	else if (channel == 2) 
	{ 
		/* declare the necessary variables */
		ALconfig portconfig;
		ALport port1;
		int32 i, a, b, g=0, f=0, h=0, j=0, dim_size[3], short_frames;
		int32 correct_sample1, correct_sample2, total_frames, count;
		int16 stored_values1[global_rate/2], stored_values2[global_rate/2];
		int16 short_segment[global_rate*2/2];
		int16 final_values1[rate[0]/2], final_values2[rate[1]/2];
		CVIO_DT_TIME absolute_time[global_rate/2], temp, temp2;
		CVIO_DT_TIME time_data1[rate[0]/2], time_data2[rate[1]/2];
		int32 t, count1, count2, check1;
		uint32 file_stream1, file_stream2;
		char *prefix = ":shmem:", tstr[2048], shmem1[1024], shmem2[1024];
		FILE *fp;
		
		/* desired number of samples to be collected */
		total_frames = global_rate*time;
		/* number of samples in one half second */
		short_frames = global_rate*0.5;

		/* determine sample interval to collect at desired rate */
		correct_sample1 = big_rate/rate[0];
		correct_sample2 = big_rate/rate[1];
		
		/* set up audio ports configuration with desired specifications */
		/* default ALconfig has two's complement format and 16-bit width */
		portconfig = alNewConfig();
		alSetQueueSize(portconfig, 100000);
		alSetWidth(portconfig, AL_SAMPLE_16);
		alSetSampFmt(portconfig, AL_SAMPFMT_TWOSCOMP);
		alSetChannels(portconfig, 2);
		
		fp = fopen("time","w");

		/* open one port up */
		port1 = alOpenPort("porta", "r", portconfig);
		
		temp2 = cvio_current_time();
		fprintf(fp,"initial time: %lu\n", temp2); 
		
		/* find the size of a single sample in chars */
		for (i=0; i<3; i++) {
			dim_size[i] = 1;
		} /* end for i */

		/* create a shared memory segment */
		shmem1[0]='\0';
		shmem2[0]='\0';
		strcat(shmem1, prefix);
		strcat(shmem2, prefix);		
		strcat(shmem1, &file_name[0]);
		strcat(shmem2, &file_name[256]);
		check1 = cvio_create(shmem1, total_frames, CVIO_TYP_SHORT, 1, dim_size, 0);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_create(shmem2, total_frames, CVIO_TYP_SHORT, 1, dim_size, 0);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_open(shmem1, CVIO_ACCESS_APPEND, &file_stream1);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_open(shmem2, CVIO_ACCESS_APPEND, &file_stream2);
		if (check1) exit_cvio_error(check1);

		for (i=0; i<time*2; i++) {
			/* get samples from port 1 */
			temp2 = cvio_current_time();
			fprintf(fp,"before time: %ld\t%lu\n", i, temp2);

			alReadFrames(port1,&short_segment,short_frames);

			/* get raw data and time values */
			temp = cvio_current_time();
			fprintf(fp,"after time: %ld\t%lu\n", i, temp);

			for (t=0; t<short_frames; t++) {
				absolute_time[t] = temp - ((double)(short_frames-t))*time_inc;
			} /* end for t */

			g=0; f=0;
			for (a=0; a<short_frames*2; a++) {
				if ((a % 2) == 0) {
					stored_values1[g] = short_segment[a];
					g += 1;
				}
				else {
					stored_values2[f] = short_segment[a];
					f += 1;
				}
			}
					
			h=0; j=0; count1=0;
			for (b=short_frames*i; b<short_frames*(i+1); b++) {
				if ((b % correct_sample1) == 0) {
					final_values1[h] = stored_values1[count1];
					time_data1[h] = absolute_time[count1];
					h += 1;
				} /* end if */
				if ((b % correct_sample2) == 0) {
					final_values2[j] = stored_values2[count1];
					time_data2[j] = absolute_time[count1];
					j += 1;
				} /* end if */
				count1++;
			} /* end for b */
		
			for (k=0; k<11; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values1[k], time_data1[k]);
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values2[k], time_data2[k]);
			}
			for (k=h-11; k<h; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values1[k], time_data1[k]);
			}
			for (k=j-11; k<j; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values2[k], time_data2[k]);
			}

			/* write data to shared memory files */
			check1 = cvio_add_samples(file_stream1,time_data1,final_values1,h);
			if (check1) exit_cvio_error(check1);
			check1 = cvio_add_samples(file_stream2,time_data2,final_values2,j);
			if (check1) exit_cvio_error(check1);
		} /* end for i */

		cvio_close(file_stream1);
		if (check1) exit_cvio_error(check1);
		cvio_close(file_stream2);
		if (check1) exit_cvio_error(check1);

		fclose(fp);

		sprintf(tstr, "cvio_cp %s %s", &shmem1, &file_name[0]);
		system(tstr);
		sprintf(tstr, "cvio_cp %s %s", &shmem2, &file_name[256]);
		system(tstr);
		sprintf(tstr, "cvio_rm %s", &shmem1);
		system(tstr);
		sprintf(tstr, "cvio_rm %s", &shmem2);
		system(tstr);
	 
		/* close audio ports */
		alClosePort(port1);
		alFreeConfig(portconfig); 

		printf("done\n");
	}

	else if (channel == 3) 
	{	
		ALconfig portconfig;
		ALport port1;
		int32 total_frames, check1, count;
		int32 i, a, b, g=0, j=0, f=0, h=0, c, d, e, dim_size[3];
		int32 correct_sample1, correct_sample2, correct_sample3, short_frames;
		int16 stored_values1[global_rate/2];
		int16 stored_values3[global_rate/2], stored_values4[global_rate/2];
		int16 stored_values2[global_rate/2], final_values3[rate[2]/2];
		int16 final_values1[rate[0]/2], final_values2[rate[1]/2];
		int16 short_segment[global_rate*4/2];
		CVIO_DT_TIME absolute_time[global_rate/2], time_data1[rate[0]/2];
		CVIO_DT_TIME time_data2[rate[1]/2], time_data3[rate[2]/2], temp, temp2;
		int32 t, count1, count2, count3;
		uint32 file_stream1, file_stream2, file_stream3;
		char *prefix = ":shmem:", tstr[2048], shmem1[1024], shmem2[1024], shmem3[1024];
		FILE *fp;

		/* desired number of samples to be collected */
		total_frames = global_rate*time;
		/* number of samples in one half second */
		short_frames = global_rate*0.5;

		/* determine sample interval to collect at desired rate */
		correct_sample1 = big_rate/rate[0];
		correct_sample2 = big_rate/rate[1];
		correct_sample3 = big_rate/rate[2];
		
		/* set up audio ports configuration with desired specifications */
		/* default ALconfig has two's complement format and 16-bit width */
		portconfig = alNewConfig();
		alSetQueueSize(portconfig, 100000);
		alSetWidth(portconfig, AL_SAMPLE_16);
		alSetSampFmt(portconfig, AL_SAMPFMT_TWOSCOMP);
		alSetChannels(portconfig, 4);
		
		fp = fopen("time","w");

		/* open one port up */
		port1 = alOpenPort("porta", "r", portconfig);
		
		temp2 = cvio_current_time();
		fprintf(fp,"initial time: %lu\n", temp2); 
		
		/* find the size of a single sample in chars */
		for (i=0; i<3; i++) {
			dim_size[i] = 1;
		} /* end for i */
	
		/* create a shared memory segment */
		shmem1[0]='\0';
		shmem2[0]='\0';
		shmem3[0]='\0';
		strcat(shmem1, prefix);
		strcat(shmem2, prefix);	
		strcat(shmem3, prefix);		
		strcat(shmem1, &file_name[0]);
		strcat(shmem2, &file_name[256]);
		strcat(shmem3, &file_name[512]);
		check1 = cvio_create(shmem1, total_frames, CVIO_TYP_SHORT, 1, dim_size, 0);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_create(shmem2, total_frames, CVIO_TYP_SHORT, 1, dim_size, 0);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_create(shmem3, total_frames, CVIO_TYP_SHORT, 1, dim_size, 0);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_open(shmem1, CVIO_ACCESS_APPEND, &file_stream1);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_open(shmem2, CVIO_ACCESS_APPEND, &file_stream2);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_open(shmem3, CVIO_ACCESS_APPEND, &file_stream3);
		if (check1) exit_cvio_error(check1);

		for (i=0; i<time*2; i++) {
			/* get samples from port 1 */
			temp2 = cvio_current_time();
			fprintf(fp,"before time: %ld\t%lu\n", i, temp2);

			alReadFrames(port1,&short_segment,short_frames);

			/* get raw data and time values */
			temp = cvio_current_time();
			fprintf(fp,"after time: %ld\t%lu\n", i, temp);

			for (t=0; t<short_frames; t++) {
				absolute_time[t] = temp - ((double)(short_frames-t))*time_inc;
			} /* end for t */

			f=0; g=0; h=0; j=0;
			for (a=0; a<short_frames*4; a++) {
				if ((a % 4) == 0) {
					stored_values1[f] = short_segment[a];
					f += 1;
				}
				else if (((a-1) % 4) == 0) {
					stored_values2[g] = short_segment[a];
					g += 1;
				}
				else if (((a-2) % 4) == 0) {
					stored_values3[h] = short_segment[a];
					h += 1;
				}
				else {
					stored_values4[j] = short_segment[a];
					j += 1;
				} 				
			}
					
			c=0; d=0; e=0; count1=0;
			for (b=short_frames*i; b<short_frames*(i+1); b++) {
				if ((b % correct_sample1) == 0) {
					final_values1[c] = stored_values1[count1];
					time_data1[c] = absolute_time[count1];
					c += 1;
				} /* end if */
				if ((b % correct_sample2) == 0) {
					final_values2[d] = stored_values2[count1];
					time_data2[d] = absolute_time[count1];
					d += 1;
				} /* end if */
				if ((b % correct_sample3) == 0) {
					final_values3[e] = stored_values3[count1];
					time_data3[e] = absolute_time[count1];
					e += 1;
				} /* end if */
				count1++;
			} /* end for b */
			
			for (k=0; k<11; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values1[k], time_data1[k]);
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values2[k], time_data2[k]);
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values3[k], time_data3[k]);
			}
			for (k=c-11; k<c; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values1[k], time_data1[k]);
			}
			for (k=d-11; k<d; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values2[k], time_data2[k]);
			}
			for (k=e-11; k<e; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values3[k], time_data3[k]);
			}

			/* write data to shared memory files */
			check1 = cvio_add_samples(file_stream1,time_data1,final_values1,c);
			if (check1) exit_cvio_error(check1);
			check1 = cvio_add_samples(file_stream2,time_data2,final_values2,d);
			if (check1) exit_cvio_error(check1);
			check1 = cvio_add_samples(file_stream3,time_data3,final_values3,e);
			if (check1) exit_cvio_error(check1);
		} /* end for i */
		
		cvio_close(file_stream1);
		if (check1) exit_cvio_error(check1);
		cvio_close(file_stream2);
		if (check1) exit_cvio_error(check1);
		cvio_close(file_stream3);
		if (check1) exit_cvio_error(check1);

		fclose(fp);

		sprintf(tstr, "cvio_cp %s %s", &shmem1, &file_name[0]);
		system(tstr);
		sprintf(tstr, "cvio_cp %s %s", &shmem2, &file_name[256]);
		system(tstr);
		sprintf(tstr, "cvio_cp %s %s", &shmem3, &file_name[512]);
		system(tstr);		
		sprintf(tstr, "cvio_rm %s", &shmem1);
		system(tstr);
		sprintf(tstr, "cvio_rm %s", &shmem2);
		system(tstr);
		sprintf(tstr, "cvio_rm %s", &shmem3);
		system(tstr);
	 
		/* close audio ports */
		alClosePort(port1);
		alFreeConfig(portconfig); 

		printf("done\n");
	}

	else if (channel == 4) 
	{	

		ALconfig portconfig;
		ALport port1;
		int32 total_frames, check1, count;
		int32 i, a, b, g=0, j=0, k=0, f=0, h=0, c, d, e, z, dim_size[3];
		int32 correct_sample1, correct_sample2, correct_sample3, short_frames, correct_sample4;
		int16 stored_values1[global_rate/2];
		int16 stored_values3[global_rate/2], stored_values4[global_rate/2];
		int16 stored_values2[global_rate/2], final_values3[rate[2]/2];
		int16 final_values1[rate[0]/2], final_values2[rate[1]/2], final_values4[rate[3]/2];
		int16 short_segment[global_rate*4/2];
		CVIO_DT_TIME absolute_time[global_rate/2], time_data1[rate[0]/2];
		CVIO_DT_TIME time_data2[rate[1]/2], time_data3[rate[2]/2];
		CVIO_DT_TIME time_data4[rate[3]/2], temp, temp2;
		int32 t, count1, count2, count3, count4;
		uint32 file_stream1, file_stream2, file_stream3, file_stream4;
		char *prefix = ":shmem:", tstr[2048], shmem1[1024], shmem2[1024];
		char shmem4[1024], shmem3[1024];
		FILE *fp;

		/* desired number of samples to be collected */
		total_frames = global_rate*time;
		/* number of samples in one half second */
		short_frames = global_rate*0.5;

		/* determine sample interval to collect at desired rate */
		correct_sample1 = big_rate/rate[0];
		correct_sample2 = big_rate/rate[1];
		correct_sample3 = big_rate/rate[2];
		correct_sample4 = big_rate/rate[3];
		
		/* set up audio ports configuration with desired specifications */
		/* default ALconfig has two's complement format and 16-bit width */
		portconfig = alNewConfig();
		alSetQueueSize(portconfig, 100000);
		alSetWidth(portconfig, AL_SAMPLE_16);
		alSetSampFmt(portconfig, AL_SAMPFMT_TWOSCOMP);
		alSetChannels(portconfig, 4);
			
		fp = fopen("time","w");

		/* open one port up */
		port1 = alOpenPort("porta", "r", portconfig);
		
		temp2 = cvio_current_time();
		fprintf(fp,"initial time: %lu\n", temp2); 
		
		/* find the size of a single sample in chars */
		for (i=0; i<3; i++) {
			dim_size[i] = 1;
		} /* end for i */
			
		/* create a shared memory segment */
		shmem1[0]='\0';
		shmem2[0]='\0';
		shmem3[0]='\0';
		shmem4[0]='\0';	
		strcat(shmem1, prefix);
		strcat(shmem2, prefix);	
		strcat(shmem3, prefix);
		strcat(shmem4, prefix);
		strcat(shmem1, &file_name[0]);
		strcat(shmem2, &file_name[256]);
		strcat(shmem3, &file_name[512]);
		strcat(shmem4, &file_name[768]);
		check1 = cvio_create(shmem1, total_frames, CVIO_TYP_SHORT, 1, dim_size, 0);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_open(shmem1, CVIO_ACCESS_APPEND, &file_stream1);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_create(shmem2, total_frames, CVIO_TYP_SHORT, 1, dim_size, 0);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_open(shmem2, CVIO_ACCESS_APPEND, &file_stream2);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_create(shmem3, total_frames, CVIO_TYP_SHORT, 1, dim_size, 0);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_open(shmem3, CVIO_ACCESS_APPEND, &file_stream3);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_create(shmem4, total_frames, CVIO_TYP_SHORT, 1, dim_size, 0);
		if (check1) exit_cvio_error(check1);
		check1 = cvio_open(shmem4, CVIO_ACCESS_APPEND, &file_stream4);
		if (check1) exit_cvio_error(check1);

		for (i=0; i<time*2; i++) {
			/* get samples from port 1 */
			temp2 = cvio_current_time();
			fprintf(fp,"before time: %ld\t%lu\n", i, temp2);

			alReadFrames(port1,&short_segment,short_frames);

			/* get raw data and time values */
			temp = cvio_current_time();
			fprintf(fp,"after time: %ld\t%lu\n", i, temp);

			for (t=0; t<short_frames; t++) {
				absolute_time[t] = temp - ((double)(short_frames-t))*time_inc;
			} /* end for t */

			f=0; g=0; h=0; j=0;
			for (a=0; a<short_frames*4; a++) {
				if ((a % 4) == 0) {
					stored_values1[f] = short_segment[a];
					f += 1;
				}
				else if (((a-1) % 4) == 0) {
					stored_values2[g] = short_segment[a];
					g += 1;
				}
				else if (((a-2) % 4) == 0) {
					stored_values3[h] = short_segment[a];
					h += 1;
				}
				else {
					stored_values4[j] = short_segment[a];
					j += 1;
				} 				
			}
					
			c=0; d=0; e=0; z=0; count1=0;
			for (b=short_frames*i; b<short_frames*(i+1); b++) {
				if ((b % correct_sample1) == 0) {
					final_values1[c] = stored_values1[count1];
					time_data1[c] = absolute_time[count1];
					c += 1;
				} /* end if */
				if ((b % correct_sample2) == 0) {
					final_values2[d] = stored_values2[count1];
					time_data2[d] = absolute_time[count1];
					d += 1;
				} /* end if */
				if ((b % correct_sample3) == 0) {
					final_values3[e] = stored_values3[count1];
					time_data3[e] = absolute_time[count1];
					e += 1;
				} /* end if */
				if ((b % correct_sample4) == 0) {
					final_values4[z] = stored_values4[count1];
					time_data4[z] = absolute_time[count1];
					z += 1;
				} /* end if */
				count1++;
			} /* end for b */
			
			
			for (k=0; k<11; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values1[k], time_data1[k]);
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values2[k], time_data2[k]);
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values3[k], time_data3[k]);
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values4[k], time_data4[k]);
			}
			for (k=c-11; k<c; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values1[k], time_data1[k]);
			}
			for (k=d-11; k<d; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values2[k], time_data2[k]);
			}
			for (k=e-11; k<e; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values3[k], time_data3[k]);
			}
			for (k=z-11; k<z; k++) {
				fprintf(fp,"%ld\t%ld\t%ld\t%lu\n", i, k, final_values4[k], time_data4[k]);
			}

			/* write data to shared memory files */
			check1 = cvio_add_samples(file_stream1,time_data1,final_values1,c);
			if (check1) exit_cvio_error(check1);
			check1 = cvio_add_samples(file_stream2,time_data2,final_values2,d);
			if (check1) exit_cvio_error(check1);
			check1 = cvio_add_samples(file_stream3,time_data3,final_values3,e);
			if (check1) exit_cvio_error(check1);
			check1 = cvio_add_samples(file_stream4,time_data4,final_values4,z);
			if (check1) exit_cvio_error(check1);
		} /* end for i */
	
		cvio_close(file_stream1);
		if (check1) exit_cvio_error(check1);
		cvio_close(file_stream2);
		if (check1) exit_cvio_error(check1);
		cvio_close(file_stream3);
		if (check1) exit_cvio_error(check1);
		cvio_close(file_stream4);
		if (check1) exit_cvio_error(check1);

		fclose(fp);

		sprintf(tstr, "cvio_cp %s %s", &shmem1, &file_name[0]);
		system(tstr);
		sprintf(tstr, "cvio_cp %s %s", &shmem2, &file_name[256]);
		system(tstr);
		sprintf(tstr, "cvio_cp %s %s", &shmem3, &file_name[512]);
		system(tstr);
		sprintf(tstr, "cvio_cp %s %s", &shmem3, &file_name[768]);
		system(tstr);		
		sprintf(tstr, "cvio_rm %s", &shmem1);
		system(tstr);
		sprintf(tstr, "cvio_rm %s", &shmem2);
		system(tstr);
		sprintf(tstr, "cvio_rm %s", &shmem3);
		system(tstr);
		sprintf(tstr, "cvio_rm %s", &shmem4);
		system(tstr);

		/* close audio ports */
		alClosePort(port1);
		alFreeConfig(portconfig); 

		printf("done\n");
	}
#endif
}

int main(int argc, char **argv)
{
	double duration, rate[8];
	char *filename[8];
	int32 num_channels, i;

	if (argc & 1) exit_cmd(argv[0]);

	num_channels = argc/2 - 1;

	if (num_channels > 8) num_channels = 8;

	for (i=0; i<num_channels; i++) {
		rate[i] = atof(argv[i*2+2]);
		filename[i] = argv[i*2+3];
	}

 	duration = atof(argv[1]);
	
	audio_port_setup(duration,num_channels,rate,filename);
	
	exit(0);
}

void exit_cmd(char *s)
{
	fprintf(stderr,"(%s) %s duration [rate channelname] ...\n",
		__DATE__,s);
	exit(1);
}
