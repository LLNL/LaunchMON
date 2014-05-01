/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/sdbg_linux_mach.hxx,v 1.3.2.1 2008/02/20 17:37:57 dahn Exp $
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
 *        Mar  06 2009 DHA: Remove dynloader_iden
 *        Mar  11 2008 DHA: Linux PowerPC support 
 *        Feb  09 2008 DHA: Added LLNS Copyright
 *        Jan  09 2007 DHA: Linux X86-64 support 
 *        Jan  11 2006 DHA: Created file.
 */ 

#ifndef SDBG_LINUX_MACH_HXX
#define SDBG_LINUX_MACH_HXX 1

extern "C" {
#include <sys/user.h>
#include <thread_db.h>
}

#include <string>
#include "sdbg_std.hxx"
#include "sdbg_base_mach.hxx"
#include "sdbg_linux_std.hxx"
#include "sdbg_linux_symtab.hxx"


#if X86_ARCHITECTURE || X86_64_ARCHITECTURE

//! linux_x86_gpr_set_t: 
/*!
    The class represents linux x86 and x86-64 general purpose register set. 

    The offset in the USER area is calculated as following. 

    Given the USER area of 

    === X86 (32 bit) ===

    struct user
    {
    struct user_regs_struct       regs;
    int                           u_fpvalid;
    struct user_fpregs_struct     i387;
    unsigned long int             u_tsize;
    unsigned long int             u_dsize;
    unsigned long int             u_ssize;
    unsigned long                 start_code;
    unsigned long                 start_stack;
    long int                      signal;
    int                           reserved;
    struct user_regs_struct*      u_ar0;
    struct user_fpregs_struct*    u_fpstate;
    unsigned long int             magic;
    char                          u_comm [32];
    int                           u_debugreg [8];
    };

    General purpose register set (regs) resides at the zero offset. 
    General purpose register set (struct user_regs_struct) is

    struct user_regs_struct
    {
    long int ebx;
    long int ecx;
    long int edx;
    long int esi;
    long int edi;
    long int ebp;
    long int eax;
    long int xds;
    long int xes;
    long int xfs;
    long int xgs;
    long int orig_eax;
    long int eip;
    long int xcs;
    long int eflags;
    long int esp;
    long int xss;
    };

    === X86-64 (64 bit) ===

    struct user
    {
    struct user_regs_struct     regs;
    int                         u_fpvalid;
    struct user_fpregs_struct   i387;
    unsigned long int           u_tsize;
    unsigned long int           u_dsize;
    unsigned long int           u_ssize;
    unsigned long               start_code;
    unsigned long               start_stack;
    long int                    signal;
    int                         reserved;
    struct user_regs_struct*    u_ar0;
    struct user_fpregs_struct*  u_fpstate;
    unsigned long int           magic;
    char                        u_comm [32];
    unsigned long int           u_debugreg [8];
    };

    General purpose register set resides at the zero offset.
    General purpose register set (struct user_regs_struct) is

    struct user_regs_struct
    {
    unsigned long r15;
    unsigned long r14;
    unsigned long r13;
    unsigned long r12;
    unsigned long rbp;
    unsigned long rbx;
    unsigned long r11;
    unsigned long r10;
    unsigned long r9;
    unsigned long r8;
    unsigned long rax;
    unsigned long rcx;
    unsigned long rdx;
    unsigned long rsi;
    unsigned long rdi;
    unsigned long orig_rax;
    unsigned long rip;
    unsigned long cs;
    unsigned long eflags;
    unsigned long rsp;
    unsigned long ss;
    unsigned long fs_base;
    unsigned long gs_base;
    unsigned long ds;
    unsigned long es;
    unsigned long fs;
    unsigned long gs;
    };


 */

class linux_x86_gpr_set_t 
  : public register_set_base_t<T_GRS, T_VA, T_WT>
{

public:

  // constructors and destructor
  //
  linux_x86_gpr_set_t  ();
  virtual ~linux_x86_gpr_set_t () { }  

  virtual void set_pc  (T_VA addr);
  virtual T_VA const get_pc () const; 
  virtual T_VA const get_ret_addr() const;
  virtual T_VA const get_memloc_for_ret_addr() const;

private:
  bool LEVELCHK(self_trace_verbosity level)
       { return (self_trace_t::tracer_module_trace.verbosity_level >= level); }

  std::string MODULENAME;
};

//! linux_x86_frs_set_t:
/*!
    The class represents linux x86 floating point register set.
  
    The offset in the USER area is calculated as following. 

    Given the USER area of 

    === X86 (32 bit) ===

    struct user
    {
    struct user_regs_struct       regs;
    int                           u_fpvalid;
    struct user_fpregs_struct     i387;
    unsigned long int             u_tsize;
    unsigned long int             u_dsize;
    unsigned long int             u_ssize;
    unsigned long                 start_code;
    unsigned long                 start_stack;
    long int                      signal;
    int                           reserved;
    struct user_regs_struct*      u_ar0;
    struct user_fpregs_struct*    u_fpstate;
    unsigned long int             magic;
    char                          u_comm [32];
    int                           u_debugreg [8];
    };
  
    floating point register set resides in following offset.
    offset = sizeof(struct user_regs_struct)+sizeof(int);    

    === X86-64 (64 bit) ===

    struct user
    {
    struct user_regs_struct     regs;
    int                         u_fpvalid;
    struct user_fpregs_struct   i387;
    unsigned long int           u_tsize;
    unsigned long int           u_dsize;
    unsigned long int           u_ssize;
    unsigned long               start_code;
    unsigned long               start_stack;
    long int                    signal;
    int                         reserved;
    struct user_regs_struct*    u_ar0;
    struct user_fpregs_struct*  u_fpstate;
    unsigned long int           magic;
    char                        u_comm [32];
    unsigned long int           u_debugreg [8];
    };

    General purpose register set resides in the zero offset.
    General purpose register set appears

    struct user_regs_struct
    {
    unsigned long r15;
    unsigned long r14;
    unsigned long r13;
    unsigned long r12;
    unsigned long rbp;
    unsigned long rbx;
    unsigned long r11;
    unsigned long r10;
    unsigned long r9;
    unsigned long r8;
    unsigned long rax;
    unsigned long rcx;
    unsigned long rdx;
    unsigned long rsi;
    unsigned long rdi;
    unsigned long orig_rax;
    unsigned long rip;
    unsigned long cs;
    unsigned long eflags;
    unsigned long rsp;
    unsigned long ss;
    unsigned long fs_base;
    unsigned long gs_base;
    unsigned long ds;
    unsigned long es;
    unsigned long fs;
    unsigned long gs;
    };

*/
class linux_x86_fpr_set_t 
  : public register_set_base_t<T_FRS, T_VA, T_WT>
{

public:

  // constructors and destructor
  //
  linux_x86_fpr_set_t ();
  virtual ~linux_x86_fpr_set_t() { }

private:
  bool LEVELCHK(self_trace_verbosity level)
       { return (self_trace_t::tracer_module_trace.verbosity_level >= level); }

  std::string MODULENAME;
};


//! class linux_x86_thread_t:
/*!
    linux_x86_thread_t is linux x86 and x86-64 implementation of thread_base_t
    class. The constructor sets register model. 
*/
class linux_x86_thread_t 
  : public thread_base_t<SDBG_LINUX_DFLT_INSTANTIATION> 
{

public:
  
  // constructors and destructor
  //
  linux_x86_thread_t();
  virtual ~linux_x86_thread_t();

  // virtual method to convert thread id to lwp
  // which the kernel understands
  virtual pid_t thr2pid();
   

private:
  bool LEVELCHK(self_trace_verbosity level)
       { return (self_trace_t::tracer_module_trace.verbosity_level >= level); }

  std::string MODULENAME;
};


//! linux_x86_process_t:
/*!
  linux_x86_process_t is linux x86 and x86-64 implementation of process_base_t
  class. The constructor sets register model. 
*/
class linux_x86_process_t 
  : public process_base_t<SDBG_LINUX_DFLT_INSTANTIATION> 
{

public:

  // constructors and destructor
  //
  linux_x86_process_t ();
  linux_x86_process_t ( const std::string& mi, 
			const std::string& md, 
			const std::string& mt,
			const std::string& mc );
  linux_x86_process_t ( const pid_t& pid, 
			const std::string& mi, 
			const std::string& md,
			const std::string& mt,
		        const std::string& mc );
  linux_x86_process_t ( const pid_t& pid, 
			const std::string& mi );

  explicit linux_x86_process_t ( const pid_t& pid );
  ~linux_x86_process_t() { } 

protected:
  bool basic_init ( const std::string& mi, 
		    const std::string& md, 
		    const std::string& mt,
		    const std::string& mc );

  bool basic_init ( const std::string& mi );

private:
  void launcher_symbols_init();
  bool LEVELCHK(self_trace_verbosity level)
       { return (self_trace_t::tracer_module_trace.verbosity_level >= level); }

  std::string MODULENAME;

};

// data structure needed by proc service layer
//
struct ps_prochandle {
  process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>* p;
};


#elif PPC_ARCHITECTURE

//! linux_ppc_gpr_set_t:
/*!
    The class represents linux ppc general purpose register set.
 
    The offset in the USER area is calculated as following.
    (This is defined in sys/user.h) 
 
    === ppc (32 bit process) ===

    struct user {
        struct pt_regs  regs;                   
        size_t          u_tsize;               
        size_t          u_dsize;             
        size_t          u_ssize;            
        unsigned long   start_code;        
        unsigned long   start_data;       
        unsigned long   start_stack;   
        long int        signal;       
        struct regs *   u_ar0;      
        unsigned long   magic;     
        char            u_comm[32]; 
    };
 
    Thus, general purpose register set (regs) resides 
    at the zero offset. General purpose register set 
    (struct pt_regs: defined in ppc-asm/ptrace.h) is 

    struct pt_regs {
        unsigned long gpr[32];
        unsigned long nip;
        unsigned long msr;
        unsigned long orig_gpr3;        
        unsigned long ctr;
        unsigned long link;
        unsigned long xer;
        unsigned long ccr;
        unsigned long mq;               
        unsigned long trap;             
        unsigned long dar;              
        unsigned long dsisr;            
        unsigned long result;           
    };

  The offset used by ptrace is also defined for this architecture
  in ppc-asm/ptrace.h 

  GPR0  = RT_R0
  GPR1  = RT_R1
  ...
  GPR31 = RT_R31)

  Also, the header file defins PT_FPR0 = 48 with a caveat that 
  Each FP reg occupies 2 slots in this space. PT_FPR31 (PT_FPR0 + 2*31)
  and PT_FPSCR (PT_FPR0 + 2*32 + 1)
 */
 
class linux_ppc_gpr_set_t
  : public register_set_base_t<T_GRS, T_VA, T_WT>
{
 
public:
 
  // constructors and destructor
  //
  linux_ppc_gpr_set_t  ();
  virtual ~linux_ppc_gpr_set_t () { }
 
  virtual void set_pc  (T_VA addr);
  virtual T_VA const get_pc () const;
  virtual T_VA const get_ret_addr() const;
  virtual T_VA const get_memloc_for_ret_addr() const;

private:
  bool LEVELCHK(self_trace_verbosity level)
       { return (self_trace_t::tracer_module_trace.verbosity_level >= level); }

  std::string MODULENAME;
};
 
//! class linux_ppc_frs_set_t
/*!
    The class represents linux ppc general purpose register set.

    The offset in the USER area is calculated as following.
    (This is defined in sys/user.h)

    === ppc (32 bit process) ===

    struct user {
        struct pt_regs  regs;
        size_t          u_tsize;
        size_t          u_dsize;
        size_t          u_ssize;
        unsigned long   start_code;
        unsigned long   start_data;
        unsigned long   start_stack;
        long int        signal;
        struct regs *   u_ar0;
        unsigned long   magic;
        char            u_comm[32];
    };

    Thus, general purpose register set (regs) resides
    at the zero offset. General purpose register set
    (struct pt_regs: defined in ppc-asm/ptrace.h) is

    struct pt_regs {
        unsigned long gpr[32];
        unsigned long nip;
        unsigned long msr;
        unsigned long orig_gpr3;
        unsigned long ctr;
        unsigned long link;
        unsigned long xer;
        unsigned long ccr;
        unsigned long mq;
        unsigned long trap;
        unsigned long dar;
        unsigned long dsisr;
        unsigned long result;
    };

  The offset used by ptrace is also defined for this architecture
  in ppc-asm/ptrace.h

  GPR0  = RT_R0
  GPR1  = RT_R1
  ...
  GPR31 = RT_R31)

  Also, the header file defins PT_FPR0 = 48 with a caveat that
  Each FP reg occupies 2 slots in this space. PT_FPR31 (PT_FPR0 + 2*31)
  and PT_FPSCR (PT_FPR0 + 2*32 + 1)
*/
class linux_ppc_fpr_set_t
  : public register_set_base_t<T_FRS, T_VA, T_WT>
{
 
public:
 
  // constructors and destructor
  //
  linux_ppc_fpr_set_t ();
  virtual ~linux_ppc_fpr_set_t() { }

private:
  bool LEVELCHK(self_trace_verbosity level)
       { return (self_trace_t::tracer_module_trace.verbosity_level >= level); }

  std::string MODULENAME;
};
 
 
//! linux_ppc_thread_t:
/*!
    linux_ppc_thread_t is the linux ppc implementation of thread_base_t
    class. The constructor sets the register model.
*/
class linux_ppc_thread_t
  : public thread_base_t<SDBG_LINUX_DFLT_INSTANTIATION>
{
 
public:

  // constructors and destructor
  //
  linux_ppc_thread_t();
  virtual ~linux_ppc_thread_t();
 
  // virtual method to convert thread id to lwp 
  // which the kernel understand.
  virtual pid_t thr2pid();


private:
  bool LEVELCHK(self_trace_verbosity level)
       { return (self_trace_t::tracer_module_trace.verbosity_level >= level); }

  std::string MODULENAME;
};
 
 
//!
/*!
  linux_ppc_process_t is linux ppc implementation of process_base_t
  class. The constructor sets the register model.
*/
class linux_ppc_process_t
  : public process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>
{
 
public:
 
  // constructors and destructor
  //
  linux_ppc_process_t ();
  linux_ppc_process_t ( const std::string& mi,
                        const std::string& md,
                        const std::string& mt,
                        const std::string& mc );
  linux_ppc_process_t ( const pid_t& pid,
                        const std::string& mi,
                        const std::string& md,
                        const std::string& mt,
                        const std::string& mc );
  linux_ppc_process_t ( const pid_t& pid,
                        const std::string& mi );
 
  explicit linux_ppc_process_t ( const pid_t& pid );
  ~linux_ppc_process_t() { }
 
protected:
  bool basic_init ( const std::string& mi,
                    const std::string& md,
                    const std::string& mt,
                    const std::string& mc );
 
  bool basic_init ( const std::string& mi );
 
private:
  void launcher_symbols_init();
  bool LEVELCHK(self_trace_verbosity level)
       { return (self_trace_t::tracer_module_trace.verbosity_level >= level); }

  std::string MODULENAME; 
};
 
// data structure needed by proc service layer
//
struct ps_prochandle {
  process_base_t<SDBG_LINUX_DFLT_INSTANTIATION> *p;
};

#endif // ARCHITECTURES
#endif // SDBG_LINUX_MACH_HXX
