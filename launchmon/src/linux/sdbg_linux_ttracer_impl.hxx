/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/sdbg_linux_ttracer_impl.hxx,v 1.4.2.1 2008/02/20 17:37:57 dahn Exp $
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
 *        Mar 09 2009 DHA: Removed an equality check between the thread_db-provided 
 *                         thread creation breakpoint address and the one that 
 *                         the pthread library itself provides for the sake of simplicity.
 *                         It turns out the one that thread_db provides gives 
 *                         different addresses depending on the mode in which 
 *                         the thread_db library is linked.
 *                         o runtime linking --> address of the function itself
 *                         o loadtime linking --> address of a global data variable
 *                         that has the address of this function.
 *        Mar 06 2009 DHA: Added indirect breakpoint support 
 *                         DLL support for the thread debug library
 *        Sep 25 2008 DHA: Fixed a compatibility issue against thread_db 
 *                         in the 2.6.18 Linux kernel
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Jul 03 2006 DHA: Added self tracing support
 *        Mar 30 2006 DHA: Added exception handling support
 *        Feb 07 2006 DHA: Created file.
 */

#ifndef SDBG_LINUX_TTRACER_IMPL_HXX
#define SDBG_LINUX_TTRACER_IMPL_HXX 1

#include "sdbg_std.hxx"

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
#endif

#ifndef UINT32_C
#define UINT32_C(c)    c ## U
#endif

#include "sdbg_linux_ttracer.hxx"

////////////////////////////////////////////////////////////////////
//
// Static member definitions
//
//
template <typename VA, typename WT, typename IT, typename GRS, typename FRS>
std::string 
linux_thread_tracer_t<VA,WT,IT,GRS,FRS>::MODULENAME 
     = self_trace_t::thread_tracer_module_trace.module_name;

template <typename VA, typename WT, typename IT, typename GRS, typename FRS>
td_thr_events_t linux_thread_tracer_t<VA,WT,IT,GRS,FRS>::thread_event_mask;

template <typename VA, typename WT, typename IT, typename GRS, typename FRS>
linux_thr_db linux_thread_tracer_t<VA,WT,IT,GRS,FRS>::linux_thread_callback_t::td;


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class process_base_t)
//
//


//! PUBLIC: ttracer_init
/*!
    Linux Thread tracer initializer. This should be called when 
    a debugger detects the target has linked against a thread
    library.
*/
template <typename VA, typename WT, typename IT, typename GRS, typename FRS>
ttracer_error_e 
linux_thread_tracer_t<VA,WT,IT,GRS,FRS>::ttracer_init ( 
                 process_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>& p, 
		 tracer_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>* pt )
  throw ( thread_tracer_exception_t )
{  
  try {
    using namespace std;
  
    string e;
    string func = "[linux_thread_tracer_t::ttracer_init]";
    td_err_e te;
    td_notify_t event_notify;
    td_thragent_t *ta_pp = NULL;
    image_base_t<VA,elf_wrapper>* thrlib_im = NULL;
    breakpoint_base_t<VA, IT>* bp = NULL;  
#if THREAD_DEATH_EVENT_NEEDED
    breakpoint_base_t<VA, IT>* tdbp = NULL;  
#endif  
    struct ps_prochandle ph_p;
    char *cppath, *dirpath_libpthread; 


    {
      self_trace_t::trace ( LEVELCHK(level2), 
        MODULENAME,0, 
	"thread tracer initialization begins");
    }


    set_tracer(pt);
    thrlib_im = p.get_mythread_lib_image();

    if ( (thrlib_im == NULL) || 
	 (thrlib_im && 
	  thrlib_im->get_image_base_address() == SYMTAB_UNINIT_ADDR)) {
      e = func +
	" Thread library image or its base addr has not been set.";
      throw thread_tracer_exception_t(e, SDBG_TTRACE_FAILED);
    }

    cppath = strdup(thrlib_im->get_path().c_str());
    dirpath_libpthread = dirname(cppath);

    if (!linux_thread_callback_t::td.bind(string(dirpath_libpthread))) {
      e = func +
	" Fails to dlopen and bind thread debug library.";
      throw thread_tracer_exception_t(e, SDBG_TTRACE_FAILED);
    }

    //
    // NOTE: on linux free cppath should be safe
    //
    free(cppath);

    if ( ( te = linux_thread_callback_t::td.dll_td_init () ) 
         != TD_OK ) {
      e = func +
	" td_init is not TD_OK.";
      throw thread_tracer_exception_t(e, SDBG_TTRACE_FAILED);
    }

    ph_p.p = &p;
    if ( ( te = linux_thread_callback_t::td.dll_td_ta_new (&ph_p, &ta_pp)) 
	 != TD_OK ) {
      e = func +
	" td_ta_new is not TD_OK.";
      throw thread_tracer_exception_t(e, SDBG_TTRACE_FAILED);
    }

    td_event_emptyset ( &thread_event_mask );
    td_event_addset   ( &thread_event_mask, TD_CREATE );
//    td_event_addset   ( &thread_event_mask, TD_DEATH );
    linux_thread_callback_t::td.dll_td_ta_set_event( ta_pp, 
      &thread_event_mask );

    te = linux_thread_callback_t::td.dll_td_ta_thr_iter ( ta_pp,
            linux_thread_callback_t::ttracer_thread_iter_callback,
	    (void*) (&p),
	    TD_THR_ANY_STATE, 
	    TD_THR_LOWEST_PRIORITY, 
	    TD_SIGNO_MASK, 
	    TD_THR_ANY_USER_FLAGS);

    if (te != TD_OK) {
      e = func +
	" td_ta_thr_iter is not TD_OK.";
      throw thread_tracer_exception_t(e, SDBG_TTRACE_FAILED);
    }

    te = linux_thread_callback_t::td.dll_td_ta_event_addr (ta_pp, 
            TD_CREATE, &event_notify);

    if ( te != TD_OK ) {
	 e = func +
	   " td_ta_event_addr TD_CREATE is not TD_OK.";	 
	 throw thread_tracer_exception_t(e, SDBG_TTRACE_FAILED);
    }

    if ( event_notify.type == NOTIFY_BPT ) {   
      bp = new linux_breakpoint_t();
      const symbol_base_t<VA>& thread_create_bp_sym = 
	thrlib_im->get_a_symbol (p.get_thread_creation_sym());

      bp->set_address_at (thread_create_bp_sym.get_relocated_address());
#if RM_BG_MPIRUN
      bp->set_use_indirection();
#endif
      bp->status = breakpoint_base_t<VA, IT>::set_but_not_inserted;
      p.set_thread_creation_hidden_bp(bp);
      pt->insert_breakpoint ( p, 
			      *(p.get_thread_creation_hidden_bp()),
			      true);

      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0, 
	  "Thread creation bp inserted [0x%8x]",
	  event_notify.u.bptaddr);
      }

    }

#if THREAD_DEATH_EVENT_NEEDED
    te = linux_thread_callback_t::td.dll_td_ta_event_addr ( ta_pp,
            TD_DEATH, &event_notify);

    if (te != TD_OK) {
      e = func +
	" td_ta_event_addr TD_DEATH is not TD_OK.";	 
      throw thread_tracer_exception_t(e, SDBG_TTRACE_FAILED);
    }

    if ( event_notify.type == NOTIFY_BPT ) {   
      tdbp = new linux_breakpoint_t();
      const symbol_base_t<VA>& thread_death_bp_sym = 
	thrlib_im->get_a_symbol (p.get_thread_death_sym());

#if WITH_INDIRECT_BP
      tdbp->set_use_indirection();
#endif
      tdbp->set_address_at(thread_create_bp_sym.get_relocated_address());
      tdbp->status = breakpoint_base_t<VA, IT>::set_but_not_inserted;
      p.set_thread_death_hidden_bp(tdbp);
      pt->insert_breakpoint ( p, 
			      *(p.get_thread_death_hidden_bp()),
			      true);
    }
#endif
    return SDBG_TTRACE_OK;
  } 
  catch ( symtab_exception_t e ) {
    e.report();
    abort();
  }
  catch ( tracer_exception_t e ) {
    e.report();
    abort();
  }
}


//! PUBLIC: ttracer_attach
/*!
    a method to handle thread attach
*/
template <typename VA, typename WT, typename IT, typename GRS, typename FRS>
ttracer_error_e 
linux_thread_tracer_t<VA,WT,IT,GRS,FRS>::ttracer_attach ( 
                 process_base_t<VA,WT,IT,GRS,FRS,td_thrinfo_t,elf_wrapper>& p )
  throw ( thread_tracer_exception_t )
{
  try {
    using namespace std;

    string e;
    string func = "[linux_thread_tracer_t::ttracer_attach]";
    int nthr;  
    td_err_e te;
    td_thragent_t* new_ta;
    struct ps_prochandle ph_p;

    ph_p.p = &p;


    {
      self_trace_t::trace ( LEVELCHK(level2), 
	MODULENAME,0, 
	"A thread newly created and ttracer_attach event handler invoked");
    }

    te = linux_thread_callback_t::td.dll_td_ta_new (&ph_p, &new_ta);
    if (te != TD_OK) {
      e = func +
	" td_ta_thr_iter is not TD_OK.";
      throw thread_tracer_exception_t (e, SDBG_TTRACE_FAILED);
    }

    te = linux_thread_callback_t::td.dll_td_ta_get_nthreads (new_ta, &nthr);
    if (te != TD_OK) {
      e = func +
	" td_ta_thr_iter is not TD_OK.";
      throw thread_tracer_exception_t (e, SDBG_TTRACE_FAILED);
    }

    te = linux_thread_callback_t::td.dll_td_ta_thr_iter ( new_ta,
            linux_thread_callback_t::ttracer_thread_iter_attach_callback,
	    (void*) (&p),
	    TD_THR_ANY_STATE,
	    TD_THR_LOWEST_PRIORITY,
	    TD_SIGNO_MASK,
	    TD_THR_ANY_USER_FLAGS);

    if ( te != TD_OK) {
#if 0
      e = func +
	" td_ta_thr_iter is not TD_OK.";
      throw thread_tracer_exception_t(e, SDBG_TTRACE_FAILED);
#endif
      //cout << "TD NOT OK" << endl;
    }

    return SDBG_TTRACE_OK;
  }
  catch ( symtab_exception_t e ) {
    e.report();
    abort();
  }
  catch ( tracer_exception_t e ) {
    e.report();
    abort();
  }
}

#endif // SDBG_LINUX_TTRACER_IMPL_HXX 
