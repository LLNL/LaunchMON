/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/sdbg_linux_driver.hxx,v 1.4.2.2 2008/02/20 17:37:57 dahn Exp $
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
 *        Feb  09 2008 DHA: Added LLNS Copyright
 *        Jan  08 2006 DHA: Created file.          
 */ 

#ifndef SDBG_LINUX_DRIVER_HXX
#define SDBG_LINUX_DRIVER_HXX 1

#include <string>
#include "sdbg_base_driver.hxx"
#include "sdbg_linux_mach.hxx"

#define LINUX_DRIVER_TEMPLATELIST typename VA, \
                                  typename WT, \
                                  typename IT, \
                                  typename GRS,\
                                  typename FRS
#define LINUX_DRIVER_TEMPLPARAM   VA,WT,IT,GRS,FRS


//! class linux_driver_t
/*!
    
*/
template <LINUX_DRIVER_TEMPLATELIST>
class linux_driver_t 
  : public driver_base_t<LINUX_DRIVER_TEMPLPARAM,my_thrinfo_t,elf_wrapper> 
{

public:
  
  //
  // constructors & destructor
  //
  linux_driver_t();
  linux_driver_t(const linux_driver_t& d);
  virtual ~linux_driver_t();

  //
  // "main" for the whole project 
  //
  int driver_main (int argc, char *argv[]);

  virtual process_base_t<LINUX_DRIVER_TEMPLPARAM,my_thrinfo_t,elf_wrapper>* 
      create_process (pid_t pid, 
		      const std::string& mi,
		      const std::string& md, 
		      const std::string& mt,
		      const std::string& mc);
  
  virtual process_base_t<LINUX_DRIVER_TEMPLPARAM,my_thrinfo_t,elf_wrapper>* 
      create_process ( pid_t pid, const std::string &mi);
};

#endif // SDBG_LINUX_DRIVER_HXX
