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
 *        Mar 04 2009 DHA: Deprecated function definitions in favor of
 *                         ones in debugger_interface.h 
 *        Mar 21 2008 DHA: Created file.
 *
 */

#include <lmon_api/common.h>

#include <iostream>

#if HAVE_UNISTD_H
# include <unistd.h>
#else
# error unistd.h is required
#endif 

#if HAVE_ERRNO_H
# include <errno.h>
#else
# error errno.h is required
#endif

#include "debugger_interface.h"

#if 0
using namespace DebuggerInterface;

static uint32_t seqnum = 0;


bool 
BG_Debugger_Msg::writeOnFd (int fd, BGL_Debugger_Msg &msg)
{
  int rc;

  msg.header.sequence = seqnum;

  if ( (rc = write(fd, &msg.header, sizeof(msg.header))) < 0 ) 
     {
       fprintf(stdout, "[CIOD DEBUG INTERFACE]  writeOnFd -- writing header failed \n");       
       return false;
     }

  if ( msg.header.dataLength )
    {  
      if ( (rc = write(fd, &msg.dataArea, msg.header.dataLength)) < 0 )
	{
	  fprintf(stdout, "[CIOD DEBUG INTERFACE]  writeOnFd -- writing data failed \n");       
	  return false;
	}
    }

   ++seqnum;
   return true;
}


bool 
BG_Debugger_Msg::readFromFd (int fd, BGL_Debugger_Msg &msg)
{
   int rc;

   if ( (rc =  read(fd, &msg.header, sizeof(msg.header))) < 0 )
     {
       fprintf(stderr, "[CIOD DEBUG INTERFACE]  readFromFd -- reading header failed \n");       
       return false;
     }

   if ( msg.header.dataLength )
     {
       if ( (rc = read(fd, &msg.dataArea, msg.header.dataLength)) < 0)
	 {
	   fprintf(stderr, "[CIOD DEBUG INTERFACE]  readFromFd -- reading data failed \n");       
	  return false;

	 }
     }
   
   return true;
}
#endif
