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
 *        Jul 02 2010 DHA: Created file.
 */

#ifndef SDBG_BASE_SPAWNER_HXX
#define SDBG_BASE_SPAWNER_HXX 1

#include "sdbg_std.hxx"

#if HAVE_STRING
# include <string>
#else
# error string is required
#endif 

#if HAVE_VECTOR
# include <vector>
#else
# error vector is required
#endif 

const char LMON_HOST_DELIM[]=":";

class spawner_base_t
{
public:
  ////////////////////////////////////////////////////////////
  //
  //  Public Interfaces
  //
  spawner_base_t()
    {
      // Empty
    }

  spawner_base_t(const std::string &rac,
		 const std::vector<std::string> &racargs)
    {
      //
      // Copy remote access command like rsh
      //
      remote_launch_cmd = rac;

      //
      // Copy remote access command arguments
      //
      launch_cmd_args = racargs;
    }

  spawner_base_t(const std::string &rac,
		 const std::vector<std::string> &racargs,
		 const std::string &dpath,
		 const std::vector<std::string> &dmonopts)
    {
      //
      // Copy remote access command like rsh
      //
      remote_launch_cmd = rac;

      //
      // Copy remote access command arguments
      //
      launch_cmd_args = racargs;

      //
      // Copy daemon path
      //
      daemon_path = dpath;

      //
      // Copy daemon args
      //
      daemon_args = dmonopts;
    }

  spawner_base_t(const std::string &rac,
		 const std::vector<std::string> & racargs,
		 const std::string &dpath,
		 const std::vector<std::string> &dmonopts,
		 const std::vector<std::string> &hosts)
    {
      //
      // Copy remote access command like rsh
      //
      remote_launch_cmd = rac;

      //
      // Copy remote access command arguments
      //
      launch_cmd_args = racargs;

      //
      // Copy daemon path
      //
      daemon_path = dpath;

      //
      // Copy daemon args
      //
      daemon_args = dmonopts;

      //
      // Copy hosts: the semantics of vector to vector assignment is deep copy.
      // Note that though, a vector element is string which does "deep copy"
      // on write so you may see the same c_str poiner addresses between src and dest.
      // Bottom line: this is safe.
      //
      m_hosts = hosts;
    }

  virtual ~spawner_base_t()
    {
      if (!launch_cmd_args.empty())
        {
	  launch_cmd_args.clear();
        }

      if (daemon_args.empty())
        {
	  daemon_args.clear();
        }

      if (m_hosts.empty())
        {
	  m_hosts.clear();
        }
    }

  //
  // Method that starts remote process spawning
  //  platform-specific derive class must implment this method.
  //  Its role is simply spawn the daemon executable to a set of 
  //  hosts -- a simple hostlist, colocation with BE daemons, 
  //           existing or newly-allocated set of hosts
  virtual bool spawn()                                          = 0;

  //
  // Method that append my hosts to the combined hosts
  //
  //
  virtual bool combineHosts(std::vector<std::string> &combHosts)
    {
      std::vector<std::string>::const_iterator iter;
      int s = combHosts.size();

      for (iter = m_hosts.begin(); iter != m_hosts.end(); ++iter)
        {
          combHosts.push_back((*iter));
        }

      return ((combHosts.size() - s) > 0)? true : false;
    }


  ////////////////////////////////////////////////////////////
  //
  //  Accessors
  //
  define_gset(std::string, remote_launch_cmd)
  define_gset(std::string, daemon_path)
  define_gset(std::string, command_str)
  define_gset(std::string, err_str)

  std::vector<std::string> & get_hosts_vector()
    {
      return m_hosts;
    }

  std::vector<std::string> & get_launch_cmd_args()
    {
      return launch_cmd_args;
    }

  std::vector<std::string> & get_daemon_args()
    {
      return daemon_args;
    }

  //
  // Adds the hostlist (null-terminated list), hl, to my hostlist
  //   Returns the number of hosts in the hostlist
  //
  int addHosts(const char *hl[])
    {
      int i;
      int nhosts = m_hosts.size();
      if (!hl)
	{
          return nhosts;
	}

      for (i=0; hl[i] != NULL; i++)
        {
          std::string tmph = hl[i];
          m_hosts.push_back(tmph);
        }

      nhosts = m_hosts.size();
      return nhosts;
    }

private:

  //
  // Won't allow copying of this object
  //
  explicit spawner_base_t (const spawner_base_t & s)
    {
      // does nothing
    }

  std::string remote_launch_cmd;
  std::vector<std::string> launch_cmd_args;
  std::string daemon_path;
  std::vector<std::string> daemon_args;
  std::vector<std::string> m_hosts;
  std::string command_str;
  std::string err_str;
};

#endif // SDBG_BASE_SPAWNER_HXX
