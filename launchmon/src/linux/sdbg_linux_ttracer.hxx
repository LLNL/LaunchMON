/*
 * $Header: $
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
 *        Feb 10 2010 DHA: Rewrite some of the thread iterator callback routines
 *                         to handle an exception arising from the thread_db where
 *                         it provides a garbage value when a target thread
 *                         recently exited and is in zombie state.  
 *                         The W/R will simply stop the iteration down in the
 *                         thread_db avoiding the infinite loop formed.  
 *                         Under this condition, the new logic within the ttracer_attach 
 *                         calls td_ta_thr_iter to fail over. If that also fails, 
 *                         attaching to the new thread is deferred until the next 
 *                         pthread create point, which can be bad!.
 *        Oct 30 2009 DHA: A patch to work around a bug in NPTL thread debug 
 *                         library's td_thr_get_info call.
 *        Mar 06 2008 DHA: Thread debug library dll support 
 *        Sep 25 2008 DHA: Fixed a compatibility issue against thread_db 
 *                         in the 2.6.18 Linux kernel
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Jul 03 2006 DHA: Added self tracing support
 *        Jun 29 2006 DHA: Added thread list update logic into 
 *                          ttracer_thread_iter_callback. This is needed for
 *                          attach-to-a-running-job case
 *        Mar 30 2006 DHA: Added exception handling support
 *        Feb 07 2006 DHA: Created file.
 */

#ifndef SDBG_LINUX_TTRACER_HXX
#define SDBG_LINUX_TTRACER_HXX 1

#if HAVE_STRING
# include <string>
#else
# error string is required
#endif

#if HAVE_IOSTREAM
# include <iostream>
#else
# error iostream is required
#endif

#if HAVE_MAP
# include <map>
#else
# error map is required
#endif

#include "sdbg_base_ttracer.hxx"
#include "sdbg_self_trace.hxx"
#include "sdbg_linux_mach.hxx"
#include "sdbg_linux_thr_db.hxx"

extern "C" {
#include <thread_db.h>
}

//! class linux_thread_tracer_t
/*!
    linux thread tracer class 
*/
template <typename VA, typename WT, typename IT, typename GRS, typename FRS>
class linux_thread_tracer_t 
  : public thread_tracer_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>
{
public:

  linux_thread_tracer_t()          { my_thragent = NULL;}
  virtual ~linux_thread_tracer_t() { }

  class linux_thread_callback_t {
  public:

    static linux_thr_db td; // Thread debug library DLL support 

    static int ttracer_thread_iter_callback ( const td_thrhandle_t* tt,  
					      void* proc ) 
    {
      using namespace std;

      td_err_e te;

      if ( (te = td.dll_td_thr_event_enable(tt, 1)) != TD_OK )
	return SDBG_TTRACE_FAILED;

      process_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper> *p 
	     = (process_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>*) proc;

      td_thrinfo_t tinfo; 
      memset(&tinfo, '\0', sizeof(td_thrinfo_t));
      te = td.dll_td_thr_get_info(tt, &tinfo);

      if ( te != TD_OK )
        {
          self_trace_t::trace ( LEVELCHK(level1),
             MODULENAME, 1,
             "td_thr_get_info returned a non TD_OK code.");

          return -1;
        }

      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0, 
	  "ttracer_thread_iteration is called back by thread_db" );
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0, 
	  "this callback is for the thread %d",
	  tinfo.ti_lid);
      }

      if (tinfo.ti_lid < 0)
        {
          return -1;
        }

      if ( tinfo.ti_lid == p->get_master_thread_pid() )
        {
          p->get_thrlist()[tinfo.ti_lid]->copy_thread_info(tinfo) ;
          if ( (te = td.dll_td_thr_event_enable(tt, 1)) != TD_OK )
            {
              self_trace_t::trace ( LEVELCHK(level1),
                MODULENAME, 1,
                "td_thr_event_enable didn't return TD_OK");

	      return -1;
            }
        }
      else
        {
          if ( p->get_thrlist().find ( tinfo.ti_lid ) 
	       == p->get_thrlist().end())
	    {
              // this thread has not been seen
#if X86_ARCHITECTURE || X86_64_ARCHITECTURE
             thread_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper> *thrinfo 
	       = new linux_x86_thread_t();
#elif PPC_ARCHITECTURE
             thread_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper> *thrinfo 
	       = new linux_ppc_thread_t();
#endif
	     thrinfo->copy_thread_info(tinfo);
             // if the process doesn't contain tid of the given thread,
             // those threads must have not been attached
             //   
	     p->get_thrlist().insert ( make_pair ( tinfo.ti_lid, thrinfo ) );
	     thread_tracer_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>::
		    get_tracer()->tracer_attach ( *p, false, tinfo.ti_lid);
             if ( (te = td.dll_td_thr_event_enable(tt, 1)) != TD_OK )
               {
                 self_trace_t::trace ( LEVELCHK(level1),
                   MODULENAME, 1,
                   "td_thr_event_enable didn't return TD_OK");

	         return -1;
               }
	    }
	}

      return 0;
    }

    static int ttracer_thread_iter_attach_callback ( const td_thrhandle_t* tt, 
						     void* proc) 
    {
      using namespace std;

      td_err_e te;

      process_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper> *p 
	     = (process_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>*) proc;

      td_thrinfo_t tinfo; 
      memset(&tinfo, '\0', sizeof(td_thrinfo_t));
      te = td.dll_td_thr_get_info(tt, &tinfo);

      if ( te != TD_OK )
        return SDBG_TTRACE_FAILED;

      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0, 
	  "ttracer_thread_iteration is called back by thread_db" );
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0, 
	  "this callback is for the thread %d",
	  tinfo.ti_lid);
      }

      if (tinfo.ti_lid < 0)
        {
          return -1;
        }

      if ( tinfo.ti_lid == p->get_master_thread_pid() )
        {
          p->get_thrlist()[tinfo.ti_lid]->copy_thread_info(tinfo) ;
          if ( (te = td.dll_td_thr_event_enable(tt, 1)) != TD_OK )
            {
              self_trace_t::trace ( LEVELCHK(level1),
                MODULENAME, 1,
                "td_thr_event_enable didn't return TD_OK");

	      return -1;
            }
        }
      else
        {
          if ( p->get_thrlist().find ( tinfo.ti_lid ) 
	       == p->get_thrlist().end())
	    {
              // this thread has not been seen
#if X86_ARCHITECTURE || X86_64_ARCHITECTURE
             thread_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper> *thrinfo 
	       = new linux_x86_thread_t();
#elif PPC_ARCHITECTURE
             thread_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper> *thrinfo 
	       = new linux_ppc_thread_t();
#endif
	     thrinfo->copy_thread_info(tinfo);
             // if the process doesn't contain tid of the given thread,
             // those threads must have not been attached
             //   
	     p->get_thrlist().insert ( make_pair ( tinfo.ti_lid, thrinfo ) );
	     thread_tracer_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>::
		    get_tracer()->tracer_attach ( *p, false, tinfo.ti_lid);

             if ( (te = td.dll_td_thr_event_enable(tt, 1)) != TD_OK )
               {
                 self_trace_t::trace ( LEVELCHK(level1),
                   MODULENAME, 1,
                   "td_thr_event_enable didn't return TD_OK");

	         return -1;
               }
            }
	}

      return 0;
    }

  }; // nested class linux_thread_callback_t 

  ////////////////////////////////////////////////////////////
  //
  //  Interfaces. 
  //
  //  ttracer_init gets called right after the target 
  //  is detected threaded.
  //
  //  ttracer_attach is called when a new thread gets
  //  created and need to be attached.
  // 
  virtual 
  ttracer_error_e ttracer_init 
    ( process_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>& p, 
      tracer_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>* pt )
    throw ( thread_tracer_exception_t );
  
  virtual 
  ttracer_error_e ttracer_attach
    ( process_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>& p)
    throw ( thread_tracer_exception_t );

private:

  // the global mask that sets kinds of event to be notified.
  //
  static td_thr_events_t thread_event_mask;

  //
  // make sure there is only one td_thragent_t and ps_prochandle
  //
  td_thragent_t *my_thragent;
  struct ps_prochandle my_ph_p;

  static bool LEVELCHK(self_trace_verbosity level) 
       { return (self_trace_t::thread_tracer_module_trace.verbosity_level >= level); }
  
  // For self tracing
  //
  static std::string MODULENAME;
};

#endif // SDBG_LINUX_TTRACER_HXX 
