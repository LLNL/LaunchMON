/*
 * Copyright (c) 2009, Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 * Written by Adam Moody <moody20@llnl.gov>.
 * LLNL-CODE-411040.
 * All rights reserved.
 * This file is part of the PMGR_COLLECTIVE library.
 * For details, see https://sourceforge.net/projects/pmgrcollective.
 * Please also read this file: LICENSE.TXT.
*/

/*
 * PMGR_COLLECTIVE ============================================================
 * This protocol enables MPI to bootstrap itself through a series of collective
 * operations.  The collective operations are modeled after MPI collectives --
 * all tasks must call them in the same order and with consistent parameters.
 *
 * MPI may invoke any number of collectives, in any order, passing an arbitrary
 * amount of data.  All message sizes are specified in bytes.
 * PMGR_COLLECTIVE ============================================================
 *
 * This file implements the interface used by the MPI tasks (clients).
 *
 * An MPI task should make calls in the following sequenece:
 *
 *   pmgr_init
 *   pmgr_open
 *   [collectives]
 *   pmgr_close
 *   pmgr_finalize
 *
 * MPI may invoke any number of collectives, in any order, passing an arbitrary
 * amount of data.  All message sizes are specified in bytes.
 *
 * All functions return PMGR_SUCCESS on successful completion.
*/

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "pmgr_collective_client.h"

#define PMGR_DEBUG_LEVELS (3)

/* set env variable to select which trees to use, if any -- all enabled by default */
#ifndef MPIRUN_USE_TREES
#define MPIRUN_USE_TREES (1)
#endif
#ifndef MPIRUN_USE_GATHER_TREE
#define MPIRUN_USE_GATHER_TREE (1)
#endif
#ifndef MPIRUN_USE_SCATTER_TREE
#define MPIRUN_USE_SCATTER_TREE (1)
#endif
#ifndef MPIRUN_USE_BCAST_TREE
#define MPIRUN_USE_BCAST_TREE (1)
#endif

/* set env variable to configure socket timeout parameters */
#ifndef MPIRUN_CONNECT_TRIES
#define MPIRUN_CONNECT_TRIES (7)
#endif
#ifndef MPIRUN_CONNECT_TIMEOUT
#define MPIRUN_CONNECT_TIMEOUT (2) /* seconds */
#endif
#ifndef MPIRUN_CONNECT_BACKOFF
#define MPIRUN_CONNECT_BACKOFF (5) /* seconds */
#endif
#ifndef MPIRUN_CONNECT_RANDOM
#define MPIRUN_CONNECT_RANDOM (1) /* enable/disable randomized option for backoff */
#endif

/* set envvar MPIRUN_USE_TREES={0,1} to disable/enable tree algorithms */
static int mpirun_use_trees       = MPIRUN_USE_TREES;
/* set envvar MPIRUN_USE_GATHER_TREE={0,1} to disable/enable gather tree */
static int mpirun_use_gather_tree  = MPIRUN_USE_GATHER_TREE;
/* set envvar MPIRUN_USE_SCATTER_TREE={0,1} to disable/enable scatter tree */
static int mpirun_use_scatter_tree = MPIRUN_USE_SCATTER_TREE;
/* set envvar MPIRUN_USE_BCAST_TREE={0,1} to disable/enable bcast tree */
static int mpirun_use_bcast_tree  = MPIRUN_USE_BCAST_TREE;

static int mpirun_connect_tries    = MPIRUN_CONNECT_TRIES;
static int mpirun_connect_timeout  = MPIRUN_CONNECT_TIMEOUT; /* seconds */
static int mpirun_connect_backoff  = MPIRUN_CONNECT_BACKOFF; /* seconds */
static int mpirun_connect_random   = MPIRUN_CONNECT_RANDOM;

static char* mpirun_hostname;
static int   mpirun_port;
static int   mpirun_socket;
static int   pmgr_nprocs = -1;
static int   pmgr_id     = -1;

/* tree data structures */
static int  pmgr_parent;         /* MPI rank of parent */
static int  pmgr_parent_s;       /* socket fd to parent */
static int* pmgr_child;          /* MPI ranks of children */
static int* pmgr_child_s;        /* socket fds to children */
static int  pmgr_num_child;      /* number of children */
static int* pmgr_child_incl;     /* number of children each child is responsible for (includes itself) */
static int  pmgr_num_child_incl; /* total number of children this node is responsible for */
static struct in_addr* pmgr_child_ip;   /* IP addresses of children */
static short*          pmgr_child_port; /* port number of children */

/* startup time, time between starting pmgr_open and finishing pmgr_close */
static struct timeval time_open, time_close;

static unsigned pmgr_backoff_rand_seed;

/*
 * =============================
 * Utility functions for use by other functions in this file
 * =============================
 */

/* read size bytes into buf from mpirun_socket */
static int pmgr_read(void* buf, int size)
{
    int rc = 0;
    if ((rc = pmgr_read_fd(mpirun_socket, buf, size)) < 0) {
        pmgr_error("Reading from mpirun at %s:%d (read(buf=%x,size=%d) %m errno=%d) @ file %s:%d",
            mpirun_hostname, mpirun_port, buf, size, errno, __FILE__, __LINE__);
    }
    return rc;
}

/* write size bytes into mpirun_socket from buf */
static int pmgr_write(void* buf, int size)
{
    int rc = 0;
    if ((rc = pmgr_write_fd(mpirun_socket, buf, size)) < 0) {
        pmgr_error("Writing to mpirun at %s:%d (write(buf=%x,size=%d) %m errno=%d) @ file %s:%d",
            mpirun_hostname, mpirun_port, buf, size, errno, __FILE__, __LINE__);
    }
    return rc;
}

/* write integer into mpirun_socket */
static int pmgr_write_int(int value)
{
    return pmgr_write(&value, sizeof(value));
}

/* 
 * =============================
 * The mpirun_* functions implement PMGR_COLLECTIVE operations through
 * the mpirun process.  Typically, this amounts to a flat tree with the
 * mpirun process at the root.  These functions implement the client side
 * of the protocol specified in pmgr_collective_mpirun.c.
 * =============================
 */

/*
 * Perform barrier, each task writes an int then waits for an int
 */
static int mpirun_barrier()
{
    /* send BARRIER op code, then wait on integer reply */
    int buf;

    pmgr_write_int(PMGR_BARRIER);
    pmgr_read(&buf, sizeof(int));

    return PMGR_SUCCESS;
}

/*
 * Perform MPI-like Broadcast, root writes sendcount bytes from buf,
 * into mpirun_socket, all receive sendcount bytes into buf
 */
static int mpirun_bcast(void* buf, int sendcount, int root)
{
    /* send BCAST op code, then root, then size of data */
    pmgr_write_int(PMGR_BCAST);
    pmgr_write_int(root);
    pmgr_write_int(sendcount);

    /* if i am root, send data */
    if (pmgr_me == root) {
      pmgr_write(buf, sendcount);
    }

    /* read in data */
    pmgr_read(buf, sendcount);

    return PMGR_SUCCESS;
}

/*
 * Perform MPI-like Gather, each task writes sendcount bytes from sendbuf
 * into mpirun_socket, then root receives N*sendcount bytes into recvbuf
 */
static int mpirun_gather(void* sendbuf, int sendcount, void* recvbuf, int root)
{
    /* send GATHER op code, then root, then size of data, then data itself */
    pmgr_write_int(PMGR_GATHER);
    pmgr_write_int(root);
    pmgr_write_int(sendcount);
    pmgr_write(sendbuf, sendcount);

    /* only the root receives data */
    if (pmgr_me == root) {
       pmgr_read(recvbuf, sendcount * pmgr_nprocs);
    }

    return PMGR_SUCCESS;
}

/*
 * Perform MPI-like Scatter, root writes N*sendcount bytes from sendbuf
 * into mpirun_socket, then each task receives sendcount bytes into recvbuf
 */
static int mpirun_scatter(void* sendbuf, int sendcount, void* recvbuf, int root)
{
    /* send SCATTER op code, then root, then size of data, then data itself */
    pmgr_write_int(PMGR_SCATTER);
    pmgr_write_int(root);
    pmgr_write_int(sendcount);

    /* if i am root, send all chunks to mpirun */
    if (pmgr_me == root) {
      pmgr_write(sendbuf, sendcount * pmgr_nprocs);
    }

    /* receive my chunk */
    pmgr_read(recvbuf, sendcount);

    return PMGR_SUCCESS;
}

/*
 * Perform MPI-like Allgather, each task writes sendcount bytes from sendbuf
 * into mpirun_socket, then receives N*sendcount bytes into recvbuf
 */
static int mpirun_allgather(void* sendbuf, int sendcount, void* recvbuf)
{
    /* send ALLGATHER op code, then size of data, then data itself */
    pmgr_write_int(PMGR_ALLGATHER);
    pmgr_write_int(sendcount);
    pmgr_write(sendbuf, sendcount);
    pmgr_read (recvbuf, sendcount * pmgr_nprocs);

    return PMGR_SUCCESS;
}

/*
 * Perform MPI-like Alltoall, each task writes N*sendcount bytes from sendbuf
 * into mpirun_socket, then recieves N*sendcount bytes into recvbuf
 */
static int mpirun_alltoall(void* sendbuf, int sendcount, void* recvbuf)
{
    /* send ALLTOALL op code, then size of data, then data itself */
    pmgr_write_int(PMGR_ALLTOALL);
    pmgr_write_int(sendcount);
    pmgr_write(sendbuf, sendcount * pmgr_nprocs);
    pmgr_read (recvbuf, sendcount * pmgr_nprocs);

    return PMGR_SUCCESS;
}

/* 
 * =============================
 * Functions to open/close/gather/bcast the TCP/socket tree.
 * =============================
*/

/* Open a connection on socket FD to peer at ADDR (which LEN bytes long).
 * This function uses a non-blocking filedescriptor for the connect(),
 * and then does a bounded poll() for the connection to complete.  This
 * allows us to timeout the connect() earlier than TCP might do it on
 * its own.  We have seen timeouts that failed after several minutes,
 * where we would really prefer to time out earlier and retry the connect.
 *
 * Return 0 on success, -1 for errors.
 */
static int pmgr_connect_w_timeout (int fd, struct sockaddr const * addr,
                                   socklen_t len, int millisec)
{
    int rc, flags, err;
    socklen_t err_len;
    struct pollfd ufds;

    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    err = 0;
    rc = connect(fd , addr , len);
    if (rc < 0 && errno != EINPROGRESS) {
        pmgr_error("Nonblocking connect failed immediately (connect() %m errno=%d) @ file %s:%d",
            errno, __FILE__, __LINE__);
        return -1;
    }
    if (rc == 0) {
        goto done;  /* connect completed immediately */
    }

    ufds.fd = fd;
    ufds.events = POLLIN | POLLOUT;
    ufds.revents = 0;

again:	rc = poll(&ufds, 1, millisec);
    if (rc == -1) {
        /* poll failed */
        if (errno == EINTR) {
            /* NOTE: connect() is non-interruptible in Linux */
            goto again;
        } else {
            pmgr_error("Polling connection (poll() %m errno=%d) @ file %s:%d",
                errno, __FILE__, __LINE__);
        }
        return -1;
    } else if (rc == 0) {
        /* poll timed out before any socket events */
        /* perror("pmgr_connect_w_timeout poll timeout"); */
        return -1;
    } else {
        /* poll saw some event on the socket
         * We need to check if the connection succeeded by
         * using getsockopt.  The revent is not necessarily
         * POLLERR when the connection fails! */
        err_len = (socklen_t) sizeof(err);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR,
                       &err, &err_len) < 0)
        {
            pmgr_error("Failed to read event on socket (getsockopt() %m errno=%d) @ file %s:%d",
                errno, __FILE__, __LINE__);
            return -1; /* solaris pending error */
        }
    }

done:
    fcntl(fd, F_SETFL, flags);

    /* NOTE: Connection refused is typically reported for
     * non-responsive nodes plus attempts to communicate
     * with terminated launcher. */
    if (err) {
        pmgr_error("Error on socket in pmgr_connect_w_timeout() (getsockopt() set err=%d) @ file %s:%d",
            err, __FILE__, __LINE__);
        return -1;
    }
 
    return 0;
}

/* Connect to given IP:port.  Upon successful connection, pmgr_connect
 * shall return the connected socket file descriptor.  Otherwise, -1 shall be
 * returned.
 */
static int pmgr_connect(struct in_addr ip, int port)
{
    struct sockaddr_in sockaddr;
    int sockfd;
    int i;

    /* set up address to connect to */
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr = ip;
    sockaddr.sin_port = port;

    /* Try making the connection several times, with a random backoff
       between tries. */
    for (i = 0; ; i++) {
        /* create a socket */
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd < 0) {
            pmgr_error("Creating socket (socket() %m errno=%d) @ file %s:%d",
                errno, __FILE__, __LINE__);
            return -1;
        }

        /* connect socket to address */
        if (pmgr_connect_w_timeout(sockfd, (struct sockaddr *) &sockaddr,
                              sizeof(sockaddr), mpirun_connect_timeout * 1000) < 0) {
            if (i >= mpirun_connect_tries) {
                pmgr_error("Connecting socket: pmgr_connect_w_timeout() failed @ file %s:%d",
                    __FILE__, __LINE__);
                close(sockfd);
                return -1;
            } else {
                close(sockfd);
                if (mpirun_connect_random) {
                    usleep(((rand_r(&pmgr_backoff_rand_seed) % (mpirun_connect_backoff * 1000)) + 1) * 1000);
                } else {
                    usleep(mpirun_connect_backoff * 1000 * 1000);
                }
            }
        } else {
            break;
        }
    }

    return sockfd;
}

/* open socket tree across MPI tasks */
static int pmgr_open_tree()
{
    /* currently implements a binomial tree */

    /* initialize parent and children based on pmgr_me and pmgr_nprocs */
    int rc = PMGR_SUCCESS;
    int n = 1;
    int max_children = 0;
    while (n < pmgr_nprocs) {
      n <<= 1;
      max_children++;
    }

    pmgr_parent = 0;
    pmgr_num_child = 0;
    pmgr_num_child_incl = 0;
    pmgr_child      = (int*) pmgr_malloc(max_children * sizeof(int), "Child MPI rank array");
    pmgr_child_s    = (int*) pmgr_malloc(max_children * sizeof(int), "Child socket fd array");
    pmgr_child_incl = (int*) pmgr_malloc(max_children * sizeof(int), "Child children count array");
    pmgr_child_ip   = (struct in_addr*) pmgr_malloc(max_children * sizeof(struct in_addr), "Child IP array");
    pmgr_child_port = (short*)          pmgr_malloc(max_children * sizeof(short),          "Child port array");

    /* find our parent and list of children */
    int low  = 0;
    int high = pmgr_nprocs - 1;
    while (high - low > 0) {
        int mid = (high - low) / 2 + (high - low) % 2 + low;
        if (low == pmgr_me) {
            pmgr_child[pmgr_num_child] = mid;
            pmgr_child_incl[pmgr_num_child] = high - mid + 1;
            pmgr_num_child++;
            pmgr_num_child_incl += (high - mid + 1);
        }
        if (mid == pmgr_me) { pmgr_parent = low; }
        if (mid <= pmgr_me) { low  = mid; }
        else                { high = mid-1; }
    }

    /* create a socket to accept connection from parent */
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        pmgr_error("Creating parent socket (socket() %m errno=%d) @ file %s:%d",
            errno, __FILE__, __LINE__);
        exit(1);
    }

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(0); /* bind ephemeral port - OS will assign us a free port */

    /* bind socket */
    if (bind(sockfd, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        pmgr_error("Binding parent socket (bind() %m errno=%d) @ file %s:%d",
            errno, __FILE__, __LINE__);
        exit(1);
    }

    /* set the socket to listen for connections */
    if (listen(sockfd, 1) < 0) {
        pmgr_error("Setting parent socket to listen (listen() %m errno=%d) @ file %s:%d",
            errno, __FILE__, __LINE__);
        exit(1);
    }

    /* ask which port the OS assigned our socket to */
    socklen_t len = sizeof(sin);
    if (getsockname(sockfd, (struct sockaddr *) &sin, &len) < 0) {
        pmgr_error("Reading parent socket port number (getsockname() %m errno=%d) @ file %s:%d",
            errno, __FILE__, __LINE__);
        exit(1);
    }

    /* extract our ip and port number to send to mpirun */
    char hn[256];
    gethostname(hn, 256);
    struct hostent* he = gethostbyname(hn);
    struct in_addr ip = * (struct in_addr *) *(he->h_addr_list);
    short port = sin.sin_port;

    /* allocate buffers to receive ip:port table for all tasks */
    int sendcount = sizeof(ip) + sizeof(port);
    void* sendbuf = (void*) pmgr_malloc(sendcount, "Send buffer for socket data");
    void* recvbuf = (void*) pmgr_malloc(sendcount * pmgr_nprocs, "Receive buffer for socket table");

    /* fill in send buffer with our ip:port */
    memcpy(sendbuf, &ip, sizeof(ip));
    memcpy((char*)sendbuf + sizeof(ip), &port, sizeof(port));

    /* gather ip:port info to rank 0 -- explicitly call mpirun_gather since tcp tree is not setup */
    mpirun_gather(sendbuf, sendcount, recvbuf, 0);

    /* if i'm not rank 0, accept a connection (from parent) and receive socket table */
    if (pmgr_me != 0) {
        socklen_t parent_len;
        struct sockaddr parent_addr;
	parent_len = sizeof(parent_addr);
        pmgr_parent_s = accept(sockfd, (struct sockaddr *) &parent_addr, &parent_len);
        if (pmgr_read_fd(pmgr_parent_s, recvbuf, sendcount * pmgr_nprocs) < 0) {
            pmgr_error("Receiving IP:port table from parent failed @ file %s:%d",
                __FILE__, __LINE__);
            rc = !PMGR_SUCCESS;
        }
    }

    /* for each child, open socket connection and forward socket table */
    int i;
    for(i=0; i<pmgr_num_child; i++) {
        int c = pmgr_child[i];
        pmgr_child_ip[i]   = * (struct in_addr *)  ((char*)recvbuf + sendcount*c);
        pmgr_child_port[i] = * (short*) ((char*)recvbuf + sendcount*c + sizeof(ip));
        pmgr_child_s[i]    = pmgr_connect(pmgr_child_ip[i], pmgr_child_port[i]);
        if (pmgr_child_s[i] == -1) {
            pmgr_error("Connecting to child (rank %d) at %s:%d failed @ file %s:%d",
                pmgr_child[i], inet_ntoa(pmgr_child_ip[i]), htons(pmgr_child_port[i]), __FILE__, __LINE__);
            rc = !PMGR_SUCCESS;
            exit(1);
        }
        if (pmgr_write_fd(pmgr_child_s[i], recvbuf, sendcount * pmgr_nprocs) < 0) {
            pmgr_error("Writing IP:port table to child (rank %d) at %s:%d failed @ file %s:%d",
                pmgr_child[i], inet_ntoa(pmgr_child_ip[i]), htons(pmgr_child_port[i]), __FILE__, __LINE__);
            rc = !PMGR_SUCCESS;
        }
    }

    pmgr_free(sendbuf);
    pmgr_free(recvbuf);

    return rc;
}

/*
 * close down socket connections for tree (parent and any children), free
 * related memory
 */
static int pmgr_close_tree()
{
    /* if i'm not rank 0, close socket connection with parent */
    if (pmgr_me != 0) {
        close(pmgr_parent_s);
    }

    /* and all children */
    int i;
    for(i=0; i<pmgr_num_child; i++) {
        close(pmgr_child_s[i]);
    }

    /* free data structures */
    pmgr_free(pmgr_child);
    pmgr_free(pmgr_child_s);
    pmgr_free(pmgr_child_incl);
    pmgr_free(pmgr_child_ip);
    pmgr_free(pmgr_child_port);

    return PMGR_SUCCESS;
}

/* broadcast size bytes from buf on rank 0 using socket tree */
static int pmgr_bcast_tree(void* buf, int size)
{
    int rc = PMGR_SUCCESS;
    int i;

    /* if i'm not rank 0, receive data from parent */
    if (pmgr_me != 0) {
        if (pmgr_read_fd(pmgr_parent_s, buf, size) < 0) {
            pmgr_error("Receiving broadcast data from parent failed @ file %s:%d",
                __FILE__, __LINE__);
            rc = !PMGR_SUCCESS;
        }
    }

    /* for each child, forward data */
    for(i=0; i<pmgr_num_child; i++) {
        if (pmgr_write_fd(pmgr_child_s[i], buf, size) < 0) {
            pmgr_error("Broadcasting data to child (rank %d) at %s:%d failed @ file %s:%d",
                pmgr_child[i], inet_ntoa(pmgr_child_ip[i]), htons(pmgr_child_port[i]), __FILE__, __LINE__);
            rc = !PMGR_SUCCESS;
        }
    }

    return rc;
}

/* gather sendcount bytes from sendbuf on each task into recvbuf on rank 0 */
static int pmgr_gather_tree(void* sendbuf, int sendcount, void* recvbuf)
{
    int rc = PMGR_SUCCESS;
    int bigcount = (pmgr_num_child_incl+1) * sendcount;
    void* bigbuf = recvbuf;

    /* if i'm not rank 0, create a temporary buffer to gather child data */
    if (pmgr_me != 0) {
        bigbuf = (void*) pmgr_malloc(bigcount, "Temporary gather buffer in pmgr_gather_tree");
    }

    /* copy my own data into buffer */
    memcpy(bigbuf, sendbuf, sendcount);

    /* if i have any children, receive their data */
    int i;
    int offset = sendcount;
    for(i=pmgr_num_child-1; i>=0; i--) {
        if (pmgr_read_fd(pmgr_child_s[i], (char*)bigbuf + offset, sendcount * pmgr_child_incl[i]) < 0) {
            pmgr_error("Gathering data from child (rank %d) at %s:%d failed @ file %s:%d",
                pmgr_child[i], inet_ntoa(pmgr_child_ip[i]), htons(pmgr_child_port[i]), __FILE__, __LINE__);
            rc = !PMGR_SUCCESS;
        }
        offset += sendcount * pmgr_child_incl[i];
    }

    /* if i'm not rank 0, send to parent and free temporary buffer */
    if (pmgr_me != 0) {
        if (pmgr_write_fd(pmgr_parent_s, bigbuf, bigcount) < 0) {
            pmgr_error("Sending gathered data to parent failed @ file %s:%d",
                __FILE__, __LINE__);
            rc = !PMGR_SUCCESS;
        }
        pmgr_free(bigbuf);
    }

    return rc;
}

/* scatter sendcount byte chunks from sendbuf on rank 0 to recvbuf on each task */
static int pmgr_scatter_tree(void* sendbuf, int sendcount, void* recvbuf)
{
    int rc = PMGR_SUCCESS;
    int bigcount = (pmgr_num_child_incl+1) * sendcount;
    void* bigbuf = sendbuf;

    /* if i'm not rank 0, create a temporary buffer to receive child data, and receive data from parent */
    if (pmgr_me != 0) {
        bigbuf = (void*) pmgr_malloc(bigcount, "Temporary scatter buffer in pmgr_scatter_tree");
        if (pmgr_read_fd(pmgr_parent_s, bigbuf, bigcount) < 0) {
            pmgr_error("Receiving scatter data from parent failed @ file %s:%d",
                __FILE__, __LINE__);
            rc = !PMGR_SUCCESS;
        }
    }

    /* if i have any children, receive their data */
    int i;
    int offset = sendcount;
    for(i=0; i<pmgr_num_child; i++) {
        if (pmgr_write_fd(pmgr_child_s[i], (char*)bigbuf + offset, sendcount * pmgr_child_incl[i]) < 0) {
            pmgr_error("Scattering data to child (rank %d) at %s:%d failed @ file %s:%d",
                pmgr_child[i], inet_ntoa(pmgr_child_ip[i]), htons(pmgr_child_port[i]), __FILE__, __LINE__);
            rc = !PMGR_SUCCESS;
        }
        offset += sendcount * pmgr_child_incl[i];
    }

    /* copy my data into buffer */
    memcpy(recvbuf, bigbuf, sendcount);

    /* if i'm not rank 0, free temporary buffer */
    if (pmgr_me != 0) {
        pmgr_free(bigbuf);
    }

    return rc;
}

/*
 * =============================
 * The pmgr_* collectives are the user interface (what the MPI tasks call).
 * =============================
 */

/* Perform barrier, each task writes an int then waits for an int */
int pmgr_barrier()
{
    struct timeval start, end;
    pmgr_gettimeofday(&start);
    pmgr_debug(3, "Starting pmgr_barrier()");

    char c;
    void* recvbuf = NULL;

    /* check that we have an mpirun process */
    if (mpirun_hostname != NULL) {

    if (mpirun_use_trees) {
        /* allocate temporary gather buffer on rank 0 */
        if (pmgr_me == 0) {
            recvbuf = (void*) pmgr_malloc(sizeof(c) * pmgr_nprocs, "Temporary gather buffer in pmgr_barrier");
        }

        /* gather a character to rank 0 */
        if (mpirun_use_gather_tree) {
            pmgr_gather_tree(&c, sizeof(c), recvbuf);
	} else {
            mpirun_gather(&c, sizeof(c), recvbuf, 0);
	}

        /* broadcast a character from rank 0 */
        if (mpirun_use_bcast_tree) {
            pmgr_bcast_tree(&c, sizeof(c));
	} else {
            mpirun_bcast(&c, sizeof(c), 0);
	}

        /* free temporary gather buffer on rank 0 */
        if (pmgr_me == 0) {
            pmgr_free(recvbuf);
        }
    } else {
        /* trees aren't enabled, use mpirun to do barrier */
        mpirun_barrier();
    }

    } /* check that (mpirun_hostname != NULL) */

    pmgr_gettimeofday(&end);
    pmgr_debug(2, "Exiting pmgr_barrier(), took %f seconds for %d procs", pmgr_getsecs(&end,&start), pmgr_nprocs);
    return PMGR_SUCCESS;
}

/*
 * Perform MPI-like Broadcast, root writes sendcount bytes from buf,
 * into mpirun_socket, all receive sendcount bytes into buf
 */
int pmgr_bcast(void* buf, int sendcount, int root)
{
    struct timeval start, end;
    pmgr_gettimeofday(&start);
    pmgr_debug(3, "Starting pmgr_bcast()");

    int rc = PMGR_SUCCESS;

    /* check that we have an mpirun process */
    if (mpirun_hostname != NULL) {

    /* if root is rank 0 and bcast tree is enabled, use it */
    /* (this is a common case) */
    if (root == 0 && mpirun_use_trees && mpirun_use_bcast_tree) {
        rc = pmgr_bcast_tree(buf, sendcount);
    } else {
        rc = mpirun_bcast(buf, sendcount, root);
    }

    } /* check that (mpirun_hostname != NULL) */

    pmgr_gettimeofday(&end);
    pmgr_debug(2, "Exiting pmgr_bcast(), took %f seconds for %d procs", pmgr_getsecs(&end,&start), pmgr_nprocs);
    return rc;
}

/*
 * Perform MPI-like Gather, each task writes sendcount bytes from sendbuf
 * into mpirun_socket, then root receives N*sendcount bytes into recvbuf
 */
int pmgr_gather(void* sendbuf, int sendcount, void* recvbuf, int root)
{
    struct timeval start, end;
    pmgr_gettimeofday(&start);
    pmgr_debug(3, "Starting pmgr_gather()");

    int rc = PMGR_SUCCESS;

    /* check that we have an mpirun process */
    if (mpirun_hostname != NULL) {

    /* if root is rank 0 and gather tree is enabled, use it */
    /* (this is a common case) */
    if (root == 0 && mpirun_use_trees && mpirun_use_gather_tree) {
        rc = pmgr_gather_tree(sendbuf, sendcount, recvbuf);
    } else {
        rc = mpirun_gather(sendbuf, sendcount, recvbuf, root);
    }

    } else { /* check that (mpirun_hostname != NULL) */
        /* just copy the data over */
        memcpy(recvbuf, sendbuf, sendcount);
    }

    pmgr_gettimeofday(&end);
    pmgr_debug(2, "Exiting pmgr_gather(), took %f seconds for %d procs", pmgr_getsecs(&end,&start), pmgr_nprocs);
    return rc;
}

/*
 * Perform MPI-like Scatter, root writes N*sendcount bytes from sendbuf
 * into mpirun_socket, then each task receives sendcount bytes into recvbuf
 */
int pmgr_scatter(void* sendbuf, int sendcount, void* recvbuf, int root)
{
    struct timeval start, end;
    pmgr_gettimeofday(&start);
    pmgr_debug(3, "Starting pmgr_scatter()");

    int rc = PMGR_SUCCESS;

    /* check that we have an mpirun process */
    if (mpirun_hostname != NULL) {

    /* if root is rank 0 and gather tree is enabled, use it */
    /* (this is a common case) */
    if (root == 0 && mpirun_use_trees && mpirun_use_scatter_tree) {
        rc = pmgr_scatter_tree(sendbuf, sendcount, recvbuf);
    } else {
        rc = mpirun_scatter(sendbuf, sendcount, recvbuf, root);
    }


    } else { /* check that (mpirun_hostname != NULL) */
        /* just copy the data over */
        memcpy(recvbuf, sendbuf, sendcount);
    }

    pmgr_gettimeofday(&end);
    pmgr_debug(2, "Exiting pmgr_scatter(), took %f seconds for %d procs", pmgr_getsecs(&end,&start), pmgr_nprocs);
    return rc;
}

/*
 * Perform MPI-like Allgather, each task writes sendcount bytes from sendbuf
 * into mpirun_socket, then receives N*sendcount bytes into recvbuf
 */
int pmgr_allgather(void* sendbuf, int sendcount, void* recvbuf)
{
    struct timeval start, end;
    pmgr_gettimeofday(&start);
    pmgr_debug(3, "Starting pmgr_allgather()");

    /* check that we have an mpirun process */
    if (mpirun_hostname != NULL) {

    if (mpirun_use_trees) {
        /* gather data to rank 0 */
        if (mpirun_use_gather_tree) {
            pmgr_gather_tree(sendbuf, sendcount, recvbuf);
	} else {
            mpirun_gather(sendbuf, sendcount, recvbuf, 0);
	}

        /* broadcast data from rank 0 */
        if (mpirun_use_bcast_tree) {
            pmgr_bcast_tree(recvbuf, sendcount * pmgr_nprocs);
	} else {
            mpirun_bcast(recvbuf, sendcount * pmgr_nprocs, 0);
	}
    } else {
        /* trees aren't enabled, use mpirun to do allgather */
        mpirun_allgather(sendbuf, sendcount, recvbuf);
    }

    } else { /* check that (mpirun_hostname != NULL) */
        /* just copy the data over */
        memcpy(recvbuf, sendbuf, sendcount);
    }

    pmgr_gettimeofday(&end);
    pmgr_debug(2, "Exiting pmgr_allgather(), took %f seconds for %d procs", pmgr_getsecs(&end,&start), pmgr_nprocs);
    return PMGR_SUCCESS;
}

/*
 * Perform MPI-like Alltoall, each task writes N*sendcount bytes from sendbuf
 * into mpirun_socket, then recieves N*sendcount bytes into recvbuf
 */
int pmgr_alltoall(void* sendbuf, int sendcount, void* recvbuf)
{
    struct timeval start, end;
    pmgr_gettimeofday(&start);
    pmgr_debug(3, "Starting pmgr_alltoall()");

    int rc = PMGR_SUCCESS;

    /* check that we have an mpirun process */
    if (mpirun_hostname != NULL) {

    rc = mpirun_alltoall(sendbuf, sendcount, recvbuf);

    } else { /* check that (mpirun_hostname != NULL) */
        /* just copy the data over */
        memcpy(recvbuf, sendbuf, sendcount);
    }

    pmgr_gettimeofday(&end);
    pmgr_debug(2, "Exiting pmgr_alltoall(), took %f seconds for %d procs", pmgr_getsecs(&end,&start), pmgr_nprocs);
    return rc;
}

/*
 * Perform MPI-like Allreduce maximum of a single int from each task
 */
int pmgr_allreducemaxint(int* sendint, int* recvint)
{
    struct timeval start, end;
    pmgr_gettimeofday(&start);
    pmgr_debug(3, "Starting pmgr_allreducemaxint()");

    /* allocate space to receive ints from everyone */
    int* all = NULL;
    if (pmgr_me == 0) {
        all = (int*) pmgr_malloc((size_t)pmgr_nprocs * sizeof(int), "One int for each task");
    }

    /* gather all ints to rank 0 */
    pmgr_gather((void*) sendint, sizeof(int), (void*) all, 0);

    /* rank 0 searches through list for maximum value */
    int max = *sendint;
    if (pmgr_me == 0) {
        int i;
        for (i=0; i<pmgr_nprocs; i++) {
            if (all[i] > max) { max = all[i]; }
        }
        pmgr_free(all);
    }

    /* broadcast max int from rank 0 and set recvint */
    pmgr_bcast((void*) &max, sizeof(int), 0);
    *recvint = max;

    pmgr_gettimeofday(&end);
    pmgr_debug(2, "Exiting pmgr_allreducemaxint(), took %f seconds for %d procs", pmgr_getsecs(&end,&start), pmgr_nprocs);
    return PMGR_SUCCESS;
}

/*
 * Perform MPI-like Allgather of NULL-terminated strings (whose lengths may vary
 * from task to task).
 *
 * Each task provides a pointer to its NULL-terminated string as input.
 * Each task then receives an array of pointers to strings indexed by rank number
 * and also a pointer to the buffer holding the string data.
 * When done with the strings, both the array of string pointers and the
 * buffer should be freed.
 *
 * Example Usage:
 *   char host[256], **hosts, *buf;
 *   gethostname(host, sizeof(host));
 *   pmgr_allgatherstr(host, &hosts, &buf);
 *   for(int i=0; i<nprocs; i++) { printf("rank %d runs on host %s\n", i, hosts[i]); }
 *   free(hosts);
 *   free(buf);
 */
int pmgr_allgatherstr(char* sendstr, char*** recvstr, char** recvbuf)
{
    struct timeval start, end;
    pmgr_gettimeofday(&start);
    pmgr_debug(3, "Starting pmgr_allgatherstr()");

    /* determine max length of send strings */
    int mylen  = strlen(sendstr) + 1;
    int maxlen = 0;
    pmgr_allreducemaxint(&mylen, &maxlen);

    /* pad my string to match max length */
    char* mystr = (char*) pmgr_malloc(maxlen, "Padded String");
    memset(mystr, '\0', maxlen);
    strcpy(mystr, sendstr);

    /* allocate enough buffer space to receive a maxlen string from all tasks */
    char* stringbuf = (char*) pmgr_malloc(pmgr_nprocs * maxlen, "String Buffer");

    /* gather strings from everyone */
    pmgr_allgather((void*) mystr, maxlen, (void*) stringbuf);

    /* set up array and free temporary maxlen string */
    char** strings = (char **) pmgr_malloc(pmgr_nprocs * sizeof(char*), "Array of String Pointers");
    int i;
    for (i=0; i<pmgr_nprocs; i++) {
        strings[i] = stringbuf + i*maxlen;
    }
    pmgr_free(mystr);

    *recvstr = strings;
    *recvbuf = stringbuf;

    pmgr_gettimeofday(&end);
    pmgr_debug(2, "Exiting pmgr_allgatherstr(), took %f seconds for %d procs", pmgr_getsecs(&end,&start), pmgr_nprocs);
    return PMGR_SUCCESS;
}

/*
 * Open socket to mpirun launch process, then send protocol version and rank
 * number
 */
int pmgr_open()
{
    struct timeval start, end;
    pmgr_gettimeofday(&time_open);
    pmgr_gettimeofday(&start);
    pmgr_debug(3, "Starting pmgr_open()");

    /* seed used for random backoff in pmgr_connect later */
    pmgr_backoff_rand_seed = time_open.tv_usec + pmgr_me;

    /* check that we have an mpirun process */
    if (mpirun_hostname != NULL) {

    struct hostent* mpirun_hostent = gethostbyname(mpirun_hostname);
    if (!mpirun_hostent) {
        pmgr_error("Hostname lookup of mpirun failed (gethostbyname(%s) %s h_errno=%d) @ file %s:%d",
            mpirun_hostname, hstrerror(h_errno), h_errno, __FILE__, __LINE__);
        exit(1);
    }

    mpirun_socket = pmgr_connect(*(struct in_addr *) (*mpirun_hostent->h_addr_list),
                                 htons(mpirun_port));
    if (mpirun_socket == -1) {
        pmgr_error("Connecting mpirun socket to %s at %s:%d failed @ file %s:%d",
            mpirun_hostent->h_name, inet_ntoa(*(struct in_addr *) (*mpirun_hostent->h_addr_list)),
            mpirun_port, __FILE__, __LINE__);
        exit(1);
    }

    /* we are now connected to the mpirun process */

    /* 
     * Exchange information with mpirun.  If you make any changes
     * to this protocol, be sure to increment the version number
     * in the header file.  This is to permit compatibility with older
     * executables.
     */

    /* send version number, then rank */
    pmgr_write_int(PMGR_COLLECTIVE);
    pmgr_write(&pmgr_me, sizeof(pmgr_me));


    if (pmgr_me == -1) {  
      pmgr_read(&pmgr_me, sizeof(pmgr_me));     
    }
    pmgr_id = pmgr_me;
   
    if (pmgr_nprocs == -1) { 
      pmgr_read(&pmgr_nprocs, sizeof(pmgr_nprocs));  
    }

    /* open up socket tree, if enabled */
    if (mpirun_use_trees) {
        pmgr_open_tree();
    }
    } /* check that (mpirun_hostname != NULL) */

    pmgr_gettimeofday(&end);
    pmgr_debug(2, "Exiting pmgr_open(), took %f seconds for %d procs", pmgr_getsecs(&end,&start), pmgr_nprocs);
    return PMGR_SUCCESS;
}

int pmgr_getmyrank (int *rank) 
{
  if ( (pmgr_id >= 0) && (pmgr_id < pmgr_nprocs) ) {
    (*rank) = pmgr_id; 
    return PMGR_SUCCESS;
  }

  return -1;
}

int pmgr_getmysize (int *size) 
{
  if ( size > 0 ) {
    (*size) = pmgr_nprocs; 
    return PMGR_SUCCESS;
  }

  return -1;
}

int pmgr_getsockfd (int *fd)
{
  if ( mpirun_socket > 0 )
    {
      (*fd) = mpirun_socket;
      return PMGR_SUCCESS;
    }

  return mpirun_socket;
}


/*
 * Closes the mpirun socket
 */
int pmgr_close()
{
    struct timeval start, end;
    pmgr_gettimeofday(&start);
    pmgr_debug(3, "Starting pmgr_close()");

    /* check that we have an mpirun process */
    if (mpirun_hostname != NULL) {

    /* shut down the tree, if enabled */
    if (mpirun_use_trees) {
        pmgr_close_tree();
    }

    /* send CLOSE op code, then close socket */
    pmgr_write_int(PMGR_CLOSE);
    close(mpirun_socket);

    } /* check that (mpirun_hostname != NULL) */

    pmgr_gettimeofday(&end);
    pmgr_gettimeofday(&time_close);
    pmgr_debug(2, "Exiting pmgr_close(), took %f seconds for %d procs", pmgr_getsecs(&end,&start), pmgr_nprocs);
    pmgr_debug(1, "Total time from pmgr_open() to pmgr_close() took %f seconds for %d procs",
        pmgr_getsecs(&time_close, &time_open), pmgr_nprocs);
    return PMGR_SUCCESS;
}

/*
 * =============================
 * Handle init and finalize
 * =============================
 */

int pmgr_init(int *argc_p, char ***argv_p, int *np_p, int *me_p, int *id_p)
{
    int i = 0;
    int n_pmgropt = 0;
    char **nargv = *argv_p;
    int lazy_rank_assignment = 0;
    int lazy_size_assignment = 0;

    setvbuf(stdout, NULL, _IONBF, 0);
    char *value;

    struct timeval start, end;
    pmgr_gettimeofday(&start);

    for (i=1; i < (*argc_p); ++i) {
      if ( (nargv[i][0] == '-') && (nargv[i][1]) ) {
        if ( strncmp (&(nargv[i][2]), "pmgrsize=", 9)  == 0 )
	  {
	    /* exporting a commandline option to an envVar */
            setenv ("MPIRUN_NPROCS", &(nargv[i][2])+9, 1); 
	    n_pmgropt++;
	  }
	else if ( strncmp (&(nargv[i][2]), "pmgrip=", 7) == 0 ) 
	  {
	    /* exporting a commandline option to an envVar */
            setenv ("MPIRUN_HOST", &(nargv[i][2])+7, 1); 
	    setenv (TOOL_HOST_ENV, &(nargv[i][2])+7, 1);
	    n_pmgropt++;
	  }
	else if ( strncmp (&(nargv[i][2]), "pmgrport=", 9)== 0 ) 
	  {
	    /* exporting a commandline option to an envVar */
            setenv ("MPIRUN_PORT", &(nargv[i][2])+9, 1); 
	    setenv (TOOL_PORT_ENV, &(nargv[i][2])+9, 1);
	    n_pmgropt++;
  	  }
	else if ( strncmp (&(nargv[i][2]), "pmgrjobid=", 10)== 0 ) 
	  {
	    /* exporting a commandline option to an envVar */
            setenv ("MPIRUN_ID", &(nargv[i][2])+10, 1); 
	    n_pmgropt++;
	  }	
	else if ( strncmp (&(nargv[i][2]), "pmgrsharedsec=", 14)== 0 ) 
	  {
	    /* exporting a commandline option to an envVar */
            setenv (TOOL_SS_ENV, &(nargv[i][2])+14, 1); 
	    n_pmgropt++;
	  }	
	else if ( strncmp (&(nargv[i][2]), "pmgrsecchk=", 11)== 0 ) 
	  {
	    /* exporting a commandline option to an envVar */
            setenv (TOOL_SCH_ENV, &(nargv[i][2])+11, 1); 
	    n_pmgropt++;
	  }	
	else if ( strncmp (&(nargv[i][2]), "pmgrdebug=", 10)== 0 ) 
	  {
	    /* exporting a commandline option to an envVar */
            setenv ("MPIRUN_CLIENT_DEBUG", &(nargv[i][2])+10, 1); 
	    n_pmgropt++;
	  }
	else if ( strncmp (&(nargv[i][2]), "pmgrlazyrank=", 13)== 0 ) 
	  {
	    /* exporting a commandline option to an envVar */
            lazy_rank_assignment = 1; 
	    n_pmgropt++;
  	  }
	else if ( strncmp (&(nargv[i][2]), "pmgrlazysize=", 13)== 0 ) 
	  {
	    /* exporting a commandline option to an envVar */
            lazy_size_assignment = 1; 
	    n_pmgropt++;
  	  }
      }
    } /* for */

    (*argc_p) -= n_pmgropt;
    nargv[(*argc_p)+0] = NULL;        	 
    char tmpbuf[1024];
    for (i=1; i < (n_pmgropt+1); ++i )
      {
	 sprintf(tmpbuf, "PMGRNOOP%d=%d", i,i);
         /* this region of memory might not be safe to be freed, so we just leak */
	 /* and nargv[(*argc_p)+n_pmgropt] must be null */
	 nargv[(*argc_p)+i] = strdup (tmpbuf); 
      }

    pmgr_echo_debug = 0;

    /* =======================================================
     * Until told otherwise, assume we are rank 0 of a 1-task MPI job
     * this enables serial launching, e.g., "./mpiHello" vs "mpirun -np 1 ./mpiHello"
     * TODO: may want to protect this usage via a compile flag and/or env var
     * ======================================================= */

    /* Take a stab at something unique for the id (timestamp.secs | pid)
     * TODO: !!NOTE!!
     * Using a pid in the jobid *ONLY* works for a single process job
     * Obviously, multiple tasks will have different pids */
    pmgr_id = 0x7FFFFFFF & ((start.tv_sec << 16) | (0x0000FFFF & getpid()));

    if (lazy_rank_assignment) {
      pmgr_me     = -1;
    }
    else {
      pmgr_me     = 0;
    }

    if (lazy_size_assignment) {
      pmgr_me     = -1;
    }
    else {
      pmgr_nprocs = 1;
    }

    mpirun_hostname = NULL;
    mpirun_port = 1;

    /* =======================================================
     * Get information from environment, not from the argument list
     * ======================================================= */

    /* if MPIRUN_RANK is set, require RANK, NPROCS, ID, HOST, and PORT to all be set */
    /* this ensures that if one process aborts in a multitask job,
     * then something is there to abort the others, namely the mpirun process */

    if ( !lazy_rank_assignment
	 && (value = pmgr_getenv("MPIRUN_RANK", ENV_OPTIONAL)) != NULL) {
        /* MPI rank of current process */
        pmgr_me = atoi(pmgr_getenv("MPIRUN_RANK", ENV_REQUIRED));

        /* number of MPI processes in job */
        pmgr_nprocs = atoi(pmgr_getenv("MPIRUN_NPROCS", ENV_REQUIRED));

        /* unique jobid of current application */
        pmgr_id = atoi(pmgr_getenv("MPIRUN_ID", ENV_REQUIRED));
   }

   /* mpirun host IP string in dotted decimal notation */
   mpirun_hostname = strdup(pmgr_getenv("MPIRUN_HOST", ENV_REQUIRED));

   /* mpirun port number */
   mpirun_port = atoi(pmgr_getenv("MPIRUN_PORT", ENV_REQUIRED));


    /* MPIRUN_USE_TREES={0,1} disables/enables tree algorithms */
    if ((value = pmgr_getenv("MPIRUN_USE_TREES", ENV_OPTIONAL))) {
        mpirun_use_trees = atoi(value);
    }

    /* MPIRUN_USE_GATHER_TREE={0,1} disables/enables gather tree */
    if ((value = pmgr_getenv("MPIRUN_USE_GATHER_TREE", ENV_OPTIONAL))) {
        mpirun_use_gather_tree = atoi(value);
    }

    /* MPIRUN_USE_SCATTER_TREE={0,1} disables/enables scatter tree */
    if ((value = pmgr_getenv("MPIRUN_USE_SCATTER_TREE", ENV_OPTIONAL))) {
        mpirun_use_scatter_tree = atoi(value);
    }

    /* MPIRUN_USE_BCAST_TREE={0,1} disables/enables bcast tree */
    if ((value = pmgr_getenv("MPIRUN_USE_BCAST_TREE", ENV_OPTIONAL))) {
        mpirun_use_bcast_tree = atoi(value);
    }

    if ((value = pmgr_getenv("MPIRUN_CONNECT_TRIES", ENV_OPTIONAL))) {
            mpirun_connect_tries = atoi(value);
    }

    /* seconds */
    if ((value = pmgr_getenv("MPIRUN_CONNECT_TIMEOUT", ENV_OPTIONAL))) {
            mpirun_connect_timeout = atoi(value);
    }

    /* seconds */
    if ((value = pmgr_getenv("MPIRUN_CONNECT_BACKOFF", ENV_OPTIONAL))) {
            mpirun_connect_backoff = atoi(value);
    }

    /* enable/disable radomized option in backoff */
    if ((value = pmgr_getenv("MPIRUN_CONNECT_RANDOM", ENV_OPTIONAL))) {
            mpirun_connect_random = atoi(value);
    }

    /* =======================================================
     * Check that we have valid values
     * ======================================================= */

    /* MPIRUN_CLIENT_DEBUG={0,1} disables/enables debug statements */
    /* this comes *after* MPIRUN_RANK and MPIRUN_NPROCS since those are used to print debug messages */
    if ((value = pmgr_getenv("MPIRUN_CLIENT_DEBUG", ENV_OPTIONAL)) != NULL) {
        pmgr_echo_debug = atoi(value);
        int print_rank = 0;
        if (pmgr_echo_debug > 0) {
            if        (pmgr_echo_debug <= 1*PMGR_DEBUG_LEVELS) {
                print_rank = (pmgr_me == 0); /* just rank 0 prints */
            } else if (pmgr_echo_debug <= 2*PMGR_DEBUG_LEVELS) {
                print_rank = (pmgr_me == 0 || pmgr_me == pmgr_nprocs-1); /* just rank 0 and rank N-1 print */
            } else {
                print_rank = 1; /* all ranks print */
            }
            if (print_rank) {
                pmgr_echo_debug = 1 + (pmgr_echo_debug-1) % PMGR_DEBUG_LEVELS;
            } else {
                pmgr_echo_debug = 0;
            }
        }
    }
    pmgr_debug(3, "In pmgr_init():\n" \
        "  MPIRUN_RANK: %d, MPIRUN_NPROCS: %d, MPIRUN_ID: %d, MPIRUN_HOST: %s, MPIRUN_PORT: %d\n" \
        "  MPIRUN_USE_TREES: %d, MPIRUN_USE_GATHER_TREE: %d, MPIRUN_USE_SCATTER_TREE: %d, MPIRUN_USE_BCAST_TREE: %d\n" \
        "  MPIRUN_CONNECT_TRIES: %d, MPIRUN_CONNECT_TIMEOUT: %d, MPIRUN_CONNECT_BACKOFF: %d, MPIRUN_CONNECT_RANDOM: %d",
        pmgr_me, pmgr_nprocs, pmgr_id, mpirun_hostname, mpirun_port,
        mpirun_use_trees, mpirun_use_gather_tree, mpirun_use_scatter_tree, mpirun_use_bcast_tree,
        mpirun_connect_tries, mpirun_connect_timeout, mpirun_connect_backoff, mpirun_connect_random
    );

    /* Performs the sanity check only when lazy assignments aren't set */
    if (!lazy_rank_assignment && lazy_size_assignment) {

      /* check that we have a valid number of processes */
      if ( pmgr_nprocs <= 0) {
        pmgr_error("Invalid MPIRUN_NPROCS %s @ file %s:%d",
            pmgr_getenv("MPIRUN_NPROCS", ENV_REQUIRED), __FILE__, __LINE__);
        exit(1);
      }

      /* check that our rank is valid */
      if (pmgr_me < 0 || pmgr_me >= pmgr_nprocs) {
        pmgr_error("Invalid MPIRUN_RANK %s @ file %s:%d",
            pmgr_getenv("MPIRUN_RANK", ENV_REQUIRED), __FILE__, __LINE__);
        exit(1);
      }

      /* check that we have a valid jobid */
      if (pmgr_id == 0) {
        pmgr_error("Invalid MPIRUN_ID %s @ file %s:%d",
            pmgr_getenv("MPIRUN_ID", ENV_REQUIRED), __FILE__, __LINE__);
        exit(1);
      }

      /* check that we have a valid mpirun port */
      if (mpirun_port <= 0) {
        pmgr_error("Invalid MPIRUN_PORT %s @ file %s:%d",
            pmgr_getenv("MPIRUN_PORT", ENV_REQUIRED), __FILE__, __LINE__);
        exit(1);
      }
    }

    *np_p = pmgr_nprocs;
    *me_p = pmgr_me;
    *id_p = pmgr_id;

    pmgr_gettimeofday(&end);
    pmgr_debug(2, "Exiting pmgr_init(), took %f seconds for %d procs", pmgr_getsecs(&end,&start), pmgr_nprocs);
    return PMGR_SUCCESS;
}

/*
 * No cleanup necessary here.
 */
int pmgr_finalize()
{
    if (mpirun_hostname != NULL) { free(mpirun_hostname); }
    return PMGR_SUCCESS;
}

/*
 * =============================
 * Handle aborts
 * =============================
 */

static int vprint_msg(char *buf, size_t len, const char *fmt, va_list ap)
{
    int n;

    n = vsnprintf(buf, len, fmt, ap);

    if ((n >= len) || (n < 0)) {
        /* Add trailing '+' to indicate truncation */
        buf[len - 2] = '+';
        buf[len - 1] = '\0';
    }

    return (0);
}

/*
 * Call into the process spawner, using the same port we were given
 * at startup time, to tell it to abort the entire job.
 */
int pmgr_abort(int code, const char *fmt, ...)
{
    int s;
    struct sockaddr_in sin;
    struct hostent* he;
    va_list ap;
    char buf [256];
    int len;

    /* check that we have an mpirun process */
    if (mpirun_hostname != NULL) {

    he = gethostbyname(mpirun_hostname);
    if (!he) {
        pmgr_error("pmgr_abort: Hostname lookup of mpirun failed (gethostbyname(%s) %s h_errno=%d) @ file %s:%d",
            mpirun_hostname, hstrerror(h_errno), h_errno, __FILE__, __LINE__);
        return -1;
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        pmgr_error("pmgr_abort: Failed to create socket (socket() %m errno=%d) @ file %s:%d",
            errno, __FILE__, __LINE__);
        return -1;
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = he->h_addrtype;
    memcpy(&sin.sin_addr, he->h_addr_list[0], sizeof(sin.sin_addr));
    sin.sin_port = htons(mpirun_port);
    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        pmgr_error("pmgr_abort: Connect to mpirun failed (connect() %m errno=%d) @ file %s:%d",
            errno, __FILE__, __LINE__);
        return -1;
    }

    va_start(ap, fmt);
    vprint_msg(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    /* write an abort code (may be destination rank), our rank to mpirun */
    pmgr_write_fd(s, &code, sizeof(code));
    pmgr_write_fd(s, &pmgr_me, sizeof(pmgr_me));

    /* now length of error string, and error string itself to mpirun */
    len = strlen(buf) + 1;
    pmgr_write_fd(s, &len, sizeof(len));
    pmgr_write_fd(s, buf, len);

    close(s);

    } else { /* check that (mpirun_hostname != NULL) */
        va_start(ap, fmt);
        vprint_msg(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        /* TODO: want to echo this message here?  Want to do this for every user abort? */
        pmgr_error("Called pmgr_abort() Code: %d, Msg: %s", code, buf);
    }

    return PMGR_SUCCESS;
}
