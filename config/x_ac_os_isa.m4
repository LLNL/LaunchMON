# $Header: $
#
# x_ac_os_isa.m4
#
# --------------------------------------------------------------------------------
# Copyright (c) 2008, Lawrence Livermore National Security, LLC. Produced at
# the Lawrence Livermore National Laboratory. Written by Dong H. Ahn <ahn1@llnl.gov>.
# LLNL-CODE-409469. All rights reserved.
# 
#  This file is part of LaunchMON. For details, see
#  https://computing.llnl.gov/?set=resources&page=os_projects
# 
#  Please also read LICENSE -- Our Notice and GNU Lesser General Public License.
# 
# 
#  This program is free software; you can redistribute it and/or modify it under the
#  terms of the GNU General Public License (as published by the Free Software
#  Foundation) version 2.1 dated February 1999.
# 
#  This program is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU
#  General Public License for more details.
# 
#  You should have received a copy of the GNU Lesser General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc., 59 Temple
#  Place, Suite 330, Boston, MA 02111-1307 USA
# --------------------------------------------------------------------------------
# 
#   Update Log:
#         Jun 11 2008 DHA: File created. 
#

AC_DEFUN([X_AC_OS_ISA], [  
  case "$host_os" in
    *linux*) AC_DEFINE(LINUX_CODE_REQUIRED,1,[Define 1 for LINUX_CODE_REQUIRED])
	     ac_have_known_os="yes"
	     ac_target_os="linux"
	     LMONENABSLOC=`config/ap launchmon/src/linux`
	     AC_SUBST(LMONENABSLOC)
             AC_SUBST(LMONENLOC,[launchmon/src/linux])
             AC_SUBST(LMONAPILOC,[launchmon/src/linux/lmon_api]) 
             ;;
    *aix*)   AC_DEFINE(AIX_CODE_REQUIRED,1,[Define 1 for AIX_CODE_REQUIRED])
	     ac_have_known_os="yes"	
	     ac_target_os="aix"
     	     ;;
    *)       AC_MSG_ERROR([Cannot build for this platform])
	     ac_have_known_os="no"
     	     ;;
  esac		

  case "$host_cpu" in
    *x86_64*) AC_DEFINE(X86_64_ARCHITECTURE,1,[Define 1 for X86_64_ARCHITECTURE])
	      ac_have_known_isa="yes"
	      ac_target_isa="x86_64"
     	      ;;
    *i686*)    AC_DEFINE(X86_ARCHITECTURE,1,[Define 1 for X86_ARCHITECTURE])
	      ac_have_known_isa="yes"
	      ac_target_isa="i686"
	      ;;
    *powerpc*)AC_DEFINE(PPC_ARCHITECTURE,1,[Define 1 for PPC_ARCHITECTURE])
	      ac_have_known_isa="yes"
	      ac_target_isa="ppc"
	      ;;
    *)    
     	      AC_MSG_ERROR([Cannot build for this platform])
	      ac_have_known_isa="no"
	      ;;
  esac
])

