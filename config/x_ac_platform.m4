# $Header: $
#
# x_ac_platform.m4
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

AC_DEFUN([X_AC_PLATFORM], [

  #                                                              #
  # Test OS                                                      #
  #                                                              #
  case "$host_os" in
    *linux*) AC_DEFINE(LINUX_CODE_REQUIRED,1,[Define 1 for LINUX_CODE_REQUIRED])
             ac_have_known_os="yes"
             ac_target_os="linux"
             #LMONENABSLOC=`config/ap $top_srcdir/launchmon/src/linux`
             LMONENABSLOC=`$srcdir/config/ap $srcdir/launchmon/src/linux`
             AC_SUBST(LMONENABSLOC)
             AC_SUBST(LMONENLOC,[$srcdir/launchmon/src/linux])
             AC_SUBST(LMONAPILOC,[$srcdir/launchmon/src/linux/lmon_api])
             ;;
    *aix*)   AC_DEFINE(AIX_CODE_REQUIRED,1,[Define 1 for AIX_CODE_REQUIRED])
             ac_have_known_os="yes"	
             ac_target_os="aix"
             ;;
    *)       AC_MSG_ERROR([Cannot build for this platform])
             ac_have_known_os="no"
             ;;
  esac

  #                                                              #
  # Test CPU                                                     #
  #                                                              #
  case "$host_cpu" in
    *x86_64*) AC_DEFINE(X86_64_ARCHITECTURE,1,[Define 1 for X86_64_ARCHITECTURE])
              ac_have_known_isa="yes"
              ac_target_isa="x86_64"
              AC_SUBST(LNCHR_BIT_FLAGS, -m64)
              AC_DEFINE(BIT64, 1, [64bit])
              ;;
    *i686*)   AC_DEFINE(X86_ARCHITECTURE,1,[Define 1 for X86_ARCHITECTURE])
              ac_have_known_isa="yes"
              ac_target_isa="x86"
              AC_SUBST(LNCHR_BIT_FLAGS, -m32)
              AC_DEFINE(BIT64, 1, [64bit])
              ;;
    *powerpc*)AC_DEFINE(PPC_ARCHITECTURE,1,[Define 1 for PPC_ARCHITECTURE])
              ac_have_known_isa="yes"
              ac_target_isa="power"
              AC_SUBST(LNCHR_BIT_FLAGS, -m64)
              AC_DEFINE(BIT64, 1, [64bit])
              ;;
    *)
              AC_MSG_ERROR([Cannot build for this platform])
              ac_have_known_isa="no"
              ;;
  esac

  ac_target_rm="generic"

  #                                                              #
  # Special Check is needed for Blue Gene Family and Cray Family #
  #                                                              #
  if test "x$ac_target_os" = "xlinux" -a "x$ac_target_isa" = "xpower"; then
    if test ! -z "/bgl/BlueLight/ppcfloor/bglsys/bin/mpirun" \
         -a -f "/bgl/BlueLight/ppcfloor/bglsys/bin/mpirun"; then
        ac_target_rm="bglrm"
        AC_DEFINE(SUB_ARCH_BGL,1,[Define 1 for SUB_ARCH_BGL])
        AC_SUBST(ARCHHEADER,"/bgl/BlueLight/ppcfloor/bglsys/include")
        AC_SUBST(ARCHLIB,"/bgl/BlueLight/ppcfloor/bglsys/lib64")
        AC_SUBST(CIODLOC, [tools/ciod])
    elif test ! -z "/bgsys/drivers/ppcfloor/bin/mpirun" \
         -a -f "/bgsys/drivers/ppcfloor/bin/mpirun"; then
        ac_target_rm="bgprm"
        AC_DEFINE(SUB_ARCH_BGP,1,[Define 1 for SUB_ARCH_BGP])
        AC_SUBST(ARCHHEADER,"/bgsys/drivers/ppcfloor/arch/include")
        AC_SUBST(ARCHLIB,"/bgsys/drivers/ppcfloor/lib64")
        AC_SUBST(CIODLOC, [tools/ciod])
    elif test ! -z "/bgsys/drivers/ppcfloor/hlcs/bin/runjob" \
         -a -f "/bgsys/drivers/ppcfloor/hlcs/bin/runjob"; then
        AC_DEFINE(SUB_ARCH_BGQ,1,[Define 1 for SUB_ARCH_BGQ])
        #
        # change this to the absolute path to bgsys
        #
        AC_SUBST(ARCHHEADER,"/bgsys/drivers/ppcfloor")
        AC_SUBST(ARCHLIB,"/bgsys/drivers/ppcfloor")
        AC_SUBST(CIODLOC, [tools/ciod])
        ac_target_rm="bgqrm"
    fi
  elif test "x$ac_target_os" = "xlinux" -a "x$ac_target_isa" = "xx86_64"; then
    if test ! -z "/use/bin/aprun" -a -f "/usr/bin/aprun"; then
        AC_DEFINE(SUB_ARCH_ALPS,1,[Define 1 for SUB_ARCH_ALPS])
        AC_DEFINE(RM_BE_STUB_CMD, "alps_be_starter", [be starter stub location])
        AC_DEFINE(RM_FE_COLOC_CMD, "alps_fe_colocator", [bulk launcher location])
	AC_SUBST(RMINC,"/usr/include/alps")
	AC_SUBST(RMLIB,"/usr/lib/alps/libalps.a")
        ac_target_rm="alps"
    else
        AC_SUBST(ARCHHEADER,"/")
        AC_SUBST(ARCHLIB,"/")
    fi
  else
        AC_SUBST(ARCHHEADER,"/")
        AC_SUBST(ARCHLIB,"/")
  fi

  AC_DEFINE_UNQUOTED(TARGET_OS_ISA_STRING, "$ac_target_os-$ac_target_isa", [Define os-isa string])
  AC_DEFINE_UNQUOTED(TARGET_RM_STRING, "$ac_target_rm" ,[Define rm string])
  AM_CONDITIONAL([WITH_ALPS], [test "x$ac_target_rm" = "alps"])
  AM_CONDITIONAL([WITH_CIOD], [test "x$ac_target_rm" = "xbglrm" \
                                    -o "x$ac_target_rm" = "xbgprm" \
                                    -o "x$ac_target_rm" = "xbgqrm"])
])

