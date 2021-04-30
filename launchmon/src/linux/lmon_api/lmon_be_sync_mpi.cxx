/*
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008, Lawrence Livermore National Security, LLC. Produced at
 * the Lawrence Livermore National Laboratory. Written by Dong H. Ahn
 * <ahn1@llnl.gov>. LLNL-CODE-409469. All rights reserved.
 *
 * This file is part of LaunchMON. For details, see
 * https://computing.llnl.gov/?set=resources&page=os_projects
 *
 * Please also read LICENSE.txt -- Our Notice and GNU Lesser General Public
 * License.
 *
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *--------------------------------------------------------------------------------
 *
 *  Update Log:
 *              Apr 01 2015 ADG: Added Cray CTI support.
 *              Dec 14 2012 DHA: Add support to fix MPIR_Breakpoint
 *                               release race
 *              Nov 23 2011 DHA: File created
 *
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/lmon_api_std.h>

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX-like OS
#endif

#include "../../sdbg_rm_map.hxx"
#include "lmon_api/lmon_proctab.h"
#include "lmon_api/lmon_say_msg.hxx"
#include "lmon_be_sync_mpi.hxx"
#include "lmon_be_sync_mpi_bg.hxx"
#include "lmon_be_sync_mpi_bgq.hxx"
#include "lmon_be_sync_mpi_generic.hxx"
#include "lmon_daemon_internal.hxx"

static int dontstop_fastpath = 0;

//////////////////////////////////////////////////////////////////////////////////
//
// LAUNCHMON MPI-Tool SYNC LAYER PUBLIC INTERFACE
//

lmon_rc_e LMON_be_procctl_tester_init(rm_catalogue_e rmtype,
                                      MPIR_PROCDESC_EXT *ptab, int islaunch,
                                      int psize, int dontstop) {
  lmon_rc_e rc = LMON_OK;

  //
  // tester procctller init should be a noop for BGP as OS doesn't
  // allow launchmon layer to cleanly undo the process control effects
  // Thus, the tester cannot go through its own ATTACH sequence cleanly
  // This has been fixed in BGQ though.
  //

  if ((rmtype != RC_bgprm) && (rmtype != RC_bglrm)) {
    rc = LMON_be_procctl_init(rmtype, ptab, islaunch, psize, dontstop);
  }

  return rc;
}

lmon_rc_e LMON_be_procctl_init(rm_catalogue_e rmtype, MPIR_PROCDESC_EXT *ptab,
                               int islaunch, int psize, int dontstop) {
  lmon_rc_e rc = LMON_EINVAL;

  // 0: don't use fastpath
  // 1: use fastpath
  // anything else: inherit the state

  if (dontstop == 0 || dontstop == 1) {
    dontstop_fastpath = dontstop;
  }

#if VERBOSE
  LMON_say_msg(
      LMON_BE_MSG_PREFIX, false,
      "Platform-independent proc control init"
      " visited with rmtype(%d) psize(%d), islaunch(%d), dontstop_fastpath(%d)",
      rmtype, psize, islaunch, dontstop_fastpath);
#endif

  switch (rmtype) {
    case RC_slurm:
      //
      // Call generic Linux init
      //
      if (islaunch) {
        rc = LMON_be_procctl_init_ptrace(ptab, islaunch, psize);
      } else {
        rc = LMON_be_procctl_init_generic(ptab, islaunch, psize);
      }
      break;

    case RC_orte:
    case RC_alps:
    case RC_cray:
    case RC_gupc:
    case RC_mpiexec_hydra:
    case RC_ibm_spectrum:
      //
      // Call generic Linux init
      //
      rc = LMON_be_procctl_init_generic(ptab, islaunch, psize);
      break;

    case RC_bglrm:
    case RC_bgprm:
      //
      // Call RM-specific init with BG CIOD debug interface
      //
      rc = LMON_be_procctl_init_bg(ptab, islaunch, psize);
      break;

    case RC_bgqrm:
    case RC_bgq_slurm:
      //
      // Call RM-specific init with BGQ CDTI interface
      //
      rc = (dontstop_fastpath) ? LMON_OK : LMON_be_procctl_init_bgq(
                                               ptab, islaunch, psize);
      break;

    case RC_mchecker_rm:
    case RC_none:
    default:
#if VERBOSE
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
                   "Unsupported RM Type (%d) for procctl_init", rmtype);
#endif
      break;
  }

  return rc;
}

lmon_rc_e LMON_be_procctl_stop(rm_catalogue_e rmtype, MPIR_PROCDESC_EXT *ptab,
                               int psize) {
  lmon_rc_e rc = LMON_EINVAL;

#if VERBOSE
  LMON_say_msg(LMON_BE_MSG_PREFIX, false,
               "Platform-independent proc control stop"
               " visited with rmtype(%d)",
               rmtype);
#endif

  switch (rmtype) {
    case RC_slurm:
    case RC_orte:
    case RC_alps:
    case RC_cray:
    case RC_gupc:
    case RC_mpiexec_hydra:
    case RC_ibm_spectrum:
      //
      // Call generic Linux stop
      //
      rc = LMON_be_procctl_stop_generic(ptab, psize);
      break;

    case RC_bglrm:
    case RC_bgprm:
      //
      // Call RM-specific stop with BG CIOD debug interface
      //
      rc = LMON_be_procctl_stop_bg(ptab, psize);
      break;

    case RC_bgqrm:
    case RC_bgq_slurm:
      //
      // Call RM-specific stop with BGQ CDTI interface
      //
      rc =
          (dontstop_fastpath) ? LMON_OK : LMON_be_procctl_stop_bgq(ptab, psize);
      break;

    case RC_mchecker_rm:
    case RC_none:
    default:
#if VERBOSE
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
                   "Unsupported RM Type (%d) for procctl_stop", rmtype);
#endif
      break;
  }

  return rc;
}

lmon_rc_e LMON_be_procctl_tester_run(rm_catalogue_e rmtype, int signum,
                                     MPIR_PROCDESC_EXT *ptab, int psize) {
  return LMON_be_procctl_run(rmtype, signum, ptab, psize);
}

lmon_rc_e LMON_be_procctl_run(rm_catalogue_e rmtype, int signum,
                              MPIR_PROCDESC_EXT *ptab, int psize) {
  lmon_rc_e rc = LMON_EINVAL;

#if VERBOSE
  LMON_say_msg(
      LMON_BE_MSG_PREFIX, false,
      "Platform-independent proc control run"
      " visited with rmtype(%d) dontstop_fastpath(%d) signum(%d) psize(%d)",
      rmtype, dontstop_fastpath, signum, psize);
#endif

  switch (rmtype) {
    case RC_slurm:
    case RC_orte:
    case RC_alps:
    case RC_cray:
    case RC_gupc:
    case RC_mpiexec_hydra:
    case RC_ibm_spectrum:
      //
      // Call generic Linux run
      //
      rc = LMON_be_procctl_run_generic(signum, ptab, psize);
      break;

    case RC_bglrm:
    case RC_bgprm:
      //
      // Call RM-specific run with BG CIOD debug interface
      //
      rc = LMON_be_procctl_run_bg(signum, ptab, psize);
      break;

    case RC_bgqrm:
    case RC_bgq_slurm:
      //
      // Call RM-specific run with BGQ CDTI interface
      //
      rc = (dontstop_fastpath) ? LMON_OK
                               : LMON_be_procctl_run_bgq(signum, ptab, psize);
      break;

    case RC_mchecker_rm:
    case RC_none:
    default:
#if VERBOSE
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
                   "Unsupported RM Type (%d) for procctl_run", rmtype);
#endif
      break;
  }

  return rc;
}

lmon_rc_e LMON_be_procctl_initdone(rm_catalogue_e rmtype,
                                   MPIR_PROCDESC_EXT *ptab, int islaunch,
                                   int psize) {
  lmon_rc_e rc = LMON_EINVAL;

#if VERBOSE
  LMON_say_msg(LMON_BE_MSG_PREFIX, false,
               "Platform-independent proc control initdone"
               " visited with rmtype(%d)",
               rmtype);
#endif

  switch (rmtype) {
    case RC_slurm:
      //
      // Call ptrace Linux initdone for launch case
      //
      rc = (islaunch == 1) ? LMON_be_procctl_initdone_ptrace(ptab, psize)
                           : LMON_be_procctl_initdone_generic(ptab, psize);
      break;

    case RC_orte:
    case RC_alps:
    case RC_cray:
    case RC_gupc:
    case RC_mpiexec_hydra:
    case RC_ibm_spectrum:
      //
      // Call generic Linux initdone
      //
      rc = LMON_be_procctl_initdone_generic(ptab, psize);
      break;

    case RC_bglrm:
    case RC_bgprm:
      //
      // Call RM-specific initdone with BG CIOD debug interface
      //
      rc = LMON_be_procctl_initdone_bg(ptab, islaunch, psize);
      break;

    case RC_bgqrm:
    case RC_bgq_slurm:
      //
      // Call RM-specific initdone with BGQ CDTI interface
      //
      rc = (dontstop_fastpath) ? LMON_OK : LMON_be_procctl_initdone_bgq(
                                               ptab, islaunch, psize);
      break;

    case RC_mchecker_rm:
    case RC_none:
    default:
#if VERBOSE
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
                   "Unsupported RM Type (%d) for procctl_initdone", rmtype);
#endif
      break;
  }

  return rc;
}

lmon_rc_e LMON_be_procctl_done(rm_catalogue_e rmtype, MPIR_PROCDESC_EXT *ptab,
                               int psize) {
  lmon_rc_e rc = LMON_EINVAL;

#if VERBOSE
  LMON_say_msg(LMON_BE_MSG_PREFIX, false,
               "Platform-independent proc control done"
               " visited with rmtype(%d) dontstop_faststop",
               rmtype, dontstop_fastpath);
#endif

  switch (rmtype) {
    case RC_slurm:
    case RC_orte:
    case RC_alps:
    case RC_cray:
    case RC_gupc:
    case RC_mpiexec_hydra:
    case RC_ibm_spectrum:
      //
      // You need to do nothing for these resource managers
      //
      rc = LMON_be_procctl_done_generic(ptab, psize);
      break;

    case RC_bglrm:
    case RC_bgprm:
      //
      // Call RM-specific init with BG CIOD debug interface
      //
      rc = LMON_be_procctl_done_bg(ptab, psize);
      break;

    case RC_bgqrm:
    case RC_bgq_slurm:
      //
      // Call RM-specific init with BGQ CDTI interface
      //
      rc =
          (dontstop_fastpath) ? LMON_OK : LMON_be_procctl_done_bgq(ptab, psize);
      break;

    case RC_mchecker_rm:
    case RC_none:
    default:
#if VERBOSE
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
                   "Unsupported RM Type (%d) for procctl_done", rmtype);
#endif
      break;
  }

  return rc;
}

lmon_rc_e LMON_be_procctl_perf(rm_catalogue_e rmtype, MPIR_PROCDESC_EXT *ptab,
                               int psize, long unsigned int membase,
                               unsigned int numbytes, unsigned int *fetchunit,
                               unsigned int *usecperunit) {
  lmon_rc_e rc = LMON_EINVAL;

#if VERBOSE
  LMON_say_msg(LMON_BE_MSG_PREFIX, false,
               "Platform-independent proc control perf"
               " visited with rmtype(%d)",
               rmtype);
#endif

  switch (rmtype) {
    case RC_slurm:
    case RC_orte:
    case RC_alps:
    case RC_cray:
    case RC_gupc:
    case RC_mpiexec_hydra:
    case RC_ibm_spectrum:
      //
      // You need to do nothing for these resource managers
      //
      rc = LMON_be_procctl_perf_generic(ptab, psize, membase, numbytes,
                                        fetchunit, usecperunit);
      break;

    case RC_bglrm:
    case RC_bgprm:
      //
      // Call RM-specific init with BG CIOD debug interface
      //
      rc = LMON_be_procctl_perf_bg(ptab, psize, membase, numbytes, fetchunit,
                                   usecperunit);
      break;

    case RC_bgqrm:
    case RC_bgq_slurm:
      //
      // Call RM-specific init with BGQ CDTI interface
      //
      rc = LMON_be_procctl_perf_bgq(ptab, psize, membase, numbytes, fetchunit,
                                    usecperunit);

      break;

    case RC_mchecker_rm:
    case RC_none:
    default:
#if VERBOSE
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
                   "Unsupported RM Type (%d) for procctl_perf", rmtype);
#endif
      break;
  }

  return rc;
}

/*
 * ts=2 sw=2 expandtab
 */
