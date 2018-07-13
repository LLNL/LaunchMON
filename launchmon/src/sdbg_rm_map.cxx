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
 *
 *  Update Log:
 *        Apr 01 2015 ADG: Added Cray CTI support.
 *        Feb 20 2015 andrewg@cray.com: Added support for RMs that build the
 *                         proctable on demand. Checks added for launch helpers
 *                         that are included with launchmon.
 *        Oct 06 2011 DHA: Created file.
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/common.h>

#include <limits.h>
#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Jun 07 2012 DHA TODO: We need to check boost/tokenizer.hpp;
// doing so generates whole lot of errors on some platforms
// so I'm skipping that check for now. We need to
// revisit that issue.
// Effectively, the following currently assumes that boost is ubiquitious
//#if HAVE_BOOST_TOKENIZER_HPP
#include <boost/tokenizer.hpp>
//#else
//# error boost/tokenizer is required; no alternative tokenizer
//#endif

#include <iostream>

#include "sdbg_rm_map.hxx"

///////////////////////////////////////////////////////////////////
//                                                               //
//                 PUBLIC INTERFACES                             //
//                                                               //
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//                                                               //
// PUBLIC METHODS of the resource_manager_t-related classes      //
//                                                               //
///////////////////////////////////////////////////////////////////

rm_id_t::rm_id_t()
    : from(from_unknown_source), method(unknown_method), dtype(unknown_type) {
  id.symbol_name = NULL;
}

rm_id_t::rm_id_t(const rm_id_t &o) {
  from = o.from;
  method = o.method;
  dtype = o.dtype;

  if ((o.method == symbol) && o.id.symbol_name) {
    id.symbol_name = strdup(o.id.symbol_name);
  } else {
    memcpy(&id, &(o.id), sizeof(id));
  }
}

rm_id_t::~rm_id_t() {
  if ((method == symbol) && id.symbol_name) {
    free(id.symbol_name);
  }
}

rm_id_t &rm_id_t::operator=(const rm_id_t &rhs) {
  from = rhs.from;
  method = rhs.method;
  dtype = rhs.dtype;
  if ((rhs.method == symbol) && rhs.id.symbol_name) {
    id.symbol_name = strdup(rhs.id.symbol_name);
  } else {
    memcpy(&id, &(rhs.id), sizeof(id));
  }

  return *this;
}

resource_manager_t::resource_manager_t()
    : rm(RC_none),
      mpir(x_none),
      fail_detection_supported(false),
      launch_string(""),
      expanded_launch_string(""),
      has_launcher_so(false),
      launcher_so_name(""),
      attach_fifo_path("") {}

resource_manager_t::resource_manager_t(const resource_manager_t &r) {
  rm = r.rm;
  mpir = r.mpir;
  launchers = r.launchers;
  launcher_ids = r.launcher_ids;
  job_id = r.job_id;
  launch_helper = r.launch_helper;
  kill_signals = r.kill_signals;
  fail_detection_supported = r.fail_detection_supported;
  launch_string = r.launch_string;
  expanded_launch_string = r.expanded_launch_string;
  has_launcher_so = r.has_launcher_so;
  launcher_so_name = r.launcher_so_name;
  attach_fifo_path = r.attach_fifo_path;
}

resource_manager_t::~resource_manager_t() {
  if (!launchers.empty()) {
    launchers.clear();
  }

  if (!kill_signals.empty()) {
    launchers.clear();
  }
}

resource_manager_t &resource_manager_t::operator=(const resource_manager_t &r) {
  rm = r.rm;
  mpir = r.mpir;
  launchers = r.launchers;
  launcher_ids = r.launcher_ids;
  job_id = r.job_id;
  launch_helper = r.launch_helper;
  kill_signals = r.kill_signals;
  fail_detection_supported = r.fail_detection_supported;
  launch_string = r.launch_string;
  expanded_launch_string = r.expanded_launch_string;
  has_launcher_so = r.has_launcher_so;
  launcher_so_name = r.launcher_so_name;
  attach_fifo_path = r.attach_fifo_path;
}

const std::vector<rm_id_t> &resource_manager_t::get_const_launcher_ids() const {
  return launcher_ids;
}

const std::vector<std::string> &resource_manager_t::get_const_launchers()
    const {
  return launchers;
}

void resource_manager_t::fill_rm_type(const std::string &v) {
  if (v == std::string("alps")) {
    rm = RC_alps;
  } else if (v == std::string("bglrm")) {
    rm = RC_bglrm;
  } else if (v == std::string("bgprm")) {
    rm = RC_bgprm;
  } else if (v == std::string("bgqrm")) {
    rm = RC_bgqrm;
  } else if (v == std::string("bgq_slurm")) {
    rm = RC_bgq_slurm;
  } else if (v == std::string("cray")) {
    rm = RC_cray;
  } else if (v == std::string("modelchecker")) {
    rm = RC_mchecker_rm;
  } else if (v == std::string("openrte")) {
    rm = RC_orte;
  } else if (v == std::string("slurm")) {
    rm = RC_slurm;
  } else if (v == std::string("mpiexec_hydra")) {
    rm = RC_mpiexec_hydra;
  } else if (v == std::string("gupc")) {
    rm = RC_gupc;
  } else {
    rm = RC_none;
  }
}

void resource_manager_t::fill_mpir_type(const std::string &v) {
  if (v == std::string("STD")) {
    mpir = standard;
  } else if (v == std::string("STD_CRAY")) {
    mpir = x_cray;
  } else if (v == std::string("STD_COLOC")) {
    mpir = x_coloc;
  } else if (v == std::string("STD_FIFO")) {
    mpir = x_fifo;
  } else if (v == std::string("STD_COLOC_FIFO")) {
    mpir = x_coloc_fifo;
  } else {
    mpir = x_none;
  }
}

void resource_manager_t::fill_launchers(const std::vector<std::string> &vect) {
  if (!vect.empty()) {
    launchers = vect;
  }
}

void resource_manager_t::fill_id(rm_id_t &target_id, const std::string &v) {
  std::string na("na");
  std::string key = na;
  std::string method = na;
  std::string id = na;
  std::string dt = na;

  size_t ix = v.find_first_of("|", 0);
  if (ix != std::string::npos) {
    key = v.substr(0, ix);
    size_t ix2 = v.find_first_of("|", ix + 1);
    if (ix2 != std::string::npos) {
      method = v.substr(ix + 1, ix2 - (ix + 1));
      size_t ix3 = v.find_first_of("|", ix2 + 1);
      if (ix3 != std::string::npos) {
        id = v.substr(ix2 + 1, ix3 - (ix2 + 1));
        dt = v.substr(ix3 + 1, std::string::npos);
      } else {
        id = v.substr(ix2 + 1, std::string::npos);
      }
    } else {
      method = v.substr(ix + 1, std::string::npos);
    }
  }

  if (key == std::string("RM_launcher")) {
    target_id.from = from_launcher;
  } else {
    target_id.from = from_unknown_source;
  }

  if (method == std::string("sym")) {
    target_id.method = symbol;
    target_id.id.symbol_name = strdup(id.c_str());
    if (dt == std::string("string")) {
      target_id.dtype = cstring;
    } else if ((dt == std::string("int32")) || (dt == std::string("integer"))) {
      target_id.dtype = integer32;
    } else if (dt == std::string("int64")) {
      target_id.dtype = integer64;
    } else {
      target_id.dtype = unknown_type;
    }
  } else if (method == std::string("pid")) {
    target_id.method = pid;
    target_id.id.process_id = 0;
  } else {
    target_id.method = unknown_method;
  }
}

void resource_manager_t::fill_job_id(const std::string &v) {
  fill_id(job_id, v);
}

void resource_manager_t::fill_launcher_id(
    const std::vector<std::string> &vect) {
  std::vector<std::string>::const_iterator c_i;
  for (c_i = vect.begin(); c_i != vect.end(); c_i++) {
    rm_id_t a_launcher_id;
    fill_id(a_launcher_id, (*c_i));
    launcher_ids.push_back(a_launcher_id);
  }
}

void resource_manager_t::fill_launcher_so(const std::string &v) {
  has_launcher_so = true;
  launcher_so_name = v;
}

void resource_manager_t::fill_kill_singals(const std::string &v) {
  int signal_type = 0;
  size_t ix = 0;
  size_t ix2 = 0;

  while ((ix2 = v.find_first_of("|", ix)) != std::string::npos) {
    signal_type = resolve_signal(v.substr(ix, ix2));
    kill_signals.push_back(signal_type);
    ix = ix2 + 1;
  }

  signal_type = resolve_signal(v.substr(ix, ix2));
  kill_signals.push_back(signal_type);
}

void resource_manager_t::fill_fail_detection(const std::string &v) {
  fail_detection_supported = false;

  if (v == std::string("true")) {
    fail_detection_supported = true;
  }
}

void resource_manager_t::fill_launch_helper(const std::string &v) {
  if (v == std::string("mpir")) {
    launch_helper.launch_method = mpir_coloc;
  } else {
    launch_helper.launch_method = launch_helper_method;
    // Some RMs will package their launch helper with launchmon. If v is not
    // found, then we will test to see if it is included with launchmon.
    std::string v2 = v;
    if (access(v.c_str(), R_OK) < 0) {
      char *pref;
      //
      // FIXME: DHA May 6 2016. This probably won't work with the
      // new in-tree testing support introduced with PR #12.
      //
      if (pref = getenv("LMON_PREFIX")) {
        v2.clear();
        v2 = std::string(pref) + std::string("/bin/") + v;
        // Check for packaged launch helper, if it is not found then we
        // assume that v is found in PATH.
        if (access(v2.c_str(), R_OK) < 0) {
          // reset back to v, it is in PATH.
          v2.clear();
          v2 = v;
        }
      }
    }
    launch_helper.launcher_command = v2;
  }
}

void resource_manager_t::fill_launch_string(const std::string &v) {
  launch_string = v;
}

void resource_manager_t::fill_expanded_launch_string(const std::string &v) {
  expanded_launch_string = v;
}

void resource_manager_t::fill_attach_fifo_path(const std::string &v) {
  attach_fifo_path = v;
}

///////////////////////////////////////////////////////////////////
//                                                               //
// PUBLIC METHODS of the rc_rm_t class                           //
//                                                               //
///////////////////////////////////////////////////////////////////

rc_rm_t::rc_rm_t() {
  MODULENAME = self_trace_t::self_trace().rm_module_trace.module_name;
}

rc_rm_t::rc_rm_t(const rc_rm_t &o) {
  resource_manager = o.resource_manager;
  coloc_paramset = o.coloc_paramset;
}

rc_rm_t &rc_rm_t::operator=(const rc_rm_t &rhs) {
  resource_manager = rhs.resource_manager;
  coloc_paramset = rhs.coloc_paramset;
}

rc_rm_t::~rc_rm_t() {
  if (coloc_paramset.sharedsec) free(coloc_paramset.sharedsec);

  if (coloc_paramset.randomid) free(coloc_paramset.randomid);

  if (coloc_paramset.hnfilename) free(coloc_paramset.hnfilename);

  if (!supported_rms.empty()) {
    supported_rms.clear();
  }
}

bool rc_rm_t::init(const std::string &os_isa_string) {
  char *pref;
  bool found_supported_rm = false;
  std::vector<std::string> supported_rm_fnames;
  std::string rm_info_conf_path(RM_INFO_CONFIG);
  std::string config_dir = std::string("/etc/");

  if (pref = getenv("LMON_PREFIX")) {
    config_dir = std::string(pref) + config_dir;
    rm_info_conf_path = config_dir + rm_info_conf_path;
  } else if (pref = getenv("LMON_RM_CONFIG_DIR")) {
    config_dir = std::string(pref) + std::string("/");
    rm_info_conf_path = config_dir + rm_info_conf_path;
  } else {
    config_dir = std::string(LMON_PREFIX) + config_dir;
    rm_info_conf_path = config_dir + rm_info_conf_path;
  }

  if (!read_supported_rm_confs(os_isa_string, rm_info_conf_path,
                               supported_rm_fnames)) {
    self_trace_t::trace(LEVELCHK(level1), MODULENAME, 1,
                        "read_supported_rm_confs returned false.");

    return found_supported_rm;
  }

  if (supported_rm_fnames.empty()) {
    self_trace_t::trace(LEVELCHK(level1), MODULENAME, 1,
                        "supported rm files not found.");

    return found_supported_rm;
  }

  found_supported_rm = true;
  std::vector<std::string>::const_iterator c_i;
  resource_manager_t target_rm;

  for (c_i = supported_rm_fnames.begin(); c_i != supported_rm_fnames.end();
       c_i++) {
    resource_manager_t a_rm;
    std::string c_path = config_dir + (*c_i);
    parse_and_fill_rm(c_path, a_rm);
    supported_rms.push_back(a_rm);
  }

  return found_supported_rm;
}

bool rc_rm_t::set_paramset(int n_nodes, int n_daemons, char *secret,
                           char *ran_id, int resource_id,
                           char *host_file_name) {
  if (!secret || !ran_id) {
    self_trace_t::trace(LEVELCHK(level1), MODULENAME, 1,
                        "shared secret or random id null.");

    return false;
  }

  coloc_paramset.nnodes = n_nodes;
  coloc_paramset.ndaemons = n_daemons;
  coloc_paramset.sharedsec = strdup(secret);
  coloc_paramset.randomid = strdup(ran_id);
  coloc_paramset.resourceid = resource_id;

  if (host_file_name) {
    coloc_paramset.hnfilename = strdup(host_file_name);
  }

  return true;
}

const std::list<std::string> rc_rm_t::expand_launch_string(
    std::string &expanded_string) {
  std::string tmp_lstr = resource_manager.get_launch_string();
  launch_method_e mth = resource_manager.get_launch_helper().launch_method;
  std::list<std::string> tokens;
  std::string token;
  size_t ix = 0;
  size_t ix2 = 0;

  if (mth == launch_helper_method) {
    if (getenv("LMON_DEBUG_BES") &&
        resource_manager.get_launch_helper().launcher_command !=
            std::string("LMON_REMOTE_LOGIN")) {
      //
      // Can't support debugging if the launch method is LMON_REMOTE_LOGIN
      //
      tokens.push_back(std::string(TVCMD));
      tokens.push_back(resource_manager.get_launch_helper().launcher_command);
      tokens.push_back(std::string("-a"));
    } else {
      if (resource_manager.get_launch_helper().launcher_command ==
          std::string("LMON_REMOTE_LOGIN")) {
        char *rmthd = getenv("LMON_REMOTE_LOGIN");
        if (rmthd) {
          tokens.push_back(std::string(rmthd));
        } else {
          tokens.push_back(std::string(SSHCMD));
        }
      } else {
        tokens.push_back(resource_manager.get_launch_helper().launcher_command);
      }
    }
  }

  while ((ix2 = tmp_lstr.find_first_of(" ", ix)) != std::string::npos) {
    token = tmp_lstr.substr(ix, ix2 - ix);
    tokens.push_back(token);
    ix = ix2 + 1;
  }

  if (ix != std::string::npos) {
    token = tmp_lstr.substr(ix, ix2 - ix);
    tokens.push_back(token);
  }

  if (tokens.empty()) {
    self_trace_t::trace(LEVELCHK(level1), MODULENAME, 1,
                        "ill-formed RM_launch_str?");

    return tokens;
  }

  std::list<std::string>::iterator itr;

  for (itr = tokens.begin(); itr != tokens.end(); itr++) {
    size_t perc_sign_ix = (*itr).find_first_of("%", 0);
    if ((perc_sign_ix < std::string::npos) &&
        ((perc_sign_ix + 1) < (*itr).size())) {
      bool split_case = false;
      std::string exp_substr =
          expand_a_letter((*itr)[perc_sign_ix + 1], &split_case);
      if (exp_substr != std::string("na")) {
        (*itr).replace(perc_sign_ix, 2, exp_substr);
      }

      if (split_case) {
        std::string sep1("");
        std::string sep2(" ");
        std::string sep3("\"\'");
        boost::escaped_list_separator<char> esc(sep1, sep2, sep3);
        boost::tokenizer<boost::escaped_list_separator<char> > boost_token(
            (*itr), esc);
        boost::tokenizer<boost::escaped_list_separator<char> >::iterator
            boost_itr;
        std::list<std::string> sub_list;
        for (boost_itr = boost_token.begin(); boost_itr != boost_token.end();
             boost_itr++) {
          sub_list.push_back(*boost_itr);
        }

        if (sub_list.size() > 1) {
          // insert the new sublist into the master list
          tokens.splice(itr, sub_list);
          // erase the current element and make sure
          // to update the itr to its previous iter so that
          // current for-loop can progress.
          tokens.erase(itr--);
        }
      }
    }
  }

  tokens.remove(std::string(""));

  expanded_string = "";
  if (!tokens.empty()) {
    int i;
    for (itr = tokens.begin(); itr != tokens.end(); itr++) {
      expanded_string += (*itr);
      if (itr != tokens.end()) {
        expanded_string += " ";
      }
    }
  }

  resource_manager.fill_expanded_launch_string(expanded_string);
  return tokens;
}

const std::string &rc_rm_t::get_expanded_launch_string() {
  return resource_manager.get_expanded_launch_string();
}

const std::vector<resource_manager_t> &rc_rm_t::get_supported_rms() {
  return supported_rms;
}

bool rc_rm_t::graceful_rmkill(int pid) {
  if (pid < 0) {
    return false;
  }

  std::vector<int>::iterator i;
  for (i = resource_manager.get_kill_signals().begin();
       i != resource_manager.get_kill_signals().end(); i++) {
    kill(pid, *i);
    usleep(GracePeriodBNSignals);
  }

  return true;
}

bool rc_rm_t::need_check_launcher_so() {
  return resource_manager.get_has_launcher_so();
}

bool rc_rm_t::is_modelchecker() {
  return (resource_manager.get_rm() == RC_mchecker_rm);
}

bool rc_rm_t::is_coloc_sup() {
  return ((resource_manager.get_mpir() == x_coloc) ||
          (resource_manager.get_mpir() == x_coloc_fifo));
}

bool rc_rm_t::is_attfifo_sup() {
  return ((resource_manager.get_mpir() == x_fifo) ||
          (resource_manager.get_mpir() == x_coloc_fifo));
}

bool rc_rm_t::is_cont_on_att() {
  return (resource_manager.get_mpir() == x_cray);
}

bool rc_rm_t::is_rid_sup() {
  return (resource_manager.get_job_id().from != from_unknown_source);
}

bool rc_rm_t::is_fail_detect_sup() {
  return resource_manager.get_fail_detection_supported();
}

bool rc_rm_t::is_rid_via_symbol() {
  return (resource_manager.get_job_id().method == symbol);
}

bool rc_rm_t::is_rid_via_pid() {
  return (resource_manager.get_job_id().method == pid);
}

resource_manager_t &rc_rm_t::get_resource_manager() { return resource_manager; }

coloc_str_param_t &rc_rm_t::get_coloc_paramset() { return coloc_paramset; }

const coloc_str_param_t &rc_rm_t::get_const_coloc_paramset() {
  return coloc_paramset;
}

const char *rc_rm_t::get_hostnames_fn() { return coloc_paramset.hnfilename; }

const std::string &rc_rm_t::get_launcher_so_name() {
  return resource_manager.get_launcher_so_name();
}

const std::string &rc_rm_t::get_attach_fifo_path() {
  return resource_manager.get_attach_fifo_path();
}

void rc_rm_t::set_attach_fifo_path(const std::string &fifo_path) {
  resource_manager.fill_attach_fifo_path(fifo_path);
}

void rc_rm_t::set_resource_manager(const resource_manager_t &rmgr) {
  resource_manager = rmgr;
}

///////////////////////////////////////////////////////////////////
//
// PRIVATE METHODS of the rc_rm_t-related class
//
///////////////////////////////////////////////////////////////////

int resource_manager_t::resolve_signal(const std::string &v) {
  int ret_sig = -1;

  if (v == std::string("SIGINT")) {
    ret_sig = SIGINT;
  } else if (v == std::string("SIGTERM")) {
    ret_sig = SIGTERM;
  }

  return ret_sig;
}

bool rc_rm_t::read_supported_rm_confs(
    const std::string &os_isa_string, const std::string &rm_info_conf_path,
    std::vector<std::string> &supported_rm_fnames) {
  std::ifstream ri_conf;
  char line_max[PATH_MAX];
  bool found = false;
  bool plat_found = false;
  std::string a_line;

  ri_conf.open(rm_info_conf_path.c_str());
  if (ri_conf.is_open()) {
    while (!ri_conf.eof()) {
      ri_conf.getline(line_max, PATH_MAX);
      if (ri_conf.bad() || ri_conf.fail()) {
        self_trace_t::trace(LEVELCHK(level1), MODULENAME, 1,
                            "getline enountered an error.");

        break;
      }

      a_line = line_max;

      if (a_line[0] == '#' || a_line[0] == '\0') continue;

      if (!plat_found) {
        if (a_line[0] == '[') {
          size_t ix = a_line.find_first_of(']', 1);
          if (ix != std::string::npos) {
            std::string found_os_isa = a_line.substr(1, ix - 1);
            if (found_os_isa == os_isa_string) plat_found = true;
          } else {
            self_trace_t::trace(LEVELCHK(level1), MODULENAME, 1,
                                "ill-formed line.");
          }
        }
      } else {
        if (a_line[0] == '[') break;  // OK, done with parsing for my platform
        supported_rm_fnames.push_back(a_line);
        found = true;
      }
    }
    ri_conf.close();
  }

  return found;
}

bool rc_rm_t::parse_and_fill_rm(const std::string &rm_conf_path,
                                resource_manager_t &a_rm) {
  std::ifstream inputfile;
  std::map<std::string, std::vector<std::string> > key_value_pair;
  std::map<std::string, std::vector<std::string> >::iterator iter;
  bool error_found = false;
  char line_max[PATH_MAX];
  std::string a_line;

  inputfile.open(rm_conf_path.c_str());

  if (inputfile.is_open()) {
    while (!inputfile.eof()) {
      inputfile.getline(line_max, PATH_MAX);
      if (inputfile.bad() || inputfile.fail()) {
        self_trace_t::trace(LEVELCHK(level1), MODULENAME, 1,
                            "getline encountered an error.");

        break;
      }

      a_line = line_max;
      if (a_line[0] == '#' || a_line[0] == '\n') continue;

      size_t equal = a_line.find_first_of('=', 0);
      if (equal != std::string::npos) {
        std::string key = a_line.substr(0, equal);
        std::string value = a_line.substr(equal + 1, std::string::npos);
        iter = key_value_pair.find(key);

        if (iter == key_value_pair.end()) {
          std::vector<std::string> value_vect;
          value_vect.push_back(value);
          key_value_pair[key] = value_vect;
        } else {
          iter->second.push_back(value);
        }
      } else {
        self_trace_t::trace(LEVELCHK(level1), MODULENAME, 1,
                            "ill-formed line.");

        error_found = true;
      }
    }
    inputfile.close();
  }

  iter = key_value_pair.find(std::string("RM"));
  if (iter != key_value_pair.end()) {
    a_rm.fill_rm_type(iter->second[0]);
  }

  iter = key_value_pair.find(std::string("RM_MPIR"));
  if (iter != key_value_pair.end()) {
    a_rm.fill_mpir_type(iter->second[0]);
  }

  iter = key_value_pair.find(std::string("RM_launcher"));
  if (iter != key_value_pair.end()) {
    a_rm.fill_launchers(iter->second);
  }

  iter = key_value_pair.find(std::string("RM_launcher_id"));
  if (iter != key_value_pair.end()) {
    a_rm.fill_launcher_id(iter->second);
  }

  iter = key_value_pair.find(std::string("RM_launcher_so"));
  if (iter != key_value_pair.end()) {
    a_rm.fill_launcher_so(iter->second[0]);
  }

  iter = key_value_pair.find(std::string("RM_jobid"));
  if (iter != key_value_pair.end()) {
    a_rm.fill_job_id(iter->second[0]);
  }

  iter = key_value_pair.find(std::string("RM_signal_for_kill"));
  if (iter != key_value_pair.end()) {
    a_rm.fill_kill_singals(iter->second[0]);
  }

  iter = key_value_pair.find(std::string("RM_fail_detection"));
  if (iter != key_value_pair.end()) {
    a_rm.fill_fail_detection(iter->second[0]);
  }

  iter = key_value_pair.find(std::string("RM_launch_helper"));
  if (iter != key_value_pair.end()) {
    a_rm.fill_launch_helper(iter->second[0]);
  }

  iter = key_value_pair.find(std::string("RM_launch_str"));
  if (iter != key_value_pair.end()) {
    a_rm.fill_launch_string(iter->second[0]);
  }

  return error_found;
}

const std::string rc_rm_t::expand_a_letter(const char p,
                                           bool *split_maybe_needed) {
  std::stringstream ssm;
  *split_maybe_needed = false;

  switch (p) {
    case 'b':
      ssm << coloc_paramset.rm_daemon_stub;
      break;

    case 'c':
      ssm << coloc_paramset.randomid;
      break;

    case 'd':
      ssm << coloc_paramset.rm_daemon_path;
      break;

    case 'j':
      ssm << coloc_paramset.resourceid;
      break;

    case 'l':
      ssm << coloc_paramset.hnfilename;
      break;

    case 'n':
      ssm << coloc_paramset.nnodes;
      break;

    case 'o':
      for (std::list<std::string>::iterator i =
               coloc_paramset.rm_daemon_args.begin();
           i != coloc_paramset.rm_daemon_args.end(); i++) {
        ssm << *i << " ";
      }
      *split_maybe_needed = true;
      break;

    case 's':
      ssm << coloc_paramset.sharedsec;
      break;

    case 'h':
      //
      // for now, we only support localhost as an expansion choice for
      // %h. This is to support GUPC. But this can be extended later
      // for a more complex model.
      //
      ssm << "localhost";
      break;

    default:
      ssm << "na";
      self_trace_t::trace(LEVELCHK(level1), MODULENAME, 1,
                          "Unknown RM_launch_str parameter.");
      break;
  }

  return ssm.str();
}

/*
 * ts=2 sw=2 expandtab
 */
