/*
 * $Header: $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008 ~ 2010, Lawrence Livermore National Security, LLC. Produced at 
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
 *        May 31 2012 DHA: Copied from the 0.8-middleware-support branch
 *        Jan 17 2011 JDG: Fixed unsigned int < 0 bug
 *        Jul 06 2010 DHA: Created file.
 */

#include "sdbg_std.hxx"

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
#endif

#if HAVE_STDIO_H
# include <cstdio>
#else
# error stdio.h is required
#endif

#if HAVE_STDLIB_H
# include <cstdlib>
#else
# error stdlib.h is required
#endif

#if HAVE_STRING
# include <string>
#else
# error string is required
#endif

#if HAVE_STRING_H
# include <string.h>
#else
# error string.h is required
#endif

#if HAVE_VECTOR
# include <vector>
#else
# error vector is required
#endif

#include "lmon_api/lmon_say_msg.hxx"
#include "sdbg_rsh_spawner.hxx"


//////////////////////////////////////////////////////////////////////////
//
//   Public Interface
//
bool
spawner_rsh_t::spawn()
{
  if (get_hosts_vector().empty())
    {
      return false;
    }

  unsigned int i=0;
  unsigned int j=0;
  unsigned int halfhosts = get_hosts_vector().size() / 2;
  unsigned int right_start_index = halfhosts + 1;
  unsigned int left_start_index = 1; 
  bool lefton = false;
  bool righton = false;
  std::string leftrsh = std::string(LMON_RSHSPAWNER_OPT) + std::string("=");
  std::string rightrsh = leftrsh;

  if (get_hosts_vector().size() > 0)
    {
      lefton = true;

      if (halfhosts > 0)
        righton = true;
    }


  if (righton)
    {
      for (i=right_start_index; i < (get_hosts_vector().size()-1); ++i)
        {
          rightrsh += get_hosts_vector()[i] + std::string(LMON_HOST_DELIM);
        }

      if (i == (get_hosts_vector().size()-1))
        {
          rightrsh += get_hosts_vector()[i++];
        }

      if (i == right_start_index)
        {
          rightrsh += LMON_NO_HOST;
        }
    }

  if (lefton)
    {
      for (j=left_start_index; j+1 < halfhosts; ++j)
        {
            leftrsh += get_hosts_vector()[j] + std::string(LMON_HOST_DELIM);;
        }

      if (j+1 == halfhosts)
        {
          leftrsh += get_hosts_vector()[j++];
        }

      if (j == left_start_index)
        {
          leftrsh += LMON_NO_HOST;
        }
    }

  if (lefton)
    {
      if ( !(rightpid = fork()))
        {
          if (!execute_rsh(get_hosts_vector()[0], leftrsh))
            {
              //
              // This is within the child process that failed to exec
              // So ** SINK HERE **
              //
              exit(1);
            }
         }
    }
  
  if (righton)
    {
      if ( !(leftpid = fork()))
        {
          if (!execute_rsh(get_hosts_vector()[halfhosts], rightrsh))
            {
              //
              // This is within the child process that failed to exec
              // So ** SINK HERE **
              // 
              exit(1);
            }
        }
    }

  return lefton;
}


//////////////////////////////////////////////////////////////////////////
//
//   Private Methods
//
bool
spawner_rsh_t::execute_rsh(const std::string &headhost, const std::string &hostsarg)
{
  //
  // compute required malloc size
  // 1: launcher command
  // n: launcher command args
  // 1: headhost
  // 1: deamonpath
  // m: daemonargs
  // 1: LMON_RSHSPAWNER_OPT option 
  // 1: null-termination

  int nargvs = get_launch_cmd_args().size() + get_daemon_args().size() + 5;
  int i = 0;
  char **av = (char **) malloc (nargvs * sizeof(av));
  av[i++] = strdup (get_remote_launch_cmd().c_str());
  std::vector<std::string>::const_iterator iter;
  for (iter = get_launch_cmd_args().begin(); 
         iter != get_launch_cmd_args().end(); ++iter)
    {
      av[i++] = strdup((*iter).c_str());
    }

  av[i++] = strdup(headhost.c_str());
  av[i++] = strdup(get_daemon_path().c_str());

  for (iter = get_daemon_args().begin(); iter != get_daemon_args().end(); ++iter)
    {
      av[i++] = strdup ((*iter).c_str());
    }

  av[i++] = strdup(hostsarg.c_str());
  av[i++] = NULL;

  //
  // We use excvp so that we don't have to copy environ
  // If works, this is SINK
  //
  if ( execvp ( av[0], av) < 0 )
    {
      return false;
    }

  //
  // Unreachable but to satisfy the compiler
  //
  return true;
}

