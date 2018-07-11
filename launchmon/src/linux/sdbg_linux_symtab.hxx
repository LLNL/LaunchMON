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
 *
 *  Update Log:
 *        Oct 27 2010 DHA: Added is_defined, is_globally_visible,
 *                         is_locally_visible virtual methods.
 *        Oct 26 2010 DHA: Added support for symbol visibility
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Mar 30 2006 DHA: Added exception handling support
 *        Mar 16 2006 DHA: The file is created so that
 *                         I can extract out linux specific
 *                         implementation from base symbol table
 *                         class. Linux only supports ELF file
 *                         format and DWARF debug info format.
 *
 */

#ifndef SDBG_LINUX_SYMTAB_HXX
#define SDBG_LINUX_SYMTAB_HXX 1

#include "sdbg_base_symtab.hxx"
#include "sdbg_std.hxx"

extern "C" {
#if HAVE_LIBELF_H
#include LOCATION_OF_LIBELFHEADER
#else
#error libelf.h is required
#endif
}

#define LINUX_SYMTAB_TEMPLATELIST typename VA
#define LINUX_SYMTAB_TEMPLPARAM VA
#define LINUX_IMAGE_TEMPLATELIST typename VA
#define LINUX_IMAGE_TEMPLPARAM VA

enum LMON_ELF_visibility {
  elf_sym_local,
  elf_sym_global,
  elf_sym_weak,
  elf_sym_procspecific,
  elf_sym_none
};

//! class linkage_symbol_t<>
/*!

*/
template <LINUX_SYMTAB_TEMPLATELIST>
class linkage_symbol_t : public symbol_base_t<LINUX_SYMTAB_TEMPLPARAM> {
 public:
  //
  // constructosr & destructor
  //
  linkage_symbol_t();
  linkage_symbol_t(const linkage_symbol_t& l);
  linkage_symbol_t(const std::string& n, const std::string& ln, const VA ra,
                   const VA rla);
  virtual ~linkage_symbol_t() {}

  //
  // accessors
  //
  void set_name(const std::string& n);
  void set_base_lib_name(const std::string& bln);
  void set_binding(const std::string& b);
  void set_visibility(const std::string& v);
  void set_type(const std::string& t);
  const std::string& get_name() const;
  const std::string& get_base_lib_name() const;
  const std::string get_binding() const;
  const std::string get_visibility() const;
  const std::string get_type() const;
  const VA& get_raw_address() const;
  const VA& get_relocated_address() const;

  virtual bool is_defined() const;
  virtual bool is_globally_visible() const;
  virtual bool is_locally_visible() const;

  define_gset(bool, defined) define_gset(LMON_ELF_visibility, vis)

      //
      // debug methods
      //
      void print_me() const;

 private:
  bool defined;
  LMON_ELF_visibility vis;
  std::string visibility;
  std::string binding;
  std::string type;
};

//!
/*! elf_wrapper

*/
class elf_wrapper {
 public:
  //
  // constructors & destructor
  //
  elf_wrapper();
  elf_wrapper(const std::string& ex);
  elf_wrapper(const elf_wrapper& elfw);
  ~elf_wrapper();

  //
  // accessors
  //
  void set_exec(const std::string& e) { exec = e; }
  const std::string& get_exec() const { return exec; }
  void set_elf_handler(Elf* e) { elf_handler = e; }
  Elf* get_elf_handler() { return elf_handler; }

  symtab_error_e init(std::string& lib);
  symtab_error_e init() throw(symtab_exception_t);
  symtab_error_e finalize();

 private:
  std::string exec;
  int fd;
  Elf* elf_handler;
};

//!
/*! linux_image_t<>

*/
template <LINUX_IMAGE_TEMPLATELIST>
class linux_image_t : public image_base_t<VA, elf_wrapper> {
 public:
  //
  // constructors & destructor
  //
  linux_image_t();
  linux_image_t(const std::string& lib);
  linux_image_t(const image_base_t<VA, elf_wrapper>& im);
  virtual ~linux_image_t();

  //
  // accessors: had to overload them because of
  //  "there are no arguments to `method' that
  // depend on a template parameter, so a declaration of
  // `method' must be available G++ warning
  void set_base_image_name(std::string& n);
  void set_path(std::string& n);
  void set_image_base_address(VA ba);
  void set_native_exec_handler(elf_wrapper* h);

  const std::string& get_base_image_name() const;
  const std::string& get_path() const;
  const VA& get_image_base_address() const;
  elf_wrapper* get_native_exec_handler();
  std::map<std::string, symbol_base_t<VA>*, ltstr>& get_linkage_symtab();

  virtual symtab_error_e init() throw(symtab_exception_t);
  virtual symtab_error_e read_linkage_symbols() throw(symtab_exception_t);
  virtual symtab_error_e fetch_DSO_info(std::string&,
                                        bool&) throw(symtab_exception_t);
  // virtual symtab_error_e read_debug_symbols() = 0;
  //   throw ( symtab_exception_t ) = 0;

  //
  // Some Util methods.
  //
  virtual LMON_ELF_visibility resolve_binding(int code) const;
  virtual void decode_binding(int code, std::string& s) const;
  virtual void decode_visibility(int code, std::string& s) const;
  virtual void decode_type(int code, std::string& s) const;

 private:
  bool LEVELCHK(self_trace_verbosity level) {
    return (self_trace_t::self_trace().symtab_module_trace.verbosity_level >=
            level);
  }

  // For self tracing
  //
  std::string MODULENAME;
};

#endif  // SDBG_LINUX_SYMTAB_HXX

/*
 * ts=2 sw=2 expandtab
 */
