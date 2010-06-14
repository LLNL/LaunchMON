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
 *        Jun 12 2008 DHA: Added GNU build system support. 
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Aug 15 2006 DHA: File created
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/common.h>

#if TIME_WITH_SYS_TIME
# include <time.h>
# include <sys/time.h>
#else
# error ctime and sys_time.h is required
#endif

static struct timeval st, et;

int 
begin_timer ()
{
  return (gettimeofday ( &st, NULL ));
}


int 
time_stamp ( const char* description )
{
  int rc;

  if ( ! (timerisset (&st)) )
    return -1;

  rc = gettimeofday ( &et, NULL ); 

  if ( et.tv_usec >= st.tv_usec ) 
    {
      fprintf ( stdout, 
		"%s: %ld (%ld - %ld) seconds %ld (%ld - %ld) usec \n",
		description, 
		(et.tv_sec - st.tv_sec), et.tv_sec, st.tv_sec,
		(et.tv_usec - st.tv_usec), et.tv_usec, st.tv_usec);
    }
  else
    {
      fprintf ( stdout, 
		"%s: %ld (%ld - %ld) seconds %ld (%ld - %ld) usec \n",
		description,
		( ( et.tv_sec - 1 ) - st.tv_sec), et.tv_sec, st.tv_sec,
		( (et.tv_usec+1000000) - st.tv_usec), et.tv_usec, st.tv_usec );
    } 

  return rc;
}
