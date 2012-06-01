/*
 * $Header: $
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
 *        May 31 2012 DHA: Copied from the 0.8-middleware-support branch
 *        Jul 02 2010 DHA: Created file.
 */

//
//
// WARNING this module has not been implemented
//
//
#ifndef SDBG_RM_SPAWNER_HXX
#define SDBG_RM_SPAWNER_HXX 1

#include "sdbg_std.hxx"

#if HAVE_STRING
# include <string>
#else
# error string is required
#endif 

#include "sdbg_rm_map.hxx"
#include "sdbg_self_trace.hxx"

class spawner_rm_t : public spawner_base_t
{
public:
  ////////////////////////////////////////////////////////////
  //
  //  Public Interfaces
  //
  spawner_rm_t();
  ~spawner_rm_t();

  spawner_rm_t(const std::string &rac,
		 const std::vector<std::string> & racargs,
		 const std::string ep);

  spawner_rm_t(const std::string &rac,
		 const std::vector<std::string> & racargs,
		 const std::string ep,
		 const std::vector<std::string> & hosts);

  virtual ~spawner_rm_t();

  virtual bool spawn();

  virtual int combineHosts(std::vector<std::string> &combHosts);

  static spawner_rm_t *new_spawner_rm_t(...);

private:

  explict spawner_rm_t (const spawner_rsh_t & s);
};

#endif // SDBG_RM_SPAWNER_HXX
