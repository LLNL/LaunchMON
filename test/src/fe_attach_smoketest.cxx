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
 *        Jun  02 2009 DHA: Added LMON_STATUS_CB_TEST support.
 *        May  19 2009 DHA: Added LMON_ERROR_CB_TEST support.
 *        Mar  13 2009 DHA: Dynamic check for proctabsize 
 *        Jun  12 2008 DHA: Added GNU build system support.  
 *        Mar  20 2008 DHA: Added BlueGene support.
 *        Mar  06 2008 DHA: Added calls-after-fail case.
 *        Mar  05 2008 DHA: Added Invalid pid test case to exercise 
 *                          connection timeout capability.
 *        Feb  09 2008 DHA: Added LLNS Copyright.
 *        Aug  06 2006 DHA: Adjust this case for minor API changes  
 *        Dec  27 2006 DHA: Created file.
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

#if HAVE_STDARG_H
# include <cstdarg>
#else
# error stdarg.h is required
#endif

#if HAVE_LIMITS_H
# include <limits.h>
#else
# error limits.h is required 
#endif

#include <lmon_api/lmon_proctab.h>
#include <lmon_api/lmon_fe.h>

#define ERROR_LOG_MAXSIZE 4096
#define STRING_MAXSIZE 128

#if MEASURE_TRACING_COST 
extern "C" {
  int begin_timer ();
  int time_stamp ( const char* description );
}
#endif

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
    fprintf(stdout, "* BE daemons have not spawned\n");

  if (WIFMWSPAWNED(stcp))
    fprintf(stdout, "* MW daemons spawned\n");
  else
    fprintf(stdout, "* MW daemons have not spawned\n");

  if (WIFDETACHED(stcp))
    fprintf(stdout, "* the job is detached\n");
  else
    fprintf(stdout, "* the job has not been detached\n");

  if (WIFKILLED(stcp))
    fprintf(stdout, "* the job is killed\n");
  else
    fprintf(stdout, "* the job has not been killed\n");

  return 0;
}

int errFunc (const char *format, va_list ap)
{
  int rc;
  char buf[ERROR_LOG_MAXSIZE];

  fprintf(stdout, "errFunc is called\n");
  rc = vsnprintf(buf, ERROR_LOG_MAXSIZE, format, ap); 
  fprintf(stdout, "errFunc: %s\n", buf);

  return rc;
}

int 
main (int argc, char* argv[])
{
  unsigned int i             = 0;
  int spid                   = 0;
  int jobidsize              = 0;
  unsigned int psize         = 0;
  unsigned int proctabsize   = 0;
  int aSession               = 0;
  char jobid[PATH_MAX]       = {0};
  char **daemon_opts         = NULL;
  MPIR_PROCDESC_EXT *proctab = NULL;
  lmon_rc_e rc;

  if ( argc < 3 )
    {
      fprintf( stdout, 
	"Usage: fe_launch_smoketest launcherpid daemonpath [daemonargs]\n");
      fprintf( stdout, "[LMON FE] FAILED\n");
      return EXIT_FAILURE;
    }

  if ( access(argv[2], X_OK) < 0 )
    {
      fprintf ( stdout, 
        "%s cannot be executed\n", argv[2] );

      fprintf ( stdout, "[LMON FE] FAILED\n" );
      return EXIT_FAILURE;
    }

  if ( argc > 3 )
    {
      daemon_opts = argv+3;
    }

  if ( ( rc = LMON_fe_init (LMON_VERSION)) 
              != LMON_OK )
    {
      fprintf ( stdout, "[LMON FE] FAILED\n");
      return EXIT_FAILURE;
    }
  
  if ( getenv ("LMON_ERROR_CB_TEST"))
    {
       if ( LMON_fe_regErrorCB(errFunc) != LMON_OK )
         {
            fprintf ( stdout, "[LMON FE] FAILED\n");
            return EXIT_FAILURE;
         } 
    } 


  if ( ( rc = LMON_fe_createSession (&aSession)) 
              != LMON_OK)
    {
      fprintf ( stdout, "[LMON FE] FAILED\n");
      return EXIT_FAILURE;
    }

  if (getenv ("LMON_STATUS_CB_TEST"))
    {
       if ( LMON_fe_regStatusCB(aSession, statusFunc) != LMON_OK )
         {
            fprintf ( stdout, "[LMON FE] FAILED\n");
            return EXIT_FAILURE;
         } 
    } 

  spid = atoi(argv[1]);
  if ( spid < 0 )
    {
      fprintf( stdout, "[LMON FE] FAILED\n");
      return EXIT_FAILURE;
    }

#if MEASURE_TRACING_COST
  begin_timer ();
#endif 

  if (getenv ("FEN_RM_DISTRIBUTED"))
    {
      /*
       * If the third argument is not null, the launchMON engine
       * gets invoked via ssh or rsh.
       */
      char hn[1024];
      gethostname ( hn, 1024);
      if ( ( rc = LMON_fe_attachAndSpawnDaemons(
                aSession,
                hn,
                spid,
                argv[2],
                daemon_opts,
                NULL,
                NULL))
                  != LMON_OK )
        {
          if ( getenv ("LMON_INVALIDPID_TEST") != NULL )
            {
              if ( rc == LMON_ETOUT )
                {
                  fprintf ( stdout,
                  "[LMON FE] PASS: returned the LMON_ETOUT error code\n");
                                                                                                                                                
                  if ( getenv ("LMON_CALLS_AFTER_FAIL_TEST") != NULL )
                    {
                      if ( (rc = LMON_fe_detach (aSession)) != LMON_OK )
                        {
                          fprintf (stdout,
                            "[LMON FE] PASS: LMON_fe_detach return with an error code [%d] \n", rc);
                                                                                                                                                
                          return EXIT_SUCCESS;
                        }
                      else
                        {
                          return EXIT_FAILURE;
                        }
                    } // if LMON_CALLS_AFTER_FAIL_TEST
                                                                                                                                                
                   return EXIT_SUCCESS;
                } // LMON_ETOUT returned
            } // if LMON_INVALIDPID_TEST 

            fprintf ( stdout,
              "[LMON FE] FAILED\n" );
            return EXIT_FAILURE;
        } // attachAndSpawn for remote
    } 
  else
    {
      if ( ( rc = LMON_fe_attachAndSpawnDaemons( 
                aSession, 
	        NULL,
		spid,
		argv[2],
		daemon_opts,
		NULL,
	        NULL ))
	      != LMON_OK )
        {
          if ( getenv ("LMON_INVALIDPID_TEST") != NULL )
            {
              if ( rc == LMON_ETOUT )
                {
                  fprintf ( stdout,
                  "[LMON FE] PASS: returned the LMON_ETOUT error code\n");

                  if ( getenv ("LMON_CALLS_AFTER_FAIL_TEST") != NULL )
                    {
                      if ( (rc = LMON_fe_detach (aSession)) != LMON_OK )
                        {
                          fprintf (stdout,
                            "[LMON FE] PASS: LMON_fe_detach return with an error code [%d] \n", rc);
                                                                                                                                                
                          return EXIT_SUCCESS;
                        }
                      else
                        {
                          return EXIT_FAILURE;
                        }
                    } // if LMON_CALLS_AFTER_FAIL_TEST
                  
                   return EXIT_SUCCESS;
                } // LMON_ETOUT returned
            } // if LMON_INVALIDPID_TEST                                                                                                            
            fprintf ( stdout, "[LMON FE] FAILED\n" );
            return EXIT_FAILURE;
        } // attachAndSpawn for local
    } 
#if MEASURE_TRACING_COST
  time_stamp ( "LMON_fe_attachAndSpawnDaemons perf" );
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

  if (proctabsize <= 0) 
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

  fprintf ( stdout,
    "[LMON FE] Please check the correctness of the following proctable\n");

#if MEASURE_TRACING_COST
  begin_timer ();
#endif 

  if ( ( rc = LMON_fe_getProctable (
                aSession, 
                proctab,
                &psize,
                proctabsize )) 
              !=  LMON_OK)
    {
       fprintf ( stdout, "[LMON FE] FAILED\n" );
       return EXIT_FAILURE;
    }

#if MEASURE_TRACING_COST
  time_stamp ( "LMON_fe_getProctable perf" );
#endif

  for (i=0; i < psize; i++)
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

  lmon_rm_info_t rminfo;
  rc = LMON_fe_getRMInfo ( aSession, &rminfo);
  if (rc != LMON_OK)
    {
      fprintf ( stdout, "[LMON FE] FAILED\n");
      return EXIT_FAILURE;
    }
  else
   {
      fprintf ( stdout,
         "\n[LMON FE] RM type is %d\n", rminfo.rm_type);
      fprintf ( stdout,
         "\n[LMON FE] RM launcher's pid is %d\n", rminfo.rm_launcher_pid);
   }
  
  if ( (getenv ("LMON_FE_SHUTDOWNBE_TEST")) != NULL )
    {
      if ( ( rc = LMON_fe_shutdownDaemons ( aSession ) ) != LMON_OK )
        {
           fprintf ( stdout, "[LMON FE]LMON_fe_shutdownBe FAILED\n");
           return EXIT_FAILURE;
        }
      else
        {
           system ("ps x");
           fprintf ( stdout, "[LMON FE] CHECK for LMON_fe_kill\n");
           fprintf ( stdout, "[LMON FE] PASS Criteria: you must not see the launcher process(es) that control(s) kicker daemons\n");
           return EXIT_SUCCESS;
        }
    }

  if ( (getenv ("LMON_FE_KILL_TEST")) != NULL )
    {
      if ( ( rc = LMON_fe_kill ( aSession ) ) != LMON_OK )
        {
           fprintf ( stdout, "[LMON FE] LMON_fe_kill FAILED\n");
           return EXIT_FAILURE;
        }
      else
        {
           system ("ps x");
           fprintf ( stdout, "[LMON FE] CHECK for LMON_fe_kill\n");
           fprintf ( stdout, "[LMON FE] PASS Criteria: you must not see the launcher process(es) nor the launchmon process.\n");
           return EXIT_SUCCESS;
        }
    }

  rc = LMON_fe_recvUsrDataBe ( aSession, NULL );
                                                                                                                                               
  if ( (rc == LMON_EBDARG )
       || ( rc == LMON_ENOMEM )
       || ( rc == LMON_EINVAL ) )
    {
      fprintf ( stdout, "[LMON FE] FAILED\n");
      return EXIT_FAILURE;
    }
                                                                                                                                               
  sleep (3); /* wait until all BE outputs are printed */
                                                                                                                                               
  fprintf ( stdout,
    "\n[LMON FE] PASS: run through the end\n");
                                                                                                                                               
  return EXIT_SUCCESS;
}
