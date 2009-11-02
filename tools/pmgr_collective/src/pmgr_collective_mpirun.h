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
 * This file defines the interface used by mpirun.  The mpirun process should call
 * pmgr_processops after accepting connections from the MPI tasks and negotiating
 * the protocol version number (PMGR_COLLECTIVE uses protocol 8).
 *
 * It should provide an array of open socket file descriptors indexed by MPI rank
 * (fds) along with the number of MPI tasks (nprocs) as arguments.
 *
 * pmgr_processops will handle all PMGR_COLLECTIVE operations and return control
 * upon an error or after receiving PMGR_CLOSE from the MPI tasks.  If no errors
 * are encountered, it will close all socket file descriptors before returning.
 *
 * Copyright (C) 2007 The Regents of the University of California.
 * Produced at Lawrence Livermore National Laboratory.
 * Author: Adam Moody <moody20@llnl.gov>
*/

#ifndef _PMGR_COLLECTIVE_MPIRUN_H
#define _PMGR_COLLECTIVE_MPIRUN_H

#include "pmgr_collective_common.h"

/*
  pmgr_processops
  This function carries out pmgr_collective operations to bootstrap MPI.
  These collective operations are modeled after MPI collectives -- all tasks
  must call them in the same order and with consistent parameters.

  fds - integer array of open sockets (file descriptors)
        indexed by MPI rank
  nprocs - number of MPI tasks in job

  returns PMGR_SUCCESS on success
  If no errors are encountered, all sockets are closed before returning.
*/
int pmgr_processops(int* fds, int nprocs);
    
#if LAUNCHMON_FINEGRAIN_MPIRUN_INTERFACE
int pmgr_process_singleop (int *fd, int nprocs, int targetop);
#endif /* FINEGRAIN_MPIRUN_INTERFACE */

#endif /* _PMGR_COLLECTIVE_MPIRUN_H */
