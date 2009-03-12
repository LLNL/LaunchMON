dnl $Header: $
dnl
dnl configure.ac 
dnl
dnl --------------------------------------------------------------------------------
dnl Copyright (c) 2008, Lawrence Livermore National Security, LLC. Produced at
dnl the Lawrence Livermore National Laboratory. Written by Dong H. Ahn <ahn1@llnl.gov>.
dnl LLNL-CODE-409469. All rights reserved.
dnl 
dnl  This file is part of LaunchMON. For details, see
dnl  https://computing.llnl.gov/?set=resources&page=os_projects
dnl 
dnl  Please also read LICENSE -- Our Notice and GNU Lesser General Public License.
dnl 
dnl 
dnl  This program is free software; you can redistribute it and/or modify it under the
dnl  terms of the GNU General Public License (as published by the Free Software
dnl  Foundation) version 2.1 dated February 1999.
dnl 
dnl  This program is distributed in the hope that it will be useful, but WITHOUT ANY
dnl  WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
dnl  FITNESS FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU
dnl  General Public License for more details.
dnl 
dnl  You should have received a copy of the GNU Lesser General Public License along
dnl  with this program; if not, write to the Free Software Foundation, Inc., 59 Temple
dnl  Place, Suite 330, Boston, MA 02111-1307 USA
dnl --------------------------------------------------------------------------------
dnl 
dnl   Update Log:
dnl         Jun 06 2008 DHA: File created. 


AC_PREREQ(2.59)
AC_INIT(LaunchMON, 0.6.4, ahn1@llnl.gov)
AC_SUBST(LMON_CURRENT, 0)
AC_SUBST(LMON_REVISION, 0)
AC_SUBST(LMON_AGE, 0)
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE(launchmon, 0.6.4)
AC_CANONICAL_HOST


dnl -----------------------------------------------
dnl Checks for the OS and CPU type
dnl -----------------------------------------------
X_AC_OS_ISA


dnl -----------------------------------------------
dnl enable debug support
dnl -----------------------------------------------
X_AC_ENABLE_DEBUG


dnl -----------------------------------------------
dnl enable verbose support
dnl -----------------------------------------------
X_AC_ENABLE_VERBOSE


dnl -----------------------------------------------
dnl underlying comm. fab to use to bootstrap launchmon's 
dnl distributed components  
dnl -----------------------------------------------
X_AC_BOOTFABRIC


dnl -----------------------------------------------
dnl with GCRYPT support?
dnl -----------------------------------------------
X_AC_GCRYPT


dnl -----------------------------------------------
dnl enable tracing cost measurement codes? 
dnl -----------------------------------------------
X_AC_TRACING_COST


dnl -----------------------------------------------
dnl Checks for the Resource Manager type
dnl -----------------------------------------------
X_AC_RM

AC_MSG_CHECKING(whether a supported MPI job launcher has been found)  
if test -z "$ac_job_launcher_path" ; then
  AC_MSG_ERROR([No supported MPI job launcher has been found])
else
  AC_DEFINE_UNQUOTED(TARGET_JOB_LAUNCHER_PATH,["$ac_job_launcher_path"], [launcher path])
  AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
  AC_MSG_RESULT([$ac_job_launcher_bits bit $ac_job_launcher_path])
fi

dnl -----------------------------------------------
dnl Checks for NNODES and Ncores per node for test cases 
dnl -----------------------------------------------
X_AC_TESTNNODES
X_AC_NCORE_SMP

dnl -----------------------------------------------
dnl Checks for programs
dnl -----------------------------------------------
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_CC_C_O


dnl -----------------------------------------------
dnl Checks for the libraries
dnl -----------------------------------------------
AC_MSG_CHECKING(for libelf)
AC_CHECK_LIB(elf,elf_begin,libelf_found=yes,libelf_found=no)
if test "$libelf_found" = yes; then
     AC_SUBST(LIBELF,-lelf)    
else
     AC_MSG_ERROR([libelf is needed to build this package])
fi

AC_MSG_CHECKING(for pthread)
AC_CHECK_LIB(pthread,pthread_create,libpthread_found=yes,libpthread_found=no)
if test "$libpthread_found" = yes; then
     AC_SUBST(LIBPTHREAD,-lpthread)
else
     AC_MSG_ERROR([libpthread is needed to build this package])
fi
     
AC_MSG_CHECKING(for librt)
AC_CHECK_LIB(rt,clock_gettime,librt_found=yes,librt_found=no)
if test "$libpthread_found" = yes; then
     AC_SUBST(LIBRT,-lrt)
else
     AC_MSG_ERROR([librt is needed to build this package])
fi

X_AC_THREAD_DB
X_AC_MPICC


dnl -----------------------------------------------
dnl Checks for header files.
dnl -----------------------------------------------
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([sys/time.h unistd.h libelf.h libelf/libelf.h poll.h signal.h errno.h sys/user.h libgen.h assert.h link.h thread_db.h netdb.h arpa/inet.h sys/socket.h netinet/in.h limits.h sys/ptrace.h execinfo.h fcntl.h sys/procfs.h pthread.h sys/resource.h stdarg.h])

if test "$ac_cv_header_libelf_h" = "yes" ; then
     AC_DEFINE(LOCATION_OF_LIBELFHEADER, [<libelf.h>],
	[Define to header that first defines elf.])
elif test "$ac_cv_header_libelf_libelf_h" = "yes"; then
     AC_DEFINE(HAVE_LIBELF_H, 1, [Define 1 if we have libelf/libelf.h])
     AC_DEFINE(LOCATION_OF_LIBELFHEADER, [<libelf/libelf.h>],
	[Define to header that first defines elf.])
fi


dnl -----------------------------------------------
dnl Checks for typedefs, structures, and compiler characteristics.
dnl -----------------------------------------------
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_PID_T
AC_HEADER_TIME


dnl -----------------------------------------------
dnl Checks for library functions.
dnl -----------------------------------------------
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_HEADER_STDC
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gettimeofday memset strdup])


dnl -----------------------------------------------
dnl configure's OUTPUTS 
dnl -----------------------------------------------
AC_CONFIG_FILES([Makefile
                 tools/Makefile
		 tools/ciod/Makefile
                 tools/pmgr_collective/Makefile
	  	 tools/pmgr_collective/src/Makefile
		 tools/pmgr_collective/test/Makefile
                 launchmon/Makefile
		 launchmon/src/Makefile
		 launchmon/src/linux/Makefile
		 launchmon/src/linux/lmon_api/Makefile
                 launchmon/man/Makefile
		 test/Makefile
		 test/src/Makefile]) 
AC_OUTPUT
