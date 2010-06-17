## $Header: $
##
## Makefile.am -- Process this file with automake to produce Makefile.in 
##
##
##--------------------------------------------------------------------------------
## Copyright (c) 2008, Lawrence Livermore National Security, LLC. Produced at
## the Lawrence Livermore National Laboratory. Written by Dong H. Ahn <ahn1@llnl.gov>.
## LLNL-CODE-409469. All rights reserved.
##
## This file is part of LaunchMON. For details, see
## https://computing.llnl.gov/?set=resources&page=os_projects
##
## Please also read LICENSE -- Our Notice and GNU Lesser General Public License.
##
##
## This program is free software; you can redistribute it and/or modify it under the
## terms of the GNU General Public License (as published by the Free Software
## Foundation) version 2.1 dated February 1999.
##
## This program is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License along
## with this program; if not, write to the Free Software Foundation, Inc., 59 Temple
## Place, Suite 330, Boston, MA 02111-1307 USA
##--------------------------------------------------------------------------------
##
##  Update Log:
##        Jun 12 2008 DHA: Added header files to the dependency list
##                         of lmonfeapi_la_SOURCES. 
##        Jun 06 2008 DHA: Taken from the old Makefile, converting
##                         to automake Makefile.am 
##        Mar 17 2008 DHA: Added PMGR Collective support.
##        Feb 09 2008 DHA: Added LLNS Copyright.
##        Dec 27 2006 DHA: Created file.
##

if WITH_CIOD
OPT_CIOD                     = -I$(top_srcdir)/@CIODLOC@ 
endif
OPT_GCRYPT                  = -I$(top_srcdir)/@GCRYPTLOC@ -I$(top_srcdir)/@GPGERRLOC@ 

INCLUDES                     = -I@ARCHHEADER@ $(OPT_CIOD) $(OPT_GCRYPT) -I.. -I../.. -I$(top_srcdir)/@COMMLOC@ 

BITS                         = @LIBBITSUFFIX@

lib_LTLIBRARIES              = libmonbeapi.la libmonfeapi.la

libmonfeapi_la_SOURCES       = lmon_fe.cxx \
			       lmon_lmonp_msg.cxx \
			       ../../sdbg_self_trace.cxx \
			       ../../sdbg_opt.cxx \
			       lmon_say_msg.cxx	\
			       lmon_be_internal.hxx \
			       ../../lmon_api/lmon_fe.h	\
			       ../../lmon_api/lmon_lmonp_msg.h \
                               ../../sdbg_opt.hxx \
			       ../../lmon_api/lmon_say_msg.hxx	

libmonfeapi_la_CFLAGS        = $(AM_CFLAGS)
libmonfeapi_la_CXXFLAGS      = $(AM_CXXFLAGS)

libmonfeapi_la_LDFLAGS       = -L$(top_srcdir)/@COMMLOC@ \
			       -L$(top_srcdir)/@GCRYPTLOC@ \
			       -L$(top_srcdir)/@GPGERRLOC@ \
			       $(AM_LDFLAGS) \
			       -version-info @LMON_CURRENT@:@LMON_REVISION@:@LMON_AGE@

libmonfeapi_la_LIBADD        = @LIBPTHREAD@ @LIBCOMM@ @LIBGCRYPT@ @LIBGPGERR@ @LIBRT@

libmonbeapi_la_SOURCES       = lmon_be.cxx \
			       lmon_be_comm.cxx \
			       lmon_lmonp_msg.cxx \
		               lmon_say_msg.cxx

libmonbeapi_la_CFLAGS        = $(AM_CFLAGS)
libmonbeapi_la_CXXFLAGS      = $(AM_CXXFLAGS)

libmonbeapi_la_LDFLAGS       = -L$(top_srcdir)/@COMMLOC@ \
			       -L$(top_srcdir)/@GCRYPTLOC@ \
			       -L$(top_srcdir)/@GPGERRLOC@ \
			       $(AM_LDFLAGS) \
			       -version-info @LMON_CURRENT@:@LMON_REVISION@:@LMON_AGE@ 

libmonbeapi_la_LIBADD        = @LIBCOMM@ @LIBGCRYPT@ @LIBGPGERR@ 
