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
 *        Mar  11 2008 DHA: Added PowerPC support
 *        Feb  09 2008 DHA: Added LLNS Copyright
 *        Mar  13 2007 DHA: pipe_t support
 *        Jan  09 2006 DHA: Linux X86/64 support   
 *        Dec  19 2006 DHA: Added driver_forkmain support
 *        Jan  08 2006 DHA: Created file.
 */

#ifndef SDBG_LINUX_DRIVER_IMPL_HXX
#define SDBG_LINUX_DRIVER_IMPL_HXX 1

#include "sdbg_base_driver.hxx"
#include "sdbg_base_driver_impl.hxx"

#include "sdbg_linux_mach.hxx"
#include "sdbg_linux_launchmon.hxx"
#include "sdbg_linux_driver.hxx"
 

////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class symbol_base_t<>)
//
////////////////////////////////////////////////////////////////////

//!
/*!  driver_base_t<> constructors
      
    
*/
template <LINUX_DRIVER_TEMPLATELIST>
linux_driver_t<LINUX_DRIVER_TEMPLPARAM>::linux_driver_t ()
{
  // more init
}

template <LINUX_DRIVER_TEMPLATELIST>
linux_driver_t<LINUX_DRIVER_TEMPLPARAM>::linux_driver_t (
                 const linux_driver_t& d )
{
  // copy
}


template <LINUX_DRIVER_TEMPLATELIST>
linux_driver_t<LINUX_DRIVER_TEMPLPARAM>::~linux_driver_t ()
{
  // destroy
}


//!
/*!  driver_base_t<> create_process
     
     creates a platform specific process object 
    
*/
template <LINUX_DRIVER_TEMPLATELIST> 
process_base_t<LINUX_DRIVER_TEMPLPARAM,my_thrinfo_t,elf_wrapper>*
linux_driver_t<LINUX_DRIVER_TEMPLPARAM>::create_process ( 
                 pid_t pid, 
		 const std::string& mi, 
		 const std::string& md, 
		 const std::string& mt,
		 const std::string& mc )
{
  
  process_base_t<LINUX_DRIVER_TEMPLPARAM,my_thrinfo_t,elf_wrapper>* 
    return_proc;

#if X86_ARCHITECTURE || X86_64_ARCHITECTURE
  return_proc = new linux_x86_process_t(pid, mi, md, mt, mc);
#elif PPC_ARCHITECTURE
  return_proc = new linux_ppc_process_t(pid, mi, md, mt, mc);
#elif IA64_ARCHITECTURE
  return_proc = new linux_ia64_process_t(pid, mi, md, mt, mc);
#endif

  return return_proc;

}


//!
/*!  driver_base_t<> create_process
     
     Method that creates a platform specific process object.
     This is the method that is called by the ::drive_engine()
    
*/
template <LINUX_DRIVER_TEMPLATELIST> 
process_base_t<LINUX_DRIVER_TEMPLPARAM,my_thrinfo_t,elf_wrapper>*
linux_driver_t<LINUX_DRIVER_TEMPLPARAM>::create_process ( 
		 pid_t pid, 
		 const std::string &mi )
{
  process_base_t<LINUX_DRIVER_TEMPLPARAM,my_thrinfo_t,elf_wrapper> *return_proc;

  //
  // different architectures require different sublayer 
  // in creating a process object.
  //
#if X86_ARCHITECTURE || X86_64_ARCHITECTURE
  return_proc = new linux_x86_process_t (pid, mi);
#elif PPC_ARCHITECTURE
  return_proc = new linux_ppc_process_t (pid, mi);
#elif IA64_ARCHITECTURE
  return_proc = new linux_ia64_process_t (pid, mi);
#endif

  return return_proc;

}

//int MPIR_being_debugged = 0;

//!
/*!  driver_base_t<> driver_main
  
     main entry point that the standalone launchmon tool invokes

*/
template <LINUX_DRIVER_TEMPLATELIST> 
int 
linux_driver_t<LINUX_DRIVER_TEMPLPARAM>::driver_main
( int argc, char **argv )
{
  try
   {
     driver_error_e error_code;

     event_manager_t<LINUX_DRIVER_TEMPLPARAM,my_thrinfo_t,elf_wrapper>* em;
     launchmon_base_t<LINUX_DRIVER_TEMPLPARAM,my_thrinfo_t,elf_wrapper>* lm;

     em = new event_manager_t<LINUX_DRIVER_TEMPLPARAM,my_thrinfo_t,elf_wrapper>();
     lm = new linux_launchmon_t();
     this->set_evman(em);
     this->set_lmon(lm);
  
     //
     // Start driving events, calling into the base driver layer 
     //
     error_code = driver_base_t<LINUX_DRIVER_TEMPLPARAM, my_thrinfo_t,elf_wrapper>::drive ( argc, argv );

     return ( ( error_code == SDBG_DRIVER_OK) ? 0 : 1 );
   }
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
}


#endif // SDBG_LINUX_DRIVER_IMPL_HXX
