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
 *        Jun 13 2008 DHA: Added GNU build system support.
 *        Mar 18 2008 DHA: Added BlueGene support. 
 *        Feb 09 2008 DHA: Added LLNS Copyright.
 *        Aug 06 2007 DHA: Created file.
 *          
 */

#include <lmon_api/common.h>
                                                                                                                                        
#if RM_BGL_MPIRUN
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


static void 
LMON_say_msg ( const char* m, const char* output, ... )
{
  va_list ap;
  char timelog[PATH_MAX];
  char log[PATH_MAX];
  const char* format = "%b %d %T";
  struct timeval tv;
  const char ei_str[] = "INFO";

  gettimeofday (&tv, NULL);
  strftime ( timelog, PATH_MAX, format, localtime(&(tv.tv_sec)));
  sprintf(log, "<%f> %s (%s): %s\n", (tv.tv_sec+(tv.tv_usec/100000.0)), m, "INFO", output);

  va_start(ap, output);
  vfprintf(stdout, log, ap);
  va_end(ap);
}


static void 
pass_its_neighbor(const int rank, const int size, int* buf)
{  
  MPI_Request request[2];
  MPI_Status status[2];

  MPI_Irecv((void*)buf, 1, MPI_INT, ((rank+size-1)%size), COMM_TAG, MPI_COMM_WORLD, &request[0]);
  MPI_Isend((void*)&rank, 1, MPI_INT, ((rank+1)%size), COMM_TAG, MPI_COMM_WORLD, &request[1]);
  MPI_Waitall(2, request, status);

  if (rank == 0) 
   {
     LMON_say_msg ( "APP", "size of this program is %d\n", size); 
   }
  MPI_Barrier(MPI_COMM_WORLD);
}


int 
main(int argc, char* argv[])
{
  int size; 
  int rank;  
  int buf;
  int i;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  pass_its_neighbor(rank, size, &buf);

  MPI_Finalize();
  return EXIT_SUCCESS;
}
