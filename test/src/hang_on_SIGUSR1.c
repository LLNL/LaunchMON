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
                                                                  
#if RM_BG_MPIRUN
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
                                                                  
#define COMM_TAG    1000
#define MAX_BUF_LEN 1024

static int global_stall = 1;
static int global_rank = -1;


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

  signal(SIGUSR1, sighandler);
  global_rank = rank;
  if (rank == 0) 
    fprintf(stdout, "[LMON APP] singal handler installed for signum=%d\n", SIGUSR1); 
 
  while(global_stall == 1 ) {
    if (rank == 0)
      fprintf(stdout, "[LMON APP] stall for a sec\n"); 
    sleep(1);
  }

  if (rank == 0)
    fprintf(stdout, "The hang unlocked\n");

  pass_its_neighbor(rank, size, &buf);
  i=0;
  while (i < 30) 
    {
      if (rank == 0)
        fprintf(stdout, "computing...\n");
      sleep (1); // inserting a stall to emulate some computation latency
      i++;
    }

  if (rank == 0) 
    fprintf(stdout, "[LMON APP] The program size is %d\n", size); 

  MPI_Finalize();
  return EXIT_SUCCESS;
}

