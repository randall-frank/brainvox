#include <stdlib.h>
#include <stdio.h>
#include <cvio_lib.h>
#include <assert.h>

int main(int argc, char **argv)
{
	CVIO_DT_TIME ots, ts;
	int32_t err;
	uint32_t stream;
	err = cvio_open(argv[1], CVIO_ACCESS_READ, &stream);
	printf("%ld\n", err);
	assert(err == CVIO_ERR_OK);
	err = cvio_read_next_samples(stream, &ots, NULL, 1);
	assert(err == CVIO_ERR_OK);
	do {
		err = cvio_read_next_samples(stream, &ts, NULL, 1);
		printf("%ld\n", ts - ots);
		ots = ts;
	} while (err == CVIO_ERR_OK);
	return 0;
}
