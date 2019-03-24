/* cvio_mux - combine multiple digital streams into a stream of shorts */
/* most ineffecient cvio util ever. */

#include <getopt.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "cvio_lib.h"

void print_usage(void)
{
	printf("cvio_mux <options> output_stream input_stream(s)\n");
	printf("output_stream must be CVIO_DT_SHORT, and the\n");
	printf("input streams must be boolean or impulse\n");
	printf("The maximum number of input streams is 16\n");
	printf("options:\n");
	printf("	-d don't create output stream (it already exists)\n");
	printf("	-b #  bounce filter threshold in timestamps\n");
}

struct buffer_args {
	pthread_t thread;
	pthread_mutex_t bufflock;
	pthread_cond_t buffcond;
	pthread_cond_t sleepcond[16];
	uint32_t outstream;
	uint32_t instream[16];
	CVIO_DT_TIME ts[16];
	int8_t data[16];
	int numthreads;    //number of running threads
	int total_streams; //total number of streams at start
	int blockers;
};

struct read_args {
	pthread_t thread;
	int num;
	uint32_t stream;
	struct buffer_args *buf;
};

void *buffer_thread(void *vargs)
{
	uint16_t mask;
	uint16_t sample = 0, last_sample = 0;
	CVIO_DT_TIME lowest_time;
	int i;

	struct buffer_args *args = vargs;

	pthread_mutex_lock(&args->bufflock);
	pthread_cond_signal(&args->buffcond); //startup
	while (args->numthreads > 0) {
//		printf("buffer loop starts, %d blockers\n", args->blockers);
		while (args->blockers < args->numthreads) {
			pthread_cond_wait(&args->buffcond, &args->bufflock);
//			printf("%d < %d\n", args->blockers, args->numthreads);
		}
		// we have one sample
//		printf("Everyone's in, processing sample\n");
		lowest_time = CVIO_TIME_NEXT;
		for (i = 0; i < args->total_streams; i++) {
//			printf("Stream %d, time %u\n", i, args->ts[i]);
			if ((args->instream[i] != args->outstream) && (args->ts[i] < lowest_time))
				lowest_time = args->ts[i];
		}
		for (i = 0; i < args->total_streams; i++) {
			if ((args->instream[i] != args->outstream) && (args->ts[i] == lowest_time)) {
				mask = 1 << i;
				if (args->data[i])
					sample |= mask;
				else sample &= ~mask;
//				printf("Waking %d\n", i);
				pthread_cond_signal(&(args->sleepcond[i]));
				args->blockers--;
			}
		}

// either write new data, or update the stream's last sample
		if (sample != last_sample) {
			cvio_add_samples(args->outstream, &lowest_time, &sample, 1);
			last_sample = sample;
//			printf("New data, %d\n", sample);
		} else {
			cvio_add_samples(args->outstream, &lowest_time, NULL, 0);
		}
	}
	cvio_close(args->outstream);
	return NULL;
}

void *read_thread(void *vargs)
{
	struct read_args *args = vargs;
	int8_t data = 0;
	uint16_t rw = 1;
	CVIO_DT_TIME lasttime = 0, ts = 0;
	int32_t timelen = sizeof(lasttime), rwlen = sizeof(rw);
	int32_t err;

	while (1) {
		do {
			cvio_getattribute(args->stream, "REQ_ATTR_TIMEOUT_TI", &lasttime, &timelen);
			assert(timelen == sizeof(lasttime));
			cvio_getattribute(args->stream, "REQ_ATTR_RWRITES_US", &rw, &rwlen);
			assert(rwlen == sizeof(rw));
			usleep(1000);
		} while ((lasttime == ts) && (rw != 0)) ;

		ts = CVIO_TIME_NEXT;
		err = cvio_read_samples(args->stream, &ts, &data, 1);
		pthread_mutex_lock(&args->buf->bufflock);
		if ((err != CVIO_ERR_OK) && (rw == 0)) {
			args->buf->numthreads--;
			args->buf->instream[args->num] = args->buf->outstream;
			pthread_cond_signal(&args->buf->buffcond);
			pthread_mutex_unlock(&args->buf->bufflock);
			cvio_close(args->stream);
			return NULL;
		}
		if (ts == CVIO_TIME_NEXT)
			ts = lasttime;

		args->buf->data[args->num] = data;
		args->buf->ts[args->num] = ts;
//printf("Data submission, thread %d, data %d, ts %u\n", args->num, data, ts);
		pthread_cond_signal(&args->buf->buffcond);
		args->buf->blockers++;
		pthread_cond_wait(&args->buf->sleepcond[args->num], &args->buf->bufflock);
//		printf("%d wakes up\n", args->num);
		pthread_mutex_unlock(&args->buf->bufflock);
	}
}

int main(int argc, char **argv)
{
	struct buffer_args bargs;
	char ch;
	int dontcreate = 0;
	int bflen;
	int32_t err;
	int i, started = 0;

	memset(&bargs.ts, 0, sizeof(bargs.ts));
	memset(&bargs.data, 0, sizeof(bargs.data));
	bargs.blockers = 0;
	bargs.numthreads = 0;
	bargs.total_streams = 0;
	pthread_mutex_init(&bargs.bufflock, NULL);
	pthread_cond_init(&bargs.buffcond, NULL);
	for (i = 0; i < 16; i++) {
		pthread_cond_init(&bargs.sleepcond[i], NULL);
	}

	while ((ch = getopt(argc, argv, "db:")) != -1) {
		switch (ch) {
		case 'd':
			dontcreate = 1;
			break;
		case 'b':
			bflen = atoi(optarg);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 2) {
		print_usage();
		return 1;
	}

	if (!dontcreate) {
		// would create stream here...
//chicken+egg, need outstream for sentinel, need input streams for file size.
	}
	err = cvio_open(argv[0], CVIO_ACCESS_APPEND, &bargs.outstream);
	if (err != CVIO_ERR_OK) {
		printf("Failed to open output stream\n");
		return 0;
	}
	for (i = 1; i < argc; i++) {
		bargs.total_streams++;
		err = cvio_open(argv[i], CVIO_ACCESS_READ, &bargs.instream[i - 1]);
		if (err != CVIO_ERR_OK) {
			printf("Stream %s is invalid\n", argv[i]);
			bargs.instream[i - 1] = bargs.outstream; // only sentinel I can think of...
		} else {
			bargs.numthreads++;
		}
	}
	printf("Will spawn %d threads for %d attempted streams\n", bargs.numthreads, bargs.total_streams);
	pthread_mutex_lock(&bargs.bufflock);
	pthread_create(&bargs.thread, NULL, buffer_thread, &bargs);
	pthread_cond_wait(&bargs.buffcond, &bargs.bufflock);

	for (i = 0; i < bargs.total_streams; i++) {
		struct read_args *ra;
		ra = malloc(sizeof(struct read_args));
		ra->num = i;
		ra->stream = bargs.instream[i];
		ra->buf = &bargs;
		if (bargs.instream[i] != bargs.outstream) {
			pthread_create(&ra->thread, NULL, read_thread, ra);
			started++;
		}
	}
	pthread_mutex_unlock(&bargs.bufflock);

	pthread_join(bargs.thread, NULL);
	return 1;
}
