# $Header: $
#
# x_ac_gcrypt.m4
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
AC_DEFUN([X_AC_GCRYPT], [  

  AC_MSG_CHECKING([whether to configure and build gcrypt])

  AC_ARG_WITH([gcrypt],  
    AS_HELP_STRING(--with-gcrypt@<:@=ARG@:>@,configure and build gcrypt @<:@default=yes@:>@), 
    [with_gcrypt=$withval],  
    [with_gcrypt="check"]
  )

  gcrypt_configured="no"
  if test "x$with_gcrypt" != "xno"; then 
    if test -d tools/libgpg-error -a -d tools/libgcrypt; then
      #
      # DHA
      # Configure both libgpg-error 
      # We can't configure libgcrypt via this method because
      # we need to pass a new config paramer to its configure
      # script and the AC_CONFIG_SUBDIRS macro does not support it.
      # I addressed this issue by inserting a dispatcher Makefile.am
      # to tools, in which configures are explictly issued. 
      #	
      AC_CONFIG_SUBDIRS([tools/libgpg-error])
      AC_SUBST(GPGERRLOC, [tools/libgpg-error/src])
      AC_SUBST(GCRYPTLOC, [tools/libgcrypt/src])
      AC_SUBST(LIBGCRYPT, [-lgcrypt])
      AC_SUBST(LIBGPGERR, [-lgpg-error])
      gcrypt_configured="yes"
    else
      AC_MSG_ERROR([tools/libgpg-error or tools/libgcrypt not found])
    fi
  else
      AC_MSG_ERROR([--with-gcrypt is now mandatory])
  fi # with_gcrypt
  AM_CONDITIONAL([WITH_GCRYPT], [test "x$gcrypt_configured" = "xyes"])
  AC_MSG_RESULT($gcrypt_configured)
])

