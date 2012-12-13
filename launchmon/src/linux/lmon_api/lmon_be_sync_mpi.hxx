/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/lmon_api/lmon_be.cxx,v 1.12.2.5 2008/02/20 17:37:57 dahn Exp $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008, Lawrence Livermore National Security, LLC. Produced at 
 * the Lawrence Livermore National Laboratory. Written by Dong H. Ahn <ahn1@llnl.gov>. 
 * LLNL-CODE-409469. All rights reserved.
 *
 * This file is part of LaunchMON. For details, see 
 * https://computing.llnl.gov/?set=resources&page=os_projects
 *
 * Please also read LICENSE.txt -- Our Notice and GNU Lesser General Public License.
 *
 * 
 * This program is free software; you can redistribute it and/or modify it under the 
 * terms of the GNU General Public License (as published by the Free Software 
 * Foundation) version 2.1 dated February 1999.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 59 Temple 
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *--------------------------------------------------------------------------------                      
 *
 *  Update Log:
 *              Oct 31 2011 DHA: File created
 *
 */

#ifndef LMON_BE_SYNC_MPI_HXX
#define LMON_BE_SYNC_MPI_HXX 1

#include "../../sdbg_rm_map.hxx"
#include "lmon_api/lmon_api_std.h"
#include "lmon_api/lmon_proctab.h"

extern lmon_rc_e LMON_be_procctl_init ( rm_catalogue_e rmtype,
                   MPIR_PROCDESC_EXT *ptab,
                   int islaunch,
                   int psize,
		   int dontstop );

extern lmon_rc_e LMON_be_procctl_tester_init ( rm_catalogue_e rmtype,
                   MPIR_PROCDESC_EXT *ptab,
                   int islaunch,
                   int psize,
		   int dontstop );

extern lmon_rc_e LMON_be_procctl_stop ( rm_catalogue_e rmtype,
                   MPIR_PROCDESC_EXT *ptab,
                   int psize );

extern lmon_rc_e LMON_be_procctl_tester_run ( rm_catalogue_e rmtype,
                   MPIR_PROCDESC_EXT *ptab,
                   int islaunch,
                   int psize,
		   int dontstop );

extern lmon_rc_e LMON_be_procctl_run ( rm_catalogue_e rmtype,
                   int signum,
                   MPIR_PROCDESC_EXT *ptab,
                   int psize );

extern lmon_rc_e LMON_be_procctl_perf ( rm_catalogue_e rmtype,
                   MPIR_PROCDESC_EXT *ptab,
                   int psize,
                   long unsigned int membase,
                   unsigned int numbytes,
                   unsigned int *fetchunit,
                   unsigned int *usecperunit);

extern lmon_rc_e LMON_be_procctl_initdone ( rm_catalogue_e rmtype,
                   MPIR_PROCDESC_EXT *ptab,
                   int islaunch,
                   int psize );

extern lmon_rc_e LMON_be_procctl_done ( rm_catalogue_e rmtype,
                   MPIR_PROCDESC_EXT *ptab,
                   int psize );

#endif // LMON_BE_SYNC_MPI_HXX
