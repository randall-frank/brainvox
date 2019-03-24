/*
 * external prototypes for the Unix .Z file format I/O system
 *
 * functions return number of bytes read/written.  On
 * error, they return negative numbers. Length is the length
 * of the buffer in bytes.
 *
 */

int read_compress_file(char *filename, int length, unsigned char *buffer);
int write_compress_file(char *filename, int length, unsigned char *buffer);
int size_compress_file(char *filename);

