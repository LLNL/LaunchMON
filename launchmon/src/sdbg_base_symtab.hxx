/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_base_symtab.hxx,v 1.4.2.1 2008/02/20 17:37:57 dahn Exp $
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
 *        Oct 27 2010 DHA: Added is_defined, is_globally_visible, 
 *                         is_locally_visible virtual methods.
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Mar 30 2006 DHA: Added exception handling support
 *        Feb 08 2006 DHA: Seperated OS dependent components 
 *                         from this file
 *        Jan 10 2006 DHA: Created file.          
 */ 

#ifndef SDBG_BASE_SYMTAB_HXX
#define SDBG_BASE_SYMTAB_HXX 1

#if HAVE_IOSTREAM
# include <iostream>
#else
# error iostream is required
#endif

#if HAVE_STRING
# include <string>
#else
# error string is required
#endif 

#if HAVE_MAP
# include <map>
#else
# error map is required
#endif

#include "sdbg_std.hxx"
#include "sdbg_self_trace.hxx"
#include "sdbg_base_exception.hxx"

#define BASE_SYMTAB_TEMPLATELIST typename VA
#define BASE_SYMTAB_TEMPLPARAM   VA
#define BASE_IMAGE_TEMPLATELIST  typename VA, typename EXECHANDLER
#define BASE_IMAGE_TEMPLPARAM    VA,EXECHANDLER

#if BIT64
const u_int64_t SYMTAB_UNINIT_ADDR = 0xdeadbeefdeadbeefULL;
const u_int64_t SYMTAB_INIT_IMAGE_BASE = SYMTAB_UNINIT_ADDR;
#else
const u_int32_t SYMTAB_UNINIT_ADDR = 0xdeadbeef;
const u_int32_t SYMTAB_INIT_IMAGE_BASE = SYMTAB_UNINIT_ADDR;
#endif
const char * const SYMTAB_UNINIT_STRING = "SDBG-NA";


//! enum symtab_error_e
/*!
    error code enumerator for the symbol table layer. 
    Populate this structure if you need finer granularity 
    for the error detecting.
*/
enum symtab_error_e {
  SDBG_SYMTAB_OK,
  SDBG_SYMTAB_FAILED
};
 

//! class symtab_exception_t : public exception_base_t
/*!
    The exception class for symbol table layer
*/
class symtab_exception_t : public exception_base_t
{
public:

  symtab_exception_t ()                          { }
  symtab_exception_t ( const char *m, symtab_error_e e )
                     { set_message (m); 
                       error_code = e;
                       set_type ( std::string ( "SDBG_SYMTAB_ERROR" ) ); 
                       set_fn ( std::string (__FILE__) );
                       set_ln ( __LINE__ ); 
                      }
             
  symtab_exception_t ( const std::string &m, symtab_error_e e )      
                     { set_message (m); 
                       error_code = e;
                       set_type ( std::string ( "SDBG_SYMTAB_ERROR" ) ); 
                       set_fn ( std::string (__FILE__) );
                       set_ln ( __LINE__ ); 
                      }
  virtual ~symtab_exception_t()                  { }

private:
  symtab_error_e error_code;  
};


//! class symbol_base_t<>
/*!
    symbol_base_t is the base class from which all kinds of 
    symbol class can be derived.
*/
template <BASE_SYMTAB_TEMPLATELIST>
class symbol_base_t
{
public:
  
  //
  // constructosr & destructor
  //
  symbol_base_t ();
  symbol_base_t ( const std::string &n, 
		  const std::string &bln, 
		  const VA rd, 
		  const VA rla);
  symbol_base_t ( const symbol_base_t &sobj ); 
  virtual ~symbol_base_t();

  //
  // accessors
  //
  void set_name (const std::string &n);
  void set_base_lib_name (const std::string &bln);
  void set_raw_address (const VA &ra);
  void set_relocated_address (const VA &ra); 
  const std::string & get_name() const;    
  const std::string & get_base_lib_name() const; 
  const VA & get_raw_address() const;
  const VA & get_relocated_address() const;

  virtual bool is_defined() const { return false; }
  virtual bool is_globally_visible() const { return false; }
  virtual bool is_locally_visible() const { return false; }

  //
  // overloading operator!
  //
  friend bool operator!(const symbol_base_t<BASE_SYMTAB_TEMPLPARAM> &sym)
    {
      return ((sym.name == SYMTAB_UNINIT_STRING)? true : false);
    }

private:
  std::string name;
  std::string base_lib_name;
  VA raw_address;
  VA relocated_address;
};


//! ltstr
/*!
    Contains helper methods for sorting symbol list
*/
struct ltstr
{
  bool operator() ( const std::string &s1, 
		    const std::string &s2 ) const
  {
    return (s1 < s2); 
  } 

  bool operator() ( const int i1, 
		    const int i2) const
  {
    return (i1 < i2);
  }
};


//!
/*! image_base_t<>
    base class representing an image (executable or libraries)
*/
template <BASE_IMAGE_TEMPLATELIST>
class image_base_t
{
public:
  
  //
  // constructors & destructor
  //
  image_base_t ();
  image_base_t ( const std::string &lib );
  image_base_t ( const image_base_t<BASE_IMAGE_TEMPLPARAM> &im );
  virtual ~image_base_t ();

  //
  // accessors
  //
  void set_base_image_name ( std::string &n );
  void set_path ( std::string &n );
  void set_image_base_address ( VA ba );
  void set_native_exec_handler ( EXECHANDLER *h );

  const std::string& get_base_image_name() const;
  const std::string& get_path() const;
  const VA& get_image_base_address() const;    
  EXECHANDLER * get_native_exec_handler();  
  const symbol_base_t<VA> & get_a_symbol 
    ( const std::string &key ) const;
  std::map<std::string, std::string, ltstr> & get_dso_list () 
    { return dso_list; }
 
  
  //
  // OPs on symtabs
  //
  void print_sorted_linkage_symtab();  
  symtab_error_e compute_reloc()
    throw ( symtab_exception_t );
  //void print_sorted_debug_symtab();  
  
  //
  // pure virtual methods
  //
  symtab_error_e init(const std::string &lib)
    throw ( symtab_exception_t );
  virtual symtab_error_e init() 
    throw ( symtab_exception_t ) = 0;
  virtual symtab_error_e read_linkage_symbols() 
    throw (symtab_exception_t) = 0;
  virtual symtab_error_e fetch_DSO_info (std::string&, bool&)
    throw (symtab_exception_t) = 0;
  // virtual symtab_error_e read_debug_symbols()
  //   throw ( symtab_exception_t ) = 0; 


  //
  // Some Util methods.
  //
  virtual const std::string decode_binding(int code) const     
                          { return std::string(SYMTAB_UNINIT_STRING); }
  virtual const std::string decode_visibility (int code) const
                          { return std::string(SYMTAB_UNINIT_STRING); }
  virtual const std::string decode_type(int code) const
                          { return std::string(SYMTAB_UNINIT_STRING); }

protected:
  std::map<std::string, symbol_base_t<VA>*, ltstr> linkage_symtab;
  std::map<std::string, symbol_base_t<VA>*, ltstr> debug_symtab;
  std::map<std::string, std::string, ltstr > dso_list;

private:

  bool LEVELCHK(self_trace_verbosity level) 
       { return (self_trace_t::symtab_module_trace.verbosity_level >= level); }

  // For self tracing
  //
  std::string MODULENAME;

  std::string base_image_name;
  std::string path;
  VA image_base_address;
  EXECHANDLER* native_exec_handler;
};

#endif // SDBG_BASE_SYMTAB_HXX
