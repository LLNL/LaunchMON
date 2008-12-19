# $Header: $
#
# x_ac_rm.m4
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
#         Dec 11 2008 DHA: File created. 
#

AC_DEFUN([X_AC_RM], [

  AC_MSG_CHECKING([resource manager type])

  AC_ARG_WITH([rm], 
    AS_HELP_STRING(--with-rm@<:@=RMTYPE@:>@,specify a system Resource Manager @<:@slurm, bglrm@:>@ @<:@default=slurm@:>@),
    [with_rm_name=$withval],
    [with_rm_name="check"]
  )

  AC_ARG_WITH([rm-launcher], 
    AS_HELP_STRING(--with-rm-launcher@<:@=LAUNCHERPATH@:>@,specify the RM launcher path @<:@default=/usr/bin/srun@:>@), 
    [with_launcher=$withval],
    [with_launcher="check"]
  )

  rm_default_dirs="/usr/bin /usr/local/bin /bgl/BlueLight/ppcfloor/bglsys/bin" 
  rm_found="no"
 
  #
  # Whether builder gave an RM and launcher path available??
  #
  if test "x$with_rm_name" = "xslurm" -o "x$with_rm_name" = "xyes"; then
    #
    # Configure for SLURM
    #
    with_rm_name="slurm"
    if test "x$with_launcher" != "xcheck" -a "x$with_launcher" != "xyes"; then
      #
      # The SRUN path is given
      #
      pth=""
      if test -f $with_launcher; then
        pth=`config/ap $with_launcher`
        if test -f $pth ; then
          ac_job_launcher_path=$pth
	  rm_found="yes"
        fi
      fi
    else
      #
      # Try the default SRUN path
      #
      for rm_dir in $rm_default_dirs; do  
        if test ! -z "$rm_dir" -a ! -d "$rm_dir" ; then
          continue;
        fi

        if test ! -z "$rm_dir/srun" -a -f "$rm_dir/srun"; then     
	  pth=`config/ap $rm_dir/srun`     
          ac_job_launcher_path=$pth               
          rm_found="yes"
          break
        fi
      done
    fi
    
    #
    # This answers whether RM given and found
    #
    AC_MSG_RESULT($with_rm_name:$rm_found)	
  
    if test "x$rm_found" = "xyes"; then  
      #
      # SLURM found, export some configuration params 
      #
      AC_DEFINE(RM_SLURM_SRUN,1,[Define 1 for RM_SLURM_SRUN])
      AC_SUBST(RM_TYPE, slurm)
      AC_MSG_CHECKING([whether srun is 32 bit or 64 bit])
      #check if job launcher is 32 bit or 64 bit.
      #TODO: the use of file | gawk isn't portable
      bits=`file $ac_job_launcher_path | gawk '{print @S|@3}'` 
      if test "$bits" = "32-bit"; then
	#
	# 32bit srun process
	#
        ac_job_launcher_bits="32"
	#
        #TODO: I need another m4 to check where the tls libraries are picked up
	#
        AC_DEFINE(NPTL_UNDER_TLS, 1, [Define 1 if TLS libraries are installed under /lib/tls]) 
	AC_SUBST(LNCHR_BIT_FLAGS, -m32)
	AC_DEFINE(GLUESYM,[""], [Define an empty string for GLUESYM])
      elif test "$bits" = "64-bit"; then
	#
	# 64bit srun process
	#
        ac_job_launcher_bits="64"
	AC_SUBST(LNCHR_BIT_FLAGS, -m64)
        AC_DEFINE(BIT64,1,[Define 1 for BIT64])
	AC_DEFINE(GLUESYM,[""], [Define an empty string for GLUESYM])
        if test "x$ac_have_known_os" = "xyes"; then
          if test "$ac_target_os" = "linux" && test "$ac_target_isa" = "x86_64"; then
	    AC_SUBST(LIBBITSUFFIX,64)
          fi
        fi 
      else
        ac_job_launcher_bits="unknown"
      fi # bit check
      AC_MSG_RESULT($ac_job_launcher_bits)
    fi  

  elif test "x$with_rm_name" = "xbglrm" ; then
    #
    # Configure for BGL/MPIRUN
    #
    if test "x$with_launcher" != "xcheck" -a "x$with_launcher" != "xyes"; then
      #
      # The MPIRUN path is given
      #
      pth=""
      if test -f $with_launcher; then
        pth=`config/ap $with_launcher`
        if test -f $pth ; then
          ac_job_launcher_path=$pth
          rm_found="yes"
        fi
      fi
    else
      #
      # Try the default MPIRUN path
      #
      for rm_dir in $rm_default_dirs; do  
        if test ! -z "$rm_dir" -a ! -d "$rm_dir" ; then
          continue;
        fi

        if test ! -z "$rm_dir/mpirun" -a -f "$rm_dir/mpirun"; then     
	  pth=`config/ap $rm_dir/mpirun`     
          ac_job_launcher_path=$pth
          rm_found="yes"
          break
        fi
      done
    fi

    AC_MSG_RESULT($with_rm_name:$rm_found)

    if test "x$rm_found" = "xyes"; then  
 
      AC_SUBST(CIODLOC,[tools/ciod])
      AC_SUBST(LIBCIOD, [-lciod_db])

      #
      #TODO: I need another m4 to check where the tls libraries are picked up
      #
      AC_DEFINE(NPTL_UNDER_TLS, 1, [Define 1 if TLS libraries are installed under /lib/tls]) 
      #
      #TODO: in this case, CN-IO ratio
      #
      SMPFACTOR=8
      AC_SUBST(SMPFACTOR)
      AC_DEFINE(RM_BGL_MPIRUN,1,[Define 1 for RM_BGL_MPIRUN])
      AC_SUBST(RM_TYPE, bglrm) 
      AC_MSG_CHECKING([whether mpirun is 32 bit 64 bit]) 
      #check if job launcher is 32 bit or 64 bit.
      #TODO: the use of file | gawk isn't portable
      bits=`file $ac_job_launcher_path | gawk '{print @S|@3}'`
      if test $bits = "32-bit"; then
        ac_job_launcher_bits="32"
	AC_SUBST(LNCHR_BIT_FLAGS, -m32)
	AC_DEFINE(GLUESYM,[""], [Define dot for GLUESYM])
      elif test $bits = "64-bit"; then
        ac_job_launcher_bits="64"
	AC_SUBST(LNCHR_BIT_FLAGS, -m64)
        AC_DEFINE(BIT64,1,[Define 1 for BIT64])
	AC_DEFINE(GLUESYM,["."], [Define dot for GLUESYM])
      else
        ac_job_launcher_bits="unknown"
      fi # test $bits = "32-bit"
      AC_MSG_RESULT($ac_job_launcher_bits)
    fi # test "x$rm_found" = "xyes"; 	
  else
    AC_MSG_ERROR([--with-rm is a required option])
  fi
  AM_CONDITIONAL([WITH_CIOD], [test "xyes" = "xyes"])
])

