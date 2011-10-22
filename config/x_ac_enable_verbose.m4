# $Header: $
#
# x_ac_enable_verbose.m4
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
#         Dec 12 2008 DHA: Remove X_AC_WITH_VER_LOGDIR and push the 
#                          logdir support to --enable-verbose as an option
#
#         Sep 19 2008 DHA: File created.
#

AC_DEFUN([X_AC_ENABLE_VERBOSE], [
  AC_MSG_CHECKING([whether to enable verbose codes])
  AC_ARG_ENABLE([verbose],
    AS_HELP_STRING(--enable-verbose@<:@=LOGDIR@:>@,enable verbose codes @<:@default=stdout@:>@),
    [verbose_enabled=$enableval],
    [verbose_enabled="check"]
  )
  if test "x$verbose_enabled" != "xno" -a "x$verbose_enabled" != "xcheck"; then
    AC_DEFINE(VERBOSE,1,[Define VERBOSE flag])
    AC_MSG_RESULT([yes])
    AC_MSG_CHECKING([whether to dump verbose files into a logging directory])
    if test "x$verbose_enabled" != "xyes"; then
      # logdir is given in this case
      AC_DEFINE(USE_VERBOSE_LOGDIR,1,[Define USE_VERBOSE_LOGDIR flag])
      AC_DEFINE_UNQUOTED(VERBOSE_LOGDIR,["$verbose_enabled"], [Define LOGDIR])
      AC_MSG_RESULT([yes:$verbose_enabled])	
    else
      AC_MSG_RESULT([no])
    fi 
  else
    AC_MSG_RESULT([no])
  fi
])
