/*
 * $Header: $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2010, Lawrence Livermore National Security, LLC. Produced at 
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
 *  Update Log:
 *        Aug 2 2010 DHA: Created file
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/common.h>
#include <iostream>
#include <lmon_api/lmon_mw.h>

#define COMMUNICATION_UNIT          3
#define SLEEP_FOR_COMM_SEC          3

/*
 * Multi-purpose comm helper daemon for testing
 *
 * Usage:
 *
 */

int
main( int argc, char* argv[] )
{
  lmon_rc_e rc;
  int size, rank;

  if ( (rc = LMON_mw_init(LMON_VERSION, &argc, &argv))
              != LMON_OK )
    {
      fprintf(stderr,
        "[COMM HELPER] FAILED: LMON_mw_init\n");
      return EXIT_FAILURE;
    }

  LMON_mw_getSize(&size);
  LMON_mw_getMyRank(&rank);

  if ( (rc = LMON_mw_handshake(NULL)) 
              != LMON_OK )
    {
      fprintf(stderr,
        "[COMM HELPER(%d)] FAILED: LMON_mw_handshake\n",
        rank );
      return EXIT_FAILURE;
    }

  if ( (rc = LMON_mw_ready(NULL))
              != LMON_OK )
    {
      fprintf(stderr,
        "[COMM HELPER(%d)] FAILED: LMON_mw_ready\n",
        rank );
      LMON_mw_finalize();
      return EXIT_FAILURE;
    }

  //
  //
  // DO COMM
  //
  //
  int remain = COMMUNICATION_UNIT * SLEEP_FOR_COMM_SEC;
  int i;
  for (i=0; i < COMMUNICATION_UNIT; i++) 
    {
      sleep(SLEEP_FOR_COMM_SEC);
      remain -= SLEEP_FOR_COMM_SEC;
      if (rank == 0)
        fprintf(stdout,
          "[COMM HELPER(%d)] %d secs remain\n", rank, remain);
    }

  // sending this to mark the end of the BE session 
  // This should be used to determine PASS/FAIL criteria 
  if ( (( rc = LMON_mw_sendUsrData ( NULL )) == LMON_EBDARG)
       || ( rc == LMON_EINVAL )
       || ( rc == LMON_ENOMEM )
       || ( rc == LMON_ENEGCB ))
     {
       fprintf(stdout, "[COMM HELPER(%d)] FAILED(%d): LMON_mw_sendUsrData\n",
               rank, rc );
       LMON_mw_finalize();
       return EXIT_FAILURE;
     }

  LMON_mw_finalize();

  return EXIT_SUCCESS;
}
