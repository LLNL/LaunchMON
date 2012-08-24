/*                                                       
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/sdbg_linux_ptracer.hxx,v 1.5.2.1 2008/02/20 17:37:57 dahn Exp $
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
 *        Jul 31 2012 DHA: Added setoptions/setunoptions
 *        Dec 16 2009 DHA: Added demangling support for C++ function names
 *                         and moved get_backtrace into sdbg_linux_ptracer.hxx
 *                         to use it for all other modules in the linux layer.
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Aug 16 2007 DHA: backtrace support for linux tracer exception
 *        Jul 03 2006 DHA: Added self tracing support
 *        Mar 30 2006 DHA: Added exception handling support
 *        Jan 10 2006 DHA: Created file.          
 */ 

#ifndef SDBG_LINUX_PTRACER_HXX
#define SDBG_LINUX_PTRACER_HXX 1

extern "C" {
#if HAVE_SYS_PTRACE_H
# include <sys/ptrace.h>
#else
# error sys/ptrace.h is required 
#endif
}

#include "sdbg_base_tracer.hxx"
#include "sdbg_self_trace.hxx"

const int LINUX_TRACER_EVENT_CLONE = PTRACE_EVENT_CLONE;


//! class linux_tracer_exception_t : public tracer_exception_t
/*!
 

*/
class linux_tracer_exception_t : public tracer_exception_t 
{

public:

  linux_tracer_exception_t ()                          { }
  linux_tracer_exception_t ( const char* m, tracer_error_e e)
                     { set_message (m);
                       error_code = e;
                       set_type ( std::string ( "SDBG_TRACER_ERROR" ) );
                       set_fn ( std::string (__FILE__) );
                       set_ln ( __LINE__ );
                       get_backtrace(); 
                      }
  linux_tracer_exception_t( const std::string& m, tracer_error_e e)
                     { set_message (m);
                       error_code = e;
                       set_type ( std::string ( "SDBG_TRACER_ERROR" ) );
                       set_fn ( std::string (__FILE__) );
                       set_ln ( __LINE__ );
                       get_backtrace();
                      }

  virtual ~linux_tracer_exception_t()                  { }
  bool get_backtrace () 
                     {
		       std::string mybt;
		       bool rc = glic_backtrace_wrapper(mybt);
		       set_bt(mybt);		       
		       return rc;
                     }
};


//! class linux_ptracer_t
/*!
    Linux ptrace implementation of trace_base_t. 

    It provides a reference ptrace implementation using
    ptrace interface on Linux/X86 system. If you have
    a ptrace variant whose semantics are slightly 
    different from Linux's, you should override the
    corresponding methods by deriving your class from
    linux_ptrace_t class.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH> 
class linux_ptracer_t : public tracer_base_t<SDBG_DEFAULT_TEMPLPARAM>
{

public:
  linux_ptracer_t();
  linux_ptracer_t(const linux_ptracer_t<SDBG_DEFAULT_TEMPLPARAM> &p);
  virtual ~linux_ptracer_t() {}

 
  ////////////////////////////////////////////////////////////
  //
  //  Interfaces
  //
  virtual tracer_error_e tracer_setregs    
  ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt )
    throw (linux_tracer_exception_t);    

  virtual tracer_error_e tracer_getregs   
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt )
    throw (linux_tracer_exception_t);   

  virtual tracer_error_e tracer_setfpregs 
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt )
    throw (linux_tracer_exception_t);      

  virtual tracer_error_e tracer_getfpregs 
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt )
    throw (linux_tracer_exception_t);   

  virtual tracer_error_e tracer_read       
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, 
      VA addr, void* buf, int size, bool use_cxt )
    throw (linux_tracer_exception_t);   

  virtual tracer_error_e tracer_read_string
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
      VA addr, void* buf, int size, bool use_cxt )
    throw (linux_tracer_exception_t);

  virtual tracer_error_e tracer_get_event_msg
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
      VA addr, void* buf, bool use_cxt )
    throw (tracer_exception_t);   

  virtual tracer_error_e tracer_write      
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
      VA addr, const void* buf, int size, bool use_cxt )	
    throw (linux_tracer_exception_t);   

  virtual tracer_error_e tracer_continue   
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt )
    throw (linux_tracer_exception_t);   

  virtual tracer_error_e tracer_deliver_signal
  ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, int sig, bool use_cxt )
    throw (linux_tracer_exception_t);  

  virtual tracer_error_e tracer_stop
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt );

  virtual tracer_error_e tracer_kill    
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt )
    throw (linux_tracer_exception_t);   

  virtual tracer_error_e tracer_singlestep 
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt )
    throw (linux_tracer_exception_t);   

  virtual tracer_error_e tracer_syscall  
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt )
    throw (linux_tracer_exception_t);    

  virtual tracer_error_e tracer_detach  
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt );

  virtual tracer_error_e tracer_setoptions
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt, pid_t newtid )
    throw (linux_tracer_exception_t);

  virtual tracer_error_e tracer_unsetoptions
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt, pid_t newtid )
    throw (linux_tracer_exception_t);

  virtual tracer_error_e tracer_attach  
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt, pid_t newtid )
    throw (linux_tracer_exception_t);
  
  virtual tracer_error_e status
    (  process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, bool use_cxt );

  virtual tracer_error_e tracer_trace_me   ()
    throw (linux_tracer_exception_t);   

  virtual tracer_error_e enable_breakpoint 
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p,
      breakpoint_base_t<VA, IT>& bp, bool use_cxt, bool change_state=true )
    throw (linux_tracer_exception_t);

  virtual tracer_error_e disable_breakpoint 
    ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, 
      breakpoint_base_t<VA, IT>& bp, bool use_cxt, bool change_state=true )
    throw (linux_tracer_exception_t);

  virtual tracer_error_e convert_error_code(int err)
    throw (linux_tracer_exception_t);   

  tracer_error_e baretracer ( int __tag, pid_t __p, VA __addr, WT* __wd)
    throw (linux_tracer_exception_t);   

private:
  
  bool LEVELCHK(self_trace_verbosity level) 
       { return (self_trace_t::tracer_module_trace.verbosity_level >= level); }

  long Pptrace ( __ptrace_request request, 
		 pid_t pid, 
		 void *addr, 
		 void *data);

  // For self tracing
  //
  std::string MODULENAME; 
};

#endif // SDBG_LINUX_PTRACER_HXX
