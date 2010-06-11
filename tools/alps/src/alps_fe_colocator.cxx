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
 *        Jun 10 2010 DHA: File created 
 */


#include "sdbg_std.hxx"

extern "C" {
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <unistd.h>
# include <signal.h>
# include "apInfo.h"
# include "libalps.h"
#if HAVE_LIBELF_H
# include LOCATION_OF_LIBELFHEADER
#else
# error libelf.h is required
#endif
extern char *alpsGetMyNid (int *nid);
}

#define _GNU_SOURCE
#include <getopt.h>
#include <vector>
#include <string>

#if BIT64
typedef Elf64_Shdr myElf_Shdr
typedef Elf64_Dyn myElf_Dyn
typedef Elf64_Addr myElf_Addr
const char *DFLT_LIB_PATH = "/lib64:/usr/lib64";
# define myelf_getshdr elf64_getshdr
#else
typedef Elf32_Shdr myElf_Shdr
typedef Elf32_Dyn myElf_Dyn
typedef Elf32_Addr myElf_Addr
const char *DFLT_LIB_PATH = "/lib:/usr/lib";
# define myelf_getshdr elf32_getshdr
#endif

struct option lopts[] = {
  {"apid", 1, 0, 'a'},
  {"bestarter", 1, 0, 'b'},
  {"daemon, 1, 0, 'p'},
  {"dargs", 1, 0, 'g'},
  {0, 0, 0, 0}
};

static void 
sighandler (int sig)
{
  if (sig == SIGINT)
    {
      std::cout << "SIGINT received" << std::endl; 
      return EXIT_SUCCESS;
    }

  return EXIT_FAILURE;
}

//
// Function that actually launches daemons using APLS tool helper 
// services 
//
//
static int
launch_daemons(std::vector<string> &dlibs, std::string &launchcmd)
{
  int rc = 0;
  int my_nid;
  char *err_str = NULL;

  if ( (err_str = alpsGetMyNid(&my_nid)) != NULL ); 
    {
      std::cerr << "alpsGetMyNid: " << err_str << std::endl;
      return -1;
    }

  int my_apid;
  if ( (my_apid = alps_get_apid(my_nid, aprun_pid)) != 0)
    {
      std::cerr << "alps_get_apid error" << std::endl;
      return -1;
    }

  appInfo_t appinfo;
  cmdDetail_t *cmdDetail;
  placeList_t *places;

  if (alps_get_appinfo(my_apid, &appinfo, &cmdDetail, &places) < 0) 
    {	
      std::cerr << "alps_get_appinfo error" << std::endl;
      return -1;
    }

  int pe0Node = places[0].nid;

  //
  // library Distribution
  //
  std::vector<std::string>::iterator iter;
  for(iter = dlibs.begin(); iter != dlibs.end(); ++iter)
    {
      if (access((*iter).c_str(), R_OK | X_OK) < 0)
        {
	  std::cerr << "Can't access " << (*iter) << std::endl;
	  rc = -1;
          continue;
        }

      err_str = alps_launch_tool_helper(my_apid,pe0Node,true,false,1,&((*iter).c_str()));
      if (err_str)
        {
	  std::cerr << "alps_launch_tool_helper error" << err_str << std::endl;
          rc = -1;
          continue;
	}
    }

  err_str = alps_launch_tool_helper(my_apid, pe0Node,true, true, 1, &(launchcmd.c_str()));
  if (err_str)
    {
      std::cerr << "alps_launch_tool_helper error" << err_str << std::endl;
      rc = -1;	
    }

  return rc;
}

//
// Function that fetches, resolves and fills dependent shared libraries
//
//
static int 
get_dep_DSO (const std::string &daemonpath, std::vector<std::string> &dlibs)
{
  Elf_Kind kind;
  Elf *arf;  
  Elf *elf_handler;
  int fd;

  if ( elf_version(EV_CURRENT) == EV_NONE ) 
    {
      std::cerr << "error in elf_version " << std::endl;
      return -1; 
    }

  if ( ( fd = open (daemonpath.c_str(), O_RDONLY)) == -1 ) 
    {
      std::cerr << "error openning " << daemonpath << std::endl;
      return -1; 
    }

  if ( ( arf = elf_begin (fd, ELF_C_READ, NULL)) == NULL ) 
    {
      std::cerr << "error elf_begin" << std::endl;
      return -1; 
    }

  if ( ( kind = elf_kind (arf)) != ELF_K_ELF )
    {
      std::cerr << "error elf_kind" << std::endl;
      return -1; 
    }

  if ( ( elf_handler = elf_begin (fd, ELF_C_READ, arf)) == NULL )
    {
      std::cerr << "error elf_begin" << std::endl;
      return -1; 
    }

  Elf_Scn *sect = NULL;
  myElf_Shdr *shdr = NULL;
  char *dynstrtab = NULL;
  char *sh_strtab = NULL;
  std::vector<myElf_Addr> dynstrtab_offsets_libs;
  std::vector<myElf_Addr> dynstrtab_offsets_rpaths;

  while ( (sect = elf_nextscn(elf_handler, sect)) != NULL ) 
    {
      if ( (shdr = myelf_getshdr (sect)) == NULL )
        continue;

      if ( shdr->sh_type == SHT_DYNAMIC ) 
        {
	  Elf_Data *dynsectdata = elf_getdata ( sect, dynsectdata );
          int ix;
          for (ix=0; ix < (dynsectdata->sh_size/dynsectdata->sh_entsize); ++ix) 
            {
              myElf_Dyn *dyndata = (myElf_Dyn *) (((myElf_Dyn *)(dynsectdata->d_buf)) + ix); 
              switch (dyndata->d_tag) 
                {
                case DT_NEEDED:
		  dynstrtab_offsets_libs.push_back(dyndata->d_un.d_ptr);
		  break;

		case DT_RPATH:
		  dynstrtab_offsets_rpaths.push_back(dyndata->d_un.d_ptr);
		  break;

		case DT_STRTAB:
		  dynstrtab = dyndata->d_un.d_ptr;
		  break;
	
		default:
		  break;
		}
	     }
         }
       else if ( shdr->sh_type == SHT_STRTAB && shdr->sh_addr == 0 )
         {
           Elf_Data *sectdata = elf_getdata ( sect, sectdata );
           sh_strtab = (char*) elf_data->d_buf;
         }
    }

  if (!sh_strtab) 
    {
      std::cerr << "section header string table is not found" << std::end;
      //
      // This is just outright wrong returning -1
      //
      return -1; 
    }

  //
  // searching for the interpreter
  //
  while ( (sect = elf_nextscn(elf_handler, sect)) != NULL ) 
    {
      if ( (shdr = myelf_getshdr (sect)) == NULL )
        continue;

      if ( shdr->sh_type == SHT_PROGBITS) 
        {
          if (std::string intern(".interp") == std::string(sh_strtab + shdr->sh_name)) 
            {
              Elf_Data *elfdata = elf_getdata(sect, elfdata);
              std::string interppath ((char *) elfdata->d_buf);
              if (access (interppath.c_str(), R_OK | X_OK) >= 0 )
                dlibs.push_back(interppath);
              break; 
            }            
         }
    }

  //
  // No interpreter? No dynamic string table? No DT_NEEDED entries?
  //
  if (dlibs.empty() || !dynstrtab || dynstrtab_offsets_libs.empty() )
    {
      std::cerr << "no dynamic library dependency?" << std::end;
      //
      // Maybe no libraries need to be shipped returning 0 
      //
      return 0; 
    }

  // 
  // Library search path order is RPATH, LD_LIBRARY_PATH and DFLT_LIB_PATH
  //

  //
  // So RPATH
  //
  std::vector<myElf_Addr>::iterator iter;
  for (iter = dynstrtab_offsets_rpaths.begin(); iter != dynstrtab_offsets_rpaths.end(); ++iter)
    {
       std::string apath((char *)dynstrtab + (*iter));  
       if (ld_lib_path.find(apath) == ld_lib_path.end())
         ld_lib_path.push_back(apath);
    }
  
  //
  // So LD_LIBRARY_PATH
  //
  char *llp, *tllp, *t, *tok; 
  if ( (llp = getenv("LD_LIBRARY_PATH")) != NULL) 
    {
      tllp = strdup(llp);
      t = tllp;
      while ( (tok = strtok(t, ":") != NULL))
       {
         std::string apath(tok);
         if (ld_lib_path.find(apath) == ld_lib_path.end())
           ld_lib_path.push_back(apath);
         t = NULL; 
       } 
      free(tllp); 
    }	

  //
  // Next DFLT_LIB_PATH
  //
  tllp = strdup(DLFT_LIB_PATH);  
  t = tllp;
  while ( (tok = strtok(t, ":") != NULL))
    {
      std::string apath(tok);
      if (ld_lib_path.find(apath) == ld_lib_path.end())
        ld_lib_path.push_back(apath);
      t = NULL;
    } 
  free(tllp); 

  //
  // Time to resolve libraries
  //
  std::vector<std::string>::iterator pathiter;
  for (iter = dynstrtab_offsets_libs.begin(); iter != dynstrtab_offsets_libs.end(); ++iter)
    {
      bool resolved = false;
      std::string alib(dynstrtab + (*iter));   
      if (alib.find('/') != alib.end())
        {
          //
          // Base name only
          // 
          for (pathiter = ld_lib_path.begin(); pathiter != ld_lib_path.end(); ++pathiter)
            {
              std::string pathtry = (*pathiter) + std::string("/") + alib;
              if (access(pathtry.c_str(), R_OK | X_OK) >= 0)
                {
                  dlibs.push_back(pathtry); 
                  resolved = true; 
                  break;
                }
            }
        }
      else
        {
          //
          // With dirname
          //
          if (access(alib.c_str(), R_OK | X_OK) >= 0)
            {
              dlibs.push_back(alib);
              resolved = true;
            } 
        }

      if (!resolved)
        {
          std::cerr << "Couldn't resolve " << alib << std::endl;
        }
    }

  if (elf_end(elf_handler) < 0 )
    {
      std::cerr << "Failed to close elf " << std::endl;
      return -1;
    }

  close(fd); 

  return 0; 
}

int
main (int argc, char *argv[])
{
  int c;
  int optix;
  int apid;
  std::string apidstr = ""; 
  std::string bestarter = "";
  std::string daemonpath = "";
  std::string dargs = "";
  std::vector<std::string> dlibs;

  while ( (c=getopt_long(argc, argv, NULL, lopts, &optix)) != -1)
    {
      switch (c)
        {
          case 'a':
            apid = atoi(optarg);
            apidstr = optarg;
            break;

          case 'b':
            bestarter = optarg;
            break;

          case 'p':
            daemonpath = optarg;
            break;

          case 'g':
            dargs = optarg;
            break;

          case '?':
          default:
	    std::cerr << "Unknown option: " << c << std::endl;
	    fprintf(stderr, "Unknown option: %c, continue parsing", c);
            break;
        }
    }

  if (access(bestarter.c_str(), R_OK | X_OK) < 0)
    {
      std::cerr << "Can't access " << bestarter << std::endl;
      return EXIT_FAILURE; 
    }
  if (access(daemonpath.c_str(), R_OK | X_OK) < 0)
    {
      std::cerr << "Can't access " << daemonpath << std::endl;
      return EXIT_FAILURE; 
    }

  if (get_dep_DSO (daemonpath, dlibs) < 0)
    {
      std::cerr << "Error returned from get_dep_dso" << std::endl;
      return EXIT_FAILURE; 
    }

  std::string launchcmd = bestarter 
    + std::string(" ") 
    + apidstr 
    + std::string(" ") 
    + daemonpath 
    + std::string(" ") 
    + dargs;

  if (launch_daemons(dlibs, launchcmd) != 0)
    {
      std::cerr << "Error occurred during launch_daemons " << std::endl;
      return EXIT_FAILURE; 
    }

  //
  // TODO: There is no monitoring service in the ALPS tool helper launcher  
  // determine the strategy to overcome this constraint
  //
  // For now, sleep with SIGINT signal handler installed
  // This way, launchmon engine won't enforce C.1
  signal(SIGINT, sighandler);
  while (1)
    sleep(3600);

  return EXIT_SUCCESS;
}

