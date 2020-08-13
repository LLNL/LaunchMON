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
 *        Aug 12 2020 DHA: Change resid to 64-bit to support modern RMs
 *                         that use JOBIDs with wider width.
 *        Oct 26 2012 DHA: Removed references to ttracer that has been
 *                         deprecated.
 *        May 11 2010 DHA: Removed gettimeofdayD from this file
 *        Sep 24 2008 DHA: Added handle_daemon_exit_event to support
 *                         better error handling
 *        Sep 22 2008 DHA: Added the last_seen and warm_period members
 *                         to support a two-phase polling scheme in which
 *                         the polling of the FE incoming socket will block
 *                         for 1 milisecond, if no launchmon handler
 *                         event has occurred in the last warm_period
 *                         seconds after last_seen. The default warm_period
 *                         is 10 seconds.
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Mar 13 2007 DHA: Removed writing_pfd, reading pfd member
 *                         and added pipe_t pfd member instead.
 *                         All access except for pipe setting must
 *                         call getConstPfd for safer access.
 *        Jul 03 2006 DHA: Added self tracing support
 *        Jun 06 2006 DHA: Added comments on the file scope
 *                         and the class (launchmon_base_t)
 *                         itself.
 *        Jan 12 2006 DHA: Created file.
 */

#ifndef SDBG_BASE_LAUNCHMON_HXX
#define SDBG_BASE_LAUNCHMON_HXX 1

#include <map>
#include <vector>

extern "C" {
#include <sys/time.h>
}

#include "sdbg_base_mach.hxx"
#include "sdbg_base_tracer.hxx"
#include "sdbg_opt.hxx"
#include "sdbg_self_trace.hxx"

#include <lmon_api/lmon_lmonp_msg.h>
#include <lmon_api/lmon_proctab.h>
#include "lmon_api/lmon_say_msg.hxx"

//! FILE: sdbg_base_launchmon.hxx
/*!
    The file contains the base launchmon class.
    The base launchmon class defines a set of interfaces
    (virtual methods) to be implemented by the
    platform-specific layer. Each interface dictates
    the set of actions when and an important debugging
    event occurs.
*/

//! enumerator launchmon_rc_e
/*!
    Defines a set of launchmon engine handler's return codes.
*/
enum launchmon_rc_e {
  LAUNCHMON_OK = 0,
  LAUNCHMON_BP_PROLOGUE,
  LAUNCHMON_STOP_TRACE,
  LAUNCHMON_FAILED,
  LAUNCHMON_MPIR_DEBUG_ABORT,
  LAUNCHMON_MAINPROG_EXITED
};

//! enumerator launchmon_event_e
/*!
    Defines a set of launchmon event codes.
*/
enum launchmon_event_e {
  LM_HIT_A_BP,
  LM_STOP_AT_LAUNCH_BP,
  LM_STOP_AT_FIRST_EXEC,
  LM_STOP_AT_FIRST_ATTACH,
  LM_STOP_AT_LOADER_BP,
  LM_STOP_AT_THREAD_CREATION,
  LM_STOP_NEW_THREAD_TRACE,
  LM_STOP_NEW_FORKED_PROCESS,
  LM_STOP_NOT_INTERESTED,
  LM_STOP_FOR_DETACH,
  LM_STOP_FOR_KILL,
  LM_REQUEST_NEW_THREAD,
  LM_RELAY_SIGNAL,
  LM_TERMINATED,
  LM_EXITED,
  LM_INVALID
};

//! class launchmon_base_t
/*!
    Abstract class that declares launchmon API as well as
    defines some architecture independent methods.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
class launchmon_base_t {
 public:
  enum engine_state_e {
    mpir_start,
    mpir_null,
    mpir_spawned,
    mpir_abort,
    mpir_unknown
  };

  launchmon_base_t();

  virtual ~launchmon_base_t();

  ////////////////////////////////////////////////////////////
  //
  //  Public Interfaces
  //

  //
  // Accessors
  //
  //
  void set_tracer(tracer_base_t<SDBG_DEFAULT_TEMPLPARAM> *t);
  void set_engine_state(int);
  tracer_base_t<SDBG_DEFAULT_TEMPLPARAM> *get_tracer();
  define_gset(int64_t, resid)
  define_gset(int, pcount)
  define_gset(int, toollauncherpid)
  define_gset(int, FE_sockfd)
  define_gset(bool, API_mode)
  define_gset(double, last_seen)
  define_gset(double, warm_period)
  std::map<std::string, std::vector<MPIR_PROCDESC_EXT *> >
      &get_proctable_copy() {
    return proctable_copy;
  }

  //
  // Method that invokes a corresponding handler based on e
  //
  //
  virtual launchmon_rc_e invoke_handler(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, const launchmon_event_e e,
      const int data);

  //
  // Method that send an event msg to the Front-end runtime
  //
  //
  launchmon_rc_e say_fetofe_msg(lmonp_fe_to_fe_msg_e msg_type);

  //
  // init must be implemented by a derived class, filling all platform
  // specific initialization procedures.
  //
  virtual launchmon_rc_e init(opts_args_t *opt) = 0;

  //
  // Method that translates a low-level event e to launchmon_event_e
  //
  //
  virtual launchmon_event_e decipher_an_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, const debug_event_t &e) = 0;

  //
  // defines a set of actions for attaching to a running job
  virtual launchmon_rc_e handle_attach_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) = 0;
  //
  // defines a set of actions when the launch breakpoint
  // (e.g. MPIR_Breakpoint) is hit
  virtual launchmon_rc_e handle_launch_bp_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) = 0;

  //
  // defines a set of actions when the detach command is
  // issued
  virtual launchmon_rc_e handle_detach_cmd_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) = 0;

  //
  // defines a set of actions when the detach command is
  // issued
  virtual launchmon_rc_e handle_kill_cmd_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) = 0;

  //
  // defines a set of actions when the target process gets
  // initially fork'ed/exec'ed
  virtual launchmon_rc_e handle_trap_after_exec_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) = 0;

  //
  // defines a set of actions when the target process gets
  // initially attached
  virtual launchmon_rc_e handle_trap_after_attach_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) = 0;

  //
  // defines a set of actions when the target process
  // loads/unloads a shared library
  virtual launchmon_rc_e handle_loader_bp_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) = 0;

  //
  // defines a set of actions when the target process exits
  virtual launchmon_rc_e handle_exit_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) = 0;

  //
  // defines a set of actions when the target process gets
  // somehow terminated
  virtual launchmon_rc_e handle_term_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) = 0;

  virtual launchmon_rc_e handle_thrcreate_request(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, int newlwpid) = 0;

  //
  // defines a set of actions when a new thread creation event
  // is notified.
  virtual launchmon_rc_e handle_thrcreate_trap_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) = 0;

  //
  // defines a set of actions when a new thread stop is reported
  // is notified.
  virtual launchmon_rc_e handle_newthread_trace_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) = 0;

  //
  // defines a set of actions when a new process is forked
  // and stopped.
  virtual launchmon_rc_e handle_newproc_forked_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) = 0;

  //
  // all unwonted stop events.
  //
  virtual launchmon_rc_e handle_not_interested_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) = 0;

  //
  // all relay-signal events.
  //
  virtual launchmon_rc_e handle_relay_signal_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, int sig) = 0;

  //
  // ships the RPDTAB to the FE API client
  //
  launchmon_rc_e ship_proctab_msg(lmonp_fe_to_fe_msg_e);

  //
  // ships the resource handle to the FE API client
  //
  launchmon_rc_e ship_resourcehandle_msg(lmonp_fe_to_fe_msg_e, int64_t);

  //
  // ships the rminfo to the FE API client
  //
  launchmon_rc_e ship_rminfo_msg(lmonp_fe_to_fe_msg_e, int, rm_catalogue_e);

  //
  // handle a message received from the FE API client
  //
  launchmon_rc_e handle_incoming_socket_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p);

  //
  // defines a set of actions when the back-end daemons exited
  //
  launchmon_rc_e handle_daemon_exit_event(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p);

  bool validate_mpir_state_transition(int s);

  //
  // Utility method that handles a detach/kill request command
  //
  bool request_detach(process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
                      pcont_req_reason reason);
  bool request_kill(process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
                    pcont_req_reason reason);
  bool request_cont_launch_bp(process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p);

 private:
  bool LEVELCHK(self_trace_verbosity level) {
    return (self_trace_t::self_trace().launchmon_module_trace.verbosity_level >=
            level);
  }

  launchmon_base_t(const launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM> &l);

  launchmon_base_t &operator=(
      const launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM> &rhs);

  //
  // process tracer
  //
  tracer_base_t<SDBG_DEFAULT_TEMPLPARAM> *tracer;

  //
  // mpir tracer state
  //
  engine_state_e engine_state;

  //
  // resource id, for slurm it is what totalview_jobid contains.
  //
  int64_t resid;
  int pcount;
  int toollauncherpid;

  //
  // The member containing all proctable entries
  // std::vector<MPIR_PROCDESC_EXT *> proctable_copy;
  //
  // This table is filled by the platform dependent layer
  //
  std::map<std::string, std::vector<MPIR_PROCDESC_EXT *> > proctable_copy;

  //
  // Unix PIPE with LAUNCHMON FE API STUB
  // deprecating PIPE support in favor of socket support
  // in anticipation for remote FE API
  //
  int FE_sockfd;
  bool API_mode;

  //
  // To support two-phase polling scheme
  //
  double last_seen;
  double warm_period;

  //
  // To support self tracing
  //
  std::string MODULENAME;
};

#endif  // __SDBG_BASE_LAUNCHMON_HXX

/*
 * vi: ts=2 sw=2 expandtab
 */
