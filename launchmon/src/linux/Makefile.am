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
##        Mar 09 2009 DHA: Added -rdynamic to export process control 
##                         primities to the dynamic symbol table for
##                         dlopen to pick up.
##        Mar 06 2009 DHA: Added sdbg_linux_thr_db.[hxx|cxx] for 
##                         runtime loading support.
##                         of the thread debug library.
##                         Deprecated LIBTHRDDB.
##        Jun 12 2008 DHA: Added header files to the dependency list
##                         of launchmon_SOURCES.
##        Jun 06 2008 DHA: Taken from the old Makefile, converting
##                         to automake Makefile.am.
##        Mar 17 2008 DHA: Added PMGR Collective support.
##        Feb 09 2008 DHA: Added LLNS Copyright.
##        Dec 27 2006 DHA: Created file.
##

SUBDIRS                 = lmon_api

MYBOOST_SUPPORT         = -I$(top_srcdir)/@MYBOOSTLOC@

INCLUDES                = $(MYBOOST_SUPPORT) -I.. -I@ARCHHEADER@
#BITS                    = @LIBBITSUFFIX@

bin_PROGRAMS            = launchmon
launchmon_SOURCES       = main.cxx \
                          sdbg_linux_launchmon.cxx \
                          lmon_api/lmon_lmonp_msg.cxx \
                          sdbg_linux_mach.cxx \
                          lmon_api/lmon_say_msg.cxx \
                          sdbg_proc_service.cxx \
	                  ../ini_reader.cpp \
                          ../ini_reader.h \
			  ../ini.c \
                          ../sdbg_self_trace.cxx \
                          ../sdbg_opt.cxx \
                          ../sdbg_rm_map.cxx \
                          ../sdbg_base_bp.hxx \
                          ../sdbg_base_launchmon_impl.hxx  \
                          ../sdbg_base_tracer.hxx \
                          ../sdbg_self_trace.hxx \
                          ../sdbg_base_driver.hxx \
                          ../sdbg_base_mach.hxx \
                          ../sdbg_signal_hlr.hxx \
                          ../sdbg_base_driver_impl.hxx  \
                          ../sdbg_base_mach_impl.hxx  \
                          ../sdbg_event_manager.hxx \
                          ../sdbg_signal_hlr_impl.hxx \
                          ../sdbg_base_exception.hxx \
                          ../sdbg_base_symtab.hxx \
                          ../sdbg_event_manager_impl.hxx  \
                          ../sdbg_std.hxx \
                          ../sdbg_base_launchmon.hxx \
                          ../sdbg_base_symtab_impl.hxx \
                          ../sdbg_opt.hxx \
                          ../sdbg_rm_map.hxx \
                          ../sdbg_base_spawner.hxx \
                          sdbg_linux_bp.hxx \
                          sdbg_linux_launchmon.hxx  \
                          sdbg_linux_ptracer_impl.hxx  \
                          sdbg_linux_symtab_impl.hxx \
                          sdbg_proc_service.hxx \
                          sdbg_linux_driver.hxx  \
                          sdbg_linux_mach.hxx \
                          sdbg_linux_std.hxx \
                          sdbg_linux_driver_impl.hxx \
                          sdbg_linux_ptracer.hxx \
                          sdbg_linux_symtab.hxx 

launchmon_LDADD         = @LIBELF@ @LIBDL@
launchmon_CFLAGS        = @LNCHR_BIT_FLAGS@ #$(AM_CFLAGS)
launchmon_CXXFLAGS      = @LNCHR_BIT_FLAGS@ #$(AM_CXXFLAGS)
launchmon_LDFLAGS       = -rdynamic @LNCHR_BIT_FLAGS@ $(AM_LDFLAGS)

## EOF
