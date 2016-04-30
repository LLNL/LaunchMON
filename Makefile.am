## $Header: $
##
## Makefile.am -- Process this file with automake to produce Makefile.in 
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
##        Apr 28 2016 DHA: Modernized build system 
##        Jan 11 2013 DHA: Added launchmon.spec and launchmon.module for
##                         RPM support. Added INSTALL.launchmon, as 'make dist'
##                         was not distributing that file.
##        Sep 30 2008 DHA: Added README.ERROR_HANDLING and package.conf 
##                         to the distribution list
##        Jun 06 2008 DHA: Copied from the old Makefile.
##        Mar 15 2008 DHA: Added PMGR Collective support
##        Mar 11 2008 DHA: Added Linux PowerPC support (BlueGene FEN)
##        Feb 09 2008 DHA: Added LLNS Copyright
##        Mar 27 2006 DHA: Created file.
##

SUBDIRS = \
  tools \
  launchmon \
  etc \
  test

EXTRA_DIST = \
  config/ap \
  README.ERROR_HANDLING \
  README.md

ACLOCAL_AMFLAGS = -I config

