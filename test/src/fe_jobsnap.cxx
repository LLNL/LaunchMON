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
 *        Jun 12 2008 DHA: Added GNU build system support.
 *        Feb 09 2008 DHA: Added LLNS Copyright.
 *        Aug 15 2006 DHA: Timer support
 *        Aug 06 2006 DHA: File created
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/common.h>
#include <lmon_api/lmon_fe.h>
#include <lmon_api/lmon_proctab.h>
#include <unistd.h>

#define MAXSTRING 128

#if MEASURE_TRACING_COST
extern "C" {
int begin_timer();
int time_stamp(const char* description);
}
#endif

int main(int argc, char* argv[]) {
  int aSession, spid;
  char** daemon_opts = NULL;
  lmon_rc_e rc;

  if (argc < 3) {
    fprintf(stderr, "Usage: fe_jshot srunpid be_jshot \n");

    return EXIT_FAILURE;
  }

  if (access(argv[2], X_OK) < 0) {
    fprintf(stderr, "[JOBSNAP] %s cannot be executed\n", argv[2]);

    return EXIT_FAILURE;
  }

  if (argc > 3) {
    daemon_opts = argv + 3;
  }

#if MEASURE_TRACING_COST
  begin_timer();
#endif

  if ((rc = LMON_fe_init(LMON_VERSION)) != LMON_OK) {
    fprintf(stderr, "[JOBSNAP] LMON_fe_init FAILED\n");

    return EXIT_FAILURE;
  }

  if ((rc = LMON_fe_createSession(&aSession)) != LMON_OK) {
    fprintf(stderr, "[JOBSNAP] LMON_fe_createFEBESession FAILED\n");

    return EXIT_FAILURE;
  }

  spid = atoi(argv[1]);
  if (spid < 0) {
    fprintf(stderr, "[JOBSNAP] invalid srun pid: %d \n", spid);

    return EXIT_FAILURE;
  }

  if ((rc = LMON_fe_attachAndSpawnDaemons(aSession, NULL, spid, argv[2],
                                          daemon_opts, NULL, NULL)) !=
      LMON_OK) {
    fprintf(stderr, "[JOBSNAP] LMON_fe_attachAndSpawnDaemons FAILED\n");

    return EXIT_FAILURE;
  }

#if MEASURE_TRACING_COST
  time_stamp("LMON_fe_init to LMON_fe_attachAndSpawnDaemons return");

  LMON_fe_recvUsrDataBe(aSession, NULL);

  time_stamp("LMON_fe_attachAndSpawnDaemons to jobsnap completion");
#endif

  if ((rc = LMON_fe_detach(aSession)) != LMON_OK) {
    fprintf(stderr, "[JOBSNAP] LMON_fe_detach FAILED\n");

    return EXIT_FAILURE;
  }

  if (getenv("JOBSNAP_TEST") != NULL) {
    fprintf(stdout, "[JOBSNAP] PASS\n");
  }

  return EXIT_SUCCESS;
}

/*
 * ts=2 sw=2 expandtab
 */
