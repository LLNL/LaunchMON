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
##        Nov 04 2010 DHA: Added lib_SCRIPTS 
##        Jun 10 2010 DHA: Added alps_fe_colocator support
##        May 13 2010 DHA: Made for alps_be_starter
##


INCLUDES                          = -I$(top_srcdir) \
				    -I$(top_srcdir)/launchmon/src \
				    -I$(top_srcdir)/@LMONAPILOC@ \
				    -I@ALPS_INC_DIR@

bin_PROGRAMS              = alps_be_starter alps_fe_colocator 
alps_be_starter_SOURCES   = alps_be_starter.c 
alps_fe_colocator_SOURCES = alps_fe_colocator.cxx

alps_be_starter: alps_be_starter.c Makefile
	$(MPICC) $(DEFS) $(DEFAULT_INCLUDES) $(AM_CFLAGS) $(CFLAGS) -o $@ $<   

#alps_fe_colocator: alps_fe_colocator.cxx Makefile
#	$(CPP) $(DEFS) $(DEFAULT_INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) -o $@ $<   

alps_fe_colocator_CXXFLAGS      = @LNCHR_BIT_FLAGS@
alps_fe_colocator_LDADD         = @LIBELF@ @LIBALPS@
lib_SCRIPTS                     = CLE-dso.conf \
                                  CLE_2_2-dso.conf \
                                  CLE_3_1-dso.conf
