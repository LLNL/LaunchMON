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
 * This file provides common implementations for
 *   pmgr_collective_mpirun - the interface used by mpirun
 *   pmgr_collective_client - the interface used by the MPI tasks
 *
 * Copyright (C) 2007 The Regents of the University of California.
 * Produced at Lawrence Livermore National Laboratory.
 * Author: Adam Moody <moody20@llnl.gov>
*/

#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "pmgr_collective_common.h"

/*
   my rank
   -3     ==> unitialized task (may be mpirun or MPI task)
   -2     ==> mpirun
   -1     ==> MPI task before rank is assigned
   0..N-1 ==> MPI task
*/
int pmgr_me = -3;

int pmgr_echo_debug = 0;

/* Return the number of secs as a double between two timeval structs (tv2-tv1) */
double pmgr_getsecs(struct timeval* tv2, struct timeval* tv1)
{
        struct timeval result;
        timersub(tv2, tv1, &result);
        return (double) result.tv_sec + (double) result.tv_usec / 1000000.0;
}

/* Fills in timeval via gettimeofday */
void pmgr_gettimeofday(struct timeval* tv)
{
        if (gettimeofday(tv, NULL) < 0) {
                pmgr_error("getting time (gettimeofday() %m errno=%d)",
                        errno);
        }
}

/* Reads environment variable, bails if not set */
char* pmgr_getenv(char* envvar, int type)
{
    char* str = getenv(envvar);
    if (str == NULL && type == ENV_REQUIRED) {
        pmgr_error("Missing required environment variable: %s", envvar);
        exit(1);
    }
    return str;
}

/* malloc n bytes, and bail out with error msg if fails */
void* pmgr_malloc(size_t n, char* msg)
{
    void* p = malloc(n);
    if (!p) {
        pmgr_error("malloc(%d) failed: %s (errno %d)", n, msg, errno);
        exit(1);
    }
    return p;
}

/* print message to stderr */
void pmgr_error(char *fmt, ...)
{
    va_list argp;
    fprintf(stderr, "PMGR_COLLECTIVE ERROR: ");
    if (pmgr_me >= 0) {
        fprintf(stderr, "%d: ", pmgr_me);
    } else if (pmgr_me == -2) {
        fprintf(stderr, "mpirun: ");
    } else if (pmgr_me == -1) {
        fprintf(stderr, "unitialized MPI task: ");
    } else {
        fprintf(stderr, "unitialized task (mpirun or MPI): ");
    }
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    fprintf(stderr, "\n");
}

/* print message to stderr */
void pmgr_debug(int level, char *fmt, ...)
{
    va_list argp;
    if (pmgr_echo_debug > 0 && pmgr_echo_debug >= level) {
        fprintf(stderr, "PMGR_COLLECTIVE DEBUG: ");
        if (pmgr_me >= 0) {
            fprintf(stderr, "%d: ", pmgr_me);
        } else if (pmgr_me == -2) {
            fprintf(stderr, "mpirun: ");
        } else if (pmgr_me == -1) {
            fprintf(stderr, "unitialized MPI task: ");
        } else {
            fprintf(stderr, "unitialized task (mpirun or MPI): ");
        }
        va_start(argp, fmt);
        vfprintf(stderr, fmt, argp);
        va_end(argp);
        fprintf(stderr, "\n");
    }
}

/* write size bytes from buf into fd, retry if necessary */
int pmgr_write_fd(int fd, void* buf, int size)
{
    int rc;
    int n = 0;
    char* offset = (char*) buf;

    while (n < size) {
	rc = write(fd, offset, size - n);

	if (rc < 0) {
	    if(errno == EINTR || errno == EAGAIN) { continue; }
	    return rc;
	} else if(rc == 0) {
	    return n;
	}

	offset += rc;
	n += rc;
    }

    return n;
}

/* read size bytes into buf from fd, retry if necessary */
int pmgr_read_fd(int fd, void* buf, int size)
{
    int rc;
    int n = 0;
    char* offset = (char*) buf;

    while (n < size) {
	rc = read(fd, offset, size - n);

	if (rc < 0) {
	    if(errno == EINTR || errno == EAGAIN) { continue; }
	    return rc;
	} else if(rc == 0) {
	    return n;
	}

	offset += rc;
	n += rc;
    }

    return n;
}
