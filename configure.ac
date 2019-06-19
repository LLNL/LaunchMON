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
dnl         Apr 28 2016 DHA: Modernize autotool support 
dnl         Feb 20 2015 andrewg@cray.com: Added Cray support.
dnl         Apr 30 2014 DHA: Declare 1.0.1 -- this doesn't have Cray support though.
dnl         Apr 15 2014 DHA: Drop tools/cobo/test
dnl         Mar 10 2014 MPL: Add secure handshake
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

dnl Users shouldn't bootstrap configure from the release tarball so 
dnl this technique should be okay
AC_INIT([LaunchMON],
        m4_esyscmd([git describe --always | awk '/.*/ {printf "%s",$1; exit}'])) 

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
AC_SUBST(LMON_AGE, 1)

dnl -----------------------------------------------
dnl AC_CONFIG support 
dnl -----------------------------------------------
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADER([config.h])
dnl config.guess and config.sub must be distributed 
AC_CANONICAL_SYSTEM

dnl
dnl Check for pkg-config
dnl
PKG_PROG_PKG_CONFIG


dnl -----------------------------------------------
dnl Automake support
dnl -----------------------------------------------
dnl The new form for AM_INIT_AUTOMAKE takes only one argument 
AM_INIT_AUTOMAKE([subdir-objects])
dnl If you don't want silent make, make V=1
AM_SILENT_RULES([yes])
dnl You don't want configure to rebuild Makefile.ins
AM_MAINTAINER_MODE

dnl -----------------------------------------------
dnl Checks for the OS/CPU/RM type
dnl -----------------------------------------------
X_AC_PLATFORM
X_AC_SLURM_MPAO_WR

dnl -----------------------------------------------
dnl Checks whether to enable SGI hostnames
dnl -----------------------------------------------
X_AC_SGI_HOSTNAMES

dnl -----------------------------------------------
dnl enable debug support
dnl -----------------------------------------------
X_AC_ENABLE_DEBUG

dnl -----------------------------------------------
dnl enable verbose support
dnl -----------------------------------------------
X_AC_ENABLE_VERBOSE

dnl -----------------------------------------------
dnl COBO is now the only comm. fab choice use to
dnl bootstrap launchmon's 
dnl distributed components  
dnl -----------------------------------------------
AC_DEFINE(COBO_BASED, 1, [Define 1 for COBO_BASED])
AC_DEFINE(TEST_MORE_COLL, 1, [Define test more coll support])
AC_DEFINE(TOOL_HOST_ENV, "LMON_FE_WHERETOCONNECT_ADDR", [Define TOOL_HOST_ENV])
AC_DEFINE(TOOL_PORT_ENV, "LMON_FE_WHERETOCONNECT_PORT", [Define TOOL_PORT_ENV] )
AC_DEFINE(TOOL_SS_ENV, "LMON_SHARED_SECRET", [Define TOOL_SS_ENV])
AC_DEFINE(TOOL_SCH_ENV, "LMON_SEC_CHK", [Define TOOL_SCH_ENV])
AC_SUBST(COMMLOC, $builddir/tools/cobo/src)
AC_SUBST(LIBCOMM, libcobo.la)
AC_ARG_VAR(COBOPORT, [Beginning port for COBO port scanning])
if test "x$COBOPORT" = "x"; then
    AC_DEFINE(COBO_BEGIN_PORT, 20101, [
        Define the beginning port for COBO port scanning])
else
    AC_DEFINE_UNQUOTED(COBO_BEGIN_PORT, [$COBOPORT], [
        Define the beginning port for COBO port scanning])
fi
AC_DEFINE(COBO_PORT_RANGE, 32, [Define 32 for COBO_BASED])

dnl -----------------------------------------------
dnl with GCRYPT support?
dnl -----------------------------------------------
AM_PATH_LIBGCRYPT([1.2.0],[
    AC_DEFINE([HAVE_GCRYPT],[1],[Gcrypt library is available])
    HAVE_GCRYPT=1
    # checking for available algorithms...
    GCRYPT_CFLAGS=`$LIBGCRYPT_CONFIG --cflags`
    GCRYPT_LIBS=`$LIBGCRYPT_CONFIG --libs`
    AC_SUBST(GCRYPT_CFLAGS)
    AC_SUBST(GCRYPT_LIBS)
    ],[
    # complain only if explicitely required
    AC_MSG_ERROR([cannot configure required gcrypt library])
    ])

dnl -----------------------------------------------
dnl Choose a security model for handshakes
dnl -----------------------------------------------
AC_CHECK_HEADERS([sys/types.h])
X_AC_HANDSHAKE

dnl -----------------------------------------------
dnl enable tracing cost measurement codes? 
dnl -----------------------------------------------
X_AC_TRACING_COST

dnl -----------------------------------------------
dnl Checks for NNODES and Ncores per node 
dnl for testing 
dnl -----------------------------------------------
X_AC_TESTNNODES
X_AC_NCORE_SMP
X_AC_TEST_RM
X_AC_MW_HOSTLIST
X_AC_TEST_INSTALLED

dnl -----------------------------------------------
dnl Checks for programs
dnl -----------------------------------------------
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_CC_C_O
AM_PROG_CC_C_O
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

LX_FIND_MPI

dnl -----------------------------------------------
dnl Checks for header files.
dnl Note: I don't think we made the code base conditionally
dnl compiled depending on the existence of these checks.
dnl But the code at least errors out during compililation 
dnl if they don't exist
dnl -----------------------------------------------
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([stack vector map list iostream string algorithm fstream sstream cxxabi.h])
AC_LANG_POP([C++])
AC_HEADER_SYS_WAIT

dnl
dnl Jun 07 2012 DHA TODO: We need to check boost/tokenizer.hpp; 
dnl doing so generates whole lot of errors on some platforms
dnl so I'm skipping that check for now. We need to 
dnl revisit that issue.

AC_CHECK_HEADERS([libelf.h libelf/libelf.h],[],[],
[[#include <elf.h>]])

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
AC_CONFIG_FILES([
  Makefile
  tools/Makefile
  tools/ciod/Makefile
  tools/cobo/Makefile
  tools/cobo/src/Makefile
  tools/alps/Makefile
  tools/alps/src/Makefile
  tools/handshake/Makefile
  launchmon/Makefile
  launchmon/src/Makefile
  launchmon/src/linux/Makefile
  launchmon/src/linux/lmon_api/Makefile
  launchmon/man/Makefile
  etc/Makefile
  etc/pkg-specs/Makefile
  etc/pkg-specs/llnl/Makefile
  test/Makefile
  test/src/Makefile])
AC_OUTPUT

