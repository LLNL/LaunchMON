/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/sdbg_linux_symtab_impl.hxx,v 1.4.4.2 2008/02/20 17:37:57 dahn Exp $
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
 *        Mar 06 2009 DHA: Folded in Mark O'Connor's patch that 
 *                         allows use of dynamic symbol table 
 *                         in the absence of reguar linkage symbol 
 *                         table.
 *                         Changed fetch_DSO_info such that it 
 *                         only cares about the runtime linker SO.
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Jan 09 2006 DHA: Linux X86/64 support
 *        Jul 03 2006 DHA: Added some self tracing support
 *        Mar 30 2006 DHA: Added exception handling support
 *        Mar 16 2006 DHA: The file is created so that
 *                         linux specific components could be
 *                         extracted out from the base symbol 
 *                         table class. Note that LINUX only 
 *                         supports ELF as the file format and 
 *                         DWARF as the debug info format.
 *
 */ 

#ifndef SDBG_LINUX_SYMTAB_IMPL_HXX
#define SDBG_LINUX_SYMTAB_IMPL_HXX 1

#include "sdbg_std.hxx"

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
#endif

#include <vector>
#include <string>
#include <iostream>

extern "C" {
#if HAVE_LIBGEN_H
# include <libgen.h>
#else
# error libgen.h is required
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#else
# error sys/types.h is required 
#endif

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#else
# error sys/stat.h is required
#endif

#if HAVE_FCNTL_H
# include <fcntl.h>
#else
# error fcntl.h is required 
#endif

#if HAVE_ERRNO_H
# include <errno.h>
#else
# error errno.h is required 
#endif
}

#include "sdbg_linux_std.hxx"
#include "sdbg_linux_symtab.hxx"


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class linkage_symbol_t<>)
//
//


//!  PUBLIC: 
/*!  linkage_symbol_t<> constructors
      
    
*/
template <LINUX_SYMTAB_TEMPLATELIST>
linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::linkage_symbol_t()
  : symbol_base_t<LINUX_SYMTAB_TEMPLPARAM>()
{ 
  section = SYMTAB_UNINIT_STRING;
  visibility = SYMTAB_UNINIT_STRING;
  binding = SYMTAB_UNINIT_STRING; 
  type = SYMTAB_UNINIT_STRING;
}


template <LINUX_SYMTAB_TEMPLATELIST>
linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::linkage_symbol_t 
(const linkage_symbol_t& l) 
  : symbol_base_t<LINUX_SYMTAB_TEMPLPARAM>(l)
{ 
  section = l.section; 
  visibility = l.visibility;
  binding = l.binding;
}


template <LINUX_SYMTAB_TEMPLATELIST>
linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::linkage_symbol_t 
(const std::string& n, const std::string& ln, 
 const VA ra, const VA rla)
  : symbol_base_t<LINUX_SYMTAB_TEMPLPARAM>(n,ln,ra,rla)
{
  section = SYMTAB_UNINIT_STRING;
  visibility = SYMTAB_UNINIT_STRING;
  binding = SYMTAB_UNINIT_STRING; 
  type = SYMTAB_UNINIT_STRING;
}
  

//!  PUBLIC: 
/*!  linkage_symbol_t<> accessors

*/
template <LINUX_SYMTAB_TEMPLATELIST>
void linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::set_name 
(const std::string& n) 
{ 
  symbol_base_t<LINUX_SYMTAB_TEMPLPARAM>::set_name(n);
}


template <LINUX_SYMTAB_TEMPLATELIST>
void linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::set_base_lib_name 
(const std::string& bln) 
{ 
  symbol_base_t<LINUX_SYMTAB_TEMPLPARAM>::set_base_lib_name(bln);
}


template <LINUX_SYMTAB_TEMPLATELIST>
void linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::set_binding 
(const std::string& b) 
{ 
  binding = b;
}


template <LINUX_SYMTAB_TEMPLATELIST>
void linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::set_visibility 
(const std::string& v) 
{ 
  visibility = v; 
}


template <LINUX_SYMTAB_TEMPLATELIST>
void linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::set_type 
(const std::string& t) 
{ 
  type = t; 
}


template <LINUX_SYMTAB_TEMPLATELIST>
const std::string& 
linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::get_name () const 
{
  return ( symbol_base_t<LINUX_SYMTAB_TEMPLPARAM>::
	   get_name()); 
}


template <LINUX_SYMTAB_TEMPLATELIST>
const std::string& 
linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::get_base_lib_name() const 
{ 
  return ( symbol_base_t<LINUX_SYMTAB_TEMPLPARAM>::
	   get_base_lib_name()); 
}


template <LINUX_SYMTAB_TEMPLATELIST>
const VA&
linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::get_raw_address() const 
{ 
  return ( symbol_base_t<LINUX_SYMTAB_TEMPLPARAM>::
	   get_raw_address()); 
}


template <LINUX_SYMTAB_TEMPLATELIST>
const VA& 
linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::get_relocated_address() const 
{ 
  return  ( symbol_base_t<LINUX_SYMTAB_TEMPLPARAM>::
	    get_relocated_address()); 
}


template <LINUX_SYMTAB_TEMPLATELIST>
const std::string
linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::get_binding () const 
{
  return binding; 
}


template <LINUX_SYMTAB_TEMPLATELIST>
const std::string
linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::get_visibility () const   
{ 
  return visibility; 
}


template <LINUX_SYMTAB_TEMPLATELIST>
const std::string
linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::get_type () const 
{ 
  return type;
}


// PUBLIC: linkage_symbol_t<>::print_me()
/*!
    Method allows this class to print its data; chiefly 
    for debugging purpose.
*/
template <LINUX_SYMTAB_TEMPLATELIST>
void linkage_symbol_t<LINUX_SYMTAB_TEMPLPARAM>::print_me () const
{
  using namespace std;

  cout << " ---- Symbol Info --- " << endl;
  cout << "Sym Name:   " << get_name() << endl;
  cout << "Value:       0x" << (int) get_raw_address() << endl;
  cout << "Type:       " << type << endl;	
  cout << "Binding:    " << binding << endl;	
  cout << "Visibility: " << visibility << endl;		
}


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class elf_wrapper)
//
//


//!  PUBLIC: 
/*!  elf_wrapper<BASE_SYMTAB_TYPES> constructors
      
    
*/
elf_wrapper::elf_wrapper() : elf_handler(NULL)
{
  exec = SYMTAB_UNINIT_STRING;    
}

elf_wrapper::elf_wrapper(const std::string& ex) 
  : elf_handler(NULL)
{
  exec = ex;  
}

elf_wrapper::elf_wrapper(const elf_wrapper& elfw) 
  : elf_handler(NULL)
{
  exec = elfw.exec;
  elf_handler = elfw.elf_handler;
}

elf_wrapper::~elf_wrapper()
{
  if (elf_handler)
    finalize(); 
}


//!  PUBLIC: 
/*!  elf_wrapper::init()
      
    
*/
symtab_error_e elf_wrapper::init()
throw(symtab_exception_t)
{
  using namespace std;

  string e;
  string func = "[elf_wrapper::init]";
  Elf_Kind kind;
  Elf *arf;

  if ( elf_version(EV_CURRENT) == EV_NONE ) 
    {
      e = func + " elf_version failed";
      throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);
    }

  if ( ( fd = open (exec.c_str(), O_RDONLY)) == -1 ) 
    {
      e = func + " couldn't open executable " + exec;
      throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);
    }

  if ( ( arf = elf_begin (fd, ELF_C_READ, NULL)) == NULL ) 
    {
      e = func + " elf_begin failed ";
      throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);
    }

  if ( ( kind = elf_kind (arf)) != ELF_K_ELF )
    {
      e = func + " elf_kind failed ";
      throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);
    }

  if ( ( elf_handler = elf_begin (fd, ELF_C_READ, arf)) == NULL )
    {
      e = func + " elf_begin failed ";
      throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);
    }

  return SDBG_SYMTAB_OK;
}


//!  PUBLIC: 
/*!  elf_wrapper::init(std::string& lib)


*/
symtab_error_e
elf_wrapper::init ( std::string& lib )
{
  exec = lib;
  return (init());  
}


//!  PUBLIC:
/*!  elf_wrapper<>::finalize()
      
    
*/
symtab_error_e elf_wrapper::finalize ()
{
  symtab_error_e rc = SDBG_SYMTAB_OK;
  
  if (elf_handler) {   

    if ( elf_end(elf_handler) < 0 ) {
      rc = SDBG_SYMTAB_FAILED;
    }
    elf_handler = NULL;
    if ( close(fd) < 0 ) {
      rc = SDBG_SYMTAB_FAILED;
    }
  }
  
  return rc;
}


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class linux_image_t<>)
//
//


//!  PUBLIC: 
/*!  linux_image_t<> constructors

*/
template <LINUX_IMAGE_TEMPLATELIST>
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::linux_image_t ()
  : image_base_t<LINUX_IMAGE_TEMPLPARAM,elf_wrapper>()
{
  MODULENAME = self_trace_t::symtab_module_trace.module_name;
}


template <LINUX_IMAGE_TEMPLATELIST>
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::linux_image_t 
(const std::string& lib) 
  : image_base_t<LINUX_IMAGE_TEMPLPARAM,elf_wrapper>(lib)
{
  MODULENAME = self_trace_t::symtab_module_trace.module_name;
}

template <LINUX_IMAGE_TEMPLATELIST>
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::linux_image_t 
(const image_base_t<VA,elf_wrapper>& im)
  : image_base_t<LINUX_IMAGE_TEMPLPARAM,elf_wrapper>(im)
{
  MODULENAME = im.MODULENAME;
}

template <LINUX_IMAGE_TEMPLATELIST>
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::~linux_image_t ()
{

}


//! PUBLIC: init
/*!
  
*/
template <LINUX_IMAGE_TEMPLATELIST>
symtab_error_e linux_image_t<LINUX_IMAGE_TEMPLPARAM>::init()
throw(symtab_exception_t)
{
  using namespace std;

  elf_wrapper* elfw;
  string e;  
  string func = "[linux_image_t::init]";

  if (get_base_image_name() == SYMTAB_UNINIT_STRING) 
    {
      e = func + 
	" Attempt to call init before setting the base image name. ";
      throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);
    }

  elfw = new elf_wrapper(get_path());  

  if (elfw->init() != SDBG_SYMTAB_OK) 
    {
      e = func + " ELF library init failed.";
      throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);
    }
  set_native_exec_handler(elfw);

  return SDBG_SYMTAB_OK;
}


//! PUBLIC: read_linkage_symbols
/*!
    It reads in all linkage symbols using the initialized
    Elf handler.
*/
template <LINUX_IMAGE_TEMPLATELIST>
symtab_error_e 
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::read_linkage_symbols ()
throw(symtab_exception_t)
{
  using namespace std;

  string e;  
  string func = "[linux_image_t::read_linkage_symbols]";
#if BIT64 
  Elf64_Shdr *shdr = NULL;
  Elf64_Sym *first_sym, *last_sym;
#else
  Elf32_Shdr *shdr = NULL;
  Elf32_Sym *first_sym, *last_sym;
#endif
  Elf_Data *elf_data = NULL;
  Elf_Scn *symtab_sect=0;
  Elf_Scn *sect=0;
  Elf_Scn *dynsym_sect=0;

  Elf *elf_h = get_native_exec_handler()->get_elf_handler();

  {
    self_trace_t::trace ( LEVELCHK(level2), 
			  MODULENAME, 0, 
			  "reading linkage symbol table for image[=%s]",
			  get_base_image_name().c_str());    
  }
 
  while ( (sect = elf_nextscn(elf_h, sect)) != NULL ) 
    {
#if BIT64 
      if ( (shdr = elf64_getshdr (sect) ) != NULL )
#else
      if ( (shdr = elf32_getshdr (sect) ) != NULL ) 
#endif
	{
	  if ( shdr->sh_type == SHT_SYMTAB ) 
	    {
	      symtab_sect = sect;
	      break;
	    }
	  else if ( shdr->sh_type == SHT_DYNSYM )
            {
	      dynsym_sect = sect;
            }
	}
    }

  if (symtab_sect == 0)
    symtab_sect = dynsym_sect;

  if (symtab_sect == 0) 
    {
      e = func 
          + " No symbol table section is found in "
          + get_base_image_name()
          + " ";
      throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);
    }

  if ( ( ( elf_data = elf_getdata ( symtab_sect, elf_data )) == NULL) 
       || elf_data->d_size == 0 ) 
    {
      e = func + " ELF data are not valid.";
      throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);
    }

#if BIT64 
  first_sym = (Elf64_Sym*) elf_data->d_buf;
  last_sym = (Elf64_Sym*) ( (char *) elf_data->d_buf + elf_data->d_size );
#else
  first_sym = (Elf32_Sym*) elf_data->d_buf;
  last_sym = (Elf32_Sym *) ( (char*)(elf_data->d_buf) + elf_data->d_size ); 
#endif

  while ( first_sym < last_sym ) 
    { 
      char* symname = elf_strptr(elf_h, 
				 shdr->sh_link, 
				 (size_t) first_sym->st_name); 

      if ( symname != 0 && (strcmp(symname, "") != 0) 
	   && (first_sym->st_value 
	       != 0)) 
	{
	  const string ssym(symname);

	  linkage_symbol_t<VA>* a_linksym 
	    =  new linkage_symbol_t<VA>( ssym,
		     get_base_image_name(),
		     (const VA) first_sym->st_value,
		     (const VA) SYMTAB_UNINIT_ADDR);

	  a_linksym->set_binding(decode_binding(first_sym->st_info));	  
	  a_linksym->set_visibility(decode_visibility(first_sym->st_other));
	  a_linksym->set_type(decode_type(first_sym->st_info));
          /* TODO: a tool says keystr is leaked, confirm if it really does */
	  string* keystr = new string(symname);

	  get_linkage_symtab().insert ( make_pair( (*keystr), 
						   (symbol_base_t<VA>*) a_linksym));
	}

      first_sym++;
    } 
  
  return SDBG_SYMTAB_OK;
}


//! PUBLIC: fetch_DSO_info
/*!

*/

template <LINUX_IMAGE_TEMPLATELIST>
symtab_error_e 
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::fetch_DSO_info
( std::string& where_is_interpreter, bool& found_interp )
  throw(symtab_exception_t)
{
  using namespace std;

  string e;
  string func = "[linux_image_t::fetch_DSO_info]";
#if BIT64 
  Elf64_Shdr *shdr = NULL;
#else
  Elf32_Shdr *shdr = NULL;
#endif
  Elf_Data *elf_data = NULL;
  Elf_Data *elf_data_dso = NULL;
  Elf_Data *interp_sect = NULL;
  char* strtab = NULL;
  char* sh_strtab = NULL;
  int strtab_size;
  int sh_strtab_size;
  Elf_Scn* sect;
#if BIT64 
  Elf64_Dyn *first_dso;
#else
  Elf32_Dyn *first_dso;
#endif
  int sz;

  sect = 0;
  Elf* elf_h = get_native_exec_handler()->get_elf_handler();
 
  while ( (sect = elf_nextscn(elf_h, sect)) != NULL ) 
    {  
#if BIT64 
      if ( (shdr = elf64_getshdr (sect) ) != NULL )
#else
      if ( (shdr = elf32_getshdr (sect) ) != NULL ) 
#endif
	{
	  if ( shdr->sh_type == SHT_STRTAB ) 
	    {
	      elf_data = 0;

	      if ( ( ( elf_data = elf_getdata ( sect, elf_data )) == NULL) 
		   || elf_data->d_size == 0 ) 
		{
		  e = func + " ELF data are not valid.";
		  throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);       
		}

	      if (shdr->sh_addr != 0 ) 
		{
		  // Section header string table's sh_addr is 0
		  // it seems the only mechanism where it differs from
		  // general string table section.	  	
		  strtab =  (char*) elf_data->d_buf;
		  strtab_size = elf_data->d_size;
		} 
	      else {
		if ( strcmp(".shstrtab", 
			    ((char*)elf_data->d_buf) + shdr->sh_name) == 0) 
		  {	    
		    sh_strtab =  (char*) elf_data->d_buf;
		    sh_strtab_size = elf_data->d_size;
		  }
	      }
	    }
	}
    } // while ( (sect = elf_nextscn(elf_h, sect)) != NULL ) 

  if (!sh_strtab) 
    {
      e = func + " No section header string table.";
      throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);  
    }
  
   while ( (sect = elf_nextscn(elf_h, sect)) != NULL ) 
     {
#if BIT64 
       if ( (shdr = elf64_getshdr (sect)) == NULL )
         continue;
#else
       if ( (shdr = elf32_getshdr (sect)) == NULL )
         continue;
#endif

       switch ( shdr->sh_type ) 
	 {
	 case SHT_PROGBITS:
           {
	     if ( strcmp(".interp", sh_strtab + shdr->sh_name) == 0 ) 
	       {
	         elf_data = 0;
	         if ( ( ( elf_data = elf_getdata ( sect, elf_data )) == NULL) 
		      || elf_data->d_size == 0 ) 
		   {
		     e = func + " ELF data are not valid.";
		     throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);       
		   }

	         interp_sect = elf_data;
	         where_is_interpreter = (char*) interp_sect->d_buf;

	         struct stat fchk_buf;	   

	         if (lstat(where_is_interpreter.c_str(), &fchk_buf) != 0) 
		   {
		     e = func + " interpreter path ("
		       + where_is_interpreter + ") is invalid.";
		     throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);  
		   }

	         if (!S_ISLNK(fchk_buf.st_mode) && S_ISREG(fchk_buf.st_mode)) 		 
		   found_interp = true;	     		
	         else if (S_ISLNK(fchk_buf.st_mode)) 
		   {	     
		     char realpath[PATH_MAX];
		     char tmpath[PATH_MAX];
		     int cnt;

		     strcpy(tmpath, where_is_interpreter.c_str());    	     
		   
		     while (S_ISLNK(fchk_buf.st_mode)) 
		       {	     
		         if ( (cnt = readlink(tmpath, realpath, PATH_MAX)) == -1 ) 
			   {
			     e = func + " readlink failed ";
			     throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);  
			   }
		       
		         realpath[cnt] = '\0';	       
		         if (realpath[0] == '/') 
			   strcpy(tmpath, realpath);
		         else
			   sprintf(tmpath, "%s/%s", dirname(tmpath), realpath); 

		         if (lstat(tmpath, &fchk_buf) != 0) 
			   {
			     e = func + " interpreter path (" + tmpath 
			       + ") is invalid.";
			     throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);  
			   }
		       } 
		   
		     if (S_ISREG(fchk_buf.st_mode)) {		     
		       where_is_interpreter = tmpath;
		       found_interp = true;

		       {
			   self_trace_t::trace ( LEVELCHK(level2), 
		             MODULENAME, 0, 
			     "interpreter found in .interp section [%s]",
			     where_is_interpreter.c_str());
		       }
		     } 
		   
		   } // else if (S_ISLNK(buf)) {
	       } // if ( strcmp(".interp", 	    
	     break;
	   }
	 default:
           //
           // Expand cases when you need to do more with other ELF sections
           //
	   break;
	 }

     } // while ( (sect = elf_nextscn(elf_h, sect)) != NULL ) {   

  if ( !strtab ) 
    {
      e = func + " No string table section is found. ";
      return SDBG_SYMTAB_FAILED;
    }  

  if ( !elf_data_dso ) 
    {
      e = func + " No DSO section is found. ";
      return SDBG_SYMTAB_FAILED;
    }
  if (!interp_sect) 
    {
      e = func + " No interpreter found. ";
      return SDBG_SYMTAB_FAILED;
    }
    
  return SDBG_SYMTAB_OK;
}


//! PROTECTED: linux_image_t<VA>::decode_type --
/*!   
    Determines the symbol type
*/
template <LINUX_IMAGE_TEMPLATELIST>
const std::string 
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::decode_type(int code) const
{
  using namespace std;

  string ret_string;

#if BIT64 
  switch(ELF64_ST_TYPE(code))
#else
  switch(ELF32_ST_TYPE(code))
#endif
    {

    case STT_NOTYPE:
      ret_string = "Symbol type is unspecified";
      break;

    case STT_OBJECT:
      ret_string = "Symbol is a data object";
      break;

    case STT_FUNC:
      ret_string = "Symbol is a code object";
      break;

    case STT_SECTION:
      ret_string = "Symbol associated with a section";
      break;

    case STT_FILE:
      ret_string = "Symbol's name is file name";
      break;

    case STT_COMMON:
      ret_string = "Symbol is a common data object";
      break;

    case STT_TLS:
      ret_string = "Symbol is thread-local data object";
      break;

    case STT_NUM:
      ret_string = "Number of defined types.";
      break;

    case STT_LOOS:
      ret_string = "Start of OS-specific";
      break;

    case STT_HIOS:
      ret_string = "End of OS-specific";
      break;

    case STT_LOPROC:
      ret_string = "Start of processor-specific";
      break;

    case STT_HIPROC:
      ret_string = "End of processor-specific";
      break;

    default:    
      ret_string = SYMTAB_UNINIT_STRING;
      break;
    }
  
  return ret_string;
}


//! PROTECTED: image_t<VA>::decode_binding --
/*!
    Determines the symbol binding information
*/
template <LINUX_IMAGE_TEMPLATELIST>
const std::string  
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::decode_binding(int code) const
{  
  using namespace std;

  string ret_string;
#if BIT64
  switch(ELF64_ST_BIND(code))
#else
  switch(ELF32_ST_BIND(code))
#endif
    {

    case STB_LOCAL:
      ret_string = "Local symbol";
      break;

    case STB_GLOBAL:
      ret_string = "Global symbol";
      break;

    case STB_WEAK:
      ret_string = "Weak symbol";
      break;

    case STB_NUM:
      ret_string = "Number of defined types";
      break;

    case STB_LOOS:
      ret_string = "Start of OS-specific";
      break;

    case STB_HIOS:
      ret_string = "End of OS-specific";
      break;

    case STB_LOPROC:
      ret_string = "Start of processor-specific";
      break;
      
    case STB_HIPROC:
      ret_string = "End of processor-specific ";
      break;

    default:
      ret_string = SYMTAB_UNINIT_STRING;
      break;
    }
  
  return ret_string; 
}


//! PROTECTED: image_t<VA>::decode_binding --
/*!
    Determines the symbol visibility information
*/
template <LINUX_IMAGE_TEMPLATELIST>
const std::string
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::decode_visibility (int code) const
{
  using namespace std;
 
  string ret_string;
#if BIT64 
  switch(ELF64_ST_VISIBILITY(code))
#else
  switch(ELF32_ST_VISIBILITY(code))
#endif
    {

    case STV_DEFAULT:
      ret_string = "Default symbol visibility rules";
      break;

    case STV_INTERNAL:
      ret_string = "Processor specific hidden class";
      break;
      
    case STV_HIDDEN:
      ret_string = "Sym unavailable in other modules";
      break;
      
    case STV_PROTECTED:
      ret_string = "Not preemptible, not exported";
      break;

    default:
      ret_string = SYMTAB_UNINIT_STRING;
      break;
    }

  return ret_string;
}


template <LINUX_IMAGE_TEMPLATELIST>
void
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::set_image_base_address(VA ba)
{
  image_base_t<VA,elf_wrapper>::set_image_base_address(ba); 
}


template <LINUX_IMAGE_TEMPLATELIST>
const VA&
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::get_image_base_address() const
{
  return (image_base_t<VA,elf_wrapper>::get_image_base_address());
}


template <LINUX_IMAGE_TEMPLATELIST>
void linux_image_t<LINUX_IMAGE_TEMPLPARAM>::set_base_image_name
(std::string& n)
{
  image_base_t<VA,elf_wrapper>::set_base_image_name(n);
}


template <LINUX_IMAGE_TEMPLATELIST>
const std::string& 
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::get_base_image_name() const
{
  return (image_base_t<VA,elf_wrapper>::get_base_image_name());
}


template <LINUX_IMAGE_TEMPLATELIST>
void linux_image_t<LINUX_IMAGE_TEMPLPARAM>::set_path
(std::string& n)
{
  image_base_t<VA,elf_wrapper>::set_path(n);
}


template <LINUX_IMAGE_TEMPLATELIST>
const std::string& 
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::get_path() const
{
  return (image_base_t<VA,elf_wrapper>::get_path());
}


template <LINUX_IMAGE_TEMPLATELIST>
void linux_image_t<LINUX_IMAGE_TEMPLPARAM>::set_native_exec_handler
(elf_wrapper* h)
{
  image_base_t<VA,elf_wrapper>::set_native_exec_handler(h);  
}


template <LINUX_IMAGE_TEMPLATELIST>
elf_wrapper* 
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::get_native_exec_handler()
{
  return (image_base_t<VA,elf_wrapper>::get_native_exec_handler());
}


template <LINUX_IMAGE_TEMPLATELIST>
std::map<std::string, symbol_base_t<VA>*, ltstr>& 
linux_image_t<LINUX_IMAGE_TEMPLPARAM>::get_linkage_symtab()
{
  return (image_base_t<VA,elf_wrapper>::linkage_symtab);
}

#endif // SDBG_LINUX_SYMTAB_IMPL_HXX





