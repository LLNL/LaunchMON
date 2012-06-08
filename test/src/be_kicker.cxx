/*
 * $Header: $
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
 *  Update Log:
 *        Mar 04 2008 DHA: Added generic BlueGene support
 *        Jun 12 2008 DHA: Added GNU build support.
 *        Mar 20 2008 DHA: Added BlueGene support.
 *        Feb 09 2008 DHA: Dynamic RPDTAB buff size support with the
 *                         addition of LMON_be_getMyProctabSize. 
 *        Feb 09 2008 DHA: Added LLNS Copyright.
 *        Jul 30 2007 DHA: Adjust this case for minor API changes. 
 *        Dec 20 2006 DHA: Created file.          
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/common.h>

#if HAVE_UNISTD_H
# include <unistd.h>
#else
# error unistd.h is required
#endif
                                                                    
#include <iostream>
                                                                    
#if HAVE_SIGNAL_H
# include <signal.h>
#else
# error signal.h is required
#endif
                                                                    
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#else
# error sys/types.h is required
#endif
                                                                    
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#else
# error sys/wait.h is required
#endif
                                                                    
#include <lmon_api/lmon_be.h>
#include "lmon_daemon_internal.hxx"
#include "lmon_be_sync_mpi.hxx"

enum be_subtest_e {
  subtest_no = 0,
  subtest_kill_detach_shutdown,
  subtest_mw_coloc, 
  subtest_reserved
};

const unsigned int kill_detach_shutdown_time = 60;

/*
 * Multipurpose kicker daemon
 *
 * Usage: be_kicker signumber waittime
 *
 * argv[1]: specify if a non-continue-signal should be delivered to the job
 * argv[2]: specify the number of seconds this daemon must wait before exit 
 *
 */

int 
main( int argc, char* argv[] )
{
  MPIR_PROCDESC_EXT* proctab;
  int proctab_size;
  int signum;
  be_subtest_e subtest = subtest_no; 
  int i, rank,size;
  lmon_rc_e lrc;

#if SUB_ARCH_BGL || SUB_ARCH_BGP
  signum = 0;
#else
  signum = SIGCONT;
#endif 

  if ( (lrc = LMON_be_init(LMON_VERSION, &argc, &argv)) 
              != LMON_OK )
    {      
      fprintf(stdout, 
        "[LMON BE] FAILED: LMON_be_init\n");
      return EXIT_FAILURE;
    }

  if (argc > 1) 
    {
      signum = atoi(argv[1]);
      printf ("[LMON BE] signum: %d, argv[1]: %s argc(%d)\n", signum, argv[1], argc);
    }

  if ( argc > 2 )
    {
      subtest = (be_subtest_e) atoi(argv[2]);
      printf ("[LMON BE] subtest: %d, argv[2]: %s\n", subtest, argv[2]);
      if (subtest >= subtest_reserved) {
         printf ("[LMON BE] Unknown subtest. Existing...\n");
         LMON_be_finalize();
         return EXIT_FAILURE;
      }
    } 

  LMON_be_getMyRank(&rank);
  LMON_be_getSize(&size);

  if ( (lrc = LMON_be_handshake(NULL)) 
              != LMON_OK )
    {
      fprintf(stdout, 
        "[LMON BE(%d)] FAILED: LMON_be_handshake\n",
        rank );
      LMON_be_finalize();
      return EXIT_FAILURE;
    }

  if ( (lrc = LMON_be_ready(NULL)) 
              != LMON_OK )
    {     
      fprintf(stdout, 
        "[LMON BE(%d)] FAILED: LMON_be_ready\n",
        rank );
      LMON_be_finalize();
      return EXIT_FAILURE;
    } 

  //
  // This routine should only be used by test cases like this
  //
  if ( (lrc = LMON_be_tester_init ( ) ) 
              != LMON_OK )
    {      
      fprintf(stdout, 
        "[LMON BE] FAILED: LMON_be_tester_init\n");
      return EXIT_FAILURE;
    }

  if ( (lrc = LMON_be_getMyProctabSize(&proctab_size)) != LMON_OK ) 
    {
      fprintf(stdout, 
        "[LMON BE(%d)] FAILED: LMON_be_getMyProctabSize\n",
        rank );
      LMON_be_finalize();
      return EXIT_FAILURE;	
    }
  
  proctab = (MPIR_PROCDESC_EXT *) 
           malloc (proctab_size*sizeof(MPIR_PROCDESC_EXT));
  if ( proctab == NULL ) 
    {
      fprintf(stdout, 
        "[LMON BE(%d)] FAILED: malloc returned null\n",
        rank );
      LMON_be_finalize();
      return EXIT_FAILURE;
    }
 
  if ( (lrc = LMON_be_getMyProctab (proctab, 
                &proctab_size, proctab_size)) != LMON_OK )
    {    
      fprintf(stdout, 
        "[LMON BE(%d)] FAILED: LMON_be_getMyProctab\n", 
        rank );
      LMON_be_finalize();
      return EXIT_FAILURE;
    }

  for(i=0; i < proctab_size; i++)
    {
      fprintf(stdout, 
        "[LMON BE(%d)] Target process: %8d, MPI RANK: %5d\n", 
        rank,
        proctab[i].pd.pid, 
        proctab[i].mpirank);
    }

  //
  // This routine should only be used by test cases like this
  //
  per_be_data_t *myBeData = NULL;
  if ( (lrc = LMON_daemon_internal_tester_getBeData (&myBeData)) 
              != LMON_OK )
    {
      fprintf(stdout, 
        "[LMON BE(%d)] FAILED: LMON_be_internal_getBeDat returned an error\n",
        rank );
      LMON_be_finalize();
      return EXIT_FAILURE;
    }

  //
  // Unless LMON_DONT_STOP_APP condition is met
  // LMON_be_procctl_init will leave the job stopped
  //
  int fastpath_state = 2; //inherit the state

  if ( signum != 0 && signum != SIGCONT)
    {
      fastpath_state = 0;
    }

  LMON_be_procctl_init ( myBeData->rmtype_instance,
                         proctab, proctab_size, fastpath_state );

  LMON_be_procctl_run ( myBeData->rmtype_instance,
                        signum,
                        proctab, proctab_size);

  sleep(1);

  switch (subtest)
    {
    case subtest_kill_detach_shutdown:
      sleep (kill_detach_shutdown_time);
      break;

    case subtest_mw_coloc:
      {
        //
        // MW daemon co-location support
        //
        if (LMON_be_assist_mw_coloc() != LMON_OK)
          {
            fprintf(stdout,
              "[LMON BE(%d)] FAILED: LMON_be_assist_mw_coloc failed \n", rank);
            LMON_be_finalize();
            return EXIT_FAILURE;
          }

        //
        // Blocks until FE MWs are done; otherwise, COLOC will kill 
        // MW daemons prematurely
        //
        if ( (( lrc = LMON_be_recvUsrData ( NULL )) == LMON_EBDARG)
             || ( lrc == LMON_EINVAL )
             || ( lrc == LMON_ENOMEM )
             || ( lrc == LMON_ENEGCB ))
           {
             fprintf(stdout, "[LMON BE(%d)] FAILED(%d): LMON_be_recvUsrData\n",
                     rank, lrc );
             LMON_be_finalize();
             return EXIT_FAILURE;
           }

         break;
      }
    default:
        break;
    }

  for (i=0; i < proctab_size; i++)
    {
      if (proctab[i].pd.executable_name)
        free(proctab[i].pd.executable_name);
      if (proctab[i].pd.host_name)
        free(proctab[i].pd.host_name);
    }
  free (proctab);

  // sending this to mark the end of the BE session 
  // This should be used to determine PASS/FAIL criteria 
  if ( (( lrc = LMON_be_sendUsrData ( NULL )) == LMON_EBDARG)
       || ( lrc == LMON_EINVAL ) 
       || ( lrc == LMON_ENOMEM )
       || ( lrc == LMON_ENEGCB )) 
     {
       fprintf(stdout, "[LMON BE(%d)] FAILED(%d): LMON_be_sendUsrData\n",
               rank, lrc );
       LMON_be_finalize();
       return EXIT_FAILURE;
     }

  LMON_be_finalize();

  return EXIT_SUCCESS;
}

