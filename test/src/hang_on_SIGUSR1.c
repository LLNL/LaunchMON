/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/testcases/src/linux/hang_on_SIGUSR1.c,v 1.3.2.2 2008/02/21 09:26:38 dahn Exp $
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
 *        Jun 13 2008 DHA: Added GNU build system support. 
 *        Mar 21 2008 DHA: Beautified the code a bit.  
 *        Mar 18 2008 DHA: Added BlueGene support. (Fixing a File IO bug) 
 *        Feb 09 2008 DHA: Added LLNS Copyright.
 *          
 */

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/common.h>

/*
 * This program may interfere with MPI implementations that 
 * use USR2 signal. 
 */ 
                                                                  
#if SUB_ARCH_BGL || SUB_ARCH_BGP
/* work around for a compiler problem on BlueGene/L */
#undef SEEK_SET
#undef SEEK_END
#undef SEEK_CUR
#endif
                                                                  
#include <mpi.h>
                                                                  
#if HAVE_SIGNAL_H
# include <signal.h>
#else
# error signal.h is required
#endif
                                                                  
#define COMM_TAG              1000
#define MAX_BUF_LEN           1024
#define COMPUTE_UNIT          20
#define SLEEP_FOR_COMPUTE_SEC 3

#if HAVE_STDARG_H
# include <stdarg.h>
#else
# error stdarg.h is required
#endif

#if HAVE_LIMITS_H
# include <limits.h>
#else
# error limits.h is required 
#endif

#if TIME_WITH_SYS_TIME
# include <time.h>
# include <sys/time.h>
#else
# error time.h and sys/time.h are required
#endif

//
// For the performance study of 4K-byte block fetching
// 10,000 x 4KB = 10,000 x 8B * 512
//
#define FETCH_ME_ARRAY_SIZE 5120000
double FETCH_ME_DBL_ARRAY[FETCH_ME_ARRAY_SIZE];

static int global_stall = 1;
static int global_rank = -1;

static void
LMON_say_msg ( const char* m, const char* output, ... )
{
  va_list ap;
  char timelog[PATH_MAX];
  char log[PATH_MAX];
  const char* format = "%b %d %T";
  time_t t;
  const char ei_str[] = "INFO";

  time(&t);
  strftime ( timelog, PATH_MAX, format, localtime(&t));
  snprintf(log, PATH_MAX, "<%s> %s (%s): %s\n", timelog, m, "INFO", output);

  va_start(ap, output);
  vfprintf(stdout, log, ap);
  va_end(ap);
}

static void
init_FETCH_ME_DBL_ARRAY()
{
  int i=0;
  for (i=0; i < FETCH_ME_ARRAY_SIZE; ++i)
    {
      FETCH_ME_DBL_ARRAY[i] = (double) i+1;
    }
#if 0
  char *tmp = (char*)FETCH_ME_DBL_ARRAY;
  fprintf (stdout, "[LMON APP] address: 0x%x\n", FETCH_ME_DBL_ARRAY);
  fprintf (stdout, "[LMON APP] 1st: 0x%x, %f\n", &FETCH_ME_DBL_ARRAY[0], FETCH_ME_DBL_ARRAY[0]);
  fprintf (stdout, "[LMON APP] 1st: %x %x %x %x %x %x %x %x \n", tmp[0], tmp[1], tmp[2], tmp[3], 
                                                                 tmp[4], tmp[5], tmp[6], tmp[7]);
  tmp = (char*)&FETCH_ME_DBL_ARRAY[512];
  fprintf (stdout, "[LMON APP] 512th: 0x%x %f\n", &FETCH_ME_DBL_ARRAY[512], FETCH_ME_DBL_ARRAY[512]);
  fprintf (stdout, "[LMON APP] 1st: %x %x %x %x %x %x %x %x \n", tmp[0], tmp[1], tmp[2], tmp[3], 
                                                                 tmp[4], tmp[5], tmp[6], tmp[7]);
#endif
}

void 
sighandler (int sig) 
{
  if ( sig == SIGUSR1)
  {
    global_stall = 0; /* unlock the hang */ 
    if (!global_rank)
      {
        fprintf (stdout, "[LMON APP] * Received SIGUSR1  * \n");
      }
  }
  return;
} 


static void 
pass_its_neighbor(const int rank, const int size, int* buf)
{  
  MPI_Request request[2];
  MPI_Status status[2];

  MPI_Irecv((void*)buf, 1, MPI_INT, ((rank+size-1)%size), COMM_TAG, MPI_COMM_WORLD, &request[0]);
  MPI_Isend((void*)&rank, 1, MPI_INT, ((rank+1)%size), COMM_TAG, MPI_COMM_WORLD, &request[1]);
  MPI_Waitall(2, request, status);

  MPI_Barrier(MPI_COMM_WORLD);
}


int 
main(int argc, char* argv[])
{
  int size, rank, i;  
  int buf;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  init_FETCH_ME_DBL_ARRAY ();
  signal(SIGUSR1, sighandler);
  global_rank = rank;
  if (rank == 0) 
    fprintf(stdout, "[LMON APP] singal handler installed for signum=%d\n", SIGUSR1); 
 
  while(global_stall == 1 ) {
    if (rank == 0)
      LMON_say_msg ( "APP", "stall for %d secs", SLEEP_FOR_COMPUTE_SEC );  
    sleep(SLEEP_FOR_COMPUTE_SEC);
  }

  if (rank == 0)
    fprintf(stdout, "The hang unlocked\n");

  pass_its_neighbor(rank, size, &buf);

  int remain = COMPUTE_UNIT * SLEEP_FOR_COMPUTE_SEC;
  for (i=0; i < COMPUTE_UNIT; i++)
    {
      sleep(SLEEP_FOR_COMPUTE_SEC);
      remain -= SLEEP_FOR_COMPUTE_SEC;
      if (rank == 0)
        LMON_say_msg ( "APP", "%d secs remain", remain);
    }

  if (rank == 0)
    LMON_say_msg ( "APP", "size of this program is %d\n", size);

  MPI_Finalize();
  return EXIT_SUCCESS;
}

