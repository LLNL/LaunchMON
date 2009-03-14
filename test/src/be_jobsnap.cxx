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
 *        Jun 12 2008 DHA: Added GNU build system support.
 *        Feb 09 2008 DHA: Added LLNS Copyright.
 *        Dec 04 2007 DHA: Change %x to %lx for sscanf
 *                         to quiet the compilation. 
 *        Aug 08 2007 DHA: Created file.          
 */

#include <lmon_api/common.h>

#define __GNU_SOURCE

#if HAVE_UNISTD_H
# include <unistd.h>
#else
# error unistd.h is required
#endif

#include <iostream>

#if HAVE_SIGNAL_H
# include <signal.h>
#else
# error signal.h is required 
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#else
# error sys/types.h is required
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#else
# error sys/wait.h is required
#endif

#if HAVE_SYS_PTRACE_H
# include <sys/ptrace.h>
#else
# error sys/ptrace.h is required
#endif 

#if HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#else
# error sys/resource.h is required 
#endif

#if HAVE_ERRNO_H
# include <errno.h>
#else
# error errno.h is required 
#endif

#include <lmon_api/lmon_be.h>

#if MEASURE_TRACING_COST
extern "C" {
extern int begin_timer ();
extern int time_stamp ( const char* description );
}
#endif

const int MAX_BE_PROCSIZE  = 8;
const int JIFFY_CONVERSION = 10000;
const int LENGTH_REQ_4_HEX = 18;
const int PAGESIZE         = 4096;

//
// procstat_t is created using the "proc" manpage
// But looking at the kernel code, the manpage seems out of date
// Following the kernel code
//
typedef struct _procstat_t {
  int mpirank; 

  int pid;                  /* process id; use %d */
  char comm[PATH_MAX];      /* exec name; use %s */
  char state;               /* RSDZTW; use %c */
  int ppid;                 /* parent's pid; use %d */
  int pgid;                 /* process group id; %d */

  int session;              /* session id; %d */
  int tty_nr;               /* the tty that the process uses; %d */
  int tpgid;                /* see manpage: %d */
  unsigned long flags;      /* the flags of the process; %lu */
  unsigned long minflt;     /* # of minor faults: %lu */

  unsigned long cminflt;    /* # of minor faults that the process waited for children have made: %lu */
  unsigned long majflt;     /* # of major faults; %lu */
  unsigned long cmajflt;    /* %lu */
  unsigned long utime;      /* # of jiffies (1/100th sec) in user mode: %lu */
  unsigned long stime;      /* # of jiffies (1/100th sec) in kernel mode: %lu */

  unsigned long cutime;              /* # of jiffies in user mode, which the process waited for children have made: %ld */
  unsigned long cstime;              /* # of jiffies in system mode, which the process waited for children have made: %ld */
  unsigned long priority;            /* %ld */
  unsigned long nice;
  int numthread;           /* addition */

  unsigned long itrealvalue;     /* %lu */
  unsigned long long starttime;  /* %lu */
  unsigned long vsize; /* %lu */
  long rss;            /* %ld */
  unsigned long rlim;  /* %lu */

  unsigned long startcode;  /* %lu */
  unsigned long endcode;    /* %lu */
  unsigned long startstack; /* %lu starting address of the stack */
  unsigned long kstkesp;    /* %lu stack pointer */
  unsigned long kstkeip;    /* %lu instruction pointer*/

  unsigned long signal;     /* %lu */
  unsigned long blocked;    /* %lu */
  unsigned long sigignore;  /* %lu */
  unsigned long sigcatch;   /* %lu */
  unsigned long wchan;      /* %lu */

  unsigned long nswap;      /* %lu */
  unsigned long cnswap;     /* %lu */
  int exit_signal;          /* %d */
  int processor;            /* %d */     
  unsigned long rt_prior;
  unsigned long policy;

} procstat_t;

typedef struct _memstat_t {
  unsigned long vm_high_watermark;
  unsigned long vm_lock_memory;
  unsigned long rss_high_watermark;
  unsigned long	vm_data;
  unsigned long vm_stack; 
  unsigned long vm_lib;
  unsigned long vm_exe;
  unsigned long brk;
  unsigned long brkstart;
} memstat_t;

const char tpsformat[] =
"%d %s %c %d %d\
 %d %d %d %lu %lu\
 %lu %lu %lu %lu %lu\
 %lu %lu %ld %ld %d\
 %lu %llu %lu %ld %lu\
 %lu %lu %lu %lu %lu\
 %lu %lu %lu %lu %lu\
 %lu %lu %d %d %lu %lu";

int 
main( int argc, char* argv[] )
{
  using namespace std;

  MPIR_PROCDESC_EXT proctab[MAX_BE_PROCSIZE];
  procstat_t tps[MAX_BE_PROCSIZE];
  memstat_t tms[MAX_BE_PROCSIZE];
  procstat_t* gathered_tps = NULL;
  memstat_t* gathered_tms = NULL;
  int i; 
  int rank;
  int be_size;
  int proctab_size;
  int signum = SIGCONT;
  lmon_rc_e lrc = LMON_EINVAL;

   
  /*
   * LMON Backend API initialization
   */
  if ( (lrc = LMON_be_init(LMON_VERSION, &argc, &argv)) 
              != LMON_OK )
    {      
      cerr << "[JOBSNAP BE: FAILED] LMON_be_init"
	   << endl;

      return EXIT_FAILURE;
    }

  if (argc > 1)
  {
    signum = atoi(argv[1]);
  }

  /*
   * LMON Backend rank that will be used in generating 
   * error messages if any
   */
  LMON_be_getMyRank (&rank);
  LMON_be_getSize (&be_size);

  /*
   * LMON BE-FE handshaking
   */
  if ( (lrc = LMON_be_handshake(NULL)) 
              != LMON_OK )
    {
      cerr << "[JOBSNAP BE("
	   << rank
	   << "): FAILED] LMON_be_handshake"
	   << endl;
      LMON_be_finalize();

      return EXIT_FAILURE;
    }

  /*
   * Handshake is over; All backend daemons are synchronized with
   * a be_ready. 
   */
  if ( (lrc = LMON_be_ready(NULL)) 
              != LMON_OK )
    {     
      cerr << "[JOBSNAP BE("
	   << rank
	   << "): FAILED] LMON_be_ready"
	   << endl;
      LMON_be_finalize();

      return EXIT_FAILURE;
    } 

  /*
   * Fetching per-node proctable
   */
  if ( (lrc = LMON_be_getMyProctab ( proctab, 
				     &proctab_size, 
				     MAX_BE_PROCSIZE)) 
              != LMON_OK )
    {   
      cerr << "[JOBSNAP BE("
	   << rank
	   << "): FAILED] LMON_be_getMyProctab"
	   << endl;    
      LMON_be_finalize();

      return EXIT_FAILURE;
    }

  for(i=0; i < proctab_size; i++)
    {
      char procfspath[PATH_MAX];      
      FILE *fptr = NULL;
      char *line = NULL;
      size_t len;
     
      sprintf ( procfspath, "/proc/%d/stat", proctab[i].pd.pid );            
      if ( ( fptr = fopen ( procfspath, "r")) == NULL )
	{
	  cerr << "[JOBSNAP BE("
	       << rank
	       << "): FAILED] "
	       << procfspath
	       << "cannot be read"
	       << endl;    

	  LMON_be_finalize();
	  return EXIT_FAILURE;
	}
      tps[i].mpirank = proctab[i].mpirank;
      fscanf ( fptr, tpsformat, &tps[i].pid, &tps[i].comm,
	       &tps[i].state, &tps[i].ppid, &tps[i].pgid,
	       &tps[i].session, &tps[i].tty_nr, &tps[i].tpgid,
	       &tps[i].flags, &tps[i].minflt, &tps[i].cminflt,
	       &tps[i].majflt, &tps[i].cmajflt, &tps[i].utime,
	       &tps[i].stime, &tps[i].cutime, &tps[i].cstime,
	       &tps[i].priority, &tps[i].nice, &tps[i].numthread, 
	       &tps[i].itrealvalue, &tps[i].starttime, &tps[i].vsize,
	       &tps[i].rss, &tps[i].rlim, &tps[i].startcode,
	       &tps[i].endcode, &tps[i].startstack, &tps[i].kstkesp,
	       &tps[i].kstkeip, &tps[i].signal, &tps[i].blocked,
	       &tps[i].sigignore, &tps[i].sigcatch, &tps[i].wchan,
	       &tps[i].nswap, &tps[i].cnswap, &tps[i].exit_signal,
	       &tps[i].processor, &tps[i].rt_prior, &tps[i].policy);      

      fclose(fptr); 

      sprintf ( procfspath, "/proc/%d/status", proctab[i].pd.pid );            
      if ( ( fptr = fopen ( procfspath, "r")) == NULL )
	{
	  cerr << "[JOBSNAP BE("
	       << rank
	       << "): FAILED] "
	       << procfspath
	       << "cannot be read"
	       << endl;    

	  LMON_be_finalize();
	  return EXIT_FAILURE;
	}

      while ((getline(&line, &len, fptr)) != -1) 
	{
	  char* tok;
	  char* linebuf = strdup (line);
	  tok = strtok ( linebuf, ": \t");
	  if ( strcmp ( tok, "VmPeak") == 0 )
	    {
	      tok = strtok ( NULL, ": \t");
	      tms[i].vm_high_watermark = atoi(tok)*1024;
	    }
	  else if ( strcmp ( tok, "VmLck") == 0 )
	    {
	      tok = strtok ( NULL, ": \t");
	      tms[i].vm_lock_memory = atoi(tok)*1024;
	    }
	  else if ( strcmp ( tok, "VmHWM") == 0 )
	    {
	      tok = strtok ( NULL, ": \t");
	      tms[i].rss_high_watermark = atoi(tok)*1024;
	    }
	  else if ( strcmp ( tok, "VmData") == 0 )
	    {
	      tok = strtok ( NULL, ": \t");
	      tms[i].vm_data = atoi(tok)*1024;
	    }
	  else if ( strcmp ( tok, "VmStk") == 0 )
	    {
	      tok = strtok ( NULL, ": \t");
	      tms[i].vm_stack = atoi(tok)*1024;
	    }
	  else if ( strcmp ( tok, "StaBrk") == 0 )
	    {
	      tok = strtok ( NULL, ": \t");
              sscanf (tok, "%lx", &(tms[i].brkstart));
	      tms[i].brkstart *= 1024;	     
	    }
	  else if ( strcmp ( tok, "Brk") == 0 )
	    {
	      tok = strtok ( NULL, ": \t");
	      sscanf (tok, "%lx", &(tms[i].brk));
              tms[i].brk *= 1024;	      
	    }
	  else if ( strcmp ( tok, "VmLib") == 0 )
	    {
	      tok = strtok ( NULL, ": \t");
	      tms[i].vm_lib= atoi(tok)*1024;
	    }
	  else if ( strcmp ( tok, "VmExe") == 0 )
	    {
	      tok = strtok ( NULL, ": \t");
	      tms[i].vm_exe= atoi(tok)*1024;
	    }
	  free (linebuf);
	}

      if (line)
	free(line);
     
      fclose(fptr);      
    }         

  while ( i < MAX_BE_PROCSIZE )
    {
      tps[i++].mpirank = -1;
    }
  
  if ( LMON_be_amIMaster () == LMON_YES )
    {
      gathered_tps = ( procstat_t* ) malloc ( 
			   be_size*sizeof(procstat_t)*MAX_BE_PROCSIZE);    
      if ( gathered_tps == NULL )
        {
          cerr << "[JOBSNAP BE("
	       << rank
	       << "): FAILED] malloc returned NULL"
	       << endl;    
          LMON_be_finalize ();

          return EXIT_FAILURE;
        }

      gathered_tms = ( memstat_t* ) malloc (
             be_size*sizeof(memstat_t)*MAX_BE_PROCSIZE );    
      if ( gathered_tms == NULL )
        {
          cerr << "[JOBSNAP BE("
	       << rank
	       << "): FAILED] malloc returned NULL"
	       << endl;    
          LMON_be_finalize ();

          return EXIT_FAILURE;
        }            
    }

  if ( (lrc = LMON_be_gather ( &tps, 
                               sizeof (procstat_t)*MAX_BE_PROCSIZE, 
                               gathered_tps ))
              != LMON_OK )
    {   
      cerr << "[JOBSNAP BE("
	   << rank
	   << "): FAILED] LMON_be_gather"
	   << endl;    

      LMON_be_finalize();

      return EXIT_FAILURE;
    }

  if ( (lrc = LMON_be_gather ( &tms, 
                               sizeof (memstat_t)*MAX_BE_PROCSIZE, 
                               gathered_tms ))
              != LMON_OK )
    {   
      cerr << "[JOBSNAP BE("
	   << rank
	   << "): FAILED] LMON_be_gather"
	   << endl;    

      LMON_be_finalize();

      return EXIT_FAILURE;
    }  

  if  ( LMON_be_amIMaster() == LMON_YES )
    {
      //
      // writing out the gathered bufs
      //	
      FILE* fout = NULL;
      char fn[PATH_MAX];
      time_t t;
      const char outformat[] =    "%6s %25s %6s %18s %11s %18s %18s %18s %18s %18s %18s %18s %18s %18s %18s %18s %15s %10s %10s\n";
      const char outformatval[] = "%6d %25s %6c %18x %11d %18x %18lu %18lu %18lu %18lu %18lu %18lu %18lu %18lu %18lu %18lu %15lu %10lu %10lu\n";
      time(&t); 
      strftime ( fn, PATH_MAX, "./jobsnap.out.%B-%d-%H-%M-%S", localtime(&t)); 

#if MEASURE_TRACING_COST
      begin_timer ();
#endif     
      if ( ( fout = fopen ( fn, "w")) == NULL )
	{
	  cerr << "[JOBSNAP BE("
	       << rank
	       << "): FAILED] "
	       << "fail to create jobsnap.out"
	       << endl;  
       
	  LMON_be_finalize();
	  
	  return EXIT_FAILURE;
	}
  
      fprintf ( fout, 
                outformat, 
                "Rank", 
                "Execname",  
                "State", 
                "PC(HEX)", 
                "NumThreads", 
	        "StartStk(HEX)",               
                "VMSize(Byte)", 
		"VMHighWM(Byte)",
		"VMLockMem(Byte)",
		"VMData(Byte)",
		"VMStk(Byte)",
		"VMLibs(Byte)",
		"VMExec(Byte)",
		"VMBrkSize(Byte)",
		"RSS HighWM(Byte)",
		"RSS Size(Byte)",    
                "MajorPageFault", 
                "Utime(us)", 
                "Stime(us)");

      for ( i=0; i < be_size*MAX_BE_PROCSIZE; i++ )
	{	 
	  if ( gathered_tps[i].mpirank != -1 )
	    {
	      fprintf ( fout, 
			outformatval, 
			gathered_tps[i].mpirank,
			gathered_tps[i].comm,
			gathered_tps[i].state,
			gathered_tps[i].kstkeip,
                        gathered_tps[i].numthread,
			gathered_tps[i].startstack,
			gathered_tps[i].vsize,
			gathered_tms[i].vm_high_watermark,
			gathered_tms[i].vm_lock_memory,
			gathered_tms[i].vm_data,
			gathered_tms[i].vm_stack, 
			gathered_tms[i].vm_lib,
			gathered_tms[i].vm_exe,
			gathered_tms[i].brk - gathered_tms[i].brkstart,
			gathered_tms[i].rss_high_watermark,
			(gathered_tps[i].rss+3)*PAGESIZE,									
                        gathered_tps[i].majflt,
			gathered_tps[i].utime*JIFFY_CONVERSION,
			gathered_tps[i].stime*JIFFY_CONVERSION );
	    }
	}      
    
      fclose (fout);
#if MEASURE_TRACING_COST
      time_stamp ( "LMON_be writing jobsnap output to a file" );
#endif
    }

#if MEASURE_TRACING_COST
  //
  // Although be_sendUsrData only affects the master, 
  // every task should call because it works as a barrier.  
  //
  LMON_be_sendUsrData ( NULL );
#endif

  if ( signum != SIGCONT )
    {
      //
      // this is for a regression test case
      //
      for(i=0; i < proctab_size; i++)
	{	  
	  kill(proctab[i].pd.pid, signum);
	}
    }

  LMON_be_finalize();

  return EXIT_SUCCESS;
}

