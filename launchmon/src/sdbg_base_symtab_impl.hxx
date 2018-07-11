/*
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008, Lawrence Livermore National Security, LLC. Produced at
 * the Lawrence Livermore National Laboratory. Written by Dong H. Ahn
 * <ahn1@llnl.gov>. LLNL-CODE-409469. All rights reserved.
 *
 * This file is part of LaunchMON. For details, see
 * https://computing.llnl.gov/?set=resources&page=os_projects
 *
 * Please also read LICENSE.txt -- Our Notice and GNU Lesser General Public
 * License.
 *
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *--------------------------------------------------------------------------------
 *
 *  Update Log:
 *        Feb  09 2008 DHA: Added LLNS Copyright
 *        Jan  10 2006 DHA: Created file.
 */

#ifndef SDBG_BASE_SYMTAB_IMPL_HXX
#define SDBG_BASE_SYMTAB_IMPL_HXX 1

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

extern "C" {
#include <assert.h>
#include <libgen.h>
}

#include <limits.h>
#include <iostream>
#include <string>

#include "sdbg_base_symtab.hxx"

////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class symbol_base_t<>)
//
//

//!  symbol_base_t<> constructors
/*!
     constructors
*/
template <BASE_SYMTAB_TEMPLATELIST>
symbol_base_t<BASE_SYMTAB_TEMPLPARAM>::symbol_base_t() {
  name = SYMTAB_UNINIT_STRING;
  base_lib_name = SYMTAB_UNINIT_STRING;
  raw_address = SYMTAB_UNINIT_ADDR;
}

template <BASE_SYMTAB_TEMPLATELIST>
symbol_base_t<BASE_SYMTAB_TEMPLPARAM>::symbol_base_t(
    const symbol_base_t &sobj) {
  name = sobj.name;
  base_lib_name = sobj.base_lib_name;
  raw_address = sobj.raw_address;
  relocated_address = sobj.relocated_address;
}

template <BASE_SYMTAB_TEMPLATELIST>
symbol_base_t<BASE_SYMTAB_TEMPLPARAM>::symbol_base_t(const std::string &n,
                                                     const std::string &bln,
                                                     const VA rd,
                                                     const VA rla) {
  name = n;
  base_lib_name = bln;
  raw_address = rd;
  relocated_address = rla;
}

//!  symbol_base_t<> destructor
/*!
     destructor
*/
template <BASE_SYMTAB_TEMPLATELIST>
symbol_base_t<BASE_SYMTAB_TEMPLPARAM>::~symbol_base_t() {
  // when the class contains allocable members, those should be
  // deleted here
}

//!  symbol_base_t<> accessors
/*!
     accessors
*/
template <BASE_SYMTAB_TEMPLATELIST>
void symbol_base_t<BASE_SYMTAB_TEMPLPARAM>::set_name(const std::string &n) {
  name = n;
}

template <BASE_SYMTAB_TEMPLATELIST>
const std::string &symbol_base_t<BASE_SYMTAB_TEMPLPARAM>::get_name() const {
  return name;
}

template <BASE_SYMTAB_TEMPLATELIST>
void symbol_base_t<BASE_SYMTAB_TEMPLPARAM>::set_base_lib_name(
    const std::string &bln) {
  base_lib_name = bln;
}

template <BASE_SYMTAB_TEMPLATELIST>
const std::string &symbol_base_t<BASE_SYMTAB_TEMPLPARAM>::get_base_lib_name()
    const {
  return base_lib_name;
}

template <BASE_SYMTAB_TEMPLATELIST>
void symbol_base_t<BASE_SYMTAB_TEMPLPARAM>::set_raw_address(const VA &ra) {
  raw_address = ra;
}

template <BASE_SYMTAB_TEMPLATELIST>
const VA &symbol_base_t<BASE_SYMTAB_TEMPLPARAM>::get_raw_address() const {
  return raw_address;
}

template <BASE_SYMTAB_TEMPLATELIST>
void symbol_base_t<BASE_SYMTAB_TEMPLPARAM>::set_relocated_address(
    const VA &ra) {
  relocated_address = ra;
}

template <BASE_SYMTAB_TEMPLATELIST>
const VA &symbol_base_t<BASE_SYMTAB_TEMPLPARAM>::get_relocated_address() const {
  return relocated_address;
}

////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class image_base_t<>)
//
//

//! PUBLIC: image_base_t<> constructors
/*!
    constructors
*/
template <BASE_IMAGE_TEMPLATELIST>
image_base_t<BASE_IMAGE_TEMPLPARAM>::image_base_t()
    : image_base_address(SYMTAB_INIT_IMAGE_BASE), native_exec_handler(NULL) {
  path = SYMTAB_UNINIT_STRING;
  base_image_name = SYMTAB_UNINIT_STRING;
  MODULENAME = self_trace_t::self_trace().symtab_module_trace.module_name;
}

template <BASE_IMAGE_TEMPLATELIST>
image_base_t<BASE_IMAGE_TEMPLPARAM>::image_base_t(const std::string &lib)
    : image_base_address(SYMTAB_INIT_IMAGE_BASE), native_exec_handler(NULL) {
  char tempstr[PATH_MAX];

  sprintf(tempstr, "%s", lib.c_str());
  path = lib;
  //
  // some memory checkers complain about basename...
  //
  base_image_name = basename((char *)tempstr);
  MODULENAME = self_trace_t::self_trace().symtab_module_trace.module_name;
}

template <BASE_IMAGE_TEMPLATELIST>
image_base_t<BASE_IMAGE_TEMPLPARAM>::image_base_t(
    const image_base_t<BASE_IMAGE_TEMPLPARAM> &im) {
  base_image_name = im.base_image_name;
  path = im.base_path_name;
  //
  // We are not copying linkage_symtab and debug_symtab
  // because of our use of std::map.
  //
  image_base_address = im.image_base_address;
  native_exec_handler = im.native_exec_handler;

  MODULENAME = im.MODULENAME;
}

//! PUBLIC: image_base_t<> destructor
/*!
    destructor
*/
template <BASE_IMAGE_TEMPLATELIST>
image_base_t<BASE_IMAGE_TEMPLPARAM>::~image_base_t() {
  if (native_exec_handler) {
    native_exec_handler->finalize();
    delete native_exec_handler;
    native_exec_handler = NULL;
  }

  if (!(linkage_symtab.empty())) {
    typename std::map<std::string, symbol_base_t<VA> *, ltstr>::iterator iter;
    for (iter = linkage_symtab.begin(); iter != linkage_symtab.end(); ++iter) {
      delete iter->second;
      iter->second = NULL;
    }
    linkage_symtab.clear();
  }
  if (!(debug_symtab.empty())) {
    typename std::map<std::string, symbol_base_t<VA> *, ltstr>::iterator iter;
    for (iter = debug_symtab.begin(); iter != debug_symtab.end(); ++iter) {
      delete iter->second;
      iter->second = NULL;
    }
    debug_symtab.clear();
  }
}

//!  PUBLIC: image_base_t<> accessors
/*!
     accessors
*/
template <BASE_IMAGE_TEMPLATELIST>
void image_base_t<BASE_IMAGE_TEMPLPARAM>::set_image_base_address(VA ba) {
  image_base_address = ba;
}

template <BASE_IMAGE_TEMPLATELIST>
const VA &image_base_t<BASE_IMAGE_TEMPLPARAM>::get_image_base_address() const {
  return image_base_address;
}

template <BASE_IMAGE_TEMPLATELIST>
void image_base_t<BASE_IMAGE_TEMPLPARAM>::set_base_image_name(std::string &n) {
  base_image_name = n;
}

template <BASE_IMAGE_TEMPLATELIST>
const std::string &image_base_t<BASE_IMAGE_TEMPLPARAM>::get_base_image_name()
    const {
  return base_image_name;
}

template <BASE_IMAGE_TEMPLATELIST>
void image_base_t<BASE_IMAGE_TEMPLPARAM>::set_path(std::string &n) {
  path = n;
}

template <BASE_IMAGE_TEMPLATELIST>
const std::string &image_base_t<BASE_IMAGE_TEMPLPARAM>::get_path() const {
  return path;
}

template <BASE_IMAGE_TEMPLATELIST>
void image_base_t<BASE_IMAGE_TEMPLPARAM>::set_native_exec_handler(
    EXECHANDLER *h) {
  assert(h != NULL);
  native_exec_handler = h;
}

template <BASE_IMAGE_TEMPLATELIST>
EXECHANDLER *image_base_t<BASE_IMAGE_TEMPLPARAM>::get_native_exec_handler() {
  return native_exec_handler;
}

//! PUBLIC: image_base_t<VA>::get_a_symbol
/*!
    returns the matching linkage symbol object
 */
template <BASE_IMAGE_TEMPLATELIST>
const symbol_base_t<BASE_SYMTAB_TEMPLPARAM>
    &image_base_t<BASE_IMAGE_TEMPLPARAM>::get_a_symbol(
        const std::string &key) const {
  symbol_base_t<BASE_SYMTAB_TEMPLPARAM> *rsym = NULL;

  if (linkage_symtab.find(key.c_str()) != linkage_symtab.end()) {
    rsym = linkage_symtab.find(key.c_str())->second;
  } else {
    rsym = new symbol_base_t<BASE_SYMTAB_TEMPLPARAM>();
  }
  return (*rsym);
}

//! PUBLIC: compute_reloc
/*!
    computes the relocated address for each symbol
*/
template <BASE_IMAGE_TEMPLATELIST>
symtab_error_e image_base_t<BASE_IMAGE_TEMPLPARAM>::compute_reloc() throw(
    symtab_exception_t) {
  using namespace std;
  string e;
  string func = "[image_base_t::compute_reloc]";
  typename map<string, symbol_base_t<VA> *, ltstr>::iterator ir;

  if (image_base_address == SYMTAB_INIT_IMAGE_BASE) {
    e = func + " image base address has not been resolved.";
    throw symtab_exception_t(e, SDBG_SYMTAB_FAILED);
  }

  for (ir = linkage_symtab.begin(); ir != linkage_symtab.end(); ++ir) {
    ir->second->set_relocated_address(ir->second->get_raw_address() +
                                      image_base_address);
  }

  return SDBG_SYMTAB_OK;
}

//! PUBLIC: print_sorted_linkage_symtab --
/*!
    iterates over linkage symbol table and print them out; chiefly
    debug routine.
*/
template <BASE_IMAGE_TEMPLATELIST>
void image_base_t<BASE_IMAGE_TEMPLPARAM>::print_sorted_linkage_symtab() {
  using namespace std;

  typename map<std::string, symbol_base_t<VA> *, ltstr>::iterator ir;

  cout << "SDBG: size, " << linkage_symtab.size() << endl;

  for (ir = linkage_symtab.begin(); ir != linkage_symtab.end(); ++ir) {
    symbol_base_t<VA> &tobj = *(ir->second);
    printf("SDBG: %55s, 0x%8x, %40s, %40s, %s\n", ir->first.c_str(),
           ir->second->get_raw_address(), ir->second->get_binding().c_str(),
           ir->second->get_type().c_str(),
           ir->second->get_visibility().c_str());
  }
}

template <BASE_IMAGE_TEMPLATELIST>
symtab_error_e image_base_t<BASE_IMAGE_TEMPLPARAM>::init(
    const std::string &lib) throw(symtab_exception_t) {
  char tempstr[PATH_MAX];

  sprintf(tempstr, "%s", lib.c_str());
  path = lib;
  base_image_name = basename((char *)tempstr);
  //
  // some memory checkers complain about basename...
  //
  return (init());
}

#endif  // SDBG_BASE_SYMTAB_IMPL_HXX

/*
 * ts=2 sw=2 expandtab
 */
