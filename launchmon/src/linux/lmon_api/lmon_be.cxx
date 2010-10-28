/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/linux/lmon_api/lmon_be.cxx,v 1.12.2.5 2008/02/20 17:37:57 dahn Exp $
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

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/lmon_api_std.h>

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

#include "lmon_api/lmon_proctab.h"
#include "lmon_api/lmon_be.h"
#include "lmon_api/lmon_lmonp_msg.h"
#include "lmon_api/lmon_say_msg.hxx"
#include "lmon_be_internal.hxx"

#if RM_BG_MPIRUN
//
// CIOD debug interface is required so that 
// launchmon back-end API can provide the client 
// with a consistent MPI process state
// before transferring control back to the client.
//
// DHA 3/4/3009, reviewed. 
// Changed RM_BGL_MPIRUN to RM_BG_MPIRUN to genericize 
// BlueGene Support
//
#include "debugger_interface.h"
using namespace DebuggerInterface;
#endif


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
// LAUNCHMON BACKEND INTERNAL INTERFACE
//
//

static int 
LMON_return_ver ( )
{
  return LMON_VERSION;
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


//! lmon_rc_e LMON_be_getWhereToConnect
/*
    returns info on the LAUNCHMON front-end's 
    listening server socket. 
*/
static lmon_rc_e 
LMON_be_getWhereToConnect ( struct sockaddr_in *servaddr )
{
  char *portinfo;
  char *ipinfo;

  if (servaddr == NULL)
    {
      LMON_say_msg (LMON_BE_MSG_PREFIX, true,
        "servaddr is NULL");

      return LMON_EBDARG;
    }

  //
  // fetching the port number from the 
  // LMON_FE_WHERETOCONNECT_PORT
  // envVar which should have been sent by FE 
  //
  if ( (portinfo = getenv (LMON_FE_PORT_ENVNAME)) == NULL )
    {
      LMON_say_msg (LMON_BE_MSG_PREFIX, true, 
        "LMON_FE_PORT_ENVNAME envVar not found");

      return LMON_EINVAL;
    }

  //
  // fetching the port number from the
  // LMON_FE_WHERETOCONNECT_PORT
  // envVar which should have been sent by FE
  //
  if ( (ipinfo = getenv (LMON_FE_ADDR_ENVNAME)) == NULL )    
    {
      LMON_say_msg (LMON_BE_MSG_PREFIX, true, 
        "LMON_FE_ADDR_ENVNAME envVar not found");

      return LMON_EINVAL;
    }
  
  servaddr->sin_family = AF_INET;
  servaddr->sin_port = htons((uint16_t) atoi(portinfo));

  //
  // converting the text IP to the binary format
  //
  if ( inet_pton(AF_INET, (const char *) ipinfo, 
                 &(servaddr->sin_addr)) < 0 )
    {
      LMON_say_msg (LMON_BE_MSG_PREFIX, true, 
        "inet_pton failed");

      return LMON_ESYS;
    }

  return LMON_OK;
}


//! lmon_rc_e LMON_be_getSharedKeyAndID
/*
    returns info on authentication; 
    shared_key must be 128 bits
*/
static lmon_rc_e 
LMON_be_getSharedKeyAndID ( char *shared_key, int *id )
{
  char *sk;
  char *char_id;

  if ( shared_key == NULL )
    {
      LMON_say_msg (LMON_BE_MSG_PREFIX, true,
        "one of the arguments is NULL" );

      return LMON_EBDARG;
    }

  if ( (sk = getenv (LMON_SHRD_SEC_ENVNAME)) == NULL )
    {
      LMON_say_msg (LMON_BE_MSG_PREFIX, true, 
        "LMON_SHRD_SEC_ENVNAME envVar not found");

      return LMON_EINVAL;
    }

  sprintf (shared_key, "%s", sk);
  if ( (char_id = getenv (LMON_SEC_CHK_ENVNAME)) == NULL )
    {
      LMON_say_msg (LMON_BE_MSG_PREFIX, true,
        "LMON_SEC_CHK_ENVNAME envVar not found");

      return LMON_EINVAL;
    } 

  (*id) = atoi (char_id);

  return LMON_OK;
}

//! bool getPersonalityField ()
static bool
getPersonalityField ( std::string& pFilePath, std::string &fieldName, std::string &value)
{
  try
    {
      using namespace std;
      char line[PATH_MAX];
      ifstream ifs(pFilePath.c_str());
      string delim = "=";
      bool found = false;

      //
      // checking if pFilePath is valid 
      //
      if ( access(pFilePath.c_str(), R_OK) < 0)
        {
          LMON_say_msg (LMON_BE_MSG_PREFIX, true,
            "%s doesn't exist", pFilePath.c_str());
          return found;
        }

      //
      // ios_base::failure exception can be thrown
      //
      while (!ifs.getline (line, PATH_MAX).eof())
        {
          string strLine (line);
          string::size_type lastPos = strLine.find_first_not_of(delim, 0);
          string::size_type pos = strLine.find_first_of(delim, lastPos);

          if (pos != string::npos || lastPos != string::npos)
            {
              if ( fieldName == strLine.substr(lastPos, pos - lastPos)) 
                {
                  lastPos = strLine.find_first_not_of(delim, pos);
	          pos = strLine.find_first_of(delim, lastPos);
                  value = strLine.substr(lastPos, pos - lastPos);
                  found = true;
                  break;
                }
             } 
          }

      ifs.close();
      return found;
    }
  catch (std::ios_base::failure f)
   {
     LMON_say_msg (LMON_BE_MSG_PREFIX, true,
       "ios_base::failure exception thrown while parsing the personality file");
     return false;
   }
}


//! bool resolvHNAlias
/*
    Resolves an alias to an IP using the hosts file 
    whose path is given by the first argument. 
    This function can throw ios_base::failure
*/
static bool
resolvHNAlias ( std::string& hostsFilePath, std::string &alias, std::string &IP)
{
  try
    {
      using namespace std;
      char line[PATH_MAX];
      ifstream ifs(hostsFilePath.c_str());
      string delim = "\t ";
      bool found = false;

      //
      // checking if hostsFilePath is valid 
      //
      if ( access(hostsFilePath.c_str(), R_OK) < 0)
        {
          LMON_say_msg (LMON_BE_MSG_PREFIX, true,
            "%s doesn't exist", hostsFilePath.c_str()); 

          return found;
        }

      //
      // ios_base::failure exception can be thrown
      //
      while (!ifs.getline (line, PATH_MAX).eof())
        {
          string strLine (line);
          string::size_type lastPos = strLine.find_first_not_of(delim, 0);

          //
          // assuming a comment always occupies whole lines
          //
          //
          if (strLine[lastPos] == '#')
            continue;

          string::size_type pos = strLine.find_first_of(delim, lastPos);
          vector<string> tokens;
          vector<string>::const_iterator iter;

          while (pos != string::npos || lastPos != string::npos)
            {
              tokens.push_back(strLine.substr(lastPos, pos - lastPos));
              lastPos = strLine.find_first_not_of(delim, pos);
              // Find next "non-delimiter"
              pos = strLine.find_first_of(delim, lastPos);
              // Found a token, add it to the vector.
              //tokens.push_back(strLine.substr(lastPos, pos - lastPos));
            }

          for (iter = tokens.begin(); iter != tokens.end(); iter++)
            {
              if (*iter == alias)
                {
                  IP = tokens[0];
                  found = true;
                  break;
                }
            }
        }

      ifs.close();
      return found;
    }
  catch (std::ios_base::failure f)
   {
     LMON_say_msg (LMON_BE_MSG_PREFIX, true,
       "ios_base::failure exception thrown while parsing lines in the hosts file");
     return false;
   }
}

//////////////////////////////////////////////////////////////////////////////////
//
// LAUNCHMON BACKEND PUBLIC INTERFACE
//
//   (Following interfaces do not depend on the underlying 
//   native comm. fabric that launchmon BE leverages. 
//
//


      #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

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

#if VERBOSE && USE_VERBOSE_LOGDIR  
  //
  // DHA 3/4/2009, reviewed. Looks fine for BGP as well.
  //
  // Changed RM_BGL_MPIRUN to RM_BG_MPIRUN to genericize 
  // BlueGene Support
  //
  // DHA 02/04/2010 Moved this up within this LMON_be_init
  // call. Removed RM_BG_MPIRUN genericizing this support 
  // further

  char debugfn[PATH_MAX];
  char local_hostname[PATH_MAX];
  gethostname (local_hostname, PATH_MAX);
  snprintf (debugfn, PATH_MAX, VERBOSE_LOGDIR);

  if ( mkdir (debugfn, S_IRWXU) < 0 )
    {
      if ( errno != EEXIST )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "LMON BE fails to mkdir %s", debugfn );

          return LMON_EINVAL; 
        }
    }

  snprintf(debugfn, PATH_MAX, "%s/stdout.%d.%s", 
	VERBOSE_LOGDIR,getpid(),local_hostname);
  if ( freopen (debugfn, "w", stdout) == NULL )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "LMON BE fails to reopen stdout: %s", debugfn );

      return LMON_EINVAL; 
    }

  snprintf(debugfn, PATH_MAX, "%s/stderr.%d.%s", 
	VERBOSE_LOGDIR,getpid(),local_hostname);
  if ( freopen (debugfn, "w", stderr) == NULL )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "LMON BE fails to reopen stderr: %s", debugfn );

      return LMON_EINVAL;
    }
#elif RM_ALPS_APRUN
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
#endif

  if ( ver != LMON_return_ver() ) 
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "LMON BE API version mismatch" );

      return LMON_EINVAL;
    }

  set_client_name(LMON_BE_MSG_PREFIX);

  //
  // Registering bedata.my_hostname and bedata.my_ip
  //
#if RM_BG_MPIRUN
  //
  // BLUEGENE/L's RPDTAB contains raw IPs instead of hostnames.
  // 
  // DHA 3/4/3009, reviewed. Looks fine for the DAWN configuration
  // /etc/hosts shows that I/O network is detnoted as FENname-io
  // following the same convention. 
  //
  // Changed RM_BGL_MPIRUN to RM_BG_MPIRUN to genericize BlueGene Support
  //
  memset ( bedata.my_hostname, 0, LMON_BE_HN_MAX );
  memset ( bedata.my_ip, 0, LMON_BE_HN_MAX );
    
  if ( gethostname ( bedata.my_hostname, LMON_BE_HN_MAX ) < 0 )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
        "gethostname failed");
    
      return LMON_ESYS;
    }
    
  struct hostent *hent = gethostbyname(bedata.my_hostname);
  if ( hent == NULL )
    { 
      std::string resIP;
      std::string pFile("/proc/personality.sh");
      std::string fieldStr("BG_IP");

      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
        "BES: this machine does not know how to resolve %s into an IP",
        bedata.my_hostname);
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
        "BES: parsing /proc/personality.sh to try to resolve");
     
      if (!getPersonalityField (pFile, fieldStr, resIP))
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,  
            "BES: getPersonalityField also failed to resolve %s through %s", 
	    bedata.my_hostname, fieldStr.c_str());

          return LMON_ESYS;
        }
      snprintf(bedata.my_ip, LMON_BE_HN_MAX, "%s", 
        resIP.c_str());
      snprintf(bedata.my_hostname, LMON_BE_HN_MAX, "%s", 
        resIP.c_str());

    }
  else
    {
      if ( inet_ntop (AF_INET, (void *)hent->h_addr, bedata.my_hostname, LMON_BE_HN_MAX-1 ) == NULL )
        {
          LMON_say_msg(LMON_BE_MSG_PREFIX, true,
          "inet_ntop failed");

          return LMON_ESYS;
        }
    }

# if VERBOSE
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
    "BES: inet_ntop converted host name representation from binary to text: %s", bedata.my_hostname);
# endif
#else /* RM_BG_MPIRUN */
  memset ( bedata.my_hostname, 0, LMON_BE_HN_MAX );
  if ( gethostname ( bedata.my_hostname, LMON_BE_HN_MAX ) < 0 )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
        "gethostname failed");

      return LMON_ESYS;
    }
#endif /* RM_BG_MPIRUN */

  if ( LMON_be_internal_init ( argc, argv, bedata.my_hostname ) != LMON_OK )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true,
        "LMON_be_internal_init failed");
     
      return LMON_ESUBCOM;
    }

  if ( LMON_be_getMyRank ( &(bedata.myrank) ) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "LMON BE getMyRank failed" );

      return LMON_ESUBCOM;
    }

  if ( LMON_be_getSize ( &(bedata.width ) ) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "LMON BE getSize failed" );

      return LMON_ESUBCOM;
    }

  bedata.proctab_msg      = NULL;
  bedata.proctab_msg_size = 0;
  bedata.pack             = NULL;
  bedata.unpack           = NULL;
 
  BEGIN_MASTER_ONLY
    /*
     *
     * The backend master initiates a handshake with FE
     *
     */
    int i;
    lmonp_t initmsg;   
    char shared_key[LMON_KEY_LENGTH];
    unsigned char sessID[LMON_KEY_LENGTH];
    unsigned char* sid_traverse;
    int32_t intsessID;
    gcry_cipher_hd_t cipher_hndl;
    gcry_error_t gcrc;

    if ( LMON_be_internal_getConnFd (&servsockfd) != LMON_OK )
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
	if ( (lrc = LMON_be_getWhereToConnect ( &servaddr ) ) != LMON_OK )
	  {
	    LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
	      "LMON_be_getWhereToConnect failed ");

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
    if ( (lrc = LMON_be_getSharedKeyAndID ( shared_key,
		  &intsessID) ) != LMON_OK)
      {
	LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
    	  "LMON_be_getSharedKeyAndID failed ");

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
    
    for ( i = 0; i < 4; i++ )
      {
	unsigned int tmpSK;
	memcpy ( (void *) &tmpSK, 
		 sid_traverse,
		 4 );
	set_msg_header ( &initmsg,
			 lmonp_fetobe,
			 lmonp_febe_security_chk,
			 0,                    /* security_key1 */
			 (unsigned int)tmpSK,  /* security_key2 */
			 0,0,0,0,0 );   
	if ( ( write_lmonp_long_msg ( servsockfd, 
				      &initmsg, 
				      sizeof (initmsg) )) < 0 )
	  {
	    LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
	      "write_lmonp_long_msg failed");

	    return LMON_ESYS;
	  }

	sid_traverse += 4;
      } // for (i=0; i < 4; i++ )
   
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
    	"barrier failed" );
	
      return LMON_ESUBCOM;
    }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BES: exiting be_init");
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
  if ( bedata.myrank < 0 )
    return LMON_EINVAL;
  
  return (( bedata.myrank == 0) ? LMON_YES : LMON_NO);
}


//! lmon_rc_e LMON_be_getMyRank
/*!
    Please refer to the header file: lmon_be.h
*/
extern "C" 
lmon_rc_e 
LMON_be_getMyRank ( int *rank )
{
  if ( LMON_be_internal_getMyRank (rank) != LMON_OK ) 
    return LMON_EINVAL;

  return LMON_OK;
}


//! lmon_rc_e LMON_be_getSize
/*   
    Please refer to the header file: lmon_be.h 
*/
extern "C" 
lmon_rc_e 
LMON_be_getSize ( int* size )
{
  if ( LMON_be_internal_getSize (size) != LMON_OK ) 
    return LMON_EINVAL;

  return LMON_OK;
}


//! lmon_rc_e LMON_be_barrier()
/*
    Please refer to the header file: lmon_be.h 
*/
extern "C" 
lmon_rc_e 
LMON_be_barrier ()
{
  if ( LMON_be_internal_barrier () != LMON_OK )
    return LMON_ESUBCOM;
  
  return LMON_OK;
}


//!lmon_rc_e LMON_be_broadcast
/* 
   Please refer to the header file: lmon_be.h 
*/
extern "C"
lmon_rc_e
LMON_be_broadcast ( void *buf, int numbyte ) 
{
  if ( LMON_be_internal_broadcast ( buf, numbyte ) != LMON_OK )
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
	         void* recvbuf )
{
  if ( LMON_be_internal_gather ( sendbuf, 
         numbyte_per_elem, recvbuf ) != LMON_OK )
    return LMON_ESUBCOM;
  
  return LMON_OK;
}


//!lmon_rc_e LMON_be_scatter
/* 
   Please refer to the header file: lmon_be.h 
*/
extern "C"
lmon_rc_e 
LMON_be_scatter (
		void *sendbuf,
		int numbyte_per_element,
		void *recvbuf )
{
  if ( LMON_be_internal_scatter ( sendbuf, 
         numbyte_per_element, recvbuf ) != LMON_OK )
    return LMON_ESUBCOM;

  return LMON_OK;      
}


//! LMON_be_finalize();
/*
    Please refer to the header file: lmon_be.h 
*/
extern "C"
lmon_rc_e 
LMON_be_finalize ()
{
  if ( LMON_be_internal_finalize () != LMON_OK )
    return LMON_ESUBCOM;

  return LMON_OK;
}


//! lmon_rc_e LMON_be_handshake
/*
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

  BEGIN_MASTER_ONLY
    hngatherbuf = (char *) malloc ( LMON_BE_HN_MAX*bedata.width );
    if ( hngatherbuf == NULL )
      {
	LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
      	  "malloc returned zero");

	return LMON_ENOMEM;
      }
#if VERBOSE
	LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
         "BE master: alloc buf of size %d x %d", 
         LMON_BE_HN_MAX, bedata.width);
#endif
  END_MASTER_ONLY

  //
  // once be_gather is performed, hngatherbuf 
  // should hold all the hostnames
  //
  if ( LMON_be_gather ( bedata.my_hostname, LMON_BE_HN_MAX, hngatherbuf ) != LMON_OK )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
        "gather failed");

      return LMON_ESUBCOM;
    }
#if VERBOSE
	LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
          "BES: host names are gathered");
#endif
  BEGIN_MASTER_ONLY

    map<string, unsigned int> hostName;     
    lmonp_t *sendbuf;
    int sendbufsize;
    unsigned int offset = 0;
    char *hntrav = NULL;
    hntrav = hngatherbuf; // hntrav will traverse the gathered hostnames   

    for ( i=0; i < bedata.width ; ++i )
      {
    	//
    	// the first pass to compute the size of the string table
    	//
    	string tmpstr ( hntrav );
    	if ( hostName.find (tmpstr) == hostName.end ())
	  {
    	    hostName[tmpstr] = offset;
    	    offset += (strlen (hntrav) + 1 );
	  }
    	 hntrav += LMON_BE_HN_MAX;
       }

    //
    // sendbuf alloc and message header work
    //
    sendbufsize = sizeof (lmonp_t)
                + bedata.width * sizeof(int)
                + offset;
    sendbuf = (lmonp_t *) malloc ( sendbufsize );
    if (bedata.width < LMON_NTASKS_THRE)
      {
        set_msg_header (
                sendbuf,
                lmonp_fetobe,
                (int) lmonp_befe_hostname,
                (unsigned short) bedata.width,
                0,
                0,
                bedata.width,
                0,
                offset+bedata.width*sizeof(int),
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
                bedata.width,
                bedata.width,
                offset+bedata.width*sizeof(int),
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
    char *hntrav2   = hngatherbuf;      
    for ( i=0; i < bedata.width; ++i )
      {
    	string tmpstr (hntrav2);
    	memcpy ((void*) packptr, 
    		(void*) &(hostName[tmpstr]),
    		sizeof (unsigned int));
    	packptr += sizeof (unsigned int);
    	memcpy ((void*) strtabptr,
    		(void*) hntrav2,
    		( strlen (hntrav2) + 1 ) );
    	strtabptr += ( strlen (hntrav2) + 1 );
    	hntrav2 += LMON_BE_HN_MAX;		   
      }
    free ( hngatherbuf );

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: about to write the hosts list to the FE");
#endif
    
    //
    // shipping it out and free the message buffer
    //
    write_lmonp_long_msg ( 
    		servsockfd, 
    		sendbuf, 
    		sendbufsize );
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
	  "Unexpected message type");

	return LMON_EBDMSG;
      }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BE master: received RPDTAB message header: lmon payload %d", 
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
    	bedata.unpack ( usrpl, recvmsg.usr_payload_length, udata );

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
	"Broadcast failed"); 

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

  BEGIN_SLAVE_ONLY
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
  if ( LMON_be_broadcast ( bedata.proctab_msg, 
         bedata.proctab_msg_size ) != LMON_OK )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	"Broadcast failed"); 
      return LMON_ESUBCOM;
    }

#if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BES: proctab_msg Bcasted"); 
#endif

  //
  // Depending on the architecture, we need to enforce 
  // launchmon's semantics for the job state, which are
  // A. the job loaded and stopped for the launch case
  // B. the job continues runninng for the attach case. 
  //

  MPIR_PROCDESC_EXT* proctab;
  int proctab_size;

  if ( LMON_be_getMyProctabSize(&proctab_size) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "My proctable size is unavailable");

      return LMON_EDUNAV;
    }

  proctab = (MPIR_PROCDESC_EXT *)
        malloc (proctab_size*sizeof(MPIR_PROCDESC_EXT));

  if ( proctab == NULL )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "Out of memory");

      return LMON_ENOMEM;
    }

  if ( LMON_be_getMyProctab (proctab, 
         &proctab_size, proctab_size) != LMON_OK )
    {
      LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
        "My proctable is unavailable");
                                                                                                                                                          
      return LMON_EDUNAV;
    }

#if RM_SLURM_SRUN 
  //
  // SLURM leaves the app processes in the SIGSTOP'ed STATE
  // trm_launch and trm_attach are noop
  //
  if (bedata.is_launch == trm_launch_dontstop)
    {
      for (i=0; i < proctab_size; i++)
        kill(proctab[i].pd.pid, SIGCONT);
    }
  else if (bedata.is_launch == trm_attach_stop)
    {
      for (i=0; i < proctab_size; i++)
        kill(proctab[i].pd.pid, SIGSTOP);
    }
#elif RM_ORTE_ORTERUN || RM_ALPS_APRUN
  //
  // ORTE and ALPS don't not leave the app processes
  // in the SIGSTOP'ed state It uses a barrier mechanism 
  // to prevent the app from running away. So we explicit send
  // a SIGSTOP on trm_launch. And NOOP on trm_launch_dontstop 
  // 
  if (bedata.is_launch == trm_launch)
    {
      for (i=0; i < proctab_size; i++)
        kill(proctab[i].pd.pid, SIGSTOP);
    }
  else if (bedata.is_launch == trm_attach_stop)
    {
      for (i=0; i < proctab_size; i++)
        kill(proctab[i].pd.pid, SIGSTOP);
    }
#elif RM_BG_MPIRUN
  /*
   * In the case of BlueGene, we want to register ATTACH msgs here
   * so that the job will stop when loaded. This can minimize the
   * chance where a race condition can occur: if the job gets released
   * too early (by FEN's continuing mpirun off of MPIR_Breakpoint)
   * and thus loaded before LMON_be_ready.
   *
   */

  // DHA 3/4/3009, reviewed. 
  // Change the message type to BG_Debugger_Msg from BGL_Debugger_Msg.
  // Changed RM_BGL_MPIRUN to RM_BG_MPIRUN to genericize 
  // BlueGene Support
  //

  for (i=0; i < proctab_size; i++)
    {
      BG_Debugger_Msg dbgmsg(ATTACH,proctab[i].pd.pid,0,0,0);
      BG_Debugger_Msg ackmsg;
      BG_Debugger_Msg ackmsg2;
      dbgmsg.header.dataLength = sizeof(dbgmsg.dataArea.ATTACH);

      if ( !BG_Debugger_Msg::writeOnFd (BG_DEBUGGER_WRITE_PIPE, dbgmsg) )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "ATTACH command failed.");

          return LMON_EINVAL;
        }
      if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg) )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "ATTACH_ACK failed.");

          return LMON_EINVAL;
        }
      if ( ackmsg.header.messageType != ATTACH_ACK)
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "readFromFd received a wrong msg type: %d.",
              ackmsg.header.messageType);

          return LMON_EINVAL;
        }

      if ( ackmsg.header.nodeNumber != (unsigned int) proctab[i].pd.pid )
        {
          LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
            "ATTACH_ACK contains a wrong nodeNumber");

          return LMON_EINVAL;
        }
    }
    
  if (bedata.is_launch == trm_launch 
      || bedata.is_launch == trm_launch_dontstop)
    {
      for (i=0; i < proctab_size; i++)
        {
          //
          // For plain launch case, we leave app processes in a stopped state
          //
          BG_Debugger_Msg ackmsg;

          if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg) )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "SIGNAL_ENCOUNTERED");

              return LMON_EINVAL;
            }
          if ( ackmsg.header.messageType != SIGNAL_ENCOUNTERED )
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "readFromFd received a wrong msg type: %d.",
                  ackmsg.header.messageType);

              return LMON_EINVAL;
            }
        }
# if VERBOSE
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
        "BES: trm_launch case is handled"); 
# endif
      if (bedata.is_launch == trm_launch_dontstop)
        {
          //
          // continue
          //  
          for (i=0; i < proctab_size; i++)
           {
             BG_Debugger_Msg dbgmsg(CONTINUE,proctab[i].pd.pid,0,0,0);
             BG_Debugger_Msg ackmsg;
             dbgmsg.dataArea.CONTINUE.signal = SIGCONT;
             dbgmsg.header.dataLength = sizeof(dbgmsg.dataArea.CONTINUE);

             if ( !BG_Debugger_Msg::writeOnFd (BG_DEBUGGER_WRITE_PIPE, dbgmsg ))
               {
                 LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                  "writeOnFd for CONTINUE.\n");

                 return LMON_EINVAL;
               }
             if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg ))
               {
                 LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                   "readFromFd for CONTINUE ACK.\n");

                 return LMON_EINVAL;
               }
             if ( ackmsg.header.messageType != CONTINUE_ACK)
               {
                 LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                   "msg type isn't CONTINUE ACK.\n");

                 return LMON_EINVAL;
               }
             if ( ackmsg.header.nodeNumber != (unsigned int) proctab[i].pd.pid )
               {
                 LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                   "Incorrect pid in the returned debug msg.\n");

                 return LMON_EINVAL;
               } 
           }
# if VERBOSE
         LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
           "BES: trm_launch_dontstop case is handled");
# endif
        }
    }
  else if (bedata.is_launch == trm_attach_stop)
    {
      //
      // For attach_stop, we attach and leave those processes in a stopped state
      //
      for (i=0; i < proctab_size; i++)
        {
          BG_Debugger_Msg dbgmsg(KILL,proctab[i].pd.pid,0,0,0);
          dbgmsg.dataArea.KILL.signal = SIGSTOP;
          dbgmsg.header.dataLength = sizeof(dbgmsg.dataArea.KILL);
          BG_Debugger_Msg ackmsg;
          BG_Debugger_Msg ackmsg2;

          if ( !BG_Debugger_Msg::writeOnFd (BG_DEBUGGER_WRITE_PIPE, dbgmsg ))
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "writeOnFd for KILL.\n");
              return LMON_EINVAL;
            }
          if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg ))
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "readFromFd for KILL ACK.\n");
              return LMON_EINVAL;
            }
          if ( ackmsg.header.messageType != KILL_ACK)
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "msg type isn't KILL ACK.\n");
              return LMON_EINVAL;
            }
          if ( !BG_Debugger_Msg::readFromFd (BG_DEBUGGER_READ_PIPE, ackmsg2 ))
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "readFromFd for SINGANL ENCOUNTERED ACK.\n");
              return LMON_EINVAL;
            }
          if ( ackmsg2.header.messageType != SIGNAL_ENCOUNTERED)
            {
              LMON_say_msg ( LMON_BE_MSG_PREFIX, true,
                "msg type isn't KILL ACK.\n");
              return LMON_EINVAL;
            }
	}
# if VERBOSE
      LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
        "BES: trm_attach_stop case is handled"); 
# endif
     }

# if VERBOSE
    LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
      "BES: primed the job for next operations"); 
# endif
#endif /* RM_BG_MPIRUN */

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

  //
  // synch-point
  //
  if ( LMON_be_barrier () != LMON_OK )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	"Barrier failed"); 
      return LMON_ESUBCOM;
    }

#if VERBOSE
  LMON_say_msg ( LMON_BE_MSG_PREFIX, false,
    "BES: returning from LMON_be_handshake"); 
#endif
 
  return LMON_OK;
}


//! lmon_rc_e LMON_be_ready()
/*
    Please refer to the header file: lmon_be.h 
*/
extern "C"
lmon_rc_e 
LMON_be_ready ( void *udata )
{  
  BEGIN_MASTER_ONLY
    lmonp_t *readymsg;
  
    if ( (udata != NULL) && (bedata.pack != NULL) )
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
    	bedata.pack ( udata,  
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
    readymsg->type.fetobe_type = lmonp_be_ready;
    readymsg->sec_or_jobsizeinfo.security_key1 = 0;
    readymsg->sec_or_stringinfo.security_key2 = 0;
    readymsg->lmon_payload_length = 0;
    write_lmonp_long_msg ( 
    		servsockfd, 
    		readymsg, 
    		sizeof (lmonp_t) + readymsg->usr_payload_length );
    free (readymsg);
    
  END_MASTER_ONLY

  if ( LMON_be_barrier() != LMON_OK )
    return LMON_ESUBCOM;

  return LMON_OK;
}


//! lmon_rc_e LMON_be_getMyProctab
/*
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

  key = bedata.my_hostname;

  string myhostname (key);
  (*size) = (int) proctab_cache[myhostname].size ();
  map<string, vector<MPIR_PROCDESC_EXT* > >::const_iterator viter 
    = proctab_cache.find (myhostname);  

  for ( i=0; (i < (*size)) && (i < proctab_num_elem); ++i )
    {
      proctabbuf[i].pd.executable_name = strdup ((*viter).second[i]->pd.executable_name);
      proctabbuf[i].pd.host_name = strdup ((*viter).second[i]->pd.host_name);
      proctabbuf[i].pd.pid = (*viter).second[i]->pd.pid;
      proctabbuf[i].mpirank = (*viter).second[i]->mpirank;
    }

  if ( ( i == proctab_num_elem ) && ( proctab_num_elem < (*size) ) )
    return LMON_EINVAL;

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

  key = bedata.my_hostname;

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
  if ( bedata.pack != NULL )
  {
    LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
      "pack has already been registered or has not been initialized");  
    LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
      "replacing the pack function...");  
  }
  
  bedata.pack = packBefe;

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
  if ( bedata.unpack != NULL )
    {
      LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
        "pack has already been registered or has not been initialized");  
      LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
        "replacing the pack function...");  	  
    }
	  
  bedata.unpack = unpackFebe;
  
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

  BEGIN_MASTER_ONLY
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
	if ( bedata.unpack )	  
	  {
	    if (bedata.unpack ( usrpl, recvmsg.usr_payload_length, udata ) < 0)	  
              {
	        LMON_say_msg(LMON_BE_MSG_PREFIX, true,
                "a negative return code from bedata.unpack.");
	   
                lrc = LMON_ENEGCB;
              }
          }
	else
	  {
	    LMON_say_msg(LMON_BE_MSG_PREFIX, true, 
	      "bedata.unpack has not been registered!  ");

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

  BEGIN_MASTER_ONLY
    
    lmonp_t *usrmsg;
  
    if ( (udata != NULL) && (bedata.pack != NULL) )
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
        if ( bedata.pack ( udata, uoffset, LMON_MAX_USRPAYLOAD, &upl_leng ) < 0 )
          {
	    lrc = LMON_ENEGCB;
	    goto something_bad;
          }

    	assert ( upl_leng <= LMON_MAX_USRPAYLOAD );
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
