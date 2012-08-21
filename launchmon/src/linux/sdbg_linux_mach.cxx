/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/sdbg_linux_mach.cxx,v 1.3.2.2 2008/02/20 17:37:57 dahn Exp $
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
 *        Sep 02 2010 DHA: Added MPIR_attach_fifo support
 *        Aug  07 2009 DHA: Added more comments; added exception throwing 
 *                          into linux_<arch>_process_t constructors that
 *                          calls basic_init.
 *        Mar  11 2008 DHA: Added self-tracing support 
 *        Mar  11 2008 DHA: PowerPC support 
 *        Feb  09 2008 DHA: Added LLNS Copyright
 *        Jan  09 2006 DHA: Linux-X86/86 support
 *        Jan  11 2006 DHA: File created.          
 */ 

#include "sdbg_std.hxx"

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
#endif

#include "sdbg_base_mach.hxx"
#include "sdbg_base_mach_impl.hxx"
#include "sdbg_linux_symtab.hxx"
#include "sdbg_linux_symtab_impl.hxx"
#include "sdbg_linux_mach.hxx"


extern "C" {
#if HAVE_THREAD_DB_H
# include <thread_db.h>
#else
# error thread_db.h is required 
#endif
}

#if X86_ARCHITECTURE || X86_64_ARCHITECTURE

////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class linux_x86_gpr_set_t)
//
//

//! PUBLIC: linux_x86_gpr_set_t
/*!
    Default constructor.
*/
linux_x86_gpr_set_t::linux_x86_gpr_set_t() 
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
  // This part of the code may be difficult to read. It was trial and error
  //   to detemine what registers are writable and what are not.
  // 
  unsigned int gpr_writable_mask = 0;

#if X86_ARCHITECTURE

  gpr_writable_mask |= (1 << 0);
  gpr_writable_mask |= (1 << 1);
  gpr_writable_mask |= (1 << 2);    
  gpr_writable_mask |= (1 << 3);
  gpr_writable_mask |= (1 << 4);
  gpr_writable_mask |= (1 << 5);
  gpr_writable_mask |= (1 << 6);
  gpr_writable_mask |= (1 << 12); // eip (program counter) is writable

#elif X86_64_ARCHITECTURE

  gpr_writable_mask |= (1 << 0);
  gpr_writable_mask |= (1 << 1);
  gpr_writable_mask |= (1 << 2);
  gpr_writable_mask |= (1 << 3);
  gpr_writable_mask |= (1 << 4);
  gpr_writable_mask |= (1 << 5);
  gpr_writable_mask |= (1 << 6);
  gpr_writable_mask |= (1 << 7);
  gpr_writable_mask |= (1 << 8);
  gpr_writable_mask |= (1 << 9);
  gpr_writable_mask |= (1 << 10);
  gpr_writable_mask |= (1 << 11);
  gpr_writable_mask |= (1 << 12);
                                   
  gpr_writable_mask |= (1 << 16); // rip (program counter) is writable

#endif /* X86_64_ARCHITECTURE */

  set_user_offset(0);
  set_writable_mask(gpr_writable_mask);

} // linux_x86_gpr_set_t


//! PUBLIC: linux_x86_gpr_set_t
/*!
    get_pc
*/
const T_VA 
linux_x86_gpr_set_t::get_pc() const
{
  T_GRS gpr = get_native_rs();

#ifdef X86_ARCHITECTURE
  return gpr.eip;
#elif X86_64_ARCHITECTURE
  return gpr.rip;
#endif
}


//! PUBLIC: linux_x86_gpr_set_t
/*!
    get_memloc_for_ret_addr
*/
const T_VA 
linux_x86_gpr_set_t::get_ret_addr() const
{
  //
  // We need to keep track of the "return address"
  // because that address can result from a single-step 
  // needed for a breakpoint operation. 
  // Essentially, we need to be able to indentify all possible
  // reasons for a process stop event; otherwise, our
  // process state transition will cause a BAD thing. 
  //
  // In the case of Linux X86 class, stack pointer register
  // contains this information.  
  //
  // This info is not available in x86 based architecture
  // so we simply return T_UNINIT_HEX

  return T_UNINIT_HEX;
}


//! PUBLIC: linux_x86_gpr_set_t
/*!
    get_memloc_for_ret_addr
*/
const T_VA 
linux_x86_gpr_set_t::get_memloc_for_ret_addr() const
{
  //
  // We need to keep track of the "return address"
  // because that address can result from a single-step 
  // needed for a breakpoint operation. 
  // Essentially, we need to be able to indentify all possible
  // reasons for a process stop event; otherwise, our
  // process state transition will cause a BAD thing. 
  //
  // In the case of Linux X86 class, stack pointer register
  // contains this information.  
  //
  T_GRS gpr = get_native_rs();

#ifdef X86_ARCHITECTURE
  return gpr.esp;
#elif X86_64_ARCHITECTURE
  return gpr.rsp;
#endif
}


//! PUBLIC: linux_x86_gpr_set_t
/*!
  set_pc method doesn't actually write value into tho PC register
  but into its corresponding data strucuture. The user of this method
  must ensure writing this back into the register. 
*/
void 
linux_x86_gpr_set_t::set_pc(T_VA addr) 
{

#ifdef X86_ARCHITECTURE
  rs.eip = addr;  
#elif X86_64_ARCHITECTURE
  rs.rip = addr;
#endif
}


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class linux_x86_fpr_set_t)
//
//

//! PUBLIC: linux_x86_fpr_set_t
/*!
    Default constructor.
*/
linux_x86_fpr_set_t::linux_x86_fpr_set_t() 
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{

  unsigned int fpr_writable_mask = 0xffffff80;
  set_user_offset(sizeof(T_GRS)+sizeof(int));
  set_writable_mask(fpr_writable_mask);

} 


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class linux_x86_thread_t)
//
//

//! PUBLIC: linux_x86_thread_t
/*!
    Default constructor.
*/
linux_x86_thread_t::linux_x86_thread_t() 
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
  set_gprset(new linux_x86_gpr_set_t());
  set_fprset(new linux_x86_fpr_set_t());
}


linux_x86_thread_t::~linux_x86_thread_t() 
{

}

//! PUBLIC: linux_x86_thread_t
/*!
    accessors
*/
pid_t 
linux_x86_thread_t::thr2pid()
{
  //
  // ti_lid is defined by the pthread debug library.  
  // It contains the lightweight process ID (lwp) that 
  // the kernel can understand. For example, passing this
  // information into the ptrace system call, the kernel 
  // should operate on the thread.
  //
  // thr2pid is a misnomer, it should rather be thr2lwp. 
  //

  if ( is_master_thread ())
    return (pid_t) get_master_pid ();
  else
    return (pid_t) get_thread_info().ti_lid;
}


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class linux_x86_process_t)
//
//

//! PUBLIC: linux_x86_process_t
/*!
    Default constructor.
*/
linux_x86_process_t::linux_x86_process_t ( )
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
 
}


//! PUBLIC: linux_x86_process_t
/*!
     A constructor.
*/
linux_x86_process_t::linux_x86_process_t ( 
                 const std::string& mi, 
                 const std::string& md, 
                 const std::string& mt,
                 const std::string& mc )
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
  basic_init(mi, md, mt, mc);
}


//! PUBLIC: linux_x86_process_t
/*!
    A constructor.
*/
linux_x86_process_t::linux_x86_process_t ( 
                 const pid_t& pid )
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
  using namespace std;

  linux_x86_thread_t* master_thread = new linux_x86_thread_t ();
  master_thread->set_master_thread (true);
  master_thread->set_master_pid (pid);
  get_thrlist().insert (make_pair((int)pid, master_thread));

  //
  // TODO: I need to be able to fetch image information 
  // using pid to push it into basic_init.   
  //

}


//! PUBLIC: linux_x86_process_t
/*!
    A constructor: this is the currently recommended constructor
*/
linux_x86_process_t::linux_x86_process_t ( 
                 const pid_t &pid, const std::string &mi )
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
  using namespace std;

  //
  // Once pid is in place, we assume that at least 
  // the main thread already has come into existence
  // with the LMON_RM_CREATED state.
  //

  //
  // default lwp state is LMON_RM_CREATED
  //
  linux_x86_thread_t *master_thread 
    = new linux_x86_thread_t();

  //
  // indicating this represents the main thread 
  //
  master_thread->set_master_thread(true);

  //
  // pid is lwp pid 
  //
  master_thread->set_master_pid(pid);

  //
  // update the thread list
  //
  get_thrlist().insert(make_pair((int)pid, master_thread));

  //
  // now is the time to initialize my images!
  //
  if (!basic_init(mi)) {
    throw (machine_exception_t("fail to initialize a process"));
  }
}


//! PUBLIC: linux_x86_process_t
/*!
    A constructor. This will be deprecated soon
*/
linux_x86_process_t::linux_x86_process_t ( 
                 const pid_t& pid, 
                 const std::string& mi, 
                 const std::string& md, 
                 const std::string& mt,
                 const std::string& mc )
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
  using namespace std;

  // Once pid is in place, we assume that at least 
  // the main thread already has come into existence
  linux_x86_thread_t* master_thread = new linux_x86_thread_t();
  master_thread->set_master_thread(true);
  master_thread->set_master_pid(pid);

  get_thrlist().insert(make_pair((int)pid, master_thread));
  basic_init(mi, md, mt, mc);
} // linux_x86_process_t


//! PRIVATE: linux_x86_process_t
/*!
    launcher_symbols_init
*/
void 
linux_x86_process_t::launcher_symbols_init()
{  
  //
  // symbols relevant to daemon launching
  //
  set_launch_breakpoint_sym (LAUNCH_BREAKPOINT);
  set_launch_being_debug (LAUNCH_BEING_DEBUG);
  set_launch_debug_state (LAUNCH_DEBUG_STATE);
  set_launch_debug_gate (LAUNCH_DEBUG_GATE);
  set_launch_proctable (LAUNCH_PROCTABLE);
  set_launch_proctable_size (LAUNCH_PROCTABLE_SIZE);
  set_launch_acquired_premain (LAUNCH_ACQUIRED_PREMAIN);
  set_launch_exec_path (LAUNCH_EXEC_PATH);
  set_launch_server_args (LAUNCH_SERVER_ARGS);
  set_launch_attach_fifo (LAUNCH_ATTACH_FIFO);
  set_loader_breakpoint_sym(LOADER_BP_SYM);
  set_loader_r_debug_sym(LOADER_R_DEBUG);
  set_loader_start_sym(LOADER_START);
  //set_libc_iden(LIBC_IDEN);

  set_resource_handler_sym(RESOURCE_HANDLER_SYM);
}


//! PUBLIC: linux_x86_process_t
/*!
    Method that initializes static matters including 
    parsing of symbol tables within target images. mi is the
    name of main executable that has all information regarding
    other dependent DSO information. To remain lightweight,
    we only parse out images that matter to the daemon launching 
    interface. 
*/
bool 
linux_x86_process_t::basic_init (const std::string &mi)
{
  struct stat pathchk;


  //
  // make sure paths exist
  //
  if ( stat( mi.c_str(), &pathchk ) != 0 ) 
    {
      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: invalid path: %s", mi.c_str () );

      return false;
    }  
 
  //
  // image object representing the main RM launcher 
  // details are filled in proctected_init down in the base layer
  //
  set_myimage (new linux_image_t<T_VA>(mi));

  //
  // image object representing the dynamic linker 
  // details are filled in proctected_init down in the base layer 
  //
  set_mydynloader_image (new linux_image_t<T_VA>());

  //
  // image object representing the POSIX thread library 
  // details are filled in dynamically at SO load 
  //
  set_mythread_lib_image (new linux_image_t<T_VA>());

  //
  // image object representing the LIBC library 
  // details are filled in dynamically at SO load 
  //
  set_mylibc_image (new linux_image_t<T_VA>());

  //
  // image object representing a RM SO library 
  // details are filled in dynamically at SO load.
  //
  set_myrmso_image (new linux_image_t<T_VA>());

  if (!protected_init(mi)) 
    {
      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: protected_init failed");

      return false;
    }
  
  launcher_symbols_init();

  return true;
}


//! PUBLIC: linux_x86_process_t
/*!
    Method that initializes static matters including 
    parsing of symbol tables within target images. mi is the
    name of main executable that has all information regarding
    other dependent DSO information. To remain lightweight,
    we only parse out images that matter to the daemon launching 
    interface. 
*/
bool 
linux_x86_process_t::basic_init ( 
                 const std::string& mi, 
		 const std::string& md, 
		 const std::string& mt,
		 const std::string& mc )
{ 
  struct stat pathchk;
  
  // make sure paths exist
  //
  if ( stat( mi.c_str(), &pathchk ) != 0 ) 
    {
      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: invalid path: %s", mi.c_str ());

      return false;
    } 
 
  if ( stat( md.c_str(), &pathchk ) != 0 ) 
    {

      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: invalid path: %s", md.c_str ());

      return false;
    }

  if ( stat( mt.c_str(), &pathchk ) != 0 ) 
    {

      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: invalid path: %s", mt.c_str ());

      return false;
    }

  if ( stat( mc.c_str(), &pathchk ) != 0 ) 
    {

      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: invalid path: %s", mc.c_str ());

      return false;
    }
  
  //
  // image object representing the main RM launcher 
  // details are filled in proctected_init down in the base layer
  //
  set_myimage(new linux_image_t<T_VA>(mi));

  //
  // image object representing the dynamic linker 
  // details are filled in proctected_init down in the base layer 
  //
  set_mydynloader_image(new linux_image_t<T_VA>(md));

  //
  // image object representing the POSIX thread library 
  // details are filled in proctected_init down in the base layer 
  //
  set_mythread_lib_image(new linux_image_t<T_VA>(mt));

  //
  // image object representing the LIBC library 
  // details are filled in proctected_init down in the base layer 
  //
  set_mylibc_image( new linux_image_t<T_VA>(mc));

  if (!protected_init(mi, md, mt, mc)) 
    {
      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: protected_init failed");
  
      return false;
    }

  launcher_symbols_init();

  return true;
}


#elif PPC_ARCHITECTURE


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class linux_ppc_gpr_set_t)
//
//
 
//! PUBLIC: linux_ppc_gpr_set_t
/*!
    Default constructor.
*/
linux_ppc_gpr_set_t::linux_ppc_gpr_set_t ()
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
  // This part of the code may be difficult to read. It was trial and error
  //   to detemine what registers are writable and what are not.
  // Mar 11 2008 DHA
  // If somebody has ABI documentation that describes all of these,
  // let me know.  
  //
  unsigned int gpr_writable_mask = 0;
  int i;

  for ( i=0; i < 32; ++i ) 
    {
      gpr_writable_mask |= (1 << i);
    }
 
  set_user_offset (0);
  set_writable_mask (gpr_writable_mask);
 
} // linux_ppc_gpr_set_t
 
 
//! PUBLIC: linux_ppc_gpr_set_t
/*!
    get_pc
*/
const T_VA
linux_ppc_gpr_set_t::get_pc () const
{
  //
  // T_GRP for PowerPC is pt_reg in which nip is defined.
  //
  T_GRS gpr = get_native_rs ();
 
  return gpr.nip;
}
 

//! PUBLIC: linux_x86_gpr_set_t
/*!
    get_memloc_for_ret_addr
*/
const T_VA
linux_ppc_gpr_set_t::get_ret_addr() const
{
  //
  // We need to keep track of the "return address"
  // because that address can result from a single-step
  // needed for a breakpoint operation.
  // Essentially, we need to be able to indentify all possible
  // reasons for a process stop event; otherwise, our
  // process state transition will cause a BAD thing.
  //
  // In the case of Linux PPC class, according to PowerPC ABI
  // doc, the Link Register (LR) contains that information.  
  // (Hence LR is volatil across function calls: this is a
  // moot point for our purpose though.) The struct member 
  // representing the LR is "link." 
  //
  T_GRS gpr = get_native_rs ();
 
  return gpr.link;
}
 

//! PUBLIC: linux_ppc_gpr_set_t
/*!
    get_memloc_for_ret_addr
*/
const T_VA
linux_ppc_gpr_set_t::get_memloc_for_ret_addr () const
{
  //
  // We need to keep track of the "return address"
  // because that address can result from a single-step 
  // needed for a breakpoint operation. 
  // Essentially, we need to be able to indentify all possible
  // reasons for a process stop event; otherwise, our
  // process state transition will cause a BAD thing. 
  //
  // In this architecture, this information is kepted in
  // the register set. We simply return T_UNINIT_HEX. 
 
  return T_UNINIT_HEX;
}
 
 
//! PUBLIC: linux_ppc_gpr_set_t
/*!
  set_pc method doesn't actually write value into tho PC register
  but into its corresponding data strucuture. The user of this method
  must ensure writing this back into the register. 
*/
void
linux_ppc_gpr_set_t::set_pc (T_VA addr)
{
  rs.nip = addr;
}
 
 
////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class linux_ppc_fpr_set_t)
//
//
 
//! PUBLIC: linux_ppc_fpr_set_t
/*!
    Default constructor.
*/
linux_ppc_fpr_set_t::linux_ppc_fpr_set_t()
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
  //
  // asm-ppc/ptrace.h defines PT_FPR0 with 48
  // The offset into the USER area that allows a tool to fetch 
  // floating point registers begins with PT_FPR0 (48)
  // end with PT_FPR0 + 2*31 (which is PT_FPR31). The following
  // in the USER area is PT_FPSCR but again this is a moot
  // point for this project. 
  // 

#ifndef PT_FPR0
#define PT_FPR0 48
#endif 

  unsigned int fpr_writable_mask = 0xffffffff;
  set_user_offset (PT_FPR0);
  set_writable_mask(fpr_writable_mask);
}

////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class linux_ppc_thread_t)
//
//
 
//! PUBLIC: linux_ppc_thread_t
/*!
    Default constructor.
*/
linux_ppc_thread_t::linux_ppc_thread_t()
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
  set_gprset (new linux_ppc_gpr_set_t());
  set_fprset (new linux_ppc_fpr_set_t());
}
 
 
linux_ppc_thread_t::~linux_ppc_thread_t()
{

}
 
//! PUBLIC: linux_ppc_thread_t
/*!
    accessors
*/
pid_t
linux_ppc_thread_t::thr2pid()
{
  //
  // ti_lid is defined by the pthread debug library.  
  // It contains the lightweight process ID (lwp) that 
  // the kernel can understand. For example, passing this
  // information into the ptrace system call, the kernel 
  // should operate on the thread.
  //
  // thr2pid is a misnomer, it should rather be thr2lwp. 
  //

  if ( is_master_thread ())
    return (pid_t) get_master_pid ();
  else
    return (pid_t) get_thread_info().ti_lid;
}
 
 
////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class linux_ppc_process_t)
//
//
 
//! PUBLIC: linux_ppc_process_t
/*!
    Default constructor.
*/
linux_ppc_process_t::linux_ppc_process_t ( )
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
  
}
 
 
//! PUBLIC: linux_ppc_process_t
/*!
     A constructor.
*/
linux_ppc_process_t::linux_ppc_process_t ( 
                 const std::string& mi,
                 const std::string& md,
                 const std::string& mt,
                 const std::string& mc )
{
  basic_init (mi, md, mt, mc);
} // linux_ppc_process_t
 
 
//! PUBLIC: linux_ppc_process_t
/*!
    A constructor.
*/
linux_ppc_process_t::linux_ppc_process_t ( 
                 const pid_t& pid )
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
  using namespace std;

  //
  // default lwp state is LMON_RM_CREATED
  //
  linux_ppc_thread_t* master_thread 
    = new linux_ppc_thread_t ();

  //
  // indicating this represents the main thread 
  //
  master_thread->set_master_thread (true);

  //
  // pid is lwp pid 
  //
  master_thread->set_master_pid (pid);

  //
  // update the thread list
  //
  get_thrlist().insert (make_pair((int)pid, master_thread));

  //
  // TODO: I need to be able to fetch image information 
  // using pid to push it into basic_init.   
  //
} // linux_ppc_process_t
 
 
//! PUBLIC: linux_ppc_process_t
/*!
    A constructor: this is the currently recommended constructor
*/
linux_ppc_process_t::linux_ppc_process_t (
                 const pid_t& pid, const std::string& mi )
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
  using namespace std;
 
  //
  // Once pid is in place, at least the main thread has already 
  // come into existence
  //

  //
  // default lwp state is LMON_RM_CREATED
  //
  linux_ppc_thread_t *master_thread 
    = new linux_ppc_thread_t();

  //
  // indicating this represents the main thread 
  //
  master_thread->set_master_thread (true);

  //
  // pid is lwp pid 
  //
  master_thread->set_master_pid (pid);

  //
  // update the thread list
  //
  get_thrlist().insert (make_pair((int)pid, master_thread));

  //
  // now is the time to initialize my image!
  //
  if (!basic_init(mi)) {
    throw (machine_exception_t("fail to initialize a process"));
  }
} // linux_ppc_process_t
 
 
//! PUBLIC: linux_ppc_process_t
/*!
    A constructor.
*/
linux_ppc_process_t::linux_ppc_process_t ( 
                 const pid_t& pid,
                 const std::string& mi,
                 const std::string& md,
                 const std::string& mt,
                 const std::string& mc )
   : MODULENAME (self_trace_t::machine_module_trace.module_name)
{
  using namespace std;
 
  //
  // Once pid is in place, at least the main thread has already 
  // come into existence
  //
  linux_ppc_thread_t *master_thread 
    = new linux_ppc_thread_t();

  master_thread->set_master_thread (true);

  master_thread->set_master_pid (pid);
 
  get_thrlist().insert (make_pair((int)pid, master_thread));

  //
  // now is the time to initialize my image!
  //
  if (!basic_init(mi)) {
    throw (machine_exception_t("fail to initialize a process"));
  }
} // linux_ppc_process_t
 
 
//! PRIVATE: linux_ppc_process_t
/*!
    launcher_symbols_init
*/
void
linux_ppc_process_t::launcher_symbols_init()
{
  //
  // symbols relevant to daemon launching
  //
  set_launch_breakpoint_sym (LAUNCH_BREAKPOINT);
  set_launch_being_debug (LAUNCH_BEING_DEBUG);
  set_launch_debug_state (LAUNCH_DEBUG_STATE);
  set_launch_debug_gate (LAUNCH_DEBUG_GATE);
  set_launch_proctable (LAUNCH_PROCTABLE);
  set_launch_proctable_size (LAUNCH_PROCTABLE_SIZE);
  set_launch_acquired_premain (LAUNCH_ACQUIRED_PREMAIN);
  set_launch_exec_path (LAUNCH_EXEC_PATH);
  set_launch_server_args (LAUNCH_SERVER_ARGS);
  set_launch_attach_fifo (LAUNCH_ATTACH_FIFO);
  set_loader_breakpoint_sym(LOADER_BP_SYM);
  set_loader_r_debug_sym(LOADER_R_DEBUG);
  set_loader_start_sym(LOADER_START);
  set_resource_handler_sym(RESOURCE_HANDLER_SYM);
}


//! PUBLIC: linux_ppc_process_t
/*!
    basic_init
*/
bool
linux_ppc_process_t::basic_init (
                 const std::string& mi )
{
  struct stat pathchk;
 
  //  
  // make sure paths exist
  //
  if ( stat( mi.c_str(), &pathchk ) != 0 ) 
    {

      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: invalid path %s", mi.c_str ());
  
      return false;
    }
  
  set_myimage (new linux_image_t<T_VA>(mi));
  set_mydynloader_image (new linux_image_t<T_VA>());
  set_mythread_lib_image (new linux_image_t<T_VA>());
  set_mylibc_image (new linux_image_t<T_VA>());
  set_myrmso_image (new linux_image_t<T_VA>());

  if (!protected_init(mi)) 
    {
 
      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: protected_init failed." );
  
      return false;
    }
   
  launcher_symbols_init();
 
  return true;
}
 
 
//! PUBLIC: linux_ppc_process_t
/*!
    basic_init
*/
bool
linux_ppc_process_t::basic_init (
                 const std::string& mi,
                 const std::string& md,
                 const std::string& mt,
                 const std::string& mc )
{
  struct stat pathchk;
   
  //
  // make sure paths exist
  //
  if ( stat( mi.c_str(), &pathchk ) != 0 ) 
    {
 
      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: invalid path %s", mi.c_str ());

      return false;
    }
  if ( stat( md.c_str(), &pathchk ) != 0 ) 
    {
 
      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: invalid path %s", md.c_str ());

      return false;
    }
  if ( stat( mt.c_str(), &pathchk ) != 0 ) 
    {
 
      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: invalid path %s", mt.c_str ());

      return false;
    }
  if ( stat( mc.c_str(), &pathchk ) != 0 ) 
    {
 
      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: invalid path %s", mc.c_str ());
 
      return false;
    }
   
  set_myimage (new linux_image_t<T_VA>(mi));
  set_mydynloader_image (new linux_image_t<T_VA>(md));
  set_mythread_lib_image (new linux_image_t<T_VA>(mt));
  set_mylibc_image (new linux_image_t<T_VA>(mc));
 
  if (!protected_init(mi, md, mt, mc)) 
    {
 
      self_trace_t::trace ( 1,
            MODULENAME,1,
            "Machine: protected_init failed." );
 
      return false;
    }

  launcher_symbols_init();
 
  return true;
}

#endif // ARCHITECTURES
