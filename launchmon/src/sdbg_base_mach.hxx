/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_base_mach.hxx,v 1.7.2.2 2008/02/20 17:37:57 dahn Exp $
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
 *  Update Log:
 *        May 08 2008 DHA: Added an alias (is_master_thread) for get_master_thread
 *                         because the latter isn't entirely intuitive.
 *        Mar 18 2008 DHA: Added BlueGene support.
 *        Feb 09 2008 DHA: Added LLNS Copyright.
 *        May 22 2006 DHA: Added exception class for the machine layer..
 *        Jan 11 2006 DHA: Created file.          
 */ 

#ifndef SDBG_BASE_MACH_HXX
#define SDBG_BASE_MACH_HXX 1

#include <map>
#include <stack>
#include <string>
#include "sdbg_std.hxx"
#include "sdbg_opt.hxx"
#include "sdbg_base_bp.hxx"
#include "sdbg_base_symtab.hxx"
#include "sdbg_base_exception.hxx"

const int THREAD_KEY_INVALID = -1;



////////////////////////////////////////////////////////////////////////////
//
//

//! class machine_exception_t : public exception_base_t
/*!
  exception class for machine layer

*/
class machine_exception_t : public exception_base_t
{
public:  

  machine_exception_t ()                          { }
  machine_exception_t ( const char* m )
                      { set_message (m);
                        set_type ( std::string ( "SDBG_MACHINE_ERROR" ) );
                        set_fn ( std::string (__FILE__) );
                        set_ln ( __LINE__ );
                      }
  machine_exception_t ( const std::string& m)  
                      { set_message (m);
                        set_type ( std::string ( "SDBG_MACHINE_ERROR" ) );
                        set_fn ( std::string (__FILE__) );
                        set_ln ( __LINE__ );
                      }

  virtual ~machine_exception_t()                  { }

};
  

////////////////////////////////////////////////////////////////////////////
//
//

//! class register_set_base_t
/*!
    register_set_t is the wrapper class for native register set 
    data structure. Native register sets are typically 
    defined in the system headers. For example, sys/user.h 
    in the case of Linux. Users of this class first have to wrap 
    those native register sets (FP register set and General 
    register set) into this class.
 */
template <typename NATIVE_RS,typename VA,typename WT>
class register_set_base_t
{

public:

  // constructors and destructor
  //
  register_set_base_t ();
  explicit register_set_base_t (const int offset);
  register_set_base_t(const register_set_base_t<NATIVE_RS,VA,WT>& r);  
  virtual ~register_set_base_t ();

  //
  // accessors 
  //
  int get_offset_in_user ( ) const           { return offset_in_user; }
  WT* get_rs_ptr ()                          { return rs_ptr; }
  NATIVE_RS& get_native_rs()                 { return rs; }  
  virtual VA get_pc ()                       { return 0; }
  virtual VA get_ret_addr()                  { return 0; }
  virtual VA get_memloc_for_ret_addr()       { return 0; }
  unsigned int get_writable_mask()           { return writable_mask; }
  void set_user_offset (int offset)          { offset_in_user = offset; }
  void set_ptr_to_regset ()                  { rs_ptr = (WT*) &rs; }
  void set_writable_mask (unsigned int m)    { writable_mask = m; }
  virtual void set_pc (VA p)                 { }
 
  void inc_ptr_by_word ()                    { rs_ptr++; }
  void write_word_to_ptr (WT w)              { (*rs_ptr) = w; }
  unsigned int size_in_word ();

private:
  // "rs" retains register set object in its native data structure. 
  // "offset_in_user" contains info about what offset 
  // in "USER" area does regset reside.
  // "rs_ptr" is a temporary pointer which will be used 
  // in traversing USER area
  // Some registers are not simply writtable. 
  // Need a mask(writable_mask).
  NATIVE_RS rs;
  int offset_in_user;
  WT* rs_ptr;
  unsigned int writable_mask; 
};


////////////////////////////////////////////////////////////////////////////
//
//

//! enum debug_event_e, class debug_event_t
/*!
    enumerating process states, C++ class wrapper
*/
enum debug_event_e { 
  EV_EXITED,
  EV_TERMINATED,
  EV_STOPPED,
  EV_NOCHILD,
  EV_INVALID
};

class debug_event_t {

public:
  debug_event_t()                          { ev = EV_INVALID; u.exitcode = -1; }
  ~debug_event_t()                         { }
  void set_ev (const enum debug_event_e e) { ev = e; }
  void set_signum (const int s)            { u.signum = s; }
  void set_exitcode (const int ec)         { u.exitcode = ec; }
  debug_event_e get_ev()                   { return ev; }
  int get_signum()                         { return u.signum; }
  int get_exitcode()                       { return u.exitcode; }
  
private:
  debug_event_e ev;
  union {
    int signum;
    int exitcode;
  } u; 
};


////////////////////////////////////////////////////////////////////////////
//
//

//! class thread_base_t
/*!
    thread_base_t is the base thread class. Since each
    thread has its own register set, it contains 
    gprset for General Purpose Register set and
    fprset for Floating Point Register set.

    

*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
class thread_base_t 
{

public:

  // constructors and destructor
  //
  thread_base_t();  
  virtual ~thread_base_t();

  // pure virtual method. A derived class must define this method,
  // returning pid that the kernel understands for each user thread
  virtual pid_t thr2pid() = 0;

  // this implements waitpid for this thread
  // override this only when the generic sniff_debug_event isn't enough
  //virtual bool sniff_debug_event ( debug_event_t& );

  // accessors
  // 
  NT& get_thread_info();
  define_gset(bool,master_thread)
  define_gset(pid_t,master_pid)  

  bool is_master_thread () { return get_master_thread(); }
  register_set_base_t<GRS,VA,WT>* get_gprset();
  void set_gprset(register_set_base_t<GRS,VA,WT>* g);
  register_set_base_t<FRS,VA,WT>* get_fprset();
  void set_fprset(register_set_base_t<FRS,VA,WT>* f);

private:

  bool master_thread; // indicator for the master thread
  pid_t master_pid;   // process id of the containing proc
  NT thread_info;     // parameterized thread info
  register_set_base_t<GRS,VA,WT>* gprset; 
  register_set_base_t<FRS,VA,WT>* fprset; 
};


////////////////////////////////////////////////////////////////////////////
//
//

enum pcont_req_reason {
  RM_BE_daemon_exited = 0,
  RM_MW_daemon_exited,
  RM_JOB_exited,
  RM_JOB_mpir_aborting,
  FE_requested_detach,  
  FE_requested_kill,
  FE_requested_shutdown_dmon,	
  FE_disconnected,
  ENGINE_dying_wsignal,
  reserved_for_rent
};

//! class process_base_t
/*!
    process_base_t is the base process class. 
 
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
class process_base_t 
{

public:
 
  // constructors and destructor
  //
  process_base_t  ();
  process_base_t  
    ( const std::string &mi, const std::string &md,  
      const std::string &mt, const std::string& mc );
  virtual ~process_base_t (); 
   
  bool make_context(const int key) throw(machine_exception_t);
  bool check_and_undo_context(const int key) throw(machine_exception_t);
  register_set_base_t<GRS,VA,WT>* get_gprset(bool context_sensitive);
  register_set_base_t<FRS,VA,WT>* get_fprset(bool context_sensitive);

  // accessors
  //
  const pid_t get_master_thread_pid();
  const pid_t get_pid(bool context_sensitive);
  std::map<int, thread_base_t<SDBG_DEFAULT_TEMPLPARAM>*, ltstr>& get_thrlist();
  typename std::map<int, thread_base_t<SDBG_DEFAULT_TEMPLPARAM>*, ltstr>::iterator thr_iter;
  
  image_base_t<VA,EXECHANDLER>* get_myimage ();
  image_base_t<VA,EXECHANDLER>* get_mydynloader_image ();
  image_base_t<VA,EXECHANDLER>* get_mythread_lib_image ();
  image_base_t<VA,EXECHANDLER>* get_mylibc_image();
  breakpoint_base_t<VA,IT>* get_launch_hidden_bp ();
  breakpoint_base_t<VA,IT>* get_loader_hidden_bp ();
  breakpoint_base_t<VA,IT>* get_thread_creation_hidden_bp ();
  breakpoint_base_t<VA,IT>* get_thread_death_hidden_bp ();
  breakpoint_base_t<VA,IT>* get_fork_hidden_bp ();
  opts_args_t* get_myopts() { return myopts; }

  void set_myimage (image_base_t<VA,EXECHANDLER>* i);
  void set_mydynloader_image (image_base_t<VA,EXECHANDLER>* i);
  void set_mythread_lib_image (image_base_t<VA,EXECHANDLER>* i);
  void set_mylibc_image (image_base_t<VA,EXECHANDLER>* i);
  void set_launch_hidden_bp(breakpoint_base_t<VA,IT>* b);
  void set_loader_hidden_bp(breakpoint_base_t<VA,IT>* b);
  void set_thread_creation_hidden_bp(breakpoint_base_t<VA,IT>* b);
  void set_thread_death_hidden_bp(breakpoint_base_t<VA,IT>* b); 
  void set_fork_hidden_bp(breakpoint_base_t<VA,IT>* b);
  void set_myopts(opts_args_t* o) { myopts = o; }

  define_gset(bool,never_trapped)
  define_gset(bool, please_detach)
  define_gset(bool, please_kill)
  define_gset(enum pcont_req_reason, reason)
  define_gset(std::string,launch_breakpoint_sym)
  define_gset(std::string,launch_being_debug)
  define_gset(std::string,launch_debug_state)
  define_gset(std::string,launch_debug_gate)
  define_gset(std::string,launch_proctable)
  define_gset(std::string,launch_proctable_size)  
  define_gset(std::string,launch_acquired_premain)
  define_gset(std::string,launch_exec_path)
  define_gset(std::string,launch_server_args)
  define_gset(std::string,thread_creation_sym)
  define_gset(std::string,thread_death_sym)
  define_gset(std::string,loader_breakpoint_sym)
  define_gset(std::string,loader_start_sym)
  define_gset(std::string,loader_r_debug_sym)
  define_gset(std::string,resource_handler_sym)
  define_gset(std::string,fork_sym)
  //define_gset(int,key_to_thread_context)
  define_gset(int,rid)
  int get_cur_thread_ctx();  
 
protected:
  bool protected_init ( const std::string& mi, 
			const std::string& md, 
			const std::string& mt,
		        const std::string& mc );

  bool protected_init ( const std::string& mi );

private:

  // Has the process ever initally exec'ed and stopped ?
  bool never_trapped;
  
  // anyone wants to detach?
  bool please_detach;

  // anyone wants to kill?
  bool please_kill;

  enum pcont_req_reason reason;

  image_base_t<VA,EXECHANDLER>* myimage;                 
  image_base_t<VA,EXECHANDLER>* mydynloader_image;
  image_base_t<VA,EXECHANDLER>* mythread_lib_image;
  image_base_t<VA,EXECHANDLER>* mylibc_image;

  opts_args_t* myopts;
  int rid;

  //
  // hidden breakpoints
  //
  breakpoint_base_t<VA,IT>* launch_hidden_bp;
  breakpoint_base_t<VA,IT>* loader_hidden_bp;
  breakpoint_base_t<VA,IT>* thread_creation_hidden_bp;
  breakpoint_base_t<VA,IT>* thread_death_hidden_bp;
  breakpoint_base_t<VA,IT>* fork_hidden_bp; 

  //
  // launcher/debugger ABI symbols  
  //
  std::string launch_breakpoint_sym;
  std::string launch_being_debug;
  std::string launch_debug_state;
  std::string launch_debug_gate;
  std::string launch_proctable;
  std::string launch_proctable_size;
  std::string launch_acquired_premain;
  std::string launch_exec_path;
  std::string launch_server_args;
  std::string thread_creation_sym;
  std::string thread_death_sym;
  std::string loader_breakpoint_sym;
  std::string loader_start_sym;
  std::string loader_r_debug_sym;
  std::string resource_handler_sym; 
  std::string fork_sym;

  // WARNING: Do not attempt to copy thrclist to another list
  // of the same type. It will copy the pointers but not pointees.
  // It is tricky to implement polymorphism using STL containers.
  std::map<int, thread_base_t<SDBG_DEFAULT_TEMPLPARAM>*, ltstr> thrlist;
  std::stack<int> thread_ctx_stack;
};


#endif // SDBG_BASE_MACH_HXX
