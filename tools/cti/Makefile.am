## $Header: $
##
## Makefile.am -- Process this file with automake to produce Makefile.in 
##
##--------------------------------------------------------------------------------
## Copyright (c) 2010, Lawrence Livermore National Security, LLC. Produced at
## the Lawrence Livermore National Laboratory. Written by Dong H. Ahn <ahn1@llnl.gov>.
## LLNL-CODE-401141. All rights reserved.
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
##        Mar 31 2015 DHA: Made for cti_fe_colocator
##


AM_CPPFLAGS                	= -I$(top_srcdir) \
                           	  -I$(top_srcdir)/launchmon/src \
                              -I$(top_srcdir)/@LMONAPILOC@

bin_PROGRAMS                = cti_fe_colocator 
cti_fe_colocator_SOURCES    = cti_fe_colocator.cxx 
cti_fe_colocator_CXXFLAGS   = @LNCHR_BIT_FLAGS@ $(CRAY_CTI_CFLAGS)
cti_fe_colocator_LDADD      = $(CRAY_CTI_LIBS)

