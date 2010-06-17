/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/lmon_api/lmon_be_internal.hxx,v 1.3.2.2 2008/02/20 17:37:57 dahn Exp $
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
 *        Feb  05 2010 DHA: Changed the LMON_be_internal_init prototype and
 *                          augmented the per_be_data_t data structure to
 *                          support registering the local host name into 
 *                          lower layers if needed.
 *        Feb  09 2008 DHA: Added LLNS Copyright
 *        Jul  25 2007 DHA: Data structure change to deal with
 *                          slightly modified lmonp protocol
 *        Dec  29 2006 DHA: Created file. 
 */

#ifndef LMON_BE_COMM_HXX
#define LMON_BE_COMM_HXX 1
 
#define LMON_BE_HN_MAX      108
#define LMON_BE_MASTER      0
#define LMON_INIT           -1
#define LMON_BE_MSG_PREFIX  "<LMON BE API>"
#define BEGIN_MASTER_ONLY   if ( bedata.myrank == LMON_BE_MASTER ) {
#define END_MASTER_ONLY     }
#define BEGIN_SLAVE_ONLY    if ( bedata.myrank != LMON_BE_MASTER ) {
#define END_SLAVE_ONLY      }

typedef struct _per_be_data_t {
	
  /*
   *  global rank of this backend daemon
   */
  int myrank;  
  
  /*
   *  the number of backend daemon involved in this session
   */
  int width;
  
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
  int is_launch; /*  1 for launch 0 for attach */
  
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
  char my_hostname[LMON_BE_HN_MAX];
  char my_ip[LMON_BE_HN_MAX]; 
  
} per_be_data_t;


extern lmon_rc_e LMON_be_internal_init ( int* argc, char*** argv, char *myhn);

extern lmon_rc_e LMON_be_internal_getConnFd ( int* fd );

extern lmon_rc_e LMON_be_internal_getMyRank ( int *rank );

extern lmon_rc_e LMON_be_internal_getSize ( int* size );

extern lmon_rc_e LMON_be_internal_barrier ();

extern lmon_rc_e LMON_be_internal_broadcast ( void* buf, int numbyte );

extern lmon_rc_e LMON_be_internal_gather ( void *sendbuf,
		     int numbyte_per_elem,
		     void* recvbuf );

extern lmon_rc_e LMON_be_internal_scatter ( void *sendbuf,
		     int numbyte_per_element,
		     void* recvbuf );

extern lmon_rc_e LMON_be_internal_finalize ();

#endif // LMON_BE_COMM_HXX
