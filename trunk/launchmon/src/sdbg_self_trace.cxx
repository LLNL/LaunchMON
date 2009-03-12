/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_self_trace.cxx,v 1.7.2.1 2008/02/20 17:37:57 dahn Exp $
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
 *        Feb 09 2008 DHA: Added LLNS Copyright 
 *        Jul 03 2006 DHA: trace method
 *        Mar 30 2006 DHA: File created      
 */ 

#include <lmon_api/common.h>
#include <iostream>
#include <cstdarg>

#if TIME_WITH_SYS_TIME 
# include <ctime>
# include <sys/time.h>
#else
# error ctime and sys_time.h is required 
#endif

#include "sdbg_self_trace.hxx"

self_trace_entry_t self_trace_t::launchmon_module_trace 
     =  { quiet, "Launchmon", "launchmon" };

self_trace_entry_t self_trace_t::tracer_module_trace 
     = { quiet, "ProcTracer", "tracer" };

self_trace_entry_t self_trace_t::symtab_module_trace 
     = { quiet, "Symtable", "symtab" };

self_trace_entry_t self_trace_t::thread_tracer_module_trace 
     = { quiet, "ThreadTracer", "ttracer" };

self_trace_entry_t self_trace_t::machine_module_trace 
     = { quiet, "Machine", "machine"};

self_trace_entry_t self_trace_t::event_module_trace
     = { quiet, "EventMan", "event"};

self_trace_entry_t self_trace_t::driver_module_trace
     = { quiet, "Driver", "driver"};

self_trace_entry_t self_trace_t::opt_module_trace
     = { quiet, "OptionParsing", "option"};

self_trace_entry_t self_trace_t::sighandler_module_trace
     = { quiet, "SigHandler", "sighandler"};


FILE* self_trace_t::tracefptr = stdout;


//!  opts_args_t::trace
/*!        
    logs self-tracing event
*/
bool 
self_trace_t::trace ( bool levelchk, 		      
		      const std::string& mn, 
		      bool error_or_info,
		      const char* output, ... )
{
  using namespace std;

  va_list ap;
  
  char timelog[PATH_MAX];
  char log[PATH_MAX];
  const char* format = "%b %d %T";
  //struct timeval tv;  
  time_t t;

  if (!levelchk) 
    return false;

  string ei_str = error_or_info ? "ERROR" : "INFO";

  time(&t);
  strftime ( timelog, PATH_MAX, format, localtime(&t) );
  sprintf(log, "<%s> %s (%s): %s\n", timelog, mn.c_str(), ei_str.c_str(), output);

  va_start(ap, output);
  vfprintf(tracefptr, log, ap);
  va_end(ap);

  return true; 
}
