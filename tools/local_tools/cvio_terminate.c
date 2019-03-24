#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include "cvio_lib.h"

struct threaddata {
	uint32_t stream;
};

pthread_mutex_t outlock;
pthread_cond_t outcond;

float threshold = 10000.0;
float arm_level = 50.0;
CVIO_DT_TIME outts = CVIO_TIME_NEXT;
CVIO_DT_TIME start_ts;

int criteria(float a)
{
	static int armed = 0;

	if (armed == 0 && a >= arm_level) {
		printf("cvio_terminate is armed\n");
		armed = 1;
	}

	if (!armed)
		return 0;
	printf("%f < %f = %d\n", a, threshold, a < threshold);
	return a < threshold;
}

void print_usage(void)
{
	printf("cvio_terminate <options> acquire_control criteria_stream output_stream\n");
	printf("options:");
	printf("-t # threshold value\n");
	printf("-d   don't create output stream (it already exists)\n");
}

void *acquire_control_thread(void *args)
{
	int32_t err;
	CVIO_DT_TIME ts;
	struct threaddata *td = args;
	int count;

	count = 1;
	err = cvio_read_next_samples_blocking(td->stream, &ts, NULL, &count, 0, 0);
	pthread_mutex_lock(&outlock);
	outts = ts;
	pthread_cond_signal(&outcond);
	printf("Acquire control terminated run\n");
	pthread_mutex_unlock(&outlock);
	return NULL;
}

void *threshold_thread(void *args)
{
	int32_t err;
	struct threaddata *td = args;
	float data[4096];
	int i, done = 0, count;
	CVIO_DT_TIME ts[4096];

	while (!done) {
		count = 4096;
		err = cvio_read_next_samples_blocking(td->stream, ts, data, &count, 0, 0);
		if (err == CVIO_ERR_EOF) {
			ts[0] = cvio_current_time();
			i = 0;
			break;
		}
		for (i = 0; i < count; i++)
			if (criteria(data[i])) {
				done = 1;
				break;
			}
	}
	pthread_mutex_lock(&outlock);
	outts = ts[i] + start_ts;
	pthread_cond_signal(&outcond);
	printf("Criteria terminated run, TS = %d\n", ts[i]);
	pthread_mutex_unlock(&outlock);
	return NULL;
}

int main(int argc, char **argv)
{
	int count = 1;
	int dontcreate = 0;
	int32_t out = 0;
	int32_t err;
	uint32_t outstream;
	struct threaddata ctd, ttd;
	pthread_t ct, tt;
	char *controlname, *thresholdname, *outputname;
	char ch;

	while ((ch = getopt(argc, argv, "dt:")) != -1) {
		switch (ch) {
			case 't':
				threshold = atof(optarg);
printf("termination threshold is %f\n", threshold);
				break;
			case 'd':
				dontcreate = 1;
				break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 3) {
		print_usage();
		return 1;
	}

	controlname = argv[0];
	thresholdname = argv[1];
	outputname = argv[2];

	cvio_init();
	err = cvio_open(controlname, CVIO_ACCESS_READ, &ctd.stream);
	assert(err == CVIO_ERR_OK);
	err = cvio_open(thresholdname, CVIO_ACCESS_READ, &ttd.stream);
	assert(err == CVIO_ERR_OK);
	err = cvio_open(outputname, CVIO_ACCESS_APPEND, &outstream);
	assert(err == CVIO_ERR_OK);

	count = 1;
	err = cvio_read_next_samples_blocking(ctd.stream, &start_ts, NULL, &count, 0, 0);
	assert(err == CVIO_ERR_OK);
	if (err == CVIO_ERR_OK) {
		err = cvio_add_samples(outstream, &start_ts, NULL, 1);
		assert(err == CVIO_ERR_OK);
		pthread_mutex_init(&outlock, NULL);
		pthread_cond_init(&outcond, NULL);
		pthread_mutex_lock(&outlock);
		pthread_create(&ct, NULL, acquire_control_thread, &ctd);
		pthread_create(&tt, NULL, threshold_thread, &ttd);
		pthread_cond_wait(&outcond, &outlock);
		err = cvio_add_samples(outstream, &outts, &out, 1);
	}
	cvio_close(ctd.stream);
	cvio_close(ttd.stream);
	cvio_close(outstream);
	cvio_cleanup(0);
	return 0;
}
