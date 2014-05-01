# $Header: $
#
# x_ac_myboost.m4
#
# --------------------------------------------------------------------------------
# Copyright (c) 2012, Lawrence Livermore National Security, LLC. Produced at
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
#         Jun 08 2012 DHA: File created.
#
AC_DEFUN([X_AC_MYBOOST], [

  AC_MSG_CHECKING([whether to use my boost])

  AC_ARG_WITH([myboost],
    AS_HELP_STRING(--with-myboost@<:@=ARG@:>@,use my boost@<:@default=yes@:>@), 
    [with_myboost=$withval],
    [with_myboost="check"]
  )

  myboost_configured="no"
  if test "x$with_myboost" == "xyes"; then 
    if test -d tools/myboost; then
      AC_SUBST(MYBOOSTLOC, [tools/myboost])
      myboost_configured="yes"
    else
      AC_MSG_ERROR([tools/myboost not found])
    fi
  else
      AC_SUBST(MYBOOSTLOC, [./])
  fi # with_myboost

  AC_MSG_RESULT($myboost_configured)
])

