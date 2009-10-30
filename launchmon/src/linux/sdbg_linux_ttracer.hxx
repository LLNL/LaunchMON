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

  linux_thread_tracer_t()          { }
  virtual ~linux_thread_tracer_t() { }

  class linux_thread_callback_t {
  public:

    static linux_thr_db td; // Thread debug library DLL support 

    static int ttracer_thread_iter_callback ( const td_thrhandle_t* tt,  
					      void* proc ) 
    {
      using namespace std;

      td_err_e te;
      int threadpid;

      if (tt->th_unique == 0) {
        // this is the main thread
        // this is to work around a bug in the linux debug library's 
        // td_thr_get_info call which has a conditional based upon an uninitialized
        // value. And the condition happens when the thread is the main thread
        // and we don't need to do much with the main thread anyway. 
        return SDBG_TTRACE_OK;
      }

#if X86_ARCHITECTURE || X86_64_ARCHITECTURE
      thread_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>* thrinfo 
	     = new linux_x86_thread_t();
#elif PPC_ARCHITECTURE
      thread_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>* thrinfo 
	     = new linux_ppc_thread_t();
#endif

      process_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>* p 
	     = (process_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>*) proc;

      if ( (te = td.dll_td_thr_event_enable(tt, 1)) != TD_OK )
	return SDBG_TTRACE_FAILED;

      td_thrinfo_t &tinfo = thrinfo->get_thread_info();
      memset(&tinfo, '\0', sizeof(td_thrinfo_t));
      te = td.dll_td_thr_get_info(tt, &tinfo);
      if ( te != TD_OK )
        return SDBG_TTRACE_FAILED;

      threadpid = (int) thrinfo->thr2pid(); 

      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0, 
	  "ttracer_thread_iteration is called back by thread_db" );
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0, 
	  "this callback is for the thread %d",
	  threadpid );
      }

      if ( p->get_thrlist().find ( threadpid ) 
	   == p->get_thrlist().end())
	{
          // if the process doesn't contain tid of the given thread,
          // those threads must have not been attached
          //   
	  p->get_thrlist().insert ( make_pair ( threadpid, thrinfo ) );
	  if( threadpid != p->get_master_thread_pid()) 
	    {

	      thread_tracer_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>::
		    get_tracer()->tracer_attach ( *p, false, threadpid );
	    }
	}
      else 
	{

	  delete thrinfo;	 
 
	  //if (te != TD_OK)  
	    //return SDBG_TTRACE_FAILED;
	}

      return SDBG_TTRACE_OK;
    }

    static int ttracer_thread_iter_attach_callback ( const td_thrhandle_t* tt, 
						     void* proc) 
    {
      using namespace std;

      td_err_e te;
      int threadpid;
 
      if (tt->th_unique == 0) {
        // this is the main thread
        // this is to work around a bug in the linux debug library's 
        // td_thr_get_info call which has a conditional based upon an uninitialized
        // value. And the condition happens when the thread is the main thread
        // and we don't need to do much with the main thread anyway. 
        return SDBG_TTRACE_OK;
      }

#if X86_ARCHITECTURE || X86_64_ARCHITECTURE
      thread_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper> *thrinfo 
	= new linux_x86_thread_t();
#elif PPC_ARCHITECTURE
      thread_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper> *thrinfo 
	= new linux_ppc_thread_t();
#endif   
      process_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper> *p 
	     = (process_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>*) proc;

      if ( (te = td.dll_td_thr_event_enable(tt, 1)) != TD_OK ) 
	return SDBG_TTRACE_FAILED;

      td_thrinfo_t &tinfo = thrinfo->get_thread_info();
      memset(&tinfo, '\0', sizeof(td_thrinfo_t));
      te = td.dll_td_thr_get_info(tt, &tinfo);
      if ( te != TD_OK )
        return SDBG_TTRACE_FAILED;

      threadpid = thrinfo->thr2pid();
      //cout << "thread lwpid" << threadpid << endl;
      {
        self_trace_t::trace ( LEVELCHK(level2),
          MODULENAME,0,
          "ttracer_thread_iter_attach_callback is called back by thread_db" );
        self_trace_t::trace ( LEVELCHK(level2),
          MODULENAME,0,
          "this callback is for the thread %d",
          threadpid );
      }

      if ( p->get_thrlist().find ( threadpid )
           == p->get_thrlist().end())
        {
          // if the process doesn't contain tid of the given thread,
          // those threads must have not been attached
          //
          p->get_thrlist().insert ( make_pair ( threadpid, thrinfo ) );
          if( threadpid != p->get_master_thread_pid())
            {
              thread_tracer_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>::
                    get_tracer()->tracer_attach ( *p, false, threadpid );
            }
        }
      else
        {
          // does thrinfo has a pointer to point to data belonging to thread_db?
          delete thrinfo;

          //if (te != TD_OK)
            //return SDBG_TTRACE_FAILED;
        }

      return SDBG_TTRACE_OK;      
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

  static bool LEVELCHK(self_trace_verbosity level) 
       { return (self_trace_t::thread_tracer_module_trace.verbosity_level >= level); }
  
  // For self tracing
  //
  static std::string MODULENAME;
};

#endif // SDBG_LINUX_TTRACER_HXX 
