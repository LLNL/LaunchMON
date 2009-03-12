/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_base_bp.hxx,v 1.4.2.1 2008/02/20 17:37:56 dahn Exp $
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
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Jun 06 2006 DHA: Added DOXYGEN comments on the file scope 
 *                         and the class (breakpoint_base_t) 
 *        Jan 12 2006 DHA: Created file.          
 */ 

//! FILE: sdbg_base_bp.hxx
/*!
    The file contains the base class for breakpoint objects.
    The templated breakpoint_base_t gets instantiated by 
    a platform-specific layer in regards to the virtual address
    type and the instruction type. 

    Since most of the archtecture employ a trap instruction
    to implement the breakpoint, this class carries the trap 
    instruction itself around. 

    Note that one should only put platform independent logics
    into this file. 
*/

#ifndef SDBG_BASE_BP_HXX
#define SDBG_BASE_BP_HXX 1

//! FILE: breakpoint_base_t
/*!
    The platform-independent breakpoint class. A platform
    specific layer should provide this with a virtual 
    address type and an instruction type.

    A noteworthy feature is that this class attemps to blend
    the original instruction with the provided trap instruction
    to implement a breakpoint instruction. 
*/
template <class VA, class IT>
class breakpoint_base_t 
{

public:

  enum bp_status_e { 
    uninit,
    set_but_not_inserted,
    enabled, 
    disabled
  };

  bp_status_e status;
  
  breakpoint_base_t ()                     { status = uninit; }
  breakpoint_base_t ( const breakpoint_base_t<VA, IT>& b )
                                           {                 
					     trap_instruction = b.trap_instruction;
					     orig_instruction = b.orig_instruction;
					     status = uninit;
					   }
  virtual ~breakpoint_base_t ()            { }

  IT& get_trap_instruction ()              { return trap_instruction; }
  IT& get_orig_instruction ()              { return orig_instruction; }
  IT& get_blend_mask ()                    { return blend_mask; }
  VA& get_address_at()                     { return address_at; }
  VA& get_return_addr()                    { return return_addr; }
  virtual VA& get_where_pc_would_be()      { return address_at; }
  virtual bool is_pc_part_of_bp_op(VA pc)  { return ((pc==address_at)?true: false); }

  void set_trap_instruction ( const IT& i ){ trap_instruction = i; }
  void set_orig_instruction ( const IT& i ){ orig_instruction = i; }
  void set_blend_mask ( const IT& i)       { blend_mask = i; }
  void set_address_at ( const VA& addr )   { address_at = addr; }
  void set_return_addr (const VA& raddr)   { return_addr = raddr; }

private:

  VA address_at;
  VA return_addr;
  IT trap_instruction;
  IT orig_instruction;
  IT blend_mask;
  
};

#endif // SDBG_BASE_BP_HXX
