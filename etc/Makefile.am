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
##        May 18 2018 ADG: Added Cray CTI support
##        Apr 28 2016 DHA: Created file.
##        Aug 13 2021 GLL: use input file for slurm to allow configure-time options
##

SUBDIRS = pkg-specs

etcdir = ${prefix}/etc
etc_SCRIPTS = \
  rm_info.conf \
  rm_alps.conf \
  rm_bglrm.conf \
  rm_bgprm.conf \
  rm_bgqrm.conf \
  rm_bgq_slurm.conf \
  rm_cray.conf \
  rm_mchecker.conf \
  rm_openrte.conf \
  rm_slurm.conf \
  rm_mpiexec_hydra.conf

EXTRA_DIST = \
  rm_info.conf \
  rm_alps.conf \
  rm_bglrm.conf \
  rm_bgprm.conf \
  rm_bgqrm.conf \
  rm_cray.conf \
  rm_gupc.conf \
  rm_bgq_slurm.conf \
  rm_mchecker.conf \
  rm_openrte.conf \
  rm_slurm.conf.in \
  rm_mpiexec_hydra.conf

