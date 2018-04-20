/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/lmon_api/lmon_api_std.h,v 1.5.2.4 2008/02/20 17:37:57 dahn Exp $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008 ~ 2012, Lawrence Livermore National Security, LLC. Produced at 
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
 *        May  31 2012 DHA: Merged with the middleware support from
 *                          the 0.8-middleware-support branch.
 *        Oct 10 2011 DHA: Augmented _lmon_rm_info_t in support of the dynamic
 *                         resource manager detection scheme.
 *        Jul 02 2010 DHA: Typedef'ed lmon_mw_mode_t in prep for MW support
 *                         Added MW related macros
 *        Jun 28 2010 DHA: Added LMON_fe_getRMInfo support;
 *                         moved rm_catalogue_e to here and added
 *                         lmon_rm_info_t here
 *        May 20 2009 DHA: Change LMON_VERSION.
 *        Jun 06 2008 DHA: Change C++ compiler support style.
 *                         (GNU).
 *        Feb 09 2008 DHA: Added LLNS Copyright.
 *        Aug 15 2006 DHA: More error code Populated
 *        Dec 20 2006 DHA: Created file.
 */

#ifndef LMON_API_LMON_API_STD_H
#define LMON_API_LMON_API_STD_H 1

#include <lmon_api/common.h>

BEGIN_C_DECLS

#undef  LMON_VERSION
#define LMON_VERSION          900100 /* version 1.0beta */

#define LMON_FE_ADDR_ENVNAME  "LMON_FE_WHERETOCONNECT_ADDR"
#define LMON_FE_PORT_ENVNAME  "LMON_FE_WHERETOCONNECT_PORT"
#define LMON_SHRD_SEC_ENVNAME "LMON_SHARED_SECRET"
#define LMON_SEC_CHK_ENVNAME  "LMON_SEC_CHK"
#define LMON_VERBOSE_ENVNAME  "LMON_VERBOSITY"
#define LMON_KEY_LENGTH       16      /* 128 bits */
#define LMON_MAX_USRPAYLOAD   4194304 /* 4 MB */
#define LMON_MAX_NDAEMONS     8192
#define LMON_NTASKS_THRE      32769   /* nTasks cutoff to switching over to long_num_tasks */
//#define LMON_NTASKS_THRE      1025
#define LMON_HOSTS_FN_BASE    "hostnamefn"
#define LMON_MW_COLOC         0x1
#define LMON_MW_EXISTINGALLOC 0x1 << 1
#define LMON_MW_NEWALLOC      0x1 << 2
#define LMON_MW_HOSTLIST      0x1 << 3
#define LMON_N_MW_TYPES       4       /* how many LMON_MW types? */
#define IS_MW_COLOC(flag)     (flag & LMON_MW_COLOC)? 1:0
#define IS_MW_EXISTINGALLOC(flag) (flag & LMON_MW_EXISTINGALLOC)? 1:0
#define IS_MW_NEWALLOC(flag)  (flag & LMON_MW_NEWALLOC)? 1:0
#define IS_MW_HOSTLIST(flag)  (flag & LMON_MW_HOSTLIST)? 1:0


//! lmon_api_std.h
/*!
    this file defines standard data structure necessary to 
    implement LMON BE, MW and FE.
*/
typedef enum _lmon_rc_e {
  LMON_OK = 0,
  LMON_EINVAL,
  LMON_EBDARG,
  LMON_ELNCHR,
  LMON_EINIT,
  LMON_ESYS,
  LMON_ESUBCOM,
  LMON_ESUBSYNC,
  LMON_ETOUT,
  LMON_ENOMEM,
  LMON_ENCLLB,
  LMON_ECLLB,
  LMON_ENEGCB,
  LMON_ENOPLD,
  LMON_EBDMSG,
  LMON_EDUNAV,
  LMON_ETRUNC,
  LMON_EBUG,
  LMON_NOTIMPL,
  LMON_YES,
  LMON_NO
} lmon_rc_e;

typedef enum _rm_catalogue_e
{
  RC_mchecker_rm = 0,
  RC_slurm,
  RC_bglrm,
  RC_bgprm,
  RC_bgqrm,
  RC_bgq_slurm,
  RC_bgrm,
  RC_alps,
  RC_orte,
  RC_mpiexec_hydra,
  RC_gupc,
  RC_none
  /*
    new RMs should be added here as LaunchMON is ported 
    on other RMs	
  */
} rm_catalogue_e;

typedef struct _lmon_rm_info_t {
  rm_catalogue_e *rm_supported_types;
  int num_supported_types;
  int index_to_cur_instance;
  pid_t rm_launcher_pid;
} lmon_rm_info_t;

typedef struct _lmon_daemon_env_t {
  char *envName;
  char *envValue;
  struct _lmon_daemon_env_t *next;
} lmon_daemon_env_t;


typedef struct _lmon_daemon_local_t {
  pid_t pid;
  int pidMPIRank;
  const char *exename;
  struct _lmon_daemon_local_t *next;
} lmon_daemon_local_t;


typedef enum _dist_req_opt_e { 
  subset_stride,
  subset_hosts,
  hostlists,
  newalloc_nhosts,
  req_none,
  space_for_rent
} dist_req_opt_e;

typedef int lmon_mw_mode_t;

typedef struct _dist_request_t {
  lmon_mw_mode_t md;      /* which volume is this request? */
  char *mw_daemon_path;   /* middleware daemon path */
  char **d_argv;          /* Null-terminated daemon args */
  int ndaemon;            /* if one daemon per node meets all the requirements, this can go away */
  int block;              /* how many to fill a node at a time */
  int cyclic;       /* cyclic? if ndaemon > nNode in the volume * block, cyclic=yes allows to iterate */
  dist_req_opt_e optkind;
  union u {
    int stride_unit;
    int nhosts;
    char **subset_hl;
    char **hl;
  } option;
} dist_request_t;

END_C_DECLS

#endif /* LMON_API_LMON_API_STD_H */
