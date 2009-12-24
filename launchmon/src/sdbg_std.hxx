/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_std.hxx,v 1.4.2.1 2008/02/20 17:37:57 dahn Exp $
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
 *        Jun 01 2009 DHA: Upped GracePeriodBNSignals by x10
 *                         to deal with unreliable signals
 *        Feb 09 2008 DHA: Added LLNS Copyright 
 *        Mar 30 2006 DHA: Added exception handling support
 *        Mar 16 2006 DHA: Created file. 
 *
 */ 

#ifndef SDBG_STD_HXX
#define SDBG_STD_HXX 1

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif
#include <lmon_api/common.h>

#define CONCATENATE(A,B) A##B

#define DEFINE_GET_METHOD(TYPENAME,MEMBER)       \
  TYPENAME CONCATENATE(get_,MEMBER) ()  { return (MEMBER); }

#define DEFINE_SET_METHOD(TYPENAME,MEMBER)       \
  void CONCATENATE(set_,MEMBER) (TYPENAME param) { MEMBER = param; }

#define define_gset(TYPENAME,MEMBER)             \
  DEFINE_GET_METHOD(TYPENAME,MEMBER)             \
  DEFINE_SET_METHOD(TYPENAME,MEMBER) 

#define SDBG_DEFAULT_TEMPLATE_WIDTH typename VA, \
                                    typename WT, \
                                    typename IT, \
                                    typename GRS,\
                                    typename FRS,\
                                    typename NT, \
                                    typename EXECHANDLER

#define SDBG_DEFAULT_TEMPLPARAM VA,WT,IT,GRS,FRS,NT,EXECHANDLER

const unsigned int GracePeriodBNSignals       = 100000; // 100 millisecs
const unsigned int GracePeriodFEDisconnection = 2000000; // 2 secs
const double DefaultWarmPeriods               = 10.0;
#endif // SDBG_STD_HXX
