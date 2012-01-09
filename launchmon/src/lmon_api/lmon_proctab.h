/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/lmon_api/lmon_proctab.h,v 1.3.2.1 2008/02/20 17:37:58 dahn Exp $
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
 *
 *  Update Log:
 *        Feb 09 2008 DHA: Added LLNS Copyright 
 *        Dec 21 2006 DHA: Created file.          
 */

#ifndef LMON_API_LMON_PROC_TAB_H
#define LMON_API_LMON_PROC_TAB_H 1

#include <lmon_api/common.h> 

#define MPIR_NULL           0
#define MPIR_DEBUG_SPAWNED  1
#define MPIR_DEBUG_ABORTING 2

BEGIN_C_DECLS

//! MPIR_PROCDESC
/*!
    this should be matched with one that launcher itself used.
*/
typedef struct {
  char *host_name;           /* Something we can pass to inet_addr */
  char *executable_name;     /* The name of the image */
  int   pid;                 /* The pid of the process */
} MPIR_PROCDESC;


//! MPIR_PROCDESC_EXT
/*!
    We want to be able to encode the MPI rank information in addition
    to what's available in MPIR_PROCDESC; thus is MPIR_PROCDESC_EXT
    is a simple extension to MPIR_PROCDESC.

    The reason is Launchmon will distribute a fraction of this table to 
    each LMON BE so that the index info into the global table won't
    provide the necessary MPI rank info any longer. 

*/
typedef struct {
  MPIR_PROCDESC pd;
  int mpirank;
  int cnodeid;
} MPIR_PROCDESC_EXT;

END_C_DECLS 

#endif /* LMON_API_LMON_PROC_TAB_H */
