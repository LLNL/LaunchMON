/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_base_ttracer.hxx,v 1.5.2.1 2008/02/20 17:37:57 dahn Exp $
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
 *        Mar 30 2006 DHA: Added exception handling support
 *        Feb 07 2006 DHA: Created file.
 */

#ifndef SDBG_BASE_TTRACER_HXX
#define SDBG_BASE_TTRACER_HXX 1

#include "sdbg_std.hxx"
#include "sdbg_base_mach.hxx"
#include "sdbg_base_exception.hxx"


//! ttracer_error_e enumerator
/*!
    defines error codes for thread tracer
*/
enum ttracer_error_e {
  SDBG_TTRACE_OK,
  SDBG_TTRACE_FAILED
};


//! class thread_tracer_exception_t : public exception_base_t
/*!
    The exception class for symbol table layer
*/
class thread_tracer_exception_t : public exception_base_t
{

public:
  ttracer_error_e error_code;

  thread_tracer_exception_t ()                          { }
  thread_tracer_exception_t ( const char* m, ttracer_error_e e )
                     { set_message (m);
                       error_code = e;
                       set_type ( std::string ( "SDBG_THRTRACER_ERROR" ) );
                       set_fn ( std::string (__FILE__) );
                       set_ln ( __LINE__ );
                      }
  thread_tracer_exception_t ( const std::string& m, ttracer_error_e e )      
                     { set_message (m);
                       error_code = e;
                       set_type ( std::string ( "SDBG_THRTRACER_ERROR" ) );
                       set_fn ( std::string (__FILE__) );
                       set_ln ( __LINE__ );
                      }
  virtual ~thread_tracer_exception_t()                  { }
};


//! class thread_tracer_base_t
/*!

    Abstract thread tracer class 
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
class thread_tracer_base_t 
{

public:
  thread_tracer_base_t ()          { tracer = NULL; }
  virtual ~thread_tracer_base_t () { }

  static 
  void set_tracer ( tracer_base_t<SDBG_DEFAULT_TEMPLPARAM>* t ) 
                                   { tracer = t; }
  static 
  tracer_base_t<SDBG_DEFAULT_TEMPLPARAM>* get_tracer()          
                                   { return tracer; }

  ////////////////////////////////////////////////////////////
  //
  //  Virutal Interfaces. 
  //
  //  ttracer_init typically gets called right after the target 
  //  is detected threaded.
  //
  //  ttracer_attach is typically called when a new thread gets
  //  created and need to be attached.
  //
  virtual ttracer_error_e ttracer_init 
     ( process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, 
       tracer_base_t<SDBG_DEFAULT_TEMPLPARAM>* pt )       
    throw ( thread_tracer_exception_t ) = 0;
  
  virtual ttracer_error_e ttracer_attach
     ( process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p )       
    throw ( thread_tracer_exception_t )= 0; 

private:
  static tracer_base_t<SDBG_DEFAULT_TEMPLPARAM>* tracer;

};


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_base_t<SDBG_DEFAULT_TEMPLPARAM>* 
thread_tracer_base_t<SDBG_DEFAULT_TEMPLPARAM>::tracer = NULL;

#endif // SDBG_BASE_TTRACER_HXX 
