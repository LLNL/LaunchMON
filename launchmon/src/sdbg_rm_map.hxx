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
 *        Feb 20 2015 andrewg@cray.com: Added Cray support.
 *        Dec 07 2012 DHA: init_rm_instance returns a meaningful return code
 *        Oct 06 2011 DHA: Restructuring to support
 *                         runtime detection of target resource managers
 *        Sep 12 2011 DHA: Added be_fail_detection_supported support
 *                         for the orphaned alps_fe_colocat problem
 *                         (ID: 3408210).
 *        Nov 22 2010 DHA: Includes limits.h
 *        Oct 22 2010 DHA: Added support for launching of tool daemons
 *                         into a subset of an allocation under SLURM.
 *                         (ID: 2871927).
 *        Jun 28 2010 DHA: Moved the rm_catalogue_e defintition into the
 *                         LMON API standard header.
 *        Jun 09 2010 DHA: Created file.
 */

#ifndef SDBG_RM_MAP_HXX
#define SDBG_RM_MAP_HXX 1

#include "sdbg_std.hxx"

#include <limits.h>
#include <signal.h>
#include <cstdio>
#include <list>
#include <string>
#include <vector>

#include <lmon_api/lmon_api_std.h>

#include "sdbg_base_symtab.hxx"
#include "sdbg_self_trace.hxx"

#define RM_INFO_CONFIG "rm_info.conf"

enum mpir_catalogue_e {
  standard,
  x_cray,
  x_coloc,
  x_fifo,
  x_coloc_fifo,
  x_none
};

enum id_method_e { symbol, pid, unknown_method };

enum id_from_e { from_launcher, from_unknown_source };

enum id_datatype_e { cstring, integer32, integer64, unknown_type };

enum launch_method_e {
  mpir_coloc,
  launch_helper_method,
  launch_method_unknown
};

struct rm_id_t {
  rm_id_t();

  rm_id_t(const rm_id_t &o);

  ~rm_id_t();

  rm_id_t &operator=(const rm_id_t &rhs);

  id_from_e from;
  id_method_e method;
  id_datatype_e dtype;
  union u {
    pid_t process_id;
    char *symbol_name;
  } id;
};

struct launch_method_t {
  launch_method_t()
      : launch_method(launch_method_unknown),
        launcher_command(std::string("na")) {}

  launch_method_t(const launch_method_t &o) {
    launch_method = o.launch_method;
    launcher_command = o.launcher_command;
  }

  ~launch_method_t() {}

  launch_method_t &operator=(const launch_method_t &rhs) {
    launch_method = rhs.launch_method;
    launcher_command = rhs.launcher_command;
    return *this;
  }

  launch_method_e launch_method;
  std::string launcher_command;
};

struct coloc_str_param_t {
  coloc_str_param_t()
      : rm_daemon_path(std::string("na")),
        rm_daemon_stub(std::string("na")),
        nnodes(-1),
        ndaemons(-1),
        sharedsec(NULL),
        randomid(NULL),
        resourceid(-1),
        hnfilename(NULL) {}

  coloc_str_param_t(const coloc_str_param_t &o) {
    rm_daemon_path = o.rm_daemon_path;
    rm_daemon_args = o.rm_daemon_args;
    rm_daemon_stub = o.rm_daemon_stub;
    nnodes = o.nnodes;
    ndaemons = o.ndaemons;
    sharedsec = o.sharedsec;
    randomid = o.randomid;
    resourceid = o.resourceid;
    hnfilename = o.hnfilename;
  }

  ~coloc_str_param_t() {}

  coloc_str_param_t &operator=(const coloc_str_param_t &rhs) {
    rm_daemon_path = rhs.rm_daemon_path;
    rm_daemon_args = rhs.rm_daemon_args;
    rm_daemon_stub = rhs.rm_daemon_stub;
    nnodes = rhs.nnodes;
    ndaemons = rhs.ndaemons;
    sharedsec = rhs.sharedsec;
    randomid = rhs.randomid;
    resourceid = rhs.resourceid;
    hnfilename = rhs.hnfilename;
    return *this;
  }

  std::string rm_daemon_path;
  std::list<std::string> rm_daemon_args;
  std::string rm_daemon_stub;
  int nnodes;
  int ndaemons;
  char *sharedsec;
  char *randomid;
  int resourceid;
  char *hnfilename;
};

class resource_manager_t {
 public:
  resource_manager_t();
  resource_manager_t(const resource_manager_t &r);
  ~resource_manager_t();
  resource_manager_t &operator=(const resource_manager_t &rhs);

  define_gset(rm_catalogue_e, rm)
  define_gset(mpir_catalogue_e, mpir)
  define_gset(std::vector<std::string> &, launchers)
  define_gset(std::vector<rm_id_t> &, launcher_ids)
  define_gset(rm_id_t &, job_id)
  define_gset(launch_method_t &, launch_helper)
  define_gset(std::vector<int> &, kill_signals)
  define_gset(bool, fail_detection_supported)
  define_gset(std::string &, launch_string)
  define_gset(std::string &, expanded_launch_string)
  define_gset(bool, has_launcher_so)
  define_gset(std::string &, launcher_so_name)
  define_gset(std::string &, attach_fifo_path)
  const std::vector<rm_id_t> &get_const_launcher_ids() const;
  const std::vector<std::string> &get_const_launchers() const;

  void fill_rm_type(const std::string &v);
  void fill_mpir_type(const std::string &v);
  void fill_launchers(const std::vector<std::string> &v);
  void fill_job_id(const std::string &v);
  void fill_launcher_id(const std::vector<std::string> &vect);
  void fill_id(rm_id_t &id, const std::string &v);
  void fill_fail_detection(const std::string &vect);
  void fill_kill_singals(const std::string &v);
  void fill_launcher_so(const std::string &v);
  void fill_launch_helper(const std::string &v);
  void fill_launch_string(const std::string &v);
  void fill_expanded_launch_string(const std::string &v);
  void fill_attach_fifo_path(const std::string &v);

 private:
  int resolve_signal(const std::string &v);

  rm_catalogue_e rm;
  mpir_catalogue_e mpir;
  std::vector<std::string> launchers;
  std::vector<rm_id_t> launcher_ids;
  rm_id_t job_id;
  launch_method_t launch_helper;
  std::vector<int> kill_signals;
  bool fail_detection_supported;
  std::string launch_string;
  std::string expanded_launch_string;
  bool has_launcher_so;
  std::string launcher_so_name;
  std::string attach_fifo_path;
};

class rc_rm_t {
 public:
  rc_rm_t();
  rc_rm_t(const rc_rm_t &o);
  ~rc_rm_t();
  rc_rm_t &operator=(const rc_rm_t &rhs);

  bool init(const std::string &os_isa_string);

  bool set_paramset(int n_nodes, int n_daemons, char *secret, char *ran_id,
                    int resource_id, char *host_file_name);

  const std::list<std::string> expand_launch_string(std::string &s);

  const std::string &get_expanded_launch_string();

  bool graceful_rmkill(int pid);

  const std::vector<resource_manager_t> &get_supported_rms();

  bool need_check_launcher_so();

  bool is_modelchecker();

  bool is_coloc_sup();

  bool is_attfifo_sup();

  bool is_cont_on_att();

  bool is_rid_sup();

  bool is_rid_via_symbol();

  bool is_rid_via_pid();

  bool is_fail_detect_sup();

  resource_manager_t &get_resource_manager();

  coloc_str_param_t &get_coloc_paramset();

  const coloc_str_param_t &get_const_coloc_paramset();

  const char *get_hostnames_fn();

  const std::string &get_launcher_so_name();

  const std::string &get_attach_fifo_path();

  void set_attach_fifo_path(const std::string &fifo_path);

  void set_resource_manager(const resource_manager_t &rmgr);

 private:
  bool LEVELCHK(self_trace_verbosity level) {
    return (self_trace_t::self_trace().rm_module_trace.verbosity_level >=
            level);
  }

  bool read_supported_rm_confs(const std::string &os_isa_string,
                               const std::string &rm_info_conf_path,
                               std::vector<std::string> &supported_rm_fnames);

  bool parse_and_fill_rm(const std::string &rm_conf_path,
                         resource_manager_t &a_rm);

  const std::string expand_a_letter(const char p, bool *split_maybe_needed);

  resource_manager_t resource_manager;

  coloc_str_param_t coloc_paramset;

  std::vector<resource_manager_t> supported_rms;

  // For self tracing
  //
  std::string MODULENAME;
};

template <BASE_IMAGE_TEMPLATELIST>
class rc_rm_plat_matcher {
 public:
  rc_rm_plat_matcher() {
    MODULENAME = self_trace_t::self_trace().rm_module_trace.module_name;
  }
  rc_rm_plat_matcher(const rc_rm_plat_matcher &o) { MODULENAME = o.MODULENAME; }
  ~rc_rm_plat_matcher() {}

  bool init_rm_instance(
      rc_rm_t &rm_obj, const std::string &lnchrpath,
      const std::string &tool_daemon_path,
      const std::list<std::string> &tool_daemon_opts,
      const image_base_t<BASE_IMAGE_TEMPLPARAM> *main_symtab = NULL,
      const image_base_t<BASE_IMAGE_TEMPLPARAM> *rmso_symtab = NULL,
      const std::string be_stub_path = "");

 private:
  bool has_matching_signiture(
      const std::string &lnchrpath, const resource_manager_t &a_rm,
      const image_base_t<BASE_IMAGE_TEMPLPARAM> *main_symtab = NULL,
      const image_base_t<BASE_IMAGE_TEMPLPARAM> *rmso_symtab = NULL);

  // For self tracing
  //
  std::string MODULENAME;
};

///////////////////////////////////////////////////////////////////
//                                                               //
// Best practice for templated methods is to include their def   //
// directly to the user.                                         //
//                                                               //
///////////////////////////////////////////////////////////////////

template <BASE_IMAGE_TEMPLATELIST>
bool rc_rm_plat_matcher<BASE_IMAGE_TEMPLPARAM>::init_rm_instance(
    rc_rm_t &rm_obj, const std::string &launchr_path,
    const std::string &tool_daemon_path,
    const std::list<std::string> &tool_daemon_opts,
    const image_base_t<BASE_IMAGE_TEMPLPARAM> *main_symtab,
    const image_base_t<BASE_IMAGE_TEMPLPARAM> *rmso_symtab,
    const std::string be_stub_path) {
  bool found_matched_rm = false;
  std::vector<resource_manager_t>::const_iterator r_i;

  for (r_i = rm_obj.get_supported_rms().begin();
       r_i != rm_obj.get_supported_rms().end(); r_i++) {
    if (has_matching_signiture(launchr_path, (*r_i), main_symtab,
                               rmso_symtab)) {
      rm_obj.set_resource_manager(*r_i);
      found_matched_rm = true;
      break;
    }
  }

  rm_obj.get_coloc_paramset().rm_daemon_path = tool_daemon_path;
  rm_obj.get_coloc_paramset().rm_daemon_args = tool_daemon_opts;
  rm_obj.get_coloc_paramset().rm_daemon_stub = be_stub_path;

  return found_matched_rm;
}

template <BASE_IMAGE_TEMPLATELIST>
bool rc_rm_plat_matcher<BASE_IMAGE_TEMPLPARAM>::has_matching_signiture(
    const std::string &lnchrpath, const resource_manager_t &a_rm,
    const image_base_t<BASE_IMAGE_TEMPLPARAM> *main_symtab,
    const image_base_t<BASE_IMAGE_TEMPLPARAM> *rmso_symtab) {
  char *bnbuf = strdup(lnchrpath.c_str());
  char *launcher_name = basename(bnbuf);
  bool match = false;
  std::vector<std::string>::const_iterator r_i;
  std::vector<rm_id_t>::const_iterator id_i;

  for (r_i = a_rm.get_const_launchers().begin();
       r_i != a_rm.get_const_launchers().end(); r_i++) {
    if (launcher_name == (*r_i) || (*r_i) == std::string("*")) {
      //
      // Name matched!
      //
      const std::vector<rm_id_t> &id_vector = a_rm.get_const_launcher_ids();
      for (id_i = id_vector.begin(); id_i != id_vector.end(); id_i++) {
        switch ((*id_i).from) {
          case from_launcher: {
            if ((*id_i).method == symbol) {
              if (main_symtab && (*id_i).id.symbol_name) {
                const std::string sn((*id_i).id.symbol_name);
                const symbol_base_t<VA> &symb = main_symtab->get_a_symbol(sn);
                if (!(!symb)) {
                  //
                  // Symbol matched!
                  //
                  match = true;
                  break;
                }
              }

              if (rmso_symtab && (*id_i).id.symbol_name) {
                const std::string sn((*id_i).id.symbol_name);
                const symbol_base_t<VA> &symb = rmso_symtab->get_a_symbol(sn);
                if (!(!symb)) {
                  //
                  // Symbol matched!
                  //
                  match = true;
                  break;
                }
              }
            } else {
              self_trace_t::trace(
                  true, MODULENAME, 1,
                  "only symbol-based id method is supported (%s)!",
                  (*r_i).c_str());
            }
          } break;

          default: {
            self_trace_t::trace(
                true, MODULENAME, 1,
                "only symbol-based id method is supported (%s)!",
                (*r_i).c_str());
          } break;
        }
      }
    }
  }

  free(bnbuf);
  return match;
}

#endif  // SDBG_RM_MAP_HXX

/*
 * ts=2 sw=2 expandtab
 */
