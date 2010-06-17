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
 * This file provides common definitions for
 *   pmgr_collective_mpirun - the interface used by mpirun
 *   pmgr_collective_client - the interface used by the MPI tasks
 *
 * Copyright (C) 2007 The Regents of the University of California.
 * Produced at Lawrence Livermore National Laboratory.
 * Author: Adam Moody <moody20@llnl.gov>
*/

#ifndef PMGR_COLLECTIVE_COMMON_H
#define PMGR_COLLECTIVE_COMMON_H

# include "config.h"

#if defined(_IA64_)
#undef htons
#undef ntohs
#define htons(__bsx) ((((__bsx) >> 8) & 0xff) | (((__bsx) & 0xff) << 8))
#define ntohs(__bsx) ((((__bsx) >> 8) & 0xff) | (((__bsx) & 0xff) << 8))
#endif

/* PMGR_VERSION for pmgr_collective is PMGR_COLLECTIVE (== 8) */
#define PMGR_COLLECTIVE 9

#define PMGR_SUCCESS 0

#define PMGR_OPEN      0
#define PMGR_CLOSE     1
#define PMGR_ABORT     2
#define PMGR_BARRIER   3
#define PMGR_BCAST     4
#define PMGR_GATHER    5
#define PMGR_SCATTER   6
#define PMGR_ALLGATHER 7
#define PMGR_ALLTOALL  8

/*
   my rank
   -3     ==> unitialized task (may be mpirun or MPI task)
   -2     ==> mpirun
   -1     ==> MPI task before rank is assigned
   0..N-1 ==> MPI task
*/
extern int pmgr_me;

extern int pmgr_echo_debug;

/* Return the number of secs as a double between two timeval structs (tv2-tv1) */
double pmgr_getsecs(struct timeval* tv2, struct timeval* tv1);

/* Fills in timeval via gettimeofday */
void pmgr_gettimeofday(struct timeval* tv);

/* Reads environment variable, bails if not set */
#define ENV_REQUIRED 0
#define ENV_OPTIONAL 1
char* pmgr_getenv(char* envvar, int type);

/* malloc n bytes, and bail out with error msg if fails */
void* pmgr_malloc(size_t n, char* msg);

/* macro to free the pointer if set, then set it to NULL */
#define pmgr_free(p) { if(p) { free((void*)p); p=NULL; } }

/* print message to stderr */
void pmgr_error(char *fmt, ...);

/* print message to stderr */
void pmgr_debug(int leve, char *fmt, ...);

/* write size bytes from buf into fd, retry if necessary */
int pmgr_write_fd(int fd, void* buf, int size);

/* read size bytes into buf from fd, retry if necessary */
int pmgr_read_fd (int fd, void* buf, int size);

#endif /* PMGR_COLLECTIVE_COMMON_H */
