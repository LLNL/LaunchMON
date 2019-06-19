## $Header: $
##
## Makefile.am -- Process this file with automake to produce Makefile.in
##
##
##--------------------------------------------------------------------------------
## Copyright (c) 2008 ~ 2012, Lawrence Livermore National Security, LLC. Produced at
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
##        Apr 28 2016 DHA: Modernize autotools and build support
##        Jun 01 2012 DHA: Merged with the 0.8-middleware-support branch.
##        Nov 28 2011 DHA: Added MPI-tool sync layer.
##        Jun 12 2008 DHA: Added header files to the dependency list
##                         of lmonfeapi_la_SOURCES.
##        Jun 06 2008 DHA: Taken from the old Makefile, converting
##                         to automake Makefile.am.
##        Mar 17 2008 DHA: Added PMGR Collective support.
##        Feb 09 2008 DHA: Added LLNS Copyright.
##        Dec 27 2006 DHA: Created file.
##

AUTOMAKE_OPTIONS = subdir-objects
PLAT_SRC_DIR = $(top_srcdir)/launchmon/src/linux
BASE_SRC_DIR = $(top_srcdir)/launchmon/src
VPATH = $(PLAT_SRC_DIR):$(BASE_SRC_DIR):$(srcdir)
if WITH_CIOD
CIOD_SUPPORT =-I$(top_srcdir)/@CIODLOC@
endif
AM_CPPFLAGS = \
  -I@ARCHHEADER@ \
  $(CIOD_SUPPORT) \
  -I$(PLAT_SRC_DIR)/ \
  -I$(BASE_SRC_DIR)/ \
  -I$(top_srcdir)/tools/cobo/src \
  -I$(top_srcdir)/tools/handshake

lib_LTLIBRARIES = \
  libmonbeapi.la \
  libmonfeapi.la \
  libmonmwapi.la

####################################################
# Frontend API
####################################################
libmonfeapi_la_SOURCES = \
  lmon_fe.cxx \
  lmon_lmonp_msg.cxx \
  lmon_coloc_spawner.cxx \
  lmon_say_msg.cxx \
  $(BASE_SRC_DIR)/sdbg_rm_map.cxx \
  $(BASE_SRC_DIR)/sdbg_self_trace.cxx \
  $(BASE_SRC_DIR)/sdbg_opt.cxx \
  $(PLAT_SRC_DIR)/sdbg_rsh_spawner.cxx \
  lmon_coloc_spawner.hxx \
  $(PLAT_SRC_DIR)/sdbg_rsh_spawner.hxx \
  $(BASE_SRC_DIR)/lmon_api/lmon_api_std.h \
  $(BASE_SRC_DIR)/lmon_api/lmon_fe.h \
  $(BASE_SRC_DIR)/lmon_api/lmon_proctab.h \
  $(BASE_SRC_DIR)/lmon_api/lmon_lmonp_msg.h \
  $(BASE_SRC_DIR)/sdbg_opt.hxx \
  $(BASE_SRC_DIR)/sdbg_rm_map.hxx \
  $(BASE_SRC_DIR)/lmon_api/lmon_say_msg.hxx \
  $(BASE_SRC_DIR)/sdbg_base_spawner.hxx
libmonfeapi_la_CFLAGS = $(AM_CFLAGS)
libmonfeapi_la_CXXFLAGS = $(AM_CXXFLAGS)
libmonfeapi_la_LDFLAGS = \
  $(AM_LDFLAGS) -version-info \
  @LMON_CURRENT@:@LMON_REVISION@:@LMON_AGE@
libmonfeapi_la_LIBADD = \
  @LIBPTHREAD@ \
  $(top_builddir)/@COMMLOC@/@LIBCOMM@ \
  $(GCRYPT_LIBS) \
  @LIBRT@

####################################################
# Backend API
####################################################
libmonbeapi_la_SOURCES = \
  lmon_be.cxx \
  lmon_daemon_internal.cxx \
  lmon_coloc_spawner.cxx \
  lmon_lmonp_msg.cxx \
  lmon_say_msg.cxx \
  lmon_be_sync_mpi.cxx \
  lmon_be_sync_mpi_generic.cxx \
  lmon_be_sync_mpi_bg.cxx \
  lmon_be_sync_mpi_bgq.cxx \
  lmon_coloc_spawner.hxx \
  lmon_daemon_internal.hxx \
  lmon_be_sync_mpi.hxx \
  lmon_be_sync_mpi_generic.hxx \
  lmon_be_sync_mpi_bg.hxx \
  lmon_be_sync_mpi_bgq.hxx \
  $(BASE_SRC_DIR)/lmon_api/lmon_api_std.h \
  $(BASE_SRC_DIR)/lmon_api/lmon_be.h \
  $(BASE_SRC_DIR)/lmon_api/lmon_proctab.h \
  $(BASE_SRC_DIR)/lmon_api/lmon_lmonp_msg.h \
  $(BASE_SRC_DIR)/lmon_api/lmon_say_msg.hxx \
  $(BASE_SRC_DIR)/sdbg_base_spawner.hxx
libmonbeapi_la_CFLAGS = $(AM_CFLAGS)
libmonbeapi_la_CXXFLAGS = $(AM_CXXFLAGS)
libmonbeapi_la_LDFLAGS = \
  $(AM_LDFLAGS) \
  -version-info \
  @LMON_CURRENT@:@LMON_REVISION@:@LMON_AGE@
libmonbeapi_la_LIBADD = \
  $(top_builddir)/@COMMLOC@/@LIBCOMM@ \
  $(GCRYPT_LIBS)

#####################################################
# Middleware API
####################################################
libmonmwapi_la_SOURCES = \
  lmon_mw.cxx \
  lmon_daemon_internal.cxx \
  lmon_lmonp_msg.cxx \
  lmon_say_msg.cxx \
  lmon_coloc_spawner.cxx \
  $(PLAT_SRC_DIR)/sdbg_rsh_spawner.cxx \
  lmon_coloc_spawner.hxx \
  lmon_daemon_internal.hxx \
  $(PLAT_SRC_DIR)/sdbg_rsh_spawner.hxx \
  $(PLAT_SRC_DIR)/sdbg_rm_spawner.hxx \
  $(BASE_SRC_DIR)/lmon_api/lmon_api_std.h \
  $(BASE_SRC_DIR)/lmon_api/lmon_mw.h \
  $(BASE_SRC_DIR)/lmon_api/lmon_proctab.h \
  $(BASE_SRC_DIR)/lmon_api/lmon_lmonp_msg.h \
  $(BASE_SRC_DIR)/lmon_api/lmon_say_msg.hxx \
  $(BASE_SRC_DIR)/sdbg_base_spawner.hxx
libmonmwapi_la_CFLAGS = $(AM_CFLAGS)
libmonmwapi_la_CXXFLAGS = $(AM_CXXFLAGS)
libmonmwapi_la_LDFLAGS = \
  $(AM_LDFLAGS) \
  -version-info \
  @LMON_CURRENT@:@LMON_REVISION@:@LMON_AGE@
libmonmwapi_la_LIBADD = \
  $(top_builddir)/@COMMLOC@/@LIBCOMM@ \
  $(GCRYPT_LIBS)

