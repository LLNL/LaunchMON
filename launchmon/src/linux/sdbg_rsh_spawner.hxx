/*
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008, Lawrence Livermore National Security, LLC. Produced at
 * the Lawrence Livermore National Laboratory. Written by Dong H. Ahn
 * <ahn1@llnl.gov>. LLNL-CODE-409469. All rights reserved.
 *
 * This file is part of LaunchMON. For details, see
 * https://computing.llnl.gov/?set=resources&page=os_projects
 *
 * Please also read LICENSE.txt -- Our Notice and GNU Lesser General Public
 * License.
 *
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *--------------------------------------------------------------------------------
 *
 *  Update Log:
 *        May 31 2012 DHA: Copied from the 0.8-middleware-support branch
 *        Jul 02 2010 DHA: Created file.
 */

#ifndef SDBG_RSH_SPAWNER_HXX
#define SDBG_RSH_SPAWNER_HXX 1

#include "sdbg_std.hxx"

#include <string.h>
#include <string>
#include <vector>
#include "sdbg_base_spawner.hxx"

const char LMON_RSHSPAWNER_OPT[] = "--lmon-rsh";
const char LMON_NO_HOST[] = "nohost";

class spawner_rsh_t : public spawner_base_t {
 public:
  ////////////////////////////////////////////////////////////
  //
  //  Public Interfaces
  //
  spawner_rsh_t() {}

  spawner_rsh_t(const std::string &rac, const std::vector<std::string> &racargs,
                const std::string &dpath,
                const std::vector<std::string> &dmonopts)
      : spawner_base_t(rac, racargs, dpath, dmonopts) {}

  //
  // ctor to use when a standard RSHCMD will
  // perform what you want
  //
  spawner_rsh_t(const std::string &dpath,
                const std::vector<std::string> &dmonopts)
      : spawner_base_t(std::string(RSHCMD), std::vector<std::string>(), dpath,
                       dmonopts) {}

  spawner_rsh_t(const std::string &rac, const std::vector<std::string> &racargs,
                const std::string &dpath,
                const std::vector<std::string> &dmonopts,
                const std::vector<std::string> &hosts)
      : spawner_base_t(rac, racargs, dpath, dmonopts, hosts) {}

  //
  // ctor to use when a standard RSHCMD will
  // perform what you want
  //
  spawner_rsh_t(const std::string &dpath,
                const std::vector<std::string> &dmonopts,
                const std::vector<std::string> &hosts)
      : spawner_base_t(std::string(RSHCMD), std::vector<std::string>(), dpath,
                       dmonopts, hosts) {}

  spawner_rsh_t(const std::string &rac, const std::vector<std::string> &racargs,
                const std::string &dpath,
                const std::vector<std::string> &dmonopts, const char *hlsarg)
      : spawner_base_t(rac, racargs, dpath, dmonopts) {
    char *hlsargcp = strdup(hlsarg);
    const char *token = strtok(hlsargcp, LMON_HOST_DELIM);

    while (token) {
      if (strcmp(token, LMON_NO_HOST) != 0) {
        get_hosts_vector().push_back(std::string(token));
      }
      token = strtok(NULL, LMON_HOST_DELIM);
    }
  }

  //
  // ctor to use when a standard RSHCMD will
  // perform what you want over a delimited hostlist c-string
  //
  spawner_rsh_t(const std::string &dpath,
                const std::vector<std::string> &dmonopts, const char *hlsarg)
      : spawner_base_t(std::string(RSHCMD), std::vector<std::string>(), dpath,
                       dmonopts) {
    char *hlsargcp = strdup(hlsarg);
    const char *token = strtok(hlsargcp, LMON_HOST_DELIM);

    while (token) {
      if (strcmp(token, LMON_NO_HOST) != 0) {
        get_hosts_vector().push_back(std::string(token));
      }
      token = strtok(NULL, LMON_HOST_DELIM);
    }
  }

  virtual ~spawner_rsh_t() {}

  virtual bool spawn();

 private:
  explicit spawner_rsh_t(const spawner_rsh_t &s) {
    // does nothing
  }

  bool execute_rsh(const std::string &headhost, const std::string &hostsarg);

  pid_t leftpid;
  pid_t rightpid;
};

#endif  // SDBG_RSH_SPAWNER_HXX

/*
 * ts=2 sw=2 expandtab
 */
