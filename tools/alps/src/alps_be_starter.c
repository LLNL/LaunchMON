/*
 * $Header: Exp $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2010, Lawrence Livermore National Security, LLC. Produced at 
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
 *        Jun 12 2010 DHA: Created based on UW's daemon_launcher. Credit to 
 *                         Ramya Olichandran 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

int main(int argc,char **argv)
{
  /*
   * This BE starter stub for ALPS must be invoked with 
   * alps_be_starter alpsJobId toolDaemonPath <args>
   *
   */
  char *lib_path;
  char reloc_ld_lib_path[PATH_MAX];
  char add_lib_path[PATH_MAX];
  char **daemon_argv;
  int my_apid=atoi(argv[1]);

  if (argc < 3) {
    fprintf(stderr, "Usage: alps_be_starter jobID daemonPath [daemonArgsOpts]\n");
    exit(1);
  }

  snprintf (reloc_ld_lib_path, PATH_MAX,
	   "/var/spool/alps/%d/toolhelper%d", 
           my_apid, my_apid);

  if ( access (reloc_ld_lib_path, R_OK) < 0 ) {
    fprintf(stderr, "access to %s failed\n", reloc_ld_lib_path);
    exit(1);
  }
  
  if ( lib_path = getenv("LD_LIBRARY_PATH") ) {
    sprintf(add_lib_path, "%s:%s", lib_path, reloc_ld_lib_path);
  }
  else {
    sprintf(add_lib_path, "%s", reloc_ld_lib_path);
  }  
  
  if ( setenv("LD_LIBRARY_PATH", (const char*) add_lib_path, 1) < 0 ) {
    fprintf(stderr, "setenv return neg");
    exit(1);
  } 

  daemon_argv = &argv[2];

  if ( execv(daemon_argv[0], daemon_argv) < 0 ) {
    fprintf(stderr, "execv fails to exec %s", daemon_argv[0]);
    exit(1);
  }

  return EXIT_SUCCESS;
}

