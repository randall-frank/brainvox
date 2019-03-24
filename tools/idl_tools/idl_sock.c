#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "export.h"
#include "idl_tools.h"

#define MAX_SOCKETS 256
#define SOCK_UNUSED 0
#define SOCK_LISTEN 1
#define SOCK_IO 2

#ifndef WIN32
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/ioctl.h>
#define SOCKET	int
#define IOCTL	ioctl
#define CLOSE	close
#else
#include <winsock2.h>
#define IOCTL	ioctlsocket
#define CLOSE	closesocket
#endif

typedef struct _sock {
	IDL_LONG	iState;
	SOCKET		socket;
} sock;

/* local prototypes */
static int recv_packet(SOCKET s,void *buffer,int len);
static void rebuffer_socket(SOCKET s,int len);
static void nodelay_socket(SOCKET s,int flag);

/* global list of sockets */
sock sock_list[MAX_SOCKETS];

/* function protos */
extern IDL_VPTR IDL_CDECL sock_createport(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sock_close(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sock_connect(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sock_accept(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sock_send(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sock_recv(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sock_query(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sock_sendvar(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sock_recvvar(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sock_select(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sock_name2host(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL sock_host2name(int argc, IDL_VPTR argv[], char *argk);

/* define the SOCK functions */
static IDL_SYSFUN_DEF sock_functions[] = {

    { sock_createport,	"SOCK_CREATEPORT",	1, 1, 0},
    { sock_close,	"SOCK_CLOSE",		1, 1, 0},
    { sock_connect,	"SOCK_CONNECT",		2, 2, IDL_SYSFUN_DEF_F_KEYWORDS},
    { sock_accept,	"SOCK_ACCEPT",		1, 1, IDL_SYSFUN_DEF_F_KEYWORDS},
    { sock_send,	"SOCK_SEND",		2, 2, 0},
    { sock_recv,	"SOCK_RECV",		2, 2, IDL_SYSFUN_DEF_F_KEYWORDS},
    { sock_query,	"SOCK_QUERY",		1, 1, IDL_SYSFUN_DEF_F_KEYWORDS},
    { sock_sendvar,	"SOCK_SENDVAR",		2, 2, 0},
    { sock_recvvar,	"SOCK_RECVVAR",		2, 2, 0},
    { sock_select,	"SOCK_SELECT",		2, 2, 0},
    { sock_name2host,	"SOCK_NAME2HOST",	0, 1, 0},
    { sock_host2name,	"SOCK_HOST2NAME",	0, 1, 0},

};

#ifdef WIN32
static	int	iInitW2 = 0;
#endif

/* startup call when DLM is loaded */
int sock_startup(void)
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	
	wVersionRequested = MAKEWORD( 2, 0 ); 
	err = WSAStartup( wVersionRequested, &wsaData );
	if (!err) {
		iInitW2 = 1;
//	} else {
//		return(IDL_FALSE);
	}
#endif
	if (!IDL_AddSystemRoutine(sock_functions, TRUE, 
		ARRLEN(sock_functions))) {
		return IDL_FALSE;
	}
	IDL_ExitRegister(sock_exit_handler);

	return(IDL_TRUE);
}

/* clean up all the open sockets on IDL shutdown */
void sock_exit_handler(void)
{
	IDL_LONG	i;

	for(i=0;i<MAX_SOCKETS;i++) {
		if (sock_list[i].iState != SOCK_UNUSED) {
			shutdown(sock_list[i].socket,2);
			CLOSE(sock_list[i].socket);
		}
	}

#ifdef WIN32
	if (iInitW2) WSACleanup();
#endif

}

/*
 * 	General notes: 
 *		All error codes return -1 on failure.
 *		The socket identifier is NOT the actual underlying OS
 *		socket number, it is an IDL abstraction.
 *		ports are 16bit integers.
 *		hosts are unsigned 32bit integers.
 */

/*
 *	socket = SOCK_CREATEPORT(portnum)
 *
 *	Creates a socket listening on the specified port for a new
 *	connection.  SOCK_SELECT returns true for this socket if
 *	there is an attempt to connect to it (which should be
 *	serviced by SOCK_ACCEPT).
 */
IDL_VPTR IDL_CDECL sock_createport(int argc, IDL_VPTR argv[], char *argk)
{
	SOCKET	s;
	struct  sockaddr_in sin;
	short	port;
	int	err;
	IDL_LONG	i;

	for(i=0;i<MAX_SOCKETS;i++) {
		if (sock_list[i].iState == SOCK_UNUSED) break;
	}
	if (i == MAX_SOCKETS) return(IDL_GettmpLong(-2));

	port = (short)IDL_LongScalar(argv[0]);

	s = socket(AF_INET,SOCK_STREAM,0);
	if (s == -1) return(IDL_GettmpLong(-1));

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);
	err=bind(s,(struct sockaddr *)&sin,sizeof(sin));
	if (err == -1) {
		CLOSE(s);
		return(IDL_GettmpLong(-1));
	}
	err=listen(s,5);
	if (err == -1) {
		CLOSE(s);
		return(IDL_GettmpLong(-1));
	}

	sock_list[i].iState = SOCK_LISTEN;
	sock_list[i].socket = s;

	return(IDL_GettmpLong(i));
}

/*
 *	err = SOCK_CLOSE(socket)
 *
 *	Close and free the socket in question.
 */
IDL_VPTR IDL_CDECL sock_close(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_LONG	i;

	i = IDL_LongScalar(argv[0]);
	if ((i<0) || (i>=MAX_SOCKETS)) return(IDL_GettmpLong(-1));
	if (sock_list[i].iState == SOCK_UNUSED) return(IDL_GettmpLong(-1));

	shutdown(sock_list[i].socket,2);
	CLOSE(sock_list[i].socket);

	sock_list[i].iState = SOCK_UNUSED;
	
	return(IDL_GettmpLong(0));
}

/*
 *	socket = SOCK_CONNECT(host,port[,BUFFER=size][,/NODELAY])
 *
 *	Connect to a socket listener on some specified host and port.
 *	The returned socket can be used for I/O after the server
 *	"accepts" the connection.
 */
IDL_VPTR IDL_CDECL sock_connect(int argc, IDL_VPTR inargv[], char *argk)
{	
	SOCKET		s;
	struct sockaddr_in sin;
	int			addr_len,err;
	short		port;
	int			host;
	IDL_LONG	i;
	IDL_VPTR	argv[2];

static	IDL_LONG	iBuffer,iNoDelay;
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
	{"BUFFER",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iBuffer)},
	{"NODELAY",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iNoDelay)},
        {NULL}
    };

	IDL_KWCleanup(IDL_KW_MARK);
	IDL_KWGetParams(argc,inargv,argk,kw_pars,argv,1);
	host = IDL_ULongScalar(argv[0]);
	port = (short)IDL_LongScalar(argv[1]);
	IDL_KWCleanup(IDL_KW_CLEAN);

	for(i=0;i<MAX_SOCKETS;i++) {
		if (sock_list[i].iState == SOCK_UNUSED) break;
	}
	if (i == MAX_SOCKETS) return(IDL_GettmpLong(-2));

	s = socket(AF_INET,SOCK_STREAM,0);
	if (s == -1) return(IDL_GettmpLong(-1));
	if (iBuffer) rebuffer_socket(s,iBuffer);
	if (iNoDelay) nodelay_socket(s,1);

	sin.sin_addr.s_addr = host;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	addr_len = sizeof(struct sockaddr_in);
	err=connect(s,(struct sockaddr *)&sin,addr_len);
	if (err == -1) {
		CLOSE(s);
		return(IDL_GettmpLong(-1));
	}

	sock_list[i].iState = SOCK_IO;
	sock_list[i].socket = s;

	return(IDL_GettmpLong(i));
}

/*
 *	socket = SOCK_ACCEPT(socket[,BUFFER=size][,/NODELAY])
 *
 *	Accepts a requested connection and returns a socket on which
 *	I/O can be performed.
 */
IDL_VPTR IDL_CDECL sock_accept(int argc, IDL_VPTR inargv[], char *argk)
{
	IDL_LONG	i,j;
	struct sockaddr_in     peer_addr;
	int			addr_len;
	SOCKET		s;
	IDL_VPTR	argv[1];

static	IDL_LONG	iBuffer,iNoDelay;
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
	{"BUFFER",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iBuffer)},
	{"NODELAY",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iNoDelay)},
        {NULL}
    };

	IDL_KWCleanup(IDL_KW_MARK);
	IDL_KWGetParams(argc,inargv,argk,kw_pars,argv,1);
	j = IDL_LongScalar(argv[0]);
	IDL_KWCleanup(IDL_KW_CLEAN);

	if ((j<0) || (j>=MAX_SOCKETS)) return(IDL_GettmpLong(-1));
	if (sock_list[j].iState != SOCK_LISTEN) return(IDL_GettmpLong(-1));

	for(i=0;i<MAX_SOCKETS;i++) {
		if (sock_list[i].iState == SOCK_UNUSED) break;
	}
	if (i == MAX_SOCKETS) return(IDL_GettmpLong(-2));

	addr_len = sizeof(struct sockaddr_in);
	s = accept(sock_list[j].socket,(struct sockaddr *)&peer_addr,&addr_len);
	if (s == -1) return(IDL_GettmpLong(-1));

	if (iBuffer) rebuffer_socket(s,iBuffer);
	if (iNoDelay) nodelay_socket(s,1);
	sock_list[i].iState = SOCK_IO;
	sock_list[i].socket = s;

	return(IDL_GettmpLong(i));
}

/*
 *	nbytes = SOCK_SEND(socket, variable)
 *
 *	Sends the raw byte data from the IDL variable on the
 *	socket.  Returns the number of bytes sent or -1 for error.
 *	Note: no byteswapping is performed.
 */
IDL_VPTR IDL_CDECL sock_send(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_LONG	i,iNum,iRet;
	IDL_VPTR	vpTmp;
	char		*pbuffer;

	i = IDL_LongScalar(argv[0]);
	if ((i<0) || (i>=MAX_SOCKETS)) return(IDL_GettmpLong(-1));
	if (sock_list[i].iState != SOCK_IO) return(IDL_GettmpLong(-1));
	IDL_ENSURE_SIMPLE(argv[1]);
	vpTmp = argv[1];
	
	if (vpTmp->type == IDL_TYP_STRING) {
		vpTmp  = IDL_CvtByte(1,&vpTmp);
	}

	IDL_VarGetData(vpTmp, &iNum, &pbuffer, 1);
	iNum = iNum * IDL_TypeSizeFunc(vpTmp->type);

	iRet = send(sock_list[i].socket,pbuffer,iNum,0);

	if (vpTmp != argv[1]) IDL_Deltmp(vpTmp);

	return(IDL_GettmpLong(iRet));
}

/*
 *	nbytes = SOCK_RECV(socket, variable [, MAXIMUM_BYTES=b])
 *
 *	Reads the raw data available on the socket and returns a BYTE
 *	array in variable.  The maximum number of bytes to read
 *	can be specified by the MAXIMUM_BYTES keyword.  The default
 *	is to read all the data available on the socket.
 *	Note: no byteswapping is performed.
 */
IDL_VPTR IDL_CDECL sock_recv(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_LONG	i,iRet,err;
	int 		len;
	IDL_VPTR        vpPlainArgs[2],vpTmp;
	char		*pbuffer;

static	IDL_LONG	iMax;
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
	{"MAXIMUM_BYTES",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iMax)},
        {NULL}
        };

	IDL_KWCleanup(IDL_KW_MARK);
	IDL_KWGetParams(argc,argv,argk,kw_pars,vpPlainArgs,1);

	i = IDL_LongScalar(vpPlainArgs[0]);
	if ((i<0) || (i>=MAX_SOCKETS)) return(IDL_GettmpLong(-1));
	if (sock_list[i].iState != SOCK_IO) return(IDL_GettmpLong(-1));
	IDL_EXCLUDE_EXPR(vpPlainArgs[1]);

	err = IOCTL(sock_list[i].socket,FIONREAD,&len);
	if (err != 0) {
		iRet = -1;
		goto err;
	}
	if (iMax) len = IDL_MIN(iMax,len);

	pbuffer = (char *)IDL_MakeTempVector(IDL_TYP_BYTE,
		len, IDL_ARR_INI_NOP, &vpTmp);
	IDL_VarCopy(vpTmp,vpPlainArgs[1]);

	iRet = recv(sock_list[i].socket,pbuffer,len,0);

err:
	IDL_KWCleanup(IDL_KW_CLEAN);

	return(IDL_GettmpLong(iRet));
}

/*
 *	err = SOCK_QUERY(socket [,AVAILABLE_BYTES=a] [,REMOTE_HOST=r]
 *		[,IS_LISTENER=l])
 *
 *	Returns various information about the socket in question.
 *		AVAILABLE_BYTES : number of bytes available for reading.
 *		REMOTE_HOST: host number of the remote host the socket
 *			is connected to.
 *		IS_LISTENER: true if the socket was created using
 *			SOCK_CREATEPORT()
 */
IDL_VPTR IDL_CDECL sock_query(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_LONG	i;
	IDL_VPTR        vpPlainArgs[1],vpTmp;
	struct  sockaddr_in     peer_addr;
	int     	addr_len,err;
	IDL_LONG	iRet = 0;

static	IDL_VPTR	vpHost,vpAvail,vpListen;
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
	{"AVAILABLE_BYTES",IDL_TYP_UNDEF,1,IDL_KW_OUT|IDL_KW_ZERO,0,
		IDL_CHARA(vpAvail)},
	{"IS_LISTENER",IDL_TYP_UNDEF,1,IDL_KW_OUT|IDL_KW_ZERO,0,
		IDL_CHARA(vpListen)},
	{"REMOTE_HOST",IDL_TYP_UNDEF,1,IDL_KW_OUT|IDL_KW_ZERO,0,
		IDL_CHARA(vpHost)},
        {NULL}
        };

	IDL_KWCleanup(IDL_KW_MARK);
	IDL_KWGetParams(argc,argv,argk,kw_pars,vpPlainArgs,1);

	i = IDL_LongScalar(vpPlainArgs[0]);
	if ((i<0) || (i>=MAX_SOCKETS)) return(IDL_GettmpLong(-1));

	if (vpHost) {
		addr_len = sizeof(struct  sockaddr_in);
		err = getpeername(sock_list[i].socket,
			(struct sockaddr *)&peer_addr,&addr_len);
		if (err != 0) {
			iRet = -1;
		} else {
			vpTmp = IDL_GettmpULong(peer_addr.sin_addr.s_addr);
			IDL_VarCopy(vpTmp,vpHost);
		}
	}
	if (vpAvail) {
		int 	len;
		err = IOCTL(sock_list[i].socket,FIONREAD,&len);
		if (err != 0) {
			iRet = -1;
		} else {
			vpTmp = IDL_GettmpULong(len);
			IDL_VarCopy(vpTmp,vpAvail);
		}
	}
	if (vpListen) {
		vpTmp = IDL_GettmpLong(sock_list[i].iState == SOCK_LISTEN);
		IDL_VarCopy(vpTmp,vpListen);
	}

	IDL_KWCleanup(IDL_KW_CLEAN);

	return(IDL_GettmpLong(iRet));
}

/* function to read a (potentially fragmented) block from a socket */
static	int	recv_packet(SOCKET s,void *buffer,int len)
{
	int	n;
	int	num = 0;
	char	*pbuf = (char *)buffer;

	while(num < len) {
		n = recv(s,pbuf,len-num,0);
		if (n == -1) return(n);
		pbuf += n;
		num += n;
#ifdef	INTERRUPTABLE_READ
		if (IDL_BailOut(IDL_FALSE)) return(-1);
#endif
	}

	return(len);
}



/*
 *	err = SOCK_SENDVAR(socket, variable)
 *
 *	Sends a complete IDL variable to a socket for reading by
 *	SOCK_RECVVAR.  The variable must be one of the basic types,
 *	but strings and arrays are sent with array dimensions and
 *	lengths intact.  
 *	Note: This is the easiest way to send a complete variable from
 *		one IDL to another.  The receiver will automatically
 *		byteswap the data if necessary.  One should be careful
 *		not to mix calls to SOCK_SEND/RECV and SOCK_SENDVAR/RECVVAR
 *		as the latter send formatted information.  You can use
 *		the two calls on the same socket as long as they are
 *		paired.
 */
IDL_VPTR IDL_CDECL sock_sendvar(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_LONG	i;
	i_var		var;

        IDL_LONG        iRet;
        IDL_VPTR        vpTmp;
        char            *pbuffer;

	i = IDL_LongScalar(argv[0]);
	if ((i<0) || (i>=MAX_SOCKETS)) return(IDL_GettmpLong(-1));
	if (sock_list[i].iState != SOCK_IO) return(IDL_GettmpLong(-1));
	IDL_ENSURE_SIMPLE(argv[1]);
	vpTmp = argv[1];

	var.token = TOKEN;
	var.type = vpTmp->type;
	if ((var.type == IDL_TYP_STRUCT) ||
	    (var.type == IDL_TYP_PTR) ||
	    (var.type == IDL_TYP_OBJREF) ||
	    (var.type == IDL_TYP_UNDEF)) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADTYPE, 
			IDL_MSG_LONGJMP,IDL_TypeNameFunc(var.type));
	}

	if (vpTmp->type == IDL_TYP_STRING) {
		if (vpTmp->flags & IDL_V_ARR) return(IDL_GettmpLong(-1));
		pbuffer = IDL_STRING_STR(&(vpTmp->value.str));
		var.ndims = 0;
		var.len = vpTmp->value.str.slen+1;
		var.nelts = var.len;
	} else if (vpTmp->flags & IDL_V_ARR) {
		pbuffer = vpTmp->value.arr->data;
		var.ndims = vpTmp->value.arr->n_dim;
		var.len = vpTmp->value.arr->arr_len;
		var.nelts = vpTmp->value.arr->n_elts;
		memcpy(var.dims,vpTmp->value.arr->dim,
			IDL_MAX_ARRAY_DIM*sizeof(IDL_LONG));
	} else {
		pbuffer = &(vpTmp->value.c);
		var.ndims = 0;
		var.len = IDL_TypeSizeFunc(var.type);
		var.nelts = 1;
	}

/* send native, recvvar swaps if needed */
	iRet = send(sock_list[i].socket,(char *)&var,sizeof(i_var),0);
	if (iRet == -1) return(IDL_GettmpLong(iRet));

	iRet = send(sock_list[i].socket,pbuffer,var.len,0);

	return(IDL_GettmpLong(1));
}

/*
 *	err = SOCK_RECVVAR(socket, variable)
 *
 *	Reads an IDL variable from the socket in the form written by
 *	SOCK_SENDVAR.  The complete variable is reconstructed.
 *	See: SOCK_SENDVAR for more details.
 */
IDL_VPTR IDL_CDECL sock_recvvar(int argc, IDL_VPTR argv[], char *argk)
{
	IDL_LONG	i,iRet;
	IDL_LONG	swab = 0;
	i_var 		var;
	IDL_VPTR	vpTmp;
	char		*pbuffer;

	i = IDL_LongScalar(argv[0]);
	if ((i<0) || (i>=MAX_SOCKETS)) return(IDL_GettmpLong(-1));
	if (sock_list[i].iState != SOCK_IO) return(IDL_GettmpLong(-1));
	IDL_EXCLUDE_EXPR(argv[1]);

/* read the header */
	iRet = recv_packet(sock_list[i].socket,&var,sizeof(i_var));
	if (iRet == -1) return(IDL_GettmpLong(-1));
	if (var.token == SWAPTOKEN) {
		byteswap(&var,sizeof(i_var),sizeof(IDL_LONG));
		swab = 1;
	}
	if (var.token != TOKEN) return(IDL_GettmpLong(-1));

/* allocate the variable */
	if (var.type == IDL_TYP_STRING) {
		vpTmp = IDL_StrToSTRING("");
		IDL_StrEnsureLength(&(vpTmp->value.str),var.len);
		vpTmp->value.str.slen = var.len - 1;
		pbuffer = vpTmp->value.str.s;
		memset(pbuffer,0x20,var.len-1);
		pbuffer[var.len] = '\0';
		IDL_VarCopy(vpTmp,argv[1]);
	} else if (var.ndims != 0) {
		pbuffer = IDL_MakeTempArray(var.type, var.ndims,
			var.dims, IDL_BARR_INI_NOP, &vpTmp);
		IDL_VarCopy(vpTmp,argv[1]);
	} else {
		vpTmp = IDL_GettmpLong(0);
		IDL_VarCopy(vpTmp,argv[1]);
		IDL_StoreScalarZero(argv[1],var.type);
		pbuffer = &(argv[1]->value.c);
	}

/* read the data */
	iRet = recv_packet(sock_list[i].socket,pbuffer,var.len);
	if (iRet == -1) return(IDL_GettmpLong(-1));
	if (swab) {
		int	swapsize = var.len/var.nelts;
		if ((var.type == IDL_TYP_COMPLEX) ||
		    (var.type == IDL_TYP_DCOMPLEX)) {
			swapsize /= 2;
		}
		byteswap(pbuffer,var.len,swapsize);
	}

	return(IDL_GettmpLong(1));
}

/*
 *	out = SOCK_SELECT(sockets[], timeout)
 *
 *	Checks to see if there is data waiting to be read or a 
 *	connection has been requested for a list of sockets.
 *	The return value is -1 on error, scalar 0 if no sockets
 *	are ready or returns a list of the sockets which are ready.
 *	The routine waits the number of seconds specified by the
 *	timeout argument for sockets to become ready.  A timeout
 *	value of 0 results in a poll of the sockets.
 */
IDL_VPTR IDL_CDECL sock_select(int argc, IDL_VPTR argv[], char *argk)
{
	struct timeval	timeval;
	fd_set	rfds;

	IDL_LONG	i,j;
	IDL_LONG	n,num;

	float		fWait;
	IDL_LONG        *piSocks,iNum;
	IDL_VPTR	vpSocks;

	vpSocks = IDL_CvtLng(1,&(argv[0]));
	IDL_VarGetData(vpSocks, &iNum, (char **)&piSocks, 1);
	fWait = (float)IDL_DoubleScalar(argv[1]);

	num = -1;
	FD_ZERO(&rfds);
	
	for(j=0;j<iNum;j++) {
		i = piSocks[j];
		if ((i<0) || (i>=MAX_SOCKETS)) {
			if (vpSocks != argv[0]) IDL_Deltmp(vpSocks);
			return(IDL_GettmpLong(-1));
		}
		if (sock_list[i].iState != SOCK_UNUSED) {
			FD_SET(sock_list[i].socket,&rfds);
			if (sock_list[i].socket > (SOCKET)num) num=sock_list[i].socket;
		}
	}
	while(fWait >= 0.0) {
		if (fWait >= 2.0) {
			timeval.tv_sec = 2;
			timeval.tv_usec = 0;
		} else {
			timeval.tv_sec = (long)fWait;
			fWait = fWait - timeval.tv_sec;
			timeval.tv_usec = (long)(fWait * 1000000);
		}
		n = select(num+1,&rfds,NULL,NULL,&timeval);
		if (n == -1) fWait = -1.0;
		if (n > 0) fWait = -1.0;
		fWait -= 2.0;
		if (IDL_BailOut(IDL_FALSE)) {
			n = -1;
			fWait = -1.0;
		}
	}

	if (n > 0) {
		IDL_LONG	*pOut;
		IDL_VPTR	vpTmp;

		pOut = (IDL_LONG *)IDL_MakeTempVector(IDL_TYP_LONG,
			n, IDL_ARR_INI_NOP, &vpTmp);
		for(j=0;j<iNum;j++) {
			i = piSocks[j];
			if (sock_list[i].iState != SOCK_UNUSED) {
				if (FD_ISSET(sock_list[i].socket,&rfds)){
					*pOut++ = i;
				}
			}
		}
		if (vpSocks != argv[0]) IDL_Deltmp(vpSocks);
		return(vpTmp);
	}

	if (vpSocks != argv[0]) IDL_Deltmp(vpSocks);

	return(IDL_GettmpLong(n));
}

/*
 *	host = SOCK_NAME2HOST(['name'])
 *
 *	Converts the ASCII host name into an unsigned long host value.
 *  If ['name'] is not specified, the local host name is used.
 */
IDL_VPTR IDL_CDECL sock_name2host(int argc, IDL_VPTR argv[], char *argk)
{
	struct  hostent *hp;
	char	*pName,host_name[256];

	if (argc == 0) {
		if (gethostname(host_name,256) == -1) {
			host_name[0] = '\0';
		}
		pName = host_name;
	} else {
		IDL_ENSURE_STRING(argv[0]);
		IDL_ENSURE_SCALAR(argv[0]);
		pName = IDL_STRING_STR(&(argv[0]->value.str));
	}

	hp = gethostbyname(pName);
	if (!hp) return(IDL_GettmpLong(0));

	return(IDL_GettmpULong(((struct in_addr *)(hp->h_addr))->s_addr));
}

/*
 *	name = SOCK_HOST2NAME([host])
 *
 *	Converts the unsigned long host value into an ASCII hostname.
 *  If [host] is not specified, the local hostname is returned.
 */
IDL_VPTR IDL_CDECL sock_host2name(int argc, IDL_VPTR argv[], char *argk)
{
	struct	in_addr	addr;
	struct	hostent *hp;
	char	host_name[256];

	if (argc == 0) {
		if (gethostname(host_name,256) == -1) {
			host_name[0] = '\0';
		}
		return(IDL_StrToSTRING(host_name));
	} else {
		addr.s_addr = IDL_ULongScalar(argv[0]);
		hp = gethostbyaddr((char *)&addr, sizeof(struct in_addr), AF_INET);
		if (!hp) return(IDL_StrToSTRING(""));
	}
	return(IDL_StrToSTRING(hp->h_name));
}

/* function to adjust socket buffering for things like
 * gigE performance (sometimes referred to as "flogging") */
void rebuffer_socket(SOCKET s,int len)
{
	if (len < 10000) return; /* why would you do this??? */
	setsockopt(s,SOL_SOCKET,SO_RCVBUF,(void *)&len,sizeof(int));
	setsockopt(s,SOL_SOCKET,SO_SNDBUF,(void *)&len,sizeof(int));
	return;
}
void nodelay_socket(SOCKET s,int flag)
{
	setsockopt(s,IPPROTO_TCP,TCP_NODELAY,(void *)&flag,sizeof(int));
}

/* function to perform general 2, 4 and 8 byte byteswapping */
void  byteswap(void *buffer,int len,int swapsize)
{
	int	num;
	char	*p = (char *)buffer;
	char	t;

	switch(swapsize) {
		case 2:
			num = len/swapsize;
			while(num--) {
				t = p[0]; p[0] = p[1]; p[1] = t;
				p += swapsize;
			}
			break;
		case 4:
			num = len/swapsize;
			while(num--) {
				t = p[0]; p[0] = p[3]; p[3] = t;
				t = p[1]; p[1] = p[2]; p[2] = t;
				p += swapsize;
			}
			break;
		case 8:
			num = len/swapsize;
			while(num--) {
				t = p[0]; p[0] = p[7]; p[7] = t;
				t = p[1]; p[1] = p[6]; p[6] = t;
				t = p[2]; p[2] = p[5]; p[5] = t;
				t = p[3]; p[3] = p[4]; p[4] = t;
				p += swapsize;
			}
			break;
		default:
			break;
	}
	return;
}

