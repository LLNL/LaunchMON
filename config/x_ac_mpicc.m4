# $Header: $
#
# x_ac_mpi.m4
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
#         Jun 13 2008 DHA: File created. 
#
 
AC_DEFUN([X_AC_MPICC], [ 
  AC_ARG_VAR(MPICC, [MPI C compiler wrapper])
  AC_ARG_VAR(MPICXX, [MPI C++ compiler wrapper])
  AC_CHECK_PROGS(MPICC, mpigcc mpicc mpiicc mpxlc cc, $CC)
  AC_CHECK_PROGS(MPICXX, mpig++ mpiicpc mpxlC CC, $CXX)
  AC_SUBST(MPICC)
  AC_SUBST(MPICXX)
])

