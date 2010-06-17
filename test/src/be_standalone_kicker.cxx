/*
 * $Header: $
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
 *        Jun 12 2008 DHA: Added GNU build system support.
 *        Feb 09 2008 DHA: Added LLNS Copyright.
 *        Jul 30 2006 DHA: Created file.          
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/common.h>

#if HAVE_UNISTD_H
# include <unistd.h>
#else
# error unistd.h is required
#endif
                                                                          
#include <iostream>

#if HAVE_STDARG_H
# include <cstdarg>
#else
# error stdarg.h is required
#endif

#if HAVE_SIGNAL_H
# include <signal.h>
#else
# error signal.h is required
#endif
                                                                          
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#else
# error sys/types.h is required
#endif
                                                                          
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#else
# error sys/wait.h is required
#endif

#if HAVE_LIMITS_H
# include <climits>
#else
# error limits.h is required
#endif

#if TIME_WITH_SYS_TIME
# include <ctime>
# include <sys/time.h>
#else
# error ctime and sys_time.h is required
#endif

void 
LMON_say_msg ( const char* m, const char* output, ... )
{
  va_list ap;
  char log[PATH_MAX];
  struct timeval tv;
  gettimeofday (&tv, NULL);
  sprintf(log, "<%f> %s (%s): %s\n", tv.tv_sec+(tv.tv_usec/100000.0), m, "INFO", output);                                                                                                                                                                                    
  va_start(ap, output);
  vfprintf(stdout, log, ap);
  va_end(ap);
}

int 
main( int argc, char* argv[] )
{
  using namespace std;
  char hostName[PATH_MAX];
  char envName[PATH_MAX];
  char* envRet;
  char* token;
  int rc;
 
  rc = gethostname (hostName, PATH_MAX);
  if ( rc < 0 )
    return -1;

  sprintf (envName, "LAUNCHMON_%s", hostName); 
  envRet = getenv(envName); 

  if ( envRet == NULL )
    return -1;

  token = strtok (envRet, ":");
  if (token == NULL )
    return -1;

  do 
    {
      kill ( atoi(token), SIGCONT); 
    }
  while ( (token = strtok (NULL, ":")) != NULL );
  LMON_say_msg ("[BE KICKER]", "finished sending SIGCONT"); 

  return 0;
}

