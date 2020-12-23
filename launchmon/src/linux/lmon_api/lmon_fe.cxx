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
 *        Aug 12 2020 DHA: lmon_reshandle_t is now typedef'ed to int64_t.
 *        Apr 01 2015 ADG: Added Cray CTI support.
 *        Apr 22 2014 DHA: Applied a patch to store RM args/opts into
 *                         a std::list object instead of a std:string string.
 *        Apr 15 2014 DHA: Integrate COBO secure handshake
 *        Jan 09 2013 DHA: Remove verbosity ref to the deprecated
 *                         thread tracer module.
 *        May 31 2012 DHA: Merged with the middleware support
 *                         from the 0.8-middleware-support branch.
 *        May 30 2012 DHA: (ID: 3530680) Added better debug info.
 *        Jan 17 2011 JDG: Bug fix in LMON_fe_regPackForFeToMw, variable typo.
 *        Jul 30 2010 DHA: Added LMON MW support with limited functionality
 *        Nov 01 2010 DHA: Fix small memory leaks in createSession
 *        Jun 30 2010 DHA: Added faster engine parsing error detection support
 *        Jun 28 2010 DHA: Added LMON_fe_getRMInfo support
 *        Apr 27 2010 DHA: Added MEASURE_TRACING_COST support.
 *        Feb 04 2010 DHA: Added LMON_FE_HOSTNAME_TO_CONN support
 *        Dec 23 2009 DHA: Added explict config.h inclusion
 *        Dec 16 2009 DHA: COBO support
 *        Dec 11 2009 DHA: BE hostname list generation in preparation for
 *                         scalable PMGR Collective bootstrapping
 *        Jun 01 2009 DHA: Changed LMON_fe_regStatusCB to be aware of
 *                         a target session.
 *        May 19 2009 DHA: Changed static int (*statusCB) (void *status)
 *                         to static int (*statusCB) (int *status):
 *                         the argument type for the status callback function
 *                         is now a pointer to an interger.
 *        May 19 2009 DHA: Added LMON_fe_regErrorCB ( int (*errorCB)(char *msg)
 *)
 *                         support.
 *        Mar 13 2009 DHA: Added large nTasks support
 *        Mar 11 2009 DHA: Bug fix in the pthread_cond_timedwait
 *                         return code checking logic for launchmon engine
 *                         connection.
 *                         Added ENVCMD, SSHCMD, and TVCMD.
 *                         Added comm_pair_e for readability.
 *                         which the new configure check would detect.
 *        Mar 04 2009 DHA: Added BlueGene/P support.
 *                         In particular, changed RM_BGL_MPIRUN to RM_BG_MPIRUN
 *                         to genericize BlueGene Support
 *        Sep 24 2008 DHA: Enforced the error handling semantics
 *                         defined in README.ERROR_HANDLING.
 *        Mar 17 2008 DHA: Added PMGR Collective support
 *        Mar 06 2008 DHA: Added timeout for daemon control
 *                         interfaces (detach, kill, and shutdown)
 *        Mar 05 2008 DHA: Added more engine timeout support (fe-engine, fe-be);
 *                         added a timedaccept call into lmon_lmonp_msg.cxx
 *                         and added the O_NONBLOCK flag to part of listening
 *                         sockets to prevent any indefinte hang.
 *        Mar 05 2008 DHA: Added better verbosity support for remote use
 *                         of launchmon engine
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Dec 07 2007 DHA: Added support for secure connection between
 *                         frontend and backend daemons.
 *                         The frontend first sends the shared key
 *                         random session ID, unencrypted, to the
 *                         daemons via the environment variables.
 *                         (This is secure since we rely on the RM
 *                         to do this job.)
 *                         Then, the master daemons encrypts the
 *                         random session ID using the shared key
 *                         and send it back to the front-end.
 *                         The received session ID gets decrypted and
 *                         compared with the original session ID.
 *                         I'm currently using libgcrypt's blowfish
 *                         encryption algorithm.
 *                         The only way this can be hijacked is
 *                         when the target RM does not provide
 *                         a secure mechanism in propagating
 *                         the environment variables to the daemons.
 *        Dec 04 2007 DHA: More rigorous sessionHandle validity check
 *        Sep 24 2007 DHA: Change FEBESessionHandle to sessionHandle
 *                         in anticipation for adding middleware API support
 *                         Along the same line, fe_shutdownBe changed to
 *                         fe_shutdownDaemons.
 *                         Expanding LMON_fe_sendUsrData into
 *                         LMON_fe_sendUsrDataBe and LMON_fe_sendUsrDataMw
 *                         vice versa for LMON_fe_recvUsrDataBe and
 *                         LMON_fe_recvUsrDataMw
 *        Aug 13 2007 DHA: LMON_fe_sendUsrData added
 *        Aug 10 2007 DHA: LMON_fe_detach added
 *        Jul 25 2007 DHA: Change the lmonp protocol to better handle the
 *                         flowing of proctab info
 *        Mar 09 2007 DHA: Low-quality implementation to get the correct
 *                         behavior for FE to BE proctab packing. Now it uses
 *                         hostname reference array FE gets from the master BE
 *                         daemon.
 *                         This portion of the code needs to be optimized.
 *        Dec 29 2006 DHA: 1. LMON_freeDaemonEnvList function prototype now
 *                         takes lmon_daemon_env_t** as function argument
 *                         data type.
 *                         It used to be lmon_daemon_env_t*. Along with function
 *                         definition changes, it allows NULL assignment to the
 *                         passed argument effective on function exit. Bug fix
 *                         on a segfault violation from
 *                         test.basic_multisession_launch.tst test case.
 *                         2.  LMON_destroy_sess calls LMON_init_sess at the end
 *                         so that cleaned session descriptor can be reused. Bug
 *                         fix for a segfault violation from
 *                         test.stress_sess_descriptor.sh test case.
 *        Dec 27 2006 DHA: Polishing up the data types
 *        Dec 22 2006 DHA: proctab communication sequence done
 *        Dec 15 2006 DHA: Created file.
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif
#include <lmon_api/lmon_api_std.h>

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
#endif

#include <arpa/inet.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <vector>
#include "gcrypt.h"

#include "sdbg_base_spawner.hxx"
#include "sdbg_opt.hxx"
#include "sdbg_rm_map.hxx"
#include "sdbg_self_trace.hxx"
//
// spawners to support Middleware
//
#include "sdbg_rsh_spawner.hxx"
//#include "sdbg_rm_spawner.hxx"
#include "lmon_api/lmon_coloc_spawner.hxx"
#include "lmon_api/lmon_say_msg.hxx"

#include "lmon_api/lmon_fe.h"
#include "lmon_api/lmon_lmonp_msg.h"
#include "lmon_api/lmon_proctab.h"

#if COBO_BASED
extern "C" {
#include <cobo.h>
#include <handshake.h>
}
#endif

#ifdef DEBUG_SESSION_DESC
const int MAX_LMON_SESSION = 3;
#else  /* DEBUG_SESSION_DESC */
//
// The default number of session descriptors is 100
//
const int MAX_LMON_SESSION = 100;
#endif /* DEBUG_SESSION_DESC */

const int MAX_LMON_STRING = 4096;
const int LMON_INIT = -1;
const int DFLT_FE_ENGINE_TOUT = 120;
const int DFLT_FE_BE_TOUT = 120;
const int DFLT_FE_MW_TOUT = 120;
const int MAX_TIMEOUT = 6000;
const char *LMON_FE_MSG_PREFIX = "<LMON FE API>";

//////////////////////////////////////////////////////////////////////////////////
//
// STATIC DATA TYPES
//
//
//

typedef enum _mboolean_e { LMON_TRUE, LMON_FALSE } boolean_e;

//
// Don't change the integer value for the
// following enumerator.
//
typedef enum _comm_pair_e {
  fe_be_conn = 0,
  fe_mw_conn = 1,
  fe_engine_conn = 2
} comm_pair_e;
const int conn_size = 3;

typedef int64_t lmon_reshandle_t;

class lexGraphCmp {
 public:
  bool operator()(const std::string &s1, const std::string &s2) {
    return (s1 < s2);
  }
};

//! lmon_session_comm_desc_t
/*!
  This data type describes per-session resources
  - servAddr:
            The TCP server socket FE listens in on
  - ipInfo:
            The string info on the server socket
  - sessionListenSockFd:
            The actual file descriptor for the server socket
  - sessionAcceptSockFd:
            The file descriptor for "accept"

*/
typedef struct _lmon_session_comm_desc_t {
  /*
   * listening server address
   */
  struct sockaddr_in servAddr;

  /*
   * additional info for the server socket
   */
  char ipInfo[MAX_LMON_STRING];

  /*
   * listening server socket descriptor
   */
  int sessionListenSockFd;

  /*
   * accept socket descriptor
   */
  int sessionAcceptSockFd;

  /*
   * # of daemons
   */
  int nDaemons;

  /*
   * file descriptors that aid in setting ICCL
   */
  int ICCL_assist_fds[LMON_MAX_NDAEMONS];

} lmon_session_comm_desc_t;

//! lmon_thr_desc_t
/*!
    This data type describes per-thread resources

    FE API spawns a launchmon engine process and a pthread that
    handles communcation with the launchmon process. This
    data type describes the resources needed for each
    of those threads.

*/
typedef struct _lmon_thr_desc_t {
  /*
   * thread id that monitors a forked launchmon
   */
  pthread_t fetofeMonitoringThr;
  pthread_cond_t condVar;
  pthread_mutex_t eventMutex;

} lmon_thr_desc_t;

//! lmon_session_desc_t
/*!
  huge data type to describe per-session resources

*/
typedef struct _lmon_session_desc_t {
  /*
   * boolean to indicate this descriptor slot is taken
   */
  boolean_e registered;

  /*
   * boolean to indicate tool backend daemons have spawned
   */
  boolean_e spawned;

  /*
   * boolean to indicate middleware daemons have spawned
   */
  boolean_e mw_spawned;

  /*
   * indicator whether the launchmon engine detached from
   * the target or not
   */
  boolean_e detached;

  /*
   * indicator whether the launchmon engine killed
   * the target or not
   */
  boolean_e killed;

  /*
   * the pid of the launchmon engine process
   */
  pid_t le_pid;

  /*
   * the pid of the job launcher process
   */
  pid_t rm_launcher_pid;

  /*
   * the RM type that the launchmon engine is handling
   */
  lmon_rm_info_t rm_info;

  /*
   * cipher handler
   */
  gcry_cipher_hd_t cipher_hndl;

  /*
   * the 128 bit shared key
   */
  char shared_key[LMON_KEY_LENGTH];

  /*
   * random ID
   */
  int32_t randomID;

  /*
   * user pack function for a message from BE
   */
  int (*pack)(void *, void *, int, int *);

  /*
   * user unpack function for a message from BE
   */
  int (*unpack)(void *, int, void *);

  /*
   * user pack function for a message to MW
   */
  int (*mw_pack)(void *, void *, int, int *);

  /*
   * user unpack function for a message from MW
   */
  int (*mw_unpack)(void *, int, void *);

  /*
   * communication descriptors
   *
   * commDesc[fe_be_conn]: FE-BE communication
   * commDesc[fe_mw_conn]: FE-MW communication
   * commDesc[fe_engine_conn]: FE-ENGINE communication
   */
  lmon_session_comm_desc_t commDesc[conn_size];

  /*
   * watchdog thread descriptors
   */
  lmon_thr_desc_t watchdogThr;

  /*
   * resource handle such as "totalview_jobid": type is int for now
   */
  lmon_reshandle_t resourceHandle;

  /*
   * envVar list to write into remote systems
   *
   * daemonEnvList[0]: for backends
   * daemonEnvList[1]: for middleware
   */
  lmon_daemon_env_t *daemonEnvList[2];

  /*
   * the raw RPDTAB msg received from launchmon engine
   */
  lmonp_t *proctab_msg;

  /*
   * the raw hostname table msg received from the master BE daemon
   */
  lmonp_t *hntab_msg;

  /*
   * the raw hostname table msg received from the master MW daemon
   */
  lmonp_t *hntab_mw_msg;

  /*
   * the host list constructed from proctab_msg
   */
  std::map<std::string, std::vector<MPIR_PROCDESC_EXT *>, lexGraphCmp> pMap;

  /*
   * the vector of spawner objects
   */
  std::vector<spawner_base_t *> spawner_vector;

  /*
   * status check callback for this function
   */
  int (*statusCB)(int *status);

} lmon_session_desc_t;

//! lmon_session_array_t
/*!
    Data type to hold an array of elements of lmon_session_desc_t type.
    MAX_LMON_SESSION should be reasonably big; otherwise, (an) element(s)
    describing live sessions could be overwritten: this is a circular
    array.
*/
typedef struct _lmon_session_array_t {
  /*
   * session descriptor array, circular array
   */
  lmon_session_desc_t sessionDescArray[MAX_LMON_SESSION];

  /*
   * always pointing to the next available desc element
   */
  int sessionPtrIndex;

} lmon_session_array_t;

//////////////////////////////////////////////////////////////////////////////////
//
// STATIC DATA AREA
//
//
//
static lmon_session_array_t sess;
static rc_rm_t resmanager;

//////////////////////////////////////////////////////////////////////////////////
//
// STATIC FUNCTIONS
//
//
//

//! LMON_handle_signals (int sig)
/*!
  signal handler
*/
static void LMON_handle_signals(int sig) {
  int i;
  int numbytes;
  lmonp_t msg;
  lmon_session_desc_t *mydesc;

  init_msg_header(&msg);
  msg.msgclass = lmonp_fetofe;
  msg.type.fetofe_type = lmonp_shutdownbe;

  //
  // it is an exception to the share memory access policy
  //
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  for (i = 0; i < MAX_LMON_SESSION; ++i) {
    mydesc = &sess.sessionDescArray[i];

    if (mydesc->registered != LMON_TRUE) continue;

    if (mydesc->commDesc[fe_engine_conn].sessionAcceptSockFd != LMON_INIT) {
      //
      // assuming this isn't a watchdog thread, a watchdog thread should
      // get the ack msg.
      //
      numbytes = write_lmonp_long_msg(
          mydesc->commDesc[fe_engine_conn].sessionAcceptSockFd, &msg,
          sizeof(msg));
    }
  }
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));

  abort();
}

//! LMON_return_ver ()
/*!
  returns version info
*/
static int LMON_return_ver() { return LMON_VERSION; }

//! LMON_freeDaemonEnvList ( lmon_daemon_env_t** n )
/*!
  A helper routine to free the memory allocated for
  an element of lmon_daemon_env_t type.
*/
static void LMON_freeDaemonEnvList(lmon_daemon_env_t **n) {
  if ((*n) == NULL) return;

  LMON_freeDaemonEnvList(&((*n)->next));

  free(*n);
  *n = NULL;

  return;
}

static lmon_rc_e LMON_fe_putToDaemonEnv(lmon_daemon_env_t **sessEnv,
                                        lmon_daemon_env_t *dmonEnv,
                                        int numElem) {
  int i;
  lmon_rc_e rc = LMON_OK;

  if ((dmonEnv == NULL) || (numElem <= 0)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "no daemon environment variable is passed in!");

    return LMON_EINVAL;
  }

  i = 0;
  if (*sessEnv == NULL) {
    lmon_daemon_env_t *anode =
        (lmon_daemon_env_t *)malloc(sizeof(lmon_daemon_env_t));

    if (anode == NULL) return LMON_ENOMEM;

    anode->envName = strdup(dmonEnv[i].envName);
    anode->envValue = strdup(dmonEnv[i].envValue);
    anode->next = NULL;
    (*sessEnv) = anode;
    i++;
  }

  lmon_daemon_env_t *trav = (*sessEnv);
  while (trav->next != NULL) trav = trav->next;

  while (i < numElem) {
    lmon_daemon_env_t *anode =
        (lmon_daemon_env_t *)malloc(sizeof(lmon_daemon_env_t));

    if (anode == NULL) return LMON_ENOMEM;

    anode->envName = strdup(dmonEnv[i].envName);
    anode->envValue = strdup(dmonEnv[i].envValue);
    anode->next = NULL;
    trav->next = anode;
    trav = trav->next;
    i++;
  }

  return rc;
}

//! LMON_init_sess ( lmon_session_desc_t* s )
/*!
  Initialize a session
  return 0 on success; -1 on failure
*/
static int LMON_init_sess(lmon_session_desc_t *s) {
  if (s == NULL) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session descriptor is null!");

    return -1;
  }

  s->pack = NULL;
  s->unpack = NULL;
  s->mw_pack = NULL;
  s->mw_unpack = NULL;

  s->daemonEnvList[0] = NULL;
  s->daemonEnvList[1] = NULL;

  s->registered = LMON_FALSE;
  s->spawned = LMON_FALSE;
  s->mw_spawned = LMON_FALSE;
  s->detached = LMON_FALSE;
  s->killed = LMON_FALSE;
  s->le_pid = LMON_FALSE;
  s->rm_launcher_pid = LMON_FALSE;

  std::vector<resource_manager_t> tmpinfo;
  tmpinfo = resmanager.get_supported_rms();
  if (!tmpinfo.empty()) {
    s->rm_info.rm_supported_types =
        (rm_catalogue_e *)malloc(sizeof(rm_catalogue_e) * tmpinfo.size());
    s->rm_info.num_supported_types = tmpinfo.size();

    int ix;
    for (ix = 0; ix < tmpinfo.size(); ++ix) {
      s->rm_info.rm_supported_types[ix] = tmpinfo[ix].get_rm();
    }
    s->rm_info.index_to_cur_instance = -1;
    s->rm_info.rm_launcher_pid = -1;
  } else {
    s->rm_info.rm_supported_types = NULL;
    s->rm_info.num_supported_types = 0;
    s->rm_info.index_to_cur_instance = -1;
    s->rm_info.rm_launcher_pid = -1;
  }

  bzero(s->shared_key, LMON_KEY_LENGTH);
  s->randomID = LMON_FALSE;

  //
  // FE-BE comm
  //
  bzero(&(s->commDesc[fe_be_conn].servAddr),
        sizeof(s->commDesc[fe_be_conn].servAddr));
  bzero(s->commDesc[fe_be_conn].ipInfo, sizeof(MAX_LMON_STRING));
  s->commDesc[fe_be_conn].sessionListenSockFd = LMON_INIT;
  s->commDesc[fe_be_conn].sessionAcceptSockFd = LMON_INIT;

  //
  // FE-MW comm
  //
  bzero(&(s->commDesc[fe_mw_conn].servAddr),
        sizeof(s->commDesc[fe_mw_conn].servAddr));
  bzero(s->commDesc[fe_mw_conn].ipInfo, sizeof(MAX_LMON_STRING));
  s->commDesc[fe_mw_conn].sessionListenSockFd = LMON_INIT;
  s->commDesc[fe_mw_conn].sessionAcceptSockFd = LMON_INIT;

  //
  // FE-Engine comm
  //
  bzero(&(s->commDesc[fe_engine_conn].servAddr),
        sizeof(s->commDesc[fe_engine_conn].servAddr));
  bzero(s->commDesc[fe_engine_conn].ipInfo, sizeof(MAX_LMON_STRING));
  s->commDesc[fe_engine_conn].sessionListenSockFd = LMON_INIT;
  s->commDesc[fe_engine_conn].sessionAcceptSockFd = LMON_INIT;

  s->proctab_msg = NULL;
  s->hntab_msg = NULL;

  // make_sure: s->pMap.size == 0
  s->resourceHandle = LMON_INIT;

  // make_sure: s->spawner_vector.empty()
  s->statusCB = NULL;

  return 0;
}

//! LMON_destroy_sess ( lmon_session_desc_t* s )
/*!
    destroys a session
    return 0 on success; -1 on failure
*/
static int LMON_destroy_sess(lmon_session_desc_t *s) {
  int rc = 0;

  s->pack = NULL;
  s->unpack = NULL;
  s->mw_pack = NULL;
  s->mw_unpack = NULL;

  LMON_freeDaemonEnvList(&(s->daemonEnvList[0]));
  LMON_freeDaemonEnvList(&(s->daemonEnvList[1]));

  s->registered = LMON_FALSE;
  s->spawned = LMON_FALSE;
  s->mw_spawned = LMON_FALSE;
  s->detached = LMON_FALSE;
  s->killed = LMON_FALSE;
  s->le_pid = LMON_FALSE;
  s->rm_launcher_pid = LMON_FALSE;

  if (s->rm_info.rm_supported_types) {
    free(s->rm_info.rm_supported_types);
    s->rm_info.rm_supported_types = NULL;
  }
  s->rm_info.num_supported_types = LMON_FALSE;
  s->rm_info.index_to_cur_instance = LMON_FALSE;
  s->rm_info.rm_launcher_pid = LMON_FALSE;

  bzero(s->shared_key, LMON_KEY_LENGTH);
  s->randomID = LMON_FALSE;

  gcry_cipher_close(s->cipher_hndl);

  bzero(&(s->commDesc[fe_be_conn].servAddr),
        sizeof(s->commDesc[fe_be_conn].servAddr));
  bzero(s->commDesc[fe_be_conn].ipInfo, sizeof(MAX_LMON_STRING));
  close(s->commDesc[fe_be_conn].sessionListenSockFd);
  close(s->commDesc[fe_be_conn].sessionAcceptSockFd);

  bzero(&(s->commDesc[fe_mw_conn].servAddr),
        sizeof(s->commDesc[fe_mw_conn].servAddr));
  bzero(s->commDesc[fe_mw_conn].ipInfo, sizeof(MAX_LMON_STRING));
  close(s->commDesc[fe_mw_conn].sessionListenSockFd);
  close(s->commDesc[fe_mw_conn].sessionAcceptSockFd);

  bzero(&(s->commDesc[fe_engine_conn].servAddr),
        sizeof(s->commDesc[fe_engine_conn].servAddr));
  bzero(s->commDesc[fe_engine_conn].ipInfo, sizeof(MAX_LMON_STRING));
  close(s->commDesc[fe_engine_conn].sessionListenSockFd);
  close(s->commDesc[fe_engine_conn].sessionAcceptSockFd);

  if (s->proctab_msg) free(s->proctab_msg);
  s->proctab_msg = NULL;
  if (s->hntab_msg) free(s->hntab_msg);
  s->hntab_msg = NULL;
  if (s->hntab_mw_msg) free(s->hntab_mw_msg);

  s->resourceHandle = LMON_INIT;

  if (s->pMap.size() != 0) {
    //
    // TODO: ugly, someday, change pointers to smart pointers
    // that mix well with STL
    //
    std::map<std::string, std::vector<MPIR_PROCDESC_EXT *>,
             lexGraphCmp>::iterator iter;
    std::vector<MPIR_PROCDESC_EXT *>::iterator vectiter;
    for (iter = s->pMap.begin(); iter != s->pMap.end(); ++iter) {
      for (vectiter = iter->second.begin(); vectiter != iter->second.end();
           ++vectiter) {
        free((*vectiter)->pd.executable_name);
        free((*vectiter)->pd.host_name);
        free((*vectiter));
      }
    }
    s->pMap.clear();
  }

  if (!s->spawner_vector.empty()) {
    std::vector<spawner_base_t *>::iterator iter;
    for (iter = s->spawner_vector.begin(); iter != s->spawner_vector.end();
         ++iter) {
      delete (*iter);
    }
    s->spawner_vector.clear();
  }

  s->statusCB = NULL;

  return rc;
}

static lmon_rc_e LMON_fe_handleFeBeUsrData(int sessionHandle, void *febe_data) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e lrc = LMON_EINVAL;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");

    return LMON_EBDARG;
  }

  mydesc = &sess.sessionDescArray[sessionHandle];

  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job has been killed?");
    return LMON_EBDARG;
  }

  if (febe_data == NULL) {
    lmonp_t empty_udata_msg;

    set_msg_header(&empty_udata_msg, lmonp_fetobe, lmonp_febe_usrdata, 0, 0, 0,
                   0, 0, 0, 0);

    write_lmonp_long_msg(mydesc->commDesc[fe_be_conn].sessionAcceptSockFd,
                         &empty_udata_msg, sizeof(empty_udata_msg));

    lrc = LMON_ENOPLD;
  } else {
    if (mydesc->pack == NULL) {
      lmonp_t empty_udata_msg;

      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "did you register a FEBE pack function?");

      set_msg_header(&empty_udata_msg, lmonp_fetobe, lmonp_febe_usrdata, 0, 0,
                     0, 0, 0, 0, 0);
      //
      // sending a short message
      //
      write_lmonp_long_msg(mydesc->commDesc[fe_be_conn].sessionAcceptSockFd,
                           &empty_udata_msg, sizeof(empty_udata_msg));

      lrc = LMON_ENCLLB;
    } else {
      lmonp_t *udata_msg;
      char *udata;
      int outlen;

      udata_msg = (lmonp_t *)malloc(sizeof(*udata_msg) + LMON_MAX_USRPAYLOAD);
      if (udata_msg == NULL) return LMON_ENOMEM;

      set_msg_header(udata_msg, lmonp_fetobe, lmonp_febe_usrdata, 0, 0, 0, 0, 0,
                     0, LMON_MAX_USRPAYLOAD);

      udata = get_usrpayload_begin(udata_msg);

      //
      // pack func must serialize febe_data into udata
      // serialized stream cannot be bigger than febe_data_len
      //
      if (mydesc->pack(febe_data, udata, LMON_MAX_USRPAYLOAD, &outlen) < 0) {
        return LMON_EINVAL;
      }

      if (outlen > LMON_MAX_USRPAYLOAD) return LMON_EINVAL;

      udata_msg->usr_payload_length = outlen;
      write_lmonp_long_msg(mydesc->commDesc[fe_be_conn].sessionAcceptSockFd,
                           udata_msg,
                           sizeof(lmonp_t) + udata_msg->usr_payload_length);

      lrc = LMON_OK;

      free(udata_msg);
    }
  }

  return lrc;
}

static lmon_rc_e LMON_fe_handleFeMwUsrData(int sessionHandle, void *femw_data) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e lrc = LMON_EINVAL;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid for LMON_fe_handleFeMwUsrData");

    return LMON_EBDARG;
  }

  mydesc = &sess.sessionDescArray[sessionHandle];

  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid for LMON_fe_handleFeMwUsrData,"
                 " the job has been killed?");

    return LMON_EBDARG;
  }

  if (femw_data == NULL) {
    lmonp_t empty_udata_msg;

    set_msg_header(&empty_udata_msg, lmonp_fetomw, lmonp_femw_usrdata, 0, 0, 0,
                   0, 0, 0, 0);

    write_lmonp_long_msg(mydesc->commDesc[fe_mw_conn].sessionAcceptSockFd,
                         &empty_udata_msg, sizeof(empty_udata_msg));

    lrc = LMON_ENOPLD;
  } else {
    if (mydesc->mw_pack == NULL) {
      lmonp_t empty_udata_msg;

      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "did you register a FEMW pack function?");

      set_msg_header(&empty_udata_msg, lmonp_fetomw, lmonp_femw_usrdata, 0, 0,
                     0, 0, 0, 0, 0);

      //
      // sending a short message
      //
      write_lmonp_long_msg(mydesc->commDesc[fe_mw_conn].sessionAcceptSockFd,
                           &empty_udata_msg, sizeof(empty_udata_msg));

      lrc = LMON_ENCLLB;
    } else {
      lmonp_t *udata_msg;
      char *udata;
      int outlen;

      udata_msg = (lmonp_t *)malloc(sizeof(*udata_msg) + LMON_MAX_USRPAYLOAD);

      if (udata_msg == NULL) return LMON_ENOMEM;

      set_msg_header(udata_msg, lmonp_fetomw, lmonp_femw_usrdata, 0, 0, 0, 0, 0,
                     0, LMON_MAX_USRPAYLOAD);

      udata = get_usrpayload_begin(udata_msg);

      //
      // pack func must serialize febe_data into udata
      // serialized stream cannot be bigger than febe_data_len
      //
      mydesc->mw_pack(femw_data, udata, LMON_MAX_USRPAYLOAD, &outlen);

      if (outlen > LMON_MAX_USRPAYLOAD) return LMON_EINVAL;

      udata_msg->usr_payload_length = outlen;
      write_lmonp_long_msg(mydesc->commDesc[fe_mw_conn].sessionAcceptSockFd,
                           udata_msg,
                           sizeof(lmonp_t) + udata_msg->usr_payload_length);

      lrc = LMON_OK;
      free(udata_msg);
    }
  }

  return lrc;
}

static lmon_rc_e LMON_fe_handleBeFeUsrData(int sessionHandle, void *befe_data) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e lrc = LMON_EINVAL;
  lmonp_t msg;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, true,
        "session is invalid for handling backend-frontend user payload");

    return LMON_EBDARG;
  }

  mydesc = &sess.sessionDescArray[sessionHandle];

  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, true,
        "session is invalid for handling backend-frontend user payload,"
        " the job has been killed?");
    return LMON_EBDARG;
  }

  read_lmonp_msgheader(mydesc->commDesc[fe_be_conn].sessionAcceptSockFd, &msg);

  if ((msg.msgclass != lmonp_fetobe) ||
      !((msg.type.fetobe_type == lmonp_befe_ready) ||
        (msg.type.fetobe_type == lmonp_befe_usrdata)) ||
      (msg.lmon_payload_length != 0)) {
    return LMON_EBDMSG;
  }

  if (msg.usr_payload_length > 0) {
    //
    // BE shipped user data via the ready msg
    //
    char *usrdatabuf = NULL;

    usrdatabuf = (char *)malloc(msg.usr_payload_length);

    if (usrdatabuf == NULL) return LMON_ENOMEM;

    read_lmonp_payloads(mydesc->commDesc[fe_be_conn].sessionAcceptSockFd,
                        usrdatabuf, msg.usr_payload_length);

    if ((mydesc->unpack == NULL) || (befe_data == NULL)) {
      //
      // if FE fails to have registered unpack func,
      // do nothiing
      //
      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "did you register a BEFE unpack function?");
      LMON_say_msg(LMON_FE_MSG_PREFIX, true, "or pass a null pointer?");

      lrc = LMON_ENCLLB;
    } else {
      //
      // we invoke the unpack callback, which will
      // unpack usrpayload to befe_data.
      //
      if ((mydesc->unpack(usrdatabuf, msg.usr_payload_length, befe_data)) < 0) {
        LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                     "the unpack function you had registered returned a "
                     "negative return code");

        lrc = LMON_ENEGCB;
      } else {
        lrc = LMON_OK;
      }
    }
  } else {
    lrc = LMON_ENOPLD;
  }

  return lrc;
}

static lmon_rc_e LMON_fe_handleMwFeUsrData(int sessionHandle, void *mwfe_data) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e lrc = LMON_EINVAL;
  lmonp_t msg;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, true,
        "session is invalid in handling middleware-frontent user payload");

    return LMON_EBDARG;
  }

  mydesc = &sess.sessionDescArray[sessionHandle];

  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, true,
        "session is invalid in handling middleware-frontent user payload,"
        " the job has been killed?");

    return LMON_EBDARG;
  }

  read_lmonp_msgheader(mydesc->commDesc[fe_mw_conn].sessionAcceptSockFd, &msg);

  if ((msg.msgclass != lmonp_fetomw) ||
      !((msg.type.fetomw_type == lmonp_mwfe_ready) ||
        (msg.type.fetomw_type == lmonp_mwfe_usrdata)) ||
      (msg.lmon_payload_length != 0)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "invalid LMONP Msg received in LMON_fe_handleMwFeUsrData");
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "  A msg of {Class(%s),"
                 "Type(%s),"
                 "LMON_payload_size(%s),"
                 "USR_payload_size(%s)} has been received.",
                 lmon_msg_to_str(field_class, &msg),
                 lmon_msg_to_str(field_type, &msg),
                 lmon_msg_to_str(field_lmon_payload_length, &msg),
                 lmon_msg_to_str(field_usr_payload_length, &msg));

    return LMON_EBDMSG;
  }

  if (msg.usr_payload_length > 0) {
    //
    // BE shipped user data via the ready msg
    //
    char *usrdatabuf = NULL;

    usrdatabuf = (char *)malloc(msg.usr_payload_length);
    if (usrdatabuf == NULL) return LMON_ENOMEM;

    read_lmonp_payloads(mydesc->commDesc[fe_mw_conn].sessionAcceptSockFd,
                        usrdatabuf, msg.usr_payload_length);

    if ((mydesc->mw_unpack == NULL) || (mwfe_data == NULL)) {
      //
      // if FE fails to have registered unpack func,
      // do nothiing
      //
      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "did you register a BEMW unpack function?");
      LMON_say_msg(LMON_FE_MSG_PREFIX, true, "or pass a null pointer?");

      lrc = LMON_ENCLLB;
    } else {
      //
      // we invoke the unpack callback, which will
      // unpack usrpayload to befe_data.
      //
      if ((mydesc->mw_unpack(usrdatabuf, msg.usr_payload_length, mwfe_data)) <
          0) {
        LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                     "the unpack function that you registered returned a "
                     "negative return code");

        lrc = LMON_ECLLB;
      } else {
        lrc = LMON_OK;
      }
    }
  } else {
    lrc = LMON_ENOPLD;
  }

  return lrc;
}

static lmon_rc_e LMON_fe_acceptEngine(int sessionHandle) {
  //
  // Mar 05 2008 DHA:
  //
  // We need to perform non-blocking accept in order to
  // be able to return to the tool client when a connection
  // error occurs (LMON_ETOUT), hence, "select" followed by
  // accept. Also, the upper layer must set O_NONBLOCK attribute
  // for the listening socket.
  //
  struct sockaddr_in clientaddr;
  socklen_t clientaddr_len;
  lmon_session_desc_t *mydesc = NULL;
  lmonp_t msg;
  char *tout = NULL;
  int listenfd = -1;
  int tosec = 0;

  clientaddr_len = sizeof(clientaddr);

  mydesc = &(sess.sessionDescArray[sessionHandle]);

  listenfd = mydesc->commDesc[fe_engine_conn].sessionListenSockFd;

  tout = getenv("LMON_FE_ENGINE_TIMEOUT");
  if (tout && ((atoi(tout) > 0) && (atoi(tout) <= MAX_TIMEOUT)))
    tosec = atoi(tout);
  else
    tosec = DFLT_FE_ENGINE_TOUT;

  mydesc->commDesc[fe_engine_conn].sessionAcceptSockFd = lmon_timedaccept(
      listenfd, (struct sockaddr *)&clientaddr, &clientaddr_len, tosec);

  if (mydesc->commDesc[fe_engine_conn].sessionAcceptSockFd == -2) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "connection to the launchmon engine timed out");
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, true,
        "the launchmon engine has been crashed or never been invoked?");

    return LMON_ETOUT;
  } else if (mydesc->commDesc[fe_engine_conn].sessionAcceptSockFd < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "connection to the launchmon engine failed");
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, true,
        "the launchmon engine has been crashed or never been invoked?");

    return LMON_ESYS;
  }

  if (read_lmonp_msgheader(mydesc->commDesc[fe_engine_conn].sessionAcceptSockFd,
                           &msg) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "reading an LMON msg from the launchmon engine returns a "
                 "negative return code");
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "LMONP protocol mismatch or problems while invoking the "
                 "launchmon engine?");

    return LMON_ESYS;
  }

  if (msg.type.fetofe_type == lmonp_conn_ack_parse_error) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "the launchmon engine encountered an error while parsing its "
                 "command line.");
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "for example, has an incorrect pid been provided?");
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "please check the command line provided to the engine.");

    return LMON_EINVAL;
  } else if (msg.type.fetofe_type != lmonp_conn_ack_no_error) {
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, true,
        "the engine sent an unexpected msg type. LMONP version mismatch?");

    return LMON_EINVAL;
  }

  return LMON_OK;
}

static int set_cobosec_mode() {
  int rc = 0;
#if defined(MUNGE)
  cobo_sec_protocol.mechanism = hs_munge;
#elif defined(KEYFILE)
  int fd;
  unsigned char akey[8];
  char kp[PATH_MAX];
  snprintf(kp, PATH_MAX, "%s/keyfile.%d", SEC_KEYDIR, getuid());
  cobo_sec_protocol.mechanism = hs_key_in_file;
  cobo_sec_protocol.data.key_in_file.key_filepath = strdup(kp);
  cobo_sec_protocol.data.key_in_file.key_length_bytes = 8;

  if ((fd = open("/dev/urandom", O_RDONLY)) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "opening /dev/urandom failed");
    rc = -1;
    goto done;
  }
  if (lmon_read_raw(fd, akey, sizeof(akey)) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "read from /dev/urandom failed.");
    rc = -1;
    goto done;
  }
  close(fd);
  remove(kp);
  if ((fd = open(cobo_sec_protocol.data.key_in_file.key_filepath,
                 O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "Failed to open %s.",
                 cobo_sec_protocol.data.key_in_file.key_filepath);
    rc = -1;
    goto done;
  }
  if (lmon_write_raw(fd, akey, sizeof(akey)) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "write to %s failed.", SEC_KEYDIR);
    rc = -1;
    goto done;
  }
  if (fchmod(fd, S_IRUSR) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "chmod %s failed.",
                 cobo_sec_protocol.data.key_in_file.key_filepath);
    rc = -1;
    goto done;
  }
  close(fd);
#elif defined(ENABLE_NULL_ENCRYPTION)
  cobo_sec_protocol.mechanism = hs_none;
#else
#error No recognized handshake mechanism enabled
#endif

done:
  return rc;
}

static lmon_rc_e LMON_assist_ICCL_BE_init(lmon_session_desc_t *mydesc) {
#if MEASURE_TRACING_COST
  double be_ts;
  double be_ts_buf;
  double c_start_ts;
  double c_end_ts;
  c_start_ts = gettimeofdayD();
  be_ts = 0.0f;
  {
    LMON_say_msg(LMON_FE_MSG_PREFIX, false, "LMON_assist_ICCL_BE_init begins");
  }
#endif

#if COBO_BASED
  /*
   * The following code is from Adam Moody's
   * PMGR Collective package, implementing
   * the communication protocol that allows talking
   * with the PMGR Collective's clients.
   *
   * And his COBO support
   *
   */
  int i = 0;
  int tosec = 0;
  int ndmons = 0;
  char *tout = NULL;
  unsigned int hcnt = 0;
  const char **hostlist = NULL;
  int *portlist = NULL;

  struct sockaddr_in sockaddr;
  unsigned int sockaddr_len = sizeof(sockaddr);

  tout = getenv("LMON_BE_DAEMON_TIMEOUT");
  if (tout && ((atoi(tout) > 0) && (atoi(tout) <= MAX_TIMEOUT)))
    tosec = atoi(tout);
  else
    tosec = DFLT_FE_BE_TOUT;

  if (!mydesc->proctab_msg) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "proctab msg has not yet arrived from the launchmon engine! A "
                 "race condition?");

    return LMON_EBUG;
  } else {
    if (parse_raw_RPDTAB_msg(mydesc->proctab_msg, &(mydesc->pMap)) < 0) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, false, "failed to parse MPIR_proctable");

      return LMON_ESYS;
    }

    if ((mydesc->pMap.size()) != 0) {
      int j;
      std::map<std::string, std::vector<MPIR_PROCDESC_EXT *> >::const_iterator
          iter;
      hostlist =
          (const char **)malloc(mydesc->pMap.size() * sizeof(const char *));
      if (hostlist == NULL) {
        LMON_say_msg(LMON_FE_MSG_PREFIX, false, "malloc returned NULL");

        return LMON_ENOMEM;
      }

      for (iter = mydesc->pMap.begin(); iter != mydesc->pMap.end(); iter++) {
        // Unless mydesc->pMap destroyed strings that hostlist points
        // to should be valid
        hostlist[hcnt] = iter->first.c_str();
        hcnt++;
      }

      /* This is only for testing */
      portlist = (int *)malloc(COBO_PORT_RANGE * sizeof(int));
      if (portlist == NULL) {
        LMON_say_msg(LMON_FE_MSG_PREFIX, false, "malloc returned NULL");

        return LMON_ENOMEM;
      }

      for (j = 0; j < COBO_PORT_RANGE; ++j) portlist[j] = COBO_BEGIN_PORT + j;
    }
  }

  mydesc->commDesc[fe_be_conn].nDaemons =
      mydesc->proctab_msg->sec_or_stringinfo.exec_and_hn.num_host_name;
  ndmons = mydesc->commDesc[fe_be_conn].nDaemons;

  if (set_cobosec_mode() < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "Failed to set COBO secure"
                 " handshake mode for back-end daemons.");
    return LMON_ESYS;
  }

  /*
   * Now taking advantage of COBO's new scalable bootstrapping
   * Session id=10 for now.
   */
  if (cobo_server_open(10, (char **)hostlist, hcnt, portlist,
                       COBO_PORT_RANGE) != COBO_SUCCESS) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "cobo_server_open failed.");

    return LMON_ESYS;
  }

  if (cobo_server_get_root_socket(&(
          mydesc->commDesc[fe_be_conn].sessionAcceptSockFd)) != COBO_SUCCESS) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "cobo_server_get_rootsocket failed.");

    return LMON_ESYS;
  }

  free(hostlist);
  free(portlist);

#if MEASURE_TRACING_COST
  if (lmon_read_raw(mydesc->commDesc[fe_be_conn].sessionAcceptSockFd,
                    (void *)&be_ts, sizeof(double)) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "read on a socket failed during MEASURE_TRACING_COST.");

    return LMON_ESYS;
  }
#endif

#endif /* COBO_BASED */

#if MEASURE_TRACING_COST
  c_end_ts = gettimeofdayD();
  {
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, false,
        "The inclusive LMON overhead of bootstrapping the ICCL layer: %f secs",
        (c_end_ts - c_start_ts));
    if (be_ts > (0.0f + 1E-10)) {
      if (be_ts > c_end_ts) {
        LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                     "The BE time stamp is greater than the end of the ICCL "
                     "layer bootstrapping!");
        LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                     "Must be out-of-synch clocks across remote nodes. The "
                     "exclusive LMON overhead cannot be determined");
      } else {
        LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                     "The exclusive LMON overhead of bootstrapping the ICCL "
                     "layer: %f secs",
                     (c_end_ts - be_ts));
      }
    }
  }
#endif

  return LMON_OK;
}

static lmon_rc_e LMON_assist_ICCL_MW_init(lmon_session_desc_t *mydesc) {
#if MEASURE_TRACING_COST
  double mw_ts = 0.0f;
  double c_start_ts;
  double c_end_ts;
  c_start_ts = gettimeofdayD();
  {
    LMON_say_msg(LMON_FE_MSG_PREFIX, false, "LMON_assist_ICCL_MW_init begins");
  }
#endif

  std::vector<spawner_base_t *>::const_iterator spawner_iter;

#ifdef COBO_BASED
  for (spawner_iter = mydesc->spawner_vector.begin();
       spawner_iter != mydesc->spawner_vector.end(); spawner_iter++) {
    // Spawn isn't blocking so that we can overlap the spawning
    // daemons across different volumns
    // Operation will come to the completion when
    // we throw COBO bootstrap on them as part of fe_mwHandshakeSequence
    char ssec[128];
    char secchk[128];
    snprintf(ssec, 128, "--lmonsharedsec=%s", mydesc->shared_key);
    snprintf(secchk, 128, "--lmonsecchk=%d", mydesc->randomID);
    (*spawner_iter)->get_daemon_args().push_back(std::string(ssec));
    (*spawner_iter)->get_daemon_args().push_back(std::string(secchk));
    (*spawner_iter)->spawn();
  }

  std::vector<std::string> combinedHostList;

  std::vector<spawner_base_t *>::const_iterator iter;
  for (iter = mydesc->spawner_vector.begin();
       iter != mydesc->spawner_vector.end(); ++iter) {
    (*iter)->combineHosts(combinedHostList);
  }

  //
  // COBO BOOTSTRAP of MW daemons
  //
  //
  int COBO_MW_BEGIN_PORT = COBO_BEGIN_PORT + COBO_PORT_RANGE;
  int *portlist = (int *)malloc(COBO_PORT_RANGE * sizeof(int));
  if (portlist == NULL) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, false, "malloc returned NULL");

    return LMON_ENOMEM;
  }

  int i;
  for (i = 0; i < COBO_PORT_RANGE; ++i) portlist[i] = COBO_MW_BEGIN_PORT + i;

  const char **cobohl = NULL;
  cobohl =
      (const char **)malloc(sizeof(const char *) * combinedHostList.size());
  if (cobohl == NULL) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, false, "malloc returned NULL");

    return LMON_ENOMEM;
  }

  int j = 0;
  std::vector<std::string>::const_iterator h_it;
  for (h_it = combinedHostList.begin(); h_it != combinedHostList.end();
       ++h_it) {
    cobohl[j] = (*h_it).c_str();
    j++;
  }

  if (set_cobosec_mode() < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "Failed to set COBO secure"
                 " handshake mode for middleware deamons.");
    return LMON_ESYS;
  }

  //
  // session ID = 11 for now
  //
  if (cobo_server_open(11, (char **)cobohl, combinedHostList.size(), portlist,
                       COBO_PORT_RANGE) != COBO_SUCCESS) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "cobo_server_open failed for launchMwDaemons.");

    return LMON_ESYS;
  }

  if (cobo_server_get_root_socket(&(
          mydesc->commDesc[fe_mw_conn].sessionAcceptSockFd)) != COBO_SUCCESS) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "cobo_server_get_rootsocket failed for launchMwDaemons");

    return LMON_ESYS;
  }

  free(portlist);
  free(cobohl);
// combinedHostList will be freed by the dtor

#if MEASURE_TRACING_COST
  if (lmon_read_raw(mydesc->commDesc[fe_mw_conn].sessionAcceptSockFd,
                    (void *)&mw_ts, sizeof(double)) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "read on a socket failed during MEASURE_TRACING_COST.");

    return LMON_ESYS;
  }

  c_end_ts = gettimeofdayD();
  {
    LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                 "The LMON overhead of bootstrapping the ICCL layer for MW "
                 "daemons is: %f secs",
                 (c_end_ts - c_start_ts));
  }
#endif

  return LMON_OK;
#else

  return LMON_EDUNAV;
#endif
}

static bool LMON_fe_is_secure(unsigned char *decryptedID, int k_len,
                              lmon_session_desc_t *mydesc) {
  int32_t int_decryptedID;
  gcry_error_t gcrc;

#if VERBOSE
  LMON_say_msg(LMON_FE_MSG_PREFIX, false, "GCRYPT encrypt, %x:%x:%x:%x",
               *(int *)decryptedID, *(int *)(decryptedID + 4),
               *(int *)(decryptedID + 8), *(int *)(decryptedID + 12));
#endif

  //
  // We're currenting using a symmetric cryptography
  //
  if ((gcrc = gcry_cipher_decrypt(mydesc->cipher_hndl,
                                  (unsigned char *)decryptedID, k_len, NULL,
                                  0)) != GPG_ERR_NO_ERROR) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "gcry_cipher_decrypt failed: %s",
                 gcry_strerror(gcrc));

    return false;
  }

#if VERBOSE
  LMON_say_msg(LMON_FE_MSG_PREFIX, false, "GCRYPT decrypt, %x:%x:%x:%x",
               *(int *)decryptedID, *(int *)(decryptedID + 4),
               *(int *)(decryptedID + 8), *(int *)(decryptedID + 12));
#endif

  memcpy((void *)&int_decryptedID, (void *)decryptedID, 4);
  if (mydesc->randomID != int_decryptedID) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "connection cannot be authorized, disconnecting...");

    return false;
  }

  return true;
}

//! lmon_rc_e LMON_fe_beHandshakeSequence
/*!
    -- assist BE's ICCL layer bootstrap (this can be NOOP most of the cases)
    -- accept the connection made by the master BE daemon
    -- read a msg of lmonp_febe_security_chk
    -- read "lmonp_befe_hostname" message along with the BE hostname array
    -- write "lmonp_febe_proctab" message along with the proctab
    -- write "lmon_febe_launch" or "lmon_febe_attach"
    -- write "lmonp_febe_usrdata" message along with the user data if there
       are data to ship out
    -- read "lmonp_be_ready" message along with BE user data piggybacked
    -- write "lmonp_cont_launch_bp" message to the engine indicating
       the be handshake is done
*/
static lmon_rc_e LMON_fe_beHandshakeSequence(
    int sessionHandle, bool is_launch, /* true for launch, false for attach*/
    void *febe_data,                   /* usrdata in        */
    void *befe_data)                   /* usrdata out       */
{
  struct sockaddr_in clientaddr;
  socklen_t clientaddr_len;
  lmon_session_desc_t *mydesc;
  lmonp_t msg;
  gcry_error_t gcrc;
  clientaddr_len = sizeof(clientaddr);
  unsigned char decryptedID[LMON_KEY_LENGTH];
  unsigned char *dec_traverse;
  int32_t int_decryptedID;
  char *tv;
  char *tout = NULL;
  int len;
  int i;
  int tosec = 0;

  mydesc = &(sess.sessionDescArray[sessionHandle]);

  //
  // participating in ICCL BE Bootstraping, if necessary
  //
  //
  if (LMON_assist_ICCL_BE_init(mydesc) != LMON_OK) {
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, true,
        "back ends bootstrapping throgh LMON_assist_ICCL_BE_init failed ");

    return LMON_EINVAL;
  }

  if (mydesc->commDesc[fe_be_conn].sessionAcceptSockFd < 0) {
    //
    // ACCEPTING CONNECTION FROM THE MASTER BACKEND DAEMON
    //   we use the timed version of accept to prevent
    //   an indefinite hang.
    //
    tout = getenv("LMON_BE_DAEMON_TIMEOUT");
    if (tout && ((atoi(tout) > 0) && (atoi(tout) <= MAX_TIMEOUT)))
      tosec = atoi(tout);
    else
      tosec = DFLT_FE_BE_TOUT;

    mydesc->commDesc[fe_be_conn].sessionAcceptSockFd = lmon_timedaccept(
        mydesc->commDesc[fe_be_conn].sessionListenSockFd,
        (struct sockaddr *)&clientaddr, &clientaddr_len, tosec);

    if (mydesc->commDesc[fe_be_conn].sessionAcceptSockFd == -2) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "connection to the back end master timed out");
      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "the back ends have been crashed or never been invoked?");

      return LMON_ETOUT;
    } else if (mydesc->commDesc[fe_be_conn].sessionAcceptSockFd < 0) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "connection to the back end master failed");
      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "the back ends have been crashed or never been invoked?");

      return LMON_ESYS;
    }
  }

  dec_traverse = decryptedID;
  for (i = 0; i < LMON_KEY_LENGTH / sizeof(int32_t); i++) {
    //
    // SECCHK MSG
    //   -- read and assert lmonp_febe_security_chk type message
    //
    read_lmonp_msgheader(mydesc->commDesc[fe_be_conn].sessionAcceptSockFd,
                         &msg);

    if ((msg.msgclass != lmonp_fetobe) ||
        (msg.type.fetobe_type != lmonp_febe_security_chk) ||
        (msg.lmon_payload_length != 0) || (msg.lmon_payload_length != 0)) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "Received an invalid LMONP msg: "
                   "Front-end back-end protocol mismatch? "
                   "or back-end disconnected?");

      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "  A proper msg of "
                   "{Class(%s),"
                   "Type(%s),"
                   "LMON_payload_size(%s)} is expected."
                   "lmonp_fetobe",
                   "lmonp_febe_security_chk",
                   lmon_msg_to_str(field_lmon_payload_length, &msg));

      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "  A msg of "
                   "{Class(%s),"
                   "Type(%s),"
                   "LMON_payload_size(%d)} has been received.",
                   lmon_msg_to_str(field_class, &msg),
                   lmon_msg_to_str(field_type, &msg),
                   lmon_msg_to_str(field_lmon_payload_length, &msg));

      return LMON_EBDMSG;
    }

    //
    // Make sure decrypting security_key2 == randomID
    //
    int32_t tmpsec = msg.sec_or_stringinfo.security_key2;
    memcpy((void *)dec_traverse, (void *)&tmpsec, sizeof(int32_t));
    dec_traverse += sizeof(int32_t);
  }

#if VERBOSE
  LMON_say_msg(LMON_FE_MSG_PREFIX, false, "BE authentication");
#endif

  //
  // is this connection secure?
  //
  if (!LMON_fe_is_secure(decryptedID, LMON_KEY_LENGTH, mydesc)) {
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, true,
        "BE connection cannot be authorized... discontinuing the handshake");

    close(mydesc->commDesc[fe_be_conn].sessionAcceptSockFd);
    //
    // BE ICCL should detect the connection and start to clear itself up.
    //
    return LMON_ESYS;
  }

  //
  // BE HOSTNAME MSG
  //   -- handshake  message from BE master with BE HOSTNAME array
  //
  if (read_lmonp_msgheader(mydesc->commDesc[fe_be_conn].sessionAcceptSockFd,
                           &msg) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "read_lmonp_msgheader failed"
                 " while attempting to handshake with back end master");

    return LMON_ESYS;
  }

  if ((msg.msgclass != lmonp_fetobe) ||
      (msg.type.fetobe_type != lmonp_befe_hostname) ||
      (msg.lmon_payload_length + msg.usr_payload_length) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "Received an invalid LMONP msg: "
                 "Front-end back-end protocol mismatch? "
                 "or back-end disconnected?");

    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "  A proper msg of "
                 "{Class(%s),"
                 "Type(%s),"
                 "LMON_payload_size(%s)} is expected."
                 "lmonp_fetobe",
                 "lmonp_febe_hostname", ">=0");

    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "  A msg of "
                 "{Class(%s),"
                 "Type(%s),"
                 "LMON_payload_size(%s),"
                 "USR_payload_size(%s)} has been received.",
                 lmon_msg_to_str(field_class, &msg),
                 lmon_msg_to_str(field_type, &msg),
                 lmon_msg_to_str(field_lmon_payload_length, &msg),
                 lmon_msg_to_str(field_usr_payload_length, &msg));

    return LMON_EBDMSG;
  }

  len = msg.lmon_payload_length + msg.usr_payload_length;

  mydesc->hntab_msg = (lmonp_t *)malloc(len + sizeof(msg));
  if (mydesc->hntab_msg == NULL) return LMON_ENOMEM;

  memcpy(mydesc->hntab_msg, &msg, sizeof(msg));
  tv = (char *)mydesc->hntab_msg;
  tv += sizeof(msg);
  if (read_lmonp_payloads(mydesc->commDesc[fe_be_conn].sessionAcceptSockFd, tv,
                          len) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "read_lmonp_payloads failed"
                 "while attempting to handshake with back end master");

    return LMON_ESYS;
  }

  if (mydesc->proctab_msg == NULL) return LMON_EBUG;

  //
  // PROCTAB MSG
  //   -- We simply flip some bits to convert this message to
  //      "lmonp_febe_proctab" type
  //
  mydesc->proctab_msg->msgclass = lmonp_fetobe;
  mydesc->proctab_msg->type.fetobe_type = lmonp_febe_proctab;
  if (write_lmonp_long_msg(
          mydesc->commDesc[fe_be_conn].sessionAcceptSockFd, mydesc->proctab_msg,
          sizeof(lmonp_t) + mydesc->proctab_msg->lmon_payload_length +
              mydesc->proctab_msg->usr_payload_length) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "write_lmonp_long_msg failed"
                 "while attempting to handshake with back end master");

    return LMON_ESYS;
  }

  //
  // launch case or attach case?
  //
  lmonp_t use_type_msg;
  if (is_launch) {
    lmonp_fe_to_be_msg_e tracemode = lmonp_febe_launch;
    const char *dontstop;
    if ((dontstop = getenv("LMON_DONT_STOP_APP"))) {
      if (atoi(dontstop) == 1) {
        tracemode = lmonp_febe_launch_dontstop;
      }
    }
    set_msg_header(&use_type_msg, lmonp_fetobe, tracemode, 0, 0, 0, 0, 0, 0, 0);
  } else {
    lmonp_fe_to_be_msg_e tracemode = lmonp_febe_attach;
    const char *dontstop;
    if ((dontstop = getenv("LMON_DONT_STOP_APP"))) {
      if (atoi(dontstop) == 0) {
        tracemode = lmonp_febe_attach_stop;
      }
    }

    set_msg_header(&use_type_msg, lmonp_fetobe, tracemode, 0, 0, 0, 0, 0, 0, 0);
  }

  write_lmonp_long_msg(mydesc->commDesc[fe_be_conn].sessionAcceptSockFd,
                       &use_type_msg, sizeof(use_type_msg));

  //
  // rm_type
  //
  lmonp_t rm_type_msg;
  rm_catalogue_e rm_entry =
      mydesc->rm_info.rm_supported_types[mydesc->rm_info.index_to_cur_instance];
  set_msg_header(&rm_type_msg, lmonp_fetobe, lmonp_febe_rm_type,
                 (unsigned short)rm_entry, 0, 0, 0, 0, 0, 0);

  write_lmonp_long_msg(mydesc->commDesc[fe_be_conn].sessionAcceptSockFd,
                       &rm_type_msg, sizeof(rm_type_msg));

  //
  // USRDATA MSG
  //  -- writing the lmonp_febe_usrdata message along with the user data
  //     if there are data to ship out
  //
  lmon_rc_e lrc;
  lrc = LMON_fe_handleFeBeUsrData(sessionHandle, febe_data);
  if (lrc != LMON_OK && lrc != LMON_ENOPLD) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "LMON_fe_handleFeBeUsrData returned an error code");
    return lrc;
  }

  //
  // CONTINUE LAUNCH MSG
  //
  lmonp_t cont_req_msg;
  if (read_lmonp_msgheader(mydesc->commDesc[fe_be_conn].sessionAcceptSockFd,
                           &cont_req_msg) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "read_lmonp_msgheader failed"
                 " while attempting to receive continue launch message from "
                 "back end master");

    return LMON_ESYS;
  }

  if ((cont_req_msg.msgclass != lmonp_fetobe) ||
      (cont_req_msg.type.fetobe_type != lmonp_befe_cont_launch_bp)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "Received an invalid LMONP msg: "
                 "Front-end back-end protocol mismatch? "
                 "or back-end disconnected?");

    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "  A proper msg of "
                 "{Class(%s),"
                 "Type(%s),"
                 "LMON_payload_size(%s)} is expected."
                 "lmonp_fetobe",
                 "lmonp_befe_cont_launch_bp", "==0");

    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "  A msg of "
                 "{Class(%s),"
                 "Type(%s),"
                 "LMON_payload_size(%s),"
                 "USR_payload_size(%s)} has been received.",
                 lmon_msg_to_str(field_class, &cont_req_msg),
                 lmon_msg_to_str(field_type, &cont_req_msg),
                 lmon_msg_to_str(field_lmon_payload_length, &cont_req_msg),
                 lmon_msg_to_str(field_usr_payload_length, &cont_req_msg));

    return LMON_EBDMSG;
  }

  //
  // Release the continue message to the engine that it is now safe to continue
  // out of launch-bp
  //
  lmonp_t engineMsg;
  init_msg_header(&engineMsg);
  engineMsg.msgclass = lmonp_fetofe;
  engineMsg.type.fetofe_type = lmonp_cont_launch_bp;

  write_lmonp_long_msg(mydesc->commDesc[fe_engine_conn].sessionAcceptSockFd,
                       &engineMsg, sizeof(engineMsg));

  //
  // READY MSG ( usrdata can be piggybacked )
  //  -- receiving the ready message from BE master
  //
  lrc = LMON_fe_handleBeFeUsrData(sessionHandle, befe_data);
  if (lrc != LMON_OK && lrc != LMON_ENOPLD) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "LMON_fe_handleBeFeUsrData returned an error code");
    return lrc;
  }

  //
  // Handshake has been successful
  //
  return LMON_OK;
}

//! lmon_rc_e LMON_fe_mwHandshakeSequence
/*!
    -- accept the connection made by the master MW daemon (ICCL)
         via LMON_assist_ICCL_MW_init
    -- read a msg of lmonp_femw_security_chk (SECURITY)
    -- read "lmonp_mwfe_hostname" message along with the BE hostname array
   (HOSTNAME)
    -- write "lmonp_femw_usrdata" message along with the user data if there
   (USER COMM.)
       are data to ship out
    -- read "lmonp_mw_ready" message along with BE user data piggybacked (READY)
*/
static lmon_rc_e LMON_fe_mwHandshakeSequence(int sessionHandle,
                                             void *femw_data, /* usrdata in */
                                             void *mwfe_data) /* usrdata out */
{
  lmon_rc_e lrc;
  lmon_session_desc_t *mydesc;
  unsigned int i;

  mydesc = &(sess.sessionDescArray[sessionHandle]);

  //
  // TODO: need a timeout mechanism for COBO-based boostrapping
  //       talk to Adam Moody
  //
  lrc = LMON_assist_ICCL_MW_init(mydesc);
  if (lrc != LMON_OK) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "LMON_assist_ICCL_MW_init failed ");

    return lrc;
  }

  unsigned char encrypt_packet_buf[LMON_KEY_LENGTH];
  unsigned char *encrypt_trav = encrypt_packet_buf;
  lmonp_t msg;
  for (i = 0; i < LMON_KEY_LENGTH / sizeof(int32_t); i++) {
    //
    // SECCHK MSG
    //   -- read and assert lmonp_femw_security_chk type message
    //
    read_lmonp_msgheader(mydesc->commDesc[fe_mw_conn].sessionAcceptSockFd,
                         &msg);

    if ((msg.msgclass != lmonp_fetomw) ||
        (msg.type.fetomw_type != lmonp_femw_security_chk) ||
        (msg.lmon_payload_length != 0) || (msg.usr_payload_length != 0)) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "Received an invalid LMONP msg: "
                   "Front-end middleware protocol mismatch? "
                   "or middleware disconnected?");

      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "  A proper msg of "
                   "{Class(%s),"
                   "Type(%s),"
                   "LMON_payload_size(%s)} is expected."
                   "lmonp_fetomw",
                   "lmonp_femw_security_chk", "0");

      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "  A msg of "
                   "{Class(%s),"
                   "Type(%s),"
                   "LMON_payload_size(%d)} has been received.",
                   lmon_msg_to_str(field_class, &msg),
                   lmon_msg_to_str(field_type, &msg),
                   lmon_msg_to_str(field_lmon_payload_length, &msg));

      LMON_say_msg(LMON_FE_MSG_PREFIX, true, "discontinuing the handshake...");

      return LMON_EBDMSG;
    }

    //
    // Filling the buf of LMON_KEY_LENGTH bytes with receiving encripted
    // payloads
    //
    int32_t tmpsec = msg.sec_or_stringinfo.security_key2;
    memcpy((void *)encrypt_trav, (void *)&tmpsec, sizeof(int32_t));
    encrypt_trav += sizeof(int32_t);
  }

#if VERBOSE
  LMON_say_msg(LMON_FE_MSG_PREFIX, false, "MW authentication");
#endif

  //
  // is this connection secure?
  //   -- read a msg of lmonp_femw_security_chk
  //
  if (!LMON_fe_is_secure(encrypt_packet_buf, LMON_KEY_LENGTH, mydesc)) {
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, true,
        "BE connection cannot be authorized... discontinuing the handshake");

    close(mydesc->commDesc[fe_mw_conn].sessionAcceptSockFd);
    //
    // MW ICCL should detect the connection and start to clear itself up.
    //
    return LMON_ESYS;
  }

  //
  // MW HOSTNAME MSG
  //   -- handshake  message from MW master with BE HOSTNAME array
  //
  if (read_lmonp_msgheader(mydesc->commDesc[fe_mw_conn].sessionAcceptSockFd,
                           &msg) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "read_lmonp_msgheader failed");

    return LMON_ESYS;
  }

  if ((msg.msgclass != lmonp_fetomw) ||
      (msg.type.fetomw_type != lmonp_mwfe_hostname) ||
      ((msg.lmon_payload_length + msg.usr_payload_length) < 0)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "Received an invalid LMONP msg: "
                 "Front-end middleware protocol mismatch? "
                 "or middleware disconnected?");

    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "  A proper msg of "
                 "{Class(%s),"
                 "Type(%s),"
                 "LMON_payload_size(%s)} is expected."
                 "lmonp_fetomw",
                 "lmonp_femw_hostname", ">0");

    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "  A msg of "
                 "{Class(%s),"
                 "Type(%s),"
                 "LMON_payload_size(%d)} has been received.",
                 lmon_msg_to_str(field_class, &msg),
                 lmon_msg_to_str(field_type, &msg),
                 lmon_msg_to_str(field_lmon_payload_length, &msg));

    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "discontinuing the handshake...");

    return LMON_EBDMSG;
  }

  unsigned int len = msg.lmon_payload_length + msg.usr_payload_length;

  mydesc->hntab_mw_msg = (lmonp_t *)malloc(len + sizeof(msg));
  if (mydesc->hntab_mw_msg == NULL) return LMON_ENOMEM;

  memcpy(mydesc->hntab_mw_msg, &msg, sizeof(msg));
  char *tv = (char *)mydesc->hntab_mw_msg;
  tv += sizeof(msg);
  if (read_lmonp_payloads(mydesc->commDesc[fe_mw_conn].sessionAcceptSockFd, tv,
                          len) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "read_lmonp_payloads failed"
                 "while attempting to handshake with middleware master");

    return LMON_ESYS;
  }

  //
  // USRDATA MSG
  //  -- writing the lmonp_femw_usrdata message along with the user data
  //     if there are data to ship out
  //
  lrc = LMON_fe_handleFeMwUsrData(sessionHandle, femw_data);
  if (lrc != LMON_OK && lrc != LMON_ENOPLD) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "LMON_fe_handleFeMwUsrData returned an error code");
    return lrc;
  }

  //
  // READY MSG ( usrdata can be piggybacked )
  //  -- receiving the ready message from MW master
  //
  lrc = LMON_fe_handleMwFeUsrData(sessionHandle, mwfe_data);
  if (lrc != LMON_OK && lrc != LMON_ENOPLD) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "LMON_fe_handleMwFeUsrData returned an error code");
    return lrc;
  }

  //
  // Handshake has been successful
  //
  return LMON_OK;
}

//! LMON_set_options
/*!
  sets the given options in a way that the main launchmon
  engine understands
*/
static lmon_rc_e LMON_set_options(lmon_session_desc_t *mydesc, opts_args_t &opt,
                                  int verbose, bool attach,
                                  const char *launcher, char *launcher_argv[],
                                  int launcherPid, const char *toolDaemon,
                                  char *daemon_argopts[]) {
  using namespace std;

  int i;
  char portinfo[16];
  opt_struct_t *optcontext;
  enum self_trace_verbosity ver;

  optcontext = opt.get_my_opt();
  optcontext->verbose = verbose;
  optcontext->debugtarget = "";

  if (verbose == 0)
    ver = quiet;
  else if (verbose == 1)
    ver = level1;
  else if (verbose == 2)
    ver = level2;
  else if (verbose == 3)
    ver = level3;
  else
    ver = quiet;

  self_trace_t::self_trace().launchmon_module_trace.verbosity_level = ver;
  self_trace_t::self_trace().tracer_module_trace.verbosity_level = ver;
  self_trace_t::self_trace().symtab_module_trace.verbosity_level = ver;
  self_trace_t::self_trace().event_module_trace.verbosity_level = ver;
  self_trace_t::self_trace().driver_module_trace.verbosity_level = ver;
  self_trace_t::self_trace().machine_module_trace.verbosity_level = ver;
  self_trace_t::self_trace().opt_module_trace.verbosity_level = ver;

  sprintf(portinfo, "%d",
          (unsigned short)ntohs(
              mydesc->commDesc[fe_engine_conn].servAddr.sin_port));

  optcontext->remote = true;
  optcontext->remote_info = mydesc->commDesc[fe_engine_conn].ipInfo;
  optcontext->remote_info += ":";
  optcontext->remote_info += portinfo;

  char tmprandomID[128];

  sprintf(tmprandomID, "%d", mydesc->randomID);

  optcontext->lmon_sec_info = mydesc->shared_key;
  optcontext->lmon_sec_info += ":";
  optcontext->lmon_sec_info += tmprandomID;
  optcontext->attach = attach;
  optcontext->tool_daemon = toolDaemon;

  if (launcher != NULL) {
    optcontext->debugtarget = launcher;
  }

  if (launcherPid > 0) {
    optcontext->launcher_pid = launcherPid;
  }

  optcontext->tool_daemon_opts.clear();

  if (launcher_argv != NULL) {
    optcontext->remaining = launcher_argv;
  }

  if (daemon_argopts != NULL) {
    for (i = 0;; ++i) {
      if (daemon_argopts[i] != NULL) {
        optcontext->tool_daemon_opts.push_back(string(daemon_argopts[i]));
      } else
        break;
    }
  }

  lmon_daemon_env_t *trav = mydesc->daemonEnvList[0];

  while (trav != NULL) {
    optcontext->envMap.insert(
        make_pair(string(trav->envName), string(trav->envValue)));
    trav = trav->next;
  }

  return LMON_OK;
}

//! LMON_openBindAndListen
/*!
  opens up a TCP socket, binding it with a random port and listening in
  on it. This is per-session server socket.

  Mar 5 2008: DHA We added "O_NONBLOCK flag to the socket here to
  be able to prevent indefinite blocking on socket calls.
*/
static lmon_rc_e LMON_openBindAndListen(int *sfd) {
  int rc;
  struct sockaddr_in servaddr;
  struct hostent *hp;
  struct in_addr **addr;
  char hn[MAX_LMON_STRING];
  char *hngiven;

  if ((rc = gethostname(hn, MAX_LMON_STRING)) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "gethostname call failed");

    return LMON_ESYS;
  }

#if SUB_ARCH_BGL || SUB_ARCH_BGP || SUB_ARCH_BGQ
  //
  // BlueGene/L uses NIC whose name is suffixed with -io to communicate
  // packets to/from IO nodes.
  //
  // DHA 3/4/3009, reviewed. Looks fine for the DAWN configuration
  // /etc/hosts shows that I/O network is detnoted as FENname-io
  // following the same convention.
  //
  // Changed RM_BGL_MPIRUN to RM_BG_MPIRUN to genericize BlueGene Support
  //
  strcat(hn, "-io");
#elif SUB_ARCH_ALPS || SUB_ARCH_CRAY
  //
  // Cray uses its own way to name a node, so overwriting
  // hn with the name generated with their method
  //
  std::ifstream ifs("/proc/cray_xt/nid");
  uint32_t nid;
  ifs >> nid;

  std::ostringstream nidStr;
  nidStr << "nid" << std::setw(5) << std::setfill('0') << nid << std::ends;
  snprintf(hn, MAX_LMON_STRING, "%s", nidStr.str().c_str());
#endif

  //
  // LMON_FE_HOSTNAME_TO_CONN support to handle a condition where
  // the hostname returned by gethostname doesn't represent a
  // hostname for remote tool daemons to connect to
  //
  if ((hngiven = getenv("LMON_FE_HOSTNAME_TO_CONN")) != NULL) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                 "front-end hostname that remote daemons use is given");
    memset(hn, '\0', MAX_LMON_STRING);
    snprintf(hn, MAX_LMON_STRING, "%s", hngiven);
  }

  if ((hp = gethostbyname(hn)) == NULL) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "gethostbyname call failed");

    return LMON_ESYS;
  }

  addr = (struct in_addr **)hp->h_addr_list;
  if (((*sfd) = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "socket call failed");

    return LMON_ESYS;
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  memcpy(&servaddr.sin_addr, *addr, sizeof(struct in_addr));
  servaddr.sin_port = 0;

  if ((rc = bind((*sfd), (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "bind call failed");

    return LMON_ESYS;
  }

  if ((rc = listen((*sfd), SOMAXCONN)) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "listen call failed");

    return LMON_ESYS;
  }

  if ((rc = fcntl((*sfd), F_SETFL, O_NONBLOCK)) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "setting O_NONBLOCK for a socket failed");

    return LMON_ESYS;
  }

  return LMON_OK;
}

//! LMON_handle_proctab_event
/*!
  fetches the proctable message from the LaunchMON Engine
*/
static int LMON_handle_proctab_event(int readingFd, lmon_session_desc_t *mydesc,
                                     lmonp_t *msg) {
  int bytesread;
  char *proctab_message;
  char *trav_ptr;

  proctab_message = (char *)malloc(sizeof(*msg) + msg->lmon_payload_length +
                                   msg->usr_payload_length);

  if (proctab_message == NULL) return LMON_ENOMEM;

  memcpy(proctab_message, msg, sizeof(*msg));
  trav_ptr = proctab_message;
  trav_ptr += sizeof(*msg);

  bytesread = read_lmonp_payloads(
      readingFd, trav_ptr, msg->lmon_payload_length + msg->usr_payload_length);

  if (bytesread != (int)(msg->lmon_payload_length + msg->usr_payload_length)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "read_lmonp_payloads returned a bad return code");

    return -1;
  }

  //
  // registering a raw proctable message
  // one might expect a race condition here for fear that other thread may use
  // mydesc->proctab_msg before this assignment. But this assignment is
  // supposed to happen while the main thread is being block on a condition
  // variable either in LMON_fe_attachAndSpawnDaemons or
  // LMON_fe_launchAndSpawnDaemons
  //
  mydesc->proctab_msg = (lmonp_t *)proctab_message;

  return 0;
}

static int LMON_handle_resourcehandle_event(int readingFd,
                                            lmon_session_desc_t *mydesc,
                                            lmonp_t *msg) {
  int64_t rid;
  int bytesread;

  if ((msg->lmon_payload_length + msg->usr_payload_length) != sizeof(int64_t)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "payload size mismatch detected during "
                 "LMON_handle_resourcehandle_event");
    return -1;
  }

  bytesread = read_lmonp_payloads(
      readingFd, &rid, msg->lmon_payload_length + msg->usr_payload_length);

  if (bytesread != (int)(msg->lmon_payload_length + msg->usr_payload_length)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "read_lmonp_payloads returned a bad return code");

    return -1;
  }

  mydesc->resourceHandle = rid;

  return 0;
}

static int LMON_handle_rminfo_event(int readingFd, lmon_session_desc_t *mydesc,
                                    lmonp_t *msg) {
  uint32_t pid_rm_pair[2];
  int bytesread;

  /*
   * lmonp_payload: launcherpid (4bytes) rm_type (4bytes)
   */
  if ((msg->lmon_payload_length + msg->usr_payload_length) !=
      sizeof(pid_rm_pair)) {
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, true,
        "payload size mismatch detected during LMON_handle_rminfo_event");
    return -1;
  }

  bytesread =
      read_lmonp_payloads(readingFd, pid_rm_pair,
                          msg->lmon_payload_length + msg->usr_payload_length);

  if (bytesread != (int)(msg->lmon_payload_length + msg->usr_payload_length)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "read_lmonp_payloads returned a bad return code");

    return -1;
  }

  mydesc->rm_launcher_pid = (pid_t)pid_rm_pair[0];
  mydesc->rm_info.rm_launcher_pid = (pid_t)pid_rm_pair[0];

  rm_catalogue_e rm_type = (rm_catalogue_e)pid_rm_pair[1];
  int i;
  for (i = 0; i < mydesc->rm_info.num_supported_types; ++i) {
    if (mydesc->rm_info.rm_supported_types[i] == rm_type) {
      mydesc->rm_info.index_to_cur_instance = i;
      break;
    }
  }

  return (i < mydesc->rm_info.num_supported_types) ? 0 : -1;
}

static void LMON_child_fork_handler(void) {
  int i;
  lmon_session_desc_t *mydesc;

  /*
   *
   * This fork handler terminates all active threads because
   * they may cause unwonted contentions with
   * the parent process's threads.
   *
   * The recent code which allows LaunchMON to invoke a separate
   * LaunchMON Engine binary will make this handler unnecessary...
   *
   */

  for (i = 0; i < MAX_LMON_SESSION; ++i) {
    mydesc = &(sess.sessionDescArray[i]);

    /*
     * I wouldn't worry about data races for this routine. (no locks)
     */
    if ((mydesc->registered == LMON_TRUE) && (mydesc->spawned == LMON_TRUE)) {
#ifdef WATCHDOG_THREAD_DEBUG
      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "child fork handler, cancelling watchdog threads");
#endif
      pthread_cancel(mydesc->watchdogThr.fetofeMonitoringThr);
    }
  }
}

static int fe_getStatus(lmon_session_desc_t *mydesc, int *status) {
  int globalmask = 0;
  int localmask = 0;
  (*status) = 0;

  if (mydesc->registered == LMON_TRUE) {
    localmask = 1;
    globalmask |= localmask;
  }

  if (mydesc->spawned == LMON_TRUE) {
    localmask = 1;
    localmask = localmask << 1;
    globalmask |= localmask;
  }

  if (mydesc->mw_spawned == LMON_TRUE) {
    localmask = 1;
    localmask = localmask << 2;
    globalmask |= localmask;
  }

  if (mydesc->detached == LMON_TRUE) {
    localmask = 1;
    localmask = localmask << 3;
    globalmask |= localmask;
  }

  if (mydesc->killed == LMON_TRUE) {
    localmask = 1;
    localmask = localmask << 4;
    globalmask |= localmask;
  }

  (*status) = globalmask;

  return 0;
}

//! LMON_fetofe_watchdog_thread ( void* arg )
/*!
  The main watchdog thread routine. designed to watch per-session launchmon
  engine process.


  Currently, this function uses fine-grain pthread locks.
*/
static void *LMON_fetofe_watchdog_thread(void *arg) {
  int rc;
  int readingFd;
  int sessionId;
  int status;
  int *argconv;
  lmonp_t msg;
  lmon_session_desc_t *mydesc;

  /*
   ***************** IMPORTANT ***********************
   *
   * NOTE: Shared data access policy
   * Watchdog threads
   * 1. For watchdog threads, only this routine (top-level thread function)
   *    can acquire/release locks.
   * 2. Watchdog threads are responsible for destorying and initializing
   *    their session descriptors only when "asynchronous" termination
   *    event occurs.
   *
   * The Main thread
   * 1. For the main thread, only the top-level external API calls
   *    can acquire/release locks and use condition variables.
   * 2. The main thread is responsible for destorying and initializing
   *    its session descriptors when "synchronous" termination
   *    event occurs.
   *
   ***************** IMPORTANT ***********************
   */

  argconv = (int *)arg;
  readingFd = argconv[0];
  sessionId = argconv[1];
  mydesc = &(sess.sessionDescArray[sessionId]);

#ifdef WATCHDOG_THREAD_DEBUG
  char message[PATH_MAX];
  sprintf(message, "watchdog thread created: sessionid %d", sessionId);
  LMON_say_msg(LMON_FE_MSG_PREFIX, true, message);
#endif

  /*
   * While the per-session watchdog thread is alive, the session
   * descriptor cannot be destroyed and re-initialized.
   */
  for (;;) {
    //
    //
    // per-session watchdog thread's main event loop ...
    //
    //
    if ((rc = read_lmonp_msgheader(readingFd, &msg)) < 0) {
      //
      // If "read a message" fails, we should loop over
      //
      //
      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "read_lmonp_msg returned a negative return code");
      LMON_say_msg(
          LMON_FE_MSG_PREFIX, true,
          "front end's connection to launchmon engine is disconnected?");

      break;
    }

    switch (msg.type.fetofe_type) {
      case lmonp_stop_at_launch_bp_spawned:
      case lmonp_stop_at_first_attach:
//
// The events indicating "app tasks and tool daemons are spawned
//
//
#if VERBOSE
        LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                     "launch_bp or first_attach done...");
#endif
        break;

      case lmonp_proctable_avail:
        //
        // The event indicating the proctable is available.
        //
        //
        pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
        if (LMON_handle_proctab_event(readingFd, mydesc, &msg) != 0) {
          LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                       "LMON_handle_proctab_event failed");

          pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
          goto watchdog_done;
        }
        mydesc->spawned = LMON_TRUE;
        fe_getStatus(mydesc, &status);
        if (mydesc->statusCB != NULL) {
          if (mydesc->statusCB(&status) != 0) {
            LMON_say_msg(
                LMON_FE_MSG_PREFIX, false,
                "registered status call back returned non-zero... continue");
          }
        }
#if VERBOSE
        LMON_say_msg(LMON_FE_MSG_PREFIX, false, "RPDTAB message received...");
#endif
        //
        // Let the main thread know, the LaunchMON engine says
        // "it is okay to spawn daemons
        //
        pthread_cond_signal(&(mydesc->watchdogThr.condVar));
        pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
        break;

      case lmonp_resourcehandle_avail:
        //
        // The event indicating the resourcehandle is available.
        //
        //
        pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
        if (LMON_handle_resourcehandle_event(readingFd, mydesc, &msg) != 0) {
          LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                       "LMON_handle_resourcehandle_event failed");
          pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
          goto watchdog_done;
        }
#if VERBOSE
        LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                     "rid available event received...");
#endif
        pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
        break;

      case lmonp_rminfo:
        //
        // The event indicating RMInfo is available.
        //
        //
        pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
        if (LMON_handle_rminfo_event(readingFd, mydesc, &msg) != 0) {
          LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                       "LMON_handle_rminfo_event failed");
          pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
          goto watchdog_done;
        }
#if VERBOSE
        LMON_say_msg(LMON_FE_MSG_PREFIX, false, "rminfo event received...");
#endif
        pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
        break;

      case lmonp_detach_done:
        //
        // Synchronous detach event
        //
        pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
        mydesc->detached = LMON_TRUE;
        fe_getStatus(mydesc, &status);
        if (mydesc->statusCB != NULL) {
          if (mydesc->statusCB(&status) != 0) {
            LMON_say_msg(
                LMON_FE_MSG_PREFIX, false,
                "registered status call back returned non-zero... continue");
          }
        }
#if VERBOSE
        LMON_say_msg(LMON_FE_MSG_PREFIX, false, "detach cmd done...");
#endif
        pthread_cond_signal(&(mydesc->watchdogThr.condVar));
        pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
        goto watchdog_done;
        break;

      case lmonp_kill_done:
        //
        // Synchronous termination event
        //
        pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
        mydesc->killed = LMON_TRUE;
        fe_getStatus(mydesc, &status);
        if (mydesc->statusCB != NULL) {
          if (mydesc->statusCB(&status) != 0) {
            LMON_say_msg(
                LMON_FE_MSG_PREFIX, false,
                "registered status call back returned non-zero... continue");
          }
        }
#if VERBOSE
        LMON_say_msg(LMON_FE_MSG_PREFIX, false, "kill cmd done...");
#endif
        pthread_cond_signal(&(mydesc->watchdogThr.condVar));
        pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
        goto watchdog_done;
        break;

      case lmonp_stop_tracing:
        pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
        mydesc->detached = LMON_TRUE;
        fe_getStatus(mydesc, &status);
        if (mydesc->statusCB != NULL) {
          if (mydesc->statusCB(&status) != 0) {
            LMON_say_msg(
                LMON_FE_MSG_PREFIX, false,
                "registered status call back returned non-zero... continue");
          }
        }
#if VERBOSE
        LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                     "the engine stopped tracing the job...");
#endif
        // this can unlock the main thread that's possible waiting on a cond var
        pthread_cond_signal(&(mydesc->watchdogThr.condVar));
        pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
        goto watchdog_done;
        break;

      case lmonp_bedmon_exited:
      case lmonp_mwdmon_exited:
        pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
        mydesc->detached = LMON_TRUE;
        mydesc->spawned = LMON_FALSE;
        fe_getStatus(mydesc, &status);
        if (mydesc->statusCB != NULL) {
          if (mydesc->statusCB(&status) != 0) {
            LMON_say_msg(
                LMON_FE_MSG_PREFIX, false,
                "registered status call back returned non-zero... continue");
          }
        }
#if VERBOSE
        LMON_say_msg(LMON_FE_MSG_PREFIX, false, "the daemons terminated...");
#endif
        // this can unlock the main thread that's possible waiting on a cond var
        pthread_cond_signal(&(mydesc->watchdogThr.condVar));
        pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
        goto watchdog_done;
        break;

      case lmonp_exited:
      case lmonp_terminated:
      case lmonp_stop_at_launch_bp_abort:
        //
        // Asynchronous termination event
        // The event indicating the job is done, either normally or abnormally
        //
        pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
        mydesc->killed = LMON_TRUE;
        fe_getStatus(mydesc, &status);
        if (mydesc->statusCB != NULL) {
          if (mydesc->statusCB(&status) != 0) {
            LMON_say_msg(
                LMON_FE_MSG_PREFIX, false,
                "registered status call back returned non-zero... continue");
          }
        }
#if VERBOSE
        LMON_say_msg(LMON_FE_MSG_PREFIX, false, "the job terminated...");
#endif
        pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
        goto watchdog_asynch_termination;
        break;

      default:
        goto watchdog_done;
        break;
    }
  }

watchdog_asynch_termination:

#if 0
    pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
    LMON_destroy_sess (mydesc);
    LMON_init_sess (mydesc);
    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
#endif

watchdog_done:
  pthread_exit(NULL);
}

static void tokenize(std::string str, std::list<std::string> &result) {
#define append_to_buffer(C) buffer[buffer_pos++] = C

  size_t len = str.length(), buffer_pos = 0;
  char *buffer = (char *)malloc(len + 1);
  bool escaped = false, quoted = false;

  for (int i = 0; i < len; i++) {
    char c = str[i];
    if (escaped) {
      switch (c) {
        case 'a':
          append_to_buffer('\a');
          break;
        case 'b':
          append_to_buffer('\b');
          break;
        case 'f':
          append_to_buffer('\f');
          break;
        case 'n':
          append_to_buffer('\n');
          break;
        case 'r':
          append_to_buffer('\r');
          break;
        case 't':
          append_to_buffer('\t');
          break;
        case 'v':
          append_to_buffer('\v');
          break;
        default:
          append_to_buffer(c);
          break;
      }
      escaped = false;
    } else if (c == '\\') {
      escaped = true;
    } else if (c == '"') {
      quoted = !quoted;
    } else if (quoted) {
      append_to_buffer(c);
    } else if (c == ' ') {
      append_to_buffer('\0');
      result.push_back(buffer);
      buffer_pos = 0;

      while (i + 1 < len && buffer[i + 1] == ' ') i++;
    } else {
      append_to_buffer(c);
    }
  }

  if (buffer_pos > 0) {
    append_to_buffer('\0');
    result.push_back(buffer);
  }

  free(buffer);
}

static int tokenize(const std::string &str, char **myargv, int myargvMax) {
  char *first = (char *)str.c_str();
  char *last = NULL;
  int i = 0;
  int quote = 0;

  if (!first) return -1;

  for (i = 0; i < myargvMax; ++i) {
    while (*first != '\0' && ((*first == ' ') || (*first) == '"')) {
      if ((*first) == '"') quote = 1;
      first++;
    }

    last = first;

    if (quote) {
      while (*last != '\0' && *last != '"') last++;

      if (*last == '\0') return -1;
    } else {
      while (*last != '\0' && *last != ' ') last++;
    }

    if (first == last) break;

    myargv[i] = (char *)malloc(last - first + 1);
    memcpy(myargv[i], first, last - first);
    myargv[i][last - first] = '\0';

    if (quote) last++;

    quote = 0;
    first = last;
  }

  return i;
}

static lmon_rc_e bld_exec_lmon_launch_str(
    bool isLocal, const char *hostname,
    const std::list<std::string> &lmonOptArgs, opts_args_t &opt) {
  using namespace std;

  std::list<std::string> cmdlist;
  char *rm;
  char *lmonpath;
  char *debugflag;
  int k = 0;

  if (isLocal) {
    map<string, string>::const_iterator envListPos;
    for (envListPos = opt.get_my_opt()->envMap.begin();
         envListPos != opt.get_my_opt()->envMap.end(); envListPos++) {
      setenv(envListPos->first.c_str(), envListPos->second.c_str(), 1);
    }

    if (getenv("LMON_DEBUG_LAUNCHMON_ENGINE")) {
      tokenize(TVCMD, cmdlist);
    }

    if ((lmonpath = getenv("LMON_LAUNCHMON_ENGINE_PATH")) != NULL) {
      cmdlist.push_back(lmonpath);
    } else {
      cmdlist.push_back("launchmon");
    }

    if (getenv("LMON_DEBUG_LAUNCHMON_ENGINE")) {
      cmdlist.push_back("-a");
    }
    cmdlist.insert(cmdlist.end(), lmonOptArgs.begin(), lmonOptArgs.end());

    if (k < 0) return LMON_EINVAL;

  }  // The tool FEN is already co-located with the RM.
  else {
    // LMON_REMOTE_LOGIN (default=ssh)
    if ((rm = getenv("LMON_REMOTE_LOGIN")) != NULL)
      tokenize(rm, cmdlist);
    else
      tokenize(SSHCMD, cmdlist);

    cmdlist.push_back(hostname);

    cmdlist.push_back(ENVCMD);

    map<string, string>::const_iterator envListPos;
    for (envListPos = opt.get_my_opt()->envMap.begin();
         envListPos != opt.get_my_opt()->envMap.end(); envListPos++) {
      string lstring = envListPos->first + "=" + envListPos->second;
      cmdlist.push_back(lstring);
    }

    char *pref = getenv("LMON_PREFIX");
    if (pref) {
      cmdlist.push_back(string("LMON_PREFIX=") + string(pref));
    } else if (pref = getenv("LMON_RM_CONFIG_DIR")) {
      cmdlist.push_back(string("LMON_RM_CONFIG_DIR=") + string(pref));
      if (pref = getenv("LMON_COLOC_UTIL_DIR"))
        cmdlist.push_back(string("LMON_COLOC_UTIL_DIR=") + string(pref));
    }

    if (getenv("LMON_DEBUG_LAUNCHMON_ENGINE")) {
      tokenize(TVCMD, cmdlist);
    }

    if ((lmonpath = getenv("LMON_LAUNCHMON_ENGINE_PATH")) != NULL) {
      cmdlist.push_back(lmonpath);

      //
      // perform a sanity check here of the given path
      //
    } else {
      cmdlist.push_back("launchmon");
      //
      // perform a sanity check if this is in the user's PATH
      //
    }

    if (getenv("LMON_DEBUG_LAUNCHMON_ENGINE")) {
      cmdlist.push_back("-a");
    }

    cmdlist.insert(cmdlist.end(), lmonOptArgs.begin(), lmonOptArgs.end());
  }  // The tool FEN is not co-located with the RM.

  char **myargv = (char **)malloc(sizeof(char *) * (cmdlist.size() + 1));
  unsigned int j = 0;
  for (list<string>::iterator i = cmdlist.begin(); i != cmdlist.end(); i++) {
    myargv[j++] = const_cast<char *>(i->c_str());
  }
  myargv[j] = NULL;

  if (execvp(myargv[0], myargv) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "LaunchMON Engine invocation failed, exiting: %s",
                 strerror(errno));

    //
    // If execv fails, sink here.
    //
    exit(1);
  }

  return LMON_EINVAL;
}

//////////////////////////////////////////////////////////////////////////////////
//
// ** LAUNCHMON FRONTEND PUBLIC INTERFACE **
//
//
//

//! lmon_rc_e LMON_fe_init ( int LMON_VERSION )
/*!

    Please refer to the manpage

*/
extern "C" lmon_rc_e LMON_fe_init(int ver) {
  int i;

  if (ver != LMON_return_ver()) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "LMON FE API version mismatch");

    return LMON_EINVAL;
  }

#if VERBOSE
  /* This will enable debug print for security handshake
   * The security messages will not go to the errorCB
   */
  handshake_enable_debug_prints(stdout);
#endif

  set_client_name(LMON_FE_MSG_PREFIX);
  std::string os_isa_str = TARGET_OS_ISA_STRING;
  resmanager.init(os_isa_str);

  for (i = 0; i < MAX_LMON_SESSION; ++i) {
    /*
     * We initialize eventMutex and condition variable separately since this
     * lock
     * should be persistent throughout the exectuion unlike other
     * pthread primitives.
     */
    pthread_mutex_init(&(sess.sessionDescArray[i].watchdogThr.eventMutex),
                       NULL);
    pthread_cond_init(&(sess.sessionDescArray[i].watchdogThr.condVar), NULL);
  }

  for (i = 0; i < MAX_LMON_SESSION; ++i) {
    /*
     * Being paranoid on data race conditions...
     */
    pthread_mutex_lock(&(sess.sessionDescArray[i].watchdogThr.eventMutex));
    if (LMON_init_sess(&(sess.sessionDescArray[i])) != 0) {
      pthread_mutex_unlock(&(sess.sessionDescArray[i].watchdogThr.eventMutex));

      return LMON_EINIT;
    }
    pthread_mutex_unlock(&(sess.sessionDescArray[i].watchdogThr.eventMutex));
  }

  sess.sessionPtrIndex = 0;

  //
  // calling pthread_atfork so that forked process won't inherit
  // active watchdog threads. This is possible if the client
  // is requesting more than one active sessions.
  // Apparently, you don't want the
  // launchmon engine to have those watchdog threads...
  //
  if ((pthread_atfork(NULL, NULL, LMON_child_fork_handler)) != 0)
    return LMON_ESYS;

  return LMON_OK;
}

//! lmon_rc_e LMON_fe_createsession(int* sessionHandle)
/*!

    Please refer to the manpage

*/
extern "C" lmon_rc_e LMON_fe_createSession(int *sessionHandle) {
  char portinfo[10];
  char rannum[LMON_KEY_LENGTH];
  gcry_error_t gcrc;
  lmon_session_desc_t *mydesc;
  lmon_rc_e lrc = LMON_EINVAL;
  lmon_daemon_env_t fe_listensock_info_secchk[4];
  lmon_daemon_env_t fe_listensock_info_secchk_mw[4];

  (*sessionHandle) = sess.sessionPtrIndex;
  mydesc = &(sess.sessionDescArray[(*sessionHandle)]);

  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if ((mydesc->registered) == LMON_FALSE) {
    int rc;
    int i;

    for (i = 0; i < 3; i++) {
      //
      // opens up a TCP socket, binding it with a random port and listening
      // in on it. This is per-session server socket.
      //
      // 0: FE-BE
      // 1: FE-MW
      // 2: FE-Engine
      //
      if (LMON_openBindAndListen(&(mydesc->commDesc[i].sessionListenSockFd)) !=
          LMON_OK) {
        LMON_say_msg(LMON_FE_MSG_PREFIX, true, "LMON_openBindAndListen Failed");

        pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

        return LMON_ESYS;
      }

      socklen_t len = sizeof(mydesc->commDesc[i].servAddr);
      if ((rc = getsockname(mydesc->commDesc[i].sessionListenSockFd,
                            (struct sockaddr *)&(mydesc->commDesc[i].servAddr),
                            &len)) < 0) {
        LMON_say_msg(LMON_FE_MSG_PREFIX, true, "getsockname call failed");

        pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

        return LMON_ESYS;
      }

      if ((inet_ntop(AF_INET, &(mydesc->commDesc[i].servAddr.sin_addr),
                     mydesc->commDesc[i].ipInfo, MAX_LMON_STRING)) == NULL) {
        LMON_say_msg(LMON_FE_MSG_PREFIX, true, "inet_ntop call failed");

        pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

        return LMON_ESYS;
      }
    }  // for ( i = 0; i < 3; i++ )

    //
    // a random seed for the random number generator
    //
    srand(time(NULL));

    //
    // randomID is a random number and shared_key is a encryption key,
    // both of which get propagated to daemons via an environment variable:
    // RM propagates this to daemons and hence it should be secure.
    //
    mydesc->randomID = rand();
    bzero((void *)mydesc->shared_key, LMON_KEY_LENGTH);
    sprintf(mydesc->shared_key, "%d", rand());

    //
    // open a cipher: BLOWFISH Algorithm on 128 bit key
    //
    if ((gcrc = gcry_cipher_open(&(mydesc->cipher_hndl), GCRY_CIPHER_BLOWFISH,
                                 GCRY_CIPHER_MODE_ECB, 0)) !=
        GPG_ERR_NO_ERROR) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, true, "gcry_cipher_open call failed: %s",
                   gcry_strerror(gcrc));

      pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

      return LMON_ESYS;
    }

    //
    // setting a 128 bit key
    //
    if ((gcrc = gcry_cipher_setkey(mydesc->cipher_hndl, mydesc->shared_key,
                                   LMON_KEY_LENGTH)) != GPG_ERR_NO_ERROR) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "gcry_cipher_setkey call failed: %s", gcry_strerror(gcrc));

      pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

      return LMON_ESYS;
    }

#if VERBOSE
    LMON_say_msg(LMON_FE_MSG_PREFIX, false, "GCRYPT shared_key: %s",
                 mydesc->shared_key);
    LMON_say_msg(LMON_FE_MSG_PREFIX, false, "GCRYPT setkey, %x:%x:%x:%x",
                 *(int *)(mydesc->shared_key), *(int *)(mydesc->shared_key + 4),
                 *(int *)(mydesc->shared_key + 8),
                 *(int *)(mydesc->shared_key + 12));
#endif

    //
    // turn on a flag saying "session occupied"
    //
    mydesc->registered = LMON_TRUE;

    //
    // envVar to communicate neccessary info to the BE master
    // and the MW master
    // LMON_FE_ADDR_ENVNAME="LMON_FE_WHERETOCONNECT_ADDR"
    // LMON_FE_PORT_ENVNAME="LMON_FE_WHERETOCONNECT_PORT"
    // LMON_SHRD_SEC_ENVNAME="LMON_SHARED_SECRET"
    // LMON_SEC_CHK_ENVNAME="LMON_SEC_CHK"
    //

    //
    // Front-end's IP information
    //
    fe_listensock_info_secchk[0].envName = strdup(LMON_FE_ADDR_ENVNAME);
    fe_listensock_info_secchk[0].envValue =
        strdup(mydesc->commDesc[fe_be_conn].ipInfo);
    fe_listensock_info_secchk[0].next = NULL;

    //
    // Front-end's Port information
    //
    fe_listensock_info_secchk[1].envName = strdup(LMON_FE_PORT_ENVNAME);
    sprintf(
        portinfo, "%d",
        (unsigned short)ntohs(mydesc->commDesc[fe_be_conn].servAddr.sin_port));
    fe_listensock_info_secchk[1].envValue = strdup(portinfo);
    fe_listensock_info_secchk[1].next = NULL;

    //
    // Front-end's random session ID for secure connection
    //
    fe_listensock_info_secchk[2].envName = strdup(LMON_SEC_CHK_ENVNAME);
    sprintf(rannum, "%d", mydesc->randomID);
    fe_listensock_info_secchk[2].envValue = strdup(rannum);
    fe_listensock_info_secchk[2].next = NULL;

    //
    // Shared Key for secure connection
    //
    fe_listensock_info_secchk[3].envName = strdup(LMON_SHRD_SEC_ENVNAME);
    fe_listensock_info_secchk[3].envValue = strdup(mydesc->shared_key);
    fe_listensock_info_secchk[3].next = NULL;

    //
    // registering above info to the environment variable list
    //
    LMON_fe_putToDaemonEnv(&(mydesc->daemonEnvList[0]),
                           fe_listensock_info_secchk, 4);

    //
    // release memory
    //
    for (i = 0; i < 4; ++i) {
      free(fe_listensock_info_secchk[i].envName);
      free(fe_listensock_info_secchk[i].envValue);
    }

    if (getenv("LMON_DEBUG_BES")) {
      //
      // backend daemon debugging support
      //
      lmon_daemon_env_t de[2];
      de[0].envName = strdup("LMON_DEBUG_BES");
      de[0].envValue = strdup("yes");
      de[0].next = NULL;
      de[1].envName = strdup("DISPLAY");
      de[1].envValue = strdup(getenv("DISPLAY"));
      de[1].next = NULL;
      LMON_fe_putToDaemonEnv(&(mydesc->daemonEnvList[0]), de, 2);
      //
      // release memory
      //
      free(de[0].envName);
      free(de[0].envValue);
      free(de[1].envName);
      free(de[1].envValue);
    }

    fe_listensock_info_secchk_mw[0].envName = strdup(LMON_FE_ADDR_ENVNAME);
    fe_listensock_info_secchk_mw[0].envValue =
        strdup(mydesc->commDesc[fe_mw_conn].ipInfo);
    fe_listensock_info_secchk_mw[0].next = NULL;

    fe_listensock_info_secchk_mw[1].envName = strdup(LMON_FE_PORT_ENVNAME);
    sprintf(
        portinfo, "%d",
        (unsigned short)ntohs(mydesc->commDesc[fe_mw_conn].servAddr.sin_port));
    fe_listensock_info_secchk_mw[1].envValue = strdup(portinfo);
    fe_listensock_info_secchk_mw[1].next = NULL;

    fe_listensock_info_secchk_mw[2].envName = strdup(LMON_SEC_CHK_ENVNAME);
    fe_listensock_info_secchk_mw[2].envValue = strdup(rannum);
    fe_listensock_info_secchk_mw[2].next = NULL;

    fe_listensock_info_secchk_mw[3].envName = strdup(LMON_SHRD_SEC_ENVNAME);
    fe_listensock_info_secchk_mw[3].envValue = strdup(mydesc->shared_key);
    fe_listensock_info_secchk_mw[3].next = NULL;

    //
    // registering above information into the MW environment variable list
    //
    LMON_fe_putToDaemonEnv(&(mydesc->daemonEnvList[1]),
                           fe_listensock_info_secchk_mw, 4);

    //
    // release memory
    //
    for (i = 0; i < 4; ++i) {
      free(fe_listensock_info_secchk_mw[i].envName);
      free(fe_listensock_info_secchk_mw[i].envValue);
    }

    if (getenv("LMON_DEBUG_MWS")) {
      //
      // middleware deamon debugging support
      //
      lmon_daemon_env_t de[1];
      de[0].envName = strdup("LMON_DEBUG_MWS");
      de[0].envValue = strdup("yes");
      de[0].next = NULL;
      de[1].envName = strdup("DISPLAY");
      de[1].envValue = strdup(getenv("DISPLAY"));
      de[1].next = NULL;
      LMON_fe_putToDaemonEnv(&(mydesc->daemonEnvList[1]), de, 2);
      //
      // release memory
      //
      free(de[0].envName);
      free(de[0].envValue);
      free(de[1].envName);
      free(de[1].envValue);
    }

    sess.sessionPtrIndex++;  // increment sessionPtrIndex to point to the next
                             // available session resource element

    if ((sess.sessionPtrIndex / MAX_LMON_SESSION) == 1) {
      //
      // we're wrapped around!
      //
      sess.sessionPtrIndex %= MAX_LMON_SESSION;
      if (sess.sessionDescArray[sess.sessionPtrIndex].registered == LMON_TRUE) {
        LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                     "the current backend session was granted, but the next "
                     "session can be a problem");
        LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                     "the session descriptor to which the incremented "
                     "sessionPtrIndex points has already been registered");
        LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                     "Too many active sessions going on?");
        LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                     "the next call must wait until one of the active session "
                     "descriptors is freed");

        lrc = LMON_EINVAL;
      }
    } else {
      lrc = LMON_OK;
    }
  } else {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "the session descriptor has already been registered");
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "Too many active sessions going on?");
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "a new session request cannot be granted until one of the "
                 "active sessions is freed");

    lrc = LMON_EINVAL;
  }

  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return lrc;
}

//! lmon_rc_e LMON_fe_regPackForFeToBe
/*!

    Please refer to the manpage

*/
extern "C" lmon_rc_e LMON_fe_regPackForFeToBe(
    int sessionHandle,
    int (*packFebe)(void *udata, void *msgbuf, int msgbufmax, int *msgbuflen)) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e rc = LMON_OK;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");

    return LMON_EBDARG;
  }

  if (packFebe == NULL) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "packFebe is null!");
    return LMON_EBDARG;
  }

  mydesc = &(sess.sessionDescArray[(sessionHandle)]);

  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job killed?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  if (mydesc->spawned == LMON_FALSE) {
    if (mydesc->pack != NULL) {
      LMON_say_msg(
          LMON_FE_MSG_PREFIX, false,
          "pack was already registered. replacing it with the new func...");
    }

    mydesc->pack = packFebe;
  } else {
    rc = LMON_EINVAL;
  }
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return rc;
}

//! lmon_rc_e LMON_fe_regUnpackForBeToFe
/*!

    Please refer to the manpage

*/
extern "C" lmon_rc_e LMON_fe_regUnpackForBeToFe(
    int sessionHandle,
    int (*unpackBefe)(void *udatabuf, int udatabuflen, void *udata)) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e rc = LMON_OK;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");

    return LMON_EBDARG;
  }

  if (unpackBefe == NULL) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "unpackBebe is null!");

    return LMON_EBDARG;
  }

  mydesc = &(sess.sessionDescArray[(sessionHandle)]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job killed?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  if (mydesc->spawned == LMON_FALSE) {
    if (mydesc->unpack != NULL) {
      LMON_say_msg(
          LMON_FE_MSG_PREFIX, false,
          "unpack was already registered. replacing it with the new func...");
    }
    mydesc->unpack = unpackBefe;
  } else {
    rc = LMON_EINVAL;
  }
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return rc;
}

//! lmon_rc_e LMON_fe_regPackForFeToMw
/*!

    Please refer to the manpage

*/
lmon_rc_e LMON_fe_regPackForFeToMw(int sessionHandle,
                                   int (*packFemw)(void *udata, void *msgbuf,
                                                   int msgbufmax,
                                                   int *msgbuflen)) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e rc = LMON_OK;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");

    return LMON_EBDARG;
  }

  if (packFemw == NULL) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "packFemw is null!");

    return LMON_EBDARG;
  }

  mydesc = &(sess.sessionDescArray[(sessionHandle)]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job killed?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  if (mydesc->spawned == LMON_FALSE) {
    if (mydesc->mw_pack != NULL) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                   "middleware pack was already registered. replacing it with "
                   "the new func...");
    }

    mydesc->mw_pack = packFemw;
  } else {
    rc = LMON_EINVAL;
  }
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return rc;
}

//! lmon_rc_e LMON_fe_regUnpackForMwToFe
/*!

    Please refer to the manpage

*/
lmon_rc_e LMON_fe_regUnpackForMwToFe(int sessionHandle,
                                     int (*unpackMwfe)(void *udatabuf,
                                                       int udatabuflen,
                                                       void *udata)) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e rc = LMON_OK;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");

    return LMON_EBDARG;
  }

  if (unpackMwfe == NULL) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "unpackMwbe is null!");

    return LMON_EBDARG;
  }

  mydesc = &(sess.sessionDescArray[(sessionHandle)]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job killed?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  if (mydesc->spawned == LMON_FALSE) {
    if (mydesc->mw_unpack != NULL) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                   "middleware unpack was already registered. replacing it "
                   "with the new func...");
    }
    mydesc->mw_unpack = unpackMwfe;
  } else {
    rc = LMON_EINVAL;
  }
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return rc;
}

//! lmon_rc_e LMON_fe_putToBeDaemonEnv
/*!

    Please refer to the manpage

*/
extern "C" lmon_rc_e LMON_fe_putToBeDaemonEnv(int sessionHandle,
                                              lmon_daemon_env_t *dmonEnv,
                                              int numElem) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e rc;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");
    return LMON_EBDARG;
  }

  rc = LMON_OK;
  mydesc = &(sess.sessionDescArray[(sessionHandle)]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job killed?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  if (mydesc->spawned == LMON_FALSE) {
    rc = LMON_fe_putToDaemonEnv(&(mydesc->daemonEnvList[0]), dmonEnv, numElem);
  } else {
    rc = LMON_EINVAL;
  }

  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
  return rc;
}

//! lmon_rc_e LMON_fe_putToMwDaemonEnv
/*!

    Please refer to the manpage

*/
lmon_rc_e LMON_fe_putToMwDaemonEnv(int sessionHandle,
                                   lmon_daemon_env_t *dmonEnv, int numElem) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e rc;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");
    return LMON_EBDARG;
  }

  rc = LMON_OK;
  mydesc = &(sess.sessionDescArray[(sessionHandle)]);

  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job killed?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  if (mydesc->spawned == LMON_FALSE) {
    rc = LMON_fe_putToDaemonEnv(&(mydesc->daemonEnvList[1]), dmonEnv, numElem);
  } else {
    rc = LMON_EINVAL;
  }

  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
  return rc;
}

//! lmon_rc_e LMON_fe_sendUsrData
/*!

    Please refer to the manpage

*/
lmon_rc_e LMON_fe_sendUsrDataBe(int sessionHandle, void *febe_data) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e rc;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");
    return LMON_EBDARG;
  }
  mydesc = &(sess.sessionDescArray[(sessionHandle)]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  rc = LMON_fe_handleFeBeUsrData(sessionHandle, febe_data);
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return rc;
}

//! lmon_rc_e LMON_fe_recvUsrDataBE
/*!

    Please refer to the manpage

*/
lmon_rc_e LMON_fe_recvUsrDataBe(int sessionHandle, void *befe_data) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e rc;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");
    return LMON_EBDARG;
  }
  mydesc = &(sess.sessionDescArray[(sessionHandle)]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  rc = LMON_fe_handleBeFeUsrData(sessionHandle, befe_data);
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return rc;
}

//! lmon_rc_e LMON_fe_sendUsrData
/*!

    Please refer to the manpage

*/
lmon_rc_e LMON_fe_sendUsrDataMw(int sessionHandle, void *femw_data) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e rc;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");
    return LMON_EBDARG;
  }
  mydesc = &(sess.sessionDescArray[(sessionHandle)]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  rc = LMON_fe_handleFeMwUsrData(sessionHandle, femw_data);
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return rc;
}

//! lmon_rc_e LMON_fe_recvUsrDataBE
/*!

    Please refer to the manpage

*/
lmon_rc_e LMON_fe_recvUsrDataMw(int sessionHandle, void *mwfe_data) {
  lmon_session_desc_t *mydesc;
  lmon_rc_e rc;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");
    return LMON_EBDARG;
  }
  mydesc = &(sess.sessionDescArray[(sessionHandle)]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  rc = LMON_fe_handleMwFeUsrData(sessionHandle, mwfe_data);
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return rc;
}

//! lmon_rc_e LMON_fe_detach
/*!

    Please refer to the manpage

*/
extern "C" lmon_rc_e LMON_fe_detach(int sessionHandle) {
  lmonp_t msg;
  lmon_session_desc_t *mydesc;
  int numbytes;
  int rc;
  struct timespec ts;
  char *tout = NULL;
  lmon_rc_e lrc = LMON_OK;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");

    return LMON_EBDARG;
  }

  mydesc = &sess.sessionDescArray[sessionHandle];
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if ((mydesc->registered == LMON_FALSE) || (mydesc->detached == LMON_TRUE) ||
      (mydesc->killed == LMON_TRUE)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                 "the given session is invalid, the job/daemons already "
                 "killed/detached?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  init_msg_header(&msg);
  msg.msgclass = lmonp_fetofe;
  msg.type.fetofe_type = lmonp_detach;

  numbytes = write_lmonp_long_msg(
      mydesc->commDesc[fe_engine_conn].sessionAcceptSockFd, &msg, sizeof(msg));

  if (numbytes != sizeof(msg)) return LMON_EINVAL;

  tout = getenv("LMON_FE_DMONCTL_TIMEOUT");
  clock_gettime(CLOCK_REALTIME, &ts);

  if (tout && ((atoi(tout) > 0) && (atoi(tout) <= MAX_TIMEOUT)))
    ts.tv_sec += atoi(tout);
  else
    ts.tv_sec += DFLT_FE_ENGINE_TOUT;

  while (mydesc->detached != LMON_TRUE) {
    /*
     * This must be signaled by "lmonp_detach_done" event
     */
    rc = pthread_cond_timedwait(&(mydesc->watchdogThr.condVar),
                                &(mydesc->watchdogThr.eventMutex), &ts);

    if (mydesc->detached != LMON_TRUE) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                   "the main thread woke up out of LMON_fe_detach, but the "
                   "required condition is not met");
      LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                   "probably the engine had retired");
    }

    if (rc == ETIMEDOUT) {
      lrc = LMON_ETOUT;
      break;
    } else if (rc != 0) {
      lrc = LMON_EINVAL;
      break;
    }
  }

  /*
   * at this point all locks must have been released
   *
   */
  // LMON_destroy_sess (mydesc);
  // LMON_init_sess (mydesc);
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return lrc;
}

//! lmon_rc_e LMON_fe_kill
/*!

    Please refer to the manpage

*/
lmon_rc_e LMON_fe_kill(int sessionHandle) {
  lmonp_t msg;
  lmon_session_desc_t *mydesc;
  int numbytes;
  int rc;
  struct timespec ts;
  char *tout = NULL;
  lmon_rc_e lrc = LMON_OK;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "the given session is invalid");

    return LMON_EBDARG;
  }

  mydesc = &sess.sessionDescArray[sessionHandle];
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if ((mydesc->registered == LMON_FALSE) || (mydesc->detached == LMON_TRUE) ||
      (mydesc->killed == LMON_TRUE)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "the given session is invalid, the job/daemons already "
                 "killed/detached?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  init_msg_header(&msg);
  msg.msgclass = lmonp_fetofe;
  msg.type.fetofe_type = lmonp_kill;

  numbytes = write_lmonp_long_msg(
      mydesc->commDesc[fe_engine_conn].sessionAcceptSockFd, &msg, sizeof(msg));
  if (numbytes != sizeof(msg)) return LMON_EINVAL;

  tout = getenv("LMON_FE_DMONCTL_TIMEOUT");
  clock_gettime(CLOCK_REALTIME, &ts);

  if (tout && ((atoi(tout) > 0) && (atoi(tout) <= MAX_TIMEOUT)))
    ts.tv_sec += atoi(tout);
  else
    ts.tv_sec += DFLT_FE_ENGINE_TOUT;

  while (mydesc->killed != LMON_TRUE) {
    /*
     * This must be signaled by "lmonp_kill_done" event
     */
    rc = pthread_cond_timedwait(&(mydesc->watchdogThr.condVar),
                                &(mydesc->watchdogThr.eventMutex), &ts);

    if (mydesc->killed != LMON_TRUE) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                   "the main thread woke up out of LMON_fe_kill, but the "
                   "required condition is not met");
      LMON_say_msg(
          LMON_FE_MSG_PREFIX, false,
          "probably the engine had retired before getting the kill message");
    }

    if (rc == ETIMEDOUT) {
      lrc = LMON_ETOUT;
      break;
    } else if (rc != 0) {
      lrc = LMON_EINVAL;
      break;
    }
  }

  /*
   * at this point all locks must have been released
   *
   */
  LMON_destroy_sess(mydesc);
  LMON_init_sess(mydesc);
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return lrc;
}

//! lmon_rc_e LMON_fe_shutdownDaemons
/*!

    Please refer to the manpage

*/
lmon_rc_e LMON_fe_shutdownDaemons(int sessionHandle) {
  lmonp_t msg;
  lmon_session_desc_t *mydesc;
  int numbytes;
  int rc;
  struct timespec ts;
  char *tout = NULL;
  lmon_rc_e lrc = LMON_OK;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");
    return LMON_EBDARG;
  }

  mydesc = &sess.sessionDescArray[sessionHandle];
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job killed?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  init_msg_header(&msg);
  msg.msgclass = lmonp_fetofe;
  msg.type.fetofe_type = lmonp_shutdownbe;

  numbytes = write_lmonp_long_msg(
      mydesc->commDesc[fe_engine_conn].sessionAcceptSockFd, &msg, sizeof(msg));

  if (numbytes != sizeof(msg)) return LMON_EINVAL;

  tout = getenv("LMON_FE_DMONCTL_TIMEOUT");
  clock_gettime(CLOCK_REALTIME, &ts);

  if (tout && ((atoi(tout) > 0) && (atoi(tout) <= MAX_TIMEOUT)))
    ts.tv_sec += atoi(tout);
  else
    ts.tv_sec += DFLT_FE_ENGINE_TOUT;

  while (mydesc->detached != LMON_TRUE) {
    /*
     * This must be signaled by "lmonp_detach_done" event
     */
    rc = pthread_cond_timedwait(&(mydesc->watchdogThr.condVar),
                                &(mydesc->watchdogThr.eventMutex), &ts);
    if (mydesc->detached != LMON_TRUE) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                   "the main thread woke up, but the associated boolean "
                   "predicate is still false");
    }

    if (rc == ETIMEDOUT) {
      lrc = LMON_ETOUT;
      break;
    } else if (rc != 0) {
      lrc = LMON_EINVAL;
      break;
    }
  }

  /*
   * at this point all locks must have been released
   *
   */
  LMON_destroy_sess(mydesc);
  LMON_init_sess(mydesc);
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return lrc;
}

//! lmon_rc_e LMON_fe_getProctableSize
/*!

    Please refer to the manpage

*/
extern "C" lmon_rc_e LMON_fe_getProctableSize(int sessionHandle,
                                              unsigned int *size) {
  lmon_session_desc_t *mydesc;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "an argument is invalid");

    return LMON_EBDARG;
  }

  mydesc = &(sess.sessionDescArray[(sessionHandle)]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job killed?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  if (mydesc->spawned == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "has this session already finished?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EDUNAV;
  }

  if (!(mydesc->proctab_msg)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "per-session proctab_msg is null!  ");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EDUNAV;
  }

  if (mydesc->proctab_msg->sec_or_jobsizeinfo.num_tasks < LMON_NTASKS_THRE) {
    (*size) = (unsigned int)(mydesc->proctab_msg->sec_or_jobsizeinfo.num_tasks);
  } else {
    (*size) = (unsigned int)(mydesc->proctab_msg->long_num_tasks);
  }
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return LMON_OK;
}

//! lmon_rc_e LMON_fe_getProctable
/*!

    Please refer to the manpage

*/
extern "C" lmon_rc_e LMON_fe_getProctable(int sessionHandle,
                                          MPIR_PROCDESC_EXT *proctable,
                                          unsigned int *size,
                                          unsigned int maxlen) {
  unsigned int i;
  char *traverse;
  char *strtab_offset;

  lmon_session_desc_t *mydesc;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION) ||
      (maxlen < 0)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "an argument is invalid");

    return LMON_EBDARG;
  }

  mydesc = &(sess.sessionDescArray[(sessionHandle)]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job killed?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  if (mydesc->spawned == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "has this session already finished?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EDUNAV;
  }

  if (!(mydesc->proctab_msg)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "per-session proctab_msg is null!  ");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EDUNAV;
  }

  if (!(proctable)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "proctable is null ! ");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EDUNAV;
  }

  traverse = get_lmonpayload_begin((mydesc->proctab_msg));
  if (!traverse) {
    return LMON_EDUNAV;
  }
  if (mydesc->proctab_msg->sec_or_jobsizeinfo.num_tasks < LMON_NTASKS_THRE) {
    (*size) = (unsigned int)(mydesc->proctab_msg->sec_or_jobsizeinfo.num_tasks);
  } else {
    (*size) = (unsigned int)(mydesc->proctab_msg->long_num_tasks);
  }

  strtab_offset = get_strtab_begin(mydesc->proctab_msg);

  for (i = 0; (i < (*size)) && i < maxlen; i++) {
    //
    // fetch a host_name
    // possible error check what  ( strtab_offset + ( *((unsigned
    // int*)traverse)) ) contains
    proctable[i].pd.host_name =
        strdup(strtab_offset + (*((unsigned int *)traverse)));
    traverse += sizeof(unsigned int);

    //
    // fetch a exectuable_name
    //
    proctable[i].pd.executable_name =
        strdup(strtab_offset + (*((unsigned int *)traverse)));
    traverse += sizeof(unsigned int);

    //
    // fetch a pid
    //
    memcpy(&(proctable[i].pd.pid), (void *)traverse, sizeof(int));
    traverse += sizeof(int);

    //
    // fetch a mpirank
    //
    memcpy(&(proctable[i].mpirank), (void *)traverse, sizeof(int));
    traverse += sizeof(int);

    memcpy(&(proctable[i].cnodeid), (void *)traverse, sizeof(int));
    traverse += sizeof(int);
  }
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  if ((i == maxlen) && maxlen < (*size)) return LMON_ETRUNC;

  return LMON_OK;
}

//! lmon_rc_e LMON_fe_regErrorCB
/*!
  registers a callback function that gets
  invoked whenever an error message should
  go out.
*/
extern "C" lmon_rc_e LMON_fe_regErrorCB(int (*func)(const char *format,
                                                    va_list ap)) {
  if (func == NULL) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "an argument is invalid");

    return LMON_EBDARG;
  }

  if (errorCB != NULL) {
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, false,
        "previously registered error callback func will be invalidated");
  }

  errorCB = func;

  return LMON_OK;
}

//! lmon_rc_e LMON_fe_regStatusCB
/*!
  registers a callback function that gets
  invoked whenever there is a status change.
*/
extern "C" lmon_rc_e LMON_fe_regStatusCB(int sessionHandle,
                                         int (*func)(int *status)) {
  lmon_session_desc_t *mydesc;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "an argument is invalid");

    return LMON_EBDARG;
  }

  mydesc = &(sess.sessionDescArray[(sessionHandle)]);

  if (func == NULL) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "an argument is invalid");

    return LMON_EBDARG;
  }

  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));

  if (mydesc->statusCB != NULL) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                 "previously registered status cb func is invalidated");
  }

  mydesc->statusCB = func;
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return LMON_OK;
}

//! lmon_rc_e LMON_fe_getStatus
/*!
  returns the status of the session through the status
  argument. It encodes the status using the lowest
  five bits as follows. The problem with this routine is
  that it can't preciously catch some events. The callback
  version is highly recommended.

  lowest bit: session is registered (0) or not (1)
    next bit: back-end daemons are spawned or not
    next bit: middleware daemons are spawned or not
    next bit: engine has detached from the job or not
    next bit: the job has been killed or not
*/
extern "C" lmon_rc_e LMON_fe_getStatus(int sessionHandle, int *status) {
  lmon_session_desc_t *mydesc;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION) ||
      (status == NULL)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "an argument is invalid");

    return LMON_EBDARG;
  }

  mydesc = &(sess.sessionDescArray[(sessionHandle)]);

  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  fe_getStatus(mydesc, status);
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return LMON_OK;
}

//! lmon_rc_e LMON_fe_getResourceHandle
/*!
  Returns the resource handle via handle only after
  LMON_fe_attachAndSpawnDaemons
  or LMON_fe_launchAndSpawnDaemons call has been called successfully.
  For SLURM-based system, the handle is exported through a symbol
  "totalview_jobid"
  whose type is const char* from the srun process.
*/
extern "C" lmon_rc_e LMON_fe_getResourceHandle(int sessionHandle, char *handle,
                                               int *size, int maxstring) {
  lmon_session_desc_t *mydesc;
  char resbuf[PATH_MAX];

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION) ||
      (maxstring < 0)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "an argument is invalid");

    return LMON_EBDARG;
  }

  mydesc = &(sess.sessionDescArray[(sessionHandle)]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job killed?");
    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  if (mydesc->spawned == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "has this session already finished?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EDUNAV;
  }

  if (mydesc->resourceHandle == LMON_INIT) {
    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EDUNAV;
  }

  if (sprintf(resbuf, "%jd",
              static_cast<intmax_t> (mydesc->resourceHandle)) < 0) {
    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_ESYS;
  }

  strncpy(handle, resbuf, maxstring);
  int len = strlen(resbuf) + 1;
  (*size) = (len > maxstring) ? maxstring : len;

  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return LMON_OK;
}

//! lmon_rc_e LMON_fe_getRMInfo ( int sessionHandle, lmon_rm_info_t *info)
/*!
    Please refer to the manpage
*/
extern "C" lmon_rc_e LMON_fe_getRMInfo(int sessionHandle,
                                       lmon_rm_info_t *info) {
  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "the sessionHandle argument is invalid");

    return LMON_EBDARG;
  }

  if (!info) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "the info argument is null");

    return LMON_EBDARG;
  }

  lmon_session_desc_t *mydesc = &(sess.sessionDescArray[sessionHandle]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(
        LMON_FE_MSG_PREFIX, true,
        "the session is invalid, the job killed or uninitialized session?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EINVAL;
  }

  if (mydesc->rm_info.num_supported_types > 0) {
    int memsz = sizeof(rm_catalogue_e) * mydesc->rm_info.num_supported_types;
    info->rm_supported_types = (rm_catalogue_e *)malloc(memsz);
    memcpy(info->rm_supported_types, mydesc->rm_info.rm_supported_types, memsz);
  }

  info->index_to_cur_instance = mydesc->rm_info.index_to_cur_instance;
  info->num_supported_types = mydesc->rm_info.num_supported_types;
  info->rm_launcher_pid = mydesc->rm_info.rm_launcher_pid;

  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  return (info->rm_launcher_pid != -1) ? LMON_OK : LMON_EDUNAV;
}

//! lmon_rc_e LMON_fe_launchAndSpawnDaemons
/*!

    Please refer to the manpage.

*/
extern "C" lmon_rc_e LMON_fe_launchAndSpawnDaemons(
    int sessionHandle, const char *hostname, const char *launcher,
    char *l_argv[], const char *toolDaemon, char *d_argv[], void *febe_data,
    void *befe_data) {
  using namespace std;

  int verbosity_level;
  char *vb;
  lmon_session_desc_t *mydesc;
  lmon_rc_e lrc = LMON_EINVAL;
  string ip_port_pair;
  pid_t remote_login_pid;
  opts_args_t opt;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "an argument is invalid");

    return LMON_EBDARG;
  }

  mydesc = &(sess.sessionDescArray[sessionHandle]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job killed?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  verbosity_level = 0;

  if ((vb = getenv(LMON_VERBOSE_ENVNAME)) != NULL) {
    /*
     * The value specified via the "LMON_VERBOSITY" envVar
     * sets an appropriate verbose level
     */
    verbosity_level = atoi(vb);
  }

  if ((verbosity_level > 3) || (verbosity_level) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                 "verbosity level must be between 0 and 3");
    verbosity_level = 0;
  }

  if ((lrc = LMON_set_options(mydesc, opt, verbosity_level, false, launcher,
                              l_argv, 0, toolDaemon, d_argv)) != LMON_OK) {
    return LMON_EBDARG;
  }

  if ((remote_login_pid = fork()) != 0) {
    //
    // A separate process got spawned, which will either directly
    // execute the launchmon engine or perform a login to the given
    // host to invoke the launchmon engine.
    //
    // To facilitate communications among all components, the FEN API
    // runtime will spawn a watchdog thread (POSIX Thread) that handles
    // asynchronous communications with the engine.
    //
    //
    int thrarg[2];
    pthread_t *mythrHandle = &(mydesc->watchdogThr.fetofeMonitoringThr);

    if ((lrc = LMON_fe_acceptEngine(sessionHandle)) != LMON_OK) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                   "internal LMON_fe_acceptEgine call failed");

      return lrc;
    }

    //
    // This is the front-end LaunchMON Engine pair
    //
    thrarg[0] = mydesc->commDesc[fe_engine_conn].sessionAcceptSockFd;
    thrarg[1] = sessionHandle;

    //
    // LMON_fetofe_watchdog_thread is the thread-entry function
    //
    // It will begin polling the FE-Engine message queue
    //
    if ((pthread_create(mythrHandle, NULL, LMON_fetofe_watchdog_thread,
                        (void *)thrarg)) != 0) {
      return LMON_ESYS;
    }

    //
    // This API call gets blocked until the watchdog thread get notified of
    // spawned event.
    //
    pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
    if (mydesc->spawned != LMON_TRUE) {
      struct timespec ts;
      char *tout = getenv("LMON_FE_ENGINE_TIMEOUT");
      clock_gettime(CLOCK_REALTIME, &ts);
      int prc;

      if (tout && ((atoi(tout) > 0) && (atoi(tout) <= MAX_TIMEOUT)))
        ts.tv_sec += atoi(tout);
      else
        ts.tv_sec += DFLT_FE_ENGINE_TOUT;

      if ((prc = pthread_cond_timedwait(&(mydesc->watchdogThr.condVar),
                                        &(mydesc->watchdogThr.eventMutex),
                                        &ts)) != 0) {
        if (prc == ETIMEDOUT) {
          lrc = LMON_ETOUT;
          LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                       "FE-ENGINE connection timed out: %d",
                       tout ? atoi(tout) : DFLT_FE_ENGINE_TOUT);
        } else {
          lrc = LMON_EBUG;
          LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                       "FE-ENGINE connection failed with an error within "
                       "pthread_cond_timedwait: err %d",
                       prc);
        }

        pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
        return lrc;
      }
    }
    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

    //
    // Once you are here, the watchdog thread should have already
    // updated the mydesc->spawned flag
    //
    if ((lrc = LMON_fe_beHandshakeSequence(sessionHandle,
                                           true, /* this is launch case */
                                           febe_data, befe_data)) != LMON_OK) {
      return lrc;
    }
  } else {
    //
    // This process bootstraps the LaunchMON ENGINE with ssh or rsh
    // or directly exec's the launchMON ENGINE.
    //
    bool isLocal;
    int i;
    list<string> lmonOptArgs;

    isLocal = (hostname == NULL) ? true : false;

    if ((isLocal == false) && getenv("LMON_DEBUG_FE_ENGINE_RSH"))
      LMON_TotalView_debug();

    lmonOptArgs.push_back("--remote");
    lmonOptArgs.push_back(opt.get_my_opt()->remote_info);
    lmonOptArgs.push_back("--lmonsec");
    lmonOptArgs.push_back(opt.get_my_opt()->lmon_sec_info);
    lmonOptArgs.push_back("--daemonpath");
    lmonOptArgs.push_back(opt.get_my_opt()->tool_daemon);

    const list<string> &tool_daemon_opts = opt.get_my_opt()->tool_daemon_opts;
    if (!tool_daemon_opts.empty()) {
      lmonOptArgs.push_back("--daemonopts");

      stringstream ss;
      ss << tool_daemon_opts.size();
      lmonOptArgs.push_back(ss.str());

      lmonOptArgs.insert(lmonOptArgs.end(), tool_daemon_opts.begin(),
                         tool_daemon_opts.end());
    }
    lmonOptArgs.push_back(opt.get_my_opt()->debugtarget);
    lmonOptArgs.push_back("-a");

    for (i = 1; opt.get_my_opt()->remaining[i] != NULL; i++) {
      lmonOptArgs.push_back(opt.get_my_opt()->remaining[i]);
    }

    if (bld_exec_lmon_launch_str(isLocal, hostname, lmonOptArgs, opt) ==
        LMON_EINVAL) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "The program didn't sink with an exec call in "
                   "bld_exec_lmon_launch_str");
      exit(1);
    }
  }

  return lrc;
}

//! lmon_rc_e LMON_fe_attachAndSpawnDaemons
/*!

    Please refer to the manpage.


*/
extern "C" lmon_rc_e LMON_fe_attachAndSpawnDaemons(
    int sessionHandle, const char *hostname, pid_t launcherPid,
    const char *toolDaemon, char *d_argv[], void *febe_data, void *befe_data) {
  using namespace std;

  int verbosity_level;
  char *vb;
  lmon_session_desc_t *mydesc;
  lmon_rc_e lrc = LMON_EINVAL;
  string ip_port_pair;
  pid_t remote_login_pid;
  opts_args_t opt;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");
    return LMON_EBDARG;
  }

  mydesc = &sess.sessionDescArray[sessionHandle];
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "the session is invalid, the job killed?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  verbosity_level = 0;
  if ((vb = getenv(LMON_VERBOSE_ENVNAME)) != NULL) {
    /*
     * The value specified via the "LMON_VERBOSITY" envVar
     * sets an appropriate verbose level
     */
    verbosity_level = atoi(vb);
  }

  if ((verbosity_level > 3) || (verbosity_level) < 0) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "verbosity level must be between 0 to 3");
    verbosity_level = 0;
  }

  if ((lrc = LMON_set_options(mydesc, opt, verbosity_level, true, NULL, NULL,
                              launcherPid, toolDaemon, d_argv)) != LMON_OK) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "LMON_set_option failed");
    return LMON_EBDARG;
  }

  if ((remote_login_pid = fork()) != 0) {
    //
    // A separate process got spawned, which will perform
    // the meat of the launchmon operations on the parallel
    // launcher of interest
    //
    // As an API level, we will spawn a watchdog to deal with comm
    // with this launchmon process
    //
    int thrarg[2];
    pthread_t *mythrHandle = &(mydesc->watchdogThr.fetofeMonitoringThr);

    if ((lrc = LMON_fe_acceptEngine(sessionHandle)) != LMON_OK) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, true, "LMON_fe_acceptEngine failed");

      return lrc;
    }

    thrarg[0] = mydesc->commDesc[fe_engine_conn].sessionAcceptSockFd;
    thrarg[1] = sessionHandle;

    //
    // LMON_fetofe_watchdog_thread is the thread-entry function
    //
    if ((pthread_create(mythrHandle, NULL, LMON_fetofe_watchdog_thread,
                        (void *)thrarg)) != 0) {
      return LMON_ESYS;
    }

    //
    // This API call gets blocked until the watchdog thread get notified of
    // spawned event.
    //
    pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
    if (mydesc->spawned != LMON_TRUE) {
      struct timespec ts;
      char *tout = getenv("LMON_FE_ENGINE_TIMEOUT");
      clock_gettime(CLOCK_REALTIME, &ts);
      int prc;

      if (tout && ((atoi(tout) > 0) && (atoi(tout) <= MAX_TIMEOUT)))
        ts.tv_sec += atoi(tout);
      else
        ts.tv_sec += DFLT_FE_ENGINE_TOUT;

      if ((prc = pthread_cond_timedwait(&(mydesc->watchdogThr.condVar),
                                        &(mydesc->watchdogThr.eventMutex),
                                        &ts)) != 0) {
        if (prc == ETIMEDOUT) {
          lrc = LMON_ETOUT;
          LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                       "FE-ENGINE connection timed out: %d",
                       tout ? atoi(tout) : DFLT_FE_ENGINE_TOUT);
        } else {
          lrc = LMON_EBUG;
          LMON_say_msg(LMON_FE_MSG_PREFIX, false,
                       "FE-ENGINE connection failed with an error within "
                       "pthread_cond_timedwait: err %d",
                       prc);
        }

        pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
        return lrc;
      }
    }
    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

    //
    // Once you are here, the watchdog thread updated the mydesc->spawned flag
    //
    if (mydesc->spawned != LMON_TRUE) {
      return LMON_EBUG;
    }

    if ((lrc = LMON_fe_beHandshakeSequence(sessionHandle,
                                           false, /* this is attach case */
                                           febe_data, befe_data)) != LMON_OK) {
      return lrc;
    }
  } else {
    //
    // This process bootstraps the LaunchMON ENGINE with ssh or rsh
    // or directly exec's the launchMON ENGINE.
    //
    bool isLocal;
    char pidstr[10];
    list<string> lmonOptArgs;

    isLocal = (hostname == NULL) ? true : false;
    if ((isLocal == false) && getenv("LMON_DEBUG_FE_ENGINE_RSH"))
      LMON_TotalView_debug();

    lmonOptArgs.push_back("--remote");
    lmonOptArgs.push_back(opt.get_my_opt()->remote_info);
    lmonOptArgs.push_back("--lmonsec");
    lmonOptArgs.push_back(opt.get_my_opt()->lmon_sec_info);
    lmonOptArgs.push_back("--daemonpath");
    lmonOptArgs.push_back(opt.get_my_opt()->tool_daemon);

    const list<string> &tool_daemon_opts = opt.get_my_opt()->tool_daemon_opts;
    if (!tool_daemon_opts.empty()) {
      lmonOptArgs.push_back("--daemonopts");

      stringstream ss;
      ss << tool_daemon_opts.size();
      lmonOptArgs.push_back(ss.str());

      lmonOptArgs.insert(lmonOptArgs.end(), tool_daemon_opts.begin(),
                         tool_daemon_opts.end());
    }

    lmonOptArgs.push_back("--pid");
    sprintf(pidstr, "%d", opt.get_my_opt()->launcher_pid);
    lmonOptArgs.push_back(pidstr);

    if (bld_exec_lmon_launch_str(isLocal, hostname, lmonOptArgs, opt) ==
        LMON_EINVAL) {
      LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                   "The program didn't sink with an exec call in "
                   "bld_exec_lmon_launch_str");
      exit(1);
    }
  }

  return lrc;
}

//! TODO: launchMwDaemons currently supports limited cases--i.e., one daemon per
//! node
/*! Upon receiving an non-supported case, this call returns LMON_NOTIMPL

*/
extern "C" lmon_rc_e LMON_fe_launchMwDaemons(int sessionHandle,
                                             dist_request_t req[], int nreq,
                                             void *femw_data, void *mwfe_data) {
  lmon_rc_e lrc;
  lmon_session_desc_t *mydesc;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "session is invalid");
    return LMON_EBDARG;
  }

  if (nreq <= 0 || nreq > LMON_N_MW_TYPES) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "invalid nreq. Does nreq exceed the number of different MW "
                 "volume kinds?");
    return LMON_EBDARG;
  }

  mydesc = &sess.sessionDescArray[sessionHandle];

  //
  // acquire the session lock
  //
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));

  //
  // spawner setup and call the spawn method of the spawners
  //
  int i;
  for (i = 0; i < nreq; ++i) {
    //
    // process each distribution requests and
    // push a right kind of spawner into the ordered spawner vector
    //
    lmon_mw_mode_t curmode = req[i].md;
    std::vector<std::string> argvect;

    int j = 0;
    while (req[i].d_argv && req[i].d_argv[j]) {
      argvect.push_back(std::string(req[i].d_argv[j]));
      j++;
    }

    if (IS_MW_COLOC(curmode)) {
      if ((req[i].ndaemon == -1) && (req[i].block == -1) &&
          (req[i].cyclic == -1)) {
        std::vector<std::string> hostvect;
        std::map<std::string, std::vector<MPIR_PROCDESC_EXT *>,
                 lexGraphCmp>::const_iterator miter;
        for (miter = mydesc->pMap.begin(); miter != mydesc->pMap.end();
             miter++) {
          hostvect.push_back(miter->first);
        }
        spawner_base_t *colocSpawner = new spawner_coloc_t(
            mydesc->commDesc[fe_be_conn].sessionAcceptSockFd,
            std::string(req[i].mw_daemon_path), argvect, hostvect);

        mydesc->spawner_vector.push_back(colocSpawner);
      } else {
        lrc = LMON_NOTIMPL;
        goto op_went_bad;
      }
    } else if (IS_MW_EXISTINGALLOC(curmode)) {
      // TODO: not implemented yet
      lrc = LMON_NOTIMPL;
      goto op_went_bad;
    } else if (IS_MW_NEWALLOC(curmode)) {
      // TODO: not implemented yet
      lrc = LMON_NOTIMPL;
      goto op_went_bad;
    } else if (IS_MW_HOSTLIST(curmode)) {
      if ((req[i].ndaemon == -1) && (req[i].block == -1) &&
          (req[i].cyclic == -1)) {
        if (req[i].optkind == hostlists) {
          std::vector<std::string> hostvect;
          char **htrav = req[i].option.hl;
          int k = 0;
          while (htrav && htrav[k]) {
            hostvect.push_back(std::string(htrav[k]));
            k++;
          }

          spawner_base_t *rshSpawner = new spawner_rsh_t(
              std::string(req[i].mw_daemon_path), argvect, hostvect);

          mydesc->spawner_vector.push_back(rshSpawner);
        } else {
          LMON_say_msg(LMON_FE_MSG_PREFIX, true, "Bad optkind: req[%d]", i);
          lrc = LMON_EBDARG;
          goto op_went_bad;
        }
      } else {
        lrc = LMON_NOTIMPL;
        goto op_went_bad;
      }
    } else {
      LMON_say_msg(LMON_FE_MSG_PREFIX, true, "Bad request: req[%d]", i);
      lrc = LMON_EBDARG;
      goto op_went_bad;
    }
  }

  lrc = LMON_fe_mwHandshakeSequence(sessionHandle, femw_data, femw_data);
  if (lrc != LMON_OK) {
    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    goto op_went_bad;
  }

  mydesc->mw_spawned = LMON_TRUE;

  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
  return LMON_OK;

op_went_bad:
  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
  return lrc;
}

extern "C" lmon_rc_e LMON_fe_getMwHostlist(int sessionHandle, char **hostlist,
                                           unsigned int *size,
                                           unsigned int maxlen) {
  lmon_session_desc_t *mydesc;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "an argument is invalid");

    return LMON_EBDARG;
  }

  mydesc = &(sess.sessionDescArray[(sessionHandle)]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job killed?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  if (mydesc->mw_spawned != LMON_TRUE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "middleware daemons have not been sapwned!");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EDUNAV;
  }

  std::vector<std::string> chosts;
  std::vector<std::string>::const_iterator hiter;
  (*size) = 0;
  std::vector<spawner_base_t *>::const_iterator iter;
  for (iter = mydesc->spawner_vector.begin();
       iter != mydesc->spawner_vector.end(); ++iter) {
    (*iter)->combineHosts(chosts);
  }

  (*size) = chosts.size();
  unsigned int i = 0;

  //
  // This builds hostlist in ascending LMON MW rank order
  //
  for (hiter = chosts.begin(); ((hiter != chosts.end()) && i <= maxlen);
       hiter++) {
    //
    // Freeing is upto the caller
    //
    hostlist[i] = strdup((*hiter).c_str());
    i++;
  }

  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));

  if ((i == maxlen) && maxlen < (*size)) return LMON_ETRUNC;

  return LMON_OK;
}

extern "C" lmon_rc_e LMON_fe_getMwHostlistSize(int sessionHandle,
                                               unsigned int *size) {
  lmon_session_desc_t *mydesc;

  if ((sessionHandle < 0) || (sessionHandle > MAX_LMON_SESSION)) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true, "an argument is invalid");

    return LMON_EBDARG;
  }

  mydesc = &(sess.sessionDescArray[(sessionHandle)]);
  pthread_mutex_lock(&(mydesc->watchdogThr.eventMutex));
  if (mydesc->registered == LMON_FALSE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "session is invalid, the job killed?");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EBDARG;
  }

  if (mydesc->mw_spawned != LMON_TRUE) {
    LMON_say_msg(LMON_FE_MSG_PREFIX, true,
                 "middleware daemons have not been sapwned!");

    pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
    return LMON_EDUNAV;
  }

  (*size) = 0;

  std::vector<spawner_base_t *>::const_iterator iter;
  for (iter = mydesc->spawner_vector.begin();
       iter != mydesc->spawner_vector.end(); ++iter) {
    (*size) += (*iter)->get_hosts_vector().size();
  }

  pthread_mutex_unlock(&(mydesc->watchdogThr.eventMutex));
  return LMON_OK;
}

/*
 * ts=2 sw=2 expandtab
 */
