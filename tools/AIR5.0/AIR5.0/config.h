/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define as 1 for UNIX, 2 for PC */
#define AIR_CONFIG_PATHS 1

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

/* MACHINE GENERATED, non-zero if popen() and pclose() were found */
#define AIR_CONFIG_PIPES 1

/* MACHINE GENERATED, non-zero if gunzip was found */
#define AIR_CONFIG_DECOMPRESS 1

/* MACHINE GENERATED, 1 if unistd.h, sys/types.h, sys/stat.h and stat() were found, 2 if lstat() was also found */
#define AIR_CONFIG_REQ_PERMS 2

