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
 *        Mar 04 2008 DHA: Added generic BlueGene support
 *        Jun 17 2008 DHA: Added BlueGene support 
 *        Jun 12 2008 DHA: Added GNU build system support
 *        Feb 09 2008 DHA: Added LLNS Copyright. 
 *        Aug 06 2007 DHA: Created file   
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/common.h>
                                                                                                                                          
#if HAVE_UNISTD_H
# include <unistd.h>
#else
# error unistd.h is required
#endif

#if HAVE_LIMITS_H
# include <limits.h>
#else
# error limits.h is required 
#endif
                                                                                                                                          
#include <string>
#include <map>
                                                                                                                                          
#include <lmon_api/lmon_proctab.h>
#include <lmon_api/lmon_fe.h>

const int MAXPROCOUNT  = 12000;

/*
 * OUR PARALLEL JOB LAUNCHER 
 */
const char* mylauncher    = TARGET_JOB_LAUNCHER_PATH;
std::map<std::string, int> usrdata; // this will have "hostname," "port" pair  
std::map<std::string, int> recvdata; // this will have "hostname," "port" pair echoed back from BE


static int 
fill_usrdata ( )
{
  using namespace std;

  int i;
  
  //
  // 128 is an arbitrary number; i.e. emulating 
  // 128 ip/port pair vector
  //
  for ( i=0; i < 128; ++i )
    {
      char hnconcat[128];

      if ( i < 0 )
        break;

      if ( i < 10 )
        {
          sprintf ( hnconcat, 
            "myhostname00%d.llnl.gov", i); // faking a hostname 
        }
      else if ( i < 100 )
        {
          sprintf ( hnconcat, 
            "myhostname0%d.llnl.gov", i); // faking a hostname
        }
      else if ( i < 1000 )
        {
          sprintf ( hnconcat, 
            "myhostname%d.llnl.gov", i);  // faking a hostname
        }

      usrdata[string(hnconcat)] = i+1024; // faking a port 
    }

  return 0;
}


static int 
packfebe_cb ( void *udata, 
              void *msgbuf, 
              int msgbufmax, 
              int *msgbuflen )
{
  using namespace std;
  
  char *trav  = (char *) msgbuf;
  int usedbuf = 0;
  int trun    = 0;

  map<string, int>* u = ( map<string, int>* ) udata; 
  map<string, int>& myudata =  *u;
  map<string, int>::const_iterator iter; 

  if ( ( msgbuf == NULL ) || ( msgbufmax < 0) )
    return -1; 

  for ( iter = myudata.begin(); iter != myudata.end(); iter++)
    {
      int accum_len = usedbuf + iter->first.size() + + sizeof(int);
      if ( accum_len > msgbufmax )
        {
          trun = 1; 
          break;
        } 

      memcpy ( (void*) trav, iter->first.c_str(), iter->first.size()+1); 
      trav += iter->first.size()+1; 
      memcpy ( (void*) trav, (void*) &(iter->second), sizeof (int) );
      trav += sizeof (int); 
      usedbuf += iter->first.size()+1+sizeof (int);
    }

   (*msgbuflen) = usedbuf;

   return ((trun == 0) ? 0 : -1);
}


static int
unpackbefe_cb  ( void* udatabuf, 
		 int udatabuflen, 
		 void* udata )
{
  using namespace std;

  char *trav  = (char*) udatabuf;
  int usedbuf = 0;
  map<string, int> *u = ( map<string, int> *) udata; 
  map<string, int> &myudata = *u;

  while ( usedbuf < udatabuflen )
    {
      myudata[string(trav)] = (int) *(trav+strlen(trav)+1);
      usedbuf += strlen(trav)+1+sizeof(int);
      trav += strlen(trav)+1+sizeof(int);
    }
  
  return 0;    
}


int 
main (int argc, char* argv[])
{
  using namespace std;
  
  int aSession    = 0;
  unsigned int psize       = 0;
  unsigned int proctabsize = 0;
  int jobidsize   = 0;
  int i           = 0;
  char jobid[PATH_MAX]        = {0};
  char **launcher_argv        = NULL;
  char **daemon_opts          = NULL;
  MPIR_PROCDESC_EXT *proctab  = NULL;

  lmon_rc_e rc;
  string numprocs_opt;
  string numnodes_opt;
  string partition_opt;

  if ( argc < 6 )
    {
      fprintf ( stdout, 
        "Usage: fe_launch_smoketest appcode numprocs numnodes partition daemonpath [daemonargs]\n" );
      fprintf ( stdout, 
        "[LMON FE] FAILED\n" );
      return EXIT_FAILURE;	      
    }

  if ( access(argv[1], X_OK) < 0 )
    {
      fprintf ( stdout, 
        "%s cannot be executed\n", 
        argv[1] );
      fprintf ( stdout, 
        "[LMON FE] FAILED\n" );
      return EXIT_FAILURE;     
    }

  if ( access(argv[5], X_OK) < 0 )
    {
      fprintf(stdout, 
        "%s cannot be executed\n", 
        argv[2]);
      fprintf(stdout, 
        "[LMON FE] FAILED\n");
      return EXIT_FAILURE;	      
    }
  if ( argc > 6 )
    daemon_opts = argv+6;

  fill_usrdata (); // filling in user data 

#if RM_SLURM_SRUN
  numprocs_opt = string("-n") + string(argv[2]);
  numnodes_opt = string("-N") + string(argv[3]);
  partition_opt = string("-p") + string(argv[4]);
    
  launcher_argv = (char**) malloc(7*sizeof(char*));
  launcher_argv[0] = strdup(mylauncher);
  launcher_argv[1] = strdup(numprocs_opt.c_str());
  launcher_argv[2] = strdup(numnodes_opt.c_str());
  launcher_argv[3] = strdup(partition_opt.c_str());
  launcher_argv[4] = strdup("-l");
  launcher_argv[5] = strdup(argv[1]);
  launcher_argv[6] = NULL;
#elif RM_BG_MPIRUN 
  launcher_argv = (char**) malloc(8*sizeof(char*));
  launcher_argv[0] = strdup(mylauncher);
  launcher_argv[1] = strdup("-verbose");
  launcher_argv[2] = strdup("1");
  launcher_argv[3] = strdup("-np");
  launcher_argv[4] = strdup(argv[2]);
  launcher_argv[5] = strdup("-exe");
  launcher_argv[6] = strdup(argv[1]);
  launcher_argv[7] = NULL;
  fprintf (stdout, "[LMON FE] launching the job/daemons via %s\n", mylauncher);
#else
# error add support for the RM of your interest here
#endif  

  if ( ( rc = LMON_fe_init ( LMON_VERSION ) ) 
              != LMON_OK )
    {
      fprintf ( stdout, 
        "[LMON FE] LMON_fe_init FAILED\n" );
      return EXIT_FAILURE;
    }

  if ( ( rc = LMON_fe_createSession (&aSession)) 
              != LMON_OK)
    {
      fprintf ( stdout, 
        "[LMON FE] LMON_fe_createFEBESession FAILED\n");
      return EXIT_FAILURE;
    }

  if ( ( rc = LMON_fe_regPackForFeToBe (
		aSession,
                packfebe_cb ))  
              != LMON_OK )
     {
       fprintf ( stdout,
        "[LMON FE] LMON_fe_regPackForFeToBe FAILED\n");
       return EXIT_FAILURE;
     } 

  if ( ( rc = LMON_fe_regUnpackForBeToFe (
                aSession,
                unpackbefe_cb ))  
              != LMON_OK )
     {
       fprintf ( stdout,
        "[LMON FE] LMON_fe_regUnpackForBeToFe FAILED\n");
       return EXIT_FAILURE;
     }  

  if ( ( rc = LMON_fe_launchAndSpawnDaemons ( 
                aSession, 
                NULL,
		launcher_argv[0],
		launcher_argv,
		argv[5],
		daemon_opts,
		&usrdata,
		&recvdata)) 
              != LMON_OK )
    {
      fprintf ( stdout, 
        "[LMON FE] LMON_fe_launchAndSpawnDaemons FAILED\n" );
      return EXIT_FAILURE;
    }

  if ( ( rc = LMON_fe_getProctableSize (
                aSession,
                &proctabsize ))
              !=  LMON_OK )
    {
       fprintf ( stdout,
         "[LMON FE] FAILED in LMON_fe_getProctableSize\n");
       return EXIT_FAILURE;
    }
 
  proctab = (MPIR_PROCDESC_EXT*) malloc (
                 proctabsize*sizeof (MPIR_PROCDESC_EXT) );

  if ( !proctab )
    {
       fprintf ( stdout, "[LMON FE] malloc returned null\n");
       return EXIT_FAILURE;
    }

  if ( ( rc = LMON_fe_getProctable ( 
                aSession, 
                proctab,
                &psize,
                proctabsize )) 
              !=  LMON_OK )
    {
       fprintf ( stdout, 
         "[LMON FE] FAILED\n");
       return EXIT_FAILURE;
    }

  fprintf ( stdout, 
            "[LMON FE] Please check the correctness of the following proctable\n");

  for(i=0; i < psize; i++)
    {
      fprintf ( stdout, 
        "[LMON FE] host_name: %s\n", proctab[i].pd.host_name);
      fprintf ( stdout, 
        "[LMON FE] executable_name: %s\n", proctab[i].pd.executable_name);
      fprintf ( stdout, 
        "[LMON FE] pid: %d(rank %d)\n", proctab[i].pd.pid, proctab[i].mpirank);      
      fprintf ( stdout, "[LMON FE] \n");
    }

  rc = LMON_fe_getResourceHandle ( aSession, jobid,
                                 &jobidsize, PATH_MAX);
  if ((rc != LMON_OK) && (rc != LMON_EDUNAV))
    {
      if ( rc != LMON_EDUNAV )
        {
          fprintf ( stdout, "[LMON FE] FAILED\n");
           return EXIT_FAILURE;
        }
    }
  else
    {
      if (rc != LMON_EDUNAV)
        {
          fprintf ( stdout,
            "\n[LMON FE] Please check the correctness of the following resource handle\n");
          fprintf ( stdout,
            "[LMON FE] resource handle[slurm jobid]: %s\n", jobid);
          fprintf ( stdout,
            "[LMON FE]");
       }
    }

  if ( ( LMON_fe_sendUsrDataBe ( aSession, (void*) &usrdata ) ) 
       != LMON_OK )
    {
       fprintf ( stdout, "[LMON FE] FAILED\n");
    
       return EXIT_FAILURE;
    }

  if ( ( LMON_fe_recvUsrDataBe ( aSession, (void*) &recvdata ) ) 
       != LMON_OK )
    {
       fprintf ( stdout, "[LMON FE] FAILED\n");
    
       return EXIT_FAILURE;
    }

  rc = LMON_fe_recvUsrDataBe ( aSession, NULL );

  if ( (rc == LMON_EBDARG )
       || ( rc == LMON_ENOMEM )
       || ( rc == LMON_EINVAL ) )
    {
      fprintf ( stdout, "[LMON FE] FAILED\n");
      return EXIT_FAILURE;
    }


  sleep (3);

  fprintf ( stdout,
    "\n[LMON FE] PASS: run through the end\n");
  
  return EXIT_SUCCESS;
}
