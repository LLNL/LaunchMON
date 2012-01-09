/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/sdbg_linux_std.hxx,v 1.5.2.2 2008/02/20 17:37:57 dahn Exp $
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
 *        Dec  16 2009 DHA: Moved backtrace support with C++ demangling here
 *        Aug  10 2009 DHA: Added more comments
 *        Mar  06 2008 DHA: Deprecate GLUESYM support
 *        Mar  11 2008 DHA: Added PowerPC support (BlueGene FEN)
 *        Feb  09 2008 DHA: Added LLNS Copyright
 *        Aug  15 2007 DHA: Change C macroes to C++ style: const char * const 
 *        Jan  09 2006 DHA: Linux X86-64 support
 *        Mar  22 2006 DHA: Created file.
 *
 */ 

#ifndef SDBG_LINUX_STD_HXX
#define SDBG_LINUX_STD_HXX 1

extern "C" {
#if HAVE_EXECINFO_H
# include <execinfo.h>
#else
#error execinfo.h is required 
#endif

#if HAVE_LIBGEN_H
# include <libgen.h>
#else
# error libgen.h is required
#endif

#if HAVE_STDDEF_H
# include <stddef.h>
#endif
}

#if HAVE_STRING
# include <string>
#else
# error string is required
#endif

#if HAVE_CXXABI_H
# if HAVE_STDDEF_H
#  include <cxxabi.h> /* picking up demangling service */
# endif
#endif

//! LAUNCH_BREAKPOINT:
/*!
    Symbol for the dummy function that gets invoked
    by the RM to notify state changes of the target job
*/
const char * const LAUNCH_BREAKPOINT        = "MPIR_Breakpoint";

//! LAUNCH_BEING_DEBUG:
/*!
    Symbol that tells the RM whether it is traced or not
*/
const char * const LAUNCH_BEING_DEBUG       = "MPIR_being_debugged";

//! LAUNCH_DEBUG_STATE:
/*!
    Symbol that tells the engine the state of the job when
    LAUNCH_BREAKPOINT is called
*/
const char * const LAUNCH_DEBUG_STATE       = "MPIR_debug_state";

//! LAUNCH_DEBUG_GATE:
/*!
    Symbol to tells the job to gate the starting of the job
*/
const char * const LAUNCH_DEBUG_GATE        = "MPIR_debug_gate";

//! LAUNCH_PROCTABLE:
/*!
    Symbol that holds an array of MPI process table
*/
const char * const LAUNCH_PROCTABLE         = "MPIR_proctable";

//! LAUNCH_PROCTABLE:
/*!
    Symbol that holds the size of LAUNCH_PROCTABLE
*/
const char * const LAUNCH_PROCTABLE_SIZE    = "MPIR_proctable_size";

//! LAUNCH_ACQUIRED_PREMAIN:
/*!
    Symbol that tells whether a debugger can force the main 
    for the first attach point
*/
const char * const LAUNCH_ACQUIRED_PREMAIN  = "MPIR_acquired_pre_main";

//! LAUNCH_EXEC_PATH:
/*!
    Symbol that provides the tool daemon exectuable path to the RM
*/
const char * const LAUNCH_EXEC_PATH         = "MPIR_executable_path";

//! LAUNCH_SERVER_ARGS:
/*!
    Symbol that provides the tool daemons with their arguments
*/
const char * const LAUNCH_SERVER_ARGS       = "MPIR_server_arguments";

//! LAUNCH_ATTACH_FIFO:
/*!
    Symbol that provides the tool daemons with their arguments
*/
const char * const LAUNCH_ATTACH_FIFO       = "MPIR_attach_fifo";

//! LOADER_BP_SYM:
/*!
    Symbol within the dynamic linker, which gets invoked on shared library 
    load
*/
const char * const LOADER_BP_SYM            = "_dl_debug_state";

//! LOADER_R_DEBUG:
/*!
    Symbol within the dynamic linker, which holds the current link map
*/
const char * const LOADER_R_DEBUG           = "_r_debug";

//! LOADER_START:
/*!
    Symbol within the dynamic linker, which is the first function to be
    called after exec
*/
const char * const LOADER_START             = "_start";

//! NPTL_CREATE_SYM:
/*!
    Symbol within the POSIX thread library, which is invokded when a 
    new thread is created
*/
const char * const NPTL_CREATE_SYM          = "__nptl_create_event";

//! NPTL_CREATE_SYM:
/*!
    Symbol within the POSIX thread library, which is invokded when a 
    thread dies/exits
*/
const char * const NPTL_DEATH_SYM           = "__nptl_death_event";

//! FORK_SYM:
/*!
    Symbol within the libc for the normal fork call 
*/
const char * const FORK_SYM                 = "__fork";

//! VFORK_SYM:
/*!
    Symbol within the libc for the virtual fork call 
*/
const char * const VFORK_SYM                = "__vfork";

const char * const LIBC_IDEN                = "libc.";
const char * const LIBPTHREAD_IDEN          = "libpthread."; 
const char * const RESOURCE_HANDLER_SYM     = "totalview_jobid";
const char * const ERRMSG_PTRACE            = " error returned from ptrace ";
const char * const ERRMSG_KILL              = " error returned from kill ";
const char * const LIBTHREAD_DB             = "libthread_db.so.1";

const int MAX_LIB_PATH                      = 128;
const int MAX_STRING_SIZE                   = 1024;
const int BPCHAINMAX                        = 128;

inline 
bool glic_backtrace_wrapper (std::string &bt)
{
  using namespace std;
  void *StFrameArray[BPCHAINMAX] = {0};
  char **stacksymbols, **ssUsed;
  int size;
  int i;
  string mybt;

  if ( (size = backtrace (StFrameArray, BPCHAINMAX)) <= 0 )
    {
      return false;
    }

  size = (size > BPCHAINMAX) ? BPCHAINMAX: size;
  stacksymbols = backtrace_symbols (StFrameArray, BPCHAINMAX);

#if HAVE_STDDEF_H && HAVE_CXXABI_H 
  /* demangle support */
  char **demangleStSyms = (char **) malloc (size * sizeof(char *));
  size_t dnSize; 
  if (!demangleStSyms) 
    ssUsed = stacksymbols;
  else 
    {
      int i;
      string delims("()+[]");
      /* starting from 2 to remove two top stack frames */
      for (i=2; i < size; ++i) 
	{
	  char *demangledName;
	  int status;
	  string targetStr (stacksymbols[i]);
	  string binName, funcName, newStr;
	  string::size_type begIdx, endIdx;
	  
	  begIdx = targetStr.find_first_not_of (delims); 	
	  endIdx = targetStr.find_first_of (delims, begIdx);
	  if ( (begIdx == string::npos) || (endIdx == string::npos) ) 
	    binName = "BinaryNameNA";
	  else
	    {
	      binName = targetStr.substr(begIdx, endIdx-begIdx);
	      char *bn = strdup (binName.c_str());
	      binName = basename(bn);
	      free(bn);
	    }
	  
	  begIdx = targetStr.find_first_not_of (delims, endIdx);
	  endIdx = targetStr.find_first_of (delims, begIdx);
	  if ( (begIdx == string::npos) || (endIdx == string::npos) ) 
	    funcName = "FuncNameNA";
	  else
	    {
	      funcName = targetStr.substr(begIdx, endIdx-begIdx);
	      
	      demangledName = abi::__cxa_demangle (funcName.c_str(), NULL, NULL, &status);
	      if ( status < 0 )
		newStr = binName + ": " + funcName;
	      else
		newStr = binName + ": " + demangledName;
	      demangleStSyms[i] = strdup (newStr.c_str());
	    }
	}
      ssUsed = demangleStSyms;
    }
#else
  ssUsed = stacksymbols;
#endif

  bt = "BACKTRACE: \n";  
  /* starting from 2 to remove two top stack frames */  
  for( i=2 ; i < size ; ++i)
    {
      bt += ssUsed[i]; 
      bt += "\n";
    }

  return true;
}


#if X86_ARCHITECTURE

  // 
  // 
  // insert linux X86 architecture macros here ...
  //
  //
  //

  typedef u_int32_t                             T_VA;
  typedef u_int32_t                             T_WT;
  //
  // For the purpose of what we are doing, the fixed-length
  // T_IT is fine to represent an instruction
  //
  typedef u_int32_t                             T_IT;
  typedef struct user_regs_struct               T_GRS;
  typedef struct user_fpregs_struct             T_FRS;

  const T_IT T_TRAP_INSTRUCTION               = 0x000000cc;
  const T_IT T_BLEND_MASK                     = 0xffffff00;
  const T_IT IT_UNINIT_HEX                    = 0xdeadbeef;
  const T_VA T_UNINIT_HEX                     = 0xdeadbeef;

#define SDBG_LINUX_DFLT_INSTANTIATION T_VA, \
                                      T_WT, \
                                      T_IT, \
                                      T_GRS,\
                                      T_FRS,\
                                      td_thrinfo_t,\
                                      elf_wrapper

#elif X86_64_ARCHITECTURE

  // 
  // 
  // insert linux X86-64 architecture macros here ...
  //
  //
  //

# if BIT64
  //
  // if the target is 64 bit, use the following
  //
  typedef u_int64_t                             T_VA;
  typedef u_int64_t                             T_WT;
  typedef u_int64_t                             T_IT;
  typedef struct user_regs_struct               T_GRS;
  typedef struct user_fpregs_struct             T_FRS;
  const T_IT T_TRAP_INSTRUCTION               = 0x00000000000000cc;
  const T_IT T_BLEND_MASK                     = 0xffffffffffffff00;
  const T_IT IT_UNINIT_HEX                    = 0xdeadbeefdeadbeeffULL;
  const T_VA T_UNINIT_HEX                     = 0xdeadbeefdeadbeef;
# else
  //
  // if the target is 32 bit, use the following
  //
  typedef u_int32_t                             T_VA;
  typedef u_int32_t                             T_WT;
  typedef u_int32_t                             T_IT;
  typedef struct user_regs_struct               T_GRS;
  typedef struct user_fpregs_struct             T_FRS;
  const T_IT T_TRAP_INSTRUCTION               = 0x000000cc;
  const T_IT T_BLEND_MASK                     = 0xffffff00;
  const T_IT IT_UNINIT_HEX                    = 0xdeadbeef;
  const T_VA T_UNINIT_HEX                     = 0xdeadbeef;
# endif // BIT64

#define SDBG_LINUX_DFLT_INSTANTIATION T_VA, \
                                      T_WT, \
                                      T_IT, \
                                      T_GRS,\
                                      T_FRS,\
                                      td_thrinfo_t,\
                                      elf_wrapper


#elif PPC_ARCHITECTURE

  // 
  // 
  // Insert linux ppc architecture macroes here ...
  // BlueGene FEN architecture is covered here.
  //
  //

# if BIT64
  //
  // if the target is 64 bit, use the following
  //
  typedef u_int64_t                             T_VA;
  typedef u_int64_t                             T_WT;
  typedef u_int32_t                             T_IT;
  typedef struct pt_regs                        T_GRS;
  typedef struct pt_regs                        T_FRS;
  const T_IT T_TRAP_INSTRUCTION               = 0x7d821008;
  const T_IT T_BLEND_MASK                     = 0x00000000;
  const T_IT IT_UNINIT_HEX                    = 0xdeadbeef;
  const T_VA T_UNINIT_HEX                     = 0xdeadbeefdeadbeefULL;
# else
  typedef u_int32_t                             T_VA;
  typedef u_int32_t                             T_WT;
  typedef u_int32_t                             T_IT;
  typedef struct pt_regs                        T_GRS;
  typedef struct pt_regs                        T_FRS;
  const T_IT T_TRAP_INSTRUCTION               = 0x7d821008;
  const T_IT T_BLEND_MASK                     = 0x00000000;
  const T_IT IT_UNINIT_HEX                    = 0xdeadbeef;
  const T_VA T_UNINIT_HEX                     = 0xdeadbeef;
# endif // BIT64

#define SDBG_LINUX_DFLT_INSTANTIATION T_VA, \
                                      T_WT, \
                                      T_IT, \
                                      T_GRS,\
                                      T_FRS,\
                                      td_thrinfo_t,\
                                      elf_wrapper
#endif // ARCHITECTURES
#endif // SDBG_LINUX_STD_HXX
