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
 *
 *  ./fe_launch_smoketest.debug /bin/hostname 9 5 pdebug `pwd`/be_kicker.debug
 *
 *  Update Log:
 *        Oct 25 2011 DHA: Added BGQ support.
 *        Oct 21 2011 DHA: Added dynamic RM support.
 *        Nov 12 2009 DHA: Change BG mpirun options to cover /P running under IBM LL
 *        Mar 04 2009 DHA: Added generic BlueGene support.
 *        Jun 16 2008 DHA: Added LMON_fe_recvUsrDataBe at the end to 
 *                         coordinate the testing result with back-end 
 *                         daemons better. 
 *        Jun 12 2008 DHA: Added GNU build system support.
 *        Mar 18 2008 DHA: Added BlueGene support.
 *        Mar 05 2008 DHA: Added invalid daemon path test.
 *        Feb 09 2008 DHA: Added LLNS Copyright.
 *        Jul 30 2007 DHA: Adjust this case for minor API changes  
 *        Dec 27 2006 DHA: Created file.
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

#include <lmon_api/lmon_proctab.h>
#include <lmon_api/lmon_fe.h>

#if MEASURE_TRACING_COST 
extern "C" {
  int begin_timer ();
  int time_stamp ( const char* description );
}
#endif

/*
 * OUR PARALLEL JOB LAUNCHER
 */
char mylauncher[PATH_MAX] = {0};

int statusFunc ( int *status )
{
  int stcp = *status;
  fprintf (stdout, "**** status callback routine is invoked:0x%x ****\n", stcp);
  if (WIFREGISTERED(stcp))
    fprintf(stdout, "* session registered\n");
  else
    fprintf(stdout, "* session not registered\n");

  if (WIFBESPAWNED(stcp))
    fprintf(stdout, "* BE daemons spawned\n");
  else
    fprintf(stdout, "* BE daemons not spawned or exited\n");

  if (WIFMWSPAWNED(stcp))
    fprintf(stdout, "* MW daemons spawned\n");
  else
    fprintf(stdout, "* MW daemons not spawned or exited\n");

  if (WIFDETACHED(stcp))
    fprintf(stdout, "* the job is detached\n");
  else
    {
      if (WIFKILLED(stcp))
        fprintf(stdout, "* the job is killed\n");
      else
        fprintf(stdout, "* the job has not been killed\n");
    }

  return 0;
}


int 
main (int argc, char *argv[])
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

  if ( getenv ("LMON_INVALIDDAEMON_TEST") == NULL )
    {
      if ( access(argv[5], X_OK) < 0 )
        {
          fprintf(stdout,
            "%s cannot be executed\n",
            argv[5]);
          fprintf(stdout,
            "[LMON FE] FAILED\n");
          return EXIT_FAILURE;
        }
    }

  if ( argc > 6 )
    daemon_opts = argv+6;

  char *rmenv = getenv("MPI_JOB_LAUNCHER_PATH");
  if (!rmenv)
    {
      fprintf(stdout,
        "MPI_JOB_LAUNCHER_PATH envVar must be given\n" );
      return EXIT_FAILURE;
    }

  snprintf(mylauncher, PATH_MAX, "%s", rmenv);

  rmenv = getenv("RM_TYPE");
  if (!rmenv)
    {
      fprintf(stdout,
        "RM_TYPE envVar must be given\n" );
      return EXIT_FAILURE;
    }

  std::string rmenv_str = rmenv;

  if ((rmenv_str == std::string("RC_bgqrm")))
    {
      launcher_argv = (char **) malloc(14*sizeof(char *));
      launcher_argv[0] = strdup(mylauncher);
      launcher_argv[1] = strdup("--verbose");
      launcher_argv[2] = strdup("5");
      launcher_argv[3] = strdup("--np");
      launcher_argv[4] = strdup(argv[2]);
      launcher_argv[5] = strdup("--exe"); 
      launcher_argv[6] = strdup(argv[1]); 
      // manually fill the block
      launcher_argv[7] = strdup("--block");
      launcher_argv[8] = strdup("R00-M0-N04"); 
      // manually fill the corner
      launcher_argv[9] = strdup("--corner");
      launcher_argv[10] = strdup("R00-M0-N04-J07");
      // manually fill the shape 
      launcher_argv[11] = strdup("--shape");
      launcher_argv[12] = strdup("1x1x1x1x1");
      launcher_argv[13] = NULL;
      fprintf (stdout, 
        "[LMON_FE] launching the job/daemons via %s\n",
        mylauncher);
    }
  else if ((rmenv_str == std::string("RC_bgq_slurm")))
    {
      launcher_argv = (char **) malloc(7*sizeof(char *));
      launcher_argv[0] = strdup(mylauncher);
      launcher_argv[1] = strdup("-N");
      launcher_argv[2] = strdup(argv[3]);
      launcher_argv[3] = strdup("-n");
      launcher_argv[4] = strdup(argv[2]);
      launcher_argv[5] = strdup(argv[1]);
      launcher_argv[6] = NULL;
      fprintf (stdout,
        "[LMON_FE] launching the job/daemons via %s\n",
        "mylauncher");
    }
  else if ((rmenv_str == std::string("RC_bglrm")) 
      || (rmenv_str == std::string("RC_bgprm")))
    {
      launcher_argv = (char **) malloc(8*sizeof(char *));
      launcher_argv[0] = strdup(mylauncher);
      launcher_argv[1] = strdup("-verbose");
      launcher_argv[2] = strdup("1");
      launcher_argv[3] = strdup("-np");
      launcher_argv[4] = strdup(argv[2]);
      launcher_argv[5] = strdup("-exe"); 
      launcher_argv[6] = strdup(argv[1]); 
      launcher_argv[7] = NULL;
      fprintf (stdout, 
                "[LMON_FE] launching the job/daemons via %s\n",
                mylauncher);
    }
  else if (rmenv_str == std::string("RC_slurm"))
    {
      numprocs_opt     = string("-n") + string(argv[2]);
      numnodes_opt     = string("-N") + string(argv[3]);
      partition_opt    = string("-p") + string(argv[4]);
      launcher_argv    = (char **) malloc (7*sizeof(char*));
      launcher_argv[0] = strdup(mylauncher);
      launcher_argv[1] = strdup(numprocs_opt.c_str());
      launcher_argv[2] = strdup(numnodes_opt.c_str());
      launcher_argv[3] = strdup(partition_opt.c_str());
      launcher_argv[4] = strdup("-l");
      launcher_argv[5] = strdup(argv[1]);
      launcher_argv[6] = NULL;
    }
  else if (rmenv_str == std::string("RC_alps"))
    {
      numprocs_opt     = string("-n") + string(argv[2]);
      launcher_argv    = (char**) malloc(4*sizeof(char*));
      launcher_argv[0] = strdup(mylauncher);
      launcher_argv[1] = strdup(numprocs_opt.c_str());
      launcher_argv[2] = strdup(argv[1]);
      launcher_argv[3] = NULL;
    }
  else if (rmenv_str == std::string("RC_orte"))
    {
      launcher_argv    = (char **) malloc(8*sizeof(char*));
      launcher_argv[0] = strdup(mylauncher);
      launcher_argv[1] = strdup("-mca");
      launcher_argv[2] = strdup("debugger");
      launcher_argv[3] = strdup("mpirx");
      launcher_argv[4] = strdup("-np");
      launcher_argv[5] = strdup(argv[2]);
      launcher_argv[6] = strdup(argv[1]);
      launcher_argv[7] = NULL;
      fprintf (stdout,
               "[LMON_FE] launching the job/daemons via %s\n",
               mylauncher);
    }

  if ( ( rc = LMON_fe_init ( LMON_VERSION ) ) 
              != LMON_OK )
    {
      fprintf ( stdout, "[LMON FE] FAILED\n" );
      return EXIT_FAILURE;
    }

  if ( ( rc = LMON_fe_createSession (&aSession)) 
              != LMON_OK)
    {
      fprintf ( stdout, "[LMON FE] FAILED\n");
      return EXIT_FAILURE;
    }

  lmon_rm_info_t rminfo;
  rc = LMON_fe_getRMInfo ( aSession, &rminfo);
  if (rc != LMON_EDUNAV)
    {
      fprintf ( stdout, "[LMON FE] FAILED, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
  else
   {
      fprintf ( stdout,
         "\n[LMON FE] %d RM types are supported\n",
         rminfo.num_supported_types);
   }

  if ( getenv ("LMON_STATUS_CB_TEST"))
    {
       if ( LMON_fe_regStatusCB(aSession, statusFunc) != LMON_OK )
         {
            fprintf ( stdout, "[LMON FE] FAILED\n");
            return EXIT_FAILURE;
         }
    }

#if MEASURE_TRACING_COST
  begin_timer ();
#endif 

  if ( getenv ("FEN_RM_DISTRIBUTED") )
    {
      /*
       * If the third argument is not null, the launchMON engine
       * gets invoked via ssh or rsh. 
       */
      char hn[1024];
      gethostname ( hn, 1024);
      if ( ( rc = LMON_fe_launchAndSpawnDaemons( 
                    aSession, 
   	            hn,
	    	    launcher_argv[0],
		    launcher_argv,
		    argv[5],
		    daemon_opts,
		    NULL,
		    NULL)) 
                  != LMON_OK )
        {
          if ( getenv ("LMON_INVALIDDAEMON_TEST") != NULL )
            {
              if ( rc == LMON_ETOUT )
                {
                  fprintf ( stdout,
                  "[LMON FE] PASS: returned the LMON_ETOUT error code\n");
                  return EXIT_SUCCESS;
                }
            }

          fprintf ( stdout, "[LMON FE] FAILED\n" );
          return EXIT_FAILURE;
      }
    }
  else
    {
      if ( ( rc = LMON_fe_launchAndSpawnDaemons( 
                    aSession, 
   	            NULL,
		    launcher_argv[0],
		    launcher_argv,
		    argv[5],
		    daemon_opts,
		    NULL,
		    NULL)) 
                  != LMON_OK )
        {
          if ( getenv ("LMON_INVALIDDAEMON_TEST") != NULL )
            {
              if ( rc == LMON_ETOUT )
                {
                  fprintf ( stdout,
                    "[LMON FE] PASS: returned the LMON_ETOUT error code\n");
                  return EXIT_SUCCESS;
                }
            }

          fprintf ( stdout, "[LMON FE] FAILED\n" );
          return EXIT_FAILURE;
        }
    }

#if MEASURE_TRACING_COST
  time_stamp ( "LMON_fe_launchAndSpawnDaemons perf" );
#endif
 
  if ( ( rc = LMON_fe_getProctableSize ( 
                aSession, 
                &proctabsize ))
              !=  LMON_OK )
    {
       fprintf ( stdout, 
         "[LMON FE] FAILED in LMON_fe_getProctableSize\n");
       return EXIT_FAILURE;
    }

  if (proctabsize != atoi(argv[2])) 
    {
      fprintf ( stdout, 
        "[LMON FE] FAILED, proctabsize is not equal to the given: %ud\n", 
        proctabsize);
      return EXIT_FAILURE;
    }

  proctab = (MPIR_PROCDESC_EXT*) malloc (
                proctabsize*sizeof (MPIR_PROCDESC_EXT) );
  
  if ( !proctab )
    {
       fprintf ( stdout, "[LMON FE] malloc returned null\n");
       return EXIT_FAILURE;
    }

#if MEASURE_TRACING_COST
  begin_timer ();
#endif 
  if ( ( rc = LMON_fe_getProctable ( 
                aSession, 
                proctab,
                &psize,
                proctabsize )) 
              !=  LMON_OK )
    {
       fprintf ( stdout, "[LMON FE] FAILED\n");
       return EXIT_FAILURE;
    }

#if MEASURE_TRACING_COST
  time_stamp ( "LMON_fe_getProctable perf" );
#endif

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
            "[LMON FE] resource handle[jobid or job launcher's pid]: %s\n", jobid);
          fprintf ( stdout, 
            "[LMON FE]");
       }
    }

  rc = LMON_fe_getRMInfo ( aSession, &rminfo);
  if (rc != LMON_OK)
    {
      fprintf ( stdout, "[LMON FE] FAILED\n");
      return EXIT_FAILURE;
    }
  else
   {
      fprintf ( stdout, 
         "\n[LMON FE] RM type is %d\n",
           rminfo.rm_supported_types[rminfo.index_to_cur_instance]);
      fprintf ( stdout, 
         "\n[LMON FE] RM launcher's pid is %d\n", rminfo.rm_launcher_pid);
   }

  rc = LMON_fe_recvUsrDataBe ( aSession, NULL );

  if ( (rc == LMON_EBDARG ) 
       || ( rc == LMON_ENOMEM )
       || ( rc == LMON_EINVAL ) )
    {
      fprintf ( stdout, "[LMON FE] FAILED\n");
      return EXIT_FAILURE;
    } 

  //sleep (3); /* wait until all BE outputs are printed */

  if (getenv ("LMON_ADDITIONAL_FE_STALL"))
    {
      sleep (120);
    }

  fprintf ( stdout, 
    "\n[LMON FE] PASS: run through the end\n");

  return EXIT_SUCCESS;
}

