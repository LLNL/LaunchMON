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
 *        Mar 06 2008 DHA: Added indirect breakpoint support
 *        Mar 11 2008 DHA: Added Linux PowerPC support 
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Jan 09 2007 DHA: Linux X86/64 Port
 *        Feb 07 2006 DHA: Created file.
 */ 

#ifndef SDBG_LINUX_BP_HXX
#define SDBG_LINUX_BP_HXX 1

#include "sdbg_base_bp.hxx"
#include "sdbg_linux_mach.hxx"

#if BIT64
# if X86_64_ARCHITECTURE
  const int NUM_BYTE_INCR_AFTER_SINGLESTEP = 8;
# else
  const int NUM_BYTE_INCR_AFTER_SINGLESTEP = 4;
# endif //X86_64_ARCHITECTURE
#else
const int NUM_BYTE_INCR_AFTER_SINGLESTEP = 4;
#endif

#if X86_ARCHITECTURE || X86_64_ARCHITECTURE
const int NUM_BYTE_INCR_AFTER_TRAP       = 1;
#else
const int NUM_BYTE_INCR_AFTER_TRAP       = 0;
#endif

//! linux_breakpoint_t:
/*!
    linux_breakpoint_t is linux implementation of breakpoint_base_t
    class.
*/
class linux_breakpoint_t : public breakpoint_base_t<T_VA, T_IT>
{

public:

  linux_breakpoint_t()
    { 
      unset_use_indirection();
      set_trap_instruction(T_TRAP_INSTRUCTION); 
      set_orig_instruction(T_UNINIT_HEX);
      set_address_at(T_UNINIT_HEX);
      set_indirect_address_at(T_UNINIT_HEX);
      set_blend_mask(T_BLEND_MASK);
      set_return_addr(T_UNINIT_HEX);
    }

  virtual ~linux_breakpoint_t() { }

  virtual T_VA& get_where_pc_would_be()
    {
      where_pc_would_be 
        = get_use_indirection()
	  ? get_indirect_address_at()
	  : get_address_at();

      where_pc_would_be += NUM_BYTE_INCR_AFTER_TRAP;

      return ( where_pc_would_be );
    }

  virtual bool is_pc_part_of_bp_op(T_VA pc) 
    { 
      bool rval = false;
      T_VA taddr
        = get_use_indirection()
	  ? get_indirect_address_at()
	  : get_address_at();

      if ( (pc == get_where_pc_would_be())
	   || ((pc > get_where_pc_would_be())
	      && (pc <= taddr+NUM_BYTE_INCR_AFTER_SINGLESTEP)))
	{
	  rval = true;
	}
      else
	{
	  // check if PC happens to match with return_addr which is
	  // gathered whenever the breakpoint is hit during the BP
	  // prelogue operation.
	  rval = (pc == get_return_addr())? true : false;
	}

      return rval;
    }

private:

  T_VA where_pc_would_be;

};

#endif // SDBG_LINUX_BP_HXX
