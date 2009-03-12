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
 *        Mar  06 2008 DHA: Deprecate GLUESYM support
 *        Mar  11 2008 DHA: Added PowerPC support (BlueGene FEN)
 *        Feb  09 2008 DHA: Added LLNS Copyright
 *        Aug  15 2007 DHA: Change C macroes to C++ style: const char* const 
 *        Jan  09 2006 DHA: Linux X86-64 support
 *        Mar  22 2006 DHA: Created file. 
 *                   
 */ 

#ifndef SDBG_LINUX_STD_HXX
#define SDBG_LINUX_STD_HXX 1

const char * const LAUNCH_BREAKPOINT        = "MPIR_Breakpoint";
const char * const LAUNCH_BEING_DEBUG       = "MPIR_being_debugged";
const char * const LAUNCH_DEBUG_STATE       = "MPIR_debug_state";
const char * const LAUNCH_DEBUG_GATE        = "MPIR_debug_gate";
const char * const LAUNCH_PROCTABLE         = "MPIR_proctable";
const char * const LAUNCH_PROCTABLE_SIZE    = "MPIR_proctable_size";
const char * const LAUNCH_ACQUIRED_PREMAIN  = "MPIR_acquired_pre_main";
const char * const LAUNCH_EXEC_PATH         = "MPIR_executable_path";
const char * const LAUNCH_SERVER_ARGS       = "MPIR_server_arguments";
const char * const LOADER_BP_SYM            = "_dl_debug_state";
const char * const LOADER_R_DEBUG           = "_r_debug";
const char * const LOADER_START             = "_start";
const char * const NPTL_CREATE_SYM          = "__nptl_create_event";
const char * const NPTL_DEATH_SYM           = "__nptl_death_event";
const char * const FORK_SYM                 = "__fork";
const char * const VFORK_SYM                = "__vfork";
const char * const LIBC_IDEN                = "libc.";
const char * const LIBPTHREAD_IDEN          = "libpthread."; 
const char * const RESOURCE_HANDLER_SYM     = "totalview_jobid";
const char * const ERRMSG_PTRACE            = " error returned from ptrace ";
const char * const ERRMSG_KILL              = " error returned from kill ";
const char * const LIBTHREAD_DB             = "libthread_db.so.1";

const int MAX_LIB_PATH                      = 128;
const int MAX_STRING_SIZE                   = 1024;

#if X86_ARCHITECTURE

/* 
 * 
 * insert linux X86 architecture macroes here ...
 *
 *
 */

typedef u_int32_t                             T_VA;
typedef u_int32_t                             T_WT;
typedef u_int32_t                             T_IT;
typedef struct user_regs_struct               T_GRS;
typedef struct user_fpregs_struct             T_FRS;

const T_VA T_TRAP_INSTRUCTION               = 0x000000cc;
const T_VA T_BLEND_MASK                     = 0xffffff00;
const T_VA T_UNINIT_HEX                     = 0xdeadbeef;

#define SDBG_LINUX_DFLT_INSTANTIATION T_VA, \
                                      T_WT, \
                                      T_IT, \
                                      T_GRS,\
                                      T_FRS,\
                                      td_thrinfo_t,\
                                      elf_wrapper

#elif X86_64_ARCHITECTURE

/*
 *
 * Insert linux X86-64 architecture macroes here ...
 *
 *
 */

#if BIT64 
typedef u_int64_t                             T_VA;
typedef u_int64_t                             T_WT;
typedef u_int64_t                             T_IT;
typedef struct user_regs_struct               T_GRS;
typedef struct user_fpregs_struct             T_FRS;
const T_VA T_TRAP_INSTRUCTION               = 0x00000000000000cc;
const T_VA T_BLEND_MASK                     = 0xffffffffffffff00;
const T_VA T_UNINIT_HEX                     = 0xdeadbeefdeadbeef;
#else
typedef u_int32_t                             T_VA;
typedef u_int32_t                             T_WT;
typedef u_int32_t                             T_IT;
typedef struct user_regs_struct               T_GRS;
typedef struct user_fpregs_struct             T_FRS;
const T_VA T_TRAP_INSTRUCTION               = 0x000000cc;
const T_VA T_BLEND_MASK                     = 0xffffff00;
const T_VA T_UNINIT_HEX                     = 0xdeadbeef;
#endif 


#define SDBG_LINUX_DFLT_INSTANTIATION T_VA, \
                                      T_WT, \
                                      T_IT, \
                                      T_GRS,\
                                      T_FRS,\
                                      td_thrinfo_t,\
                                      elf_wrapper


#elif PPC_ARCHITECTURE

/* 
 * 
 * Insert linux ppc architecture macroes here ...
 * BlueGene FEN class
 *
 */

#if BIT64 
typedef u_int64_t                             T_VA;
typedef u_int64_t                             T_WT;
typedef u_int32_t                             T_IT;
typedef struct pt_regs                        T_GRS;
typedef struct pt_regs                        T_FRS;
const T_IT T_TRAP_INSTRUCTION               = 0x7d821008;
const T_IT T_BLEND_MASK                     = 0x00000000;
const T_VA T_UNINIT_HEX                     = 0xdeadbeefdeadbeefULL;
#else
typedef u_int32_t                             T_VA;
typedef u_int32_t                             T_WT;
typedef u_int32_t                             T_IT;
typedef struct pt_regs                        T_GRS;
typedef struct pt_regs                        T_FRS;
const T_VA T_TRAP_INSTRUCTION               = 0x7d821008;
const T_VA T_BLEND_MASK                     = 0x00000000;
const T_VA T_UNINIT_HEX                     = 0xdeadbeef;
#endif 

#define SDBG_LINUX_DFLT_INSTANTIATION T_VA, \
                                      T_WT, \
                                      T_IT, \
                                      T_GRS,\
                                      T_FRS,\
                                      td_thrinfo_t,\
                                      elf_wrapper

#endif // ARCHITECTURES
#endif // SDBG_LINUX_STD_HXX
