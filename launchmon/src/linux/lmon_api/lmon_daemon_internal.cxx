/*
 * $Header: dahn Exp $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008 ~ 2012, Lawrence Livermore National Security, LLC. Produced at 
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
 *
 *  Note:
 *        LMON BE|MW API is designed to leverage any efficient underlying
 *        communication infrastructure on a platform. This file contains
 *        the codes that depend upon this aspect.
 *
 *
 *  Update Log:
 *        Aug  02 2010 DHA: Changed the file name to lmon_daemon_internal.cxx
 *                          to support middleware daemons
 *        Oct  07 2011 DHA: Deprecated PMGR Collective support
 *        May  11 2010 DHA: Added MEASURE_TRACING_COST for the PMGR layer
 *        Feb  05 2010 DHA: Added pmgr_register_hname to register the local 
 *                          hostname to the PMGR Collective layer. This is to work 
 *                          around the system in which gethostname followed by
 *                          gethostbyname doesn't resolve the hostname into
 *                          an IP.
 *        Dec  23 2009 DHA: Added explict config.h inclusion
 *        Dec  16 2008 DHA: Added COBO support
 *        Mar  26 2008 DHA: Added the END_DEBUG message support
 *                          for BlueGene with a debugger protocol version >= 3
 *        Mar  14 2008 DHA: Added PMGR Collective support. 
 *                          The changes are contained in the PMGR_BASED 
 *                          macro.
 *        Feb  09 2008 DHA: Added LLNS Copyright
 *        Jul  25 2007 DHA: bracket MPI dependent codes using
 *                          LMON_BE_MPI_BASED macro.
 *        Mar  16 2007 DHA: Added MPI_Allreduce support
 *        Dec  29 2006 DHA: Created file. Most routines are lifted 
 *                          from lmon_be.cxx
 */

#include "sdbg_std.hxx"

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
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

#include <vector>

extern "C" {
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
}

#include "lmon_api/lmon_api_std.h"
#include "lmon_api/lmon_proctab.h"
#include "lmon_api/lmon_be.h"
#include "lmon_api/lmon_lmonp_msg.h"
#include "lmon_api/lmon_say_msg.hxx"
#include "lmon_daemon_internal.hxx"


#if MPI_BASED
extern "C" {
#include <mpi.h>
}
#elif COBO_BASED
extern "C" {
#include <cobo.h>
}
#endif

static int ICCL_rank      = -1;
static int ICCL_size      = -1;
static int ICCL_global_id = -1;
static per_be_data_t *bedataPtr = NULL;


//////////////////////////////////////////////////////////////////////////////////
//
// LAUNCHMON BACKEND INTERNAL INTERFACE
//
//
//

lmon_rc_e
LMON_daemon_internal_tester_init ( per_be_data_t *d )
{
  if (!d)
    {
      return LMON_EINVAL;
    } 

  bedataPtr = d;
  return LMON_OK;
}


lmon_rc_e
LMON_daemon_internal_tester_getBeData ( per_be_data_t **d )
{
  if (!bedataPtr)
    {
      return LMON_EINVAL;
    }

  (*d) = bedataPtr;
  return LMON_OK;
}


//! LMON_daemon_internal_init(int* argc, char*** argv)
/*!

*/
lmon_rc_e
LMON_daemon_internal_init ( int* argc, char*** argv, char *myhn, int is_be )
{
  int rc;
  char **nargv = *argv;
  int n_lmonopt = 0;
  int i;

  /*
   * The following code assumes lmon's options are specified after all other
   * options (e.g., PMGR's ) are given.
   */
  for (i=1; i < (*argc); ++i) {
    if ( (nargv[i][0] == '-') && (nargv[i][1]) ) {
      if ( strncmp (&(nargv[i][2]), "lmonsharedsec=", 14)== 0 )
	{
	   /* exporting a commandline option to an envVar */
           setenv (TOOL_SS_ENV, &(nargv[i][2])+14, 1); 
	   n_lmonopt++;
	}
      else if ( strncmp (&(nargv[i][2]), "lmonsecchk=", 11)== 0 )
	{
	  /* exporting a commandline option to an envVar */
          setenv (TOOL_SCH_ENV, &(nargv[i][2])+11, 1); 
	  n_lmonopt++;
	}
      }
    } /* for */

  if (n_lmonopt != 2) {
    LMON_say_msg(LMON_BE_MSG_PREFIX, true,
                 "LaunchMON-specific arguments have not been passed "
                 "to the daemon through the command-line arguments.");
    LMON_say_msg(LMON_BE_MSG_PREFIX, true,
                 "the command line that the user provided "
                 "could have been truncated.");
  }

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

#if MPI_BASED
  /*
   * with Message Passing Interface
   */
  if ( ( rc = MPI_Init (argc, argv)) < 0 )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
        " MPI_Init failed");
      return LMON_EINVAL;
    }
  //
  // register an MPI error handler to change the behavior of 
  // MPI calls such that they return on failure
  //
  MPI_Errhandler_set (MPI_COMM_WORLD, MPI_ERRORS_RETURN);
  MPI_Comm_size (MPI_COMM_WORLD, &ICCL_size);
  MPI_Comm_rank (MPI_COMM_WORLD, &ICCL_rank);
  ICCL_global_id = ICCL_rank;

#elif COBO_BASED

# if MEASURE_TRACING_COST
  /* Hack, but the MEASURE_TRACING_COST macro doesn't 
   * propagate to the iccl layer 
   * so we use the global variable trick here
   */
  __cobo_ts = gettimeofdayD();
# endif

   int iccl_begin_port, iccl_tmp_session;

   //
   //TODO: session id=10 for now.
   //
   iccl_begin_port = (is_be) ? COBO_BEGIN_PORT : COBO_BEGIN_PORT + COBO_PORT_RANGE;
   iccl_tmp_session = (is_be) ? 10 : 11;

   int j;
   int *portlist = (int *) malloc (COBO_PORT_RANGE * sizeof(int));

   if (portlist == NULL)
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
        "malloc returned NULL");

      return LMON_EINVAL;
    }

   for (j=0; j < COBO_PORT_RANGE; ++j)
     portlist[j] = iccl_begin_port + j;

   if ( ( rc = cobo_open (iccl_tmp_session,
                          portlist,
                          COBO_PORT_RANGE,
                          &ICCL_rank, &ICCL_size) )
            != COBO_SUCCESS )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
        "cobo_open failed");

      return LMON_EINVAL;
    }

  free(portlist);

# if VERBOSE
    LMON_say_msg(LMON_DAEMON_MSG_PREFIX, false,
      "My rank is %d, My size is %d", ICCL_rank, ICCL_size);
# endif

# if MEASURE_TRACING_COST
#  if VERBOSE
    LMON_say_msg(LMON_DAEMON_MSG_PREFIX, false,
      "__cobo_ts: %f", __cobo_ts);
#  endif
    if (ICCL_size <= 0)
      {
        LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
          "cobo_open failed; checked during MEASURE_TRACING_COST");

        return LMON_EINVAL;
      }

    double *tsvector = NULL;
    if (ICCL_rank == 0)
      {
        tsvector = (double *) malloc (ICCL_size * sizeof(double));
      }

    if ( ( rc = cobo_gather(&__cobo_ts, sizeof(__cobo_ts), tsvector, 0)) != COBO_SUCCESS )
      {
        LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
          "cobo_gather failed during MEASURE_TRACING_COST");

        return LMON_EINVAL;
      }

    if (ICCL_rank == 0)
      {
        long rix;
        double maxts=0.0f;
        int parentfd = -1;
        for ( rix=0; rix < ICCL_size; ++rix) 
          {
            maxts = (tsvector[rix] > maxts) ? tsvector[rix] : maxts; 
          }
        if ( cobo_get_parent_socket(&parentfd) != COBO_SUCCESS )
          {
            LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
	      "cobo_client_get_parent_socket failed during MEASURE_TRACING_COST");

            return LMON_EINVAL;
          } 
#  if VERBOSE
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, false,
        "sending maxts: %f", maxts);
#  endif

       if (lmon_write_raw(parentfd, &maxts, sizeof(maxts)) < 0 )
         {
            LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
              "write failed during MEASURE_TRACING_COST");

            return LMON_EINVAL;
         }
      }
# endif

#else
  LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,"no internal communication fabric to leverage");
  return LMON_EINVAL;
#endif

  if ( (ICCL_rank == -1) || (ICCL_size == -1 ) )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
        "ICCL_rank and/or ICCL_size have not been assigned");

      return LMON_EINVAL;
    }

  return LMON_OK;
}


//! lmon_rc_e LMON_daemon_internal_getConnFd
/*!

*/
lmon_rc_e
LMON_daemon_internal_getConnFd ( int *fd )
{
#if COBO_BASED
  if ( ICCL_rank == 0)
    {
      if ( cobo_get_parent_socket (fd) != COBO_SUCCESS )
        {
          LMON_say_msg(LMON_DAEMON_MSG_PREFIX, false,"no connection estabilished with FE");
          return LMON_EDUNAV;
        }
      return LMON_OK;
    }
  else
    {
      return LMON_EDUNAV;
    }
#else
  return LMON_EDUNAV;
#endif
}


//! lmon_rc_e LMON_daemon_internal_getMyRank
/*!
  Returns my rank thru rank argument.
        Returns: LMON_OK if OK, LMON_EINVAL on error
*/
lmon_rc_e
LMON_daemon_internal_getMyRank ( int *rank )
{
  if ( ICCL_rank < 0 )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
        "ICCL_rank has not been filled." );

      return LMON_EINVAL;
    }

  (*rank) = ICCL_rank; 

  return LMON_OK;
}


//! lmon_rc_e LMON_deamon_internal_getSize
/*!
  Returns the number of backend daemons thru size argument.
        Returns: LMON_OK if OK, LMON_EINVAL on error
*/
lmon_rc_e 
LMON_daemon_internal_getSize ( int *size )
{
  if ( ICCL_size < 0 )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
        "ICCL_size has not been filled." );

      return LMON_EINVAL;
    }

  (*size) = ICCL_size;

  return LMON_OK;
}


//! lmon_rc_e LMON_daemon_internal_barrier()
/*!
  Barrier call for tool daemons.
*/
lmon_rc_e
LMON_daemon_internal_barrier ()
{
  int rc;

#if MPI_BASED
  if ( (rc = MPI_Barrier(MPI_COMM_WORLD)) < 0 )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
        "MPI_Barrier failed ");

      return LMON_EINVAL;
    }
#elif COBO_BASED
  if ( (rc = cobo_barrier ()) != COBO_SUCCESS )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
        "pmgr_barrier failed ");

      return LMON_EINVAL;
    }
#else
  LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,"no internal comm fabric to leverage");
  return LMON_EINVAL;
#endif

  return LMON_OK;
}


//!lmon_rc_e LMON_daemon_internal_broadcast
/*!
   This call restricts the root of the broadcast call to the master 
   backend daemon and the width of the communicator to be "all daemons"

   This call is also datatype agnostic. It sends buf as a BYTE stream.
 */
lmon_rc_e 
LMON_daemon_internal_broadcast ( void *buf, int numbyte ) 
{
  int rc;

#if MPI_BASED
  if ( (rc = MPI_Bcast ( buf,
                         numbyte,
                         MPI_BYTE,
                         LMON_BE_MASTER,
                         MPI_COMM_WORLD)) < 0 )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true," MPI_Bcast failed");

      return LMON_EINVAL;
    }
#elif COBO_BASED
  if ( (rc = cobo_bcast( buf,
                         numbyte,
                         LMON_DAEMON_MASTER ))
       != COBO_SUCCESS )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true," cobo_bcast failed");

      return LMON_EINVAL;
    }
#else
  LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,"no internal comm fabric to leverage");
  return LMON_EINVAL;
#endif

  return LMON_OK;
}


//!lmon_rc_e LMON_daemon_internal_gather 
/*!
   Gathers data from all tool daemons and returns it via recvbuf 
   of the master tool daemon. The data must be in contiguous memory and
   be constant size across all backend daemons. FIXME:
*/
lmon_rc_e
LMON_daemon_internal_gather (
                void *sendbuf,
                int numbyte_per_elem,
                void *recvbuf )
{
  int rc;

#if MPI_BASED
  rc = MPI_Gather (sendbuf,
                   numbyte_per_elem,
                   MPI_BYTE,
                   recvbuf,
                   numbyte_per_elem,
                   MPI_BYTE,
                   LMON_BE_MASTER,
                   MPI_COMM_WORLD);

  if (rc < 0 )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,"MPI_Gather failed");

      return LMON_EINVAL;
    }
#elif COBO_BASED
  rc = cobo_gather (sendbuf,
                    numbyte_per_elem, 
                    recvbuf,
                    LMON_DAEMON_MASTER);
  if (rc != COBO_SUCCESS )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,"cobo_gather failed");

      return LMON_EINVAL;
    }
#else
  LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,"no internal comm fabric to leverage");
  return LMON_EINVAL;
#endif

  return LMON_OK;
}


//!lmon_rc_e LMON_daemon_internal_scatter
/*!
   Scatter data to all tool daemons. Each daemon receives its portion
   via recvbuf. sendbuf is only meaningful to the source.
   The data type of send buf must be in contiguous memory and
   packed so that every daemon receives the constant size per-daemon data.
*/
lmon_rc_e
LMON_daemon_internal_scatter (
                void *sendbuf,
                int numbyte_per_element,
                void *recvbuf )
{
  int rc;

#if MPI_BASED
  rc = MPI_Scatter(sendbuf,
                   numbyte_per_element,
                   MPI_BYTE,
                   recvbuf,
                   numbyte_per_element,
                   MPI_BYTE,
                   LMON_BE_MASTER,
                   MPI_COMM_WORLD);
 
  if (rc < 0 )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true," MPI_Scatter failed");

      return LMON_EINVAL;
    }
#elif COBO_BASED
  rc = cobo_scatter (sendbuf,
                     numbyte_per_element,
                     recvbuf,
                     LMON_DAEMON_MASTER);

  if (rc != COBO_SUCCESS)
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true," cobo_scatter failed");

      return LMON_EINVAL;
    }
#else
  LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,"no internal comm fabric to leverage");
  return LMON_EINVAL;
#endif

  return LMON_OK;
}


//! LMON_daemon_internal_finalize();
/*
  Finalizes the LMON BACKEND API. Every daemon must call this to 
  propertly finalize LMON BACKEND
*/
lmon_rc_e
LMON_daemon_internal_finalize (int is_be)
{
  int rc;

#if MPI_BASED
  if ( (rc = MPI_Finalize()) < 0 )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true," MPI_Finalize failed");

      return LMON_EINVAL;
    }
#elif COBO_BASED
  if ( ( rc = cobo_close () ) != COBO_SUCCESS ) 
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
        "cobo_close failed");

      return LMON_EINVAL;
    }
#else
  LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,"no internal comm fabric to leverage");
  return LMON_EINVAL;
#endif

  return LMON_OK;
}


lmon_rc_e
LMON_daemon_enable_verbose(const char *vdir, const char *local_hostname, int is_be)
{
  char debugfn[PATH_MAX];
  snprintf (debugfn, PATH_MAX, vdir);

  if (!local_hostname)
    {
      LMON_say_msg ( LMON_DAEMON_MSG_PREFIX, true,
        "local_hostname arg is null" );

      return LMON_EBDARG;
    }

  if ( mkdir (debugfn, S_IRWXU) < 0 )
    {
      if ( errno != EEXIST )
        {
          LMON_say_msg ( LMON_DAEMON_MSG_PREFIX, true,
            "LMON daemon fails to mkdir %s", debugfn );

          return LMON_EINVAL;
        }
    }

  if (is_be)
    {
      snprintf(debugfn, PATH_MAX, "%s/be.stdout.%d.%s",
               vdir, getpid(), local_hostname);
    }
  else
    {
      snprintf(debugfn, PATH_MAX, "%s/mw.stdout.%d.%s",
               vdir, getpid(), local_hostname);
    }

  if ( freopen (debugfn, "w", stdout) == NULL )
    {
      LMON_say_msg ( LMON_DAEMON_MSG_PREFIX, true,
        "LMON daemon fails to reopen stdout: %s", debugfn );

      return LMON_EINVAL;
    }

  if (is_be)
    {
      snprintf(debugfn, PATH_MAX, "%s/be.stderr.%d.%s",
               vdir, getpid(), local_hostname);
    }
  else
    {
      snprintf(debugfn, PATH_MAX, "%s/mw.stderr.%d.%s",
               vdir, getpid(), local_hostname);
    }

  if ( freopen (debugfn, "w", stderr) == NULL )
    {
      LMON_say_msg ( LMON_DAEMON_MSG_PREFIX, true,
        "LMON daemon fails to reopen stderr: %s", debugfn );

      return LMON_EINVAL;
    }

  return LMON_OK;
}


//! lmon_rc_e LMON_daemon_return_ver
/*!

*/
int
LMON_daemon_return_ver()
{
  return LMON_VERSION;
}


//! lmon_rc_e LMON_daemon_getSharedKeyAndID
/*!
    returns info on authentication; 
    shared_key must be 128 bits
*/
lmon_rc_e
LMON_daemon_getSharedKeyAndID ( char *shared_key, int *id )
{
  char *sk;
  char *char_id;

  if ( shared_key == NULL )
    {
      LMON_say_msg (LMON_DAEMON_MSG_PREFIX, true,
        "one of the arguments is NULL" );

      return LMON_EBDARG;
    }

  if ( (sk = getenv (LMON_SHRD_SEC_ENVNAME)) == NULL )
    {
      LMON_say_msg (LMON_DAEMON_MSG_PREFIX, true, 
        "LMON_SHRD_SEC_ENVNAME envVar not found");

      return LMON_EINVAL;
    }

  sprintf (shared_key, "%s", sk);
  if ( (char_id = getenv (LMON_SEC_CHK_ENVNAME)) == NULL )
    {
      LMON_say_msg (LMON_DAEMON_MSG_PREFIX, true,
        "LMON_SEC_CHK_ENVNAME envVar not found");

      return LMON_EINVAL;
    } 

  (*id) = atoi (char_id);

  return LMON_OK;
}


//! lmon_rc_e LMON_daemon_getWhereToConnect
/*!
    returns info on the LAUNCHMON front-end's
    listening server socket.
*/
lmon_rc_e
LMON_daemon_getWhereToConnect ( struct sockaddr_in *servaddr )
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


//! bool getPersonalityField
/*!
    returns info on the LAUNCHMON front-end's
    listening server socket.
*/
bool
getPersonalityField ( std::string& pFilePath,
                      std::string &fieldName,
                      std::string &value )
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
      while ( !ifs.getline (line, PATH_MAX).eof() )
        {
          string strLine (line);
          string::size_type lastPos = strLine.find_first_not_of ( delim, 0 );
          string::size_type pos = strLine.find_first_of ( delim, lastPos );

          if ( pos != string::npos || lastPos != string::npos )
            {
              if ( fieldName == strLine.substr(lastPos, pos - lastPos) ) 
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
       "ios_base::failure exception thrown "
       "while parsing the personality file");
     return false;
   }
}


//! bool resolvHNAlias
/*!
    Resolves an alias to an IP using the hosts file 
    whose path is given by the first argument.
    This function can throw ios_base::failure
*/
bool
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
       "ios_base::failure exception thrown "
       "while parsing lines in the hosts file.");

     return false;
   }
}


//! lmon_rc_e LMON_daemon_gethostname
/*!

*/
lmon_rc_e
LMON_daemon_gethostname(bool bgion, char *my_hostname, int hlen, char *my_ip, int ilen, std::vector<std::string>& aliases)
{
  memset ( my_hostname, 0, hlen );
  memset ( my_ip, 0, ilen );

  if ( gethostname ( my_hostname, hlen ) < 0 )
    {
      LMON_say_msg(LMON_DAEMON_MSG_PREFIX, true,
        "gethostname failed");

      return LMON_ESYS;
    }

  struct hostent *hent = gethostbyname(my_hostname);
  if (hent == NULL)
    {
      if (bgion)
        {
          std::string resIP;
          std::string pFile("/proc/personality.sh");
          std::string fieldStr("BG_IP");

          LMON_say_msg ( LMON_DAEMON_MSG_PREFIX, false,
            "BES: this machine does not know how to resolve %s into an IP",
            my_hostname);
          LMON_say_msg ( LMON_DAEMON_MSG_PREFIX, false,
            "BES: parsing /proc/personality.sh to try to resolve");

          if (!getPersonalityField (pFile, fieldStr, resIP))
            {
              LMON_say_msg ( LMON_DAEMON_MSG_PREFIX, true,
                "BES: getPersonalityField also failed to resolve %s through %s",
                my_hostname, fieldStr.c_str());

              return LMON_ESYS;
            }
          snprintf(my_ip, ilen, "%s", resIP.c_str());
          snprintf(my_hostname, hlen, "%s", resIP.c_str());
        }
      else 
        {
          LMON_say_msg ( LMON_DAEMON_MSG_PREFIX, true,
                         "BES: gethostbyname returned NULL.");
          return LMON_ESYS;
        }
    }
  else // hent returns valid data 
    {
      int i=0;
      std::string my_hostnameStr(my_hostname);
      std::string notAvail("na");
      std::string my_ipStr(notAvail);

      aliases.push_back(my_hostnameStr);
      if (inet_ntop(hent->h_addrtype, hent->h_addr, my_ip, ilen) != NULL) 
        {
          my_ipStr = my_ip; 
          aliases.push_back(my_ipStr);
        }

      std::string h_nameStr(notAvail); 
      if ((h_nameStr = hent->h_name) != my_hostnameStr)
        {
          aliases.push_back(h_nameStr);
        }

      for (i=0; hent->h_aliases[i] != NULL; i++)
        {
          std::string al(hent->h_aliases[i]);
          if ((al != my_hostnameStr) && (al != h_nameStr))
            {
              aliases.push_back(al);
             }
         }
       for (i=0; hent->h_addr_list[i] != NULL; i++)
         {
           char tmp_ip[LMON_DAEMON_HN_MAX];
           if (inet_ntop(hent->h_addrtype, hent->h_addr_list[i], 
                          tmp_ip, LMON_DAEMON_HN_MAX) != NULL)
             {
               std::string al_ip(tmp_ip);
               if (al_ip != my_ipStr)
                 {
                   aliases.push_back(al_ip);
                 }
             }
         }
     }

  return LMON_OK;
}

bool is_bluegene_ion()
{
  bool rc = false;
  if ((access ("/proc/personality.sh", R_OK) != -1)
      || (access ("/jobs/tools/protocol", R_OK) != -1))
    {
      rc = true;
    }

  return rc;
}
