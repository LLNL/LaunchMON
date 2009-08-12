/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_signal_hlr.hxx,v 1.3.2.1 2008/02/20 17:37:57 dahn Exp $
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
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Jul 05  2006 DHA: File created
 */ 

#ifndef SDBG_SIGNAL_HLR_HXX
#define SDBG_SIGNAL_HLR_HXX 1

#if HAVE_VECTOR
# include <vector>
#else
# error vector is required
#endif

#include "sdbg_std.hxx"
#include "sdbg_base_tracer.hxx"
#include "sdbg_base_mach.hxx"
#include "sdbg_base_tracer.hxx"
#include "sdbg_self_trace.hxx"

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
class signal_handler_t 
{

public:
  
  signal_handler_t ( );
  signal_handler_t ( const signal_handler_t& s );
  ~signal_handler_t ( );

  static void set_tracer ( tracer_base_t<SDBG_DEFAULT_TEMPLPARAM>* t ) 
                                                          { tracer = t; }
  static tracer_base_t<SDBG_DEFAULT_TEMPLPARAM>* get_tracer() 
                                                          { return tracer; } 

  static void set_launcher_proc ( process_base_t<SDBG_DEFAULT_TEMPLPARAM>* p )
                                                          { launcher_proc = p; }
 
  static process_base_t<SDBG_DEFAULT_TEMPLPARAM>* get_launcher_proc ( )
                                                          { return launcher_proc; }

  static void sighandler ( int sig );
  static void install_hdlr_for_all_sigs ( );

  static void set_evman ( event_manager_t<SDBG_DEFAULT_TEMPLPARAM>* em )
                                                          { evman = em; }
  static void set_lmon ( launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>* lm )
                                                          { lmon = lm; }

private:
  static bool LEVELCHK(self_trace_verbosity level) 
       { return (self_trace_t::sighandler_module_trace.verbosity_level >= level); }
 
  static std::vector<int> monitoring_signals;  
  static tracer_base_t<SDBG_DEFAULT_TEMPLPARAM> *tracer;
  static process_base_t<SDBG_DEFAULT_TEMPLPARAM> *launcher_proc;
  static event_manager_t<SDBG_DEFAULT_TEMPLPARAM> *evman;
  static launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM> *lmon;

  //
  // For self tracing
  //
  static std::string MODULENAME;

};

#endif // SDBG_SIGNAL_HLR_HXX 
