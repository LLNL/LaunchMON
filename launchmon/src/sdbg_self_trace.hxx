/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_self_trace.hxx,v 1.5.2.1 2008/02/20 17:37:57 dahn Exp $
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
 *
 *  Update Log:
 *        Feb  09 2008 DHA: Added LLNS Copyright
 *        Jul 03 2006 DHA: added trace method and tracefptr
 *        Mar 30 2006 DHA: File created      
 */ 

#ifndef SDBG_SELF_TRACE_HXX
#define SDBG_SELF_TRACE_HXX 1

#if HAVE_STRING
# include <string>
#else
# error string is required
#endif

enum self_trace_verbosity {
  quiet = 0,
  level1,
  level2,
  level3,
  level4
};

struct self_trace_entry_t {
  self_trace_verbosity verbosity_level;
  std::string module_name;
  std::string module_symbol;
};

struct self_trace_t {
  static self_trace_entry_t launchmon_module_trace;
  static self_trace_entry_t tracer_module_trace;
  static self_trace_entry_t symtab_module_trace;
  static self_trace_entry_t thread_tracer_module_trace;
  static self_trace_entry_t event_module_trace;
  static self_trace_entry_t driver_module_trace;
  static self_trace_entry_t machine_module_trace;
  static self_trace_entry_t opt_module_trace;
  static self_trace_entry_t sighandler_module_trace;

  static bool trace(bool, const std::string&, bool, const char*, ... );
  static FILE* tracefptr;
};

#endif // SDBG_SELF_TRACE_HXX
