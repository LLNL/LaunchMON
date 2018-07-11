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
##        Apr 28 2016 DHA: Modernize build environment:
##                           Make handshake and cobo mandatory and
##                           Drop bundling of grypt/gpg packages
##        Apr 01 2015 ADG: Added Cray CTI support
##        Mar 10 2014 MPL: Add secure handshake
##        Dec 16 2009 DHA: Added COBO support
##        Jun 10 2007 DHA: Copied from the old Makefile.
##

if WITH_CIOD
  OPT_CIOD = ciod
endif
if WITH_ALPS
  OPT_ALPS = alps
endif
if WITH_CTI
  OPT_CTI = cti
endif
  OPT_HANDSHAKE = handshake

SUBDIRS =  handshake cobo $(OPT_CIOD) $(OPT_ALPS) $(OPT_CTI)

