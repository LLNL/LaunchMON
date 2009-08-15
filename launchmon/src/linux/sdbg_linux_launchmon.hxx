/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/sdbg_linux_launchmon.hxx,v 1.9.2.2 2008/02/20 17:37:57 dahn Exp $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008, Lawrence Livermore National Security, LLC. Produced at 
 * the Lawrence Livermore National Laboratory. Written by Dong H. Ahn <ahn1@llnl.gov>. 
 * LLNL-CODE-409469. All rights reserved.
 *
 * This file is part of LaunchMON. For details, see 
 * https://computing.llnl.gov/?set=resources&page=os_projects
 *
 * Please also read LICENSE.txt -- Our Notice and GNU Lesser General Public License.
 *
 * 
 * This program is free software; you can redistribute it and/or modify it under the 
 * terms of the GNU General Public License (as published by the Free Software 
 * Foundation) version 2.1 dated February 1999.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 59 Temple 
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *--------------------------------------------------------------------------------			
 *
 *  Update Log:
 *        Sep 24 2008 DHA: Added handle_daemon_exit_event for 
 *                         better error handling
 *        Jul 03 2006 DHA: Added self tracing support
 *        Jun 08 2006 DHA: Added attach-to-a-running job support.
 *                         handle_attach_event method
 *        Mar 30 2006 DHA: Added exception handling support
 *        Jan 12 2006 DHA: Created file.
 */ 

#ifndef SDBG_LINUX_LAUNCHMON_HXX
#define SDBG_LINUX_LAUNCHMON_HXX 1

#if HAVE_VECTOR
# include <vector>
#else
# error vector is required
#endif

#include "sdbg_base_launchmon.hxx"
#include "sdbg_linux_std.hxx"
#include "sdbg_linux_mach.hxx"
#include "lmon_api/lmon_proctab.h"


//! tracing_method 
/*!
    tracing method enum for fork and vfork event tracing
*/
enum tracing_method {
  normal_continue,
  in_between,
  syscall_continue
};

//! class linux_launchmon_t<>
/*!
    implements launchmon algorithm
*/
class linux_launchmon_t : public launchmon_base_t<SDBG_LINUX_DFLT_INSTANTIATION>
{

public:
  linux_launchmon_t ( );
  virtual ~linux_launchmon_t ( );

  
  ////////////////////////////////////////////////////////////
  //
  //  Public Interfaces:
  //
  //
  virtual launchmon_rc_e init ( opts_args_t* opt );

  launchmon_rc_e handle_bp_prologue
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p,
		   breakpoint_base_t<T_VA,T_IT>* bp );

  launchmon_rc_e is_bp_prologue_done
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p,
		   breakpoint_base_t<T_VA,T_IT>* bp );
  
  virtual 
  launchmon_rc_e handle_attach_event
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );

  virtual 
  launchmon_rc_e handle_launch_bp_event 
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );
  
  virtual 
  launchmon_rc_e handle_detach_cmd_event 
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );

  virtual 
  launchmon_rc_e handle_kill_cmd_event 
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );

  virtual 
  launchmon_rc_e handle_trap_after_exec_event 
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );

  virtual 
  launchmon_rc_e handle_trap_after_attach_event
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );

  virtual 
  launchmon_rc_e handle_loader_bp_event 
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );
  
  virtual 
  launchmon_rc_e handle_exit_event 
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );

  virtual 
  launchmon_rc_e handle_term_event 
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );
  
  virtual 
  launchmon_rc_e handle_thrcreate_bp_event 
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );

  virtual 
  launchmon_rc_e handle_thrdeath_bp_event 
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );
  
  virtual 
  launchmon_rc_e handle_fork_bp_event
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );

  virtual 
  launchmon_rc_e handle_not_interested_event
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );

  virtual 
  launchmon_rc_e handle_relay_signal_event
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p, int sig);

private:

  ////////////////////////////////////////////////////////////
  //
  //  Private Methods:
  //
  //
  linux_launchmon_t (const linux_launchmon_t& l );

  bool disable_all_BPs (
                process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p, bool );

  bool enable_all_BPs ( 
                process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p, bool );

  bool chk_pthread_libc_and_init ( 
                process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );

  bool acquire_proctable ( 
                process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p, bool );
  
  bool launch_tool_daemons ( 
                process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p );

  launchmon_rc_e init_API (opts_args_t *);

  bool LEVELCHK(self_trace_verbosity level) 
       { return (self_trace_t::launchmon_module_trace.verbosity_level >= level); }

  ////////////////////////////////////////////////////////////
  //  
  // Private Data:
  //
  //

  // sets tracing method between syscall continue and regular continue
  //
  tracing_method continue_method;

  // For self tracing
  //
  std::string MODULENAME; 

};

#endif // SDBG_LINUX_LAUNCHMON_HXX
