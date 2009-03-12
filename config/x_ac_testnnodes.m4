# $Header: $
#
# x_ac_testnnodes.m4
#
# --------------------------------------------------------------------------------
# Copyright (c) 2008, Lawrence Livermore National Security, LLC. Produced at
# the Lawrence Livermore National Laboratory. Written by Dong H. Ahn <ahn1@llnl.gov>.
# LLNL-CODE-409469 All rights reserved.
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
#         Dec 15 2008 DHA: File created. 
#
 
AC_DEFUN([X_AC_TESTNNODES], [  
  AC_MSG_CHECKING([the number of compute nodes that standard test cases should use])
  AC_ARG_WITH([testnnodes],  
    AS_HELP_STRING(--with-testnnodes@<:@=NNodes@:>@,specify the number of compute nodes test cases should use @<:@default=2@:>@), 
    [with_tnn=$withval],  
    [with_tnn="check"])

  tnn_given="no"
  tnn=2

  if test "x$with_tnn" != "xno" -a "x$with_tnn" != "xcheck"; then
      tnn_given="yes"
      tnn=$with_tnn
  fi 

  AC_SUBST(TESTNNODES, $tnn)	
  AC_MSG_RESULT($tnn)
])

AC_DEFUN([X_AC_NCORE_SMP], [  
  AC_MSG_CHECKING([the number of cores per SMP node @<:@BGL: NCores per IO node@:>@])
  AC_ARG_WITH([ncore-per-CN],  
    AS_HELP_STRING(--with-ncore-per-CN@<:@=NCores@:>@,specify the core-count per compute node @<:@default=NCore of the configure system@:>@), 
    [with_smp=$withval],  
    [with_smp="check"])

  ncoresmp_given="no"
  ncoresmp=1

  if test "x$with_smp" != "xno" -a "x$with_smp" != "xcheck"; then
      ncoresmp_given="yes"
      ncoresmp=$with_smp
  else
      if test x$ac_target_os = "xlinux"; then
         ncoresmp=`cat /proc/cpuinfo | grep processor | wc -l`
      else
         AC_MSG_ERROR([how to get the ncores of this configure system not known])
      fi
  fi
 
  AC_SUBST(SMPFACTOR,$ncoresmp) 
  AC_MSG_RESULT($ncoresmp)
])

