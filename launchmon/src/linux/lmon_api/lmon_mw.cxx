/*
 * $Header: $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008 ~ 2010, Lawrence Livermore National Security, LLC. Produced at 
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
 *        May 31 2012 DHA: Copied the file from the 0.8-middleware-support branch.
 *        Aug 02 2010 DHA: File created
 */

#include "sdbg_std.hxx"

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
#endif

#if HAVE_STDIO_H
# include <cstdio>
#else
# error stdio.h is required
#endif

#if HAVE_STDLIB_H
# include <cstdlib>
#else
# error stdlib.h is required
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

#include "lmon_api/lmon_mw.h"
#include "lmon_api/lmon_lmonp_msg.h"
#include "lmon_api/lmon_say_msg.hxx"
#include "lmon_daemon_internal.hxx"
#include "sdbg_rsh_spawner.hxx"


//////////////////////////////////////////////////////////////////////////////////
//
// Static data
//
//
//
static int servsockfd = LMON_INIT;
static per_mw_data_t mwdata;


//////////////////////////////////////////////////////////////////////////////////
//
// LAUNCHMON MiddleWare PUBLIC INTERFACE
//
//
//

//! lmon_rc_e LMON_mw_amIMaster
/*!

*/
extern "C" lmon_rc_e
LMON_mw_amIMaster ()
{
  if ( mwdata.daemon_data.myrank < 0 )
    return LMON_EINVAL;

  return (( mwdata.daemon_data.myrank == 0) ? LMON_YES : LMON_NO);
}


extern "C" lmon_rc_e
LMON_mw_daemon_internal_init(int *argc, char ***argv)
{
  char **nargv = *argv;
  int n_lmonopt = 0;
  int i;
  lmon_rc_e lrc = LMON_OK;

  mwdata.daemon_data.daemon_spawner = NULL;

  /*
   * The following code assumes middleware options are specified after all other
   * options are given.
   */
  for (i=1; i < (*argc); ++i) 
    {
      if ( (nargv[i][0] == '-') && (nargv[i][1]) ) 
        {
          if ( strncmp(nargv[i], LMON_RSHSPAWNER_OPT, strlen(LMON_RSHSPAWNER_OPT)) 
               == 0 )
            {
               //
               // TODO: Can't handle daemon options yet
               //
               mwdata.daemon_data.daemon_spawner
                 = new spawner_rsh_t((*argv)[0], 
                                 std::vector<std::string>(), 
                                 nargv[i]+1+strlen(LMON_RSHSPAWNER_OPT));
               n_lmonopt++;
            }
      }
    } /* for */

  (*argc) -= n_lmonopt;
  nargv[(*argc)+0] = NULL;
  char tmpbuf[PATH_MAX];
  for (i=1; i < (n_lmonopt+1); ++i )
    {
      sprintf(tmpbuf, "LMONNOOP%d=%d", i,i);
       /* this region of memory might not be safe to be freed, so we just leak */
       /* and nargv[(*argc_p)+n_pmgropt] must be null */
      nargv[(*argc)+i] = strdup (tmpbuf);
    }

  return lrc;
} 


//! lmon_rc_e LMON_mw_init
/*!

*/
extern "C" lmon_rc_e
LMON_mw_init(int ver, int *argc, char ***argv )
{
  lmon_rc_e lrc;
  int is_be = 0;

#if VERBOSE && USE_VERBOSE_LOGDIR
  char local_hostname[PATH_MAX];
  gethostname (local_hostname, PATH_MAX);
  if ( (lrc = LMON_daemon_enable_verbose(VERBOSE_LOGDIR, local_hostname, is_be))
         != LMON_OK)
    {
      LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
        "LMON MW fails to enable verbosity %d", lrc );
    }
#elif RM_ALPS_APRUN
  //
  // Hack but without this the no-verbose build will hang under ALPS
  //
  if ( freopen ("/dev/null", "w", stdout) == NULL )
    {
      LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
        "LMON MW fails to reopen stdouts");

      return LMON_EINVAL;
    }

  if ( freopen ("/dev/null", "w", stderr) == NULL )
    {
      LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
        "LMON MW fails to reopen stderr");

      return LMON_EINVAL;
    }
#endif

#if VERBOSE
   LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
     "MW daemon spawned");
#endif

  if ( ver != LMON_daemon_return_ver() ) 
    {
      LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
        "LMON MW API version mismatch" );

      return LMON_EINVAL;
    }

  set_client_name(LMON_MW_MSG_PREFIX);


  if ( (lrc = LMON_daemon_gethostname ( is_bluegene_ion(),
                               mwdata.daemon_data.my_hostname,
                               LMON_DAEMON_HN_MAX,
                               mwdata.daemon_data.my_ip,
                               LMON_DAEMON_HN_MAX ) )
           != LMON_OK )
    {
      LMON_say_msg(LMON_MW_MSG_PREFIX, true,
        "LMON_daemon_gethostname failed: %d");

      return lrc;
    }

#if VERBOSE
   LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
     "LMON_daemon_gethostname returned %s", mwdata.daemon_data.my_hostname);
#endif

  if ( (lrc = LMON_mw_daemon_internal_init ( argc, argv )) 
         != LMON_OK )
    {
       LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
         "LMON_mw_daemon_internal_init returned non LMON_OK");

       return lrc;
    }

  if (mwdata.daemon_data.daemon_spawner)
    {
      mwdata.daemon_data.daemon_spawner->spawn();
    }

#if VERBOSE 
   LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
     "LMON_mw_daemon_internal_init and spawn finished ");
#endif

  if ( LMON_daemon_internal_init ( argc, argv, mwdata.daemon_data.my_hostname, is_be )
         != LMON_OK )
    {
      LMON_say_msg(LMON_MW_MSG_PREFIX, true,
        "LMON_daemon_internal_init failed");

      return LMON_ESUBCOM;
    }

#if VERBOSE
  LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
    "LMON_daemon_internal_init done");
  int atest=0;
  for (atest=0; atest < (*argc); atest++)
    {
      LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
        "argv[%d]: %s", atest, (*argv)[atest]);
    }
#endif

  if ( LMON_daemon_internal_getMyRank ( &(mwdata.daemon_data.myrank) ) != LMON_OK )
    {
      LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
        "LMON_daemon_getMyRank failed" );

      return LMON_ESUBCOM;
    }

  if ( LMON_daemon_internal_getSize ( &(mwdata.daemon_data.width ) ) != LMON_OK )
    {
      LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
        "LMON_daemon_getMyRank failed" );

      return LMON_ESUBCOM;
    }

#if VERBOSE 
  LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
    "My rank is %d, My size is %d", mwdata.daemon_data.myrank, mwdata.daemon_data.width);
#endif

  BEGIN_MASTER_ONLY(mwdata)
    //
    //
    // The middleware master initiates a handshake with FE
    //
    //
    unsigned int i;
    lmonp_t initmsg;
    char shared_key[LMON_KEY_LENGTH];
    unsigned char sessID[LMON_KEY_LENGTH];
    unsigned char *sid_traverse;
    int32_t intsessID;
    gcry_cipher_hd_t cipher_hndl;
    gcry_error_t gcrc;
    bzero ( shared_key, LMON_KEY_LENGTH );

    if ( LMON_daemon_internal_getConnFd (&servsockfd) != LMON_OK )
      {
        LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
          "LMON_daemon_internal_getConnFd failed ");

        return LMON_ESUBCOM;
      }

#if VERBOSE 
          LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
             "The master MW deamon now has a connection to the FE");
#endif

    //
    // getting shared key and session ID that will be used 
    // to securely connect to the front-end
    //
    if ( (lrc = LMON_daemon_getSharedKeyAndID ( shared_key, &intsessID) )
              != LMON_OK)
      {
        LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
          "LMON_daemon_getSharedKeyAndID failed ");

        return LMON_ESYS;
      }

#if VERBOSE
    LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
      "MW master: got a shared key: %s %d", shared_key, intsessID);
#endif

    if ( ( gcrc = gcry_cipher_open ( &cipher_hndl,
                                     GCRY_CIPHER_BLOWFISH,
                                     GCRY_CIPHER_MODE_ECB,
                                     0 )) 
                != GPG_ERR_NO_ERROR )
      {
        LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
          "gcry_cipher_open failed: %s", gcry_strerror (gcrc));

        return LMON_ESYS;
      }

#if VERBOSE
    LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
      "MW master: GCRYPT open");
#endif

    //
    // setting a 128 bit key 
    //
    if ( ( gcrc = gcry_cipher_setkey ( cipher_hndl,
                                       shared_key,
                                       LMON_KEY_LENGTH )) 
                != GPG_ERR_NO_ERROR )
      {
        LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
          "gcry_cipher_setkey failed: %s", gcry_strerror (gcrc));

        return LMON_ESYS;
      }

#if VERBOSE
    LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
      "MW master: GCRYPT setkey, %x:%x:%x:%x",
      *(int *) shared_key, *(int *)(shared_key+4),
      *(int *)(shared_key+8), *(int *)(shared_key+12));
#endif

    bzero ((void *)sessID, LMON_KEY_LENGTH );
    memcpy ((void *)sessID,
            (void *)&intsessID, sizeof(intsessID));

#if VERBOSE
    LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
      "MW master: GCRYPT before encrypt, %x:%x:%x:%x",
      *(int *) sessID,
      *(int *)(sessID+4), *(int *)(sessID+8), *(int *)(sessID+12));
#endif

    if ( ( gcrc = gcry_cipher_encrypt ( cipher_hndl,
                                        (unsigned char*) &sessID,
                                        LMON_KEY_LENGTH,
                                        NULL,
                                        0)) 
                != GPG_ERR_NO_ERROR )
      {
        LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
          "gcry_cipher_encrypt failed: %s", gcry_strerror (gcrc));

        return LMON_ESYS;
      }

#if VERBOSE
    LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
      "MW master: GCRYPT encrypt, %x:%x:%x:%x",
      *(int *)sessID, *(int *)(sessID+4),
      *(int *)(sessID+8), *(int *)(sessID+12));
#endif

    sid_traverse = sessID;

    for (i=0; i < LMON_KEY_LENGTH/sizeof(int32_t); i++)
      {
        uint32_t tmpSK;
        memcpy ( (void *) &tmpSK, sid_traverse, sizeof(uint32_t));
        set_msg_header ( &initmsg,
                         lmonp_fetomw,
                         lmonp_femw_security_chk,
                         0,      /* security_key1 */
                         tmpSK,  /* security_key2 */
                         0,0,0,0,0 );
        if ( ( write_lmonp_long_msg ( servsockfd,
                                      &initmsg,
                                      sizeof (initmsg) )) < 0 )
          {
            LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
              "write_lmonp_long_msg failed");

            return LMON_ESYS;
          }

        sid_traverse += sizeof(uint32_t);
      }

    gcry_cipher_close ( cipher_hndl );

#if VERBOSE
    LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
      "MW master: wrote back the security key");
#endif

    //
    // ** End of the MASTER ONLY SECTION **
    //
    //
    //
  END_MASTER_ONLY

  //
  // Synch-point: the master just finished sending 
  // "ack-back" to FE. Thus at this point every MW's 
  // init should have completed.
  //
  if ( LMON_mw_barrier() != LMON_OK )
    {
      LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
        "barrier failed" );

      return LMON_ESUBCOM;
    }

#if VERBOSE
    LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
      "MWs: exiting mw_init");
#endif

  return LMON_OK;
}


//! lmon_rc_e LMON_mw_barrier
/*!

*/
extern "C" lmon_rc_e
LMON_mw_barrier()
{
  if ( LMON_daemon_internal_barrier () != LMON_OK )
    return LMON_ESUBCOM;

  return LMON_OK;
}


//! lmon_rc_e LMON_mw_gather
/*!

*/
extern "C" lmon_rc_e
LMON_mw_gather( void *sendbuf, int numbyte_per_elem, void *recvbuf)
{
  if ( LMON_daemon_internal_gather ( sendbuf,
         numbyte_per_elem, recvbuf ) != LMON_OK )
    return LMON_ESUBCOM;

  return LMON_OK;
}


//! lmon_rc_e LMON_mw_scatter
/*!

*/
extern "C" lmon_rc_e
LMON_mw_scatter(void *sendbuf, int numbyte_per_element, void *recvbuf)
{
  if ( LMON_daemon_internal_scatter ( sendbuf,
         numbyte_per_element, recvbuf ) != LMON_OK )
    return LMON_ESUBCOM;

  return LMON_OK;
}


//! lmon_rc_e LMON_mw_broadcast
/*!

*/
extern "C" lmon_rc_e
LMON_mw_broadcast(void *buf, int numbyte)
{
  if ( LMON_daemon_internal_broadcast ( buf, numbyte ) != LMON_OK )
    return LMON_ESUBCOM;

  return LMON_OK;
}


//! lmon_rc_e LMON_mw_getSize
/*!

*/
extern "C" lmon_rc_e
LMON_mw_getSize(int *size)
{
  if ( LMON_daemon_internal_getSize (size) != LMON_OK ) 
    return LMON_EINVAL;

  return LMON_OK;
}


//! lmon_rc_e LMON_mw_getMyRank
/*!

*/
extern "C" lmon_rc_e
LMON_mw_getMyRank(int *rank)
{
  if ( LMON_daemon_internal_getMyRank (rank) != LMON_OK ) 
    return LMON_EINVAL;

  return LMON_OK;
}


//! lmon_rc_e LMON_mw_regPackForMwToFe
/*!

*/
extern "C" lmon_rc_e
LMON_mw_regPackForMwToFe(
                int (*packMwfe)
                ( void *udata,void *msgbuf,int msgbufmax,int *msgbuflen ) )
{
  if ( mwdata.daemon_data.pack != NULL )
  {
    LMON_say_msg(LMON_MW_MSG_PREFIX, true,
      "pack has already been registered or has not been initialized");
    LMON_say_msg(LMON_MW_MSG_PREFIX, true,
      "replacing the pack function...");
  }

  mwdata.daemon_data.pack = packMwfe;

  return LMON_OK; 
}


//! lmon_rc_e LMON_mw_regUnpackForFeToMw
/*!

*/
extern "C" lmon_rc_e 
LMON_mw_regUnpackForFeToMw(
                int (*unpackFemw)
                (void *udatabuf,int udatabuflen, void *udata))
{
  if ( mwdata.daemon_data.unpack != NULL )
    {
      LMON_say_msg(LMON_MW_MSG_PREFIX, true,
        "pack has already been registered or has not been initialized");
      LMON_say_msg(LMON_MW_MSG_PREFIX, true,
        "replacing the pack function...");
    }

  mwdata.daemon_data.unpack = unpackFemw;

  return LMON_OK;
}


//! lmon_rc_e LMON_mw_handshake
/*!

*/
extern "C" lmon_rc_e
LMON_mw_handshake(void *udata)
{
  using namespace std;

  //
  // gathering hostnames and other tool specific data 
  // to piggyback to FE
  //
  int i;
  char *hngatherbuf = NULL;

  BEGIN_MASTER_ONLY(mwdata)

    hngatherbuf = (char *) malloc ( LMON_DAEMON_HN_MAX * mwdata.daemon_data.width );
    if ( hngatherbuf == NULL )
      {
	LMON_say_msg(LMON_MW_MSG_PREFIX, true, 
      	  "malloc returned zero");

	return LMON_ENOMEM;
      }
#if VERBOSE
	LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
         "MW master: alloc buf of size %d x %d", 
         LMON_DAEMON_HN_MAX, mwdata.daemon_data.width);
#endif
  END_MASTER_ONLY

  //
  // once mw_gather is performed, hngatherbuf 
  // should hold all the hostnames
  //
  if ( LMON_mw_gather ( mwdata.daemon_data.my_hostname, LMON_DAEMON_HN_MAX, hngatherbuf ) != LMON_OK )
    {
      LMON_say_msg(LMON_MW_MSG_PREFIX, true, 
        "gather failed");

      return LMON_ESUBCOM;
    }
#if VERBOSE
	LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
          "MWs: host names are gathered");
#endif

  BEGIN_MASTER_ONLY(mwdata)

    map<string, vector<unsigned int> > hostName;

    lmonp_t *sendbuf;
    int sendbufsize;
    unsigned int offset = 0;
    char *hntrav = NULL;
    hntrav = hngatherbuf; // hntrav will traverse the gathered hostnames
    string tmpstr;

    for ( i=0; i < mwdata.daemon_data.width ; ++i )
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
                + mwdata.daemon_data.width * sizeof(int)
                + offset;
    sendbuf = (lmonp_t *) malloc ( sendbufsize );

    if (mwdata.daemon_data.width < LMON_NTASKS_THRE)
      {
        set_msg_header (
                sendbuf,
                lmonp_fetomw,
                (int) lmonp_mwfe_hostname,
                (unsigned short) mwdata.daemon_data.width,
                0,
                0,
                mwdata.daemon_data.width,
                0,
                offset+mwdata.daemon_data.width*sizeof(int),
                0 );
      }
    else
      {
        set_msg_header (
                sendbuf,
                lmonp_fetomw,
                (int) lmonp_mwfe_hostname,
                LMON_NTASKS_THRE,
                0,
                0,
                mwdata.daemon_data.width,
                mwdata.daemon_data.width,
                offset+mwdata.daemon_data.width*sizeof(int),
                0 );
      }

#if VERBOSE
    LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
      "MW master: set a msg header of lmonp_bemw_hostname type");
#endif

    //
    // packing hostname indices and string table into the message
    //
    char *packptr   = get_lmonpayload_begin (sendbuf);
    char *strtabptr = get_strtab_begin (sendbuf);
    if ( packptr == NULL || strtabptr == NULL )
      {
	LMON_say_msg(LMON_MW_MSG_PREFIX, true,
	  "cannot pack hostname index and string into a message" );
	return LMON_EBDMSG;
      }
    hntrav = hngatherbuf;
    for ( i=0; i < mwdata.daemon_data.width; ++i )
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

    //assert((strtabptr - (char *) sendbuf) == sendbufsize);

#if VERBOSE
    LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
      "MW master: about to write the hosts list to the FE");
#endif

    //
    // shipping it out and free the message buffer
    //
    write_lmonp_long_msg (servsockfd, sendbuf, sendbufsize );

    free (sendbuf);

#if VERBOSE
    LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
      "MW master: the hosts list written to the FE");
#endif

    //
    // receiving the USRDATA stream from the front-end
    //
    lmonp_t recvmsg;
    unsigned int bytesread;
    read_lmonp_msgheader ( servsockfd, &recvmsg );
    if ( recvmsg.type.fetomw_type != lmonp_femw_usrdata )
      {
        LMON_say_msg(LMON_MW_MSG_PREFIX, true,
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
    	mwdata.daemon_data.unpack ( usrpl, recvmsg.usr_payload_length, udata );

#if VERBOSE
    LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
      "MW master: received USRDATA"); 
#endif
      }

  END_MASTER_ONLY

  //
  // synch-point
  //
  if ( LMON_mw_barrier () != LMON_OK )
    {
      LMON_say_msg(LMON_MW_MSG_PREFIX, true,
        "Barrier failed");
      return LMON_ESUBCOM;
    }

#if VERBOSE
  LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
    "MWs: returning from LMON_mw_handshake"); 
#endif

  return LMON_OK;
}


//! lmon_rc_e LMON_mw_ready
/*!

*/
extern "C" lmon_rc_e
LMON_mw_ready(void *udata)
{
  BEGIN_MASTER_ONLY(mwdata)
    lmonp_t *readymsg;

    if ( (udata != NULL) && (mwdata.daemon_data.pack != NULL) )
      {
    	char *uoffset;
        int upl_leng;

    	readymsg = (lmonp_t *) malloc (sizeof (lmonp_t)+LMON_MAX_USRPAYLOAD );

        set_msg_header (
                    readymsg,
                    lmonp_fetomw,
                    (int) lmonp_mwfe_ready,
                    0,0,0,0,0,0,
                    LMON_MAX_USRPAYLOAD);

    	uoffset = get_usrpayload_begin ( readymsg );

    	mwdata.daemon_data.pack ( udata,
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

    readymsg->msgclass = lmonp_fetomw;
    readymsg->type.fetomw_type = lmonp_mwfe_ready;
    readymsg->sec_or_jobsizeinfo.security_key1 = 0;
    readymsg->sec_or_stringinfo.security_key2 = 0;
    readymsg->lmon_payload_length = 0;
    write_lmonp_long_msg (servsockfd,
               readymsg,
               (sizeof (lmonp_t)
               +readymsg->lmon_payload_length  
               +readymsg->usr_payload_length));

#if VERBOSE
    LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
        "mw ready msg has been sent");
#endif

  END_MASTER_ONLY

  if ( LMON_mw_barrier() != LMON_OK )
    return LMON_ESUBCOM;

  return LMON_OK;
}


//! lmon_rc_e LMON_mw_finalize
/*!

*/
extern "C" lmon_rc_e
LMON_mw_finalize()
{
  int is_be = 0;
  if ( LMON_daemon_internal_finalize (is_be) != LMON_OK )
    return LMON_ESUBCOM;

#if VERBOSE
   LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
     "MW daemon finalized");
#endif

  if (mwdata.daemon_data.daemon_spawner)
    {
      delete mwdata.daemon_data.daemon_spawner;
    }

  return LMON_OK;
}


//! lmon_rc_e LMON_mw_recvUsrData
/*!

*/
extern "C" lmon_rc_e
LMON_mw_recvUsrData(void *udata)
{
  lmonp_t recvmsg;
  unsigned int bytesread;
  lmon_rc_e lrc = LMON_OK;

  BEGIN_MASTER_ONLY(mwdata)
    //
    // receiving the USRDATA stream from the frontend
    //
    if ( read_lmonp_msgheader ( servsockfd, &recvmsg ) < 0 )
      {
	LMON_say_msg(LMON_MW_MSG_PREFIX, true,
	  "read_lmonp return a neg value");

	lrc = LMON_ESYS;
        goto something_bad;
      }

    if ( recvmsg.type.fetomw_type != lmonp_femw_usrdata )
      {
	LMON_say_msg(LMON_MW_MSG_PREFIX, true,
          "the received message doesn't match with the expected type");

	lrc = LMON_EBDMSG;
        goto something_bad; 
      }

    int usrpayloadlen = recvmsg.usr_payload_length;
    if ( usrpayloadlen > 0 )
      {
    	char *usrpl = (char *) malloc ( recvmsg.usr_payload_length );
	if ( usrpl == NULL)
	  {
	    LMON_say_msg(LMON_MW_MSG_PREFIX, true, 
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
	if ( mwdata.daemon_data.unpack )
	  {
	    if (mwdata.daemon_data.unpack ( usrpl, recvmsg.usr_payload_length, udata ) < 0)
              {
	        LMON_say_msg(LMON_MW_MSG_PREFIX, true,
                "a negative return code from mwdata.daemon_data.unpack.");

                lrc = LMON_ENEGCB;
              }
          }
	else
	  {
	    LMON_say_msg(LMON_MW_MSG_PREFIX, true,
	      "mwdata.daemon_data.unpack has not been registered!  ");

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
  if ( LMON_mw_broadcast ( &lrc, sizeof(lrc) != LMON_OK ))
    {
      LMON_say_msg(LMON_MW_MSG_PREFIX, true,
        "Broadcast failed");

      lrc = LMON_ESUBCOM;
    }

    return lrc;
}


//! lmon_rc_e LMON_mw_sendUsrData
/*!

*/
extern "C" lmon_rc_e
LMON_mw_sendUsrData(void *udata)
{
  lmon_rc_e lrc = LMON_OK;

  BEGIN_MASTER_ONLY(mwdata)

    lmonp_t *usrmsg;

    if ( (udata != NULL) && (mwdata.daemon_data.pack != NULL) )
      {
    	char *uoffset;
        int upl_leng;
    	usrmsg = (lmonp_t *) malloc (sizeof (lmonp_t)+LMON_MAX_USRPAYLOAD );
	if ( usrmsg == NULL )
	  {
	    LMON_say_msg(LMON_MW_MSG_PREFIX, true, 
 	      "Out of memory");

	    lrc = LMON_ENOMEM;
            goto something_bad;
	  }
	
        set_msg_header (
                    usrmsg,
                    lmonp_fetomw,
                    (int) lmonp_mwfe_usrdata,
                    0,0,0,0,0,0,
                    LMON_MAX_USRPAYLOAD);

    	uoffset = get_usrpayload_begin ( usrmsg );
        if ( mwdata.daemon_data.pack ( udata, uoffset, LMON_MAX_USRPAYLOAD, &upl_leng ) < 0 )
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
	    LMON_say_msg(LMON_MW_MSG_PREFIX, true, 
	      "Out of memory");

	    lrc = LMON_ENOMEM;
            goto something_bad;
	  }
    	usrmsg->usr_payload_length = 0;
	lrc = LMON_ENCLLB;
      }

    usrmsg->msgclass = lmonp_fetomw;
    usrmsg->type.fetomw_type = lmonp_mwfe_usrdata;
    usrmsg->sec_or_jobsizeinfo.security_key1 = 0;
    usrmsg->sec_or_stringinfo.security_key2 = 0;
    usrmsg->lmon_payload_length = 0;
    if ( (write_lmonp_long_msg (servsockfd,
            usrmsg,
            sizeof (lmonp_t) + usrmsg->usr_payload_length ))
	  < 0 )
      {
	LMON_say_msg(LMON_MW_MSG_PREFIX, true,
          "write_lmonp returned a neg value");

	lrc = LMON_ESYS;
        goto something_bad;
      }
    free (usrmsg);

#if VERBOSE
    LMON_say_msg(LMON_BE_MSG_PREFIX, false,
        "Master MW daemon sent a user data");
#endif

  END_MASTER_ONLY

something_bad:
  //
  // duplicating the return code
  //
  if ( LMON_mw_broadcast ( &lrc, sizeof(lrc) != LMON_OK ))
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
        "Broadcast failed on LMON_mw_sendUsrData");

      lrc = LMON_ESUBCOM;
    }

  return lrc;
}


//! lmon_rc_e LMON_mw_regErrorCB
/*!
    registers a callback function that gets
    invoked whenever an error message should
    go out.
*/
extern "C"
lmon_rc_e
LMON_mw_regErrorCB (int (*func) (const char *format, va_list ap))
{
  if ( func == NULL)
    {
      LMON_say_msg ( LMON_MW_MSG_PREFIX, true,
        "an argument is invalid" );

      return LMON_EBDARG;
    }

  if ( errorCB !=  NULL )
    {
      LMON_say_msg ( LMON_MW_MSG_PREFIX, false,
        "previously registered error callback func will be invalidated" );
    }

  errorCB = func;

  return LMON_OK;
}
