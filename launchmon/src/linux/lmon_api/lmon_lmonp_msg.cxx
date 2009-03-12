/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/lmon_api/lmon_lmonp_msg.cxx,v 1.6.2.4 2008/03/06 00:13:57 dahn Exp $
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
 *        Mar 05 2008 DHA: Added timedaccept support 
 *        Mar 05 2008 DHA: Rewrote lmon_write_raw and lmon_read_raw to enhance 
 *                         readibility.
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Dec 19 2006 DHA: Created file.          
 */

#include <lmon_api/lmon_api_std.h>

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#else
# error unistd.h is required
#endif

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

#if HAVE_ERRNO_H
#include <errno.h>
#else
# error errno.h is required
#endif

#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#else
# error sys/socket.h is required
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#else
# error sys/types.h is required
#endif

#include <lmon_api/lmon_lmonp_msg.h>

#define LMONP_MSG_OP "[LMONP MSG]"


////////////////////////////////////////////////////////////////////
//
// static data
//
//
static std::string iamusedby("Not Set");


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (of module: lmon_lmonp_msg)
//
//

//! ssize_t lmon_write_raw 
/*!
    a wrapper for the write call. 
*/
ssize_t 
lmon_write_raw ( int fd, void *buf, size_t count )
{
  ssize_t writeN;
  ssize_t global_writeN = 0;
  char* write_completely = (char*) buf;
  int errflag = 0;

  if (!buf)
    return -1;

  for ( ; ; )
    {

      writeN = write ( fd,
                       (void*)write_completely,
                       count-global_writeN );

      if (  writeN < 0 )
        {
          if ( ( errno == EINTR ) || (errno == EAGAIN)) 
            {
              continue;
            } 
          else 
            {
              // if an error other than EINTR or EAGAIN
              // occurs, we should probably want to return; 
              errflag = 1;
              break;
            }
        } 

      global_writeN += writeN;
      if ( global_writeN == (ssize_t)count )
        {
          // if we reached to our goal (count) 
          // we want to return;
          break;
        }
      else if ( global_writeN > (ssize_t)count ) 
        {
          // This should never happen 
          //
          errflag = 1;
          break;
        }
   
      //
      // otherwise, we want to advance the temporary butter pointer
      // in order to keep writing to the file descriptor.  
      //
      write_completely += writeN;
    }

  return ( (errflag==0) ? global_writeN : -1 );

}


//! ssize_t lmon_read_raw ( int fd, const void *buf, size_t count )
/*!
    a wrapper for the read call. 
*/
ssize_t 
lmon_read_raw ( int fd, void *buf, size_t count )
{
  ssize_t readN;
  ssize_t global_readN = 0;
  char* read_completely = (char*) buf; 
  int errflag = 0;

  if (!buf)
    return -1;

  for ( ; ; )
    {
      readN = read ( fd,
                     (void*) read_completely, 
                     count-global_readN );

      if ( readN == 0 )
        {
          errflag = 1; /* in the context of sockets API, this means connection lost */
          break;
        }
      else if (  readN < 0 )
        {
          if ( ( errno == EINTR ) || (errno == EAGAIN)) 
            {
              continue;
            } 
          else 
            {
              // if an error other than EINTR or EAGAIN
              // occurs, we should probably want to return; 
              errflag = 1;
              break;
            }
        } 

      global_readN += readN;
      if ( global_readN == (ssize_t)count )
        {
          // if we reached to our goal (count) 
          // we want to return;
          break;
        }
      else if ( global_readN > (ssize_t)count ) 
        {
          // This should never happen 
          //
          errflag = 1;
          break;
        }
 
      read_completely += readN;
    } 

  return ( (errflag==0) ? global_readN : -1 );
}

//! int lmon_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
/*!
    a wrapper for the accept call. 
*/
int 
lmon_accept ( int s, struct sockaddr *addr, socklen_t *addrlen )
{
  int connfd = -1;

  for ( ; ; )
    {
      connfd  = accept ( s, addr, addrlen );
      
      //
      // If s is not marked with O_NONBLOCK and no connection 
      // arrive, above accept will block. Make sure to mark
      // the socket with O_NONBLOCK. 
      //

      if (connfd < 0 )
        {
          if ( (errno == EAGAIN)
               || (errno == EWOULDBLOCK)
               || (errno == EINTR)) 
            {
              //
              // if accept returned because of those errors 
              // we want to call that again.
              //
              continue; 
            }
        }
      
      break;
    }

  return connfd;
}


//! int lmon_timed_accept
/*!
    -1: other errors
    -2: timed out
                                                                                                        
*/
int
lmon_timedaccept ( int s, struct sockaddr *addr,
                   socklen_t *addrlen, int toutsec )
{
  struct timeval to;
  fd_set readset;
  int nready = 0;
                                                                                                        
  if (toutsec < 0 )
    return -1;
                                                                                                        
  FD_ZERO (&readset);
  FD_SET (s, &readset);
                                                                                                        
  to.tv_sec = toutsec;
  to.tv_usec = 0;
                                                                                                        
  nready = select (s+1, &readset, NULL, NULL, &to);
                                                                                                        
  if ( nready < 0 )
    {
      return -1;
    }
  else if ( nready == 0 )
    {
      //
      // timed out...
      //
      return -2;
    }
                                                                                                        
 return ( lmon_accept (s, addr, addrlen) );
}


//! void set_client_name ( const char* cn );
/*!
    Which spatial component is using this lmonp message? 
*/
void 
set_client_name(const char* cn)
{
  iamusedby = cn;
}


//! print_msg_header ( my_lmon_kind_e rw, lmonp_t* msg );
/*!
    print_msg_header: debug routine, printing message type and comm parties
*/
int 
print_msg_header ( my_lmon_kind_e k, lmonp_t* msg )
{
  using namespace std;

  string kind;
  bool sanitychk = true;

  switch (k)
    {
    case lmon_fe_driver:
      kind = "[lmon_fe_driver]: ";
      break;

    case lmon_fe_watchdog:
      kind = "[lmon_fe_watchdog]: ";
      break;

    case lmon_fe:
      kind = "[lmon_fe]:";
      break;  

    case lmon_be:
      kind = "[lmon_be]:";
      break;

    case lmon_mw:
      kind = "[lmon_mw]:";
      break;

    default:
      sanitychk = false;
      break;
    }

  if (!sanitychk)
    {
      //
      // This kind is not known
      //
      return -1;   
    }
  
  if ( iamusedby != string("Not Set"))
    {
       cout << "[" << iamusedby << "]"<< kind 
            << "msg->msgclass: " << msg->msgclass 
	    << endl;
       cout << "[" << iamusedby << "]"<< kind 
            << "msg->type.fetofe_type: " << msg->type.fetofe_type 
	    << endl;
       cout << "[" << iamusedby << "]"<< kind 
            << "msg->lmon_payload_length: " << msg->lmon_payload_length 
	    << endl;
       cout << "[" << iamusedby << "]"<< kind 
            << "msg->usr_payload_length: " << msg->usr_payload_length 
	    << endl;
    }
  else
   { 
       cout << kind << "msg->msgclass: " << msg->msgclass 
	    << endl;
       cout << kind << "msg->type.fetofe_type: " << msg->type.fetofe_type 
	    << endl;
       cout << kind << "msg->lmon_payload_length: " << msg->lmon_payload_length 
	    << endl;
       cout << kind << "msg->usr_payload_length: " << msg->usr_payload_length 
	    << endl;  
   }

  return 0;
}


//! init_msg_header ( lmonp_t* msg );
/*! 
  init_msg_header: zero outthe header portion of lmonp message
  0 on success, -1 on error
*/
int 
init_msg_header ( lmonp_t* msg )
{
  if ( memset(msg,0,sizeof(*msg)) == NULL )
    {
      return -1;
    }
  return 0;
}


//! write_lmonp_long_msg ( int fd, lmonp_t* msg, int msglength )
/*! 
  The functions looks at the header of msg before shipping the 
  entire msg via fd
*/
int 
write_lmonp_long_msg ( int fd, lmonp_t* msg, int msglength )
{
  using namespace std;

  int write_byte;
  int msgsize = sizeof ( (*msg) )
                + msg->lmon_payload_length 
                + msg->usr_payload_length;

  if (msgsize != msglength)
    {
      cerr << LMONP_MSG_OP
           << "message size mismatch"
           << endl;

      return -1; 
    }

  write_byte = lmon_write_raw ( fd, msg, msgsize );

  return write_byte;
}


//! int read_lmonp_msgheader ( int fd, lmonp_t* msg )
/*! 
  The functions reads only the header portion of an
  lmonp message via fd.
*/
int 
read_lmonp_msgheader ( int fd, lmonp_t* msg )
{
  int read_byte;
  
  read_byte = lmon_read_raw ( fd, msg, sizeof(*msg));

  return read_byte;
}


//! int read_lmonp_payloads ( int fd, void* buf, int length )
/*! 
  The functions reads the payloads portion of an
  lmonp message via fd.
*/
int 
read_lmonp_payloads ( int fd, void* buf, int length )
{
  int read_byte;

  read_byte = lmon_read_raw ( fd, buf, length );

  return read_byte;
}


//! int set_msg_header (lmonp_t* msg, ...
/*! 
  a helper function setting the lmonp header 

typedef struct _lmonp_t {
  lmonp_msg_class_e msgclass            : 3;

  union u{
    lmonp_fe_to_fe_msg_e fetofe_type    : 13;
    lmonp_fe_to_be_msg_e fetobe_type    : 13;
    lmonp_fe_to_mw_msg_e fetomw_type  : 13;    
  } type;   

  union u1{
    unsigned short security_key1        : 16;
    unsigned short num_tasks            : 16;
  } sec_or_jobsizeinfo;

  union u2{
    unsigned int security_key2          : 32; 
    struct unique_str {
      unsigned short num_exec_name      : 16;
      unsigned short num_host_name      : 16;
    } exec_and_hn;
  } sec_or_stringinfo;   

  unsigned int lmon_payload_length      : 32;
  unsigned int usr_payload_length       : 32;

} lmonp_t;


*/
extern "C" 
int 
set_msg_header (lmonp_t* msg, 
		lmonp_msg_class_e mc, 
		int type, 
		unsigned short seckey_or_numtasks, 
		unsigned int security_key2,
		unsigned short num_exec_name,
		unsigned short num_host_name,
		unsigned int lmonlen, 
		unsigned int usrlen )
{
  if (!msg)
    return -1;

  msg->msgclass = mc;

  switch (mc)
    {
    case lmonp_fetofe:
      msg->type.fetofe_type = (lmonp_fe_to_fe_msg_e) type;
      break;

    case lmonp_fetobe:
      msg->type.fetobe_type = (lmonp_fe_to_be_msg_e) type;
      break;
      
    case lmonp_fetomw:
      msg->type.fetomw_type = (lmonp_fe_to_mw_msg_e) type;
      break;

    default:
      return -1;     
    }

  msg->sec_or_jobsizeinfo.security_key1 = seckey_or_numtasks;

  if (security_key2)    
    msg->sec_or_stringinfo.security_key2 = security_key2;
  else
    {
      msg->sec_or_stringinfo.exec_and_hn.num_exec_name = num_exec_name;
      msg->sec_or_stringinfo.exec_and_hn.num_host_name = num_host_name;
    }

  msg->lmon_payload_length = lmonlen;
  msg->usr_payload_length = usrlen;
  
  return 0;
}


char* 
get_lmonpayload_begin ( lmonp_t *msg )
{
  char* ret = (char *) msg;
  if (!msg)
    return NULL;

  if ( msg->lmon_payload_length <= 0 )
    return NULL;

  ret += sizeof (*msg);

  return ret; 
}


char* 
get_usrpayload_begin ( lmonp_t *msg )
{
  char* ret = (char *) msg;
  if ( !msg )
    return NULL;

  if ( msg->usr_payload_length <= 0 ) 
    return NULL;

  ret = ret + sizeof (*msg) + msg->lmon_payload_length;

  return ret;
}


char* 
get_strtab_begin ( lmonp_t *msg ) 
{ 
  char *ret = NULL;  
  if ( msg->msgclass == lmonp_fetobe )
  {
    if ( msg->type.fetobe_type == lmonp_febe_proctab )
    {
      ret = get_lmonpayload_begin ( msg ); 
      if (ret)
        ret += msg->sec_or_jobsizeinfo.num_tasks*sizeof(int)*4;
    } 
    else if ( msg->type.fetobe_type == lmonp_befe_hostname )
    {
      ret = get_lmonpayload_begin ( msg ); 
      if (ret)
        ret += msg->sec_or_jobsizeinfo.num_tasks*sizeof(int);
    }
  }
  else if ( msg->msgclass == lmonp_fetofe )
  {
    if ( msg->type.fetofe_type == lmonp_proctable_avail )
    {
      ret = get_lmonpayload_begin ( msg );
      if (ret)
        ret += msg->sec_or_jobsizeinfo.num_tasks*sizeof(int)*4;
    }
  }
 
  return ret;
}
 
