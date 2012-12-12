/*
 * $Header: Exp $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2011 - 2012, Lawrence Livermore National Security, LLC. Produced at
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

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/lmon_api_std.h>

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX-like OS
#endif

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

#include "lmon_daemon_internal.hxx"
#include "lmon_api/lmon_proctab.h"
#include "lmon_api/lmon_say_msg.hxx"
#include "lmon_be_sync_mpi_bg.hxx"

//////////////////////////////////////////////////////////////////////////////////
//
// LAUNCHMON MPI-Tool SYNC LAYER (BlueGene L and P)
//           PUBLIC INTERFACE
//

#if SUB_ARCH_BGL || SUB_ARCH_BGP
# include "debugger_interface.h"

using namespace DebuggerInterface;

lmon_rc_e
LMON_be_procctl_init_bg ( MPIR_PROCDESC_EXT *ptab,
                          int islaunch,
                          int psize)
{
  lmon_rc_e lrc = LMON_OK;

  int i;
  for (i=0; i < psize; i++)
    {
      BG_Debugger_Msg dbgmsg ( ATTACH,ptab[i].pd.pid,0,0,0 );
      BG_Debugger_Msg ackmsg;
      BG_Debugger_Msg ackmsg2;
      dbgmsg.header.dataLength = sizeof(dbgmsg.dataArea.ATTACH);

      if ( !BG_Debugger_Msg::writeOnFd (BG_DEBUGGER_WRITE_PIPE, dbgmsg) )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "ATTACH command failed.");

          lrc = LMON_EINVAL;
          break;
        }
      if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg) )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "ATTACH_ACK failed.");

          lrc = LMON_EINVAL;
          break;
        }
      if ( ackmsg.header.messageType != ATTACH_ACK )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "readFromFd received a wrong msg type: %d.",
              ackmsg.header.messageType);

          lrc = LMON_EINVAL;
          break;
        }

      if ( ackmsg.header.nodeNumber
           != (unsigned int) ptab[i].pd.pid )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "ATTACH_ACK contains a wrong nodeNumber");

          lrc = LMON_EINVAL;
          break;
        }
    }

  if ( islaunch )
    {
      //
      // if launch mode, you don't need to worry
      // about the rest of the operations
      //
      return lrc;
    }

  for ( i=0; i < psize; i++ )
    {
      BG_Debugger_Msg dbgmsg ( KILL,ptab[i].pd.pid,0,0,0 );
      dbgmsg.dataArea.KILL.signal = SIGSTOP;
      dbgmsg.header.dataLength = sizeof(dbgmsg.dataArea.KILL);

      if ( !BG_Debugger_Msg::writeOnFd (BG_DEBUGGER_WRITE_PIPE, dbgmsg ))
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "writeOnFd for KILL.");

          return LMON_EINVAL;
        }
    }

  int kill_ack_count=0;
  int sig_encountered=0;

  for ( i=0; i < psize*2; i++ )
    {
      BG_Debugger_Msg ackmsg;

      if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg ))
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "readFromFd for KILL ACK.");

          return LMON_EINVAL;
        }

      if ( ackmsg.header.messageType != KILL_ACK 
           && ackmsg.header.messageType != SIGNAL_ENCOUNTERED )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "msg type isn't KILL ACK nor SIGNAL_ENCOUNTERED (%d)",
             ackmsg.header.messageType );

          return LMON_EINVAL;
        }

      if ( ackmsg.header.messageType == KILL_ACK )
        {
          kill_ack_count++;
        }

      if ( ackmsg.header.messageType == SIGNAL_ENCOUNTERED)
        {
          sig_encountered++;
        }
    }

  if ( ( kill_ack_count != psize ) 
       || (sig_encountered != psize ) ) 
    {

      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "lost KILL_ACK (reported: %d) or SIGNAL_ENCOUNTERED (reported: %d)", 
         kill_ack_count, sig_encountered);
        
      return LMON_EINVAL; 
    }

  return lrc;
}


lmon_rc_e
LMON_be_procctl_stop_bg ( MPIR_PROCDESC_EXT *ptab,
                          int psize )
{
  lmon_rc_e lrc = LMON_OK;
  int i;

  for ( i=0; i < psize; i++ )
    {
      BG_Debugger_Msg dbgmsg ( KILL,ptab[i].pd.pid,0,0,0 );
      dbgmsg.dataArea.KILL.signal = SIGSTOP;
      dbgmsg.header.dataLength = sizeof(dbgmsg.dataArea.KILL);
      BG_Debugger_Msg ackmsg;
      BG_Debugger_Msg ackmsg2;

      if ( !BG_Debugger_Msg::writeOnFd (BG_DEBUGGER_WRITE_PIPE, dbgmsg ))
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "writeOnFd for KILL.");
          lrc = LMON_EINVAL;
          break;
        }

      if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg ))
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "readFromFd for KILL ACK." );
          lrc = LMON_EINVAL;
          break;
        }

      if ( ackmsg.header.messageType != KILL_ACK)
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "msg type isn't KILL ACK." );
          lrc = LMON_EINVAL;
          break;
        }

      if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg2 ))
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "readFromFd for SINGANL ENCOUNTERED ACK." );
          lrc = LMON_EINVAL;
          break;
        }

      if ( ackmsg2.header.messageType != SIGNAL_ENCOUNTERED)
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "msg type isn't KILL ACK." );
          lrc = LMON_EINVAL;
          break;
        }
    }

  return lrc;
}


lmon_rc_e
LMON_be_procctl_run_bg ( int signum,
                         MPIR_PROCDESC_EXT* ptab,
                         int psize)
{
  lmon_rc_e lrc = LMON_OK;
  int sendsig = (signum == 0) ? SIGCONT : signum;
  int i;
  //
  // continue
  //
  for (i=0; i < psize; i++)
    {
      BG_Debugger_Msg dbgmsg ( CONTINUE,ptab[i].pd.pid,0,0,0 );
      BG_Debugger_Msg ackmsg;
      dbgmsg.dataArea.CONTINUE.signal = sendsig;
      dbgmsg.header.dataLength = sizeof(dbgmsg.dataArea.CONTINUE);

      if ( !BG_Debugger_Msg::writeOnFd (BG_DEBUGGER_WRITE_PIPE, dbgmsg ))
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "writeOnFd for CONTINUE." );
          lrc = LMON_EINVAL;
          break;
        }

      if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg ))
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "readFromFd for CONTINUE ACK." );

          lrc = LMON_EINVAL;
          break;
        }

      if ( ackmsg.header.messageType != CONTINUE_ACK )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "msg type isn't CONTINUE ACK." );

          lrc = LMON_EINVAL;
          break;
        }

      if ( ackmsg.header.nodeNumber
           != (unsigned int) ptab[i].pd.pid )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "Incorrect pid in the returned debug msg.");

          lrc = LMON_EINVAL;
          break;
        }
    }

  return lrc;
}


lmon_rc_e
LMON_be_procctl_perf_bg ( 
                   MPIR_PROCDESC_EXT *ptab,
                   int psize,
                   long unsigned int membase,
                   unsigned int numbytes,
                   unsigned int *fetchunit,
                   unsigned int *usecperunit)
{
  return LMON_EINVAL;
}


lmon_rc_e
LMON_be_procctl_initdone_bg ( MPIR_PROCDESC_EXT *ptab,
                              int psize)
{
  return LMON_OK;
}


lmon_rc_e
LMON_be_procctl_done_bg ( MPIR_PROCDESC_EXT *ptab,
                          int psize )
{
  lmon_rc_e lrc = LMON_OK;

  BG_Debugger_Msg dbgmsg ( VERSION_MSG,0,0,0,0 );
  BG_Debugger_Msg dbgmsg2 ( END_DEBUG,0,0,0,0 );
  BG_Debugger_Msg ackmsg;
  BG_Debugger_Msg ackmsg2;
  dbgmsg.header.dataLength = sizeof(dbgmsg.dataArea.VERSION_MSG);
  dbgmsg2.header.dataLength = sizeof(dbgmsg2.dataArea.END_DEBUG);

  if ( !BG_Debugger_Msg::writeOnFd (BG_DEBUGGER_WRITE_PIPE, dbgmsg) )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "VERSION_MSG command failed.");

      lrc = LMON_EINVAL;
      goto return_loc;
    }

  if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg) )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "VERSION_MSG_ACK failed.");

      lrc = LMON_EINVAL;
      goto return_loc;
    }

  if ( ackmsg.header.messageType != VERSION_MSG_ACK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "readFromFd received a wrong msg type: %d.",
        ackmsg.header.messageType);

      lrc = LMON_EINVAL;
      goto return_loc;
    }
  else
    {
      if ( ackmsg.dataArea.VERSION_MSG_ACK.protocolVersion >= 3 )
        {

# if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "BES: debugger protocol higher than or equal to 3." );
# endif

          if ( !BG_Debugger_Msg::writeOnFd (BG_DEBUGGER_WRITE_PIPE,
                                            dbgmsg2) )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "END_DEBUG command failed.");

              lrc = LMON_EINVAL;
              goto return_loc;
            }

          if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE,
                                             ackmsg2) )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "VERSION_MSG_ACK failed.");

              lrc = LMON_EINVAL;
              goto return_loc;
            }

          }
        else
          {
# if VERBOSE
            LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
              "BES: debugger protocol lower than or equal to 3." );
# endif
          }
    }

return_loc:
  return lrc;
}

#else // if SUB_ARCH_BGL || SUB_ARCH_BGP

lmon_rc_e
LMON_be_procctl_init_bg ( MPIR_PROCDESC_EXT *ptab,
                          int islaunch,
                          int psize)
{
   return LMON_EINVAL;
}


lmon_rc_e
LMON_be_procctl_stop_bg ( MPIR_PROCDESC_EXT *ptab,
                          int psize)
{
  return LMON_EINVAL;
}


lmon_rc_e
LMON_be_procctl_run_bg ( int signum,
                         MPIR_PROCDESC_EXT *ptab,
                         int psize)
{
  return LMON_EINVAL;
}


lmon_rc_e
LMON_be_procctl_perf_bg ( 
                   MPIR_PROCDESC_EXT *ptab,
                   int psize,
                   long unsigned int membase,
                   unsigned int numbytes,
                   unsigned int *fetchunit,
                   unsigned int *usecperunit)
{
  return LMON_EINVAL;
}

lmon_rc_e
LMON_be_procctl_initdone_bg ( MPIR_PROCDESC_EXT *ptab,
                              int psize)
{
  return LMON_EINVAL;
}


lmon_rc_e
LMON_be_procctl_done_bg ( MPIR_PROCDESC_EXT *ptab,
                          int psize)
{
  return LMON_EINVAL;
}

#endif
