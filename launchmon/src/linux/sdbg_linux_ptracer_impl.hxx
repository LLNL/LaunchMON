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
 *        May 02 2018 KMD/ADG: Added aarch64 support
 *        Mar 06 2008 DHA: Added indirection Breakpoint support.
 *                         insert_breakpoint
 *                         and pullout_breakpoint method.
 *        Mar 11 2008 DHA: Added PowerPC support
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Jan 09 2007 DHA: Linux X86_64 Support
 *        Jul 03 2006 DHA: Added self tracing support by means of making
 *                         a common entry point for ptrace (Pptrace)
 *        Mar 31 2006 DHA: Added tracer_read_string
 *        Mar 30 2006 DHA: Added exception handling support
 *        Feb 06 2006 DHA: Convert_error_code support
 *        Jan 10 2006 DHA: Created file.
 */

#ifndef __SDBG_LINUX_PTRACER_IMPL_HXX
#define __SDBG_LINUX_PTRACER_IMPL_HXX

#include "sdbg_std.hxx"

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
#endif

extern "C" {

#include <elf.h>
#include <errno.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <syscall.h>
#include <unistd.h>
}

#include <fstream>
#include <sstream>
#include "sdbg_linux_ptracer.hxx"

////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class linux_ptracer_t)
//
//

//! PUBLIC: linux_ptracer_t()
/*!
    linux_ptracer_t constructor
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::linux_ptracer_t()
    : MODULENAME(self_trace_t::self_trace().tracer_module_trace.module_name) {}

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::linux_ptracer_t(
    const linux_ptracer_t& pt) {
  MODULENAME = pt.MODULENAME;
}

//! PUBLIC: tracer_setregs
/*!
    Methods that sets the general register set
    contained in the process_base_t object. This method
    can throw an exception of linux_tracer_exception_t type.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_setregs(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p,
    bool use_cxt) {
  using namespace std;

  string e;
  string func = "[linux_ptracer_t::tracer_setregs]";
  pid_t tpid = p.get_pid(use_cxt);

  register_set_base_t<GRS, VA, WT>* regset = p.get_gprset(use_cxt);

  if (!regset) {
    e = func + " No register set is found with the thread ";
    throw linux_tracer_exception_t(e, SDBG_TRACE_FAILED);
  }

  regset->set_ptr_to_regset();

#if AARCH64_ARCHITECTURE

  struct iovec iov;
  int ret;

  iov.iov_base = (void*)regset->get_rs_ptr();
  iov.iov_len = regset->size();

  // Write all general purpose registers in one go
  ret = Pptrace(PTRACE_SETREGSET, tpid, (void*)NT_PRSTATUS, (void*)&iov);
  if (ret < 0) {
    e = func + ERRMSG_PTRACE + strerror(errno);
    throw linux_tracer_exception_t(e, convert_error_code(errno));
  }

#else

  WT r;
  VA offset = regset->get_offset_in_user();
  unsigned int num_regs = regset->size_in_word();
  unsigned int i;

  for (i = 0; i < num_regs; i++) {
    if (((1 << i) & regset->get_writable_mask()) == 0) {
      //
      // We only write into what we are allowed to write into.
      //
      regset->inc_ptr_by_word();
      offset += sizeof(WT);
      continue;
    }

    r = Pptrace(PTRACE_POKEUSER, tpid, (void*)offset,
                (void*)*(regset->get_rs_ptr()));

    if (errno) {
      e = func + ERRMSG_PTRACE + strerror(errno);
      throw linux_tracer_exception_t(e, convert_error_code(errno));
    }

    regset->inc_ptr_by_word();
    offset += sizeof(WT);
  }  // for ( i=0; i < num_regs; i++ )

#endif

  return SDBG_TRACE_OK;

}  // tracer_error_e linux_ptracer_t::tracer_setregs

//! PUBLIC: tracer_getregs
/*!
    It returns the general register set through
    the process_base_t object. This method can throw
    an exception of linux_tracer_exception_t type.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_getregs(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p,
    bool use_cxt) {
  using namespace std;

  string e;
  string func = "[linux_ptracer_t::tracer_getregs]";
  pid_t tpid = p.get_pid(use_cxt);

  register_set_base_t<GRS, VA, WT>* regset = p.get_gprset(use_cxt);

  if (!regset) {
    e = func + " No register set is found with the thread ";
    throw linux_tracer_exception_t(e, SDBG_TRACE_FAILED);
  }

  regset->set_ptr_to_regset();

#if AARCH64_ARCHITECTURE

  struct iovec iov;
  int ret;

  iov.iov_base = (void*)regset->get_rs_ptr();
  iov.iov_len = regset->size();

  // Read all general purpose registers in one go
  ret = Pptrace(PTRACE_GETREGSET, tpid, (void*)NT_PRSTATUS, (void*)&iov);
  if (ret < 0) {
    e = func + ERRMSG_PTRACE + strerror(errno);
    throw linux_tracer_exception_t(e, convert_error_code(errno));
  }

#else

  WT r;
  VA offset = regset->get_offset_in_user();
  unsigned int num_regs = regset->size_in_word();
  unsigned int i;

  for (i = 0; i < num_regs; i++) {
    r = Pptrace(PTRACE_PEEKUSER, tpid, (void*)offset, NULL);

    if (errno) {
      e = func + ERRMSG_PTRACE + strerror(errno);
      throw linux_tracer_exception_t(e, convert_error_code(errno));
    }

    regset->write_word_to_ptr(r);
    regset->inc_ptr_by_word();
    offset += sizeof(WT);
  }
#endif

  return SDBG_TRACE_OK;

}  // tracer_error_e linux_ptracer_t::tracer_getfpregs

//! PUBLIC: tracer_getfpregs
/*!
    It returns floating point register set through the
    process_base_t object.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_getfpregs(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p,
    bool use_cxt) {
  using namespace std;

  string e;
  string func = "[linux_ptracer_t::tracer_getfpregs]";
  pid_t tpid = p.get_pid(use_cxt);

  register_set_base_t<FRS, VA, WT>* regset = p.get_fprset(use_cxt);

  if (!regset) {
    e = func + " No register set is found with the thread ";
    throw linux_tracer_exception_t(e, SDBG_TRACE_FAILED);
  }

  regset->set_ptr_to_regset();

#if AARCH64_ARCHITECTURE

  struct iovec iov;
  int ret;

  iov.iov_base = (void*)regset->get_rs_ptr();
  iov.iov_len = regset->size();

  // Read all floating point registers in one go
  ret = Pptrace(PTRACE_GETREGSET, tpid, (void*)NT_FPREGSET, (void*)&iov);
  if (ret < 0) {
    e = func + ERRMSG_PTRACE + strerror(errno);
    throw linux_tracer_exception_t(e, convert_error_code(errno));
  }

#else

  WT r;
  VA offset = regset->get_offset_in_user();
  unsigned int num_regs = regset->size_in_word();
  unsigned int i;

  for (i = 0; i < num_regs; i++) {
    r = Pptrace(PTRACE_PEEKUSER, tpid, (void*)offset, NULL);

    if (errno) {
      e = func + ERRMSG_PTRACE + strerror(errno);
      throw linux_tracer_exception_t(e, convert_error_code(errno));
    }

    regset->write_word_to_ptr(r);
    regset->inc_ptr_by_word();
    offset += sizeof(WT);
  }

#endif

  return SDBG_TRACE_OK;

}  // tracer_error_e linux_ptracer_t::tracer_getfpregs

//! PUBLIC: tracer_setregs
/*!
    It sets floating point register set contained in
    the process_base_t object. Currently, not implemented.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_setfpregs(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p,
    bool use_cxt) {

  return SDBG_TRACE_UNIMPLEMENTED;

}  // tracer_error_e linux_ptracer_t::tracer_setfpregs

//! PUBLIC: tracer_read
/*!
    It reads from the target process, starting from vaddr. It fills
    buf upto whatever the size in buf argument tells to do.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_read(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, VA addr, void* buf, int size,
    bool use_cxt) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_read]";
  pid_t tpid = p.get_pid(use_cxt);
  VA addr_trav = addr;
  VA trav_end = addr + size;
  WT* buf_trav = (WT*)buf;

  for (addr_trav = addr; (addr_trav + sizeof(WT)) <= trav_end;
       addr_trav += sizeof(WT)) {
    r = Pptrace(PTRACE_PEEKDATA, tpid, (void*)addr_trav, 0);

    if (r == -1 && errno != 0) {
      e = func + ERRMSG_PTRACE + strerror(errno);
      throw linux_tracer_exception_t(e, convert_error_code(errno));
    }

    (*buf_trav) = r;
    buf_trav++;
  }

  if (addr_trav != trav_end) {
    r = Pptrace(PTRACE_PEEKDATA, tpid, (void*)addr_trav, 0);

    if (r == -1 && errno != 0) {
      e = func + ERRMSG_PTRACE + strerror(errno);
      throw linux_tracer_exception_t(e, convert_error_code(errno));
    }

    memcpy((void*)buf_trav, (void*)&r, trav_end - addr_trav);
  }

  return SDBG_TRACE_OK;

}  // linux_ptracer_t::tracer_read

//! PUBLIC: tracer_read_string
/*!
    It reads char[] based string from the target process, starting from addr.
    It fills buf upto whatever the size in buf argument tells to do. If the
    size is smaller than the strlen, the resulting string can be
    truncated
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_read_string(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, VA addr, void* buf, int size,
    bool use_cxt) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_read_string]";
  pid_t tpid = p.get_pid(use_cxt);
  VA addr_trav = addr;
  VA trav_end = addr + size;
  WT* buf_trav = (WT*)buf;
  bool end_of_string = false;

  for (addr_trav = addr; (addr_trav + sizeof(WT)) <= trav_end;
       addr_trav += sizeof(WT)) {
    r = Pptrace(PTRACE_PEEKDATA, tpid, (void*)addr_trav, 0);

    if (r == -1 && errno != 0) {
      e = func + ERRMSG_PTRACE + strerror(errno);
      throw linux_tracer_exception_t(e, convert_error_code(errno));
    }

    (*buf_trav) = r;
    buf_trav++;

#if BIT64
    if (!(r & 0xff00000000000000ULL) || !(r & 0x00ff000000000000ULL) ||
        !(r & 0x0000ff0000000000ULL) || !(r & 0x000000ff00000000ULL) ||
        !(r & 0x00000000ff000000ULL) || !(r & 0x0000000000ff0000ULL) ||
        !(r & 0x000000000000ff00ULL) || !(r & 0x00000000000000ffULL)) {
      end_of_string = true;
      break;
    }
#else
    if (!(r & 0xff000000) || !(r & 0x00ff0000) || !(r & 0x0000ff00) ||
        !(r & 0x000000ff)) {
      end_of_string = true;
      break;
    }
#endif
  }  // for (addr_trav ...

  if (addr_trav != trav_end && !end_of_string) {
    r = Pptrace(PTRACE_PEEKDATA, tpid, (void*)addr_trav, 0);

    if (r == -1 && errno != 0) {
      e = func + ERRMSG_PTRACE + strerror(errno);
      throw linux_tracer_exception_t(e, convert_error_code(errno));
    }

    memcpy((void*)buf_trav, (void*)&r, trav_end - addr_trav);
  }

  return SDBG_TRACE_OK;

}  // linux_ptracer_t::tracer_read_string

//! PUBLIC: tracer_get_event_msg
/*!
    Method that fetch information about the last debug event
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_get_event_msg(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, VA addr, void* buf,
    bool use_cxt) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_get_event_msg]";
  pid_t tpid = p.get_pid(use_cxt);

  r = Pptrace((__ptrace_request)PTRACE_GETEVENTMSG, tpid, NULL, buf);
  if (r == -1 && errno != 0) {
    e = func + ERRMSG_PTRACE + strerror(errno);
    throw linux_tracer_exception_t(e, convert_error_code(errno));
  }

  return SDBG_TRACE_OK;
}

//! PUBLIC: tracer_write
/*!
    Method that write into a thread or a process
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_write(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, VA addr, const void* buf,
    int size, bool use_cxt) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_write]";
  pid_t tpid = p.get_pid(use_cxt);
  VA addr_trav = addr;
  VA trav_end = addr + size;
  WT* buf_trav = (WT*)buf;

  for (addr_trav = addr; (addr_trav + sizeof(WT)) <= trav_end;
       addr_trav += sizeof(WT)) {
    r = Pptrace(PTRACE_POKEDATA, tpid, (void*)addr_trav, (void*)*buf_trav);

    if (r == -1 && errno != 0) {
      e = func + ERRMSG_PTRACE + strerror(errno);
      throw linux_tracer_exception_t(e, convert_error_code(errno));
    }

    buf_trav++;

  }  // for

  if (addr_trav != trav_end) {
    r = Pptrace(PTRACE_PEEKDATA, tpid, (void*)addr_trav, 0);
    if (r == -1 && errno != 0) {
      e = func + ERRMSG_PTRACE + strerror(errno);
      throw linux_tracer_exception_t(e, convert_error_code(errno));
    }

    memcpy((void*)&r, (void*)buf_trav, trav_end - addr_trav);

    r = Pptrace(PTRACE_POKEDATA, tpid, (void*)addr_trav, (void*)r);
    if (r == -1 && errno != 0) {
      e = func + ERRMSG_PTRACE + strerror(errno);
      throw linux_tracer_exception_t(e, convert_error_code(errno));
    }
  }  // if ( addr_trav != trav_end )

  return SDBG_TRACE_OK;

}  // linux_ptracer_t::tracer_write

//! PUBLIC: tracer_continue
/*!
    Method that continues a thread or a process
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_continue(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p,
    bool use_cxt) {
  using namespace std;
  WT r;
  tracer_error_e rc = SDBG_TRACE_OK;
  string e;
  string func = "[linux_ptracer_t::tracer_continue]";
  pid_t tpid = p.get_pid(use_cxt);
  errno = 0;

  if ((r = Pptrace(PTRACE_CONT, tpid, 0, 0)) != 0) {
    e = func + ERRMSG_PTRACE + strerror(errno);
    throw linux_tracer_exception_t(e, convert_error_code(errno));
  }

  //
  // We now mark the state of the target thread as LMON_RM_RUNNING
  //
  p.set_lwp_state(LMON_RM_RUNNING, use_cxt);

  return SDBG_TRACE_OK;

}  // linux_ptracer_t::tracer_continue

//! PUBLIC: tracer_deliver_signal
/*!
    Method that continues a thread or a process with a signal
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_deliver_signal(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, int sig,
    bool use_cxt) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_deliver_signal]";
  pid_t tpid = p.get_pid(use_cxt);
  VA s = (VA)sig;

  if ((r = Pptrace(PTRACE_CONT, tpid, 0, (void*)s)) != 0) {
    e = func + ERRMSG_PTRACE + strerror(errno);
    throw linux_tracer_exception_t(e, convert_error_code(errno));
  }

  //
  // We now mark the state of the target thread as LMON_RM_RUNNING
  //
  p.set_lwp_state(LMON_RM_RUNNING, use_cxt);

  return SDBG_TRACE_OK;
}

//! PUBLIC: tracer_stop
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_stop(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, bool use_cxt) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_stop]";
  pid_t tpid = p.get_pid(use_cxt);

  //
  // Trying tkill instead of kill, to send a signal to a thread more
  // reliably
  //
  if ((r = syscall(__NR_tkill, tpid, SIGSTOP)) != 0) {
    return SDBG_TRACE_FAILED;
  }

  //
  // We must not set "stop" state here because it will be marked
  // when the wait call gets the status update.
  //
  return SDBG_TRACE_OK;

}  // linux_ptracer_t::tracer_stop

//! PUBLIC: tracer_kill
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_kill(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p,
    bool use_cxt) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_kill]";
  pid_t tpid = p.get_pid(use_cxt);

  if ((r = Pptrace(PTRACE_KILL, tpid, 0, 0)) != 0) {
    e = func + ERRMSG_PTRACE + strerror(errno);
    throw linux_tracer_exception_t(e, convert_error_code(errno));
  }

  //
  // We must not set "kill" state here because it will be marked
  // when the wait call gets the status update.
  //
  return SDBG_TRACE_OK;
}

//! PUBLIC: tracer_singlestep
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_singlestep(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p,
    bool use_cxt) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_singlestep]";
  pid_t tpid = p.get_pid(use_cxt);

  if ((r = Pptrace(PTRACE_SINGLESTEP, tpid, 0, 0)) != 0) {
    e = func + ERRMSG_PTRACE + strerror(errno);
    throw linux_tracer_exception_t(e, convert_error_code(errno));
  }

  //
  // We now mark the state of the target thread as LMON_RM_RUNNING
  // This will be a short state as another stop state will be recorded
  // as part of singlestep operation via waitpid.
  //
  p.set_lwp_state(LMON_RM_RUNNING, use_cxt);

  return SDBG_TRACE_OK;

}  // linux_ptracer_t::tracer_syscall

//! PUBLIC: tracer_syscall
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_syscall(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p,
    bool use_cxt) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_syscall]";
  pid_t tpid = p.get_pid(use_cxt);

  if ((r = Pptrace(PTRACE_SYSCALL, tpid, 0, 0)) != 0) {
    e = func + ERRMSG_PTRACE + strerror(errno);
    throw linux_tracer_exception_t(e, convert_error_code(errno));
  }

  //
  // We now mark the state of the target thread as LMON_RM_RUNNING
  // This can be a short state as another stop state will be recorded
  // as part of singlestep operation via waitpid.
  //
  p.set_lwp_state(LMON_RM_RUNNING, use_cxt);

  return SDBG_TRACE_OK;

}  // linux_ptracer_t::tracer_singlestep

//! PUBLIC: tracer_detach
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_detach(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, bool use_cxt) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_detach]";
  pid_t tpid = p.get_pid(use_cxt);

  if ((r = Pptrace(PTRACE_DETACH, tpid, 0, 0)) != 0) {
    errno = 0;
    return SDBG_TRACE_FAILED;
  }

  //
  // We now mark the state of the target thread as LMON_RM_RUNNING
  //
  p.set_lwp_state(LMON_RM_RUNNING, use_cxt);

  return SDBG_TRACE_OK;

}  // linux_ptracer_t::tracer_detach

//! PUBLIC: tracer_unsetoptions
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_unsetoptions(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, bool use_cxt,
    pid_t newtid) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_unsetoptions]";
  pid_t who_to_attach_to = p.get_pid(use_cxt);
  WT options = 0;

  if (newtid != -1) {
    who_to_attach_to = newtid;
  }

  if ((r = Pptrace((__ptrace_request)PTRACE_SETOPTIONS, who_to_attach_to, NULL,
                   (void*)options)) != 0) {
    {
      self_trace_t::trace(true, MODULENAME, 0,
                          "unsetoptions returned non-zero for %d...",
                          who_to_attach_to);
    }
    errno = 0;
#if 0
      e = func + ERRMSG_PTRACE + strerror (errno);
      throw linux_tracer_exception_t(e, convert_error_code (errno));
#endif
  }

  return SDBG_TRACE_OK;
}

//! PUBLIC: tracer_setoptions
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_setoptions(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, bool use_cxt,
    pid_t newtid) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_setoptions]";
  pid_t who_to_attach_to = p.get_pid(use_cxt);
  WT options = 0;

  options |= PTRACE_O_TRACECLONE;
  options |= PTRACE_O_TRACEVFORK;
  options |= PTRACE_O_TRACEFORK;

  if (newtid != -1) {
    who_to_attach_to = newtid;
  }

  if ((r = Pptrace((__ptrace_request)PTRACE_SETOPTIONS, who_to_attach_to, NULL,
                   (void*)options)) != 0) {
    {
      self_trace_t::trace(true, MODULENAME, 0,
                          "setoptions returned non-zero for %d...",
                          who_to_attach_to);
    }
    errno = 0;
  }

  return SDBG_TRACE_OK;
}

//! PUBLIC: tracer_attach
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_attach(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, bool use_cxt,
    pid_t newtid) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_attach]";
  pid_t who_to_attach_to = p.get_pid(use_cxt);

  if (newtid != -1) {
    who_to_attach_to = newtid;
  }

  if ((r = Pptrace(PTRACE_ATTACH, who_to_attach_to, 0, 0)) != 0) {
    {
      self_trace_t::trace(true, MODULENAME, 0,
                          "attach returned non-zero for %d, continue...",
                          who_to_attach_to);
    }

    e = func + ERRMSG_PTRACE + strerror(errno);
    throw linux_tracer_exception_t(e, convert_error_code(errno));
  }

  //
  // We must not set "stop" state here because it will be marked
  // when the wait call gets the status update.
  //
  return SDBG_TRACE_OK;

}  // linux_ptracer_t::tracer_attach

//! PUBLIC: status
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::status(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, bool use_cxt) {
  using namespace std;

  stringstream sst;
  string ss;
  pid_t snapTid;
  string execName;
  string tState;
  string func = "[linux_ptracer_t::status]";
  pid_t tpid = p.get_pid(use_cxt);

  sst << "/proc/" << (int)tpid << "/stat";
  sst >> ss;
  if (access(ss.c_str(), R_OK) < 0) {
    //
    // This means tpid is nonexistent.
    //
    return SDBG_TRACE_ESRCH_ERR;
  }

  ifstream fst(ss.c_str());
  if (!fst) {
    return SDBG_TRACE_ESRCH_ERR;
  }

  fst >> snapTid;
  fst >> execName;
  fst >> tState;

  if (tState == string("T")) {
    return SDBG_TRACE_STOPPED;
  }

  fst.close();

  return SDBG_TRACE_RUNNING;
}

//! PUBLIC: tracer_trace_me
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e
linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::tracer_trace_me() {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::tracer_trace_me]";

  if ((r = Pptrace(PTRACE_TRACEME, 0, NULL, NULL)) != 0) {
    e = func + ERRMSG_PTRACE + strerror(errno);
    throw linux_tracer_exception_t(e, convert_error_code(errno));
  }

  return SDBG_TRACE_OK;

}  // linux_ptracer_t::tracer_trace_me

//! PUBLIC: enable_breakpoint
/*!
  inserts the trap instruction at bp.get_address_at() and set
  "disabel" to the BP
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::enable_breakpoint(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, breakpoint_base_t<VA, IT>& bp,
    bool use_cxt, bool change_state) {
  IT blend;
  IT origInst;

  if (!(bp.is_set() || bp.is_disabled())) {
    return SDBG_TRACE_STATE_UNKNOWN;
  }

  if (bp.get_use_indirection()) {
    if (bp.get_indirect_address_at() == T_UNINIT_HEX) {
      //
      // The upper layer could have filled the
      // indirect address to handle special cases.
      //
      VA indAddr;
      tracer_read(p, bp.get_address_at(), &indAddr, sizeof(VA), use_cxt);
      bp.set_indirect_address_at(indAddr);
    }

    tracer_read(p, bp.get_indirect_address_at(), &origInst, sizeof(IT),
                use_cxt);
  } else {
    tracer_read(p, bp.get_address_at(), &origInst, sizeof(IT), use_cxt);
  }

  bp.set_orig_instruction(origInst);
  blend = bp.get_orig_instruction();
  blend &= bp.get_blend_mask();
  blend = blend | bp.get_trap_instruction();

  if (bp.get_use_indirection()) {
    tracer_write(p, bp.get_indirect_address_at(), &blend, sizeof(IT), use_cxt);
  } else {
    tracer_write(p, bp.get_address_at(), &blend, sizeof(IT), use_cxt);
  }

  //
  // BP state transitioned to enabled
  //
  if (change_state) bp.enable();

  return SDBG_TRACE_OK;

}  // linux_ptracer_t::enable_breakpoint

//! PUBLIC: disable_breakpoint
/*!
  pulls out the trap instruction at bp.get_address_at() and set
  "disable" to the BP.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::disable_breakpoint(
    process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, breakpoint_base_t<VA, IT>& bp,
    bool use_cxt, bool change_state) {
  IT origInst;

  if (!bp.is_enabled()) {
    return SDBG_TRACE_STATE_UNKNOWN;
  }

  origInst = bp.get_orig_instruction();

  if (bp.get_use_indirection()) {
    tracer_write(p, bp.get_indirect_address_at(), &origInst, sizeof(IT),
                 use_cxt);
  } else {
    tracer_write(p, bp.get_address_at(), &origInst, sizeof(IT), use_cxt);
  }

  //
  // BP state transitioned to disabled
  //
  if (change_state) bp.disable();

  return SDBG_TRACE_OK;

}  // linux_ptracer_t::disable_breakpoint

//! PUBLIC: convert_error_code
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::convert_error_code(
    int en) {
  tracer_error_e err_code;

  switch (en) {
    case EACCES:
      err_code = SDBG_TRACE_EACCESS_ERR;
      break;
    case EIO:
      err_code = SDBG_TRACE_EIO_ERR;
      break;
    case ESRCH:
      err_code = SDBG_TRACE_ESRCH_ERR;
      break;
    case EINVAL:
      err_code = SDBG_TRACE_EINVAL_ERR;
      break;
    case EPERM:
      err_code = SDBG_TRACE_EPERM_ERR;
      break;
    case EFAULT:
      err_code = SDBG_TRACE_EFAULT_ERR;
      break;
    case EBUSY:
      err_code = SDBG_TRACE_EBUSY_ERR;
      break;
    default:
      err_code = SDBG_TRACE_FAILED;
      break;
  }

  return err_code;

}  // linux_ptracer_t::convert_error_code

//! PUBLIC: baretracer
/*!

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_error_e linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::baretracer(
    int __tag, pid_t __p, VA __addr, WT* __wd) {
  using namespace std;

  WT r;
  string e;
  string func = "[linux_ptracer_t::baretracer]";

  r = Pptrace((__ptrace_request)__tag, __p, (void*)__addr, (void*)__wd);

  if (r == -1 && errno) {
    e = func + ERRMSG_PTRACE + strerror(errno);
    throw linux_tracer_exception_t(e, convert_error_code(errno));
  }

  return SDBG_TRACE_OK;

}  // linux_ptracer_t::baretracer

////////////////////////////////////////////////////////////////////
//
// PRIVATE METHODS (class linux_ptracer_t)
//
//

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
long linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM>::Pptrace(__ptrace_request request,
                                                       pid_t pid, void* addr,
                                                       void* data) {
  {
    self_trace_t::trace(LEVELCHK(level3), MODULENAME, 0,
                        "ptrace(request[%d], pid[%d], addr[0x%8x], data[0x%8x]",
                        request, pid, addr);
  }

  return (ptrace(request, pid, addr, data));
}

#endif  // __SDBG_LINUX_PTRACER_IMPL_HXX

/*
 * ts=2 sw=2 expandtab
 */
