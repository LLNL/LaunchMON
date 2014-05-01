# $Header: $
#
# x_ac_handshake.m4
#
# --------------------------------------------------------------------------------
# Copyright (c) 2014, Lawrence Livermore National Security, LLC. Produced at
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
#         Apr 11 2014 DHA: Change message for ENABLE_NULL_ENCRYPTION
#         Mar 10 2014 MPL: Add secure handshake

AC_DEFUN([X_AC_HANDSHAKE],[

        AC_ARG_ENABLE(sec-munge,
              [AS_HELP_STRING([--enable-sec-munge],[enable munge for connection authentication])],
              [WANT_MUNGE="true"; EXPLICIT_SEC="true"],)
        AC_ARG_ENABLE(sec-keydir,
              [AS_HELP_STRING([--enable-sec-keydir=keydir],[enable filesystem directory 'keydir' to propagate authentication key for connections])],
              [WANT_KEYFILE="true";SEC_KEYDIR=$enableval;EXPLICIT_SEC="true"],)
        AC_ARG_ENABLE(sec-none,
              [AS_HELP_STRING([--enable-sec-none],[disable security authentication of connections])],
              [WANT_NOSEC="true";EXPLICIT_SEC="true"],)
        m4_define_default([PKG_CHECK_MODULES],[AC_MSG_CHECKING([$1]); AC_MSG_RESULT([no]); $4])

        AC_LANG_PUSH(C)
        CFLAGS_HOLD=$CFLAGS
        LIBS_HOLD=$LIBS

        #Check for munge availability
        PKG_CHECK_MODULES(MUNGE, munge, , [AC_MSG_NOTICE([pkg-config could not find munge])])
        if test "x$MUNGE_LIBS" == "x"; then
           MUNGE_LIBS=-lmunge
        fi
        CFLAGS="$CFLAGS $MUNGE_CFLAGS"
        LIBS="$LIBS $MUNGE_LIBS"
        HAVE_MUNGE="true"
        AC_CHECK_HEADER([munge.h], , [HAVE_MUNGE="false"],
        [[#if HAVE_SYS_TYPES_H
          #include <sys/types.h>
          #endif]])
        AC_CHECK_LIB(munge, munge_encode, , [HAVE_MUNGE="false"])
        CFLAGS=$CFLAGS_HOLD
        LIBS=$LIBS_HOLD

        #If the user did not specify security options, then make some choices based on what we have
        if test "x$EXPLICIT_SEC" != "xtrue"; then
           if test "x$HAVE_MUNGE" == "xtrue"; then
             MUNGE="true"
           else
             ENABLE_NULL_ENCRYPTION="true"
           fi
        fi

        if test "x$WANT_MUNGE" == "xtrue"; then
           if test "x$HAVE_MUNGE" == "xtrue"; then
              MUNGE="true"
           else
              AC_MSG_ERROR([Could not find munge])
           fi
        fi

        if test "x$WANT_KEYFILE" == "xtrue"; then
           KEYFILE="true"
        fi

        if test "x$WANT_NOSEC" == "xtrue"; then
           ENABLE_NULL_ENCRYPTION="true"
        fi

        if test "x$MUNGE" != "xtrue"; then
           MUNGE_CFLAGS=""
           MUNGE_LIBS=""
        fi

        AC_LANG_POP

        #Remove everything between this comment and next to enable more than munge
        #if test "x$KEYFILE" != "x"; then 
        #  AC_MSG_ERROR([Keyfile security is not supported at this time])
        #fi
        #if test "x$ENABLE_NULL_ENCRYPTION x$MUNGE" == "xtrue xtrue"; then
        #  AC_MSG_ERROR([Cannot support multiple security models at this time])
        #fi
        #Remove above when expanding to more than munge

        AC_DEFINE_UNQUOTED([SEC_KEYDIR], "[$SEC_KEYDIR]",[Directory to store key files in])
        AC_SUBST(MUNGE_CFLAGS)
        AC_SUBST(MUNGE_LIBS)
        if test "x$MUNGE" == "xtrue"; then
           AC_MSG_NOTICE([Enabling munge for security authentication])        
           AC_DEFINE([MUNGE], [1], [Use munge for authentication])
        fi
        if test "x$KEYFILE" == "xtrue"; then
           AC_MSG_NOTICE([Enabling keyfile for security authentication])
           AC_DEFINE([KEYFILE], [1], [Use keyfile for authentication])
        fi
        if test "x$ENABLE_NULL_ENCRYPTION" == "xtrue"; then
           AC_MSG_NOTICE([WARNING: No secure handshake will be used for the COBO layer!!!]);
           AC_DEFINE([ENABLE_NULL_ENCRYPTION], [1], [Allow NULL encryption])
        fi])

