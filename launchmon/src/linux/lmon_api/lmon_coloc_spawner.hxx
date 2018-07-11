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
 *        May 31 2012 DHA: Copied the file from the 0.8-middleware-support
 *                         branch.
 *        Jul 02 2010 DHA: Created file.
 */

#ifndef SDBG_COLOC_SPAWNER_HXX
#define SDBG_COLOC_SPAWNER_HXX 1

#include "sdbg_std.hxx"

#include <string.h>
#include <string>
#include <vector>
#include "sdbg_base_spawner.hxx"

const std::string colocstr("coloc");

class spawner_coloc_t : public spawner_base_t {
 public:
  ////////////////////////////////////////////////////////////
  //
  //  Public Interfaces
  //
  spawner_coloc_t() {}

  spawner_coloc_t(int bemasterfd, const std::string &dpath,
                  const std::vector<std::string> &dmonopts,
                  const std::vector<std::string> &hostvect)
      : spawner_base_t(colocstr, std::vector<std::string>(), dpath, dmonopts,
                       hostvect) {
    m_is_master = false;
    m_is_fe = true;
    m_be_master_sockfd = bemasterfd;
    m_broadcast = NULL;
  }

  spawner_coloc_t(bool ismaster, int bemasterfd, int (*bcast)(void *, int))
      : spawner_base_t(colocstr, std::vector<std::string>()) {
    m_is_fe = false;
    m_is_master = ismaster;
    m_be_master_sockfd = bemasterfd;
    m_broadcast = bcast;
  }

  virtual ~spawner_coloc_t() {
    if (!m_pid_vect.empty()) {
      m_pid_vect.clear();
    }
  }

  virtual bool spawn();

 private:
  explicit spawner_coloc_t(const spawner_coloc_t &s) {
    // does nothing
  }

  bool execute_daemon();

  bool do_frontend();
  bool do_bemaster();
  bool do_beslave();
  bool parse_mw_assist_lmonpl(char *pl);

  bool m_is_fe;
  bool m_is_master;
  int m_be_master_sockfd;
  int (*m_broadcast)(void *, int);

  std::vector<pid_t> m_pid_vect;
};

#endif  // SDBG_COLOC_SPAWNER_HXX

/*
 * ts=2 sw=2 expandtab
 */
