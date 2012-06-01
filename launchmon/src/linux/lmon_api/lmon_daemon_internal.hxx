/*
 * $Header: dahn Exp $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008-2010, Lawrence Livermore National Security, LLC. Produced at
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
 *        Aug  02 2010 DHA: Changed the file name to lmon_daemon_internal.hxx
 *                          from lmon_be_internal.hxx to cover middleware support
 *        Feb  05 2010 DHA: Changed the LMON_be_internal_init prototype and
 *                          augmented the per_be_data_t data structure to
 *                          support registering the local host name into 
 *                          lower layers if needed.
 *        Feb  09 2008 DHA: Added LLNS Copyright
 *        Jul  25 2007 DHA: Data structure change to deal with
 *                          slightly modified lmonp protocol
 *        Dec  29 2006 DHA: Created file.
 */

#ifndef LMON_DAEMON_INTERNAL_HXX
#define LMON_DAEMON_INTERNAL_HXX 1

#include "sdbg_std.hxx"
#include <string>
#include "sdbg_base_spawner.hxx"
#include "sdbg_rsh_spawner.hxx"
#include "lmon_coloc_spawner.hxx"

#define LMON_DAEMON_HN_MAX       108
#define LMON_DAEMON_MASTER       0
#define LMON_INIT                -1
#define LMON_BE_MSG_PREFIX       "<LMON BE API>"
#define LMON_MW_MSG_PREFIX       "<LMON MW API>"
#define LMON_DAEMON_MSG_PREFIX   "<LMON DAEMON INTERNAL>"
#define BEGIN_MASTER_ONLY(pdmon) if ( pdmon.daemon_data.myrank == LMON_DAEMON_MASTER ) {
#define END_MASTER_ONLY          }
#define BEGIN_SLAVE_ONLY(pdmon)  if ( pdmon.daemon_data.myrank != LMON_DAEMON_MASTER ) {
#define END_SLAVE_ONLY           }

typedef enum _tracingmode_e {
  trm_attach = 0,
  trm_launch,
  trm_launch_dontstop,
  trm_attach_stop
} tracingmode_e;

typedef struct _per_daemon_data_t {
  /*
   *  global rank of this backend daemon
   */
  int myrank;

  /*
   *  the number of backend daemon involved in this session
   */
  int width;

  /*
   * user pack function 
   */
  int (*pack) (void*,void*,int,int*); 

  /*
   * user unpack function 
   */
  int (*unpack) (void*,int, void*);

  /*
   * my_hostname and my_ip
   */
  char my_hostname[LMON_DAEMON_HN_MAX];
  char my_ip[LMON_DAEMON_HN_MAX];

  /*
   * for mw colocation assistant
   */
  spawner_base_t *daemon_spawner;

} per_daemon_data_t;


typedef struct _per_be_data_t {
  /*
   * per Daemon data 
   */
  per_daemon_data_t daemon_data;

  /*
   * this points to the raw proctab message 
   */
  lmonp_t *proctab_msg; 

  /*
   * the size of proctab_msg including its lmon_payload_length 
   */
  int proctab_msg_size; /* to hold the size of proctab_msg */  

  /*
   * is this launch case or attach case? 
   */
  int is_launch; /*  1: launch, 2: launch and dontstop, 0: attach, 3: attach and stop */

} per_be_data_t;


typedef struct _per_mw_data_t {
  /*
   * per Daemon data
   */
  per_daemon_data_t daemon_data;

} per_mw_data_t;


extern int LMON_daemon_return_ver();

extern lmon_rc_e LMON_daemon_getSharedKeyAndID ( char *shared_key, int *id );
extern lmon_rc_e LMON_daemon_internal_init( int *argc, char ***argv,
                                            char *myhn, int is_be);

extern lmon_rc_e LMON_daemon_internal_getConnFd( int *fd );
extern lmon_rc_e LMON_daemon_internal_getMyRank( int *rank );
extern lmon_rc_e LMON_daemon_internal_getSize( int *size );
extern lmon_rc_e LMON_daemon_internal_barrier();
extern lmon_rc_e LMON_daemon_internal_broadcast( void *buf, int numbyte );
extern lmon_rc_e LMON_daemon_internal_gather( void *sendbuf,
                     int numbyte_per_elem,
                     void* recvbuf );

extern lmon_rc_e LMON_daemon_internal_scatter( void *sendbuf,
                     int numbyte_per_element,
                     void* recvbuf );

extern lmon_rc_e LMON_daemon_internal_finalize(int is_be);
extern lmon_rc_e LMON_daemon_getWhereToConnect( struct sockaddr_in *servaddr );
extern lmon_rc_e LMON_daemon_gethostname(bool bgion,
                     char *my_hostname, int hlen, char *my_ip, int ilen);

extern lmon_rc_e LMON_daemon_enable_verbose(const char *vdir, 
                     const char *local_hostname, int is_be);

extern bool getPersonalityField(std::string &pFilePath,
                     std::string &fieldName, std::string &value);

extern bool resolvHNAlias ( std::string &hostsFilePath,
                     std::string &alias, std::string &IP);

extern bool is_bluegene_ion();

#endif // LMON_DAEMON_INTERNAL_HXX
