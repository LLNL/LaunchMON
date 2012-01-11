/*
 * $Header: Exp $
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
 *              Nov 28 2011 DHA: Added Thread Hold/Release
 *              Nov 23 2011 DHA: Added CDTI tool control messaging
 *              Oct 31 2011 DHA: File created
 *
 */

//
// TODO: 1st priority Add ControlAuthority logic for conflict case
//     : 2nd priority Make a special case for "attach" so that Control doesn't
//       have to be acquired.
//
//

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

#if HAVE_SYS_UN_H
#include <sys/un.h> 
#else
# error sys/un.h is required
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#else
# error sys/types.h is required
#endif

#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#else
# error sys/socket.h is required
#endif

#if HAVE_ERRNO_H
# include <errno.h>
#else
# error errno.h is required
#endif

#if HAVE_STRING_H
# include <string.h>
#else
# error string.h is required
#endif

#if HAVE_LIMITS_H
# include <limits.h>
#else
# error limits.h is required
#endif

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "lmon_be_internal.hxx"
#include "lmon_api/lmon_proctab.h"
#include "lmon_api/lmon_be_sync_mpi_bgq.hxx"
#include "lmon_api/lmon_say_msg.hxx"

#if SUB_ARCH_BGQ

# include "ramdisk/include/services/ToolctlMessages.h"


//////////////////////////////////////////////////////////////////////////////////
//
// File-scoped data type
//
//
//

struct CNMap
{
  std::map<int, int> CN2Sock;
  std::map<int, std::vector<int> > CN2Ranks;
};


struct SendSignalMessage
{
  bgcios::toolctl::UpdateMessage uMsg;
  bgcios::toolctl::SendSignalCmd sSigCmd;
};


struct SendSignalAckMessage
{
  bgcios::toolctl::UpdateMessage uMsg;
  bgcios::toolctl::SendSignalAckCmd sigAckCmd;
};


struct HoldThrMessage
{
  bgcios::toolctl::UpdateAckMessage uMsg;
  bgcios::toolctl::HoldThreadCmd holdThrCmd;
};


struct HoldThrAckMessage
{
  bgcios::toolctl::UpdateAckMessage uMsg;
  bgcios::toolctl::HoldThreadAckCmd holdThrAckCmd;
};


struct ReleaseThrMessage
{
  bgcios::toolctl::UpdateAckMessage uMsg;
  bgcios::toolctl::ReleaseThreadCmd releaseThrCmd;
};


struct ReleaseThrAckMessage
{
  bgcios::toolctl::UpdateAckMessage uMsg;
  bgcios::toolctl::ReleaseThreadAckCmd releaseThrAckCmd;
};


struct ContProcMessage
{
  bgcios::toolctl::UpdateMessage uMsg;
  bgcios::toolctl::SetContinuationSignalCmd sigContCmd;
  bgcios::toolctl::ContinueProcessCmd contProcCmd;
};


struct ContProcAckMessage
{
  bgcios::toolctl::UpdateAckMessage uMsg;
  bgcios::toolctl::SetContinuationSignalAckCmd sigContAckCmd;
  bgcios::toolctl::ContinueProcessAckCmd contProcCmd;
};


struct ReleaseMessage
{
  bgcios::toolctl::UpdateMessage uMsg;
  bgcios::toolctl::ReleaseControlCmd releaseCmd;
};


struct ReleaseAckMessage
{
  bgcios::toolctl::UpdateAckMessage uMsg;
  bgcios::toolctl::ReleaseControlAckCmd releaseAckCmd;
};


struct FetchMemoryMessage
{
  bgcios::toolctl::QueryMessage uMsg;
  bgcios::toolctl::GetMemoryCmd getMemCmd;
};


struct FetchMemoryAckMessage
{
  bgcios::toolctl::QueryAckMessage uMsg;
  bgcios::toolctl::GetMemoryAckCmd getMemAckCmd;
};


struct ThrListMessage
{
  bgcios::toolctl::QueryMessage uMsg;
  bgcios::toolctl::GetThreadListCmd tListCmd;
};


struct ThrListAckMessage
{
  bgcios::toolctl::QueryAckMessage uMsg;
  bgcios::toolctl::GetThreadListAckCmd tLAckCmd;
};


//////////////////////////////////////////////////////////////////////////////////
//
// Static data
//
//
//

static CNMap IOToolMap;
static uint64_t _jobid;
static uint32_t _toolid;
static uint64_t _cdtiVer;
static uint32_t _seqNum = 0;


//////////////////////////////////////////////////////////////////////////////////
//
// LAUNCHMON MPI-Tool SYNC LAYER (Blue Gene /Q)
//           INTERNAL INTERFACE
//

static void
fill_CIOS_Header ( bgcios::MessageHeader &hdr,
                 uint8_t service,
                 uint8_t version,
                 uint16_t type,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint32_t returnCode,
                 uint32_t errorCode,
                 uint32_t length,
                 uint64_t jobId )
{
  bgcios::initHeader ( &hdr );

  hdr.service = service;
  hdr.version = version;
  hdr.type = type;
  hdr.rank = rank;
  hdr.sequenceId = sequenceId;
  hdr.returnCode = returnCode;
  hdr.errorCode = errorCode;
  hdr.length = length;
  hdr.jobId = jobId;
}


static void
print_cios_header ( const bgcios::MessageHeader &hdr )
{
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
                   "[[[[MessageHeader]]]]service(%d)"
                   " version(%d)"
                   " type(%d)"
                   " rank(%d)"
                   " sequenceId(%d)"
                   " returnCode(%d)"
                   " errorCode(%d)"
                   " length(%d)"
                   " jobid(%ld)",
                   hdr.service, 
                   hdr.version, 
                   hdr.type, 
                   hdr.rank,
                   hdr.sequenceId, 
                   hdr.returnCode, 
                   hdr.errorCode, 
                   hdr.length, 
                   hdr.jobId ); 
}


static void
print_cios_toolmsg ( const bgcios::toolctl::ToolMessage *msg)
{
  LMON_say_msg (LMON_BE_MSG_PREFIX, false, "<<ToolMessage>>");
  print_cios_header ( msg->header );
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
                 "  ToolMessage: tooldId(%d)",
                 msg->toolId );

}


static void
print_cios_toolcmd (const bgcios::toolctl::ToolCommand *cmd)
{
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
                 "    <<ToolCommand>>: threadID(%d)",
                 cmd->threadID );
}


static void
print_cios_toolcmd_getthreadlist(const bgcios::toolctl::GetThreadListCmd *cmd)
{
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false, "    <GetThreadListCmd> ");
  print_cios_toolcmd ((const bgcios::toolctl::ToolCommand *)cmd);
}


static void
print_cios_toolcmd_getmemory (const bgcios::toolctl::GetMemoryCmd *cmd)
{
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false, "    <GetMemoryCmd> ");
  print_cios_toolcmd ((const bgcios::toolctl::ToolCommand *)cmd);
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false, "    addr(%d) length(%d) specaccess(%d)", cmd->addr, cmd->length, cmd->specAccess);
}


static void
print_cios_toolcmd_getthreadlistack(const bgcios::toolctl::GetThreadListAckCmd *cmd)
{
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false, "    <GetThreadListAckCmd>");
  print_cios_toolcmd ((const bgcios::toolctl::ToolCommand *)cmd);
  int i;
  for (i=0; i < cmd->numthreads; ++i)
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
                     "    GetThreadListAckCmd: threadlist[%d]: tid (%d), info.commthread(%d)",
                     i, cmd->threadlist[i].tid, cmd->threadlist[i].info.commthread);
    }
}


static void
print_cios_toolcmd_getmemoryack(const bgcios::toolctl::GetMemoryAckCmd *cmd)
{
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false, "    <GetMemoryAckCmd>");
  print_cios_toolcmd ((const bgcios::toolctl::ToolCommand *)cmd);
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
                   "    GetMemoryAckCmd: address(%x), length(%d)",
                   cmd->addr, cmd->length);
}


static void
print_cios_querymsg ( const bgcios::toolctl::QueryMessage *msg)
{
  LMON_say_msg (LMON_BE_MSG_PREFIX, false, "<QueryMessage>");
  print_cios_toolmsg ( (const bgcios::toolctl::ToolMessage *) msg);
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
                 " QueryMessage: numCommands(%d)",
                 msg->numCommands);
  int i=0;
  for (i=0; i < msg->numCommands; ++i)
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
                     " CommandDestriptor[%d] type(%d)"
                     " reserved(%d)"
                     " offset(%d)"
                     " length(%d)"
                     " returnCode(%d)",
                     i,
                     msg->cmdList[i].type,
                     msg->cmdList[i].reserved,
                     msg->cmdList[i].offset,
                     msg->cmdList[i].length,
                     msg->cmdList[i].returnCode);
      char *t;
      switch(msg->cmdList[i].type)
        {
        case bgcios::toolctl::GetThreadList:
          t = (char *)msg + msg->cmdList[i].offset;
          print_cios_toolcmd_getthreadlist ((const bgcios::toolctl::GetThreadListCmd *)t);
          break;

        case bgcios::toolctl::GetMemory:
          t = (char *)msg + msg->cmdList[i].offset;
          print_cios_toolcmd_getmemory ((const bgcios::toolctl::GetMemoryCmd *)t);
          break;

        default:
          break;
       }
    }
}


static void
print_cios_queryackmsg ( const bgcios::toolctl::QueryAckMessage *msg)
{
  LMON_say_msg (LMON_BE_MSG_PREFIX, false, "<QueryAckMessage>");
  print_cios_toolmsg ( (const bgcios::toolctl::ToolMessage *) msg);
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
                 " QueryAckMessage: numCommands(%d)",
                 msg->numCommands);
  int i=0;
  for (i=0; i < msg->numCommands; ++i)
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
                     "     CommandDestriptor[%d] type(%d)"
                     " reserved(%d)"
                     " offset(%d)"
                     " length(%d)"
                     " returnCode(%d)",
                     i,
                     msg->cmdList[i].type,
                     msg->cmdList[i].reserved,
                     msg->cmdList[i].offset,
                     msg->cmdList[i].length,
                     msg->cmdList[i].returnCode);
        char *t = NULL;
      switch(msg->cmdList[i].type)
        {
        case bgcios::toolctl::GetThreadListAck:
          t = (char *)msg + msg->cmdList[i].offset;
          print_cios_toolcmd_getthreadlistack ((const bgcios::toolctl::GetThreadListAckCmd *)t);
          break;

        case bgcios::toolctl::GetMemoryAck:
          t = (char *)msg + msg->cmdList[i].offset;
          print_cios_toolcmd_getmemoryack ((const bgcios::toolctl::GetMemoryAckCmd *)t);
          break;

        default:
          break;
        }
    }
}


static lmon_rc_e
verify_MessageHeader ( bgcios::MessageHeader &hdr,
                 uint8_t  version,
                 uint16_t type,
                 uint32_t rank,
                 uint32_t sequenceId )

{
  if ( hdr.returnCode != bgcios::Success )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "An error was returned via MessageHeader "
        "returnCode(%d) %s",
        hdr.returnCode, strerror(hdr.errorCode) );

      return LMON_EINVAL;
    }

  if ( hdr.type != type )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "A different Ack type (%d)", hdr.type );

      return LMON_EINVAL;
    }

  if ( hdr.rank != rank )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "A different rank (%d vs. %d)", hdr.rank, rank );

      return LMON_EINVAL;
    }

  if ( hdr.sequenceId != sequenceId )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "A different sequenceId (%d vs. %d)",
        hdr.sequenceId, sequenceId );

      return LMON_EINVAL;
    }

  return LMON_OK;
}


static bgcios::toolctl::AttachMessage *
create_AttachAllMessage ( uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId,
                 const char *tTag,
                 uint8_t prio )
{
  bgcios::toolctl::AttachMessage *msg
    = new bgcios::toolctl::AttachMessage ();

  fill_CIOS_Header ( msg->header,
                     bgcios::ToolctlService,
                     version,
                     bgcios::toolctl::Attach,
                     rank,
                     sequenceId,
                     bgcios::Success,
                     0, //errorCode
                     sizeof(*msg),
                     jobId);

  msg->toolId = toolId;
  snprintf ( msg->toolTag, bgcios::toolctl::ToolTagSize,
             "%s", (tTag==NULL)? "LMON" : tTag);
  msg->procSelect = bgcios::toolctl::RanksInNode;

  return msg;
}


static lmon_rc_e
verify_AttachAllAckMessage ( bgcios::toolctl::AttachAckMessage &msg,
                 uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId )
{
  return (verify_MessageHeader ( msg.header,
                                 version,
                                 bgcios::toolctl::AttachAck,
                                 rank,
                                 sequenceId ));
}


static bgcios::toolctl::DetachMessage *
create_DetachAllMessage ( uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId)
{
  bgcios::toolctl::DetachMessage *msg
    = new bgcios::toolctl::DetachMessage ();

  fill_CIOS_Header ( msg->header,
                     bgcios::ToolctlService,
                     version,
                     bgcios::toolctl::Detach,
                     rank,
                     sequenceId,
                     bgcios::Success,
                     0, // errorCode
                     sizeof(*msg),
                     jobId );

  msg->toolId = toolId;
  msg->procSelect = bgcios::toolctl::RanksInNode;

  return msg;
}


static lmon_rc_e
verify_DetachAllAckMessage ( bgcios::toolctl::DetachAckMessage &msg,
                 uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId)
{
  return ( verify_MessageHeader ( msg.header,
                                 version,
                                 bgcios::toolctl::DetachAck,
                                 rank,
                                 sequenceId ));
}


static bgcios::toolctl::ControlMessage *
create_ControlMessage ( uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId,
                 sigset_t *sigset,
                 uint32_t sndSig,
                 bgcios::toolctl::DynamicNotifyMode dNMode,
                 bgcios::toolctl::DACTrapMode dacTMode )
{
  bgcios::toolctl::ControlMessage *msg
    = new bgcios::toolctl::ControlMessage ();

  fill_CIOS_Header ( msg->header,
                     bgcios::ToolctlService,
                     version,
                     bgcios::toolctl::Control,
                     rank,
                     sequenceId,
                     bgcios::Success,
                     0, // errorCode
                     sizeof(*msg),
                     jobId);

  msg->toolId = toolId;
  msg->notifySignalSet ( sigset );
  msg->sndSignal = sndSig;
  msg->dynamicNotifyMode = dNMode;
  msg->dacTrapMode = dacTMode;

  return msg;
}


static ThrListMessage *
create_ThrListMessage ( uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId,
                 bgcios::toolctl::BG_ThreadID_t threadId)
{
  ThrListMessage *msg = new ThrListMessage ();
  msg->tListCmd.threadID = threadId;

  fill_CIOS_Header ( msg->uMsg.header,
                     bgcios::ToolctlService,
                     version,
                     bgcios::toolctl::Query,
                     rank,
                     sequenceId,
                     bgcios::Success,
                     0, // errorCode
                     sizeof ( *msg ),
                     jobId );

  msg->uMsg.toolId = toolId;
  msg->uMsg.numCommands = 1;
  msg->uMsg.cmdList[0].type = bgcios::toolctl::GetThreadList;
  msg->uMsg.cmdList[0].reserved = 0;
  msg->uMsg.cmdList[0].offset = sizeof ( msg->uMsg );
  msg->uMsg.cmdList[0].length = sizeof ( msg->tListCmd );
  msg->uMsg.cmdList[0].returnCode = bgcios::toolctl::CmdSuccess;

  return msg;
}


static lmon_rc_e
verify_ThrListAckMessage ( ThrListAckMessage &msg,
                 uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId,
                 bgcios::toolctl::BG_ThreadID_t threadId )
{
  lmon_rc_e rc = verify_MessageHeader ( msg.uMsg.header,
                                        version,
                                        bgcios::toolctl::QueryAck,
                                        rank,
                                        sequenceId );
  if ( rc != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "An error was returned in QueryAckMessage for ThrListAck");

      return LMON_EINVAL;
    }

  if ( (msg.uMsg.numCommands != 1)
       || (msg.uMsg.cmdList[0].type
          != bgcios::toolctl::GetThreadListAck)
       || (msg.uMsg.cmdList[0].returnCode
          != bgcios::toolctl::CmdSuccess) )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "numCommands (%d), "
        "type (%d), " 
        "An error was returned in QueryAckMessage(%d)",
        msg.uMsg.numCommands,
        msg.uMsg.cmdList[0].type,
        msg.uMsg.cmdList[0].returnCode );

      return LMON_EINVAL;
    }

  return LMON_OK;
}


static HoldThrMessage *
create_HoldThrMessage ( uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId,
                 bgcios::toolctl::BG_ThreadID_t threadId )
{
  HoldThrMessage *msg = new HoldThrMessage ();
  msg->holdThrCmd.threadID = threadId;

  fill_CIOS_Header ( msg->uMsg.header,
                     bgcios::ToolctlService,
                     version,
                     bgcios::toolctl::Update,
                     rank,
                     sequenceId,
                     bgcios::Success,
                     0, // errorCode
                     sizeof ( *msg ),
                     jobId );

  msg->uMsg.toolId = toolId;
  msg->uMsg.numCommands = 1;
  msg->uMsg.cmdList[0].type = bgcios::toolctl::HoldThread;
  msg->uMsg.cmdList[0].reserved = 0;
  msg->uMsg.cmdList[0].offset = sizeof ( msg->uMsg );
  msg->uMsg.cmdList[0].length = sizeof ( msg->holdThrCmd );
  msg->uMsg.cmdList[0].returnCode = bgcios::toolctl::CmdSuccess;

  return msg;
}


static lmon_rc_e
verify_HoldThrAckMessage ( HoldThrAckMessage &msg,
                 uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId,
                 bgcios::toolctl::BG_ThreadID_t threadId )
{
  lmon_rc_e rc = verify_MessageHeader ( msg.uMsg.header,
                                        version,
                                        bgcios::toolctl::UpdateAck,
                                        rank,
                                        sequenceId );
  if ( rc != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "An error was returned in UpdateAckMessage for Signal");

      return LMON_EINVAL;
    }

  if ( (msg.uMsg.numCommands != 1)
       || (msg.uMsg.cmdList[0].type
          != bgcios::toolctl::HoldThreadAck)
       || (msg.uMsg.cmdList[0].returnCode
          != bgcios::toolctl::CmdSuccess) )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "An error was returned in UpdateAckMessage(%d)",
        msg.uMsg.cmdList[0].returnCode );

      return LMON_EINVAL;
    }

  return LMON_OK;
}


static ReleaseThrMessage *
create_ReleaseThrMessage ( uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId,
                 bgcios::toolctl::BG_ThreadID_t threadId )
{
  ReleaseThrMessage *msg = new ReleaseThrMessage ();
  msg->releaseThrCmd.threadID = threadId;

  fill_CIOS_Header ( msg->uMsg.header,
                     bgcios::ToolctlService,
                     version,
                     bgcios::toolctl::Update,
                     rank,
                     sequenceId,
                     bgcios::Success,
                     0, // errorCode
                     sizeof ( *msg ),
                     jobId );

  msg->uMsg.toolId = toolId;
  msg->uMsg.numCommands = 1;
  msg->uMsg.cmdList[0].type = bgcios::toolctl::ReleaseThread;
  msg->uMsg.cmdList[0].reserved = 0;
  msg->uMsg.cmdList[0].offset = sizeof ( msg->uMsg );
  msg->uMsg.cmdList[0].length = sizeof ( msg->releaseThrCmd );
  msg->uMsg.cmdList[0].returnCode = bgcios::toolctl::CmdSuccess;

  return msg;
}


static lmon_rc_e
verify_ReleaseThrAckMessage ( ReleaseThrAckMessage &msg,
                 uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId,
                 bgcios::toolctl::BG_ThreadID_t threadId )
{
  lmon_rc_e rc = verify_MessageHeader ( msg.uMsg.header,
                                        version,
                                        bgcios::toolctl::UpdateAck,
                                        rank,
                                        sequenceId );
  if ( rc != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "An error was returned in UpdateAckMessage for Signal");

      return LMON_EINVAL;
    }

  if ( (msg.uMsg.numCommands != 1)
       || (msg.uMsg.cmdList[0].type
          != bgcios::toolctl::ReleaseThreadAck)
       || (msg.uMsg.cmdList[0].returnCode
          != bgcios::toolctl::CmdSuccess) )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "An error was returned in UpdateAckMessage(%d)",
        msg.uMsg.cmdList[0].returnCode );

      return LMON_EINVAL;
    }

  return LMON_OK;
}


static SendSignalMessage *
create_SendSignalMessage ( uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId,
                 int sndSig )
{
  SendSignalMessage *msg = new SendSignalMessage ();
  msg->sSigCmd.signum = sndSig;
  msg->sSigCmd.threadID = 0;

  fill_CIOS_Header ( msg->uMsg.header,
                     bgcios::ToolctlService,
                     version,
                     bgcios::toolctl::Update,
                     rank,
                     sequenceId,
                     bgcios::Success,
                     0, // errorCode
                     sizeof ( *msg ),
                     jobId );

  msg->uMsg.toolId = toolId;
  msg->uMsg.numCommands = 1;
  msg->uMsg.cmdList[0].type = bgcios::toolctl::SendSignal;
  msg->uMsg.cmdList[0].reserved = 0;
  msg->uMsg.cmdList[0].offset = sizeof ( msg->uMsg );
  msg->uMsg.cmdList[0].length = sizeof ( msg->sSigCmd );
  msg->uMsg.cmdList[0].returnCode = bgcios::toolctl::CmdSuccess;

  return msg;
}


static lmon_rc_e
verify_SendSignalAckMessage ( SendSignalAckMessage &msg,
                 uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId,
                 int sndSig )
{
  lmon_rc_e rc = verify_MessageHeader ( msg.uMsg.header,
                                        version,
                                        bgcios::toolctl::UpdateAck,
                                        rank,
                                        sequenceId );
  if ( rc != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "An error was returned in UpdateAckMessage for Signal");

      return LMON_EINVAL;
    }

  if ( (msg.uMsg.numCommands != 1)
       || (msg.uMsg.cmdList[0].type
          != bgcios::toolctl::SendSignalAck)
       || (msg.uMsg.cmdList[0].returnCode
          != bgcios::toolctl::CmdSuccess) )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "An error was returned in SendSignalAckMessage(%d)",
        msg.uMsg.cmdList[0].returnCode );

      return LMON_EINVAL;
    }

  return LMON_OK;
}


static ContProcMessage *
create_ContinueMessage ( uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 int signum,
                 uint64_t jobId,
                 uint32_t toolId )
{
  ContProcMessage *msg
    = new ContProcMessage ();
  msg->sigContCmd.threadID = 0;
  msg->sigContCmd.signum = signum;
  msg->contProcCmd.threadID = 0;


  fill_CIOS_Header ( msg->uMsg.header,
                     bgcios::ToolctlService,
                     version,
                     bgcios::toolctl::Update,
                     rank,
                     sequenceId,
                     bgcios::Success,
                     0, // errorCode
                     sizeof(*msg),
                     jobId );

  msg->uMsg.toolId = toolId;
  msg->uMsg.numCommands = 2;
  msg->uMsg.cmdList[0].type = bgcios::toolctl::SetContinuationSignal;
  msg->uMsg.cmdList[0].reserved = 0;
  msg->uMsg.cmdList[0].offset = sizeof ( msg->uMsg );
  msg->uMsg.cmdList[0].length = sizeof ( msg->sigContCmd ); 
  msg->uMsg.cmdList[0].returnCode = bgcios::toolctl::CmdSuccess;

  msg->uMsg.cmdList[1].type = bgcios::toolctl::ContinueProcess;
  msg->uMsg.cmdList[1].reserved = 0;
  msg->uMsg.cmdList[1].offset = msg->uMsg.cmdList[0].offset
                                + msg->uMsg.cmdList[0].length;
  msg->uMsg.cmdList[1].length = sizeof ( msg->contProcCmd ); 
  msg->uMsg.cmdList[1].returnCode = bgcios::toolctl::CmdSuccess;

  return msg;
}


static lmon_rc_e
verify_ContinueAckMessage ( ContProcAckMessage &msg,
                 uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId)
{
  lmon_rc_e rc = verify_MessageHeader ( msg.uMsg.header,
                                        version,
                                        bgcios::toolctl::UpdateAck,
                                        rank,
                                        sequenceId );
  if ( rc != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "An error was returned in UpdateAckMessage for Continue" );

      return LMON_EINVAL;
    }


  if ( (msg.uMsg.numCommands != 2)
       || (msg.uMsg.cmdList[0].type
          != bgcios::toolctl::SetContinuationSignalAck)
       || (msg.uMsg.cmdList[0].returnCode
          != bgcios::toolctl::CmdSuccess) 
       || (msg.uMsg.cmdList[1].type
          != bgcios::toolctl::ContinueProcessAck)
       || (msg.uMsg.cmdList[1].returnCode
          != bgcios::toolctl::CmdSuccess) )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
        "An error was returned in UpdateAckMessage: "
        "numCommands (%d), type (%d), returnCode (%d)"
        "type (%d), returnCode (%d)",
        msg.uMsg.numCommands,
        msg.uMsg.cmdList[0].type,
        msg.uMsg.cmdList[0].returnCode,
        msg.uMsg.cmdList[1].type,
        msg.uMsg.cmdList[1].returnCode);

     return LMON_EINVAL;
    }

  return LMON_OK;
}


static ReleaseMessage *
create_ReleaseMessage ( uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId )
{
  ReleaseMessage *msg
    = new ReleaseMessage ();

  msg->releaseCmd.threadID = 0;
  msg->releaseCmd.notify 
    = bgcios::toolctl::ReleaseControlNotify_Inactive;

  fill_CIOS_Header ( msg->uMsg.header,
                     bgcios::ToolctlService,
                     version,
                     bgcios::toolctl::Update,
                     rank,
                     sequenceId,
                     bgcios::Success,
                     0, // errorCode
                     sizeof(*msg),
                     jobId );

  msg->uMsg.toolId = toolId;
  msg->uMsg.numCommands = 1;
  msg->uMsg.cmdList[0].type = bgcios::toolctl::ReleaseControl;
  msg->uMsg.cmdList[0].reserved = 0;
  msg->uMsg.cmdList[0].offset = sizeof ( msg->uMsg );
  msg->uMsg.cmdList[0].length = sizeof ( msg->releaseCmd );
  msg->uMsg.cmdList[0].returnCode = bgcios::toolctl::CmdSuccess;

  return msg;
}


static lmon_rc_e
verify_ReleaseAckMessage ( ReleaseAckMessage &msg,
                 uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId)
{

  lmon_rc_e rc = verify_MessageHeader ( msg.uMsg.header,
                                        version,
                                        bgcios::toolctl::UpdateAck,
                                        rank,
                                        sequenceId );
  if ( rc != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "An error was returned in UpdateAckMessage for Release" );

      return LMON_EINVAL;
    }

  if ( (msg.uMsg.numCommands != 1)
       || (msg.uMsg.cmdList[0].type
          != bgcios::toolctl::ReleaseControlAck)
       || (msg.uMsg.cmdList[0].returnCode
	   != bgcios::toolctl::CmdSuccess) )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
      "An error was returned in UpdateAckMessage(%d)",
        msg.uMsg.cmdList[0].returnCode);

      return LMON_EINVAL;

    }

  return LMON_OK;
}


static FetchMemoryMessage *
create_FetchMemoryMessage (uint8_t version,
                 uint32_t rank,
                 uint32_t sequenceId,
                 uint64_t jobId,
                 uint32_t toolId,
                 bgcios::toolctl::BG_Addr_t addr,
                 int length)
{
  FetchMemoryMessage *msg = new FetchMemoryMessage ();
  msg->getMemCmd.threadID = 0;
  msg->getMemCmd.addr = addr;
  msg->getMemCmd.length = length;
  msg->getMemCmd.specAccess = bgcios::toolctl::SpecAccess_ForceNonSpeculative;

  fill_CIOS_Header ( msg->uMsg.header,
                     bgcios::ToolctlService,
                     version,
                     bgcios::toolctl::Query,
                     rank,
                     sequenceId,
                     bgcios::Success,
                     0, // errorCode
                     sizeof(*msg),
                     jobId );

  msg->uMsg.toolId = toolId;
  msg->uMsg.numCommands = 1;
  msg->uMsg.cmdList[0].type = bgcios::toolctl::GetMemory;
  msg->uMsg.cmdList[0].reserved = 0;
  msg->uMsg.cmdList[0].offset = sizeof ( msg->uMsg );
  msg->uMsg.cmdList[0].length = sizeof ( msg->getMemCmd);
  msg->uMsg.cmdList[0].returnCode = bgcios::toolctl::CmdSuccess;

  return msg;
}



static lmon_rc_e
open_domain_socket ()
{
  lmon_rc_e rc = LMON_EINVAL;

  std::map<int, std::vector<int> >::iterator i;

  for (i = IOToolMap.CN2Ranks.begin ();
           i != IOToolMap.CN2Ranks.end (); ++i)
    {
      sockaddr_un saddr;
      int result;
      socklen_t socklen = sizeof ( sockaddr_un );
      int fd  = socket ( PF_UNIX, SOCK_STREAM, 0 );
      if ( fd == -1 )
       {
          int error = errno;
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "Error creating socket: %s.",
            strerror(error) );

          goto return_loc;
       }

      bzero ( &saddr, socklen );
      saddr.sun_family = AF_UNIX;
#if 0
      result = snprintf ( saddr.sun_path, sizeof ( saddr.sun_path ),
        "/jobs/%lu/toolctl_node/%d",
        _jobid, 262144);
#else
      result = snprintf ( saddr.sun_path, sizeof ( saddr.sun_path ),
        "/jobs/%lu/toolctl_node/%d",
        _jobid, i->first);
#endif

      if (result >= sizeof ( saddr.sun_path ))
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "addr.sun_path truncated");

          goto return_loc;
        }

#if VERBOSE
      char cmd[1024];
      sprintf(cmd, "ls -l /jobs/%lu/toolctl_node/", _jobid);
      system(cmd);
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
        "socket has been created for %s ", (char *) saddr.sun_path );
#endif

      if ( connect ( fd, (struct sockaddr *) &saddr, socklen ) == -1 )
        {
          int error = errno;
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "Error connecting to a Unix domain socket: %s.",
            strerror(error) );

          goto return_loc;
        }

#if VERBOSE
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
        "connect to %s", saddr.sun_path );
#endif

      IOToolMap.CN2Sock[i->first] = fd;
    }

  rc = LMON_OK;

return_loc:
  return rc;
}


static lmon_rc_e
cleanup_resources ()
{
  std::map<int, int>::iterator i;
  
  for (i = IOToolMap.CN2Sock.begin ();
           i != IOToolMap.CN2Sock.end (); ++i)
    {
      if (close(i->second) == -1)
        {
          return LMON_EINVAL;
        }
    }

  IOToolMap.CN2Sock.clear();
  IOToolMap.CN2Ranks.clear();

  return LMON_OK;
}


static int
read_cios_longmsg ( int fd, void *msg, int buflen, int hdrlen,
                    bgcios::toolctl::CommandDescriptor *cmdDesc )
{
  int length;
  char *msgPtr = (char *) msg;

  if ( lmon_read_raw ( fd, (void *)msgPtr, hdrlen ) != hdrlen )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "lmon_read_raw did not read "
        "the matching byte count for header" );

      return -1;
    }

  length = cmdDesc->length;
  if ( length > (buflen - hdrlen) )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "length of the payload greater than the buffer" );

      return -1;
    }

  msgPtr += hdrlen;
  if ( lmon_read_raw (fd, (void *)msgPtr, length) != length )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "lmon_read_raw did not read "
        "the matching byte count for the payload" );

       return -1; 
    }

  return length + hdrlen;
}


static lmon_rc_e
attach_all ( uint8_t cdtiVer, uint32_t &sequenceId,
                 uint64_t jobId, uint32_t toolId )
{
  lmon_rc_e rc = LMON_EINVAL;

  std::map<int, int>::iterator i;
  for (i = IOToolMap.CN2Sock.begin();
           i != IOToolMap.CN2Sock.end(); ++i)
    {
      bgcios::toolctl::AttachMessage *attMsg = NULL;
      bgcios::toolctl::AttachAckMessage ackMsg;


      attMsg 
	= create_AttachAllMessage ( cdtiVer, i->first,
				    sequenceId, jobId, toolId,
				    "LMON", 1 /* priority */ );

      if (lmon_write_raw ( i->second, (void *) attMsg,
                           sizeof(*attMsg)) == -1)
        {
          LMON_say_msg(LMON_BE_MSG_PREFIX, true,
            "Error writing to a Unix domain socket");

          goto return_loc;
        }

#if VERBOSE
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
        "Sent AttachMessage to CN(%d)", i->first );
#endif

      if (lmon_read_raw ( i->second,
                          (void *) &ackMsg,
                          sizeof(ackMsg)) == -1)
        {
          LMON_say_msg(LMON_BE_MSG_PREFIX, true,
            "Error reading from a Unix domain socket");

          goto return_loc;
        }

#if VERBOSE
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
        "Recv AttachAckMessage to CN(%d)", i->first );
#endif

      if ( verify_AttachAllAckMessage ( ackMsg,
                                     cdtiVer,
                                     i->first,
                                     sequenceId,
                                     jobId,
                                     toolId ) != LMON_OK )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "verify_AttachAllMessage reports a failure" );

          goto return_loc;
        }

      delete attMsg;
      attMsg = NULL;
      sequenceId++;
    }
  
  rc = LMON_OK;
return_loc:
  return rc;
}


static lmon_rc_e
detach_all ( uint8_t cdtiVer, uint32_t &sequenceId,
                 uint64_t jobId, uint32_t toolId )
{
  lmon_rc_e rc = LMON_EINVAL;

  std::map<int, int>::iterator i;

  for (i = IOToolMap.CN2Sock.begin ();
                   i != IOToolMap.CN2Sock.end (); ++i)
    {
      bgcios::toolctl::DetachMessage *detMsg = NULL;
      bgcios::toolctl::DetachAckMessage ackMsg;
      detMsg = create_DetachAllMessage ( cdtiVer, i->first,
                                         sequenceId, jobId, toolId );

      if ( lmon_write_raw ( i->second, (void *) detMsg,
                            sizeof(*detMsg)) == -1 )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "Error writing to a Unix domain socket" );

          goto return_loc;
        }

#if VERBOSE
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
        "Sent DetachMessage to CN(%d)", i->first );
#endif

      if ( lmon_read_raw ( i->second, (void *) &ackMsg,
                           sizeof(ackMsg)) == -1 )
        {
          LMON_say_msg(LMON_BE_MSG_PREFIX, true,
            "Error reading from a Unix domain socket");

          goto return_loc;
        }

#if VERBOSE
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
        "Recv DetachAckMessage to CN(%d)", i->first );
#endif

      if ( verify_DetachAllAckMessage ( ackMsg,
                                        cdtiVer,
                                        i->first,
                                        sequenceId,
                                        jobId,
                                        toolId ) != LMON_OK )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "verify_DetachAllAckMessage reports a failure" );

          goto return_loc;
        }

      delete detMsg;
      detMsg = NULL;
      sequenceId++;
    }

  rc = LMON_OK;

return_loc:
  return rc;
}


static lmon_rc_e
hold_all_threads ( uint8_t cdtiVer, uint32_t &sequenceId,
                 uint64_t jobId, uint32_t toolId )
{
  lmon_rc_e rc = LMON_EINVAL;

  std::map<int, std::vector<int> >::iterator cn;

  for (cn = IOToolMap.CN2Ranks.begin();
          cn != IOToolMap.CN2Ranks.end(); ++cn)
    {
      std::vector<int>::iterator i;
      for (i = cn->second.begin(); i != cn->second.end(); ++i)
        {
          ThrListMessage *tListMsg = NULL;
          ThrListAckMessage ackMsg;

          tListMsg = create_ThrListMessage( cdtiVer,
                                            (uint32_t)(*i),
                                            sequenceId,
                                            jobId,
                                            toolId,
                                            0 /* directed to the dlft thread */ );

          if ( lmon_write_raw ( IOToolMap.CN2Sock[cn->first],
                                 (void *) tListMsg,
                                 sizeof(*tListMsg)) == -1 )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "Error writing to a Unix domain socket" );

              goto return_loc;
            }

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Sent QueryMessage to CN(%d) rank(%d)"
            " for ThrListMessage",
            cn->first, *i );
          print_cios_querymsg (&(tListMsg->uMsg));
#endif

          if ( read_cios_longmsg ( IOToolMap.CN2Sock[cn->first],
                                   &ackMsg, sizeof (ackMsg),
                                   sizeof (ackMsg.uMsg), 
                                   ackMsg.uMsg.cmdList) == -1 )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "Error reading from a Unix domain socket" );

              goto return_loc;
            }

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Recv QueryAckMessage from CN(%d) rank(%d)"
            " for ThrListAckMessage",
            cn->first, *i);
          print_cios_queryackmsg (&(ackMsg.uMsg));
#endif

          if ( verify_ThrListAckMessage ( ackMsg,
                                          cdtiVer,
                                          (uint32_t)(*i),
                                          sequenceId,
                                          jobId,
                                          toolId, 0) != LMON_OK )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "verify_ThrListAckMessage reports a failure");

              goto return_loc;
            }

          delete tListMsg;
          tListMsg = NULL;
          sequenceId++;

          int tx;
          for (tx=0; tx < ackMsg.tLAckCmd.numthreads; tx++)
            {
              HoldThrMessage *tHoldMsg = new HoldThrMessage ();
              HoldThrAckMessage tHoldAckMsg;
              bgcios::toolctl::BG_ThreadID_t threadId
                = (bgcios::toolctl::BG_ThreadID_t)
                  ackMsg.tLAckCmd.threadlist[tx].tid;

              tHoldMsg = create_HoldThrMessage ( cdtiVer,
                                                 (uint32_t)(*i),
                                                 sequenceId,
                                                 jobId,
                                                 toolId,
                                                 threadId );

              if ( lmon_write_raw ( IOToolMap.CN2Sock[cn->first],
                                    (void *) tHoldMsg,
                                    sizeof (*tHoldMsg)) == -1 )
                {
                  LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                    "Error writing to a Unix domain socket" );

                  goto return_loc;
                }

              if ( lmon_read_raw ( IOToolMap.CN2Sock[cn->first],
                                   (void *) &tHoldAckMsg,
                                   sizeof(tHoldAckMsg)) == -1 )
                {
                  LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                    "Error reading from a Unix domain socket" );

                  goto return_loc;
                }

              if ( verify_HoldThrAckMessage ( tHoldAckMsg,
                                              cdtiVer,
                                              (uint32_t)(*i),
                                              sequenceId,
                                              jobId,
                                              toolId,
                                              threadId ) != LMON_OK )
                {
                  LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                    "verify_HoldThrAckMessage reports a failure");

                  goto return_loc;
                }

              delete tHoldMsg;
              tHoldMsg = NULL;
              sequenceId++;
            } // for each thread

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Held all threads in CN(%d) rank(%d)",
            cn->first, *i );
#endif

        } // for each rank

    } // for each compute node

  rc = LMON_OK;

return_loc:
  return rc;
}


static lmon_rc_e
release_all_threads ( uint8_t cdtiVer, uint32_t &sequenceId,
                 uint64_t jobId, uint32_t toolId )
{
  lmon_rc_e rc = LMON_EINVAL;

  std::map<int, std::vector<int> >::iterator cn;

  for (cn = IOToolMap.CN2Ranks.begin();
          cn != IOToolMap.CN2Ranks.end(); ++cn)
    {
      std::vector<int>::iterator i;
      for (i = cn->second.begin(); i != cn->second.end(); ++i)
        {
          ThrListMessage *tListMsg = NULL;
          ThrListAckMessage ackMsg;

          tListMsg = create_ThrListMessage( cdtiVer,
                                            (uint32_t)(*i),
                                            sequenceId,
                                            jobId,
                                            toolId,
                                            0 /* directed to the dlft thread */ );

          if ( lmon_write_raw ( IOToolMap.CN2Sock[cn->first],
                                (void *) tListMsg,
                                sizeof(*tListMsg)) == -1 )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "Error writing to a Unix domain socket" );

              goto return_loc;
            }

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Sent QueryMessage to CN(%d) rank(%d)"
            " for ThrListMessage",
            cn->first, *i );
#endif

          if ( lmon_read_raw ( IOToolMap.CN2Sock[cn->first],
                               (void *) &ackMsg,
                               sizeof(ackMsg)) == -1 )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "Error reading from a Unix domain socket" );

              goto return_loc;
            }

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Recv QueryAckMessage from CN(%d) rank(%d)"
            " for ThrListAckMessage",
            cn->first, *i );
#endif

          if ( verify_ThrListAckMessage ( ackMsg,
                                          cdtiVer,
                                          (uint32_t)(*i),
                                          sequenceId,
                                          jobId,
                                          toolId, 0) != LMON_OK )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "verify_ThrListAckMessage reports a failure");

              goto return_loc;
            }

          delete tListMsg;
          tListMsg = NULL;
          sequenceId++;

          int tx;
          for (tx=0; tx < ackMsg.tLAckCmd.numthreads; tx++)
            {
              ReleaseThrMessage *tReleaseMsg = new ReleaseThrMessage ();
              ReleaseThrAckMessage tReleaseAckMsg;
              bgcios::toolctl::BG_ThreadID_t threadId
                = (bgcios::toolctl::BG_ThreadID_t)
                  ackMsg.tLAckCmd.threadlist[tx].tid;

              tReleaseMsg = create_ReleaseThrMessage ( cdtiVer,
                                                       (uint32_t)(*i),
                                                       sequenceId,
                                                       jobId,
                                                       toolId,
                                                       threadId );

              if ( lmon_write_raw ( IOToolMap.CN2Sock[cn->first],
                                    (void *) tReleaseMsg,
                                    sizeof (*tReleaseMsg)) == -1 )
                {
                  LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                    "Error writing to a Unix domain socket" );

                  goto return_loc;
                }

              if ( lmon_read_raw ( IOToolMap.CN2Sock[cn->first],
                                   (void *) &tReleaseAckMsg,
                                   sizeof(tReleaseAckMsg)) == -1 )
                {
                  LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                    "Error reading from a Unix domain socket" );

                  goto return_loc;
                }

              if ( verify_ReleaseThrAckMessage ( tReleaseAckMsg,
                                                 cdtiVer,
                                                 (uint32_t)(*i),
                                                 sequenceId,
                                                 jobId,
                                                 toolId,
                                                 threadId ) != LMON_OK )
                {
                  LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                    "verify_ReleaseThrAckMessage reports a failure");

                  goto return_loc;
                }

              delete tReleaseMsg;
              tReleaseMsg = NULL;
              sequenceId++;
            } // for each thread

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Released all threads in CN(%d) rank(%d)",
            cn->first, *i );
#endif

        } // for each rank

    } // for each compute node

  rc = LMON_OK;

return_loc:
  return rc;
}


static lmon_rc_e
acquire_control_authority ( uint8_t cdtiVer,
                 uint32_t &sequenceId,
                 uint64_t jobId, uint32_t toolId )
{
  lmon_rc_e rc = LMON_EINVAL;

  std::map<int, std::vector<int> >::iterator cn;

  for ( cn = IOToolMap.CN2Ranks.begin();
             cn != IOToolMap.CN2Ranks.end(); ++cn )
    {
      std::vector<int>::iterator i;
      for (i = cn->second.begin(); i != cn->second.end(); ++i)
        {
          bgcios::toolctl::ControlMessage *cntlMsg = NULL;
          bgcios::toolctl::ControlAckMessage ackMsg;
          bgcios::toolctl::NotifyMessage notifyMsg;

          sigset_t interestSig;
          sigemptyset (&interestSig);
          sigaddset (&interestSig, SIGSTOP);

          cntlMsg = create_ControlMessage ( cdtiVer,
                      (uint32_t)(*i),
                      sequenceId,
                      jobId,
                      toolId,
                      &interestSig,
                      0,
                      bgcios::toolctl::DynamicNotifyDLoader,
                      bgcios::toolctl::DACTrap_NoChange );

          if ( lmon_write_raw ( IOToolMap.CN2Sock[cn->first],
                                (void *) cntlMsg,
                                sizeof (*cntlMsg) ) == -1 )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "Error writing to a Unix domain socket" );

              goto return_loc;
            }

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Sent ControlMessage to CN(%d) rank(%d)",
            cn->first, *i );
#endif

          if ( lmon_read_raw ( IOToolMap.CN2Sock[cn->first],
                               (void *) &ackMsg,
                               sizeof (ackMsg) ) == -1)
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "Error reading from a Unix domain socket" );

              goto return_loc;
            }

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Recv ControlAckMessage for CN(%d) rank(%d)",
            cn->first, *i );
#endif

	  //
	  // TODO:
	  // Control Authority Arbitration is Needed here
	  //
	  //

          if ( verify_MessageHeader ( ackMsg.header,
                                      cdtiVer,
                                      bgcios::toolctl::ControlAck,
                                      *i,
                                      sequenceId ) != LMON_OK )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "An error was returned in ControlAckMessage" );

              goto return_loc;
            }

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "ControlAckMessage::controllingToolId(%d), "
            "ControlAckMessage::toolTag(%c %c %c), "
            "ControlAckMessage::priority(%d),",
            ackMsg.controllingToolId,
            ackMsg.toolTag[0],
            ackMsg.toolTag[1],
            ackMsg.toolTag[2],
            ackMsg.priority );
#endif 
          delete cntlMsg;
          cntlMsg = NULL;
          sequenceId++;
    }
  }

  //
  // This can be racy! But this should only affect testers
  //   And other realworld tool client will have a logic
  //   to handle overlapping message between Ack and Notify   
  // 
  if ( release_all_threads ( _cdtiVer, _seqNum, _jobid, _toolid ) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "release_all_threads returned an error code.");

      return LMON_EINVAL;
    }

  rc = LMON_OK;

return_loc:
  return rc;
}


static lmon_rc_e
stop_all ( uint8_t cdtiVer, uint32_t &sequenceId,
                 uint64_t jobId, uint32_t toolId )
{
  lmon_rc_e rc = LMON_EINVAL;

  std::map<int, std::vector<int> >::iterator cn;

  for ( cn = IOToolMap.CN2Ranks.begin ();
                    cn != IOToolMap.CN2Ranks.end (); ++cn )
    {
      std::vector<int>::iterator i;
      for (i = cn->second.begin ();
                      i != cn->second.end (); ++i)
	{
	  SendSignalMessage *stopMsg = NULL;
	  SendSignalAckMessage ackMsg;
	  bgcios::toolctl::NotifyMessage notifyMsg;

	  stopMsg = create_SendSignalMessage ( cdtiVer,
		      (uint32_t)(*i),
		      sequenceId,
		      jobId,
		      toolId,
                      SIGSTOP );

	  if ( lmon_write_raw ( IOToolMap.CN2Sock[cn->first],
                               (void *) stopMsg,
			       sizeof (*stopMsg) ) == -1 )
	    {
	      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "Error writing to a Unix domain socket" );

	      goto return_loc;
	    }

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Sent UpdateMessage to CN(%d) rank(%d) for stop signal",
            cn->first, *i );
#endif
	 
	  if ( lmon_read_raw ( IOToolMap.CN2Sock[cn->first],
                               (void *) &ackMsg,
			       sizeof (ackMsg) ) == -1 )
	    {
	      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
		"Error reading from a Unix domain socket" );

	      goto return_loc;
	    }

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Recv UpdateAckMessage from CN(%d) rank(%d) for stop signal",
            cn->first, *i );
#endif

          if ( verify_SendSignalAckMessage ( ackMsg,
					     cdtiVer,
		                             (uint32_t)(*i),
		                             sequenceId,
		                             jobId,
		                             toolId,
                                             SIGSTOP) != LMON_OK )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "verify_SendSignalAckMessage reports a failure");

              goto return_loc;
            }

	  if ( lmon_read_raw ( IOToolMap.CN2Sock[cn->first], (void *) &notifyMsg,
			       sizeof(notifyMsg)) == -1 )
	    {
	      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
		"Error reading from a Unix domain socket");

	      goto return_loc;
	    }

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Recv NotifyMessage from CN(%d) rank(%d) for stop signal",
            cn->first, *i );
#endif

	  if ( notifyMsg.header.returnCode != bgcios::Success )
	    {
	      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
		 "An error was returned in AttachMessageAck (%d)",
		 notifyMsg.header.returnCode);

	      goto return_loc;
	    }

	  if ( notifyMsg.notifyMessageType 
               != bgcios::toolctl::NotifyMessageType_Signal )
	    {
	      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
		"NotifyMessageType_Signal wasn't delivered (%d)",
		notifyMsg.notifyMessageType);

	      goto return_loc;
	    }

	  if ( notifyMsg.type.signal.signum != SIGSTOP )
	    {
	      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
		"Notified signal isn't SIGSTOP!");

	      goto return_loc;
	    }

	  delete stopMsg;
	  stopMsg = NULL;
	  sequenceId++;
	}
    }

  rc = LMON_OK;

return_loc:
  return rc;
}


static lmon_rc_e
fetch_mem_all ( uint8_t cdtiVer, uint32_t &sequenceId,
                uint64_t jobId, uint32_t toolId, 
                long unsigned int membase,
                unsigned int numbytes,
                unsigned int *fetchunit,
                unsigned int *usecperunit )
{
  lmon_rc_e rc = LMON_EINVAL;

  std::map<int, std::vector<int> >::iterator cn;
  int totalIter = 0;
  double start_ts, end_ts, accum=0.0;
  *fetchunit = 4096;
  char memAckBuffer[bgcios::toolctl::MaxMemorySize];
  char *mabPtr;
  memset((void*)memAckBuffer, '\0', bgcios::toolctl::MaxMemorySize);

  for (cn = IOToolMap.CN2Ranks.begin();
            cn != IOToolMap.CN2Ranks.end(); ++cn)
    {
      std::vector<int>::iterator i;
      for (i = cn->second.begin(); i != cn->second.end(); ++i)
        {
          int nIter = numbytes/(*fetchunit);
          int j;
          bgcios::toolctl::BG_Addr_t trav 
            = (bgcios::toolctl::BG_Addr_t) membase;
          for (j=0; j < nIter; ++j)
            {  
              FetchMemoryMessage *fetchMemMsg = NULL; 
              bgcios::toolctl::QueryAckMessage ackMsg;

              fetchMemMsg = create_FetchMemoryMessage ( cdtiVer,
		      (uint32_t)(*i),
		      sequenceId,
		      jobId,
		      toolId,
                      trav,
                      (*fetchunit));

              start_ts = gettimeofdayD();
	      if ( lmon_write_raw ( IOToolMap.CN2Sock[cn->first],
                                    (void *) fetchMemMsg,
		    	            sizeof(*fetchMemMsg)) == -1 )
	        {
	          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                    "Error writing to a Unix domain socket" );

	          goto return_loc;
	        }
	      if ( lmon_read_raw ( IOToolMap.CN2Sock[cn->first],
                                   (void *) &ackMsg,
		    	           sizeof(ackMsg)) == -1 )
	        {
	          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
		    "Error reading from a Unix domain socket" );

	          goto return_loc;
	        }
              mabPtr = memAckBuffer;
	      if ( lmon_read_raw ( IOToolMap.CN2Sock[cn->first],
                                   (void *) memAckBuffer,
		    	           ackMsg.cmdList[0].length) == -1 )
	        {
	          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
		    "Error reading from a Unix domain socket" );

	          goto return_loc;
	        }

              end_ts = gettimeofdayD();
              accum += (end_ts - start_ts);
              totalIter++;
              trav += (*fetchunit); 

              delete fetchMemMsg;
              fetchMemMsg = NULL;
              sequenceId++;
            }
         
          if ((j * (*fetchunit)) < numbytes)
            {
              int len = numbytes - (j * (*fetchunit));
              FetchMemoryMessage *fetchMemMsg = NULL; 
              bgcios::toolctl::QueryAckMessage ackMsg;

              fetchMemMsg = create_FetchMemoryMessage ( cdtiVer,
		      (uint32_t)(*i),
		      sequenceId,
		      jobId,
		      toolId,
                      trav,
                      (*fetchunit));
              start_ts = gettimeofdayD ();
	      if ( lmon_write_raw ( IOToolMap.CN2Sock[cn->first],
                                    (void *) fetchMemMsg,
		    	            sizeof(*fetchMemMsg)) == -1 )
	        {
	          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                    "Error writing to a Unix domain socket" );

	          goto return_loc;
	        }
	      if ( lmon_read_raw ( IOToolMap.CN2Sock[cn->first],
                                   (void *) &ackMsg,
		    	           sizeof(ackMsg)) == -1 )
	        {
	          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
		    "Error reading from a Unix domain socket" );

	          goto return_loc;
	        }
              mabPtr = memAckBuffer;
	      if ( lmon_read_raw ( IOToolMap.CN2Sock[cn->first],
                                   (void *) memAckBuffer,
		    	           ackMsg.cmdList[0].length) == -1 )
	        {
	          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
		    "Error reading from a Unix domain socket" );

	          goto return_loc;
	        }
                end_ts = gettimeofdayD();
                accum += (end_ts - start_ts);
                totalIter++;

                delete fetchMemMsg;
                fetchMemMsg = NULL;
                sequenceId++;
            }
        }
     } 
  
  (*usecperunit) = (unsigned int) (accum * 1000000.0) / totalIter;  

#if VERBOSE
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
                   "MEMFETCH PERF: %d usec to fetch %d %d bytes",
                   (int) (accum * 1000000), totalIter, *fetchunit); 
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
                   "MEMFETCH PERF: %d usec per %d bytes",
                   *usecperunit, *fetchunit); 
#endif

  rc = LMON_OK;

return_loc:
  return rc;  
}


static lmon_rc_e
continue_all ( uint8_t cdtiVer, uint32_t &sequenceId,
                 int signum,
                 uint64_t jobId, uint32_t toolId )
{
  lmon_rc_e rc = LMON_EINVAL;

  std::map<int, std::vector<int> >::iterator cn;

  for (cn = IOToolMap.CN2Ranks.begin();
            cn != IOToolMap.CN2Ranks.end(); ++cn)
    {
      std::vector<int>::iterator i;
      for (i = cn->second.begin(); i != cn->second.end(); ++i)
	{
	  ContProcMessage *contMsg = NULL;
	  ContProcAckMessage ackMsg;

	  contMsg = create_ContinueMessage ( cdtiVer,
		      (uint32_t)(*i),
		      sequenceId,
                      signum,
		      jobId,
		      toolId );

	  if ( lmon_write_raw ( IOToolMap.CN2Sock[cn->first],
                                (void *) contMsg,
			        sizeof(*contMsg)) == -1 )
	    {
	      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "Error writing to a Unix domain socket" );

	      goto return_loc;
	    }
	 
#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Sent ContProcMessage to CN(%d) rank(%d) for continue",
            cn->first, *i );
#endif

	  if ( lmon_read_raw ( IOToolMap.CN2Sock[cn->first],
                               (void *) &ackMsg,
			       sizeof(ackMsg)) == -1 )
	    {
	      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
		"Error reading from a Unix domain socket" );

	      goto return_loc;
	    }

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Recv ContProcAckMessage from CN(%d) rank(%d) for continue",
            cn->first, *i );
#endif

          if ( verify_ContinueAckMessage ( ackMsg,
                                           cdtiVer,
		                           (uint32_t)(*i),
                                           sequenceId,
		                           jobId,
		                           toolId) != LMON_OK )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "verify_ContinueAckMessage reports a failure");

              goto return_loc;
            }

	  delete contMsg;
	  contMsg = NULL;
	  sequenceId++;
	}
    }

  rc = LMON_OK;

return_loc:
  return rc;
}




static lmon_rc_e
release_control_all(uint8_t cdtiVer, uint32_t &sequenceId,
                 uint64_t jobId, uint32_t toolId)
{
  lmon_rc_e rc = LMON_EINVAL;

  std::map<int, std::vector<int> >::iterator cn;

  for (cn = IOToolMap.CN2Ranks.begin();
                    cn != IOToolMap.CN2Ranks.end(); ++cn)
    {
      std::vector<int>::iterator i;
      for (i = cn->second.begin();
                    i != cn->second.end(); ++i)
	{
	  ReleaseMessage *releaseMsg = NULL;
	  ReleaseAckMessage ackMsg;

	  releaseMsg = create_ReleaseMessage ( cdtiVer,
		         (uint32_t)(*i),
		         sequenceId,
		         jobId,
		         toolId );

	  if ( lmon_write_raw ( IOToolMap.CN2Sock[cn->first],
                                (void *) releaseMsg,
			        sizeof(*releaseMsg)) == -1 )
	    {
	      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "Error writing to a Unix domain socket" );

	      goto return_loc;
	    }

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Sent ReleaseMessage to CN(%d) rank(%d) for releaseControl",
            cn->first, *i );
#endif
 
	  if ( lmon_read_raw ( IOToolMap.CN2Sock[cn->first],
                               (void *) &ackMsg,
			       sizeof ( ackMsg ) ) == -1 )
	    {
	      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
		"Error reading from a Unix domain socket");

	      goto return_loc;
	    }

#if VERBOSE
          LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
            "Recv ReleaseAckMessage from CN(%d) rank(%d) for releaseControl",
            cn->first, *i );
#endif

          if ( verify_ReleaseAckMessage ( ackMsg,
                                          cdtiVer,
		                          (uint32_t)(*i),
		                          sequenceId,
		                          jobId,
		                          toolId ) != LMON_OK ) 
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "verify_ReleaseAckMessage reports a failure");

              goto return_loc;
            }

	  delete releaseMsg;
	  releaseMsg = NULL;
	  sequenceId++;
	}
    }

  rc = LMON_OK;

return_loc:
  return rc;
}


//////////////////////////////////////////////////////////////////////////////////
//
// LAUNCHMON MPI-Tool SYNC LAYER (BlueGene Q)
//           PUBLIC INTERFACE
//

lmon_rc_e
LMON_be_procctl_init_bgq ( MPIR_PROCDESC_EXT *ptab,
                           int psize )
{
  int i;
  char *jobidStr;
  char *toolidStr;
  std::string cdtiVerPath;

  //
  // Fetch jobid, toolid, cdti version
  //
  jobidStr = getenv("BG_JOBID");
  toolidStr = getenv("BG_TOOLID");
  if ( !jobidStr || !toolidStr )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "empty BG_JOBID or BG_TOOLID.");

      return LMON_EINVAL;
    }

  _jobid = (uint64_t) strtoll ( jobidStr, NULL, 10 );
  _toolid = (uint32_t) strtol ( toolidStr, NULL, 10 );
  cdtiVerPath = std::string ("/jobs/" )
    + std::string ( jobidStr )
    + std::string ( "/tools/protocol" );

  std::ifstream infile;
  infile.open (cdtiVerPath.c_str(), std::ifstream::in);
  std::stringstream ssBuf;
  ssBuf << infile.rdbuf ();
  _cdtiVer = (int) strtol ( ssBuf.str().c_str(), NULL, 10 );
  infile.close ();

  //
  // Fill in IOToolMap structure
  //
  std::map<int, std::vector<int> >::iterator iter;
  for ( i=0; i < psize; i++ )
    {
      iter = IOToolMap.CN2Ranks.find(ptab[i].cnodeid);
      if (iter != IOToolMap.CN2Ranks.end())
	{
	  iter->second.push_back ( ptab[i].pd.pid );
	}
      else
	{
	  std::vector<int> pidVector;
	  pidVector.push_back ( ptab[i].pd.pid );
	  IOToolMap.CN2Ranks[ptab[i].cnodeid] = pidVector;
	}
    }

#if VERBOSE
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
    "_cdtiVer(%d), _seqNum(%d), _jobid(%d), _toolid(%d) ",
    _cdtiVer, _seqNum, _jobid, _toolid);
#endif

  //
  // Open domain socket 
  //
  if ( open_domain_socket () != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "open_domain_socket returned an error code.");

      return LMON_EINVAL;
    }

  //
  // Attach all ranks
  //
  if ( attach_all ( _cdtiVer, _seqNum,
		    _jobid, _toolid ) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "attach_all returned an error code.");

      return LMON_EINVAL;
    }

  //
  // Acquire control authority
  //
  if ( acquire_control_authority ( _cdtiVer, _seqNum,
				   _jobid, _toolid ) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "attach_all returned an error code." );

      return LMON_EINVAL;
    }

  //
  // Stop all processes
  //
  if ( stop_all ( _cdtiVer, _seqNum, _jobid, _toolid) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "stop_all returned an error code.");

      return LMON_EINVAL;
    }

#if 0
  if ( hold_all_threads ( _cdtiVer, _seqNum, _jobid, _toolid ) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "hold_all_threads returned an error code.");

      return LMON_EINVAL;
    }
#endif

  return LMON_OK;
}


lmon_rc_e
LMON_be_procctl_stop_bgq ( MPIR_PROCDESC_EXT *ptab,
                           int psize)
{
  if (stop_all ( _cdtiVer, _seqNum, _jobid, _toolid) != LMON_OK)
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "stop_all returned an error code.");

      return LMON_EINVAL;
    }

  return LMON_OK;
}


lmon_rc_e
LMON_be_procctl_run_bgq ( int signum,
                          MPIR_PROCDESC_EXT *ptab,
                          int psize)
{
  if ( continue_all ( _cdtiVer, _seqNum, signum, _jobid, _toolid) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "continue_all returned an error code.");

      return LMON_EINVAL;
    }

  return LMON_OK;
}


lmon_rc_e
LMON_be_procctl_perf_bgq (
                   MPIR_PROCDESC_EXT *ptab,
                   int psize,
                   long unsigned int membase,
                   unsigned int numbytes,
                   unsigned int *fetchunit,
                   unsigned int *usecperunit)
{
  if ( fetch_mem_all ( _cdtiVer, _seqNum, _jobid, _toolid,
                       membase, numbytes, fetchunit, usecperunit) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "fetch_mem_all returned an error code.");

      return LMON_EINVAL;
    }

  return LMON_OK;
}


lmon_rc_e
LMON_be_procctl_initdone_bgq ( MPIR_PROCDESC_EXT *ptab,
                               int psize )
{
  if ( hold_all_threads ( _cdtiVer, _seqNum, _jobid, _toolid ) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "hold_all_threads returned an error code.");

      return LMON_EINVAL;
    }

  if ( release_control_all ( _cdtiVer, _seqNum, _jobid, _toolid ) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "release_control_all returned an error code.");

      return LMON_EINVAL;
    }

  if ( detach_all ( _cdtiVer, _seqNum, _jobid, _toolid ) != LMON_OK)
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "detach_all returned an error code.");
      return LMON_EINVAL;
    }

  if ( cleanup_resources ( ) != LMON_OK)
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "cleanup_resources returned an error code.");

      return LMON_EINVAL;
    }

  return LMON_OK;

}

lmon_rc_e
LMON_be_procctl_done_bgq ( MPIR_PROCDESC_EXT *ptab,
                          int psize)
{
  if ( cleanup_resources ( ) != LMON_OK)
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "cleanup_resources returned an error code.");

      return LMON_EINVAL;
    }

  return LMON_OK;
}


#else // if SUB_ARCH_BGQ

lmon_rc_e
LMON_be_procctl_init_bgq ( MPIR_PROCDESC_EXT *ptab,
                           int psize)
{
   return LMON_EINVAL;
}


lmon_rc_e
LMON_be_procctl_stop_bgq ( MPIR_PROCDESC_EXT *ptab,
                           int psize)
{
  return LMON_EINVAL;
}


lmon_rc_e
LMON_be_procctl_run_bgq ( MPIR_PROCDESC_EXT *ptab,
                          int psize)
{
  return LMON_EINVAL;
}


lmon_rc_e
LMON_be_procctl_initdone_bgq ( MPIR_PROCDESC_EXT *ptab,
                               int psize)
{
  return LMON_EINVAL;
}

lmon_rc_e
LMON_be_procctl_done_bgq(MPIR_PROCDESC_EXT *ptab,
                         int psize)
{
  return LMON_EINVAL;
}

#endif

