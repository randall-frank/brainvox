MODULE idl_tools
DESCRIPTION IDL tools extensions
VERSION 1.3.1
SOURCE Randall Frank
BUILD_DATE  29 May 2001
FUNCTION	SOCK_CREATEPORT		1	1
FUNCTION	SOCK_CLOSE		1	1
FUNCTION	SOCK_CONNECT		2	2
FUNCTION	SOCK_ACCEPT		1	1
FUNCTION	SOCK_SEND		2	2
FUNCTION	SOCK_RECV		2	2	KEYWORDS
FUNCTION	SOCK_QUERY		1	1	KEYWORDS
FUNCTION	SOCK_SENDVAR		2	2
FUNCTION	SOCK_RECVVAR		2	2
FUNCTION	SOCK_SELECT		2	2
FUNCTION	SOCK_NAME2HOST		0	1
FUNCTION	SOCK_HOST2NAME		0	1
FUNCTION	SHMEM_ALLOC		1	1
FUNCTION	SHMEM_ALLOCVAR		2	2
FUNCTION	SHMEM_FREE		1	1
FUNCTION	SHMEM_NUM_ATTACHED	1	1
FUNCTION	SHMEM_ATTACH		3	3
FUNCTION	SHMEM_ATTACHVAR		1	1
FUNCTION	SND_PLAY		2	2
FUNCTION	SND_STOP		0	1
FUNCTION	SND_QUERY		0	1
FUNCTION	SND_ACQ_START		1	1
FUNCTION	SND_ACQ_DATA		3	3
FUNCTION	SND_ACQ_STOP		0	0
FUNCTION	GZIP			2	2
FUNCTION	GUNZIP			2	2	KEYWORDS
FUNCTION	PACKVAR			2	2
FUNCTION	UNPACKVAR		2	2
FUNCTION	MESH_THIN		5	5	KEYWORDS
PROCEDURE	SLAB			2	2	KEYWORDS
FUNCTION	TWAIN_PRESENT		0	0
FUNCTION	TWAIN_SELECT		0	0
FUNCTION	TWAIN_ACQUIRE		2	5	KEYWORDS
FUNCTION	EXTPROC_DEFINE		4	4	KEYWORDS
FUNCTION	EXTPROC_QUERY		1	1
FUNCTION	EXTPROC_DEREF		1	1	KEYWORDS
FUNCTION	SHARR_MAP			3	3	KEYWORDS
FUNCTION	SHARR_TEST			1	1
FUNCTION	SHARR_FLUSH			1	1
FUNCTION	SHARR_ALLOC_MEM		3	3
FUNCTION	SHARR_FREE_MEM		1	1
