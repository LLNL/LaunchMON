/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/lmon_api/lmon_say_msg.cxx,v 1.5.2.2 2008/02/20 17:37:57 dahn Exp $
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
 *        Dec  23 2009 DHA: Added explict config.h inclusion 
 *        Feb  09 2008 DHA: Added LLNS Copyright
 *        Dec  29 2006 DHA: Created file.          
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/lmon_api_std.h>

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
#endif

#if HAVE_IOSTREAM
# include <iostream>
#else
# error iostream is required
#endif

#if HAVE_STDARG_H
# include <cstdarg>
#else
# error stdarg.h is required
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#else
# error unistd.h is required 
#endif

#if TIME_WITH_SYS_TIME
# include <ctime>
# include <sys/time.h>
#else
# error ctime and sys_time.h is required
#endif

#if HAVE_LIMITS_H
# include <limits.h>
#else
# error limits.h is required
#endif

#include <lmon_api/lmon_say_msg.hxx>

const int pipe_t::uninitializedFileDescriptor = -1;
int (*errorCB) (const char *format, va_list ap) = NULL;

pipe_t::pipe_t () 
  : readingFd (uninitializedFileDescriptor), 
    writingFd (uninitializedFileDescriptor)
{
  // other init?
}

pipe_t::pipe_t ( const int rfd, const int wfd ) 
{
  readingFd = rfd;
  writingFd = wfd;
}

pipe_t::pipe_t ( const pipe_t& dp )
{
  readingFd = dp.readingFd;
  writingFd = dp.writingFd;
}

pipe_t::~pipe_t ()
{
  // destoying routines?
}

double gettimeofdayD ()
{
  struct timeval ts;
  double rt;

  gettimeofday (&ts, NULL);
  rt = (double) (ts.tv_sec);
  rt += (double) ((double)(ts.tv_usec))/1000000.0;

  return rt;
}

void 
LMON_say_msg ( const char* m, bool error_or_info, const char* output, ... )
{
  using namespace std;

  va_list ap;

  char timelog[PATH_MAX];
  char log[PATH_MAX];
  const char *format = "%b %d %T";
  time_t t;            
  string ei_str = error_or_info ? "ERROR" : "INFO";
 
  time(&t);
  strftime ( timelog, PATH_MAX, format, localtime(&t) );
  sprintf(log, "<%s> %s (%s): %s\n", timelog, m, ei_str.c_str(), output);

  va_start(ap, output);
  if (errorCB)
    {
      errorCB (log, ap);
    }
  else
    {
      vfprintf(stdout, log, ap);
      fflush(stdout);	
    }
  va_end(ap);
}

void
LMON_TotalView_debug ()
{
  char cmd[128];
  char exen[128];
  char lk[128];
  int len;  
  sprintf(cmd, "%d", getpid());
  sprintf(exen, "/proc/%d/exe", getpid());
  len = readlink (exen, lk, 128);
  if (len >= 0 )
    lk[len]='\0';
  	
  if (!fork())
    {
      std::cout << "FE DEBUG SUPPORT: invoking totalview" << std::endl;
      execlp ("totalview", "totalview", "-pid", cmd, lk, (char*) 0 );
    }
  int stuck = 0;

  // DEBUGGING SUPPORT
  // change the value of "stuck" to be 1 to break out of 
  // this loop.
  // 	
  while (!stuck);
}
