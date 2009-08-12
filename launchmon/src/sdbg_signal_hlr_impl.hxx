/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_signal_hlr_impl.hxx,v 1.4.2.2 2008/02/20 17:37:57 dahn Exp $
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
 *
 *  Update Log:
 *        Sep 24 2008 DHA: Enforced the error handling semantics defined
 *                         in README.ERROR_HANDLING
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Jul 05 2006 DHA: File created      
 */ 

#ifndef SDBG_SIGNAL_HLR_IMPL_HXX
#define SDBG_SIGNAL_HLR_IMPL_HXX 1

#include <lmon_api/common.h>

extern "C" {
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#else
# error sys/types.h is required
#endif

#if HAVE_SIGNAL_H
# include <signal.h>  
#else
# error signal.h is required 
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#else
# error unistd.h is required 
#endif

#if HAVE_UNISTD_H
# include <libgen.h>
#else
# error libgen.h is required 
#endif
}

#if HAVE_ASSERT_H
# include <cassert>
#else
# error cassert is required
#endif

#include "sdbg_signal_hlr.hxx"
#include "sdbg_base_tracer.hxx"
#include "sdbg_base_mach.hxx"
#include "sdbg_base_tracer.hxx"

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_base_t<SDBG_DEFAULT_TEMPLPARAM>
*signal_handler_t<SDBG_DEFAULT_TEMPLPARAM>::tracer        = NULL;

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
std::vector<int> 
signal_handler_t<SDBG_DEFAULT_TEMPLPARAM>::monitoring_signals;

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
process_base_t<SDBG_DEFAULT_TEMPLPARAM> 
*signal_handler_t<SDBG_DEFAULT_TEMPLPARAM>::launcher_proc = NULL;

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
event_manager_t<SDBG_DEFAULT_TEMPLPARAM>
*signal_handler_t<SDBG_DEFAULT_TEMPLPARAM>::evman         = NULL;

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>
*signal_handler_t<SDBG_DEFAULT_TEMPLPARAM>::lmon          = NULL;

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
std::string 
signal_handler_t<SDBG_DEFAULT_TEMPLPARAM>::MODULENAME;

////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class signal_handler_t)
//
//


//! PUBLIC: signal_handler_t
/*!
    Default constructor.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
signal_handler_t<SDBG_DEFAULT_TEMPLPARAM>::signal_handler_t ( )
{

  //
  // Some signals would make it difficult to 
  // continue with the cleanup routine, but
  // it is better to try it and fail 
  // than not to try at all, which will always fail. 
  //
  monitoring_signals.push_back(SIGFPE);
  monitoring_signals.push_back(SIGINT);
  monitoring_signals.push_back(SIGBUS);
  monitoring_signals.push_back(SIGTERM);
  monitoring_signals.push_back(SIGSEGV);  
  monitoring_signals.push_back(SIGILL);  

  MODULENAME = self_trace_t::sighandler_module_trace.module_name;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
signal_handler_t<SDBG_DEFAULT_TEMPLPARAM>::signal_handler_t ( 
		 const signal_handler_t& s )
{
  
}


//! PUBLIC: signal_handler_t
/*!
    Default destructor.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
signal_handler_t<SDBG_DEFAULT_TEMPLPARAM>::~signal_handler_t ( )
{
  
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void 
signal_handler_t<SDBG_DEFAULT_TEMPLPARAM>::install_hdlr_for_all_sigs ( )
{
  using namespace std;
  
  if ( monitoring_signals.empty() )
    return;

  vector<int>::const_iterator pos;
  for (pos = monitoring_signals.begin(); 
         pos != monitoring_signals.end(); pos++)      
    signal (*pos, sighandler);   
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void 
signal_handler_t<SDBG_DEFAULT_TEMPLPARAM>::sighandler ( int sig )
{
  using namespace std;

  if ( monitoring_signals.empty() )
    return; 

  if ( tracer == NULL || launcher_proc == NULL )
    return;
     
  process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p = *launcher_proc;

#if 0
  // DHA 6/03/2009 so far the failure handling due to an anomalous signal into
  // the Engine has been the same, regardless of RMs. Genericize the following
  // code until it breaks for a certain RM... 
  //
  // failure handling sequence is specific to 
  // RM implementations. Regardless, the engine must enforce
  // the following defined error handling.
  //
  string dt 
    = basename ( strdup (p.get_myopts()->get_my_opt()->debugtarget.c_str()));
#endif

  //
  // A. When LaunchMON engine fails, the basic cleanup semantics of LaunchMON
  // is to detach from the job while keeping the client tool running.
  //  A.1. The launchmon engine fails. Unless it is due to SIGKILL,
  //       the engine performs the cleanup in a signal handler.
  //       The cleanup is (A.1.1) to detach from the RM_job
  //       process, (A.1.2) to keep the RM_daemon process running, and
  //       (A.1.3) to notify this event ("lmonp_stop_tracing")
  //       to the FEN API stub, if it can.
  //

  //
  // detach from the target RM_job (A.1.1).
  //
  for ( p.thr_iter = p.get_thrlist().begin();
        p.thr_iter != p.get_thrlist().end(); p.thr_iter++ )
    {
      p.make_context ( p.thr_iter->first );
      if (p.get_lwp_state (true) == LMON_RM_RUNNING)
        {
          get_tracer()->tracer_stop(p, true);
          // Back-to-back SIGSTOP can be lost
          // Thus, the grace period below
          usleep (GracePeriodBNSignals);
        }
      p.check_and_undo_context ( p.thr_iter->first );
    }

  p.set_please_detach ( true );
  p.set_reason (ENGINE_dying_wsignal);

  //
  // poll_processes should return false as the detach handler
  // will return LAUNCHMON_STOP_TRACE which is converted
  // to the false return code. If not, there must have been
  // more events queued up, and it's better to consume all
  // before proceed. A.1.1 and A.1.2.
  //
  while ( evman->poll_processes ( *lmon ) != false );

  //
  // sending the lmonp_stop_tracing message to FEN
  // A.1.3.
  //
  if ( lmon->get_API_mode () )
    {
      lmon->say_fetofe_msg (lmonp_stop_tracing);
    }

  {
     self_trace_t::trace ( LEVELCHK(quiet), 
 	                   MODULENAME,
			   0,
			   "Aborting...");
  }

  abort();
}

#endif //SDBG_SIGNAL_HLR_IMPL_HXX
