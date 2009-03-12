# $Header: $
#
# x_ac_thread_db.m4 
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

AC_DEFUN([X_AC_THREAD_DB], [  
  AC_MSG_CHECKING(for libthread_db)
  if test "x$ac_target_os" != "xlinux"; then
    AC_MSG_ERROR([We do not support libthread_db on non Linux systems yet])
  else
    if test "x$ac_target_isa" = "xx86_64"; then 
      if test x"$ac_job_launcher_bits" = "x64"; then
        ac_libthread_db_loc="/lib$ac_job_launcher_bits/libthread_db.so.1" 
      else
        ac_libthread_db_loc="/lib/libthread_db.so.1"
      fi
    else 
      if test x"$ac_job_launcher_bits" = "x64"; then
        ac_libthread_db_loc="/lib$ac_job_launcher_bits/tls/libthread_db.so.1" 
      else
        ac_libthread_db_loc="/lib/tls/libthread_db.so.1"
      fi
    fi
  fi

  AC_CHECK_FILES($ac_libthread_db_loc,ac_libthread_db_found=yes,ac_libthread_db_found=no)
  if test "x$ac_libthread_db_found" = "xyes"; then
    AC_SUBST(LIBTHR_DB,$ac_libthread_db_loc)
  else
    AC_MSG_ERROR([libthread_db is not found in $ac_libthread_db_loc])
  fi
])

