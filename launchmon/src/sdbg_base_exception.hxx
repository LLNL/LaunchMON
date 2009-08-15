/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_base_exception.hxx,v 1.2.2.1 2008/02/20 17:37:56 dahn Exp $
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
 *        Aug 15 2007 DHA: File created
 *
 */

#ifndef SDBG_BASE_EXCEPTION_HXX
#define SDBG_BASE_EXCEPTION_HXX 1

#if HAVE_STRING
# include <string>
#else
# error string is required
#endif

#include "sdbg_self_trace.hxx"

class exception_base_t
{
 
public:
 
  exception_base_t ()                         { }
  explicit exception_base_t ( const char* m ) { message = m; }
  explicit exception_base_t( const std::string& m )    
					      { message = m; }
  virtual ~exception_base_t ( )               { }
  virtual void report()  {
    self_trace_t::trace ( true, type, true,
                          "[filename: %s, linenum: %d] %s",
                          fn.c_str(),
                          ln,
                          message.c_str());
    if ( bt.size() != 0 )
      {
        self_trace_t::trace ( true, type, true,  "%s", bt.c_str() );
      }
  }
 
  define_gset ( std::string, message )
  define_gset ( std::string, type )
  define_gset ( std::string, bt )
  define_gset ( std::string, fn )
  define_gset ( int, ln )
 
private:
 
  std::string message;
  std::string type;
  std::string bt; 
  std::string fn;
  int ln;
};

#endif // SDBG_BASE_EXCEPTION_HXX
