/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/sdbg_proc_service.cxx,v 1.3.2.1 2008/02/20 17:37:57 dahn Exp $
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
 *        Mar  11 2008 DHA: Added PowerPC support
 *        Feb  09 2008 DHA: Added LLNS Copyright
 *        Jul  18 2006 DHA: Initial Linux PPC (BG/L) port 
 *        Jul  14 2006 DHA: Initial Linux X86-64 port
 *        Feb  07 2006 DHA: Made it compilable with templated class objects
 *        Jan  10 2006 DHA: Created file.
 */ 


//! file sdbg_proc_service.cxx
/*!
  This file contains a set of routines that Linux thread debug
  library expects from a debugger. 
*/

#include "sdbg_std.hxx"

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
#endif
 

#include "sdbg_proc_service.hxx"
#include "sdbg_linux_std.hxx"
#include "sdbg_linux_ptracer.hxx"
#include "sdbg_linux_ptracer_impl.hxx"
#include "sdbg_base_mach.hxx"
#include "sdbg_base_mach_impl.hxx"
#include "sdbg_linux_mach.hxx"

extern "C" {
#include <libgen.h>
#include <limits.h>
}

#if X86_ARCHITECTURE || PPC_ARCHITECTURE

#ifndef PTRACE_GET_THREAD_AREA
#define PTRACE_GET_THREAD_AREA 25
#endif

struct user_desc
{
  unsigned int entry_number;
  unsigned long int base_addr;
  unsigned int limit;
  unsigned int seg_32bit:1;
  unsigned int contents:2;
  unsigned int read_exec_only:1;
  unsigned int limit_in_pages:1;
  unsigned int seg_not_present:1;
  unsigned int useable:1;
  unsigned int empty:25;
};

#elif X86_64_ARCHITECTURE

#ifndef PTRACE_ARCH_PRCTL
#define PTRACE_ARCH_PRCTL       30    /* arch_prctl for child */
#endif
#ifndef ARCH_GET_GS
#define ARCH_GET_GS 0x1004
#endif
#ifndef ARCH_GET_FS
#define ARCH_GET_FS 0x1003
#endif
#ifndef GS 
#define GS 26
#endif
#ifndef FS
#define FS 25
#endif

#endif

linux_ptracer_t<SDBG_LINUX_DFLT_INSTANTIATION> myprocess_tracer;


//! PUBLIC: ps_pdread
/*!
    a routine to read from the given process, which 
    the thread debug library uses.
*/
extern "C" ps_err_e 
ps_pdread ( struct ps_prochandle *ph,  psaddr_t addr,
	    void *buf,  size_t size )
{
  bool use_cxt = true; 
  if ( myprocess_tracer.tracer_read (*(ph->p), 
				     (T_VA) addr, 
				     buf, 
				     size, 
				     use_cxt)
                                 != SDBG_TRACE_OK ) {
    return PS_ERR;
  }
  
  return PS_OK;
}


//! PUBLIC: ps_pdwrite
/*!
    a routine to write into the given process, which 
    the thread debug library uses.
*/
extern "C" ps_err_e 
ps_pdwrite ( struct ps_prochandle *ph,  psaddr_t addr, 
	     const void *buf, size_t size )
{  
  bool use_cxt = true;
  if ( myprocess_tracer.tracer_write (*(ph->p), 
				      (T_VA) addr, 
				      buf, 
				      size,
				      use_cxt)
                                 != SDBG_TRACE_OK ) {
    return PS_ERR;
  }

  return PS_OK;
}


//! PUBLIC: ps_ptread
/*!
    a routine to read from the given thread, which 
    the thread debug library uses. (Identical to process read)
*/
extern "C" ps_err_e 
ps_ptread ( struct ps_prochandle *ph,  psaddr_t addr,  
	    void *buf,  size_t size )
{
  return (ps_pdread (ph, addr, buf, size));
}


//! PUBLIC: ps_ptwrite
/*!
    a routine to read from the given thread, which 
    the thread debug library uses. (Identical to process write)
*/
extern "C" ps_err_e 
ps_ptwrite ( struct ps_prochandle *ph,  psaddr_t addr, 
	     const void *buf, size_t size )
{
  return (ps_pdwrite (ph, addr, buf, size));
}


//! PUBLIC: ps_lgetregs
/*!
    a routine to read registers from the given thread, which 
    the thread debug library uses.
*/
extern "C" ps_err_e 
ps_lgetregs ( struct ps_prochandle* ph, 
	      lwpid_t id, prgregset_t reg)
{ 
  bool use_cxt = true;

  if ( myprocess_tracer.tracer_getregs (*(ph->p), use_cxt)
                                       != SDBG_TRACE_OK ) {

    return PS_ERR;
  }

  // copy ph->p's GPR to reg
  memcpy((T_GRS*) reg, &(ph->p->get_gprset(use_cxt)->get_native_rs()), sizeof(T_GRS)); 

  return PS_OK;
}
  

//! PUBLIC: ps_lsetregs
/*!
    a routine to set registers for the given thread, which 
    the thread debug library uses.
*/
extern "C" ps_err_e 
ps_lsetregs (struct ps_prochandle* ph, 
	     lwpid_t id, const prgregset_t reg)
{
  bool use_cxt = true;

  if ( myprocess_tracer.tracer_setregs (*(ph->p), use_cxt) 
                                       != SDBG_TRACE_OK ) {

    return PS_ERR;
  }
  //printf("Please implement ps_lsetregs\n");
  return PS_OK;
}


//! PUBLIC: ps_lgetfpregs
/*!
    a routine to get FP registers for the given thread, which 
    the thread debug library uses.
*/
extern "C" ps_err_e 
ps_lgetfpregs ( struct ps_prochandle* ph, lwpid_t id, 
		prfpregset_t* reg )
{
  bool use_cxt = true;

  if ( myprocess_tracer.tracer_getfpregs (*(ph->p), use_cxt) 
                                       != SDBG_TRACE_OK ) {

    return PS_ERR;
  }
  // copy ph->p  FPR to reg  
  return PS_OK;
}


//! PUBLIC: ps_lsetfpregs
/*!
    a routine to set FP registers for the given thread, which 
    the thread debug library uses.
*/
extern "C" ps_err_e 
ps_lsetfpregs ( struct ps_prochandle* ph, lwpid_t id, 
		const prfpregset_t* reg)

{
  bool use_cxt = true;

  if ( myprocess_tracer.tracer_setfpregs (*(ph->p), use_cxt) 
                      != SDBG_TRACE_OK ) {
    return PS_ERR;
  }

  return PS_OK;
}


//! PUBLIC: ps_getpid
/*!
    a routine to get the pid of the main process, which
    the thread debug library uses.
*/
extern "C" pid_t ps_getpid ( struct ps_prochandle *ph )
{
  return (ph->p->get_master_thread_pid());
}


//! PUBLIC: ps_get_thread_area
/*!
    a routine to get the thread specific area, which
    the thread debug library uses.
*/
extern "C" ps_err_e 
ps_get_thread_area ( const struct ps_prochandle *ph,
		     lwpid_t lpid, int x, psaddr_t *addr)
{
  bool use_cxt = true;

#if X86_ARCHITECTURE || PPC_ARCHITECTURE
  /*
   * How to fetch thread-specific area for x86/linux and powerPC/linux
   *
   */ 

  /*
    a linux kernel patch added two new ptrace requests for 
    this architecture "PTRACE_GET_THREAD_AREA" and "
    "PTRACE_SET_THREAD_AREA"

    Those macroes are not exported via standard /usr/include 
    header files. So are struct user_desc definition and 
    ifndef PTRACE_GET_THREA_AREA to be 25
  */

  // -- excerpt from an email written by Roland McGrath @ Redhat
  /*
    Roland McGrath (roland@redhat.com)
    Fri, 20 Dec 2002 00:32:41 -0800 

    This patch vs 2.5.51 (should apply fine to 2.5.52) adds two new ptrace
    requests for i386, PTRACE_GET_THREAD_AREA and PTRACE_SET_THREAD_AREA.
    These let another process using ptrace do the equivalent of performing
    get_thread_area and set_thread_area system calls for another thread.

    We are working on gdb support for the new threading code in the kernel
    using the new NPTL library, and use PTRACE_GET_THREAD_AREA for that.
    This patch has been working fine for that.

    I added PTRACE_SET_THREAD_AREA just for completeness, so that you can
    change all the state via ptrace that you can read via ptrace as has
    previously been the case. It doesn't have an equivalent of set_thread_area
    with .entry_number = -1, but is otherwise the same.

    Both requests use the ptrace `addr' argument for the entry number rather
    than the entry_number field in the struct. The `data' parameter gives the
    address of a struct user_desc as used by the set/get_thread_area syscalls.

    The code is quite simple, and doesn't need any special synchronization
    because in the ptrace context the thread must be stopped already.

    I chose the new request numbers arbitrarily from ones not used on i386.
    I have no opinion on what values should be used.

    People I talked to preferred adding this interface over putting an array of
    struct user_desc in struct user as accessed by PTRACE_PEEKUSR/POKEUSR
    (which would be a bit unnatural since those calls access one word at a time).
  */

  struct user_desc tsd_desc; 
  if ( myprocess_tracer.baretracer (PTRACE_GET_THREAD_AREA, 
				    ph->p->get_pid(use_cxt), 
				    (T_VA)x, 
				    (T_WT*)&tsd_desc ) 
                                        != SDBG_TRACE_OK ) 
    {
      return PS_ERR;
    }

  *addr = (psaddr_t) (tsd_desc.base_addr);

#elif X86_64_ARCHITECTURE
  /*
   * How to fetch thread-specific area for x86-64/linux
   *
   */ 

  /*
    A comment from  x86_64/kernel/ptrace.c about PTRACE_GET_THREAD_AREA

    " This makes only sense with 32bit programs. Allow a
      64bit debugger to fully examine them too. Better
      don't use it against 64bit processes, use
      PTRACE_ARCH_PRCTL instead."

      So PTRACE_GET_THREAD_AREA isn't an option.

      and x86_64's ptrace has PTRACE_ARCH_PRCTL request field in its switch
      statement such that

      -----
      " normal 64bit interface to access TLS data.
        Works just like arch_prctl, except that the arguments
	are reversed."

      case PTRACE_ARCH_PRCTL:
              ret = do_arch_prctl(child, data, addr);
              break;


      And do_arch_prctl is defined in x86_64/kernel/process.c 

      -----
      ... prctl.h
      #define ARCH_SET_GS 0x1001
      #define ARCH_SET_FS 0x1002
      #define ARCH_GET_FS 0x1003
      #define ARCH_GET_GS 0x1004
      ...

      long do_arch_prctl(struct task_struct *task, int code, unsigned long addr)

       switch (code) {
        case ARCH_SET_GS:
	
	<CUT>
	
	case ARCH_SET_FS:

	<CUT>

	case ARCH_GET_FS: 

	<CUT>

	case ARCH_GET_GS: 


      Assuming thread debug library is passing a valid x which is 
      one of above four


      -----
      One last piece is found in /usr/include/sys/reg.h
      # define R15    0
      # define R14    1
      # define R13    2
      # define R12    3
      # define RBP    4
      # define RBX    5
      # define R11    6
      # define R10    7
      # define R9     8
      # define R8     9
      # define RAX    10
      # define RCX    11
      # define RDX    12
      # define RSI    13
      # define RDI    14
      # define ORIG_RAX 15
      # define RIP    16
      # define CS     17
      # define EFLAGS 18
      # define RSP    19
      # define SS     20
      # define FS_BASE 21
      # define GS_BASE 22
      # define DS     23
      # define ES     24
      # define FS     25
      # define GS     26

      So, I'm passing following

  */
  T_WT arg;
  psaddr_t thread_area;

  switch (x) 
    {
    case GS:
      arg = ARCH_GET_GS;
      break;
    case FS:
      arg = ARCH_GET_FS;
      break;
    default:
      // FIXME:
      arg = 0;
      break;      	
    }

  if ( myprocess_tracer.baretracer (PTRACE_ARCH_PRCTL, 
				    ph->p->get_pid(use_cxt), 
				    (T_VA) &thread_area, 
				    (T_WT*) arg ) != SDBG_TRACE_OK ) 
    {
      if ( arg == ARCH_GET_GS )	
        {
         printf ("thread area fetching failed for general purpose register\n");
	}
      else if ( arg == ARCH_GET_FS )
	{
         printf ("thread area fetching failed for floating point register\n");
	} 
      return PS_ERR;
    }

  *addr = thread_area;
  
  
#elif IA64_ARCHITECTURE
  /*
   * How to fetch thread-specific area for ia64/linux
   *
   */  

#endif

  return PS_OK;
}


static 
bool 
equal_base(std::string const & path, std::string const & refpath)
{
  char tmp[PATH_MAX];
  char *bn;

  sprintf(tmp, "%s", refpath.c_str());
  bn = basename(tmp);

  return (path == std::string(bn));
} 

static std::string& get_basename( std::string path)
{ 
  char tmp[PATH_MAX];
  char *bn; 
 
  sprintf(tmp, "%s", path.c_str());
  bn = basename(tmp); 
  std::string* rstr = new std::string(bn);
 
  return *rstr;
} 

//! PUBLIC: ps_pglobal_lookup
/*!
    a routine to fetch a global symbol, which the thread db
    library uses
*/
extern "C" ps_err_e 
ps_pglobal_lookup ( struct ps_prochandle *ph, 
			     const char *object_name, 
			     const char *sym_name, psaddr_t *sym_addr )
{
  using namespace std;
  ps_err_e error_code;

  string objpath(object_name);
  string sym(sym_name);
  string pthread_path(ph->p->get_mythread_lib_image()->get_path());
  string myimage_path(ph->p->get_myimage()->get_path());
  string loader_path(ph->p->get_mydynloader_image()->get_path());
  
  if (!object_name)
    return PS_ERR;

  //
  // TODO: This if/else should be modified once process_base_t class 
  // gets to retain std::map obj and maintain each and every library 
  // that the target process brings into its process-address space.
  //
  if ( equal_base(objpath, pthread_path) ) {

    const symbol_base_t<T_VA>& asym 
      = ph->p->get_mythread_lib_image()->get_a_symbol(sym);
    if (asym.get_raw_address()!= SYMTAB_UNINIT_ADDR && 
	asym.get_relocated_address()) {

      (*sym_addr) = (psaddr_t) asym.get_relocated_address();
      error_code = (*sym_addr)? PS_OK : PS_ERR;
    }
    else {
      error_code = PS_NOSYM;
    }
  }
  else if ( equal_base(objpath, myimage_path) ) {

    const symbol_base_t<T_VA>& asym 
      =  ph->p->get_myimage()->get_a_symbol(sym);
    if (asym.get_raw_address()!= SYMTAB_UNINIT_ADDR && 
	asym.get_relocated_address()) {

      (*sym_addr) = (psaddr_t) asym.get_relocated_address();
      error_code = (*sym_addr)? PS_OK : PS_ERR;
    }
    else {
      error_code = PS_NOSYM;
    }
  }
  else if ( equal_base(objpath, loader_path)) {

    const symbol_base_t<T_VA>& asym 
      =  ph->p->get_mydynloader_image()->get_a_symbol(sym);
    if (asym.get_raw_address()!= SYMTAB_UNINIT_ADDR && 
	asym.get_relocated_address()) {

      (*sym_addr) = (psaddr_t) asym.get_relocated_address();
      error_code = (*sym_addr)? PS_OK : PS_ERR;
    }
    else {
      error_code = PS_NOSYM;
    }
  }
  else {
    error_code = PS_ERR;
  }

  return error_code;
}


//! PUBLIC: ps_pstop
/*!
    a routine to stop the given process, which the thread db
    library uses
*/
extern "C" ps_err_e 
ps_pstop ( const struct ps_prochandle *ph)
{
  bool use_cxt = false;
  if ( myprocess_tracer.tracer_stop(*(ph->p), use_cxt)
                                    != SDBG_TRACE_OK ) {

    return PS_ERR;
  }

  return PS_OK;
}


//! PUBLIC: ps_pcontinue
/*!
    a routine to continue the given process, which the thread db
    library uses
*/
extern "C" ps_err_e 
ps_pcontinue ( const struct ps_prochandle *ph )
{
  bool use_cxt = false;
  if ( myprocess_tracer.tracer_continue(*(ph->p), use_cxt)
                                      != SDBG_TRACE_OK ) {
    return PS_ERR;
  }

  return PS_OK;
}


//! PUBLIC: ps_lstop
/*!
    a routine to stop the given thread, which the thread db
    library uses
*/
extern "C" ps_err_e 
ps_lstop ( const struct ps_prochandle *ph, lwpid_t lp)
{
  bool use_cxt = true;

  ph->p->make_context ( (const int) lp );
  if (myprocess_tracer.tracer_stop(*(ph->p), use_cxt)
	                          != SDBG_TRACE_OK) {
    ph->p->check_and_undo_context( (const int) lp );
    return PS_ERR;
  }
  ph->p->check_and_undo_context( (const int) lp );

  return PS_OK;
}


//! PUBLIC: ps_lcontinue
/*!
    a routine to continue the given thread, which the thread db
    library uses
*/
extern "C" ps_err_e 
ps_lcontinue (const struct ps_prochandle *ph, lwpid_t lp)
{
  bool use_cxt = true;

  ph->p->make_context ( (const int) lp );
  if (myprocess_tracer.tracer_continue(*(ph->p), use_cxt)
	                          != SDBG_TRACE_OK) {
    ph->p->check_and_undo_context( (const int) lp );
    return PS_ERR;
  }
  ph->p->check_and_undo_context( (const int) lp );

  return PS_OK;
}

