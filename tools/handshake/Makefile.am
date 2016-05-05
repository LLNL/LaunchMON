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
##        Apr 28 2016 DHA: Minor cleanup
##        May 01 2014 DHA: Specify gcrypt header file locations so that
##                         this module will use the integrated version of
##                         gcrypt/gpgerr package. TODO: drop support for
##                         the integrated gcrypt support and make it an
##                         external dependency.
##        Apr 15 2014 DHA: Add handshake.h so that it is included into the distro
##        Mar 10 2014 MPL: Add secure handshake
##

noinst_LTLIBRARIES = libhandshake.la 
libhandshake_la_SOURCES = handshake.c handshake.h
libhandshake_la_LIBADD = $(MUNGE_LIBS)

