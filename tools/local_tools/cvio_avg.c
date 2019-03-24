#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <cvio_lib.h>
#include <assert.h>

int main(int argc, char **argv)
{
	int slices = 20;
	CVIO_DT_TIME ots, ts;
	float *data;
	uint8_t *maskbuffer;
	int32_t err, ndims, datatype, dims[3], masksize;
	uint32_t maskstream, datastream, outstream;
	int i, voxcount = 0, slice = 0;
	float volsum = 0.0;
	int32_t count;

	cvio_init();
	err = cvio_open(argv[1], CVIO_ACCESS_READ, &maskstream);
	err = cvio_datatype(maskstream, &datatype, &ndims, dims);
	masksize = dims[0] * dims[1];
	maskbuffer = malloc(masksize);
	data = malloc(sizeof(float) * masksize);
	count = 1;
	while (1) {
		err = cvio_read_next_samples_blocking(maskstream, &ts, maskbuffer, &count, 0, 0);
		if (err != CVIO_ERR_OK)
			break;
		for (i = 0; i < masksize; i++)
			if (maskbuffer[i])
				voxcount++;
	}
	printf("%d voxels in mask\n", voxcount);

	err = cvio_open(argv[2], CVIO_ACCESS_READ, &datastream);
	assert(err == CVIO_ERR_OK);
	err = cvio_open(argv[3], CVIO_ACCESS_APPEND, &outstream);
	do {
		count = 1;
		err = cvio_read_next_samples_blocking(datastream, &ts, data, &count, 0, 0);
		if (err != CVIO_ERR_OK)
			break;
		for (i = 0; i < masksize; i++)
			volsum += data[i];
		slice++;
		if (slice > slices) {
			float out;
			slice = 0;
			printf("%f\n", volsum/(float)voxcount);
			out = volsum/(float)voxcount;
			err = cvio_add_samples(outstream, &ts, &out, 1);
			volsum = 0.0;
		}
	} while (err == CVIO_ERR_OK);
	cvio_close(datastream);
	cvio_close(outstream);
	return 0;
}
