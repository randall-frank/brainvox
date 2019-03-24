/* cvio_deconirf
  create IRF file for bin deconvolution
*/
#include <getopt.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "cvio_lib.h"

#define output(X)\
	cviomodel?ts=CVIO_TIME_NEXT,\
	err = cvio_add_samples(modelstream, &ts, tstr, 1):\
	fprintf(modelfile, tstr);

void print_usage(void)
{
	printf("cvio_deconirf <options> outputfiles\n");
	printf("output_stream must be CVIO_DT_FLOAT\n");
	printf("options:\n");
	printf("	-d don't create output stream (it already exists)\n");
	printf("	-l # total length of time (in timestamps)\n");
	printf("	-m <file> create model file for cvio_regress\n");
	printf("	-C model file should be written in cvio format\n");
}

int main(int argc, char **argv)
{
	char tstr[256];
	FILE *modelfile;
	CVIO_DT_TIME ts;
	uint32_t modelstream;
	float *dp;
	char ch, *modelname = NULL;
	int dontcreate = 0;
	int bins;
	int length;
	int cviomodel = 0;
	uint32_t *out;
	int32_t err;
	int32_t dims[3];
	int nsamps;
	int i, bin;
	int tinc;

	while ((ch = getopt(argc, argv, "l:dm:C")) != -1) {
		switch (ch) {
		case 'd':
			dontcreate = 1;
			break;
		case 'l':
			length = atoi(optarg);
			break;
		case 'm':
			modelname = strdup(optarg);
			break;
		case 'C':
			cviomodel = 1;
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1) {
		print_usage();
		return 1;
	}

	bins = argc;
	cvio_init();
	out = malloc(sizeof(uint32_t) * bins);
	if (!dontcreate) {
		nsamps = bins * 2 + 2;
		dims[0] = 1;
		dims[1] = 1;
		dims[2] = 1;
		for (i = 0; i < bins; i++) {
			err = cvio_create(argv[i], nsamps, CVIO_TYP_FLOAT, 1, dims, 0);
			if (err != CVIO_ERR_OK) {
				printf("Failed to create output\n");
				return 0;
			}
		}
	}
	for (i = 0; i < bins; i++) {
		err = cvio_open(argv[i], CVIO_ACCESS_APPEND, &out[i]);
		if (err != CVIO_ERR_OK) {
			printf("Failed to open output stream\n");
			return 0;
		}
	}
	dp = malloc(sizeof(float) * bins);
	tinc = length / bins;
	ts = 1;
	for (i = 0; i < bins; i++)
		dp[i] = 0;
	for (i = 0; i < bins; i++)
		err = cvio_add_samples(out[i], &ts, &dp[i], 1);
	for (bin = 0; bin < bins; bin++) {
		ts += 1;
		for (i = 0; i < bins; i++)
			dp[i] = 0;
		dp[bin] = 1.0;
		for (i = 0; i < bins; i++)
			err = cvio_add_samples(out[i], &ts, &dp[i], 1);
		ts += tinc - 1;
		for (i = 0; i < bins; i++)
			err = cvio_add_samples(out[i], &ts, &dp[i], 1);
	}
	ts++;
	for (i = 0; i < bins; i++)
		dp[i] = 0;
	for (i = 0; i < bins; i++)
		err = cvio_add_samples(out[i], &ts, &dp[i], 1);
	for (i = 0; i < bins; i++)
		cvio_close(out[i]);

	if (modelname) {
		if (cviomodel && !dontcreate) {
			int32_t dims[3] = {256, 1, 1};
			err = cvio_create(modelname, 10000, CVIO_TYP_STRING, 1, dims, 0);
			if (err != CVIO_ERR_OK) {
				printf("Can't create stream for writing\n");
				return 1;
			}
		}
		if (cviomodel) {
			err = cvio_open(modelname, CVIO_ACCESS_APPEND, &modelstream);
			if (err != CVIO_ERR_OK) {
				printf("Can't open model stream for writing\n");
				return 1;
			}
		} else {
			modelfile = fopen(modelname, "w");
			if (modelfile == NULL) {
				printf("Can't open model file for writing\n");
				return 1;
			}
		}
		
		sprintf(tstr, "VOLPARAMS\n");
		output(tstr);
		sprintf(tstr, "NUM_SLABS XXSLABCOUNT\n");
		output(tstr);
		sprintf(tstr, "MODEL\n");
		output(tstr);
		sprintf(tstr, "Y Y\n");
		output(tstr);
		sprintf(tstr, "CONST\n");
		output(tstr);
		for (i = 0; i < bins; i++) {
			sprintf(tstr, "COVARIABLE_VOL bin_%d\n", i);
			output(tstr);
		}
		sprintf(tstr, "OUTPUT\n");
		output(tstr);
		sprintf(tstr, "TIMESTAMPS XXTS\n");
		output(tstr);
		for (i = 0; i < bins; i++) {
			sprintf(tstr, "COVARIABLE_SLOPE bin_%d XXOUTPUT%d\n", i, i);
			output(tstr);
		}
		sprintf(tstr, "OBSERVATIONS\n");
		output(tstr);
		sprintf(tstr, "XXIMAGEFILE\n");
		output(tstr);
		sprintf(tstr, "1.0\n");
		output(tstr);
		for (i = 0; i < bins; i++) {
			sprintf(tstr, "XXINPUT%d\n", i);
			output(tstr);
		}
		sprintf(tstr, "END\n");
		output(tstr);
		if (cviomodel)
			cvio_close(modelstream);
		else
			fclose(modelfile);
	}
	cvio_cleanup(0);
	return 0;
}
