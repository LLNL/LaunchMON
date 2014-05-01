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
 *        Dec 16 2009 DHA: Added limits.h
 *        Sep 14 2009 DHA: Added Blue Gene support.
 *        Jun 12 2008 DHA: Added GNU build system support.
 *        Feb 09 2008 DHA: Added LLNS Copyright.
 *        Dec 04 2007 DHA: Change %x to %lx for sscanf
 *                         to quiet the compilation. 
 *        Aug 08 2007 DHA: Created file.
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/common.h>

#define __GNU_SOURCE

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <signal.h>
#include <limits.h>

#if SUB_ARCH_BGL || SUB_ARCH_BGP
# include "debugger_interface.h"
  using namespace DebuggerInterface;
#else 
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/resource.h>
#include <errno.h>
#endif

#include <lmon_api/lmon_be.h>

using namespace std;

#if MEASURE_TRACING_COST
extern "C" {
extern int begin_timer ();
extern int time_stamp ( const char* description );
}
#endif

const int MAX_TID          = 0x0fffffff;
const int JIFFY_CONVERSION = 10000;
const int LENGTH_REQ_4_HEX = 18;
const int PAGESIZE         = 4096;

struct ProcStat {
  int mpiRank; 
  int pid;                  /* process id; use %d */
  char comm[PATH_MAX];      /* exec name; use %s */
  char state;               /* RSDZTW; use %c */
  int nThread;
  unsigned long startStack;
  unsigned long stkSp;    /* %lu stack pointer */
  unsigned long stkIp;    /* %lu instruction pointer*/

  typedef union {
#if SUB_ARCH_BGL || SUB_ARCH_BGP
    // conditional compilation to save space
    struct {
      double jobtime;
    } BgpStat;
#else
    struct {
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
      unsigned long cutime;     /* # of jiffies in user mode, which the process waited for children have made: %ld */
      unsigned long cstime;     /* # of jiffies in system mode, which the process waited for children have made: %ld */
      unsigned long priority;   /* %ld */
      unsigned long nice;
      unsigned long itrealvalue;     /* %lu */
      unsigned long long starttime;  /* %lu */
      unsigned long vsize;      /* %lu */
      long rss;                 /* %ld */
      unsigned long rlim;       /* %lu */
      unsigned long startcode;  /* %lu */
      unsigned long endcode;    /* %lu */
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
    } LinuxStat;
#endif
  } XStat;

  XStat xStat;
};

struct MemStat {
  typedef union {
#if SUB_ARCH_BGL || SUB_ARCH_BGP
    // conditional compilation to save space
    struct {
      unsigned long brk;
      unsigned long heapStart;
      unsigned long sharedMem; 
      unsigned long persistentMem;
      unsigned long mmapMem;
    } BgpMemStat;
#else
    struct {
      unsigned long vm_high_watermark;
      unsigned long vm_lock_memory;
      unsigned long rss_high_watermark;
      unsigned long vm_data;
      unsigned long vm_stack; 
      unsigned long vm_lib;
      unsigned long vm_exe;
      unsigned long brk;
      unsigned long brkstart;
    } LinuxMemStat;
#endif
  } XMem;

  XMem xMem;
};

#if SUB_ARCH_BGL || SUB_ARCH_BGP
/*
 * Utility routines to talk to CIOD
 */
static bool
sendMessage(BG_Debugger_Msg &msg)
{
  bool rc = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);

  // There was an error writing to the pipe.
  if (rc == false) {
     cerr << "[JOBSNAP BE FAILED] Failed to write to CIOD" 
          << BG_Debugger_Msg::getMessageName(msg.header.messageType)
          << " to control daemon" << endl;
  }

  return rc;
}

static bool
receiveMessage(BG_Debugger_Msg &msg)
{
  bool rc = BG_Debugger_Msg::readFromFd(BG_DEBUGGER_READ_PIPE, msg);

  // There was an error reading from the pipe.
  if (rc == false) {
     cerr << "[JOBSNAP BE FAILED] Failed to read message from CIOD"
	  << endl;
  }

  return rc;
}

static bool
receiveMessage(BG_Debugger_Msg &msg, BG_MsgType_t type, uint32_t length)
{
   bool rc = BG_Debugger_Msg::readFromFd(BG_DEBUGGER_READ_PIPE, msg);

   // There was an error reading from the pipe.
   if (rc == false) {
     cerr << "[JOBSNAP BE FAILED] Failed to read message from CIOD"
	  << endl;
     return false;
   }

   if (msg.header.messageType != type) {
      cerr << "[JOBSNAP BE FAILED] Response %s message is not the expected message"
	   << endl;
      return false;
   }

   if (msg.header.returnCode != 0) {
      cerr << "[JOBSNAP BE FAILED] Bad return code" << endl;
      return false;
   }

   if (msg.header.dataLength != length) {
      cerr << "[JOBSNAP BE FAILED] Bad data length" << endl;
      return false;
   }

   return rc;
}
#endif

int
main(int argc, char *argv[])
{
  using namespace std;

  MPIR_PROCDESC_EXT *proctab;
  ProcStat *tps = NULL;
  MemStat *tms = NULL;
  ProcStat *gatheredTps = NULL;
  MemStat *gatheredTms = NULL;

  int i, k, t;
  int maxSize=0, *aggrSizes=NULL;
  int rank;
  int be_size;
  int proctab_size;
  int signum = SIGCONT;
  bool rcFromDMsg;
  lmon_rc_e lrc = LMON_EINVAL;

  //
  // LMON Backend API initialization
  //
  lrc = LMON_be_init(LMON_VERSION, &argc, &argv);
  if ( lrc != LMON_OK ) {
      cerr << "[JOBSNAP BE: FAILED] LMON_be_init" << endl;

      return EXIT_FAILURE;
  }

  if (argc > 1)
    signum = atoi(argv[1]);

  //
  // LMON Backend rank that will be used in generating 
  // error messages if any
  //
  LMON_be_getMyRank (&rank);
  LMON_be_getSize (&be_size);

  //
  // LMON BE-FE handshaking
  //
  lrc = LMON_be_handshake(NULL);
  if ( lrc != LMON_OK ) {
      cerr << "[JOBSNAP BE("
	   << rank
	   << "): FAILED] LMON_be_handshake"
	   << endl;
      LMON_be_finalize();

      return EXIT_FAILURE;
  }

  //
  // Handshake is over; All backend daemons are synchronized with
  // a be_ready.
  //
  if ( (lrc = LMON_be_ready(NULL)) != LMON_OK ) {
      cerr << "[JOBSNAP BE("
	   << rank
	   << "): FAILED] LMON_be_ready"
	   << endl;
      LMON_be_finalize();

      return EXIT_FAILURE;
  }

  //
  // Testing the size of the local proctab
  // note that proctab_size could differ across different BEs
  //
  lrc = LMON_be_getMyProctabSize (&proctab_size);
  if ( lrc != LMON_OK) {
      cerr << "[JOBSNAP BE("
	   << rank
	   << "): FAILED] LMON_be_getMyProctabSize"
	   << endl;
      LMON_be_finalize();

      return EXIT_FAILURE;
  }

  //
  // determine max local size?
  //
  aggrSizes = new int[be_size];

  if ( LMON_be_gather(&proctab_size, sizeof(int), aggrSizes) < 0 ) {
    cerr << "[JOBSNAP BE("
	 << rank
	 << "): FAILED] LMON_be_gather"
	 << endl;

    LMON_be_finalize();

    return EXIT_FAILURE;
  }

  if ( LMON_be_amIMaster () == LMON_YES ) {
    for(k=0; k < be_size; ++k) {
      if (aggrSizes[k] > maxSize)
        maxSize= aggrSizes[k];
    }
  }

  delete aggrSizes;

  if ( LMON_be_broadcast(&maxSize,sizeof(int)) < 0) {
    cerr << "[JOBSNAP BE("
	 << rank
	 << "): FAILED] LMON_be_broadcast"
	 << endl;

    LMON_be_finalize();

    return EXIT_FAILURE;
  }

  proctab = (MPIR_PROCDESC_EXT *) 
           malloc (maxSize*sizeof(MPIR_PROCDESC_EXT));
  if ( proctab == NULL )
    {
      fprintf(stdout, 
        "[LMON BE(%d)] FAILED: malloc returned null\n",
        rank );
      LMON_be_finalize();

      return EXIT_FAILURE;
    }

  tps = (ProcStat *) 
        malloc (maxSize*sizeof(ProcStat));
  if ( tps == NULL )
    {
      fprintf(stdout, 
        "[LMON BE(%d)] FAILED: malloc returned null\n",
        rank );
      LMON_be_finalize();

      return EXIT_FAILURE;
    }

  tms = (MemStat *) 
        malloc (maxSize*sizeof(MemStat));
  if ( tms  == NULL )
    {
      fprintf(stdout, 
        "[LMON BE(%d)] FAILED: malloc returned null\n",
        rank );
      LMON_be_finalize();

      return EXIT_FAILURE;
    }

  //
  // Fetching per-node proctable
  //
  lrc = LMON_be_getMyProctab(proctab, &proctab_size, proctab_size);
  if ( lrc != LMON_OK ) {
      cerr << "[JOBSNAP BE("
	   << rank
	   << "): FAILED] LMON_be_getMyProctab"
	   << endl;
      LMON_be_finalize();

      return EXIT_FAILURE;
  }


  for(i=0; i < proctab_size; i++) {
#if SUB_ARCH_BGL || SUB_ARCH_BGP
      //
      // BGP specific process snapshot interface
      //
      BG_Process_Data_t procData;
      BG_Thread_Data_t mainThrData;
      BG_Debugger_Msg response;

      tps[i].mpiRank = proctab[i].mpirank;
      tps[i].pid = proctab[i].pd.pid;
      strncpy (tps[i].comm, proctab[i].pd.executable_name, PATH_MAX);

      //
      // please attach
      //
      BG_Debugger_Msg attach(ATTACH, proctab[i].pd.pid, 0, 0, 0);
      attach.header.dataLength = sizeof(attach.dataArea.ATTACH);

      if (!sendMessage(attach)) {
        LMON_be_finalize();

        return EXIT_FAILURE;
      }

      if (!receiveMessage(response, ATTACH_ACK, sizeof(response.dataArea.ATTACH_ACK))) {
        LMON_be_finalize();

        return EXIT_FAILURE;
      }

      //
      // please stop
      //
      BG_Debugger_Msg stop(KILL, proctab[i].pd.pid, 0, 0, 0);
      stop.dataArea.KILL.signal = SIGSTOP;
      stop.header.dataLength = sizeof(stop.dataArea.KILL);

      if (!sendMessage(stop)) {
        LMON_be_finalize();

        return EXIT_FAILURE;
      }

      if (!receiveMessage(response, KILL_ACK, sizeof(response.dataArea.KILL_ACK))) {
        LMON_be_finalize();

        return EXIT_FAILURE;
      }

      //
      // please wait for signal encountered event
      //
      if (!receiveMessage(response, SIGNAL_ENCOUNTERED, sizeof(response.dataArea.SIGNAL_ENCOUNTERED))) {
        LMON_be_finalize();

        return EXIT_FAILURE;
      }

      //
      // please GET_THREAD_INFO
      //
      BG_Debugger_Msg threadInfo(GET_THREAD_INFO, proctab[i].pd.pid,0,0,0);
      if (!sendMessage(threadInfo)) {
        LMON_be_finalize();

        return EXIT_FAILURE;
      }

      if (!receiveMessage(response, GET_THREAD_INFO_ACK, sizeof(response.dataArea.GET_THREAD_INFO_ACK))) {
        LMON_be_finalize();
        return EXIT_FAILURE;
      }

      tps[i].nThread = response.dataArea.GET_THREAD_INFO_ACK.numThreads;

      //
      // the main thread id will be the smallest of available thread ids, which
      // is larger than 4 (1-4 are system threads
      //
      int mainTid = MAX_TID;
      for (t=0; t < tps[i].nThread ; ++t) {
	int targetTid = response.dataArea.GET_THREAD_INFO_ACK.threadIDS[t];
        if ( (targetTid > 4) && (targetTid < mainTid) )
	  mainTid =  targetTid;
      }

      if ( mainTid == MAX_TID ) {
        cerr << "[JOBSNAP BE("
	     << rank
	     << "): FAILED] main thread id info is not available."
	     << endl;

        LMON_be_finalize();
        return EXIT_FAILURE;
      }

      //
      // please GET_PROCESS_DATA
      //
      BG_Debugger_Msg processData(GET_PROCESS_DATA, proctab[i].pd.pid, mainTid, 0, 0);
      processData.header.dataLength = sizeof(processData.dataArea.GET_PROCESS_DATA);

      if (!sendMessage(processData)) {
        LMON_be_finalize();

        return EXIT_FAILURE;
      }

      if (!receiveMessage(response, GET_PROCESS_DATA_ACK, sizeof(response.dataArea.GET_PROCESS_DATA_ACK))) {
        LMON_be_finalize();

        return EXIT_FAILURE;
      }

      memcpy( &procData, 
	      &(response.dataArea.GET_PROCESS_DATA_ACK.processData), 
              sizeof(procData));

      tps[i].xStat.BgpStat.jobtime 
        = (double) procData.jobTime.tv_sec + (double) procData.jobTime.tv_usec/1000000.0;
      tms[i].xMem.BgpMemStat.brk 
        = procData.heapBreakAddr;
      tms[i].xMem.BgpMemStat.heapStart 
        = procData.heapStartAddr;
      tms[i].xMem.BgpMemStat.sharedMem 
        = procData.sharedMemoryStartAddr - procData.sharedMemoryEndAddr;
      tms[i].xMem.BgpMemStat.persistentMem 
        = procData.persistMemoryStartAddr - procData.persistMemoryEndAddr;
      tms[i].xMem.BgpMemStat.mmapMem 
        = procData.mmapStartAddr - procData.mmapEndAddr;

      //
      // please GET_THREAD_DATA for the main thread
      //
      BG_Debugger_Msg threadData(GET_THREAD_DATA, proctab[i].pd.pid, mainTid, 0, 0);
      threadData.header.dataLength = sizeof(processData.dataArea.GET_THREAD_DATA);

      if (!sendMessage(threadData)) {
        LMON_be_finalize();

        return EXIT_FAILURE;
      }

      if (!receiveMessage(response, GET_THREAD_DATA_ACK, sizeof(response.dataArea.GET_THREAD_DATA_ACK))) {
        LMON_be_finalize();

        return EXIT_FAILURE;
      }

      memcpy( &mainThrData, 
              &(response.dataArea.GET_THREAD_DATA_ACK.threadData), 
              sizeof(mainThrData));

      if ( mainThrData.threadID != mainTid) {
        LMON_be_finalize();

        return EXIT_FAILURE;
      }

      //
      // get the process state 
      //
      switch (mainThrData.state) {
	case Running:
          tps[i].state = 'R';
	  break;
        case Sleeping:
          tps[i].state = 'S';
	  break;
        case Waiting:
          tps[i].state = 'W';
	  break;
	case Zombie:
          tps[i].state = 'Z';
	  break;
	case Idle:
          tps[i].state = 'I';
	  break;
      }

      tps[i].startStack = mainThrData.stackStartAddr;    // 
      tps[i].stkSp = mainThrData.gprs.gpr[1];            // R01 is the stack pointer register
      tps[i].stkIp = mainThrData.gprs.iar;               // IAR is instruction pointer

      //
      // please continue
      //
      BG_Debugger_Msg conti(CONTINUE, proctab[i].pd.pid, 0, 0, 0);
      conti.header.dataLength = sizeof(processData.dataArea.CONTINUE);

      if (!sendMessage(conti)) {
        LMON_be_finalize();

        return EXIT_FAILURE;
      }

      if (!receiveMessage(response, CONTINUE_ACK, sizeof(response.dataArea.CONTINUE_ACK))) {
        LMON_be_finalize();

        return EXIT_FAILURE;
      }

#else 
     //
     // Linux specific process snapshot interface 
     //
      ostringstream ost1, ost2;
      ifstream ifst;
      char line[4096];
      size_t len;

      ost1 << "/proc/" << proctab[i].pd.pid << "/stat";

      // how do you want to check exceptions?
      ifst.open(ost1.str().c_str(), ios::in);

      tps[i].mpiRank = proctab[i].mpirank;

      ifst >> tps[i].pid
	   >> tps[i].comm
	   >> tps[i].state
	   >> tps[i].xStat.LinuxStat.ppid // ppid
	   >> tps[i].xStat.LinuxStat.pgid
	   >> tps[i].xStat.LinuxStat.session
	   >> tps[i].xStat.LinuxStat.tty_nr
	   >> tps[i].xStat.LinuxStat.tpgid
	   >> tps[i].xStat.LinuxStat.flags
	   >> tps[i].xStat.LinuxStat.minflt
	   >> tps[i].xStat.LinuxStat.cminflt
	   >> tps[i].xStat.LinuxStat.majflt
	   >> tps[i].xStat.LinuxStat.cmajflt
	   >> tps[i].xStat.LinuxStat.utime
	   >> tps[i].xStat.LinuxStat.stime
	   >> tps[i].xStat.LinuxStat.cutime
	   >> tps[i].xStat.LinuxStat.cstime
	   >> tps[i].xStat.LinuxStat.priority
	   >> tps[i].xStat.LinuxStat.nice
	   >> tps[i].nThread
	   >> tps[i].xStat.LinuxStat.itrealvalue
	   >> tps[i].xStat.LinuxStat.starttime
	   >> tps[i].xStat.LinuxStat.vsize
	   >> tps[i].xStat.LinuxStat.rss
	   >> tps[i].xStat.LinuxStat.rlim
	   >> tps[i].xStat.LinuxStat.startcode
	   >> tps[i].xStat.LinuxStat.endcode
	   >> tps[i].startStack
	   >> tps[i].stkSp
	   >> tps[i].stkIp
	   >> tps[i].xStat.LinuxStat.signal
	   >> tps[i].xStat.LinuxStat.blocked
	   >> tps[i].xStat.LinuxStat.sigignore
	   >> tps[i].xStat.LinuxStat.sigcatch
	   >> tps[i].xStat.LinuxStat.wchan
	   >> tps[i].xStat.LinuxStat.nswap
	   >> tps[i].xStat.LinuxStat.cnswap
	   >> tps[i].xStat.LinuxStat.exit_signal
	   >> tps[i].xStat.LinuxStat.processor
	   >> tps[i].xStat.LinuxStat.rt_prior
	   >> tps[i].xStat.LinuxStat.policy;

      ifst.close();

      ost2 << "/proc/" << proctab[i].pd.pid << "/status";

      // how do you want to check exceptions?
      ifst.open(ost2.str().c_str(), ios::in);

      while (!ifst.eof()) {
        ifst.getline(line, 4096);
	char* tok;
	char* linebuf = strdup (line);
	tok = strtok ( linebuf, ": \t");

        if (!tok)
          continue;

	if ( strcmp ( tok, "VmPeak") == 0 ) {
	  tok = strtok ( NULL, ": \t");
	  tms[i].xMem.LinuxMemStat.vm_high_watermark = atoi(tok)*1024;
	}
	else if ( strcmp ( tok, "VmLck") == 0 ) {
	   tok = strtok ( NULL, ": \t");
	   tms[i].xMem.LinuxMemStat.vm_lock_memory = atoi(tok)*1024;
	}
	else if ( strcmp ( tok, "VmHWM") == 0 ) {
	  tok = strtok ( NULL, ": \t");
	  tms[i].xMem.LinuxMemStat.rss_high_watermark = atoi(tok)*1024;
	}
	else if ( strcmp ( tok, "VmData") == 0 ) {
	  tok = strtok ( NULL, ": \t");
	  tms[i].xMem.LinuxMemStat.vm_data = atoi(tok)*1024;
	}
	else if ( strcmp ( tok, "VmStk") == 0 ) {
	  tok = strtok ( NULL, ": \t");
	  tms[i].xMem.LinuxMemStat.vm_stack = atoi(tok)*1024;
	}
	else if ( strcmp ( tok, "StaBrk") == 0 ) {
	  tok = strtok ( NULL, ": \t");
          sscanf (tok, "%lx", &(tms[i].xMem.LinuxMemStat.brkstart));
	  tms[i].xMem.LinuxMemStat.brkstart *= 1024;
	}
	else if ( strcmp ( tok, "Brk") == 0 ) {
	   tok = strtok ( NULL, ": \t");
	   sscanf (tok, "%lx", &(tms[i].xMem.LinuxMemStat.brk));
           tms[i].xMem.LinuxMemStat.brk *= 1024;
	}
	else if ( strcmp ( tok, "VmLib") == 0 ) {
	   tok = strtok ( NULL, ": \t");
	   tms[i].xMem.LinuxMemStat.vm_lib= atoi(tok)*1024;
	}
	else if ( strcmp ( tok, "VmExe") == 0 ) {
	   tok = strtok ( NULL, ": \t");
	   tms[i].xMem.LinuxMemStat.vm_exe= atoi(tok)*1024;
	}
	free (linebuf);
      }

      ifst.close();
#endif //SUB_ARCH_BGP vs LINUX with /proc
  } // local proctab loop

  //
  // fill the potentially remaining elements with the dummy rank 
  //
  while ( i < maxSize )
    tps[i++].mpiRank = -1;

  if ( LMON_be_amIMaster () == LMON_YES ) {
    gatheredTps = (ProcStat *) malloc(be_size*maxSize*sizeof(ProcStat));
    gatheredTms = (MemStat *) malloc(be_size*maxSize*sizeof(MemStat));
  }

  lrc =  LMON_be_gather((void *) tps, 
                        sizeof(ProcStat)*maxSize, 
                        (void *) gatheredTps);

  if ( lrc != LMON_OK ) {
      cerr << "[JOBSNAP BE("
	   << rank
	   << "): FAILED] LMON_be_gather"
	   << endl;

      LMON_be_finalize();

      return EXIT_FAILURE;
  }

  lrc = LMON_be_gather((void *) tms,
		       sizeof(MemStat)*maxSize, 
		       (void *) gatheredTms);

  if ( lrc != LMON_OK ) {
      cerr << "[JOBSNAP BE("
	   << rank
	   << "): FAILED] LMON_be_gather"
	   << endl;

      LMON_be_finalize();

      return EXIT_FAILURE;
  }

  if  ( LMON_be_amIMaster() == LMON_YES ) {
     //
     // the BE master is responsible for reporting the gathered data 
     //
     ofstream ofs;
     char fn[PATH_MAX];
     time_t now;
     time(&now);
     strftime ( fn, PATH_MAX, "./jobsnap.out.%Y.%m.%d.%H.%M.%S", localtime(&now));

     ofs.open(fn, ios::out);

#if MEASURE_TRACING_COST
     begin_timer ();
#endif

     ofs << setw(6) << "Rank" << setw(24) << "Execname" << setw(6) << "State" << setw(20) << "InstPtr" << setw(8) << "nThread" 
         << setw(20) << "StartStk" << setw(20) << "StackPtr" << setw(15) << "Brk Size(Byte)" << setw(15) << "Utime (secs)" << setw(15) << "Stime (secs)" << endl; 

     for ( i=0; i < be_size*maxSize; ++i ) {
       if ( gatheredTps[i].mpiRank != -1 )  {
         char bname[24];
	 char *execname = strdup(gatheredTps[i].comm);
         snprintf(bname, 24, basename(execname));
         ofs << setw(6)  << dec << gatheredTps[i].mpiRank 
             << setw(24) << bname
	     << setw(6)  << gatheredTps[i].state
	     << "  Ox" << setfill('0') << setw(16) << hex << gatheredTps[i].stkIp << setfill(' ')
	     << setw(8)  << gatheredTps[i].nThread
	     << "  Ox" << setfill('0') << setw(16) << hex << gatheredTps[i].startStack
	     << "  Ox" << setfill('0') << setw(16) << hex << gatheredTps[i].stkSp << setfill(' ')
#if SUB_ARCH_BGL || SUB_ARCH_BGP
	     << setw(15) << dec << (gatheredTms[i].xMem.BgpMemStat.brk - gatheredTms[i].xMem.BgpMemStat.heapStart)
	     << setw(15) << (double) gatheredTps[i].xStat.BgpStat.jobtime
	     << setw(15) << 0.0
	     << endl;
#else
	     << setw(15) << "Not avail yet" //dec << (gatheredTms[i].xMem.LinuxMemStat.brk - gatheredTms[i].xMem.LinuxMemStat.brkstart)
	     << setw(15) << (double)(gatheredTps[i].xStat.LinuxStat.utime)/100.0 
	     << setw(15) << (double)(gatheredTps[i].xStat.LinuxStat.stime)/100.0 
	     << endl;
#endif
           free(execname);
         }
       }

     ofs.close();

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

  if ( signum != SIGCONT ) {
      //
      // this is only to support the regression test case
      //
#if SUB_ARCH_BGL || SUB_ARCH_BGP
       for (i=0; i < proctab_size; i++) {
	  //
          // please stop so that I can continue with a signal
          //
          BG_Debugger_Msg dbgmsg(KILL,proctab[i].pd.pid,0,0,0);
          dbgmsg.dataArea.KILL.signal = SIGSTOP;
          dbgmsg.header.dataLength = sizeof(dbgmsg.dataArea.KILL);
          BG_Debugger_Msg ackmsg;
          BG_Debugger_Msg ackmsg2;

	  if(! sendMessage(dbgmsg)) {
            LMON_be_finalize();

            return EXIT_FAILURE;
          }

	  if (!receiveMessage(ackmsg, KILL_ACK, sizeof(ackmsg.dataArea.KILL_ACK))) {
            LMON_be_finalize();

            return EXIT_FAILURE;
          }
 
	  if (!receiveMessage(ackmsg2, SIGNAL_ENCOUNTERED, sizeof(ackmsg2.dataArea.SIGNAL_ENCOUNTERED))) {
            LMON_be_finalize();

            return EXIT_FAILURE;
	  }
      }

      for (i=0; i < proctab_size; i++) {
	  //
          // please continue with a signal
          //
          BG_Debugger_Msg dbgmsg(CONTINUE,proctab[i].pd.pid,0,0,0);
          BG_Debugger_Msg ackmsg;
          dbgmsg.dataArea.CONTINUE.signal = signum;
          dbgmsg.header.dataLength = sizeof(dbgmsg.dataArea.CONTINUE);

	  if(! sendMessage(dbgmsg)) {
            LMON_be_finalize();
            return EXIT_FAILURE;
          }

	  if (!receiveMessage(ackmsg, CONTINUE_ACK, sizeof(ackmsg.dataArea.CONTINUE_ACK))) {
            LMON_be_finalize();
            return EXIT_FAILURE;
          }
      }
#else

      for(i=0; i < proctab_size; i++) {
	  kill(proctab[i].pd.pid, signum);
      }
#endif
  }

  if (proctab) 
    free (proctab);

  if (tps)
    free (tps);

  if (tms)
    free (tms);

  if ( LMON_be_amIMaster () == LMON_YES ) {
    if (gatheredTps)
      free (gatheredTps);
    if (gatheredTms)
      free (gatheredTms);
  }

  LMON_be_finalize();

  return EXIT_SUCCESS;
}

