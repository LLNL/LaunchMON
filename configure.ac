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
dnl         Aug 03 2010 DHA: Added X_AC_MW_HOSTLIST support
dnl         Dec 28 2009 DHA: Deprecated X_AC_PREFIX_CONFIG_H to better support
dnl                          binary distribution
dnl         Mar 20 2009 DHA: Increase LMON_CURRENT by one as we've added 
dnl                          a few more API calls. Similarly, increase
dnl                          LMON_AGE as this still supports 0th interface. 
dnl         Mar 11 2009 DHA: Added check for env, ssh|rsh, and totalview
dnl         Mar 06 2009 DHA: Deprecate X_AC_THREAD_DB,
dnl                          Added dlfcn.h presence check
dnl         Jun 06 2008 DHA: File created.
dnl

AC_PREREQ(2.59)
AC_INIT(LaunchMON, 1.0, ahn1@llnl.gov)

dnl
dnl This implements 2nd LMON interface (increase current when the 
dnl interface is changed, added, or removed.  
dnl 
AC_SUBST(LMON_CURRENT, 2)

dnl
dnl This is the 1st revision (increase revision when the 
dnl implementation has been changed but the interface stays the same) 
dnl 
AC_SUBST(LMON_REVISION, 0)

dnl
dnl Increase age when the interface has been added not removed.
dnl
AC_SUBST(LMON_AGE, 0)

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE(launchmon, 1.0)
AC_CANONICAL_HOST


dnl -----------------------------------------------
dnl Checks for the OS and CPU type
dnl -----------------------------------------------
X_AC_PLATFORM


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
dnl X_AC_RM

dnl AC_MSG_CHECKING(whether a supported MPI job launcher has been found)
dnl if test -z "$ac_job_launcher_path" ; then
dnl   AC_MSG_ERROR([No supported MPI job launcher has been found])
dnl else
dnl   AC_DEFINE_UNQUOTED(TARGET_JOB_LAUNCHER_PATH,["$ac_job_launcher_path"], [launcher path])
dnl   AC_SUBST(TARGET_JOB_LAUNCHER_PATH,$ac_job_launcher_path)
dnl   AC_MSG_RESULT([$ac_job_launcher_bits bit $ac_job_launcher_path])
dnl fi


dnl -----------------------------------------------
dnl Checks for NNODES and Ncores per node for test cases 
dnl -----------------------------------------------
X_AC_TESTNNODES
X_AC_NCORE_SMP
X_AC_TEST_RM
X_AC_MW_HOSTLIST


dnl -----------------------------------------------
dnl Checks for programs
dnl -----------------------------------------------
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_CC_C_O
AC_PATH_PROGS(envcmd,env,[/bin/env], /usr/local/bin:/usr/bin:/bin)
AC_PATH_PROGS(sshcmd,ssh rsh,[/usr/bin/ssh],/usr/local/bin:/usr/bin:/bin)
AC_PATH_PROGS(rshcmd,rsh ssh,[/usr/bin/rsh],/usr/local/bin:/usr/bin:/bin)
AC_PATH_PROGS(tvcmd,totalview,[totalview],/usr/local/bin:/usr/bin:/bin)
AC_DEFINE_UNQUOTED(ENVCMD,["$envcmd"],[env command found])
AC_DEFINE_UNQUOTED(SSHCMD,["$sshcmd"],[ssh or its equivalent found])
AC_DEFINE_UNQUOTED(RSHCMD,["$rshcmd"],[rsh or its equivalent found])
AC_DEFINE_UNQUOTED(TVCMD,["$tvcmd"],[totalview found])
AC_DEFINE_UNQUOTED(LMON_PREFIX, ["$prefix"], [install prefix])

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
if test "$librt_found" = yes; then
     AC_SUBST(LIBRT,-lrt)
else
     AC_MSG_ERROR([librt is needed to build this package])
fi

AC_MSG_CHECKING(for libdl)
AC_CHECK_LIB(dl,dlopen,libdl_found=yes,libdl_found=no)
if test "$libdl_found" = yes; then
     AC_SUBST(LIBDL,-ldl)
else
     AC_MSG_ERROR([libdl is needed to build this package])
fi

dnl DHA: decrecate thread db checking in favor of runtime checking
dnl X_AC_THREAD_DB
dnl
X_AC_MPICC


dnl -----------------------------------------------
dnl Checks for header files.
dnl -----------------------------------------------
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([stack vector map list iostream string algorithm fstream sstream cxxabi.h])
AC_LANG_POP([C++])
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([sys/time.h stdio.h stdint.h syscall.h unistd.h libelf.h libelf/libelf.h poll.h signal.h errno.h sys/user.h libgen.h assert.h link.h thread_db.h netdb.h arpa/inet.h sys/un.h sys/socket.h netinet/in.h limits.h sys/ptrace.h execinfo.h fcntl.h sys/procfs.h pthread.h sys/resource.h stdarg.h dlfcn.h stddef.h])

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
                 tools/cobo/Makefile
                 tools/cobo/src/Makefile
                 tools/cobo/test/Makefile
                 tools/alps/Makefile
                 tools/alps/src/Makefile
                 launchmon/Makefile
                 launchmon/src/Makefile
                 launchmon/src/linux/Makefile
                 launchmon/src/linux/lmon_api/Makefile
                 launchmon/man/Makefile
                 test/Makefile
                 test/src/Makefile])
AC_OUTPUT
