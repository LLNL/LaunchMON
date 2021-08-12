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
#         May 06 2016 DHA: File created
#

AC_DEFUN([X_AC_SLURM_MPAO_WR], [
  AC_MSG_CHECKING([whether to turn on a workaround for slurm's MPIR_partitial_attach_ok bug])
  AC_ARG_ENABLE([slurm-mpao-workaround],
    AS_HELP_STRING(--enable-slurm-mpao-workaround, enables a fix that works around slurm's MPIR_partitial_attach_ok bug),
    [enable_mpao_workaround=$enableval],
    [enable_mpao_workaround=no])

  if test "x$enable_mpao_workaround" = "xyes"; then
    AC_DEFINE(LMON_SLURM_MPAO_WR, 1,
      [Enable a workaround to cope with additional SIGCONT sent by slurmstepd])
  fi
  AC_MSG_RESULT($enable_mpao_workaround)
])

AC_DEFUN([X_AC_SGI_HOSTNAMES], [
  AC_MSG_CHECKING([whether to enable use of SGI hostnames])
  AC_ARG_ENABLE([sgi-hostnames],
    AS_HELP_STRING(--enable-sgi-hostnames, enables the use of SGI hostnames as fallback if there are no matches in /etc/hosts for the hostname returned by mpiexec.hydra),
    [enable_sgi_hostnames=$enableval],
    [enable_sgi_hostnames=no])

  if test "x$enable_sgi_hostnames" = "xyes"; then
    AC_DEFINE(LMON_SGI_HOSTNAMES, 1,
      [Enable use of SGI hostnames as a fallback if no matches in /etc/hosts for the hostname returned by mpiexec.hydra])
  fi
  AC_MSG_RESULT($enable_sgi_hostnames)
])

AC_DEFUN([X_AC_SLURM_OVERLAP], [
  SRUN_OVERLAP=""
  AC_CHECK_PROG(SRUN, srun, "yes")
  if test "$SRUN" = "yes"; then
    AC_MSG_CHECKING([srun version >= 20.11 and supports --overlap])
    slurm_version=`srun --version | cut -f 2 -d " "`
    slurm_min=`printf '%s\n' "20.11" "$slurm_version" | sort -V | head -n 1`
    if test $slurm_min = "20.11" ; then
      AC_MSG_RESULT("yes")
      SRUN_OVERLAP=" --overlap "
    else
      AC_MSG_RESULT("no")
    fi
  fi
])
AC_SUBST(SRUN_OVERLAP)
