/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_base_driver.hxx,v 1.6.2.1 2008/02/20 17:37:56 dahn Exp $
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
 *			
 *  Update Log:
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Jun 06 2006 DHA: Added DOXYGEN comments on the file scope 
 *                         and the class (driver_base_t) 
 *        Jan 08 2006 DHA: Created file.
 */ 

#ifndef SDBG_BASE_DRIVER_HXX
#define SDBG_BASE_DRIVER_HXX 1

#if HAVE_STRING
# include <string>
#else
# error string is required
#endif

#include "sdbg_std.hxx"
#include "sdbg_base_mach.hxx"
#include "sdbg_event_manager.hxx"
#include "sdbg_base_launchmon.hxx"

//! FILE: sdbg_base_driver.hxx
/*!
    The file contains the base driver class.
    a driver drives the topmost launchmon procedure.
*/

//! enumerator driver_error_e
/*!
    Defines a set of driver's return codes.   
*/
enum driver_error_e {
  SDBG_DRIVER_OK,
  SDBG_DRIVER_FAILED
};


//! class driver_base_t
/*!
    This class uses its containing event_manager_t object
    and launchmon_base_t object to drive the entire 
    launchmon procedure.

    This is a pure abstract class; has a couple of pure
    virual methods: overloaded create_process method. Whoever
    inherit this base class must implement create_process
    methods in a way that makes sense to its own platform.
*/

template <SDBG_DEFAULT_TEMPLATE_WIDTH> 
class driver_base_t 
{
public:
  
  /*
   * constructors and destructors
   */
  driver_base_t();
  driver_base_t(const driver_base_t& d);
  virtual ~driver_base_t();

  /*
   * accessors
   */
  event_manager_t<SDBG_DEFAULT_TEMPLPARAM>* get_evman();
  launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>* get_lmon();
  void set_evman(event_manager_t<SDBG_DEFAULT_TEMPLPARAM>* ev); 
  void set_lmon(launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>* lm);

  driver_error_e drive_engine ( opts_args_t* opt );
  driver_error_e drive ( int argc, char** argv );
  driver_error_e drive ( opts_args_t* opt );

  virtual process_base_t<SDBG_DEFAULT_TEMPLPARAM>* 
                 create_process ( pid_t pid, 
				  const std::string &mi,
				  const std::string &md,
				  const std::string &mt,
				  const std::string &mc ) = 0;

  virtual process_base_t<SDBG_DEFAULT_TEMPLPARAM>* 
                 create_process ( pid_t pid, 
				  const std::string &mi ) = 0;

  
private:

  bool LEVELCHK(self_trace_verbosity level) 
       { return (self_trace_t::driver_module_trace.verbosity_level >= level); }

  //
  // event manage object
  //
  event_manager_t<SDBG_DEFAULT_TEMPLPARAM>* evman;

  //
  // launchmon object
  //
  launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>* lmon;

  //
  // TODO: do we still need this?
  //
  //int pipe_fd[2];

  /*
   * For self tracing
   */
  std::string MODULENAME;  

};

#endif // SDBG_BASE_DRIVER_HXX
