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
 *        Jun 09 2010 DHA: Use LMON_timestamp to get consistent timestamp format
 *        Feb 09 2008 DHA: Added LLNS Copyright 
 *        Jul 03 2006 DHA: trace method
 *        Mar 30 2006 DHA: File created      
 */ 

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/common.h>
#include <iostream>
#include <cstdarg>
#include <ctime>
#include <sys/time.h>
#include <limits.h>

#include "lmon_api/lmon_say_msg.hxx"
#include "sdbg_self_trace.hxx"

self_trace_t &
self_trace_t::self_trace(void)
{
    static self_trace_t singleton;
    return singleton;
}

FILE *self_trace_t::tracefptr = stdout;


//!  opts_args_t::trace
/*!        
    logs self-tracing event
*/
bool 
self_trace_t::trace ( bool levelchk,
		      const std::string & mn, 
		      bool error_or_info,
		      const char *output, ... )
{
  if (!levelchk) 
    return false;

  va_list ap;
  char log[PATH_MAX];
  const char *ei_str = error_or_info ? "ERROR" : "INFO";
  bool rc = false;

  if (LMON_timestamp(mn.c_str(), ei_str, output, log, PATH_MAX) >= 0)
    {
      va_start(ap, output);
      vfprintf(tracefptr, log, ap);
      va_end(ap);
      rc = true;
    }

  return rc; 
}
