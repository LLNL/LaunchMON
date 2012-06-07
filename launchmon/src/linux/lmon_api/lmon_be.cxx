/*
 * $Header: Exp $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008 - 2012, Lawrence Livermore National Security, LLC. Produced at 
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
 *        Aug 06 2010 DHA: Added middleware support including LMON_be_assist_mw_coloc.
 *        Nov 23 2011 DHA: Restructured MPI-Tool sync support to reduce
 *                         the complexity in this upper layer code.
 *                         Added Blue Gene /Q support.
 *        Apr 06 2010 DHA: It turned out the /etc/hosts trick works partially because
 *                         the entry returned by gethostname isn't unique on Eugene!
 *                         Added a new parser to parse /proc/personality.sh to
 *                         fully work around the problem described below
 *        Feb 04 2010 DHA: Added /etc/hosts parser to work around a problem of
 *                         systems that are not capable of resolving the hostname
 *                         returned by gethostname into an IP.
 *        Feb 04 2010 DHA: Moved RM_BG_MPIRUN && VERBOSE && USE_VERBOSE_LOGDIR support
 *                         to an earlier execution point within LMON_be_init. The change 
 *                         is to capture as much error messages into files as possible 
 *                         Also, genericized this support by removing RM_BG_MPIRUN
 *        Dec 23 2009 DHA: Added explict config.h inclusion 
 *        Dec 11 2009 DHA: Deprecate the static LMON_be_parse_raw_RPDTAB_msg
 *                         function in favor of parse_raw_RPDTAB_msg designed 
 *                         to used broadly.
 *        May 19 2009 DHA: Added errorCB support (ID2787962).
 *        May 06 2009 DHA: Bug fix for ID2787959: LMON_be_recvUsrData not returning
 *                         a correct error code.
 *        Mar 13 2009 DHA: Added large nTasks support
 *        Mar 04 2009 DHA: Added BlueGene/P support.
 *                         In particular, changed RM_BGL_MPIRUN to RM_BG_MPIRUN 
 *                         to genericize BlueGene Support
 *        Sep 23 2008 DHA: Added verbosity support
 *        Mar 20 2008 DHA: Added BlueGene/L support
 *        Feb 09 2008 DHA: Added LMON_be_getMyProctabSize support to better 
 *                         support STAT
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Dec 07 2007 DHA: Added support for secure connection between
 *                         distributed software components
 *        Nov 07 2007 DHA: Added more rigorous error checking 
 *        Aug 10 2007 DHA: Added LMON_be_recvUsrData 
 *        Jul 25 2007 DHA: Added logic changes to the RPDTAB flow 
 *                         Reduced the numer of different
 *                         collective call types
 *        Dec 29 2006 DHA: Moved comm. dependent routines into 
 *                         lmon_be_comm.cxx
 *        Dec 20 2006 DHA: File created
 */

#include "sdbg_std.hxx"

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX-like OS
#endif

#if HAVE_STDIO_H
# include <cstdio>
#else
# error cstdio is required
#endif

#if HAVE_STDLIB_H
# include <cstdlib>
#else
# error cstdlib is required
#endif

#if HAVE_STRING_H
# include <string.h>
#else
# error string.h is required
#endif

#if HAVE_IOSTREAM
# include <iostream>
#else
# error iostream is required
#endif

#if HAVE_FSTREAM
# include <fstream>
#else
# error fstream is required
#endif

#if HAVE_SIGNAL_H
# include <signal.h>
#else
# error signal.h is required
#endif

#if HAVE_MAP
# include <map>
#else
# error map is required
#endif

#if HAVE_VECTOR
# include <vector>
#else
# error vector is required
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#else
# error sys/stat.h is required
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

#if HAVE_ARPA_INET_H
# include <arpa/inet.h>
#else
# error arpa/inet.h is required
#endif

#include "gcrypt.h"

#if HAVE_ASSERT_H
# include <assert.h>
#else
# error assert.h is required
#endif

#if HAVE_LIMITS_H
# include <limits.h>
#else
# error limits.h is required
#endif

#include "lmon_api/lmon_proctab.h"
#include "lmon_api/lmon_be.h"
#include "lmon_api/lmon_lmonp_msg.h"
#include "lmon_api/lmon_say_msg.hxx"
#include "lmon_coloc_spawner.hxx"
#include "lmon_daemon_internal.hxx"
#include "lmon_be_sync_mpi.hxx"


//////////////////////////////////////////////////////////////////////////////////
//
// Static data
//
//
//
static int servsockfd = LMON_INIT;
static per_be_data_t bedata;
static std::map<std::string, std::vector<MPIR_PROCDESC_EXT *> > proctab_cache;

//////////////////////////////////////////////////////////////////////////////////
//
// LAUNCHMON BACKEND PUBLIC INTERFACE
//
//

extern "C"
lmon_rc_e
LMON_be_tester_init ( )
{
  return LMON_daemon_internal_tester_init ( &bedata );
}


//! lmon_rc_e LMON_be_regErrorCB
/*!
  registers a callback function that gets 
  invoked whenever an error message should 
  go out.
*/
extern "C"
lmon_rc_e
LMON_be_regErrorCB (int (*func) (const char *format, va_list ap))
{
  if ( func == NULL)
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "an argument is invalid" );

      return LMON_EBDARG;
    }

  if ( errorCB !=  NULL )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
        "previously registered error callback func will be invalidated" );
    }

  errorCB = func;

  return LMON_OK;
}


//! lmon_rc_e LMON_be_init
/*!
    Please refer to the header file: lmon_be.h
*/
extern "C" lmon_rc_e
LMON_be_init ( int ver, int *argc, char ***argv )
{
  int rc;
  lmon_rc_e lrc;
  struct sockaddr_in servaddr;
  int is_be = 1;

#if VERBOSE && USE_VERBOSE_LOGDIR
  char local_hostname[PATH_MAX];
  gethostname (local_hostname, PATH_MAX);
  if ( (lrc = LMON_daemon_enable_verbose(VERBOSE_LOGDIR, local_hostname, is_be))
         != LMON_OK)
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "LMON BE fails to enable verbosity %d", lrc );
    }
#elif SUB_ARCH_ALPS
  //
  // Without this, the no-verbose build will hang under ALPS
  //
  if ( freopen ("/dev/null", "w", stdout) == NULL )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "LMON BE fails to reopen stdouts");

      return LMON_EINVAL;
    }

  if ( freopen ("/dev/null", "w", stderr) == NULL )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "LMON BE fails to reopen stderr");

      return LMON_EINVAL;
    }
#endif // if SUB_ARCH_ALPS

  if ( ver != LMON_daemon_return_ver() ) 
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "LMON BE API version mismatch" );

      return LMON_EINVAL;
    }

  set_client_name(LMON_BE_MSG_PREFIX);

  //
  // Registering bedata.daemon_data.my_hostname and bedata.daemon_data.my_ip
  //
  if ( LMON_daemon_gethostname(is_bluegene_ion(),
                               bedata.daemon_data.my_hostname,
                               LMON_DAEMON_HN_MAX,
                               bedata.daemon_data.my_ip,
                               LMON_DAEMON_HN_MAX)
           != LMON_OK )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
        "LMON_daemon_gethostname failed: %d");

      return LMON_ESUBCOM;
    }

  if ( LMON_daemon_internal_init ( argc, argv, bedata.daemon_data.my_hostname, is_be )
         != LMON_OK )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
        "LMON_daemon_internal_init failed");

      return LMON_ESUBCOM;
    }

  if ( LMON_daemon_internal_getMyRank ( &(bedata.daemon_data.myrank) ) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "LMON BE getMyRank failed" );

      return LMON_ESUBCOM;
    }

  if ( LMON_daemon_internal_getSize ( &(bedata.daemon_data.width ) ) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "LMON BE getSize failed" );

      return LMON_ESUBCOM;
    }

  bedata.proctab_msg        = NULL;
  bedata.proctab_msg_size   = 0;
  bedata.daemon_data.pack   = NULL;
  bedata.daemon_data.unpack = NULL;
  bool ismstr = (bedata.daemon_data.myrank == LMON_DAEMON_MASTER)? true : false;
  int connfd = 0;

  BEGIN_MASTER_ONLY(bedata)
    if ( LMON_daemon_internal_getConnFd(&connfd) != LMON_OK )
      {
        LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
          "LMON_daemon_internal_getConnFd failed" );

        return LMON_ESUBCOM;
      }
  END_MASTER_ONLY

  bedata.daemon_data.daemon_spawner 
    = new spawner_coloc_t( ismstr,
                           connfd,
                          (int (*)(void*,int))LMON_be_broadcast);

  BEGIN_MASTER_ONLY(bedata)
    /*
     *
     * The backend master initiates a handshake with FE
     *
     */
    int i;
    lmonp_t initmsg;
    char shared_key[LMON_KEY_LENGTH];
    unsigned char sessID[LMON_KEY_LENGTH];
    unsigned char *sid_traverse;
    int32_t intsessID;
    gcry_cipher_hd_t cipher_hndl;
    gcry_error_t gcrc;

    if ( LMON_daemon_internal_getConnFd (&servsockfd) != LMON_OK )
      {
	/*
	 * Establishing a connection with the FE
	 * if the master has not established a connection
	 */
	if ( ( servsockfd = socket ( AF_INET, SOCK_STREAM, 0 )) < 0 )
	  {
	    LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
	      "socket failed ");
	    return LMON_ESYS;
	  }

#if VERBOSE
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
        "BE master: socket created" );
#endif
	/*
	 * Where does the master be connect to?
	 *
	 */ 
	if ( (lrc = LMON_daemon_getWhereToConnect ( &servaddr ) ) != LMON_OK )
	  {
	    LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
	      "LMON_daemon_getWhereToConnect failed ");

	    return LMON_ESYS;
	  }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: got where to connect" );
#endif

	if ( (rc = connect ( servsockfd, 
		    ( struct sockaddr* )&servaddr, 
		    sizeof(servaddr))) < 0 )
	  {
	    LMON_say_msg ( LMON_BE_MSG_PREFIX, true, 
              "connect failed " );

	    return LMON_ESYS;
	  }
      }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: got an actual connection" );
#endif

    bzero ( shared_key, LMON_KEY_LENGTH );
    /*
     * getting shared key and session ID that will be used 
     * to securely connect to the front-end.
     */
    if ( (lrc = LMON_daemon_getSharedKeyAndID ( shared_key,
		  &intsessID) ) != LMON_OK)
      {
	LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
    	  "LMON_daemon_getSharedKeyAndID failed ");

	return LMON_ESYS;
      }
  
#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: got a shared key: %s %d", shared_key, intsessID);
#endif

    if ( ( gcrc = gcry_cipher_open ( &cipher_hndl,
	            GCRY_CIPHER_BLOWFISH,
		    GCRY_CIPHER_MODE_ECB,
		    0 )) != GPG_ERR_NO_ERROR )
      {
	LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
	  "gcry_cipher_open failed: %s", gcry_strerror (gcrc));
	
	return LMON_ESYS;
      }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: GCRYPT open");
#endif

    /*
     * setting a 128 bit key 
     */
    if ( ( gcrc = gcry_cipher_setkey ( cipher_hndl, 
		    shared_key,
		    LMON_KEY_LENGTH )) != GPG_ERR_NO_ERROR )
      {
	LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
	  "gcry_cipher_setkey failed: %s", gcry_strerror (gcrc));

	return LMON_ESYS;
      }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: GCRYPT setkey, %x:%x:%x:%x", 
      *(int *) shared_key, 
      *(int *)(shared_key+4),
      *(int *)(shared_key+8),
      *(int *)(shared_key+12));
#endif

    bzero ((void *)sessID, LMON_KEY_LENGTH );
    memcpy ((void *)sessID, 
            (void *)&intsessID, sizeof(intsessID));

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: GCRYPT before encrypt, %x:%x:%x:%x",
      *(int *) sessID,
      *(int *)(sessID+4),
      *(int *)(sessID+8),
      *(int *)(sessID+12));
#endif

    if ( ( gcrc = gcry_cipher_encrypt ( cipher_hndl, 
		    (unsigned char*) &sessID,
		    LMON_KEY_LENGTH,
		    NULL, 
		    0)) != GPG_ERR_NO_ERROR )
      {
	LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
	  "gcry_cipher_encrypt failed: %s",gcry_strerror (gcrc));

	return LMON_ESYS;
      }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: GCRYPT encrypt, %x:%x:%x:%x",
      *(int *)sessID,
      *(int *)(sessID+4),
      *(int *)(sessID+8),
      *(int *)(sessID+12));
#endif

    sid_traverse = sessID;

    for (i=0; i < LMON_KEY_LENGTH/sizeof(int32_t); i++)
      {
	uint32_t tmpSK;
	memcpy ( (void *) &tmpSK, 
		 sid_traverse,
		 sizeof(uint32_t) );
	set_msg_header ( &initmsg,
			 lmonp_fetobe,
			 lmonp_febe_security_chk,
			 0,      /* security_key1 */
			 tmpSK,  /* security_key2 */
			 0,0,0,0,0 );   
	if ( ( write_lmonp_long_msg ( servsockfd, 
				      &initmsg, 
				      sizeof (initmsg) )) < 0 )
	  {
	    LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
	      "write_lmonp_long_msg failed");

	    return LMON_ESYS;
	  }

	sid_traverse += sizeof(uint32_t);
      }

    gcry_cipher_close ( cipher_hndl );

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: wrote back the security key");
#endif

    /*
     * ** End of the MASTER ONLY SECTION **
     *
     *
     */
  END_MASTER_ONLY

  //
  // Synch-point: the master just finished sending 
  // "ack-back" to FE. Thus at this point every BE's 
  // init should have completed.
  //
  if ( LMON_be_barrier() != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
    	"LMON_be_barrier failed" );
	
      return LMON_ESUBCOM;
    }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BEs: exiting LMON_be_init");
#endif

  return LMON_OK;
}


//! lmon_rc_e LMON_be_amIMaster 
/*!
    Please refer to the header file: lmon_be.h
*/
extern "C"
lmon_rc_e 
LMON_be_amIMaster ()
{
  if ( bedata.daemon_data.myrank < 0 )
    return LMON_EINVAL;
  
  return (( bedata.daemon_data.myrank == 0) ? LMON_YES : LMON_NO);
}


//! lmon_rc_e LMON_be_getMyRank
/*!
    Please refer to the header file: lmon_be.h
*/
extern "C" 
lmon_rc_e 
LMON_be_getMyRank ( int *rank )
{
  if ( LMON_daemon_internal_getMyRank (rank) != LMON_OK ) 
    return LMON_EINVAL;

  return LMON_OK;
}


//! lmon_rc_e LMON_be_getSize
/*!
    Please refer to the header file: lmon_be.h 
*/
extern "C" 
lmon_rc_e 
LMON_be_getSize ( int* size )
{
  if ( LMON_daemon_internal_getSize (size) != LMON_OK ) 
    return LMON_EINVAL;

  return LMON_OK;
}


//! lmon_rc_e LMON_be_barrier()
/*!
    Please refer to the header file: lmon_be.h 
*/
extern "C"
lmon_rc_e
LMON_be_barrier ()
{
  if ( LMON_daemon_internal_barrier () != LMON_OK )
    return LMON_ESUBCOM;
  
  return LMON_OK;
}


//!lmon_rc_e LMON_be_broadcast
/*!
   Please refer to the header file: lmon_be.h 
*/
extern "C"
lmon_rc_e
LMON_be_broadcast ( void *buf, int numbyte ) 
{
  if ( LMON_daemon_internal_broadcast ( buf, numbyte ) != LMON_OK )
    return LMON_ESUBCOM;

  return LMON_OK;
}


//!lmon_rc_e LMON_be_gather 
/* 
   Please refer to the header file: lmon_be.h 
*/
extern "C"
lmon_rc_e
LMON_be_gather ( void *sendbuf, int numbyte_per_elem,
                 void *recvbuf )
{
  if ( LMON_daemon_internal_gather ( sendbuf, 
         numbyte_per_elem, recvbuf ) != LMON_OK )
    return LMON_ESUBCOM;

  return LMON_OK;
}


//!lmon_rc_e LMON_be_scatter
/*!
   Please refer to the header file: lmon_be.h 
*/
extern "C"
lmon_rc_e
LMON_be_scatter (
		void *sendbuf,
		int numbyte_per_element,
		void *recvbuf )
{
  if ( LMON_daemon_internal_scatter ( sendbuf, 
         numbyte_per_element, recvbuf ) != LMON_OK )
    return LMON_ESUBCOM;

  return LMON_OK;
}


//! LMON_be_finalize();
/*!
    Please refer to the header file: lmon_be.h 
*/
extern "C"
lmon_rc_e
LMON_be_finalize ()
{
  MPIR_PROCDESC_EXT * proctab;
  int proctab_size;
  int i;

  if ( LMON_be_getMyProctabSize ( &proctab_size ) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "My proctable size is unavailable");

      return LMON_EDUNAV;
    }
  proctab = (MPIR_PROCDESC_EXT *)
        malloc (proctab_size*sizeof ( MPIR_PROCDESC_EXT ) );

  if ( proctab == NULL )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "Out of memory");

      return LMON_ENOMEM;
    }

  if ( LMON_be_getMyProctab ( proctab, 
         &proctab_size, proctab_size) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "My proctable is unavailable");

      return LMON_EDUNAV;
    }

  if ( LMON_be_procctl_done ( bedata.rmtype_instance,
			      proctab,
                              proctab_size) != LMON_OK)
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "proc control fini failed");

      return LMON_ESYS;
    }

  //
  // Clean up proctab
  //
  for(i=0; i < proctab_size; i++)
    {
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
  free (proctab);

  int is_be = 1;
  if ( LMON_daemon_internal_finalize (is_be) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "comm. fabric fini failed");    
      return LMON_ESUBCOM;
    }

  if ( bedata.proctab_msg != NULL )
    {
      free(bedata.proctab_msg);
      bedata.proctab_msg = NULL;
    }

  if (proctab_cache.size() != 0) 
    {
      std::map<std::string,std::vector<MPIR_PROCDESC_EXT *> >::iterator iter;
      std::vector<MPIR_PROCDESC_EXT *>::iterator vectiter;
      for (iter=proctab_cache.begin(); iter != proctab_cache.end(); ++iter)
        {
          for (vectiter = iter->second.begin(); vectiter != iter->second.end(); ++vectiter)
            {
              free((*vectiter)->pd.executable_name);
              free((*vectiter)->pd.host_name);
              free((*vectiter));
            }
        }
      proctab_cache.clear();
    }

  if (bedata.daemon_data.daemon_spawner)
    {
      delete bedata.daemon_data.daemon_spawner;
    }

  return LMON_OK;
}


//! lmon_rc_e LMON_be_handshake
/*!
  - BEs first gather the hostname list into the master BE
  - The master BE ships this to FE
  - The master BE receives RPDTAB message as an ack-back message
  - The master BE receives launch or attach message 
    and then broadcasts it
  - The master BE receives an usrdata message
  - The master BE sends the usrdata for BEs by piggybacking it
    to be-ready message
 
  This routine only returns when all BEs sychronized 
  at the end.

  Please refer to the header file for more details: lmon_be.h 
 */
extern "C"
lmon_rc_e
LMON_be_handshake ( void *udata )
{
  using namespace std;

  //
  // gathering hostnames and other tool specific data 
  // to piggyback to FE
  //
  int i;
  char *hngatherbuf = NULL;

  BEGIN_MASTER_ONLY(bedata)

    hngatherbuf = (char *) malloc ( LMON_DAEMON_HN_MAX * bedata.daemon_data.width );
    if ( hngatherbuf == NULL )
      {
	LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
      	  "malloc returned zero");

	return LMON_ENOMEM;
      }
#if VERBOSE
	LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
         "BE master: alloc buf of size %d x %d", 
         LMON_DAEMON_HN_MAX, bedata.daemon_data.width);
#endif
  END_MASTER_ONLY

  //
  // once be_gather is performed, hngatherbuf 
  // should hold all the hostnames
  //
  if ( LMON_be_gather ( bedata.daemon_data.my_hostname, LMON_DAEMON_HN_MAX, hngatherbuf ) != LMON_OK )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
        "gather failed");

      return LMON_ESUBCOM;
    }
#if VERBOSE
	LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
          "BES: host names are gathered");
#endif
  BEGIN_MASTER_ONLY(bedata)

    map<string, vector<unsigned int> > hostName;
    lmonp_t *sendbuf;
    int sendbufsize;
    unsigned int offset = 0;
    char *hntrav = NULL;
    hntrav = hngatherbuf; // hntrav will traverse the gathered hostnames   

    string tmpstr;
    for ( i=0; i < bedata.daemon_data.width ; ++i )
      {
    	//
    	// the first pass to compute the size of the string table
    	//
    	tmpstr = hntrav;
    	if ( hostName.find (tmpstr) == hostName.end ())
	  {
    	    hostName[tmpstr].push_back(offset);
            hostName[tmpstr].push_back(0);
    	    offset += (strlen (hntrav) + 1 );
	  }
    	 hntrav += LMON_DAEMON_HN_MAX;
       }

    //
    // sendbuf alloc and message header work
    //
    sendbufsize = sizeof (lmonp_t)
                + bedata.daemon_data.width * sizeof(int)
                + offset;
    sendbuf = (lmonp_t *) malloc ( sendbufsize );
    if (bedata.daemon_data.width < LMON_NTASKS_THRE)
      {
        set_msg_header (
                sendbuf,
                lmonp_fetobe,
                (int) lmonp_befe_hostname,
                (unsigned short) bedata.daemon_data.width,
                0,
                0,
                bedata.daemon_data.width,
                0,
                offset+bedata.daemon_data.width*sizeof(int),
                0 );
      }
    else
      {
        set_msg_header (
                sendbuf,
                lmonp_fetobe,
                (int) lmonp_befe_hostname,
                LMON_NTASKS_THRE,
                0,
                0,
                bedata.daemon_data.width,
                bedata.daemon_data.width,
                offset+bedata.daemon_data.width*sizeof(int),
                0 );
      }


#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: set a msg header of lmonp_befe_hostname type");
#endif

    //
    // packing hostname indices and string table into the message
    //
    char *packptr   = get_lmonpayload_begin (sendbuf); 
    char *strtabptr = get_strtab_begin (sendbuf);
    if ( packptr == NULL || strtabptr == NULL )
      {
	LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	  "cannot pack hostname index and string into a message" );
	return LMON_EBDMSG;
      }

    hntrav = hngatherbuf;
    for ( i=0; i < bedata.daemon_data.width; ++i )
      {
        tmpstr = hntrav;
        memcpy ((void*) packptr,
                (void*) &(hostName[tmpstr][0]),
                sizeof (unsigned int));
        packptr += sizeof (unsigned int);

        if (hostName[tmpstr][1] == 0)
          {
            memcpy ((void*) strtabptr,
                    (void*) hntrav,
                    (strlen(hntrav) + 1));

            strtabptr += ( strlen (hntrav) + 1 );
            hostName[tmpstr][1] = 1;
          }
        hntrav += LMON_DAEMON_HN_MAX;
      }

    free ( hngatherbuf );

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: about to write the hosts list to the FE");
#endif

    //
    // shipping it out and free the message buffer
    //
    write_lmonp_long_msg (servsockfd, sendbuf, sendbufsize );

    free (sendbuf);

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: the hosts list written to the FE");
#endif

    //
    // receiving the PROCTAB stream from the front-end 
    // 
    lmonp_t recvmsg;
    char *proctab_payload;
    unsigned int bytesread;
    
    read_lmonp_msgheader ( servsockfd, &recvmsg );
    if ( recvmsg.type.fetobe_type != lmonp_febe_proctab)
      {
	LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	  "Unexpected message type, protocol mismatch?");

	return LMON_EBDMSG;
      }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: received remote descriptor process "
      "table message header: lmon payload %d", 
      recvmsg.lmon_payload_length); 
#endif
    bedata.proctab_msg_size = sizeof (lmonp_t)
                            + recvmsg.lmon_payload_length
                            + recvmsg.usr_payload_length;
    bedata.proctab_msg = (lmonp_t *) malloc ( bedata.proctab_msg_size );
    memcpy ( bedata.proctab_msg, &recvmsg, sizeof (lmonp_t));
    proctab_payload = get_lmonpayload_begin ( bedata.proctab_msg );
    if ( proctab_payload == NULL )
      {
	LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	  "No proctab payload" );

	return LMON_EBDMSG;
      }
    bytesread = read_lmonp_payloads ( 
    		               servsockfd, 
    		               proctab_payload, 
    		               recvmsg.lmon_payload_length 
    		               + recvmsg.usr_payload_length );
    if ( bytesread 
         != recvmsg.lmon_payload_length+recvmsg.usr_payload_length )
      {
	LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	  "Bytes read don't equal the size specified in the received msg");

	return LMON_EBDMSG;
      }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: received RPDTAB payload"); 
#endif

    //
    // receiving a launch or attach message
    //
    read_lmonp_msgheader ( servsockfd, &recvmsg );
    switch (recvmsg.type.fetobe_type )
      {
      case lmonp_febe_attach:
        bedata.is_launch = trm_attach;
        break;

      case lmonp_febe_launch:
        bedata.is_launch = trm_launch;
        break;

      case lmonp_febe_launch_dontstop:
        bedata.is_launch = trm_launch_dontstop;
        break;

      case lmonp_febe_attach_stop:
        bedata.is_launch = trm_attach_stop;
        break;

      default:
        {
          LMON_say_msg(LMON_BE_MSG_PREFIX, true,
            "Wrong msg type: expected a febe launch or attach mode"); 
	  return LMON_EBDMSG;
        }
      }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: received is_launch msg %d", bedata.is_launch); 
#endif

    read_lmonp_msgheader ( servsockfd, &recvmsg );
    if ( recvmsg.type.fetobe_type != lmonp_febe_rm_type)
      {
        LMON_say_msg(LMON_BE_MSG_PREFIX, true,
          "Unexpected message type, a protocol mismatch?");

        return LMON_EBDMSG;
      }
    bedata.rmtype_instance
      = (rm_catalogue_e) recvmsg.sec_or_jobsizeinfo.rm_type;

    //
    // receiving the USRDATA stream from the front-end
    //
    read_lmonp_msgheader ( servsockfd, &recvmsg );
    if ( recvmsg.type.fetobe_type != lmonp_febe_usrdata )
      {
        LMON_say_msg(LMON_BE_MSG_PREFIX, true,
          "Wrong msg type: expected a febe user data"); 
        return LMON_EBDMSG; 
      }

    int usrpayloadlen = recvmsg.usr_payload_length;
    if ( usrpayloadlen > 0 )
      {
    	char *usrpl = (char *) malloc ( recvmsg.usr_payload_length );
    	bytesread = read_lmonp_payloads (
    			servsockfd,
    			usrpl,
    			recvmsg.usr_payload_length );
    	//
    	// with the following unpack func, udata comes to have
    	// deserialized usr data
    	//
    	bedata.daemon_data.unpack ( usrpl, recvmsg.usr_payload_length, udata );

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: received USRDATA"); 
#endif
      }

  END_MASTER_ONLY

  //
  // we want to bcast if this is launch or attach, 
  // and the aggregate size of proctab_msg
  //
  if ( LMON_be_broadcast ( &(bedata.is_launch), 
                  sizeof (bedata.is_launch)) != LMON_OK )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	"Broadcast failed for is_launch"); 

      return LMON_ESUBCOM;
    }

  if ( LMON_be_broadcast ( &(bedata.rmtype_instance), 
                  sizeof (bedata.rmtype_instance)) != LMON_OK )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
       "Broadcast failed for rmtype_instance"); 

      return LMON_ESUBCOM;
    }
 
  if ( LMON_be_broadcast (
		  &(bedata.proctab_msg_size), 
		  sizeof (bedata.proctab_msg_size)) != LMON_OK )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	"Broadcast failed"); 

      return LMON_ESUBCOM;
    }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BES: Bcasted is_launch and proctab_msg_size: %d", 
      bedata.proctab_msg_size); 
#endif

  BEGIN_SLAVE_ONLY(bedata)
    //
    // slave BEs preparing for proctab_msg broadcast 
    //
    bedata.proctab_msg = (lmonp_t *) 
      malloc ( bedata.proctab_msg_size );
    if ( bedata.proctab_msg == NULL )
      {
	LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	  "Malloc returned NULL"); 
	return LMON_ENOMEM;
      }
  END_SLAVE_ONLY

  //
  // duplicating the proctab_msg using broadcast
  //
  if ( LMON_be_broadcast ( bedata.proctab_msg, bedata.proctab_msg_size ) 
       != LMON_OK )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
	"Broadcast failed"); 
      return LMON_ESUBCOM;
    }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BEs: proctab_msg finish Bcasting");
#endif


  //
  //
  // ** Begin Tool <--> MPI synchronization **
  //
  //
  MPIR_PROCDESC_EXT* proctab;
  int proctab_size;

  if ( LMON_be_getMyProctabSize ( &proctab_size ) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "My proctable size is unavailable");

      return LMON_EDUNAV;
    }

  proctab = (MPIR_PROCDESC_EXT *)
        malloc (proctab_size*sizeof ( MPIR_PROCDESC_EXT ) );

  if ( proctab == NULL )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "Out of memory");

      return LMON_ENOMEM;
    }

  if ( LMON_be_getMyProctab ( proctab, 
         &proctab_size, proctab_size) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "My proctable is unavailable");

      return LMON_EDUNAV;
    }

  //
  // Call procctl_init which then calls into OS-dependent
  // lower-layer to complete init as a function of the
  // target RM. As part of init, the lower-layer leaves
  // the job in a stopped state.
  //
  int dontstop_fastpath = 0;
  if ( ( bedata.is_launch == trm_launch_dontstop ) 
       || ( bedata.is_launch == trm_attach ) )
    {
      dontstop_fastpath = 1;
    }

  if ( LMON_be_procctl_init ( bedata.rmtype_instance,
         proctab, proctab_size,
         dontstop_fastpath ) != LMON_OK)
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "proc control initialization failed");

      return LMON_ESUBSYNC;
    }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BES: LMON_be_procctl_init done");
#endif

  if ( ( bedata.is_launch == trm_launch_dontstop )
       || ( bedata.is_launch == trm_attach ) )
    {
      //
      // Call procctl_run if the user requested to continue
      // for the start-up case or the default attach case.
      //
      if ( LMON_be_procctl_run ( bedata.rmtype_instance, 0,
                                 proctab, proctab_size )
           != LMON_OK )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "proc control run failed");

          return LMON_ESUBSYNC;
        }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BES: LMON_be_procctl_run done");
#endif

    }

  if ( LMON_be_procctl_initdone ( bedata.rmtype_instance,
                                  proctab, proctab_size )
       != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "proc control initdone failed");

      return LMON_ESUBSYNC;
    }

# if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BES: LMON_be_procctl_initdone done; "
      "the job has been sync'ed for next operations");
# endif

  //
  // Clean up proctab
  //
  for(i=0; i < proctab_size; i++)
    {
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
  free (proctab);

  if ( LMON_be_barrier () != LMON_OK )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
        "Barrier failed");
      return LMON_ESUBCOM;
    }

  //
  //
  // ** Complete Tool <--> MPI synchronization **
  //
  //

#if VERBOSE
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
    "BES: returning from LMON_be_handshake");
#endif

  return LMON_OK;
}


//! lmon_rc_e LMON_be_ready()
/*!
    Please refer to the header file: lmon_be.h 
*/
extern "C"
lmon_rc_e 
LMON_be_ready ( void *udata )
{
  BEGIN_MASTER_ONLY(bedata)
    lmonp_t *readymsg = NULL;
    if ( (udata != NULL) && (bedata.daemon_data.pack != NULL) )
      {
    	char *uoffset;
        int upl_leng;
    	readymsg = (lmonp_t *) malloc ( 
    			      sizeof (lmonp_t) 
    			      + LMON_MAX_USRPAYLOAD );
        set_msg_header ( 
                    readymsg,
                    lmonp_fetobe,
                    (int) lmonp_befe_usrdata,
                    0,0,0,0,0,0,
                    LMON_MAX_USRPAYLOAD);

    	uoffset = get_usrpayload_begin ( readymsg );
    	bedata.daemon_data.pack ( udata,
    		      uoffset,
    		      LMON_MAX_USRPAYLOAD, 
    		      &upl_leng );
    	//assert ( upl_leng <= LMON_MAX_USRPAYLOAD );
        readymsg->usr_payload_length = upl_leng;
      }
    else
      {
    	readymsg = (lmonp_t *) malloc ( sizeof (lmonp_t));
    	readymsg->usr_payload_length = 0;
      }

    readymsg->msgclass = lmonp_fetobe;
    readymsg->type.fetobe_type = lmonp_befe_ready;
    readymsg->sec_or_jobsizeinfo.security_key1 = 0;
    readymsg->sec_or_stringinfo.security_key2 = 0;
    readymsg->lmon_payload_length = 0;
    write_lmonp_long_msg (
    		servsockfd,
    		readymsg,
    		sizeof (lmonp_t) + readymsg->usr_payload_length );

    free(readymsg);
  END_MASTER_ONLY

  if ( LMON_be_barrier() != LMON_OK )
    return LMON_ESUBCOM;

  return LMON_OK;
}


//! lmon_rc_e LMON_be_assist_mw_coloc()
/*!
    Please refer to the header file: lmon_be.h 
*/
extern "C"
lmon_rc_e
LMON_be_assist_mw_coloc()
{
  using std::string;
  if (bedata.daemon_data.daemon_spawner)
    {
      if (!bedata.daemon_data.daemon_spawner->spawn())
        {
           LMON_say_msg(LMON_BE_MSG_PREFIX, true,
             "coloc spawner failed: %s", bedata.daemon_data.daemon_spawner->get_err_str().c_str());

           return LMON_ESUBCOM;
        }
    }

#if VERBOSE
   LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
     "BE daemon assisted in MW daemon spawn");
#endif

  return LMON_OK;
}

//! lmon_rc_e LMON_be_getMyProctab
/*!
    Please refer to the header file: lmon_be.h  
*/
extern "C"
lmon_rc_e 
LMON_be_getMyProctab (
		 MPIR_PROCDESC_EXT *proctabbuf,
		 int *size, 
		 int proctab_num_elem )
{
  using namespace std;

  unsigned int i;
  char *key;
  lmon_rc_e lrc;

  if ( bedata.proctab_msg == NULL )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
        "bedata.proctab_msg is null!  ");

      return LMON_EINVAL;
    }

  if ( proctab_cache.size () == 0 )
    {
      if ( parse_raw_RPDTAB_msg ( bedata.proctab_msg, &proctab_cache) < 0 )
        {
          LMON_say_msg (LMON_BE_MSG_PREFIX, false,
            "parse_raw_RPDTAB_msg failed to parse RPDTAB");

          return LMON_EINVAL;
        }

    }

  key = bedata.daemon_data.my_hostname;

  string myhostname (key);
  (*size) = (int) proctab_cache[myhostname].size ();
  map<string, vector<MPIR_PROCDESC_EXT* > >::const_iterator viter 
    = proctab_cache.find (myhostname);

  for ( i=0; (i < (*size)) && (i < proctab_num_elem); ++i )
    {
      //
      // The caller is responsible for freeing strdup's memory
      //
      proctabbuf[i].pd.executable_name = strdup ((*viter).second[i]->pd.executable_name);
      proctabbuf[i].pd.host_name = strdup ((*viter).second[i]->pd.host_name);
      proctabbuf[i].pd.pid = (*viter).second[i]->pd.pid;
      proctabbuf[i].mpirank = (*viter).second[i]->mpirank;
      proctabbuf[i].cnodeid = (*viter).second[i]->cnodeid;
    }

  if ( ( i == proctab_num_elem ) && ( proctab_num_elem < (*size) ) )
    return LMON_ETRUNC;

  return LMON_OK;
}  


//! lmon_rc_e LMON_be_getMyProctabSize
/*
    Please refer to the header file: lmon_be.h  
*/
lmon_rc_e 
LMON_be_getMyProctabSize ( int *size )
{ 
  using namespace std;

  char *key=NULL;
  lmon_rc_e lrc;
  
  if ( bedata.proctab_msg == NULL )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
        "bedata.proctab_msg is null!  ");

      return LMON_EINVAL;
    }

  if ( proctab_cache.size () == 0 )
    {
      if ( parse_raw_RPDTAB_msg ( bedata.proctab_msg, &proctab_cache) < 0 )
        {
          LMON_say_msg (LMON_BE_MSG_PREFIX, false,
            "parse_raw_RPDTAB_msg failed to parse RPDTAB");

          return LMON_EINVAL;
        }
    }

  key = bedata.daemon_data.my_hostname;

  string myhostname (key);
  (*size) = (int) proctab_cache[myhostname].size ();

  return LMON_OK;  
}


//! lmon_rc_e LMON_be_regPackForBeToFe
/* 
    Please refer to the header file: lmon_be.h     
*/
extern "C" 
lmon_rc_e 
LMON_be_regPackForBeToFe ( int (*packBefe) ( 
                   void *udata,void *msgbuf,
                   int msgbufmax,int *msgbuflen ) )
{
  if ( bedata.daemon_data.pack != NULL )
  {
    LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
      "pack has already been registered or has not been initialized");  
    LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
      "replacing the pack function...");  
  }
  
  bedata.daemon_data.pack = packBefe;

  return LMON_OK; 
}


//! lmon_rc_e LMON_be_regUnpackForFeToBe
/*
    Please refer to the header file: lmon_be.h     
*/
extern "C"
lmon_rc_e 
LMON_be_regUnpackForFeToBe (
		 int (*unpackFebe) ( 
		 void *udatabuf,int udatabuflen, void *udata ))
{
  if ( bedata.daemon_data.unpack != NULL )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
        "pack has already been registered or has not been initialized");  
      LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
        "replacing the pack function...");  	  
    }
	  
  bedata.daemon_data.unpack = unpackFebe;
  
  return LMON_OK;
}


//! lmon_rc_e LMON_fe_recvUsrData
/*!
    Please refer to the header file: lmon_be.h 
*/
lmon_rc_e 
LMON_be_recvUsrData ( void* udata )
{
  lmonp_t recvmsg;
  unsigned int bytesread;
  lmon_rc_e lrc = LMON_OK;

  BEGIN_MASTER_ONLY(bedata)
    //
    // receiving the USRDATA stream from the frontend
    //     
    if ( read_lmonp_msgheader ( servsockfd, &recvmsg ) < 0 )
      {
	LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	  "read_lmonp return a neg value");

	lrc = LMON_ESYS;
        goto something_bad;
      }

    if ( recvmsg.type.fetobe_type != lmonp_febe_usrdata )
      {
	LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
          "the received message doesn't match with the expected type");

        LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
          "  The expected type is {Class(lmonp_fetobe),"
                       "Type(lmonp_febe_usrdata),"
                       "LMON_payload_size(0),"
                       "USR_payload_size(>=0)}.");

        LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
          "  A recv'ed msg of {Class(%s),"
                       "Type(%s),"
                       "LMON_payload_size(%s),"
                       "USR_payload_size(%s)} has been received.",
                       lmon_msg_to_str(field_class, &recvmsg),
                       lmon_msg_to_str(field_type, &recvmsg),
                       lmon_msg_to_str(field_lmon_payload_length, &recvmsg),
                       lmon_msg_to_str(field_usr_payload_length, &recvmsg));

	lrc = LMON_EBDMSG;
        goto something_bad; 
      }

    int usrpayloadlen = recvmsg.usr_payload_length;
    if ( usrpayloadlen > 0 )
      {
    	char *usrpl = (char *) malloc ( recvmsg.usr_payload_length );
	if ( usrpl == NULL)
	  {
	    LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	      "Out of memory");

	    lrc = LMON_ENOMEM;
            goto something_bad;
	  }

    	bytesread = read_lmonp_payloads (
    			servsockfd,
    			usrpl,
    			recvmsg.usr_payload_length );
    	//
    	// with the following unpack func, udata comes to have
    	// deserialized usr data
    	//
	if ( bedata.daemon_data.unpack )	  
	  {
	    if (bedata.daemon_data.unpack ( usrpl, recvmsg.usr_payload_length, udata ) < 0)	  
              {
	        LMON_say_msg(LMON_BE_MSG_PREFIX, true,
                "a negative return code from bedata.daemon_data.unpack.");
	   
                lrc = LMON_ENEGCB;
              }
          }
	else
	  {
	    LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	      "bedata.daemon_data.unpack has not been registered!  ");

	    lrc = LMON_ENCLLB;
	  }
	free (usrpl);
      }
    else
      {
	lrc = LMON_ENOPLD;
      }

  END_MASTER_ONLY


something_bad:
  //
  // duplicating the return code
  //
  if ( LMON_be_broadcast ( &lrc, sizeof(lrc) != LMON_OK ))
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
        "Broadcast failed");

      lrc = LMON_ESUBCOM;
    }

    return lrc;
}


//! lmon_rc_e LMON_fe_sendUsrData
/*!
    Please refer to the header file: lmon_be.h 
*/
lmon_rc_e 
LMON_be_sendUsrData ( void* udata )
{
  lmon_rc_e lrc = LMON_OK;

  BEGIN_MASTER_ONLY(bedata)
    
    lmonp_t *usrmsg;
  
    if ( (udata != NULL) && (bedata.daemon_data.pack != NULL) )
      {
    	char *uoffset;
        int upl_leng;
    	usrmsg = (lmonp_t *) malloc ( 
    			      sizeof (lmonp_t) 
    			      + LMON_MAX_USRPAYLOAD );
	if ( usrmsg == NULL )
	  {
	    LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
 	      "Out of memory");

	    lrc = LMON_ENOMEM;
            goto something_bad;
	  }
	
        set_msg_header ( 
                    usrmsg,
                    lmonp_fetobe,
                    (int) lmonp_befe_usrdata,
                    0,0,0,0,0,0,
                    LMON_MAX_USRPAYLOAD);

    	uoffset = get_usrpayload_begin ( usrmsg );
        if ( bedata.daemon_data.pack ( udata, uoffset, LMON_MAX_USRPAYLOAD, &upl_leng ) < 0 )
          {
	    lrc = LMON_ENEGCB;
	    goto something_bad;
          }

    	//assert ( upl_leng <= LMON_MAX_USRPAYLOAD );
        usrmsg->usr_payload_length = upl_leng;
      }
    else
      {
    	usrmsg = (lmonp_t *) malloc ( sizeof (lmonp_t));
	if ( usrmsg == NULL )
	  {
	    LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	      "Out of memory");

	    lrc = LMON_ENOMEM;
            goto something_bad;
	  }
    	usrmsg->usr_payload_length = 0;
	lrc = LMON_ENCLLB;
      }

    usrmsg->msgclass = lmonp_fetobe;
    usrmsg->type.fetobe_type = lmonp_befe_usrdata;
    usrmsg->sec_or_jobsizeinfo.security_key1 = 0;
    usrmsg->sec_or_stringinfo.security_key2 = 0;
    usrmsg->lmon_payload_length = 0;
    if ( (write_lmonp_long_msg ( 
    		       servsockfd, 
		       usrmsg, 
		       sizeof (lmonp_t) + usrmsg->usr_payload_length ))
	 < 0 )
      {
	LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
          "write_lmonp returned a neg value");

	lrc = LMON_ESYS;
        goto something_bad;
      }
    free (usrmsg);

  END_MASTER_ONLY

something_bad:
  //
  // duplicating the return code
  //
  if ( LMON_be_broadcast ( &lrc, sizeof(lrc) != LMON_OK ))
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
        "Broadcast failed");

      lrc = LMON_ESUBCOM;
    }

  return lrc;
}
