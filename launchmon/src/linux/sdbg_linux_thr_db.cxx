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

#include "sdbg_std.hxx"

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
#endif

#include "sdbg_linux_thr_db.hxx"
#include "sdbg_proc_service.hxx"

using namespace std;

bool 
linux_thr_db::bind(string libpath)
{
  string thread_db_fullpath;

  if (thr_db_handle)
    return false;

  thread_db_fullpath 
    = libpath + string("/") + string(LIBTHREAD_DB);
 
  thr_db_handle 
    = dlopen(thread_db_fullpath.c_str(),RTLD_NOW|RTLD_GLOBAL);

  if (!thr_db_handle) {
    string tmpout;
    char *errstr = dlerror();
    tmpout = "dlopen fails to open and resolve thread debug library: ";
    if (errstr) {
      tmpout += string(errstr);
    }
 
    self_trace_t::trace ( true,
      MODULENAME,1, 
      "dlopen fails to open and resolve thread debug library" );

    return false;
  }

  dll_td_init 
    = (td_err_e (*)(void)) 
      dlsym(thr_db_handle, "td_init");

  if (!dll_td_init) {
    self_trace_t::trace ( true,
      MODULENAME,1, 
      "dlopen fails to resolve td_init" );
    return false;
  }

  dll_td_thr_event_enable
    = (td_err_e (*)(const td_thrhandle_t *, int)) 
      dlsym(thr_db_handle, "td_thr_event_enable"); 

  if (!dll_td_thr_event_enable) {
    self_trace_t::trace ( true,
      MODULENAME,1, 
      "dlopen fails to resolve td_thr_event_enable" );
    return false;
  }

  dll_td_ta_set_event
    = (td_err_e (*)(const td_thragent_t *, td_thr_events_t *))
      dlsym(thr_db_handle, "td_ta_set_event");

  if (!dll_td_ta_set_event) {
    self_trace_t::trace ( true,
      MODULENAME,1, 
      "dlopen fails to resolve td_ta_set_event" );
    return false;
  }

  dll_td_thr_get_info
    = (td_err_e (*)(const td_thrhandle_t *, td_thrinfo_t *))
      dlsym(thr_db_handle, "td_thr_get_info");

  if (!dll_td_thr_get_info) {
    self_trace_t::trace ( true,
      MODULENAME,1, 
      "dlopen fails to resolve td_thr_get_info" );
    return false;
  }

  dll_td_ta_event_addr 
    = (td_err_e (*)(const td_thragent_t *, td_event_e, td_notify_t *))
    dlsym(thr_db_handle, "td_ta_event_addr");

  if (!dll_td_ta_event_addr) {
    self_trace_t::trace ( true,
      MODULENAME,1, 
      "dlopen fails to resolve td_ta_event_addr" );
    return false;
  }

  dll_td_ta_new 
    = (td_err_e (*) (struct ps_prochandle *, td_thragent_t **)) 
    dlsym(thr_db_handle, "td_ta_new");

  if (!dll_td_ta_new) {
    self_trace_t::trace ( true,
      MODULENAME,1, 
      "dlopen fails to resolve td_ta_new" );
    return false;
  }

  dll_td_ta_get_nthreads 
    = (td_err_e (*)(const td_thragent_t *, int *)) 
    dlsym(thr_db_handle, "td_ta_get_nthreads");

  if (!dll_td_ta_get_nthreads) {
    self_trace_t::trace ( true,
      MODULENAME,1, 
      "dlopen fails to resolve td_ta_get_nthreads" );
    return false;
  }

  dll_td_ta_thr_iter 
    = (td_err_e (*)(const td_thragent_t *, td_thr_iter_f *, 
      void *,td_thr_state_e, int, sigset_t *, unsigned int))
    dlsym(thr_db_handle, "td_ta_thr_iter");

  if (!dll_td_ta_thr_iter) {
    self_trace_t::trace ( true,
      MODULENAME,1, 
      "dlopen fails to resolve td_ta_thr_iter" );
    return false;
  }

  return true;
} 

