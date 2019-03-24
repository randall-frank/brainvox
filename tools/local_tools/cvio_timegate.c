/*
 Temporally compress a binary stream - after a 1 sample, don't allow any more for a specified period of time
*/
#include <getopt.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "cvio_lib.h"

void print_usage(void)
{
	printf("cvio_timegate <options> inputfile outputfile\n");
	printf("options:\n");
	printf("	-d don't create output stream (it already exists)\n");
	printf("	-t <timestamps> Event length in timestamps\n");
}

int main(int argc, char **argv)
{
	CVIO_DT_TIME ts, last, k;
	uint32_t modelstream;
	uint8_t dp;
	char ch, *modelname = NULL;
	int dontcreate = 0;
	uint32_t in, out;
	int32_t err, count;
	uint32_t nsamps;
	int i, bin, iSize;
	int tinc;
	int32_t type, ndims, dims[3];
	char *ins;
	int threshold = 20000;

	while ((ch = getopt(argc, argv, "dt:")) != -1) {
		switch (ch) {
		case 'd':
			dontcreate = 1;
			break;
		case 't':
			threshold = atoi(optarg);
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 2) {
		print_usage();
		return 1;
	}

	cvio_init();

	err = cvio_open(argv[0], CVIO_ACCESS_READ, &in);
	if (err != CVIO_ERR_OK) {
		printf("Failed to open input stream %s, %d\n", argv[0], err);
		return 0;
	}
	err = cvio_datatype(in, &type, &ndims, dims);

	iSize = (type & CVIO_TYP_SIZE_MASK)*dims[0]*dims[1]*dims[2]/8;

	if (!dontcreate) {
		CVIO_DT_TIME dummyt;
		uint32_t dummys;
		printf("Creating %s\n", argv[1]);
		err = cvio_tell(in, &dummys, &dummyt, &nsamps);
		printf("%s, %d, %d, %d, %d:%d:%d\n", argv[1], nsamps, type, ndims, dims[0], dims[1], dims[2]);
		err = cvio_create(argv[1], nsamps, type, ndims, dims, 0);
		if (err != CVIO_ERR_OK) {
			printf("Failed to create output (%d)\n", err);
			return 0;
		}
	}
	err = cvio_open(argv[1], CVIO_ACCESS_APPEND, &out);
	if (err != CVIO_ERR_OK) {
		printf("Failed to open output stream\n");
		return 0;
	}

	assert(iSize < 2);
	last = CVIO_TIME_NEXT;
	while (err == CVIO_ERR_OK) {
		count = 1;
		err = cvio_read_next_samples_blocking(in, &ts, &dp, &count, 0, 0);
		if (err != CVIO_ERR_OK) {
			printf("It's not all good (%d)\n", err);
			break;
		}
		if (iSize == 0)
			dp = 1;
//keep resetting until we have at least threshold of no trigger
//		if (dp && last != CVIO_TIME_NEXT && last + threshold > ts) {
//			last = ts;
//		}
		if (!dp) {
			err = cvio_add_samples(out, &ts, &dp, 1);
			if (err != CVIO_ERR_OK)
				printf("Failed to add (%d)\n", err);
		} else if (last == CVIO_TIME_NEXT || last + threshold < ts) {
			err = cvio_add_samples(out, &ts, &dp, 1);
			if (err != CVIO_ERR_OK)
				printf("Failed to add (%d)\n", err);
			last = ts;
		}
	}
	cvio_close(in);
	cvio_close(out);
	cvio_cleanup(0);
	return 0;
}
