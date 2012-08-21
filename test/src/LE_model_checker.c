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
 *        Mar 05 2008 DHA: Support for generic BlueGene systems
 *        Jun 12 2008 DHA: GNU build system support 
 *        Feb 09 2008 DHA: Added LLNS Copyright
 *        Dec 07 2007 DHA: Included more header files to shut up the 
 *                         compiler.
 *        Dec 05 2007 DHA: Added update log. 
 *                         Enhance the dummy mpirun to add 
 *                         a model checker capability for 
 *                         the LaunchMon Engine. 
 *                         Copied this from dummy_mpirun directory. 
 *                         Renamed the source file: LE_model_checker.c.
 *                         TODO: populate a wide range of error cases.  
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/common.h>

#if HAVE_PTHREAD_H
# include <pthread.h>
#else
# error pthread.h is required
#endif

#define MPIR_DEBUG_SPAWNED  1
#define MPIR_DEBUG_ABORTING 2
#define EXECPATH_SIZE       256
#define SERVER_ARG_SIZE     1024

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C 
#endif

typedef struct {
  char * host_name;           /* Something we can pass to inet_addr */
  char * executable_name;     /* The name of the image */
  int  pid;                   /* The pid of the process */
} MPIR_PROCDESC;

BEGIN_C_DECLS

/* Standard variables */
MPIR_PROCDESC * MPIR_proctable    = NULL;
volatile int MPIR_debug_state     = 0;
volatile int MPIR_debug_gate      = 0;
int MPIR_proctable_size           = 0;
volatile int  MPIR_being_debugged = 0;
  
/* BG/MPIRUN specific variables */
char MPIR_executable_path[EXECPATH_SIZE] = {0};
char MPIR_server_arguments[SERVER_ARG_SIZE] = {0};

/* SLURM/SRUN specific variable */
char * totalview_jobid            = NULL;

END_C_DECLS

enum errormode_e { 
  err0, 
  err1, 
  err2, 
  err3, 
  err4 }; 

enum whichthread_e { 
  mainthread, 
  slavethread };

typedef struct _option_t {
  int pcount;
  enum errormode_e emode; 
  enum whichthread_e thr;
} option_t;


/*
 * The variable to store model checker options
 */
option_t myopt;


void 
print_debug_state ()
{

  fprintf ( stdout, "*************************************************\n" );
  if ( !MPIR_debug_state )
    {
      fprintf ( stdout, 
		"[LaunchMON MODEL CHECKER] MPIR_debug_state     : %d, %s\n", 
		MPIR_debug_state, "State not set" ); 
    }
  else
    {
      fprintf ( stdout, 
		"[LaunchMON MODEL CHECKER] MPIR_debug_state     : %d, %s\n", 
		MPIR_debug_state, (MPIR_debug_state == MPIR_DEBUG_SPAWNED) 
			          ? "Spawned" : "Aborting"); 
    }

  fprintf ( stdout, 
	    "[LaunchMON MODEL CHECKER] MPIR_debug_gate      : %d\n", 
	    MPIR_debug_gate ); 
  fprintf ( stdout, 
	    "[LaunchMON MODEL CHECKER] MPIR_proctable_size  : %d\n", 
	    MPIR_proctable_size ); 
  fprintf ( stdout, 
	    "[LaunchMON MODEL CHECKER] MPIR_being_debugged  : %d\n", 
	    MPIR_being_debugged ); 
  fprintf ( stdout, 
	    "[LaunchMON MODEL CHECKER] MPIR_executable_path : %s\n", 
	    MPIR_executable_path ); 
  fprintf ( stdout, 
	    "[LaunchMON MODEL CHECKER] MPIR_server_arguments: "); 
  char* trav = MPIR_server_arguments;
  while ((*trav != '\0') && (*(trav+1) != '\0'))
    {
      fprintf ( stdout, "%s ", trav); 
      trav += strlen(trav)+1;	
    }
  fprintf ( stdout, "\n");
}


EXTERN_C int MPIR_Breakpoint ()
{

  /*  This is the dummy function that gets called
   *  when A) the job's state is about to be attached or
   *  B) is about to be terminated.     
   */

  fprintf ( stdout, "[LaunchMON MODEL CHECKER] MPIR_Breakpoint invoked \n" );
  print_debug_state ();

  return 0;
}


void *
setup_debugger ( void* arg )
{
  int i;
  char *tmphn;
  MPIR_proctable_size = myopt.pcount; 

  if ( MPIR_being_debugged ) 
    {
      MPIR_debug_state = MPIR_DEBUG_SPAWNED;
      MPIR_proctable = (MPIR_PROCDESC*) 
	malloc (MPIR_proctable_size * sizeof(MPIR_PROCDESC));     
      tmphn = (char*) malloc (21);

      for (i = 0;  i < MPIR_proctable_size; i++ )
	{
	  if ( i/10 == 0 )	    
	    sprintf ( tmphn, "virtualmachine00000%d", i);  	      	    
	  else if ( i/100 == 0 )
	    sprintf ( tmphn, "virtualmachine0000%d", i);  
	  else if ( i/1000 == 0 )
	    sprintf ( tmphn, "virtualmachine000%d", i);  	  
	  else if ( i/10000 == 0 )
	    sprintf ( tmphn, "virtualmachine00%d", i);    
	  else if ( i/100000 == 0 )
	    sprintf ( tmphn, "virtualmachine0%d", i);    
	  else
	    sprintf ( tmphn, "virtualmachine%d", i);

	  MPIR_proctable[i].host_name = strdup (tmphn);
	  MPIR_proctable[i].executable_name = strdup("appX");
	  MPIR_proctable[i].pid = i;
	}
      totalview_jobid = strdup("387266");
      MPIR_Breakpoint();
    }

  return NULL;
}


#include <sys/types.h>

void * 
abort_debugger ( void* arg )
{
  if ( MPIR_being_debugged ) 
    {
      MPIR_debug_state = MPIR_DEBUG_ABORTING;	
      MPIR_Breakpoint();
    }

  return NULL;
}


void * 
do_something ( void* arg )
{
  int cnt = 0;
  long tid = ((long)arg);

  if (tid == 24) 
    {
      setup_debugger(arg);
    }

  while (1) {
   
    sleep(1);
 
    cnt++;
    //fprintf ( stdout, ".",tid );
    //fflush ( stdout );
	
    if ( cnt == 3 ) 
      {
	break;
      }
  }       
   
  fprintf ( stdout, "tid[%ld]done \n", tid);
  pthread_exit(NULL);
}


void 
print_usage ()
{
  fprintf ( stdout, "usage: mpirun-modelchker [OPTIONS]\n");
  fprintf ( stdout, "OPTIONS\n");
  fprintf ( stdout, "%s \n\t\t\tspecify process count to emulate\n", "-p<process count>:" );
  fprintf ( stdout, "%s \n\t\t\terror injection type [0~4]\n", "-e<error type>: " );
  fprintf ( stdout, "%s \n\t\t\t0 for main thread, 1 for pthread\n", "-t<which thread to call MPIR_Breakpoint>:" );
  fprintf ( stdout, "%s \n\t\t\tprint this message\n", "-h:");

  exit(0);
  
}


void 
echo_argv (int argc, char** argv)
{
  int i;

  fprintf ( stdout, "[LaunchMON MODEL CHECKER]: Below is the launch string invoked by LaunchMON\n");

  for ( i = 0; i < argc; i++ )    
    fprintf ( stdout, "[LaunchMON MODEL CHECKER]: %s\n", argv[i]);   

  exit(0);
}


int modelchecker_run( int argc, char* argv[])
{
  int i = 0;
  int errorlevel;

  myopt.pcount = 16;
  myopt.emode = err0;
  myopt.thr = slavethread;
  
  while ( i < argc )
    {      
      if ( argv[i][0] == '-' )
	{
	  switch ( argv[i][1] )
	    {
	    case 'p':	     
	      if ( ( myopt.pcount = atoi ( argv[i]+2 ) ) < 0 )
		{
		  myopt.pcount = 16;
		  fprintf ( stderr, 
			    "[LaunchMON MODEL CHECKER]: cannot convert %s to an pcount\n", 
			    &(argv[i][2]));
		}		
	      break;
	      
	    case 'e':	     
	      errorlevel = atoi ( argv[i]+2 );
	      if ( errorlevel < 0 || errorlevel > 4 )
		{
		  fprintf ( stderr, 
			    "[LaunchMON MODEL CHECKER]: unknown error level: %s\n", 
			    &(argv[i][2]));
		}
	      else
		{
		  switch ( argv[i][2] )
		    {
		    case '0':
		      myopt.emode = err0;
		      break;
		    case '1':
		      myopt.emode = err1;
		      break;
		    case '2':
		      myopt.emode = err2;
		      break;
		    case '3':
		      myopt.emode = err3;
		      break;
		    case '4':
		      myopt.emode = err4;
		      break;
		    default:
		      myopt.emode = err0;
		      break;		      
		    }		   
		}
	      break;

	    case 't':	      
	      if ( argv[i][2] == '0' )
		myopt.thr = mainthread;	       
	      else if ( argv[i][2] == '1')
		myopt.thr = slavethread;
	      else
		{
		  fprintf ( stderr, 
			    "[LaunchMON MODEL CHECKER]: unknown thread option: %s\n", 
			    &(argv[i][2]));
		  myopt.thr = mainthread;
		}		
	      break;

	    case 'h':
	      print_usage ();
	      break;

	    case 'c':
	      echo_argv (argc, argv);
	      break;

	    default:
	      fprintf ( stderr, "[LaunchMON MODEL CHECKER]: unknown option: %c, ignore\n", argv[i][1] ); 
	      break;
	    }
	}
	  i++;            
    }

  if ( myopt.thr == slavethread )
    {
      pthread_t thr[256];
      long i;
	  
      fprintf ( stdout, "[LaunchMON MODEL CHECKER]: Starting mpirun model checker...\n");
      fprintf ( stdout, "[LaunchMON MODEL CHECKER]: Process Count: %d\n", 
		myopt.pcount);
      fprintf ( stdout, "[LaunchMON MODEL CHECKER]: Pthreads calling MPIR_Breakpoint %d\n", 
		myopt.thr);
	  
      for (i=0; i < 256; i++) 	  
	pthread_create(&thr[i], NULL, do_something, (void *) i);

      //pthread_create(&thr[1023], NULL, setup_debugger, (void *) argv[0]);
	
      for (i=0; i < 256; i++)
	pthread_join(thr[i], NULL);
	
      fprintf ( stdout, "[LaunchMON MODEL CHECKER]: Finishing up...\n");
      pthread_create(&thr[0], NULL, abort_debugger, (void *) argv[0]);
      pthread_join(thr[0], NULL);
    }
  else
    {
      fprintf ( stdout, "[LaunchMON MODEL CHECKER]: Starting mpirun model checker...\n");
      fprintf ( stdout, "[LaunchMON MODEL CHECKER]: Process Count: %d\n", 
		myopt.pcount);
      fprintf ( stdout, "[LaunchMON MODEL CHECKER]: Main process thread calling MPIR_Breakpoint %d\n", 
		myopt.pcount);

	  
      setup_debugger ((void*) argv[0]);
      abort_debugger ((void*) argv[0]);
    }
 
  fprintf ( stdout, "\n****************************************************************\n");
  fprintf ( stdout,
    "[LMON LE: OK] MPI job launcher model checker reached the end\n");
  fprintf ( stdout,
    "[LMON LE: OK] Please check if launchmon fetched all RPDTAB entries of %d tasks\n", myopt.pcount);
  fprintf ( stdout, "****************************************************************\n\n");

  return 0;
}

int
main ( int argc, char* argv[])
{
  return modelchecker_run(argc, argv);
}
