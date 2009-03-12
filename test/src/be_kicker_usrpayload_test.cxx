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
 *        Mar 04 2008 DHA: Added generic BlueGene support
 *        Jun 12 2008 DHA: Added GNU build system support
 *        Feb 09 2008 DHA: Added LLNS Copyright.
 *        Aug 06 2007 DHA: Created file.
 *          
 */

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

#include <map>

#include <lmon_api/lmon_be.h>

#if RM_BG_MPIRUN
# include "debugger_interface.h"
  using namespace DebuggerInterface;
#endif

std::map<std::string, int> usrdata;  // this will have "hostname," "port" pair  
std::map<std::string, int> recvdata; // this will have "hostname," "port" pair echoed back from BE


static int 
fill_usrdata ( )
{
  using namespace std;

  int i;
  
  //
  // 128 is an arbitrary number; i.e. emulating 
  // 128 ip/port pair vector
  //
  for ( i=0; i < 128; ++i )
    {
      char hnconcat[128];

      if ( i < 0 )
        break;

      if ( i < 10 )
        {
          sprintf ( hnconcat, 
            "BEIP00%d.llnl.gov", i); // faking a hostname 
        }
      else if ( i < 100 )
        {
          sprintf ( hnconcat, 
            "BEIP0%d.llnl.gov", i); // faking a hostname
        }
      else if ( i < 1000 )
        {
          sprintf ( hnconcat, 
            "BEIP%d.llnl.gov", i);  // faking a hostname
        }

      usrdata[string(hnconcat)] = i+1024; // faking a port 
    }

  return 0;
}


static int 
packbefe_cb ( void* udata, 
              void* msgbuf, 
              int msgbufmax, 
              int* msgbuflen )
{
  using namespace std;
  
  map<string, int>* u = ( map<string, int>* ) udata; 
  map<string, int>& myudata =  *u;
  map<string, int>::const_iterator iter; 
  char* trav  = (char*) msgbuf;
  int usedbuf = 0;
  int trun = 0;

  if ( ( msgbuf == NULL ) || ( msgbufmax < 0) )
    return -1; 

  for ( iter = myudata.begin(); iter != myudata.end(); iter++)
    {
      int accum_len = usedbuf + iter->first.size() + + sizeof(int);
      if ( accum_len > msgbufmax )
        {
          trun = 1; 
          break;
        } 

      memcpy ( (void*) trav, iter->first.c_str(), iter->first.size()+1); 
      trav += iter->first.size()+1; 
      memcpy ( (void*) trav, (void*) &(iter->second), sizeof (int) );
      trav += sizeof (int); 
      usedbuf += iter->first.size()+1+sizeof (int);
    }

   (*msgbuflen) = usedbuf;

   return ((trun == 0) ? 0 : -1);
}


static int
unpackfebe_cb  ( void* udatabuf, 
		 int udatabuflen, 
		 void* udata )
{
  using namespace std;

  map<string, int>* u = ( map<string, int>* ) udata; 
  map<string, int>& myudata = *u;
  char* trav = (char*) udatabuf;
  int usedbuf = 0;

  while ( usedbuf < udatabuflen )
    {
      myudata[string(trav)] = (int) *(trav+strlen(trav)+1);
      usedbuf += strlen(trav)+1+sizeof(int);
      trav += strlen(trav)+1+sizeof(int);
    }
  
  return 0;    
}


int 
main( int argc, char* argv[] )
{
  MPIR_PROCDESC_EXT *proctab;
  int proctab_size;
  int signum;
  int i, rank, size;
  lmon_rc_e lrc;

#if RM_BG_MPIRUN
  signum = 0;
#else
  signum = SIGCONT;
#endif

  if ( (lrc = LMON_be_init(LMON_VERSION, &argc, &argv)) 
              != LMON_OK )
    {      
      fprintf(stdout, 
        "[LMON BE: FAILED] LMON_be_init\n");
      return EXIT_FAILURE;
    }

  if (argc > 1) 
     signum = atoi(argv[1]);

  fill_usrdata ();

  LMON_be_getMyRank (&rank);
  LMON_be_getSize (&size);

  if ( ( lrc = LMON_be_regPackForBeToFe (packbefe_cb ))  
              != LMON_OK )
    {
      fprintf (stdout,
	"[LMON BE(%d)] LMON_be_regPackForBeToFe FAILED\n", 
        rank);
      return EXIT_FAILURE;
    } 

  if ( ( lrc = LMON_be_regUnpackForFeToBe (  
                unpackfebe_cb ))  
              != LMON_OK )
    {
       fprintf (stdout,
        "[LMON BE(%d)] LMON_be_regUnpackForFeToBe FAILED\n",
        rank);
       return EXIT_FAILURE;
    } 

  if ( (lrc = LMON_be_handshake((void*) &recvdata) ) 
              != LMON_OK )
    {
      fprintf(stdout, 
        "[LMON BE(%d): FAILED] LMON_be_handshake\n",
        rank );
      LMON_be_finalize();
      return EXIT_FAILURE;
    }

  if ( (lrc = LMON_be_ready((void*) &usrdata)) 
              != LMON_OK )
    {     
      fprintf(stdout, 
        "[LMON BE(%d): FAILED] LMON_be_ready\n",
        rank);
      LMON_be_finalize();
      return EXIT_FAILURE;
    } 

    if ( (lrc = LMON_be_getMyProctabSize(&proctab_size)) 
                != LMON_OK )
    {
      fprintf(stdout,
        "[LMON BE(%d)] FAILED: LMON_be_getMyProctabSize\n",
        rank);
      LMON_be_finalize();
      return EXIT_FAILURE;
    }

  proctab = (MPIR_PROCDESC_EXT *) 
            malloc (proctab_size*sizeof(MPIR_PROCDESC_EXT));
  if ( proctab == NULL )
    {
      fprintf (stdout, 
               "[LMON BE(%d): FAILED] malloc return null\n",
               rank);
      LMON_be_finalize();
      return EXIT_FAILURE;
    }

  if ( (lrc = LMON_be_getMyProctab(proctab, &proctab_size, proctab_size)) 
              != LMON_OK )
    {    
      fprintf(stdout, 
        "[LMON BE(%d): FAILED] LMON_be_getMyProctab\n", 
        rank );
      LMON_be_finalize();
      return EXIT_FAILURE;
    }

  if ( LMON_be_amIMaster() == LMON_YES )
    {
      if ( ( lrc = LMON_be_recvUsrData ( (void*) &recvdata )) 
                   != LMON_OK ) 
        {
          fprintf(stdout, 
            "[LMON BE(%d): FAILED] LMON_be_recvUsrData\n",
            rank );

          LMON_be_finalize();

          return EXIT_FAILURE;
        }

      if ( ( lrc = LMON_be_sendUsrData ( (void*) &usrdata )) 
                   != LMON_OK ) 
        {
          fprintf(stdout, 
            "[LMON BE(%d): FAILED] LMON_be_sendUsrData\n",
            rank );

          LMON_be_finalize();

          return EXIT_FAILURE;
        }
    }
 
  for(i=0; i < proctab_size; i++)
    {
      fprintf(stdout, 
        "[LMON BE(%d)] Target process: %8d, MPI RANK: %5d\n", 
        rank,
        proctab[i].pd.pid, 
        proctab[i].mpirank);
    }

#if RM_BG_MPIRUN
  /* the tool wants to send a signal other than the default */
  if (signum != 0)
    {
      for (i=0; i < proctab_size; i++)
        {
          BG_Debugger_Msg dbgmsg(KILL,proctab[i].pd.pid,0,0,0);
          dbgmsg.dataArea.KILL.signal = SIGSTOP;
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
         }
#else
  for(i=0; i < proctab_size; i++)
    {
#if 0
      printf("[LMON BE(%d)] kill %d, %d\n", rank, proctab[i].pd.pid, signum );
#endif
      kill(proctab[i].pd.pid, signum);
    }
#endif

  free (proctab);

  /* sending this to mark the end of the BE session */
  /* This should be used to determine PASS/FAIL criteria */
  if ( (( lrc = LMON_be_sendUsrData ( NULL )) == LMON_EBDARG)
       || ( lrc == LMON_EINVAL )
       || ( lrc == LMON_ENOMEM ))
     {
       fprintf(stdout, "[LMON BE(%d)] FAILED(%d): LMON_be_sendUsrData\n",
               rank, lrc );
       LMON_be_finalize();
       return EXIT_FAILURE;
     }

  LMON_be_finalize();

  return EXIT_SUCCESS;
}
