/*
 combines many cvio streams into one higher dimensional cvio stream.
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
	printf("cvio_combine <options> inputfiles outputfile\n");
	printf("options:\n");
	printf("	-d don't create output stream (it already exists)\n");
}

int main(int argc, char **argv)
{
	CVIO_DT_TIME ts;
	uint32_t modelstream;
	char *dp;
	char ch, *modelname = NULL;
	int dontcreate = 0;
	uint32_t *in, out;
	int32_t err, count;
	uint32_t nsamps;
	int i, bin, iSize;
	int tinc;
	int inputs;
	int32_t type, ndims, dims[3];
	int32_t ltype, lndims, ldims[3];
	char *ins;

	while ((ch = getopt(argc, argv, "d")) != -1) {
		switch (ch) {
		case 'd':
			dontcreate = 1;
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 2) {
		print_usage();
		return 1;
	}

	inputs = argc - 1;
	cvio_init();

	in = malloc(sizeof(uint32_t) * inputs);
	for (i = 0; i < inputs; i++) {
		err = cvio_open(argv[i], CVIO_ACCESS_READ, &in[i]);
		if (err != CVIO_ERR_OK) {
			printf("Failed to open input stream %s, %d\n", argv[i], err);
			return 0;
		}
		err = cvio_datatype(in[i], &type, &ndims, dims);
		if (i != 0) {
			if (ltype != type
			 || ndims != ndims
			 || 0 != memcmp(dims, ldims, 3 * sizeof(uint32_t))) {
				printf("Stream types don't match, %s vs %s\n", argv[i], argv[i - 1]);
				return 0;
			}
		}
		ltype = type;
		lndims = ndims;
		memcpy(ldims, dims, 3 * sizeof(uint32_t));
	}

	iSize = (type & CVIO_TYP_SIZE_MASK)*dims[0]*dims[1]*dims[2]/8;
	dims[ndims] = inputs;
	ndims += 1;

	if (!dontcreate) {
		CVIO_DT_TIME dummyt;
		uint32_t dummys;
		printf("Creating %s\n", argv[inputs]);
		err = cvio_tell(in[0], &dummys, &dummyt, &nsamps);
		printf("%s, %d, %d, %d, %d:%d:%d\n", argv[inputs], nsamps, type, ndims, dims[0], dims[1], dims[2]);
		err = cvio_create(argv[inputs], nsamps, type, ndims, dims, 0);
		if (err != CVIO_ERR_OK) {
			printf("Failed to create output (%d)\n", err);
			return 0;
		}
	}
	err = cvio_open(argv[inputs], CVIO_ACCESS_APPEND, &out);
	if (err != CVIO_ERR_OK) {
		printf("Failed to open output stream\n");
		return 0;
	}
	dp = malloc(iSize * inputs);
	while (err == CVIO_ERR_OK) {
		for (i = 0; i < inputs; i++) {
			count = 1;
			err = cvio_read_next_samples_blocking(in[i], &ts, &dp[i * iSize], &count, 0, 0);
			if (err != CVIO_ERR_OK) {
				break;
			}
	//make sure ts is the same across all
	//error out if need be
		}
		if (err)
			break;
		err = cvio_add_samples(out, &ts, dp, 1);
		if (err != CVIO_ERR_OK)
			break;
	}
	for (i = 0; i < inputs; i++)
		cvio_close(in[i]);
	cvio_close(out);
	cvio_cleanup(0);
	return 0;
}
