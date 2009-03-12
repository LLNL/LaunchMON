/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_event_manager.hxx,v 1.5.2.2 2008/02/20 17:37:57 dahn Exp $
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
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Jul 04 2006 DHA: Added self tracing support
 *        Jan 12 2006 DHA: Created file.          
 */ 

#ifndef SDBG_EVENT_MANAGER_HXX
#define SDBG_EVENT_MANAGER_HXX 1

#include <list>
#include <vector>
#include "sdbg_base_mach.hxx"
#include "sdbg_base_launchmon.hxx"


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
class monitor_proc_thread_t
{

public:
  monitor_proc_thread_t ();
  ~monitor_proc_thread_t ();
  
  bool wait_for_all (pid_t& p, debug_event_t& rc);


};


//! event_manager_t
/*!
    The class sniffs debug events from each process and  
    invokes a correspondig handler that is registered through 
    the launchmon_base_t object.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
class event_manager_t 
{

public:
  
  event_manager_t ();
  ~event_manager_t ();
  
  bool multiplex_events ( process_base_t<SDBG_DEFAULT_TEMPLPARAM>& proc,
		    launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>& lm );
  bool poll_processes ( launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>& lm );
  bool poll_FE_socket ( process_base_t<SDBG_DEFAULT_TEMPLPARAM>& proc,
		    launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>& lm );
  bool register_process ( process_base_t<SDBG_DEFAULT_TEMPLPARAM>* proc );
  bool delete_process ( process_base_t<SDBG_DEFAULT_TEMPLPARAM>* proc );
  
  //FIXME: 
  process_base_t<SDBG_DEFAULT_TEMPLPARAM>* tmp_launcher_proc;
  
private:

  bool LEVELCHK(self_trace_verbosity level) 
       { return (self_trace_t::event_module_trace.verbosity_level >= level); }


  //
  // WARNING: do not attempt to copy proclist to another list
  // of the same type. It will copy the pointers, not pointees.
  // It is just tricky and time consuming to implement 
  // polymorphism using STL containers.
  //
  std::list <process_base_t<SDBG_DEFAULT_TEMPLPARAM>* > proclist;
  monitor_proc_thread_t<SDBG_DEFAULT_TEMPLPARAM>* ev_monitor;

  //
  // For self tracing
  //
  std::string MODULENAME; 
};

#endif // SDBG_EVENT_MANAGER_HXX
