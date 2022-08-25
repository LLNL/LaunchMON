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
 *        Jul 31 2012 DHA: Added setoptions/unsetoptions
 *        Oct 26 2010 DHA: Changed method names from insert|pullout_breakpoint
 *                         to enable|disable_breakpoint. Slight linux
 *                         implementation change as part of refactoring in
 *                         the breakpoint_base_t class.
 *        May 08 2008 DHA: Added comments for the "use_cxt" argument
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Mar 31 2006 DHA: Added tracer_read_string.
 *        Mar 30 2006 DHA: Added exception handling support.
 *        Feb 06 2006 DHA: Populated tracer_error_e enumerator.
 *        Jan 10 2006 DHA: Created file.
 */

#ifndef SDBG_BASE_TRACER_HXX
#define SDBG_BASE_TRACER_HXX

#include <iostream>
#include "sdbg_base_bp.hxx"
#include "sdbg_base_exception.hxx"
#include "sdbg_base_mach.hxx"

//! enumerator tracer_error_e
/*!
    Defines a set of tracer's return codes.
    SDBG_TRACE_OK: tracer's operation succeeded.
    SDBG_TRACE_EIO_ERR: io error
    SDBG_TRACE_EACCESS_ERR: access error
    SDBG_TRACE_EACCESS_ERR,
    SDBG_TRACE_ESRCH_ERR,
    SDBG_TRACE_EINVAL_ERR,
    SDBG_TRACE_EPERM_ERR,
    SDBG_TRACE_EFAULT_ERR,
    SDBG_TRACE_EBUSY_ERR,
    SDBG_TRACE_STATE_UNKNOWN,
    SDBG_TRACE_UNIMPLEMENTED,
    SDBG_TRACE_STOPPED,
    SDBG_TRACE_RUNNING,
    SDBG_TRACE_FAILED: failed. catch-all
*/
enum tracer_error_e {
  SDBG_TRACE_OK,
  SDBG_TRACE_EIO_ERR,
  SDBG_TRACE_EACCESS_ERR,
  SDBG_TRACE_ESRCH_ERR,
  SDBG_TRACE_EINVAL_ERR,
  SDBG_TRACE_EPERM_ERR,
  SDBG_TRACE_EFAULT_ERR,
  SDBG_TRACE_EBUSY_ERR,
  SDBG_TRACE_STATE_UNKNOWN,
  SDBG_TRACE_UNIMPLEMENTED,
  SDBG_TRACE_STOPPED,
  SDBG_TRACE_RUNNING,
  SDBG_TRACE_FAILED
};

////////////////////////////////////////////////////////////////////////////
//
//
//

//! class tracer_exception_t : public exception_base_t
/*!
  exception class for tracer layer

*/
class tracer_exception_t : public exception_base_t {
 public:
  tracer_error_e error_code;

  tracer_exception_t() {}
  tracer_exception_t(const char *m, tracer_error_e e) {
    set_message(m);
    error_code = e;
    set_type(std::string("SDBG_TRACER_ERROR"));
    set_fn(std::string(__FILE__));
    set_ln(__LINE__);
  }
  tracer_exception_t(const std::string &m, tracer_error_e e) {
    set_message(m);
    error_code = e;
    set_type(std::string("SDBG_TRACER_ERROR"));
    set_fn(std::string(__FILE__));
    set_ln(__LINE__);
  }

  virtual ~tracer_exception_t() {}
};

////////////////////////////////////////////////////////////////////////////
//
//

//! class tracer_base_t
/*!
    tracer_base_t is a pure abstract class which defines
    virtual methods for process tracing operations.
    Its sub-calsses must implement those methods.

    Template parameters:
    VA:  Virtual_address type or class
    WT:  Word type or class
    IT:  Instruction class
    RTS: Native general register set struct
    FTS: Native FP register set struct
    NT:  Native thread info type
*/

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
class tracer_base_t {
 public:
  tracer_base_t() {}
  tracer_base_t(const tracer_base_t<SDBG_DEFAULT_TEMPLPARAM> &p) {}
  virtual ~tracer_base_t() {}

  ////////////////////////////////////////////////////////////
  //
  //  Abstract Interfaces:
  //
  //  "use_cxt" determines whether the process control operations
  //  apply to the main thread of the target process (use_cxt=false)
  //  or the active target thread context within that process, which
  //  the upper layer can set. This is to simplify the arguments
  //  into the process control operations at the expense of having
  //  the upper layer to set the active target thread in case it
  //  wants to use a "context-sensitive" operation.
  //
  //
  virtual tracer_error_e tracer_setregs(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
      bool use_cxt) = 0;

  virtual tracer_error_e tracer_getregs(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
      bool use_cxt) = 0;

  virtual tracer_error_e tracer_setfpregs(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
      bool use_cxt) = 0;

  virtual tracer_error_e tracer_getfpregs(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
      bool use_cxt) = 0;

  virtual tracer_error_e tracer_read(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, VA addr, void *buf, int size,
      bool use_cxt) = 0;

  virtual tracer_error_e tracer_read_string(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, VA addr, void *buf, int size,
      bool use_cxt) = 0;

  virtual tracer_error_e tracer_get_event_msg(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, VA addr, void *buf,
      bool use_cxt) = 0;

  virtual tracer_error_e tracer_write(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, VA addr, const void *buf,
      int size, bool use_cxt) = 0;

  virtual tracer_error_e tracer_continue(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
      bool use_cxt) = 0;

  virtual tracer_error_e tracer_deliver_signal(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, int sig,
      bool use_cxt) = 0;

  virtual tracer_error_e tracer_stop(process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
                                     bool use_cxt) = 0;

  virtual tracer_error_e tracer_kill(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
      bool use_cxt) = 0;

  virtual tracer_error_e tracer_singlestep(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
      bool use_cxt) = 0;

  virtual tracer_error_e tracer_syscall(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
      bool use_cxt) = 0;

  virtual tracer_error_e tracer_detach(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt) = 0;

  virtual tracer_error_e tracer_setoptions(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt,
      pid_t newtid) = 0;

  virtual tracer_error_e tracer_unsetoptions(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt,
      pid_t newtid) = 0;

  virtual tracer_error_e tracer_attach(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt,
      pid_t newtid) = 0;

  virtual tracer_error_e status(process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
                                bool use_cxt) = 0;

  virtual tracer_error_e tracer_trace_me() = 0;

  virtual tracer_error_e enable_breakpoint(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, breakpoint_base_t<VA, IT> &bp,
      bool use_cxt, bool change_state = true) = 0;

  virtual tracer_error_e disable_breakpoint(
      process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, breakpoint_base_t<VA, IT> &bp,
      bool use_cxt, bool change_state = true) = 0;

  virtual tracer_error_e convert_error_code(int err) = 0;
};

#endif  // SDBG_BASE_TRACER_HXX

/*
 * ts=2 sw=2 expandtab
 */
