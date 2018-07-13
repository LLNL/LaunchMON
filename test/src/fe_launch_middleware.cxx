/*
 * $Header: $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008-2010, Lawrence Livermore National Security, LLC. Produced
 *at
 * the Lawrence Livermore National Laboratory. Written by Dong H. Ahn
 *<ahn1@llnl.gov>.
 * LLNL-CODE-409469. All rights reserved.
 *
 * This file is part of LaunchMON. For details, see
 * https://computing.llnl.gov/?set=resources&page=os_projects
 *
 * Please also read LICENSE.txt -- Our Notice and GNU Lesser General Public
 *License.
 *
 *
 * This program is free software; you can redistribute it and/or modify it under
 *the
 * terms of the GNU General Public License (as published by the Free Software
 * Foundation) version 2.1 dated February 1999.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 *ANY
 * WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 *along
 * with this program; if not, write to the Free Software Foundation, Inc., 59
 *Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *--------------------------------------------------------------------------------
 *
 *
 *
 *  Update Log:
 *        Jun 01 2012 DHA: Copied from 0.8-middleware-support branch and merged
 *                         with 1.0-BGQ
 *        Aug 03 2020 DHA: Created file.
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <limits.h>
#include <lmon_api/common.h>
#include <unistd.h>
#include <string>
#include <vector>

#include <lmon_api/lmon_fe.h>
#include <lmon_api/lmon_mw.h>
#include <lmon_api/lmon_proctab.h>

#define MIN_NARGS 7

extern "C" {
int begin_timer();
int time_stamp(const char *description);
int statusFunc(int *status);
}

/*
 * OUR PARALLEL JOB LAUNCHER
 */
char mylauncher[PATH_MAX] = {0};

int main(int argc, char *argv[]) {
  using namespace std;

  int aSession = 0;
  unsigned int psize = 0;
  unsigned int proctabsize = 0;
  int jobidsize = 0;
  int i = 0;
  char jobid[PATH_MAX] = {0};
  char **launcher_argv = NULL;
  char **daemon_opts = NULL;
  char *application = NULL;
  char *nP = NULL;
  char *nN = NULL;
  char *part = NULL;
  char *mw_daemon = NULL;
  char *be_daemon = NULL;

  string numprocs_opt;
  string numnodes_opt;
  string partition_opt;

  lmon_rc_e rc = LMON_OK;

  if (argc < MIN_NARGS) {
    fprintf(stderr,
            "Usage: fe_launch_smoketest application numprocs numnodes "
            "partition mw_daemon be_daemon [be_daemonargs]\n");
    fprintf(stderr, "[LMON FE] FAILED\n");
    return EXIT_FAILURE;
  }

  application = argv[1];
  nP = argv[2];
  nN = argv[3];
  part = argv[4];
  mw_daemon = argv[5];
  be_daemon = argv[6];

  if (argc > MIN_NARGS) {
    daemon_opts = argv + MIN_NARGS;
  }

  if (access(argv[1], X_OK) < 0) {
    fprintf(stderr, "%s cannot be executed\n", argv[1]);
    fprintf(stderr, "[LMON FE] FAILED\n");
    return EXIT_FAILURE;
  }

  char *rmenv = getenv("MPI_JOB_LAUNCHER_PATH");
  if (!rmenv) {
    fprintf(stdout, "MPI_JOB_LAUNCHER_PATH envVar must be given\n");
    return EXIT_FAILURE;
  }

  snprintf(mylauncher, PATH_MAX, "%s", rmenv);

  rmenv = getenv("RM_TYPE");
  if (!rmenv) {
    fprintf(stdout, "RM_TYPE envVar must be given\n");
    return EXIT_FAILURE;
  }

  std::string rmenv_str = rmenv;

  if ((rmenv_str == std::string("RC_bgqrm"))) {
    launcher_argv = (char **)malloc(8 * sizeof(char *));
    launcher_argv[0] = strdup(mylauncher);
    launcher_argv[1] = strdup("--verbose");
    launcher_argv[2] = strdup("5");
    launcher_argv[3] = strdup("--np");
    launcher_argv[4] = strdup(argv[2]);
    launcher_argv[5] = strdup("--exe");
    launcher_argv[6] = strdup(argv[1]);
    // manually fill the block
    // launcher_argv[7] = strdup("--block");
    // launcher_argv[8] = strdup("R00-M0-N04");
    // manually fill the corner
    // launcher_argv[9] = strdup("--corner");
    // launcher_argv[10] = strdup("R00-M0-N04-J07");
    // manually fill the shape
    // launcher_argv[11] = strdup("--shape");
    // launcher_argv[12] = strdup("1x1x1x1x1");
    launcher_argv[7] = NULL;
    fprintf(stdout, "[LMON_FE] launching the job/daemons via %s\n", mylauncher);
  } else if ((rmenv_str == std::string("RC_bgq_slurm"))) {
    launcher_argv = (char **)malloc(7 * sizeof(char *));
    launcher_argv[0] = strdup(mylauncher);
    launcher_argv[1] = strdup("-N");
    launcher_argv[2] = strdup(argv[3]);
    launcher_argv[3] = strdup("-n");
    launcher_argv[4] = strdup(argv[2]);
    launcher_argv[5] = strdup(argv[1]);
    launcher_argv[6] = NULL;
    fprintf(stdout, "[LMON_FE] launching the job/daemons via %s\n",
            "mylauncher");
  } else if ((rmenv_str == std::string("RC_bglrm")) ||
             (rmenv_str == std::string("RC_bgprm"))) {
    launcher_argv = (char **)malloc(8 * sizeof(char *));
    launcher_argv[0] = strdup(mylauncher);
    launcher_argv[1] = strdup("-verbose");
    launcher_argv[2] = strdup("1");
    launcher_argv[3] = strdup("-np");
    launcher_argv[4] = strdup(argv[2]);
    launcher_argv[5] = strdup("-exe");
    launcher_argv[6] = strdup(argv[1]);
    launcher_argv[7] = NULL;
    fprintf(stdout, "[LMON_FE] launching the job/daemons via %s\n", mylauncher);
  } else if (rmenv_str == std::string("RC_slurm")) {
    numprocs_opt = string("-n") + string(argv[2]);
    numnodes_opt = string("-N") + string(argv[3]);
    partition_opt = string("-p") + string(argv[4]);
    launcher_argv = (char **)malloc(7 * sizeof(char *));
    launcher_argv[0] = strdup(mylauncher);
    launcher_argv[1] = strdup(numprocs_opt.c_str());
    launcher_argv[2] = strdup(numnodes_opt.c_str());
    launcher_argv[3] = strdup(partition_opt.c_str());
    launcher_argv[4] = strdup("-l");
    launcher_argv[5] = strdup(argv[1]);
    launcher_argv[6] = NULL;
  } else if (rmenv_str == std::string("RC_alps")) {
    numprocs_opt = string("-n") + string(argv[2]);
    launcher_argv = (char **)malloc(4 * sizeof(char *));
    launcher_argv[0] = strdup(mylauncher);
    launcher_argv[1] = strdup(numprocs_opt.c_str());
    launcher_argv[2] = strdup(argv[1]);
    launcher_argv[3] = NULL;
  } else if (rmenv_str == std::string("RC_orte")) {
    launcher_argv = (char **)malloc(8 * sizeof(char *));
    launcher_argv[0] = strdup(mylauncher);
    launcher_argv[1] = strdup("-mca");
    launcher_argv[2] = strdup("debugger");
    launcher_argv[3] = strdup("mpirx");
    launcher_argv[4] = strdup("-np");
    launcher_argv[5] = strdup(argv[2]);
    launcher_argv[6] = strdup(argv[1]);
    launcher_argv[7] = NULL;
    fprintf(stdout, "[LMON_FE] launching the job/daemons via %s\n", mylauncher);
  } else if (rmenv_str == std::string("RC_mpiexec_hydra")) {
    launcher_argv = (char **)malloc(5 * sizeof(char *));
    launcher_argv[0] = strdup(mylauncher);
    launcher_argv[1] = strdup("-n");
    launcher_argv[2] = strdup(argv[2]);
    launcher_argv[3] = strdup(argv[1]);
    launcher_argv[4] = NULL;
    fprintf(stdout, "[LMON_FE] launching the job/daemons via %s\n", mylauncher);
  }

  fprintf(stderr, "[LMON_FE] launching the job/daemons via %s\n", mylauncher);

  //
  // Init the front-end library
  //
  if ((rc = LMON_fe_init(LMON_VERSION)) != LMON_OK) {
    fprintf(stderr, "[LMON FE] FAILED\n");
    return EXIT_FAILURE;
  }

  //
  // Create a tool session
  //
  if ((rc = LMON_fe_createSession(&aSession)) != LMON_OK) {
    fprintf(stderr, "[LMON FE] FAILED\n");
    return EXIT_FAILURE;
  }

#if MEASURE_TRACING_COST
  begin_timer();
#endif

  //
  // Under that session, launch app tasks and
  // co-locate back ends with it
  //
  rc = LMON_fe_launchAndSpawnDaemons(aSession, NULL, launcher_argv[0],
                                     launcher_argv, be_daemon, daemon_opts,
                                     NULL, NULL);

  if (rc != LMON_OK) {
    fprintf(stderr, "[LMON FE] LMON_fe_launchAndSpawnDaemons FAILED\n");
    return EXIT_FAILURE;
  }

#if MEASURE_TRACING_COST
  //
  // LMON_fe_launchAndSpawnDaemons done
  //
  //
  time_stamp("LMON_fe_launchAndSpawnDaemons perf");
#endif

  dist_request_t *req = NULL;
  int nreq = 0;

  if (getenv("LMON_MW_HOSTLIST_TEST")) {
    nreq = 1;
    req = (dist_request_t *)malloc(sizeof(dist_request_t) * nreq);

    //
    // HOSTLIST Volume
    //
    char *hl = getenv("LMON_MW_HOSTLIST");
    if (!hl) {
      fprintf(stderr,
              "[LMON FE] FAILED: LMON_MW_HOSTLIST is needed for this test\n");
      return EXIT_FAILURE;
    }
    std::vector<std::string> hostvect;
    char delim[] = ":";
    char *hlcp = strdup(hl);
    char *token = strtok(hlcp, delim);
    while (token) {
      hostvect.push_back(token);
      token = strtok(NULL, delim);
    }
    req[0].md = LMON_MW_HOSTLIST;
    req[0].mw_daemon_path = strdup(mw_daemon);
    req[0].d_argv = (char **)malloc(3 * sizeof(char *));
    req[0].d_argv[0] = strdup("testarg1");
    req[0].d_argv[1] = strdup("testarg2");
    req[0].d_argv[2] = NULL;
    req[0].ndaemon = -1;
    req[0].block = -1;
    req[0].cyclic = -1;
    req[0].optkind = hostlists;
    req[0].option.hl = (char **)malloc(sizeof(char *) * (hostvect.size() + 1));
    std::vector<std::string>::const_iterator it;
    for (it = hostvect.begin(); it != hostvect.end(); ++it) {
      req[0].option.hl[i] = strdup((*it).c_str());
      i++;
    }
    req[0].option.hl[i] = NULL;
  } else if (getenv("LMON_MW_COLOC_TEST")) {
    nreq = 1;
    req = (dist_request_t *)malloc(sizeof(dist_request_t) * nreq);

    //
    // COLOC Volume
    //
    req[0].md = LMON_MW_COLOC;
    req[0].mw_daemon_path = strdup(mw_daemon);
    req[0].d_argv = (char **)malloc(3 * sizeof(char *));
    req[0].d_argv[0] = strdup("testarg1");
    req[0].d_argv[1] = strdup("testarg2");
    req[0].d_argv[2] = NULL;
    req[0].ndaemon = -1;
    req[0].block = -1;
    req[0].cyclic = -1;
    req[0].optkind = req_none;
  } else if (getenv("LMON_MW_MIX_TEST")) {
    nreq = 2;
    req = (dist_request_t *)malloc(sizeof(dist_request_t) * nreq);

    //
    // COLOC Volume
    //
    req[1].md = LMON_MW_COLOC;
    req[1].mw_daemon_path = strdup(mw_daemon);
    req[1].d_argv = NULL;
    req[1].ndaemon = -1;
    req[1].block = -1;
    req[1].cyclic = -1;
    req[1].optkind = req_none;

    //
    // HOSTLIST Volume
    //
    char *hl = getenv("LMON_MW_HOSTLIST");
    if (!hl) {
      fprintf(stderr,
              "[LMON FE] FAILED: LMON_MW_HOSTLIST is needed for this test\n");
      return EXIT_FAILURE;
    }
    std::vector<std::string> hostvect;
    char delim[] = ":";
    char *hlcp = strdup(hl);
    char *token = strtok(hlcp, delim);
    while (token) {
      hostvect.push_back(token);
      token = strtok(NULL, delim);
    }
    req[0].md = LMON_MW_HOSTLIST;
    req[0].mw_daemon_path = strdup(mw_daemon);
    req[0].d_argv = NULL;
    req[0].ndaemon = -1;
    req[0].block = -1;
    req[0].cyclic = -1;
    req[0].optkind = hostlists;
    req[0].option.hl = (char **)malloc(sizeof(char *) * (hostvect.size() + 1));
    std::vector<std::string>::const_iterator it;
    for (it = hostvect.begin(); it != hostvect.end(); ++it) {
      req[0].option.hl[i] = strdup((*it).c_str());
      i++;
    }
    req[0].option.hl[i] = NULL;

  } else {
    fprintf(stderr, "[LMON FE] envVar isn't given to select a test\n");
    return EXIT_FAILURE;
  }

  sleep(5);

#if MEASURE_TRACING_COST
  begin_timer();
#endif

  rc = LMON_fe_launchMwDaemons(aSession, req, nreq, NULL, NULL);

  free(req);

  if (rc != LMON_OK) {
    fprintf(stderr, "[LMON FE] LMON_fe_launchMwDaemons FAILED\n");
    return EXIT_FAILURE;
  }

#if MEASURE_TRACING_COST
  //
  // LMON_fe_launchMwDaemons done
  //
  //
  time_stamp("LMON_fe_launchMwDaemons perf");
#endif

  unsigned int mwsize = 0;
  LMON_fe_getMwHostlistSize(aSession, &mwsize);
  if (rc != LMON_OK) {
    fprintf(stderr, "[LMON FE] LMON_fe_getMwHostlistSize FAILED\n");
    return EXIT_FAILURE;
  }

  char **mwhostlist = (char **)malloc(mwsize * sizeof(char *));
  rc = LMON_fe_getMwHostlist(aSession, mwhostlist, &mwsize, mwsize);
  if (rc != LMON_OK) {
    fprintf(stderr, "[LMON FE] LMON_fe_getMwHostlist FAILED\n");
    return EXIT_FAILURE;
  }

  fprintf(stdout, "[LMON FE] **************************\n");
  for (i = 0; i < mwsize; i++) {
    fprintf(stdout, "[LMON FE] MW rank %5d: %s\n", i, mwhostlist[i]);
    free(mwhostlist[i]);
    mwhostlist[i] = NULL;
  }
  fprintf(stdout, "[LMON FE] **************************\n");

  free(mwhostlist);

  rc = LMON_fe_recvUsrDataMw(aSession, NULL);
  if ((rc == LMON_EBDARG) || (rc == LMON_ENOMEM) || (rc == LMON_EINVAL)) {
    fprintf(stderr, "[LMON FE] FAILED in LMON_fe_recvUsrDataMw\n");
    return EXIT_FAILURE;
  }

  //
  // Send an empty user msg saying MW daemons are up
  //
  rc = LMON_fe_sendUsrDataBe(aSession, NULL);
  if ((rc == LMON_EBDARG) || (rc == LMON_ENOMEM) || (rc == LMON_EINVAL)) {
    fprintf(stderr, "[LMON FE] FAILED in LMON_fe_sendUsrDataBe\n");
    return EXIT_FAILURE;
  }

  rc = LMON_fe_recvUsrDataBe(aSession, NULL);
  if ((rc == LMON_EBDARG) || (rc == LMON_ENOMEM) || (rc == LMON_EINVAL)) {
    fprintf(stdout, "[LMON FE] FAILED in LMON_fe_recvUsrDataBe\n");
    return EXIT_FAILURE;
  }

  fprintf(stderr, "\n[LMON FE] PASS: run through the end\n");

  sleep(1);

  return EXIT_SUCCESS;
}
