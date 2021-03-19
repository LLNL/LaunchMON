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
  AC_ARG_WITH([test-nnodes],
    AS_HELP_STRING(--with-test-nnodes@<:@=NNodes@:>@,specify the number of compute nodes test cases should use @<:@Blue Gene Note: use the number of IO nodes instead@:>@ @<:@default=2@:>@),
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
  AC_MSG_CHECKING([the number of cores per SMP node @<:@IBM Blue Gene Note: the number of compute nodes per IO node@:>@])
  AC_ARG_WITH([test-ncore-per-CN],
    AS_HELP_STRING(--with-test-ncore-per-CN@<:@=NCores@:>@,specify the core-count per compute node @<:@IBM Blue Gene Note: use the number of compute nodes per IO node instead@:>@ @<:@default=NCore of the configure host@:>@),
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
  AC_MSG_CHECKING([resource manager to test @<:@slurm bgqrm alps orte mpiexec_hydra ibm_spectrum@:>@])
  AC_ARG_WITH([test-rm],
    AS_HELP_STRING(--with-test-rm@<:@=RM@:>@,specify a resource manager type to test @<:@slurm bgqrm alps orte mpiexec_hydra ibm_spectrum@:>@ @<:@default=slurm on linux-x86 and linux-x86_64; alps on Cray; bgqrm on linux-power64; ibm_spectrum on linux-power64le@:>@),
    [with_rm=$withval],
    [with_rm="check"])

  AC_ARG_WITH([test-rm-launcher],
    AS_HELP_STRING(--with-test-rm-launcher@<:@=LAUNCHERPATH@:>@,specify the RM launcher path @<:@default=/usr/bin/srun@:>@),
    [with_launcher=$withval],
    [with_launcher="check"])

  AC_ARG_WITH([test-rm-lib],
    AS_HELP_STRING(--with-test-rm-lib@<:@=RMLIBDIR@:>@,specify the directory containing RM libraries @<:@default=/usr/lib@:>@),
    [with_rmlib=$withval],
    [with_rmlib="check"])

  AC_ARG_WITH([test-rm-inc],
    AS_HELP_STRING(--with-rm-inc@<:@=RMINCDIR@:>@,specify the directory containing RM include files @<:@default=/usr/include@:>@),
    [with_rminc=$withval],
    [with_rminc="check"])


  rm_found="no"
  #
  # Whether builder gave an RM and launcher path available??
  #
  dflt_str="$with_rm-$ac_target_os-$ac_target_isa"
  if test "x$with_rm" = "xslurm" -o "x$dflt_str" = "xcheck-linux-x86_64"; then
    #
    # Configure for SLURM
    #
    if test "x$with_launcher" != "xcheck"; then
      #
      # launcher path given
      #
      if test ! -z "$with_launcher" -a -f "$with_launcher"; then
        pth=`$srcdir/config/ap $with_launcher`
        ac_job_launcher_path=$pth
        rm_found="yes"
        AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
        AC_SUBST(RM_TYPE, RC_slurm)
      fi
    else
      rm_default_dirs="/usr/bin /usr/local/bin"
      for rm_dir in $rm_default_dirs; do
        if test ! -z "$rm_dir" -a ! -d "$rm_dir" ; then
          continue;
        fi

        if test ! -z "$rm_dir/srun" -a -f "$rm_dir/srun"; then
	  pth=`$srcdir/config/ap $rm_dir/srun`
          ac_job_launcher_path=$pth
          rm_found="yes"
          AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
          AC_SUBST(RM_TYPE, RC_slurm)
          break
        fi
      done
    fi

    #
    # This answers whether RM given and found
    #
    AC_MSG_RESULT($with_rm:$rm_found)

  elif test "x$with_rm" = "xorte" ; then
    #
    # Configure for OPENMPI/OPENRTE
    #
    if test "x$with_launcher" != "xcheck"; then
      #
      # launcher path given
      #
      if test ! -z "$with_launcher" -a -f "$with_launcher"; then
        pth=`$srcdir/config/ap $with_launcher`
        ac_job_launcher_path=$pth
        rm_found="yes"
        AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
        AC_SUBST(RM_TYPE, RC_orte)
      fi
    else 
      rm_default_dirs="/usr/bin /usr/local/bin"
      for rm_dir in $rm_default_dirs; do
        if test ! -z "$rm_dir" -a ! -d "$rm_dir" ; then
          continue;
        fi

        if test ! -z "$rm_dir/orterun" -a -f "$rm_dir/orterun"; then
          pth=`$srcdir/config/ap $rm_dir/srun`
          ac_job_launcher_path=$pth
          rm_found="yes"
          AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
          AC_SUBST(RM_TYPE, RC_orte)
          break
        fi
      done
    fi

    #
    # This answers whether RM given and found
    #
    AC_MSG_RESULT($with_rm:$rm_found)

  elif test "x$with_rm" = "xibm_spectrum" ; then
    #
    # Configure for IBM Spectrum (jsrun)
    #
    if test "x$with_launcher" != "xcheck"; then
      #
      # launcher path given
      #
      if test ! -z "$with_launcher" -a -f "$with_launcher"; then
        pth=`$srcdir/config/ap $with_launcher`
        ac_job_launcher_path=$pth
        rm_found="yes"
        AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
        AC_SUBST(RM_TYPE, RC_ibm_spectrum)
      fi
    else
      rm_default_dirs="/opt/ibm/spectrum_mpi/jsm_pmix/bin/stock /usr/bin /usr/local/bin"
      for rm_dir in $rm_default_dirs; do
        if test ! -z "$rm_dir" -a ! -d "$rm_dir" ; then
          continue;
        fi

        if test ! -z "$rm_dir/jsrun" -a -f "$rm_dir/jsrun"; then
          pth=`$srcdir/config/ap $rm_dir/jsrun`
          ac_job_launcher_path=$pth
          rm_found="yes"
          AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
          AC_SUBST(RM_TYPE, RC_ibm_spectrum)
          break
        fi
      done
    fi

    #
    # This answers whether RM given and found
    #
    AC_MSG_RESULT($with_rm:$rm_found)

  elif test "x$with_rm" = "xalps" ; then
    #
    # Configure for Cray ALPS RM
    #
    if test "x$with_launcher" != "xcheck"; then
      #
      # launcher path given
      #
      if test ! -z "$with_launcher" -a -f "$with_launcher"; then
        pth=`$srcdir/config/ap $with_launcher`
        ac_job_launcher_path=$pth
        rm_found="yes"
        AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
        AC_SUBST(RM_TYPE, RC_alps)
      fi
    else
      rm_default_dirs="/usr/bin/orig /usr/bin"
      for rm_dir in $rm_default_dirs; do
        if test ! -z "$rm_dir" -a ! -d "$rm_dir" ; then
          continue;
        fi

        if test ! -z "$rm_dir/aprun" -a -f "$rm_dir/aprun"; then
          pth=`$srcdir/config/ap $rm_dir/aprun`
          ac_job_launcher_path=$pth
          rm_found="yes"
          AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
          AC_SUBST(RM_TYPE, RC_alps)
          break
        fi
      done
    fi

    #
    # This answers whether RM given and found
    #
    AC_MSG_RESULT($with_rm:$rm_found)

  elif test "x$with_rm" = "xbgqrm" -o "x$ac_target_rm" = "xbgqrm"; then
    #
    # Configure for Blue Gene Q RM
    #
    if test "x$with_launcher" != "xcheck"; then
      #
      # launcher path given
      #
      if test ! -z "$with_launcher" -a -f "$with_launcher"; then
        pth=`$srcdir/config/ap $with_launcher`
        ac_job_launcher_path=$pth
        rm_found="yes"
        AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
        #
        # TODO: compare the filename of with_launcher against srun/runjob
        #
        AC_SUBST(RM_TYPE, RC_bgqrm)
      fi
    else
      rm_default_dirs="/usr/bin /bgsys/drivers/ppcfloor/hlcs/bin"
      for rm_dir in $rm_default_dirs; do
        if test ! -z "$rm_dir" -a ! -d "$rm_dir" ; then
          continue;
        fi
        if test ! -z "$rm_dir/srun" -a -f "$rm_dir/srun"; then
          pth=`$srcdir/config/ap $rm_dir/srun`
          ac_job_launcher_path=$pth
          rm_found="yes"
          AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
          AC_SUBST(RM_TYPE, RC_bgq_slurm)
          break
        elif test ! -z "$rm_dir/runjob" -a -f "$rm_dir/runjob"; then
          pth=`$srcdir/config/ap $rm_dir/runjob`
          ac_job_launcher_path=$pth
          rm_found="yes"
          AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
          AC_SUBST(RM_TYPE, RC_bgqrm)
          break
        fi
      done
    fi

    #
    # This answers whether RM given and found
    #
    AC_MSG_RESULT($with_rm:$rm_found)

  elif test "x$with_rm" = "xmpiexec_hydra" ; then
    #
    # Configure for Intel MPI (mpiexec.hydra)
    #
    if test "x$with_launcher" != "xcheck"; then
      #
      # launcher path given
      #
      if test ! -z "$with_launcher" -a -f "$with_launcher"; then
        pth=`$srcdir/config/ap $with_launcher`
        ac_job_launcher_path=$pth
        rm_found="yes"
        AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
        AC_SUBST(RM_TYPE, RC_mpiexec_hydra)
      fi
    else
      rm_default_dirs="/usr/bin /usr/local/bin"
      for rm_dir in $rm_default_dirs; do
        if test ! -z "$rm_dir" -a ! -d "$rm_dir" ; then
          continue;
        fi

        if test ! -z "$rm_dir/orterun" -a -f "$rm_dir/mpiexec.hydra"; then
          pth=`$srcdir/config/ap $rm_dir/srun`
          ac_job_launcher_path=$pth
          rm_found="yes"
          AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
          AC_SUBST(RM_TYPE, RC_mpiexec_hydra)
          break
        fi
      done
    fi

    #
    # This answers whether RM given and found
    #
    AC_MSG_RESULT($with_rm:$rm_found)

  fi
])


AC_DEFUN([X_AC_MW_HOSTLIST], [
  AC_MSG_CHECKING([a set of hosts that middleware testing should use])
  AC_ARG_WITH([test-mw-hostlist],
    AS_HELP_STRING(--with-test-mw-hostlist@<:@=host1:host2@:>@,specify the list of hosts that middleware testing should use),
    [with_hostlist=$withval],
    [with_hostlist="check"])

  hostlist=""

  if test "x$with_hostlist" != "xno" -a "x$with_hostlist" != "xcheck"; then
     hostlist=$with_hostlist
  else
      hostlist=`hostname`
  fi

  #
  # Substituting @LMONHL@ to the hostlist 
  #
  AC_SUBST(LMONHL,$hostlist)
  AC_MSG_RESULT($hostlist)
])


AC_DEFUN([X_AC_TEST_INSTALLED], [
  AC_MSG_CHECKING([whether to configure tests on installed launchmon])
  AC_ARG_WITH([test-installed],
    AS_HELP_STRING(--with-test-installed,set to "yes" to configure tests on installed launchmon and set to "install-tests" to install test binaries and scripts. "install-tests" implies "yes."),
    [with_test_inst=$withval],
    [with_test_inst=no])

  AC_MSG_RESULT($with_test_inst)
  AM_CONDITIONAL([WITH_TEST_INSTALLED], [test "x$with_test_inst" != "xno"])
  AM_CONDITIONAL([WITH_INSTALL_TESTS], [test "x$with_test_inst" = "xinstall-tests"])
])

