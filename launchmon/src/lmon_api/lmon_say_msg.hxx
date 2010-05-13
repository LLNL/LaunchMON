/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/lmon_api/lmon_say_msg.hxx,v 1.1.2.2 2008/02/20 17:37:58 dahn Exp $
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
 *
 *  Update Log:
 *        May 11 2010 DHA: Moved gettimeofdayD here
 *        May 19 2008 DHA: Added errorCB support
 *        Feb 09 2008 DHA: Added LLNS Copyright 
 *        Mar 13 2006 DHA: Added duplex pipe class a simple
 *                         wrapper class holding on UNIX fds,
 *                         "read" and "write"
 *        Dec 29 2006 DHA: Created file.          
 */

#ifndef LMON_API_LMON_SAY_MSG_HXX
#define LMON_API_LMON_SAY_MSG_HXX 1

#if HAVE_STDARG_H
# include <cstdarg>
#else
# error stdarg.h is required
#endif

class pipe_t
{
public:
  pipe_t ();  
  pipe_t ( const int rfd, const int wfd );
  pipe_t ( const pipe_t& dp );
  ~pipe_t ();

  const int getReadingFd() const  { return readingFd; }
  const int getWritingFd() const  { return writingFd; }
  void setReadingFd(int fd)  { readingFd = fd;   }
  void setWritingFd(int fd)  { writingFd = fd;   }
  bool isReadingfdValid()  const  
       { return (readingFd == uninitializedFileDescriptor)? false : true; }
  bool isWritingfdValid()  const
       { return (writingFd == uninitializedFileDescriptor)? false : true; }
  bool isPipeValid() const
       { return (writingFd == uninitializedFileDescriptor
	      && readingFd == uninitializedFileDescriptor)? false : true; }

private:
  int readingFd;
  int writingFd;
  static const int uninitializedFileDescriptor;
};

extern int (*errorCB) (const char *format, va_list ap);
extern double gettimeofdayD ();
extern void LMON_say_msg ( const char* m, bool error_or_info, const char* output, ... );
extern void LMON_TotalView_debug ( );

#endif // LMON_API_LMON_SAY_MSG_HXX
