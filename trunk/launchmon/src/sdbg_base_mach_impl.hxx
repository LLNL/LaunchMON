/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_base_mach_impl.hxx,v 1.5.2.1 2008/02/20 17:37:57 dahn Exp $
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
 *        May 22 2006 DHA: Added exception support for the machine layer
 *        Feb 06 2006 DHA: del support 
 *        Jan 11 2006 DHA: Created file.          
 */ 

#ifndef SDBG_BASE_MACH_IMPL_HXX 
#define SDBG_BASE_MACH_IMPL_HXX 1

extern "C" {
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#else
# error sys/types.h is required
#endif 

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#else
# error sys/wait.h is required
#endif

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#else
# error sys/stat.h is required
#endif

#if HAVE_SYS_USER_H
# include <sys/user.h>
#else
# error sys/user.h is required
#endif

#if HAVE_ERRNO_H
# include <errno.h>
#else
# error errno.h is required
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#else
# error unistd.h is required
#endif

#if HAVE_SIGNAL_H
# include <signal.h>
#else
# error signal.h is required
#endif
}

#include <iostream>
#include <cstring>

#include "sdbg_base_symtab.hxx"
#include "sdbg_base_symtab_impl.hxx"
#include "sdbg_base_mach.hxx"


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class register_set_t
//
//

//! PUBLIC: register_set_base_t
/*!
    Default constructor.
*/
template <typename NATIVE_RS,typename VA,typename WT>
register_set_base_t<NATIVE_RS,VA,WT>::register_set_base_t ()
  :  offset_in_user(0), rs_ptr(0), writable_mask(0)
{

}


template <typename NATIVE_RS,typename VA,typename WT>
register_set_base_t<NATIVE_RS,VA,WT>::register_set_base_t 
(const int offset) : rs_ptr(0), writable_mask(0)
{
  offset_in_user = offset; 
}


template <typename NATIVE_RS,typename VA,typename WT>
register_set_base_t<NATIVE_RS,VA,WT>::register_set_base_t
(const register_set_base_t<NATIVE_RS,VA,WT>& r)
{
  memcpy(&rs, &(r.rs), sizeof(NATIVE_RS));
  offset_in_user = r.offset_in_user;  
  rs_ptr = &rs + (r.rs_ptr - &(r.rs));
  writable_mask = r.writable_mask; 
}


template <typename NATIVE_RS,typename VA,typename WT>
register_set_base_t<NATIVE_RS,VA,WT>::~register_set_base_t ()
{
  // Nothing to destroy
}  


//! PUBLIC:  size_in_word
/*!
   
*/
template <typename NATIVE_RS,typename VA,typename WT>
unsigned int register_set_base_t<NATIVE_RS,VA,WT>::size_in_word()
{
  return (sizeof(NATIVE_RS)/sizeof(WT));
}


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class thread_base_t
//
//

//! PUBLIC: constructors and destructor
/*!
    
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
thread_base_t<SDBG_DEFAULT_TEMPLPARAM>::thread_base_t() 
  : master_thread(false), 
    master_pid(0),
    gprset(NULL), 
    fprset(NULL)
{
  /* more init ? */
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
thread_base_t<SDBG_DEFAULT_TEMPLPARAM>::~thread_base_t() 
{
  if (gprset)
    delete gprset;
 
  if (fprset)
    delete fprset;
}


//! PUBLIC: accessors
/*!
    
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
register_set_base_t<GRS,VA,WT>* 
thread_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_gprset()
{
  return gprset;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void thread_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_gprset
( register_set_base_t<GRS,VA,WT>* g )
{
  gprset = g;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
register_set_base_t<FRS,VA,WT>* 
thread_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_fprset()
{
  return fprset;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void thread_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_fprset
(register_set_base_t<FRS,VA,WT>* f)
{
  fprset = f;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
NT& thread_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_thread_info()
{
   return thread_info;
}


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class process_base_t)
//
//


//! PUBLIC: process_base_t
/*!
    Default constructor.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::process_base_t () 
  : never_trapped(true), 
    please_detach(false),
    please_kill(false),
    reason(reserved_for_rent),
    myimage(NULL), 
    mydynloader_image(NULL),  
    mythread_lib_image(NULL),
    mylibc_image(NULL),
    myopts(NULL),
    launch_hidden_bp(NULL),
    loader_hidden_bp(NULL),
    thread_creation_hidden_bp(NULL),
    thread_death_hidden_bp(NULL),
    fork_hidden_bp(NULL)
{ }


//! PUBLIC: process_base_t
/*!
    Default constructor with path info
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::process_base_t ( const std::string& mi,  
							  const std::string& md, 
							  const std::string& mt,  
							  const std::string& mc)
{
  protected_init ( mi, md, mt, mc );
}


//! PUBLIC: ~process_base_t
/*!
    Destructor.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::~process_base_t ()
{
  if (myimage)
    delete myimage;

  if (mydynloader_image)
    delete mydynloader_image;

  if (mythread_lib_image)
    delete mythread_lib_image;

  if (mylibc_image)
    delete mylibc_image;

  // this should call destructors for each containing thread obj 
  //
  thrlist.clear(); 
}


//! PUBLIC: process_base_t
/*!
    accessors
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
std::map<int, thread_base_t<SDBG_DEFAULT_TEMPLPARAM>*, ltstr>& 
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_thrlist()
{
  return thrlist; 
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
const pid_t 
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_master_thread_pid()
{
  using namespace std;

  typename
    map<int,thread_base_t<SDBG_DEFAULT_TEMPLPARAM>*,ltstr>::const_iterator
      tpos;
  
  for (tpos=thrlist.begin(); tpos!=thrlist.end(); tpos++) 
    {
      if (tpos->second->get_master_thread())  
	return (tpos->second->get_master_pid());      
    }

  // this means, above operation couldn't find the master thread
  //
  return -1;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
image_base_t<VA,EXECHANDLER>* 
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_myimage ()
{
  return myimage;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
image_base_t<VA,EXECHANDLER>* 
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_mydynloader_image ()
{
  return mydynloader_image;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
image_base_t<VA,EXECHANDLER>* 
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_mythread_lib_image ()
{
  return mythread_lib_image;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
image_base_t<VA,EXECHANDLER>*
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_mylibc_image ()
{
  return mylibc_image;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
breakpoint_base_t<VA,IT>* 
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_launch_hidden_bp ()
{
  return launch_hidden_bp;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
breakpoint_base_t<VA,IT>*
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_fork_hidden_bp ()
{
  return fork_hidden_bp;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
breakpoint_base_t<VA,IT>* 
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_loader_hidden_bp ()
{
  return loader_hidden_bp;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
breakpoint_base_t<VA,IT>* 
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_thread_creation_hidden_bp ()
{
  return thread_creation_hidden_bp;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
breakpoint_base_t<VA,IT>* 
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_thread_death_hidden_bp ()
{
  return thread_death_hidden_bp;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void process_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_myimage 
(image_base_t<VA,EXECHANDLER>* i)
{
  myimage = i;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void process_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_mydynloader_image 
(image_base_t<VA,EXECHANDLER>* i)
{
  mydynloader_image = i; 
}

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void process_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_mythread_lib_image 
(image_base_t<VA,EXECHANDLER>* i)
{
  mythread_lib_image = i;
}

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void process_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_mylibc_image
(image_base_t<VA,EXECHANDLER>* i)
{
  mylibc_image = i;
}

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void process_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_launch_hidden_bp
(breakpoint_base_t<VA,IT>* b)
{
  launch_hidden_bp = b;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
  void process_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_loader_hidden_bp
(breakpoint_base_t<VA,IT>* b)
{
  loader_hidden_bp = b;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void process_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_thread_creation_hidden_bp
(breakpoint_base_t<VA,IT>* b)
{
  thread_creation_hidden_bp = b;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void process_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_thread_death_hidden_bp
(breakpoint_base_t<VA,IT>* b) 
{
  thread_death_hidden_bp = b;
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void process_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_fork_hidden_bp
(breakpoint_base_t<VA,IT>* b)
{
  fork_hidden_bp = b;
}


//! PROTECTED: process_base_t
/*!
    get_pid: this is context sensitive get_pid
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
const pid_t 
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_pid ( bool context_sensitive )
{
  pid_t retpid = THREAD_KEY_INVALID;

  if (context_sensitive) 
    {
      if (key_to_thread_context == THREAD_KEY_INVALID) 
	{
	  // FIXME: gen_err caller wanted context sensitive
	  // gprset, yet there is no context set at the moment
	  retpid = THREAD_KEY_INVALID;
	}

      if ( thrlist.find(key_to_thread_context) != thrlist.end() ) 
	{    
	  retpid = thrlist.find(key_to_thread_context)->second->thr2pid();
	}    
    }
  else 
    {    
      retpid = get_master_thread_pid();
    }

  return retpid;
}

//! PROTECTED: process_base_t
/*!
    basic_init
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
bool process_base_t<SDBG_DEFAULT_TEMPLPARAM>::protected_init ( const std::string& mi, 
							       const std::string& md, 
							       const std::string& mt, 
							       const std::string& mc)
{
  try
    {
      bool rc = true;

      if (myimage) 
	{
	  myimage->init();
	  myimage->read_linkage_symbols();
	}
      else
	rc = false;      

      if (mydynloader_image) 
	{
	  mydynloader_image->init();
	  mydynloader_image->read_linkage_symbols();
	}
      else
	rc = false;

      if (mythread_lib_image)	
	{
	  mythread_lib_image->init();
	  mythread_lib_image->read_linkage_symbols();
	}
      else 
	rc = false;

      if (mylibc_image) 
	{               
	  mylibc_image->init();
	  mylibc_image->read_linkage_symbols();
	}
      else    
	rc = false;     

      return rc;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      abort();
    }
}

//! PROTECTED: process_base_t
/*!
    basic_init
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
bool process_base_t<SDBG_DEFAULT_TEMPLPARAM>::protected_init ( const std::string& mi ) 
{  
  try
    {
      using namespace std;

      bool rc = true;
      string loader_loc;
      string libpthread_loc;
      string libc_loc;
      bool is_threaded = false;
      bool found_interp = false;
      bool found_runtime = false;

      if (myimage) 
	{

	  myimage->init();
	  myimage->read_linkage_symbols();

	  myimage->fetch_DSO_info ( loader_loc, 
				    found_interp, 
				    libpthread_loc,
				    is_threaded,
				    libc_loc,
				    found_runtime );
	}
      else
	rc = false;      
  
      if (found_interp && mydynloader_image) 
	{    
	  mydynloader_image->init(loader_loc);
	  mydynloader_image->read_linkage_symbols();
	}  
      else
	rc = false;   

      if (is_threaded && mythread_lib_image) 
	{    
	  mythread_lib_image->init(libpthread_loc);
	  mythread_lib_image->read_linkage_symbols();
	}

      if (found_runtime && mylibc_image) 
	{     
	  mylibc_image->init(libc_loc);
	  mylibc_image->read_linkage_symbols();
	}

      return rc;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      abort();
    }
}


//! PROTECTED: process_base_t
/*!
    make_context
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
bool 
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::make_context ( const int key ) 
  throw (machine_exception_t)
{
  using namespace std;

  bool rc = false;
  string e;
  string func = "[process_base_t::make_context]";

  if ( thrlist.find(key) != thrlist.end() ) 
    {
      key_to_thread_context = key;
      rc = true;
    }
  else 
    {    
      e = func + 
	"no thread with the key value found ";
      throw (machine_exception_t(e));
    }

  return rc;    
}


//! PROTECTED: process_base_t
/*!
    check_and_undo_context
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
bool process_base_t<SDBG_DEFAULT_TEMPLPARAM>::check_and_undo_context ( const int key ) 
  throw (machine_exception_t)
{
  using namespace std;

  // 3/16 IMPORTANT FIX NEEDED: when thread gets exit event while thread list is being
  // traversed, this argument "key" can contain garbage. requires a function 
  // prototype change.

  bool rc = false;
  string e;
  string func = "[process_base_t::check_and_undo_context]";
  if (key_to_thread_context == key) 
    {      
      key_to_thread_context = THREAD_KEY_INVALID;
      rc = true;
    }
  else 
    {
      e = func + 
	"key_to_thread_context is different from the passed key";
      throw (machine_exception_t(e));
    }
  return rc;    
}

//! PROTECTED: process_base_t
/*!
    get_gprset
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
register_set_base_t<GRS,VA,WT>* 
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_gprset
(bool context_sensitive)
{
  using namespace std;
  typename
      map<int,thread_base_t<SDBG_DEFAULT_TEMPLPARAM>*,ltstr>::const_iterator
      tpos;
  

  if (context_sensitive) 
    {    
      if (key_to_thread_context == THREAD_KEY_INVALID) 
	{ 
	  return NULL;
	}

      if ( thrlist.find(key_to_thread_context) != thrlist.end() ) 
	{    
	  return (thrlist.find(key_to_thread_context)->second->get_gprset());
	}
    }
  else 
    {         
      for (tpos=thrlist.begin(); tpos!=thrlist.end(); tpos++) 
	{	  
	  if (tpos->second->get_master_thread()) 
	    {     
	      return (tpos->second->get_gprset());
	    }
	}    
    }
            
  return NULL;  
}


//! PROTECTED: process_base_t
/*!
    get_gprset
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
register_set_base_t<FRS,VA,WT>* 
process_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_fprset ( bool context_sensitive )
{
  using namespace std;

  typename
    map<int,thread_base_t<SDBG_DEFAULT_TEMPLPARAM>*,ltstr>::const_iterator
    tpos;

  if (context_sensitive) 
    {    
      if (key_to_thread_context == THREAD_KEY_INVALID) 	
	return NULL;	

      if ( thrlist.find(key_to_thread_context) != thrlist.end() )
	return (thrlist.find(key_to_thread_context)->second->get_fprset());   
    }
  else 
    {      
      for (tpos=thrlist.begin(); tpos!=thrlist.end(); tpos++) 
	{
	  if (tpos->second->get_master_thread()) 	    
	    return (tpos->second->get_fprset());     
	}
    }
            
  return NULL;  
}

#endif // SDBG_BASE_MACH_IMPL_HXX
