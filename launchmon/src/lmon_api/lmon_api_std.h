/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/lmon_api/lmon_api_std.h,v 1.5.2.4 2008/02/20 17:37:57 dahn Exp $
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
 *        Jun 06 2008 DHA: Change C++ compiler support style  
 *                         (GNU). 
 *        Feb 09 2008 DHA: Added LLNS Copyright 
 *        Aug 15 2006 DHA: More error code Populated
 *        Dec 20 2006 DHA: Created file.          
 */

#ifndef LMON_API_LMON_API_STD_H
#define LMON_API_LMON_API_STD_H 1

#include <lmon_api/common.h>

BEGIN_C_DECLS

#undef  LMON_VERSION
#define LMON_VERSION          90004 /* version 0.4 */

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


//! lmon_api_std.h
/*!
    this file defines standard data structure necessary to 
    implement LMON BE, MID and FE.
*/

typedef enum _lmon_rc_e {  
  LMON_OK = 0,
  LMON_EINVAL,
  LMON_EBDARG,
  LMON_ELNCHR,
  LMON_EINIT,
  LMON_ESYS,
  LMON_ESUBCOM,
  LMON_ETOUT,
  LMON_ENOMEM, 
  LMON_ENCLLB,
  LMON_ECLLB,
  LMON_ENEGCB,
  LMON_ENOPLD,
  LMON_EBDMSG,
  LMON_EDUNAV,
  LMON_EBUG,
  LMON_YES,
  LMON_NO
} lmon_rc_e;


typedef struct _lmon_daemon_env_t {
  char *envName;
  char *envValue;
  struct _lmon_daemon_env_t *next;
} lmon_daemon_env_t;


typedef struct _lmon_daemon_local_t {
  pid_t pid;
  int pidMPIRank;
  const char* exename;
  struct _lmon_daemon_local_t* next;
} lmon_daemon_local_t;

END_C_DECLS

#endif /* LMON_API_LMON_API_STD_H */
