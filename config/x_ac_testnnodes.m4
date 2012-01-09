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
#         Oct 22 2011 DHA: Added --with-test-rm support
#         Dec 15 2008 DHA: File created. (--with-testnnodes and --with-ncore-per-CN)
#

AC_DEFUN([X_AC_TESTNNODES], [
  AC_MSG_CHECKING([the number of compute nodes that standard test cases should use])
  AC_ARG_WITH([testnnodes],
    AS_HELP_STRING(--with-testnnodes@<:@=NNodes@:>@,specify the number of compute nodes test cases should use @<:@Blue Gene Note: use the number of IO nodes instead@:>@ @<:@default=2@:>@),
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
  AC_MSG_CHECKING([the number of cores per SMP node @<:@BGL: the number of compute nodes per IO node@:>@])
  AC_ARG_WITH([ncore-per-CN],
    AS_HELP_STRING(--with-ncore-per-CN@<:@=NCores@:>@,specify the core-count per compute node @<:@BGL Note: use the number of compute nodes per IO node instead@:>@ @<:@default=NCore of the configure host@:>@),
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


AC_DEFUN([X_AC_TEST_RM], [
  AC_MSG_CHECKING([resource manager to test @<:@slurm bglrm bgprm bgqrm alps orte@:>@])
  AC_ARG_WITH([test-rm],
    AS_HELP_STRING(--with-test-rm@<:@=RM@:>@,specify a resource manager type to test @<:@default=slurm on linux-x86 and linux-x86_64; alps on Cray; bgprm on linux-power; bgqrm on linux-power64@:>@),
    [with_rm=$withval],
    [with_rm="check"])

  rm_found="no"
  echo $with_rm
  #
  # Whether builder gave an RM and launcher path available??
  #
  dflt_str="$with_rm-$ac_target_os-$ac_target_isa"
  if test "x$with_rm" = "xslurm" -o "x$dflt_str" = "xcheck-linux-x86_64"; then
    #
    # Configure for SLURM
    #
    rm_default_dirs="/usr/bin /usr/local/bin"
    for rm_dir in $rm_default_dirs; do
      if test ! -z "$rm_dir" -a ! -d "$rm_dir" ; then
        continue;
      fi

      if test ! -z "$rm_dir/srun" -a -f "$rm_dir/srun"; then
	pth=`config/ap $rm_dir/srun`
        ac_job_launcher_path=$pth
        rm_found="yes"
        AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
        AC_SUBST(RM_TYPE, RC_slurm)
        break
      fi
    done

    #
    # This answers whether RM given and found
    #
    AC_MSG_RESULT($with_rm:$rm_found)

  elif test "x$with_rm" = "xorte" ; then
    echo "$with_rm"

  elif test "x$with_rm" = "xalps" ; then
    echo "$with_rm"

  elif test "x$with_rm" = "xbglrm" ; then
    #
    # Configure for Blue Gene P RM
    #
    rm_default_dirs="/bgl/BlueLight/ppcfloor/bglsys/bin"
    for rm_dir in $rm_default_dirs; do
      if test ! -z "$rm_dir" -a ! -d "$rm_dir" ; then
        continue;
      fi

      if test ! -z "$rm_dir/mpirun" -a -f "$rm_dir/mpirun"; then
        pth=`config/ap $rm_dir/mpirun`
        ac_job_launcher_path=$pth
        rm_found="yes"
        AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
        AC_SUBST(RM_TYPE, RC_bglrm)
        break
      fi
    done

  elif test "x$with_rm" = "xbgprm" -o "x$dflt_str" = "xcheck-linux-power"; then
    #
    # Configure for Blue Gene P RM
    #
    rm_default_dirs="/bgsys/drivers/ppcfloor/bin"
    for rm_dir in $rm_default_dirs; do
      if test ! -z "$rm_dir" -a ! -d "$rm_dir" ; then
        continue;
      fi

      if test ! -z "$rm_dir/mpirun" -a -f "$rm_dir/mpirun"; then
        pth=`config/ap $rm_dir/mpirun`
        ac_job_launcher_path=$pth
        rm_found="yes"
        AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
        AC_SUBST(RM_TYPE, RC_bgprm)
        break
      fi
    done

  elif test "x$with_rm" = "xbgqrm" -o "x$dflt_str" = "xcheck-linux-power64"; then
    #
    # Configure for Blue Gene Q RM
    #
    rm_default_dirs="/usr/bin /bgsys/drivers/ppcfloor/hlcs/bin"
    for rm_dir in $rm_default_dirs; do
      if test ! -z "$rm_dir" -a ! -d "$rm_dir" ; then
        continue;
      fi

      if test ! -z "$rm_dir/srun" -a -f "$rm_dir/srun"; then
        pth=`config/ap $rm_dir/srun`
        ac_job_launcher_path=$pth
        rm_found="yes"
        AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
        AC_SUBST(RM_TYPE, RC_bgq_slurm)
        break
      elif test ! -z "$rm_dir/runjob" -a -f "$rm_dir/runjob"; then
        pth=`config/ap $rm_dir/runjob`
        ac_job_launcher_path=$pth
        rm_found="yes"
        AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
        AC_SUBST(RM_TYPE, RC_bgqrm)
        break
      fi
    done

  fi
])
