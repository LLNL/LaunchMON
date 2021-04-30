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
 *        Sep 24 2008 DHA: Enforced the error handling semantics
 *                         defined in README.ERROR_HANDLING.
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Jul 30 2006 DHA: Fixed a subtle bug: when thread gets
 *                         an "exit: event while the thread list
 *                         is being traversed in the event manager,
 *                         the argument into check_and_undo_context
 *                         can contain garbage. This occurs very
 *                         rarely. The fix is basically to copy the
 *                         thread id into a local variable before
 *                         the beginning of thread list traversing.
 *        Jul 04 2006 DHA: Added self tracing support
 *        Feb 02 2006 DHA: Created file.
 */

#ifndef SDBG_EVENT_MANAGER_IMPL_HXX
#define SDBG_EVENT_MANAGER_IMPL_HXX 1

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

extern "C" {
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#else
#error sys/types.h is required
#endif

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#else
#error sys/wait.h is required
#endif
}

#if HAVE_MAP
#include <map>
#else
#error map is required
#endif

#if HAVE_VECTOR
#include <vector>
#else
#error vector is required
#endif

#include "sdbg_base_mach.hxx"
#include "sdbg_base_mach_impl.hxx"
#include "sdbg_event_manager.hxx"

////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES: monitor_proc_thread_t
//
///////////////////////////////////////////////////////////////////

//! PUBLIC: monitor_proc_thread_t constructors
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
monitor_proc_thread_t<SDBG_DEFAULT_TEMPLPARAM>::monitor_proc_thread_t() {}

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
monitor_proc_thread_t<SDBG_DEFAULT_TEMPLPARAM>::monitor_proc_thread_t(
    const monitor_proc_thread_t& m) {}

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
monitor_proc_thread_t<SDBG_DEFAULT_TEMPLPARAM>::~monitor_proc_thread_t() {}

//! PUBLIC: monitor_proc_thread_t::wait_for_all
/*!
    this probably needs moving to platform specific area because
    waitpid's behavior varys from platform to platform. In that
    case, monitor_proc_thread_t should set this method as virtual
    and write platform-specific class inherit it.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
bool monitor_proc_thread_t<SDBG_DEFAULT_TEMPLPARAM>::wait_for_all(
    debug_event_t& rc) {
  pid_t rpid;
  int status;
  bool rs = false;

  if ((rpid = waitpid(-1, &status, WNOHANG | WUNTRACED | __WCLONE)) <= 0) {
    rc.set_ev(EV_NOCHILD);
    rc.set_id(rpid);
    return rs;
  }

  rc.set_id(rpid);

  if (WIFEXITED(status)) {
    rc.set_ev(EV_EXITED);
    rc.set_exitcode(WEXITSTATUS(status));
    rc.set_rawstatus(status);
    rs = true;
  } else if (WIFSIGNALED(status)) {
    //
    // Child terminiated with a signal
    //
    rc.set_ev(EV_TERMINATED);
    rc.set_signum(WTERMSIG(status));
    rc.set_rawstatus(status);
    rs = true;
  } else if (WIFSTOPPED(status)) {
    //
    // Child stopped w/ signal
    // Thread note: when a new thread is created, the parent must get
    // SIGTRAP
    rc.set_ev(EV_STOPPED);
    rc.set_signum(WSTOPSIG(status));
    rc.set_rawstatus(status);
    rs = true;
  } else {
    rc.set_rawstatus(status);
  }

  return rs;
}

////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES: event_manager_t
//
///////////////////////////////////////////////////////////////////

//! PUBLIC: constructors
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
event_manager_t<SDBG_DEFAULT_TEMPLPARAM>::event_manager_t() {
  ev_monitor = new monitor_proc_thread_t<SDBG_DEFAULT_TEMPLPARAM>();
}

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
event_manager_t<SDBG_DEFAULT_TEMPLPARAM>::event_manager_t(
    const event_manager_t& e) {
  ev_monitor = e.ev_monitor;
  MODULENAME = e.MODULENAME;
}

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
event_manager_t<SDBG_DEFAULT_TEMPLPARAM>::~event_manager_t() {
  if (ev_monitor) delete ev_monitor;
}

//! PUBLIC: poll_processes
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
bool event_manager_t<SDBG_DEFAULT_TEMPLPARAM>::poll_processes(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p,
    launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>& lm) {
  using namespace std;

  debug_event_t event;
  pid_t rpid;
  launchmon_rc_e rc = LAUNCHMON_OK;
  launchmon_event_e ev;

  if (!ev_monitor->wait_for_all(event))
    goto done;

  if (event.get_id() == p.get_pid(false)) {
    //
    // The target RM_process reported
    //
    p.make_context ( event.get_id());
    ev = lm.decipher_an_event ( p, event );
    rc = lm.invoke_handler ( p, ev, event.get_signum() );
    p.check_and_undo_context ( event.get_id() );
  } else if (event.get_id() == lm.get_toollauncherpid()) {
    // RM_process that launched tool daemons reported
    // error handling semantics for C.2
    //
    if ((event.get_ev() == EV_EXITED)
           || (event.get_ev() == EV_TERMINATED)) {
      // this means that back-end daemons have exited
      // Enforcing C.2 error handling semantics.
      //
      rc = lm.handle_daemon_exit_event(p);
    }
  } else if ( p.get_thrlist().find (event.get_id()) != p.get_thrlist().end()) {
    p.make_context(event.get_id());
    ev = lm.decipher_an_event(p, event);
    rc = lm.invoke_handler (p, ev, event.get_signum());
    p.check_and_undo_context(event.get_id());
  } else {
    //
    // a new process reported -- don't follow
    //
    if (event.get_ev() == EV_STOPPED) {
      p.make_context (event.get_id());
      rc = lm.invoke_handler(p,
                             LM_STOP_NEW_FORKED_PROCESS,
                             event.get_id() );
      p.check_and_undo_context (event.get_id());
    }
    //p.make_context(event.get_id());
    //ev = lm.decipher_an_event(p, event);
    //rc = lm.invoke_handler (p, ev, event.get_signum());
    //lm.handle_thrcreate_trap_event (p);
    //p.check_and_undo_context(event.get_id());
  }

done:
  return ((rc == LAUNCHMON_OK) ? true : false);
}

//! PUBLIC: poll_FE_socket
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
bool event_manager_t<SDBG_DEFAULT_TEMPLPARAM>::poll_FE_socket(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& proc,
    launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>& lm) {
  bool rc = true;

  if (lm.handle_incoming_socket_event(proc) != LAUNCHMON_OK) {
    rc = false;
  }

  return rc;
}

//! PUBLIC: multiplex_events
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
bool event_manager_t<SDBG_DEFAULT_TEMPLPARAM>::multiplex_events(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& proc,
    launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>& lm) {
  bool rc = true;

  rc = poll_FE_socket(proc, lm);
  if (rc) {
    //
    // We only poll when poll_FE_socket returns the true code.
    // poll_FE_socket always returns true for standalone launchmon mode
    //
    rc = poll_processes(proc, lm);
  }

  return (rc);
}
#endif  // SDBG_EVENT_MANAGER_IMPL_HXX

/*
 * ts=2 sw=2 expandtab
 */

