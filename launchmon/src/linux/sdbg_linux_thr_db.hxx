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
 *        Mar 06 2009 DHA: Created file.
 */

#ifndef SDBG_LINUX_THR_DB_HXX
#define SDBG_LINUX_THR_DB_HXX 1

extern "C" {
#if HAVE_THREAD_DB_H
# include <thread_db.h>
#else
# error thread_db.h is required
#endif

#if HAVE_DLFCN_H 
# include <dlfcn.h>
#else
# error dlfcn.h is required
#endif

#if HAVE_LIBGEN_H
# include <libgen.h>
#else
# error libgen.h is required
#endif
}

#if HAVE_STRING
# include <string>
#else
# error string is required
#endif

#include "sdbg_linux_std.hxx"
#include "sdbg_self_trace.hxx"

class linux_thr_db {
public:
  linux_thr_db() 
  {
    MODULENAME = self_trace_t::thread_tracer_module_trace.module_name;
    thr_db_handle = NULL;
    dll_td_init = NULL;
    dll_td_ta_event_addr = NULL;
    dll_td_ta_get_nthreads = NULL;
    dll_td_ta_new = NULL;
    dll_td_ta_thr_iter = NULL;
    dll_td_thr_event_enable = NULL;
    dll_td_thr_get_info = NULL;
  };

  linux_thr_db(const linux_thr_db& t)
  {
    thr_db_handle = t.thr_db_handle;
    dll_td_init = t.dll_td_init;
    dll_td_ta_event_addr = t.dll_td_ta_event_addr;
    dll_td_ta_get_nthreads = t.dll_td_ta_get_nthreads;
    dll_td_ta_new = t.dll_td_ta_new;
    dll_td_ta_thr_iter = t.dll_td_ta_thr_iter;
    dll_td_thr_event_enable = t.dll_td_thr_event_enable;
    dll_td_thr_get_info = t.dll_td_thr_get_info;
    MODULENAME = t.MODULENAME;
  }

  ~linux_thr_db()
  {
    if (thr_db_handle)
      dlclose(thr_db_handle);
  }

  bool bind(std::string libpath);

  void *thr_db_handle;

  td_err_e (*dll_td_init) 
    (void);
  td_err_e (*dll_td_thr_event_enable) 
    (const td_thrhandle_t *, int);
  td_err_e (*dll_td_thr_get_info) 
    (const td_thrhandle_t *, td_thrinfo_t *);
  td_err_e (*dll_td_ta_event_addr) 
    (const td_thragent_t *, td_event_e, td_notify_t *);
  td_err_e (*dll_td_ta_set_event) 
    (const td_thragent_t *, td_thr_events_t *);
  td_err_e (*dll_td_ta_new) 
    (struct ps_prochandle *, td_thragent_t **);
  td_err_e (*dll_td_ta_get_nthreads) 
    (const td_thragent_t *, int *);
  td_err_e (*dll_td_ta_thr_iter) 
    (const td_thragent_t *, td_thr_iter_f *, void *,
     td_thr_state_e, int, sigset_t *, unsigned int);

private:
  bool LEVELCHK(self_trace_verbosity level) 
       { return (self_trace_t::tracer_module_trace.verbosity_level >= level); }

  // For self tracing
  //
  std::string MODULENAME; 
};

#endif //SDBG_LINUX_THR_DB_HXX
