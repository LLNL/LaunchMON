/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_opt.hxx,v 1.6.2.3 2008/02/20 17:37:57 dahn Exp $
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
 *        Apr 22 2014 DHA: Applied a patch to store RM args/opts into
 *                         a std::list object instead of a std:string string.
 *        Jun 30 2010 DHA: Added faster engine parsing error detection support
 *                         Deprecated option_sanity_check();
 *        Jun 09 2010 DHA: Added RM MAP support
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Dec 05 2007 DHA: Added model checker support
 *        Jul 04 2006 DHA: Added self tracing support
 *        Jun 08 2006 DHA: Added attach-to-a-running-job support
 *        Jun 07 2007 DHA: Populated more options (tool_daemon_opts, copyright)
 *        Jun 06 2006 DHA: File created
 */

//! FILE: sdbg_opt.hxx
/*!
    Self-explanatory file...
*/

#ifndef SDBG_OPT_HXX
#define SDBG_OPT_HXX 1
 
#include <lmon_api/common.h>
 
extern "C" {
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
}

#include <iostream>
#include <string>
#include <map>

#include "sdbg_std.hxx"
#include "sdbg_rm_map.hxx"
#include "sdbg_self_trace.hxx"

//! struct opt_struct_t
/*!
    Structure capturing optionset. Look at comments on each member
*/
struct opt_struct_t {
  int         verbose;          // verbose level
  bool        attach;           // is this attach-to-a-running job case?
  bool        remote;           // is this remote case? 
  std::string tool_daemon;      // path to the lightweight debug engine
  std::string debugtarget;      // parallel job launcher
  std::string copyright;        // copy right for this project
  std::string launchstring;     // launch string to be expanded
  std::list<std::string>   tool_daemon_opts; // options to the lightweight debug engine
  std::string remote_info;      // ip:port
  std::string lmon_sec_info;    // shared secret:randomID
  pid_t       launcher_pid;     // the pid of a running parallel launcher process
  char      **remaining;        // options and arguments to be passed 
  std::map<std::string, std::string> envMap;
};


//! class opt_struct_t
/*!
    constructor allocates my_opt. process_args parses options and arguments 
    filling my_opt.
*/
class opts_args_t {

public:
  opts_args_t();
  ~opts_args_t();  

  define_gset(opt_struct_t*, my_opt)
  define_gset(rc_rm_t*, my_rmconfig)
  define_gset(bool, has_parse_error)

  bool process_args (int *argc, char ***argv);
  void print_usage();
  bool construct_rm_map();
  void print_copyright();


private:
  bool LEVELCHK(self_trace_verbosity level) 
       { return (self_trace_t::self_trace().opt_module_trace.verbosity_level >= level); }

  bool check_path(std::string &base, std::string &pth);

  bool has_parse_error;

  // move the copy ctor in the private area preventing an object 
  // of this class copied
  opts_args_t(const opts_args_t &o);

  opt_struct_t *my_opt;
  rc_rm_t *my_rmconfig;

  // For self tracing
  //
  std::string MODULENAME;
};

#endif // SDBG_OPT_HXX
