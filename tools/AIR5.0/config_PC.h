/* This header file should only be used to configure PC's running under a non-Linux 
 * (e.g., Windows, Windows NT, MS-DOS) operating system
 */


/* Define as 1 unless non-compliant with ISO/IEEE Std 754-1985 */
#define AIR_CONFIG_AUTO_BYTESWAP 1

/* Define as 16 unless you want an 8 bit version of AIR, in which case define as 8 */
#define AIR_CONFIG_OUTBITS 16

/* Relevant only when AIR_CONFIG_OUTBITS is 16, can be 1, 2 or 3 */
#define AIR_CONFIG_REP16 1

/* Should be less than 256 if AIR_CONFIG_OUTBITS is 8 */
#define AIR_CONFIG_THRESHOLD1 7000

/* Should be less than 256 is AIR_CONFIG_OUTBITS is 8 */
#define AIR_CONFIG_THRESHOLD2 7000

/* Define as 1 if nonessential screen printing is desired */
#define AIR_CONFIG_VERBOSITY 0

/* voxel sizes differing by less than this amount are assumed identical */
#define AIR_CONFIG_PIX_SIZE_ERR .0001

