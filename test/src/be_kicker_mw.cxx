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
 *        Aug 04 2010 DHA: Copied most from be_kicker.cxx.
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

#if RM_BG_MPIRUN
# include "debugger_interface.h"
  using namespace DebuggerInterface;
#endif

/*
 * Multi-purpose kicker daemon
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
  int mw_coloc_test; 
  int i, rank,size;
  lmon_rc_e lrc;

#if RM_BG_MPIRUN
  signum = 0;
#else
  signum = SIGCONT;
#endif

  mw_coloc_test = 0;

  if ( (lrc = LMON_be_init(LMON_VERSION, &argc, &argv)) 
              != LMON_OK )
    {
      fprintf(stderr,
        "[LMON BE] FAILED: LMON_be_init\n");
      return EXIT_FAILURE;
    }

  if (argc > 1)
    {
      signum = atoi(argv[1]);
      fprintf (stderr, "[LMON BE] signum: %d, argv[1]: %s\n", signum, argv[1]);
    }

  if ( argc > 2 )
    {
      mw_coloc_test = atoi(argv[2]);
    }

  LMON_be_getMyRank(&rank);
  LMON_be_getSize(&size);

  if ( (lrc = LMON_be_handshake(NULL)) 
              != LMON_OK )
    {
      fprintf(stderr,
        "[LMON BE(%d)] FAILED: LMON_be_handshake\n",
        rank );
      LMON_be_finalize();
      return EXIT_FAILURE;
    }

  if ( (lrc = LMON_be_ready(NULL)) 
              != LMON_OK )
    {
      fprintf(stderr,
        "[LMON BE(%d)] FAILED: LMON_be_ready\n",
        rank );
      LMON_be_finalize();
      return EXIT_FAILURE;
    } 

  if ( (lrc = LMON_be_getMyProctabSize(&proctab_size)) != LMON_OK ) 
    {
      fprintf(stderr, 
        "[LMON BE(%d)] FAILED: LMON_be_getMyProctabSize\n",
        rank );
      LMON_be_finalize();
      return EXIT_FAILURE;	
    }

  proctab = (MPIR_PROCDESC_EXT *) 
           malloc (proctab_size*sizeof(MPIR_PROCDESC_EXT));
  if ( proctab == NULL ) 
    {
      fprintf(stderr,
        "[LMON BE(%d)] FAILED: malloc returned null\n",
        rank );
      LMON_be_finalize();
      return EXIT_FAILURE;
    }

  if ( (lrc = LMON_be_getMyProctab (proctab, &proctab_size, proctab_size)) 
              != LMON_OK )
    {
      fprintf(stderr,
        "[LMON BE(%d)] FAILED: LMON_be_getMyProctab\n", 
        rank );
      LMON_be_finalize();
      return EXIT_FAILURE;
    }

  for(i=0; i < proctab_size; i++)
    {
      fprintf(stderr,
        "[LMON BE(%d)] Target process: %8d, MPI RANK: %5d\n", 
        rank,
        proctab[i].pd.pid,
        proctab[i].mpirank);
    }

#if RM_BG_MPIRUN
  // the tool wants to send a signal other than the default 
  if (signum != 0)  
    {
      for (i=0; i < proctab_size; i++)
        {
          BG_Debugger_Msg dbgmsg(KILL,proctab[i].pd.pid,0,0,0);
          dbgmsg.dataArea.KILL.signal = SIGSTOP;
          dbgmsg.header.dataLength = sizeof(dbgmsg.dataArea.KILL);
          BG_Debugger_Msg ackmsg;	  
          BG_Debugger_Msg ackmsg2;	  

          if ( !BG_Debugger_Msg::writeOnFd (BG_DEBUGGER_WRITE_PIPE, dbgmsg ))
            {
	      fprintf(stdout, 
	        "[LMON BE(%d)] FAILED: KILL command.\n", rank);
	        return EXIT_FAILURE;
	    }
          if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg )) 
	    {
	      fprintf(stdout, 
	        "[LMON BE(%d)] FAILED: KILL_ACK command.\n", rank);
	        return EXIT_FAILURE;
	    }
          if ( ackmsg.header.messageType != KILL_ACK)
	    {
	      fprintf(stdout, 
		  "[LMON BE(%d)] FAILED: readFromFd received a wrong msg type: %d.\n", 
	  	    rank,ackmsg.header.messageType);
	      return EXIT_FAILURE;
	    }
          if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg2 )) 
	    {
	      fprintf(stdout, 
	        "[LMON BE(%d)] FAILED: SIGNAL_ENCOUNTERED command.\n", rank);
	      return EXIT_FAILURE;
	    }
          if ( ackmsg2.header.messageType != SIGNAL_ENCOUNTERED)
	    {
	      fprintf(stdout, 
		  "[LMON BE(%d)] FAILED: readFromFd received a wrong msg type: %d.\n", 
	  	    rank,ackmsg.header.messageType);
	      return EXIT_FAILURE;
	    }
        }
    } 

    for (i=0; i < proctab_size; i++)
      {
          BG_Debugger_Msg dbgmsg(CONTINUE,proctab[i].pd.pid,0,0,0);
          BG_Debugger_Msg ackmsg;	  
          dbgmsg.dataArea.CONTINUE.signal = signum;
          dbgmsg.header.dataLength = sizeof(dbgmsg.dataArea.CONTINUE);

          if ( !BG_Debugger_Msg::writeOnFd (BG_DEBUGGER_WRITE_PIPE, dbgmsg ))
            {
              fprintf(stdout, 
                "[LMON BE(%d)] FAILED: CONTINUE command.\n", rank);
                return EXIT_FAILURE;
            }
          if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg )) 
            {
              fprintf(stdout, 
                "[LMON BE(%d)] FAILED: CONTINUE_ACK command.\n", rank);
                return EXIT_FAILURE;
            }
          if ( ackmsg.header.messageType != CONTINUE_ACK)
            {
              fprintf(stdout, 
          	  "[LMON BE(%d)] FAILED: readFromFd received a wrong msg type: %d.\n", 
        	    rank,ackmsg.header.messageType);
              return EXIT_FAILURE;
            }
          if ( ackmsg.header.nodeNumber != (unsigned int) proctab[i].pd.pid )
            {
              fprintf(stdout, 
          	  "[LMON BE(%d)] FAILED: the CONTINUE_ACK msg contains a wrong nodeNumber.\n", rank);
              return EXIT_FAILURE;
            }
          if (proctab[i].pd.executable_name)
            {
              free(proctab[i].pd.executable_name);
              proctab[i].pd.executable_name = NULL;
            }
          if (proctab[i].pd.host_name)
            {
              free(proctab[i].pd.host_name);
              proctab[i].pd.host_name = NULL;
            }
      }

#else
  for(i=0; i < proctab_size; i++)
    {
      kill(proctab[i].pd.pid, signum);
      if (proctab[i].pd.executable_name)
        {
          free(proctab[i].pd.executable_name);
          proctab[i].pd.executable_name = NULL;
        }
      if (proctab[i].pd.host_name)
        {
          free(proctab[i].pd.host_name);
          proctab[i].pd.host_name = NULL;
        }
    }
#endif

  free (proctab);

  if ( mw_coloc_test != 0)
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

