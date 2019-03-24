/*
 * Copyright (c), 1990-2005 Randall Frank and Thomas Grabowski
 *
 * $Id $
 * $HeadURL $
 *
 *                University of Iowa, Department of Neurology,
 *                Human Neuroanatomy and Neuroimaging Lab
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by Randall Frank, Thomas
 *      Grabowski and other unpaid contributors.
 * 5. The names of Randall Frank or Thomas Grabowski may not be used to
 *    endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY RANDALL FRANK AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL RANDALL FRANK OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifndef WIN32
#include <unistd.h>
#include <signal.h>
#define closesocket close
#else
#include <windows.h>
#endif

#include <errno.h>

#include "zlib.h"
#include "cvio_priv.h"

/* local prototypes */
static struct sbuffer *sbuffer_create(int atomsize);
static void sbuffer_destroy(struct sbuffer *buf);
static void *sbuffer_new_ptr(struct sbuffer *buf, int *samples);
static void sbuffer_commit(struct sbuffer *buf, int samples);
static int sbuffer_get_samples(struct sbuffer *buf, void *target, int count);
static int sbuffer_count(struct sbuffer *buf);
static int send_samples(cvio_stream *s,int all);
static int sock_read(SOCKET s, void *b, size_t len, int count);
static int sock_write(SOCKET s, void *b, size_t len);
static void cvio_swap_header(header *h);
static void sock_remove_samples(cvio_stream *s,CVIO_DT_TIME *times, 
                                void *samps,int n);

/* utility functions */
static int sock_read(SOCKET s, void *b, size_t len, int count)
{
	int outcount = 0, tlen = count * len, trec = 0;
	unsigned char *p = (unsigned char *)b;

	if (count == 0)
		return 0;

	while(tlen) {
		int err = recv(s,p,tlen,0);

		if (err <= 0) {
			break;
		} else {
			trec += err;
			outcount = trec / len;
			tlen -= err;
			p += err;
			if ((tlen % len == 0) && (outcount > 0))
				break;
		}
	}
	if (outcount) {
		return outcount;
	} else return -1;
}

static int sock_write(SOCKET s, void *b, size_t len)
{
	unsigned char *p = (unsigned char *)b;
	while(len) {
		int err = send(s,p,len,0);
		if (err < 0) {
#ifndef WIN32
			if ((errno != EAGAIN) && (errno != EINTR)) return(-1);
#endif
		} else if (err == 0) {
			return(-1);
		} else {
			len -= err;
			p += err;
		}
	}
	return(0);
}

int cvio_sock_samp_size(cvio_stream *s, int time)
{
	int i,sz;

	sz = (s->hdr.type & CVIO_TYP_SIZE_MASK)/8;
	for(i=0;i<s->hdr.ndim;i++) sz *= s->hdr.dims[i];

	if (time) sz += sizeof(CVIO_DT_TIME);

	return(sz);
}

static void cvio_swap_header(header *h)
{
   if (is_bigendian() && h->bigendian) return;
   if ((!is_bigendian()) && (!h->bigendian)) return;
   byteswap(&(h->spacing),sizeof(h->spacing),sizeof(CVIO_DT_ULONG));
   byteswap(&(h->flags),sizeof(h->flags),sizeof(CVIO_DT_USHORT));
   byteswap(&(h->type),sizeof(h->type),sizeof(CVIO_DT_USHORT));
   byteswap(&(h->ndim),sizeof(h->ndim),sizeof(CVIO_DT_USHORT));
   byteswap(&(h->dims),sizeof(h->dims),sizeof(CVIO_DT_USHORT));
   byteswap(&(h->compression),sizeof(h->compression),sizeof(CVIO_DT_USHORT));
   byteswap(&(h->nsamp),sizeof(h->nsamp),sizeof(CVIO_DT_ULONG));
   byteswap(&(h->first_time),sizeof(h->first_time),sizeof(CVIO_DT_TIME));
   byteswap(&(h->last_time),sizeof(h->last_time),sizeof(CVIO_DT_TIME));
   byteswap(&(h->bigendian),sizeof(h->bigendian),sizeof(CVIO_DT_USHORT));
   byteswap(&(h->timestampbase),sizeof(h->timestampbase),sizeof(CVIO_DT_ULONG));
   byteswap(&(h->max_samples),sizeof(h->max_samples),sizeof(int32_t));
   return;
}

/* parse a potential socket filename into bits */
int parse_sock(const char *in, char *host, char *name, int *port,
    int *wait, float *waittime)
{
   char *p;
   const char *s;
   host[0] = '\0';
   name[0] = '\0';
   *port = CVIO_PORT_NONE;
   *wait = 0;
   *waittime = 0.0;

   if (!strstr(in,":sock:")) return(-1);
   s = in + 6;
   p = host;
   while(*s && (*s != ':')) *p++ = *s++; *p = '\0';
   s++;
   p = name;
   while(*s && (*s != ':')) *p++ = *s++; *p = '\0';

   p = strstr(s,":port=");
   if (p) *port = atoi(p+6);

   p = strstr(s,":wait=");
   if (p) *wait = atoi(p+6);

   p = strstr(s,":timeout=");
   if (p) *waittime = atof(p+9);

   if (strlen(host) == 0) strcpy(host,"localhost");
   if (strlen(name) == 0) strcpy(host,"unnamed");

   return(0);
}

void cvio_sock_shutdown(cvio_stream *s)
{
        void *vret;
#ifdef DEBUG
printf("cvio_sock_shutdown\n");
#endif
	/* flag the work proc to shutdown */
        pthread_mutex_lock(&s->s_mutex);
        s->port = -1; /* signal to thread to shut down */
        pthread_mutex_unlock(&s->s_mutex);

	/* wait for it to shutdown */
        pthread_join(s->worker,&vret);

	/* cleanup */
        pthread_mutex_destroy(&s->s_mutex);
        sbuffer_destroy(s->sbuf);

	return;
}

int cvio_sock_connect(cvio_stream *s, char *host, char *name, int port)
{
	/* Scan all the port (or the single one noted) on host for
         * a socket by "name".  Connect to the socket */
        struct hostent *hp;
	struct sockaddr_in sin;
	int len,err,pt,max_p;
	char *p;

	p = host;
	if (strlen(p) == 0) p = "localhost";

	hp = gethostbyname(p);
	if (!hp) return(-1);

	max_p = 100;
	if (port != CVIO_PORT_NONE) max_p = 1;

	/* scan the ports on "host" for one with "name" */
	for(pt=0;pt<max_p;pt++) {

	    /* create the socket */
	    s->s_listen = socket(AF_INET,SOCK_STREAM,0);
	    if (s->s_listen == -1) return(-1);

	    /* pick the target */
	    sin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	    sin.sin_family = AF_INET;
	    if (port == CVIO_PORT_NONE) {
	        sin.sin_port = htons(CVIO_PORT_BASE+pt);
	    } else {
	        sin.sin_port = htons(CVIO_PORT_BASE+port);
	    }
	    len = sizeof(struct sockaddr_in);
            s->port = ntohs(sin.sin_port);

#ifdef DEBUG
printf("Scanning: %d %d\n",pt,CVIO_PORT_BASE+pt);
#endif

	    /* try to hookup */
	    err = connect(s->s_listen,(struct sockaddr *)&sin,len);
	    if (err == 0) {
		/* read the header */
	        if (sock_read(s->s_listen,&(s->hdr),sizeof(s->hdr), 1) == 1) {
		    /* swap it */
	            cvio_swap_header(&(s->hdr));
	            /* found one!! */
	            if (strcmp(s->hdr.filename,name) == 0) {
			/* ack */
			err = 0;
			if (sock_write(s->s_listen,&err,sizeof(err)) == 0) {
                            /* always a writer */
			    s->hdr.flags |= HDR_FLAGS_HAS_WRITER;
                            /* other settings */
                            s->hdr.nsamp = 0;
                            s->hdr.first_time = 0;
                            s->hdr.last_time = 0;
			    s->pos.current_sample = 0;
			    s->pos.current_time = 0;
	                    /* allocate an initial buffer */
	                    s->sbuf = sbuffer_create(cvio_sock_samp_size(s,1));
	                    if (!s->sbuf) {
		                    closesocket(s->s_listen);
		                    return(-1);
	                    }
	                    /* startup the reading thread */
                    	    pthread_mutex_init(&s->s_mutex,NULL);
                            if (pthread_create(&s->worker,NULL,
                                               cvio_sock_readproc,(void*)s)) {
                    	        sbuffer_destroy(s->sbuf);
                                pthread_mutex_destroy(&s->s_mutex);
                    	        closesocket(s->s_listen);
                                return(-1);
                            }
			    return(0);
			} else {
                            return(-1);
                        }
		    } else {
			/* nack */
			err = 1;
			if (sock_write(s->s_listen,&err,sizeof(err))) {
                            return(-1);
                        }
                    }
	        }
	    }
            /* no match on this port */
	    shutdown(s->s_listen,2);
	    closesocket(s->s_listen);
	}
	
	return(-1);
}

void *cvio_sock_readproc(void *inarg)
{
	fd_set r_set;
	struct timeval tout;
	int err,ssize,ssizeT;
        cvio_stream *s = (cvio_stream *)inarg;

#ifdef DEBUG
printf("starting readproc\n");
#endif

        ssize = cvio_sock_samp_size(s,0);
        ssizeT = cvio_sock_samp_size(s,1);
        while(1) {
                /* check for incoming data (select) */
		FD_ZERO(&r_set);
		FD_SET(s->s_listen,&r_set);
		tout.tv_sec = 0;
		tout.tv_usec = (1.0/80000.) * 1.0e6;
		err = select(s->s_listen+1,&r_set,NULL,NULL,&tout);
		if (err < 0) {
		    /* real error (close down the socket later) */
                    s->port = -1; /* signal to thread to shut down */
		} else if (err == 0) {
		    /* timeout */
		} 
                pthread_mutex_lock(&s->s_mutex);
		/* is new data available for reading? */
		if ((err > 0) && FD_ISSET(s->s_listen,&r_set)) {
                    char *p;
                    int scount = 100;
                    /* read one in */
                    p = sbuffer_new_ptr(s->sbuf, &scount);
	            if (!p) {
                       /* just remaining buffers */
                       s->hdr.flags &= (~HDR_FLAGS_HAS_WRITER);
                       shutdown(s->s_listen,0);
                       closesocket(s->s_listen);
                       pthread_mutex_unlock(&s->s_mutex); 
                       return(NULL);
                    }
                    /* Note: byteswapping is done at higher level */
                    scount = sock_read(s->s_listen, p, ssizeT, scount);
		    if (scount == -1) {
                       s->port = -1;
                    } else {
                       s->hdr.nsamp += scount;
                       sbuffer_commit(s->sbuf, scount);
#ifdef DEBUG
printf("new sample received (%d) %d\n", sbuffer_count(s->sbuf),s->hdr.nsamp);
#endif
                    }
                }
                /* should we be shutting down */
                if (s->port == -1) {
                   /* we no longer have a writer, just remaining buffers */
		   s->hdr.flags &= (~HDR_FLAGS_HAS_WRITER);
                   shutdown(s->s_listen,0);
                   closesocket(s->s_listen);
                   pthread_mutex_unlock(&s->s_mutex);
                   return(NULL);
                }
                pthread_mutex_unlock(&s->s_mutex);
        }
        return(NULL);
}

static int n_work_list = 0;
static header work_list[MAX_STREAMS];

int cvio_sock_find(const char *name,header *h,int what)
{
    int i;
    for(i=0;i<n_work_list;i++) {
        if (strcmp(name,work_list[i].filename)==0) {
            if (what == 0) {
                /* create existing file == error */
                return (-1);
            } else if (what == 1) {
                /* someone trying to open this one provide the data */
                memcpy(h,work_list+i,sizeof(header));
                return(0);
            } else if (what == 2) {
                int j;
                /* remove this one */
                for(j=i+1;j<n_work_list;j++) work_list[j-1] = work_list[j];
                n_work_list--;
                return(0);
            }
        }
    }
    /* add a new one */
    if (what == 0) {
        if (n_work_list >= MAX_STREAMS) return(-1);
        memcpy(work_list+n_work_list,h,sizeof(header));
        n_work_list++;
        return(0);
    }
    return(-1);
}

int cvio_sock_startup(cvio_stream *s, char *host, char *name,
                                   int port, int wait, float f_wait)
{
        struct sockaddr_in sin;
	int err;

        s->port = port;

	/* create the socket */
	s->s_listen = socket(AF_INET,SOCK_STREAM,0);
	if (s->s_listen == -1) return(-1);

	/* bind to a port */
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	if (s->port == CVIO_PORT_NONE) {
		for(s->port=0;s->port<100;s->port++) {
#ifdef DEBUG
printf("socket startup (%d %d)\n",s->port,CVIO_PORT_BASE+s->port);
#endif
			sin.sin_port = htons(CVIO_PORT_BASE+s->port);
			err = bind(s->s_listen,(struct sockaddr *)&sin,
				   sizeof(sin));
			if (err != -1) break;
		}
	} else {
		sin.sin_port = htons(CVIO_PORT_BASE+s->port);
		err = bind(s->s_listen,(struct sockaddr *)&sin,sizeof(sin));
	}
	if (err == -1) {
		closesocket(s->s_listen);
		return(-1);
	}
	err = listen(s->s_listen,5);
	if (err == -1) {
		closesocket(s->s_listen);
		return(-1);
	}

        s->n_wait = wait;
        s->f_wait = f_wait;
	s->n_readers = 0;
	s->s_readers = NULL;
        strcpy(s->hdr.filename,name);

#ifndef WIN32
        /* we do not want the SIGPIPE signal (Broken pipe) */
        signal(SIGPIPE, SIG_IGN);
#endif

	/* allocate an initial buffer */
	s->sbuf = sbuffer_create(cvio_sock_samp_size(s,1));
	if (!s->sbuf) {
		closesocket(s->s_listen);
		return(-1);
	}

        /* we are a writer */
        s->hdr.flags |= HDR_FLAGS_HAS_WRITER;

	/* startup the listening thread */
	pthread_mutex_init(&s->s_mutex,NULL);
        if (pthread_create(&s->worker,NULL,cvio_sock_writeproc,(void*)s)) {
		sbuffer_destroy(s->sbuf);
                pthread_mutex_destroy(&s->s_mutex);
		closesocket(s->s_listen);
                return(-1);
        }
        return(0);
}

void *cvio_sock_writeproc(void *inarg)
{
	fd_set r_set;
	struct timeval tout;
	int i,err;
        cvio_stream *s = (cvio_stream *)inarg;
        CVIO_DT_TIME final = 0;

        while(1) {
                /* check for new connections (select) */
		FD_ZERO(&r_set);
		FD_SET(s->s_listen,&r_set);
		tout.tv_sec = 0;
		tout.tv_usec = (1.0/80000.) * 1.0e6;
		err = select(s->s_listen+1,&r_set,NULL,NULL,&tout);
		if (err < 0) {
		    /* real error (we will try to shutdown) */
                    pthread_mutex_lock(&s->s_mutex);
		    s->port = -1;
                    pthread_mutex_unlock(&s->s_mutex);
		} else if (err == 0) {
		    /* timeout */
		}
		/* new connection? (handshake it) */
		if (FD_ISSET(s->s_listen,&r_set)) {
		    struct sockaddr_in peer_addr;
		    unsigned int len;
		    SOCKET r;
#ifdef DEBUG
printf("Incoming connection\n");
#endif

		    len = sizeof(struct sockaddr_in);
		    r = accept(s->s_listen,(struct sockaddr *)&peer_addr,&len);
		    if (r >= 0) {
			SOCKET *tmp;

			/* send details (header structure & name) */
			if (sock_write(r,&(s->hdr),sizeof(header))) {
			    shutdown(r,2);
			    closesocket(r);
			    r = -1;
			} else {
			    /* receive an ack or nack (4bytes) */
			    if (sock_read(r,&len,sizeof(len), 1) != 1) {
			        shutdown(r,0);
			        closesocket(r);
                                r = -1;
			    } else {
				/* nonzero = nack */
				if (len) {
#ifdef DEBUG
printf("nack\n");
#endif
			            shutdown(r,0);
			            closesocket(r);
                                    r = -1;
                                }
 			    }
			}

			/* if ack, add cxn & decrement n_wait */
			if (r >= 0) {
			    tmp=(SOCKET *)malloc((s->n_readers+1)*
						sizeof(SOCKET));
			    if (!tmp) {
			        shutdown(r,2);
			        closesocket(r);
			    } else {
			        if (s->s_readers) {
			    	    memcpy(tmp,s->s_readers,
					s->n_readers*sizeof(SOCKET));
				    free(s->s_readers);
			        }
			        tmp[s->n_readers++] = r;
			        s->s_readers = tmp;
			        if (s->n_wait > 0) s->n_wait--;
#ifdef DEBUG
printf("adding a connection wait=%d\n",s->n_wait);
#endif
			    }
			}
		    }
		}

                /* check for new (usable) data & send */
                pthread_mutex_lock(&s->s_mutex);
                if ((s->n_wait < 1) && sbuffer_count(s->sbuf)) send_samples(s,0);

                /* we can timeout to exit as well */
                if ((s->port == -1)&&(s->f_wait > 0.0)) {
                    /* start the timeout timer */
                    if (final == 0) {
                	final = cvio_current_time();
                    } else if (cvio_current_time()-final > 
                               s->f_wait*CVIO_TIME_TICKS_PER_SEC) {
                        break;
                    }
                }

                /* are we done yet? */
                if (s->n_wait < 1) if (s->port == -1) break;

                pthread_mutex_unlock(&s->s_mutex);
        }

        /* no more incoming */
        shutdown(s->s_listen,0);
        closesocket(s->s_listen);

	/* push out all remaining data */
	send_samples(s,1);

        /* close all socket connections down */
        for(i=0;i<s->n_readers;i++)  {
                shutdown(s->s_readers[i],0);
                closesocket(s->s_readers[i]);
        }
        s->n_readers = 0;

        pthread_mutex_unlock(&s->s_mutex);

        return(NULL);
}

void sock_remove_samples(cvio_stream *s,CVIO_DT_TIME *times,void *samps,int n)
{
    int i, size = cvio_sock_samp_size(s, 0);
    char whole[cvio_sock_samp_size(s, 1)];
    char *data = whole + sizeof(CVIO_DT_TIME);
    char *p = samps;

    for(i=0;i<n;i++) {
       sbuffer_get_samples(s->sbuf, whole, 1);
       memcpy(times+i, whole, sizeof(CVIO_DT_TIME));
       if (samps) memcpy(p, data, size);
       p += size;
    }
    return;
}

/* if timeout == -1, try to read *num samples if not enough there, return EOF*/

/* if timeout != -1, read what one can (up to *num samples), but don't
 * wait more then timeout "ticks" i (timeout=0 is indefinite) */

/* Note: current_sample & current_time are handled above... */

int sock_read_samples(cvio_stream *s,CVIO_DT_TIME *times,void *samps,
   int32_t *num, uint32_t timeout)
{
    CVIO_DT_TIME starttime;
    int n = 0;
    int ssize,ssizeT;
    volatile cvio_stream *s_v = s;

    ssize = cvio_sock_samp_size(s,0);
    ssizeT = cvio_sock_samp_size(s,1);

    pthread_mutex_lock(&s->s_mutex); 

    /* handle the (-1) case, immediate read or fail */
    n = s->hdr.nsamp - s->pos.current_sample;
    if (timeout == -1) {
        if (n >= *num) {
            sock_remove_samples(s,times,samps,*num);
            pthread_mutex_unlock(&s->s_mutex); 
            return(0);
        } else {
            *num = 0;
            pthread_mutex_unlock(&s->s_mutex); 
            return(0);
        }
     }
     /* if we have some samples, return them */
     if (n) {
        if (n > *num) n = *num;
        *num = n;
        sock_remove_samples(s,times,samps,*num);
        pthread_mutex_unlock(&s->s_mutex); 
        return(0);
     } 

     /* waiting forever... */
     starttime = cvio_current_time();
     while(s_v->hdr.nsamp == s_v->pos.current_sample) {
        /* must allow the reading thread to do I/O */
        pthread_mutex_unlock(&s->s_mutex); 
        cvio_sleep(1.0/80000.);
        pthread_mutex_lock(&s->s_mutex); 
        /* has the writer shutdown? */
        /* if there are samples (even if the writer is dead), return them */
        if ((s_v->port == -1)&&(s_v->hdr.nsamp == s_v->pos.current_sample)) {
            pthread_mutex_unlock(&s->s_mutex); 
            return(-1);
        }
        /* timeout case */
        if (timeout != 0) {
            if (cvio_current_time() - starttime > timeout) {
                *num =0;
                return(0);
            }
        }
    }
    /* return whatever samples are there */
    n = s_v->hdr.nsamp - s_v->pos.current_sample;
    if (n > *num) n = *num;
    *num = n;
    sock_remove_samples(s,times,samps,*num);
    pthread_mutex_unlock(&s->s_mutex); 
    return(0);
}

/* could run the "socket" as a separate process and even writers
 * connect to it.  More complex code though and it would "hang
 * around" like shmem */

/* Add write blocking?  Maximum buffer size & wait on readers? */

int sock_add_samples(cvio_stream *s,CVIO_DT_TIME *times,void *samps,
                     int32_t num)
{
    int i,ssize;
    char *p,*src;

    pthread_mutex_lock(&s->s_mutex); 

    /* useful values */
    ssize = cvio_sock_samp_size(s,0);

    /* copy in the new samples */
    src = (char *)samps;

    for(i=0;i<num;i++) {
        int scount = 1;
        p = sbuffer_new_ptr(s->sbuf, &scount);
	if (!p) {
             pthread_mutex_unlock(&s->s_mutex); 
             return(-1);
        }
        /* first the timestamp */
        memcpy(p,times+i,sizeof(CVIO_DT_TIME)); p += sizeof(CVIO_DT_TIME);
        /* second the payload */
        memcpy(p,src,ssize);
        src += ssize;
        sbuffer_commit(s->sbuf, scount);
    }

    pthread_mutex_unlock(&s->s_mutex); 
    
    return(0);
}

static int send_samples(cvio_stream *s,int all)
{
    int err, i, ssize, num, len, left, tsize;
    unsigned char *tbuffer;

    /* if being asked to dump all samples, and there
     * are no readers, we are done! */
    if (all && (s->n_readers == 0)) return(0);

    ssize = cvio_sock_samp_size(s,1);
    left = sbuffer_count(s->sbuf);
    while(left) {
	/* pick a number of samples */
	/* try to send at least 1000 bytes */
        /* +1 in case samples are > 1000 bytes each */
	tsize = 1 + 1000 / ssize;
	num = (left < tsize)?left:tsize;
        len = num * ssize;

/* sbuffer_get could return a pointer to some small number
   of contiguous samples and this copy could be avoided.
   (maybe important because malloc sucks badly on OS X)
 */
        tbuffer = malloc(num * ssize);
        if (tbuffer == NULL) return -1;
        sbuffer_get_samples(s->sbuf, tbuffer, num);
	/* send some (num) samples! */
       	for(i=0;i<s->n_readers;i++)  {
#ifdef DEBUG
printf("sending %d samples to reader %d of %d\n",num,i,s->n_readers);
#endif
		err = sock_write(s->s_readers[i], tbuffer, len);
		if (err < 0) {
			shutdown(s->s_readers[i],0);
			closesocket(s->s_readers[i]);	
			s->s_readers[i] = 0;
		}
	}
        free(tbuffer);
	/* remove closed sockets */
	num = 0;
       	for(i=0;i<s->n_readers;i++)  {
		if (s->s_readers[i] > 0) {
			s->s_readers[num++] = s->s_readers[i];
		}
	}
	s->n_readers = num;

	/* if not trying to send all, we can return */
	if (!all) return(0);
        left = sbuffer_count(s->sbuf);
    }
    return(0);
}

int32_t cvio_listsockets(char *host, char *names, int32_t *size, int32_t *num)
{
	SOCKET s;
	struct hostent *hp;
	struct sockaddr_in sin;
	header hdr;
	int len,err,pt;
	char *p, *p2;

	p2 = host;
	if (strlen(p2) == 0) p2 = "localhost";

	hp = gethostbyname(p2);
	if (!hp) return(-1);

        len = *size;
        p = names;
        *size = 0;

        *num = 0;
	/* scan the ports on "host" */
	for(pt=0;pt<100;pt++) {
            int l;

	    /* create the socket */
	    s = socket(AF_INET,SOCK_STREAM,0);
	    if (s == -1) return(-1);

	    /* pick the target */
	    sin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	    sin.sin_family = AF_INET;
	    sin.sin_port = htons(CVIO_PORT_BASE+pt);
	    l = sizeof(struct sockaddr_in);

	    /* try to hookup */
	    err = connect(s,(struct sockaddr *)&sin,l);
	    if (err == 0) {
		/* readback the header */
	        if (sock_read(s,&(hdr),sizeof(hdr), 1) == 1) {
		    /* swap it */
	            cvio_swap_header(&(hdr));
	            /* found one!! */
                    l = strlen(p2) + strlen(hdr.filename)+1+7;
                    if (len - l > 0) {
                        sprintf(p, ":sock:%s:%s", p2, hdr.filename);
                        *num += 1;
                        len -= l;
                        *size += l;
                        p += l;
                    }

                    /* nack & close */
		    err = 1;
		    sock_write(s,&err,sizeof(err));
                    closesocket(s);
                    shutdown(s,2);
                }
            }
        }
                    
        return(CVIO_ERR_OK);
}

struct sbuffer_node {
	struct sbuffer_node *next;
	unsigned char *data;
};

struct sbuffer {
	int atomsize;
	int allocsize;
	int inactives;
	int readoffset;
	int writeoffset;
	int samples;
	struct sbuffer_node *active_list;
	struct sbuffer_node *inactive_list;
	struct sbuffer_node *head;
};

static struct sbuffer_node *newnode(struct sbuffer *buf)
{
	struct sbuffer_node *out;

	if (buf->inactives) {
		out = buf->inactive_list;
		buf->inactive_list = buf->inactive_list->next;
		out->next = NULL;
		buf->inactives--;
		return out;
	}
	out = malloc(sizeof(struct sbuffer_node));
	if (out == NULL)
		return NULL;
	out->data = malloc(buf->allocsize);
	if (out->data == NULL) {
		free(out);
		return NULL;
	}
	out->next = NULL;
	return out;
}

static void nodedel(struct sbuffer *buf, struct sbuffer_node *node)
{
	if (buf->inactives > 100) {
		free(node->data);
		free(node);
	} else {
		node->next = buf->inactive_list;
		buf->inactive_list = node;
		buf->inactives++;
	}
}

static struct sbuffer *sbuffer_create(int atomsize)
{
	struct sbuffer *out;
	int peralloc;

	out = malloc(sizeof(struct sbuffer));
	if (out == NULL)
		return NULL;
	out->atomsize = atomsize;
	peralloc = 1 + 20000/atomsize;
	out->allocsize = peralloc * atomsize;
	out->inactives = 0;
	out->inactive_list = NULL;
	out->active_list = newnode(out);
	out->samples = 0;
	if (out->active_list == NULL) {
		free(out);
		return NULL;
	}
	out->active_list->next = NULL;
	out->head = out->active_list;
	out->readoffset = 0;
	out->writeoffset = 0;
	return out;
}

static void sbuffer_destroy(struct sbuffer *buf)
{
	struct sbuffer_node *curr, *tmp;
	curr = buf->active_list;
	while (curr) {
		tmp = curr;
		curr = curr->next;
		free(tmp->data);
		free(tmp);
	}
	curr = buf->inactive_list;
	while (curr) {
		tmp = curr;
		curr = curr->next;
		free(tmp->data);
		free(tmp);
	}
	free(buf);
}

static void *sbuffer_new_ptr(struct sbuffer *buf, int *samples)
{
	int avail;
	void *out;

	if (buf->writeoffset + buf->atomsize > buf->allocsize) {
		/* we need another node... */
		buf->head->next = newnode(buf);
		if (buf->head->next == NULL) {
			*samples = 0;
			return NULL;
		}
		buf->writeoffset = 0;
		buf->head = buf->head->next;
	}

	out = buf->head->data + buf->writeoffset;
	avail = (buf->allocsize - buf->writeoffset) / buf->atomsize;
	if (*samples > avail)
		*samples = avail;

	if (!out) {
		*samples = 0;
	}
	return out;
}

static int sbuffer_get_samples(struct sbuffer *buf, void *target, int count)
{
	int i;
	struct sbuffer_node *tmp;
	unsigned char *ptr = target;

	for (i = 0; i < count; i++) {
		if (buf->readoffset + buf->atomsize > buf->allocsize) {
			buf->readoffset = 0;
			tmp = buf->active_list;
			buf->active_list = buf->active_list->next;
			nodedel(buf, tmp);
			if (buf->active_list == NULL)
				printf("broken\n");
		}
		memcpy(ptr, buf->active_list->data + buf->readoffset, buf->atomsize);
		buf->readoffset += buf->atomsize;
		ptr += buf->atomsize;
	}
	buf->samples -= count;
	return 1;
}

static int sbuffer_count(struct sbuffer *buf)
{
	return buf->samples;
}

static void sbuffer_commit(struct sbuffer *buf, int samples)
{
	buf->writeoffset += samples * buf->atomsize;
	buf->samples += samples;
}
