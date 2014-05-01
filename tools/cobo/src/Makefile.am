## $Header: $
##
## Makefile.am -- Process this file with automake to produce Makefile.in 
##
##--------------------------------------------------------------------------------
## Copyright (c) 2008, Lawrence Livermore National Security, LLC. Produced at
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
##        Mar 10 2014 MPL: Add secure handshake
##        Dec 16 2009 DHA: Made for COBO
##

lib_LTLIBRARIES = libcobo.la 
libcobo_la_SOURCES = cobo.c \
	          cobo.h
include_HEADERS = cobo.h 
libcobo_la_LDFLAGS = -version-info @LMON_CURRENT@:@LMON_REVISION@:@LMON_AGE@
libcobo_la_CFLAGS = -I$(top_srcdir)/tools/handshake
libcobo_la_LIBADD = $(top_builddir)/tools/handshake/libhandshake.la
