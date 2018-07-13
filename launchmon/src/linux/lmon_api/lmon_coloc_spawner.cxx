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
 *        Jul 07 2010 DHA: Created file.
 */

#include "sdbg_std.hxx"

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
#endif

#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "lmon_api/lmon_api_std.h"
#include "lmon_api/lmon_be.h"
#include "lmon_api/lmon_coloc_spawner.hxx"
#include "lmon_api/lmon_lmonp_msg.h"
#include "lmon_api/lmon_say_msg.hxx"

////////////////////////////////////////////////////////////
//
//  Public Interface
//
bool spawner_coloc_t::spawn() {
  bool rc;

  if (m_is_fe) {
    rc = do_frontend();
  } else {
    if (m_is_master) {
      rc = do_bemaster();
    } else {
      rc = do_beslave();
    }
  }

  return rc;
}

////////////////////////////////////////////////////////////
//
//  Private Methods
//
bool spawner_coloc_t::do_frontend() {
  if (!m_is_fe || m_be_master_sockfd < 0) {
    set_err_str(std::string("do_frontend invoked by non-front-end"));
    return false;
  }

  lmonp_t *msg = NULL;

  int plsize = get_daemon_path().size() + 1;
  std::vector<std::string>::const_iterator iter;
  for (iter = get_daemon_args().begin(); iter != get_daemon_args().end();
       ++iter) {
    plsize += (*iter).size() + 1;
  }
  plsize += 1; /* ending null */
  int msgsize = sizeof(*msg) + plsize;
  if (!(msg = (lmonp_t *)malloc(msgsize))) {
    set_err_str(std::string("malloc returned null"));
    return false;
  }

  set_msg_header(msg, lmonp_fetobe, lmonp_febe_assist_mw_coloc, 0, 0, 0, 0, 0,
                 plsize, 0);

  char *lmonpl = get_lmonpayload_begin(msg);
  memcpy((void *)lmonpl, get_daemon_path().c_str(),
         get_daemon_path().size() + 1);
  lmonpl += get_daemon_path().size() + 1;
  for (iter = get_daemon_args().begin(); iter != get_daemon_args().end();
       ++iter) {
    memcpy(lmonpl, (*iter).c_str(), (*iter).size() + 1);
    lmonpl += (*iter).size() + 1;
  }
  *lmonpl = '\0'; /* ending null */

  if (write_lmonp_long_msg(m_be_master_sockfd, msg, msgsize) < 0) {
    set_err_str(std::string("write_lmonp_long_msg failed"));
    return false;
  }

  free(msg);

  return true;
}

static inline char *get_next_cstr(char *s) {
  if (!s || *s == '\0') return NULL;

  while (*s != '\0') {
    s++;
  }

  return (s + 1);
}

bool spawner_coloc_t::parse_mw_assist_lmonpl(char *pl) {
  if (!pl) return false;

  char *trav = pl;
  set_daemon_path(std::string(trav));

  while ((trav = get_next_cstr(trav))) {
    get_daemon_args().push_back(std::string(trav));
  }

  return true;
}

bool spawner_coloc_t::do_bemaster() {
  if (m_is_fe || !m_is_master) return false;

  lmonp_t msg;

  read_lmonp_msgheader(m_be_master_sockfd, &msg);
  if ((msg.msgclass != lmonp_fetobe) ||
      (msg.type.fetobe_type != lmonp_febe_assist_mw_coloc) ||
      msg.lmon_payload_length == 0) {
    set_err_str(
        std::string("msg mismatch: expecting lmonp_febe_assist_mw_coloc"));
    return false;
  }

  lmonp_t *msg_with_pl =
      (lmonp_t *)malloc(sizeof(lmonp_t) + msg.lmon_payload_length);
  memcpy(msg_with_pl, &msg, sizeof(msg));
  char *lmonpl = get_lmonpayload_begin(msg_with_pl);
  int bytesread = read_lmonp_payloads(m_be_master_sockfd, lmonpl,
                                      msg_with_pl->lmon_payload_length);

  int bsanity =
      msg_with_pl->lmon_payload_length + msg_with_pl->usr_payload_length;
  if (bytesread != bsanity) {
    set_err_str(std::string(
        "Bytes read don't equal the size specified in the received msg"));
    return false;
  }

  uint32_t leng = static_cast<uint32_t>(msg_with_pl->lmon_payload_length);

  // fprintf(stdout, "do_bemaster: before bcast\n");

  if (m_broadcast(&leng, sizeof(leng)) != LMON_OK) {
    set_err_str(std::string("broadcast failed: mw assist lmon payload length"));
    return false;
  }

  // fprintf(stdout, "do_bemaster: before bcast lmonpl\n");

  if (m_broadcast(lmonpl, leng) != LMON_OK) {
    set_err_str(std::string("broadcast failed:  mw assist lmon payload"));
    return false;
  }

  // fprintf(stdout, "do_bemaster: before parse_mw_assist\n");

  if (!parse_mw_assist_lmonpl(lmonpl)) {
    set_err_str(std::string("can't parse the mw assist lmon payload"));
    return false;
  }

  free(msg_with_pl);

  // fprintf(stdout, "do_bemaster: before fork and exec\n");

  //
  // TODO: if LMON will have to deal with more than one daemon per node model
  //   here is the place to handle
  //

  pid_t id;
  if (!(id = fork())) {
    //
    // Child process
    //
    if (!execute_daemon()) {
      /* SINK */
      return false;
    }
  }

  m_pid_vect.push_back(id);

  return true;
}

bool spawner_coloc_t::do_beslave() {
  if (m_is_fe || m_is_master) return false;

  // fprintf(stdout, "do_beslave: before bcast\n");

  uint32_t leng;
  if (m_broadcast(&leng, sizeof(leng)) != LMON_OK) {
    set_err_str(std::string("broadcast failed: mw assist lmon payload length"));
    return false;
  }

  // fprintf(stdout, "do_beslave: before bcast lmonpl\n");

  char *lmonpl = (char *)malloc(leng);
  if (m_broadcast(lmonpl, leng) != LMON_OK) {
    set_err_str(std::string("broadcast failed:  mw assist lmon payload"));
    return false;
  }

  // fprintf(stdout, "do_beslave: before parse_mw_assist\n");

  if (!parse_mw_assist_lmonpl(lmonpl)) {
    set_err_str(std::string("can't parse the mw assist lmon payload"));
    return false;
  }

  free(lmonpl);

  // fprintf(stdout, "do_beslave: before fork and exec\n");

  //
  // TODO: if LMON will have to deal with more than one daemon per node model
  //   here is the place to handle
  //

  pid_t id;
  if (!(id = fork())) {
    //
    // Child process
    //
    if (!execute_daemon()) {
      /* SINK */
      return false;
    }
  }

  m_pid_vect.push_back(id);

  return true;
}

bool spawner_coloc_t::execute_daemon() {
  //
  // compute required malloc size
  // 1: deamonpath
  // m: daemonargs
  // 1: null-termination

  int nargvs = get_daemon_args().size() + 2;
  int i = 0;
  char **av = (char **)malloc(nargvs * sizeof(av));
  std::vector<std::string>::const_iterator iter;

  av[i++] = strdup(get_daemon_path().c_str());

  for (iter = get_daemon_args().begin(); iter != get_daemon_args().end();
       ++iter) {
    av[i++] = strdup((*iter).c_str());
  }

  av[i++] = NULL;

  //
  // We use excvp so that we don't have to copy environ
  // If works, this is SINK
  //
  if (execvp(av[0], av) < 0) {
    return false;
  }

  //
  // Unreachable but to satisfy the compiler
  //
  return true;
}

/*
 * ts=2 sw=2 expandtab
 */
