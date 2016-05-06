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
 *        May 05 2016 DHA: Applied @agontarek's Cray patch
 *        Nov 05 2010 DHA: Support for excluding existing system DSOs from 
 *                         being broadcast. Credit to Andrew Gontarek at Cray
 *                         for providing DSO list for CLE3.1 and CLE2.2
 *                         (ID: 3103796) 
 *        Nov 04 2010 DHA: Support for base executable relocation
 *        Jun 10 2010 DHA: File created 
 */


#include "sdbg_std.hxx"

extern "C" {
# include <cstdio>
# include <cstring>
# include <unistd.h>
# include <stdint.h>
# include <signal.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <time.h>
# include <sys/time.h>
# include <stdarg.h>
# include <limits.h>
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
# include <getopt.h>
# if HAVE_LIBELF_H
#  include LOCATION_OF_LIBELFHEADER
# else
#  error libelf.h is required
#endif
# include "alps/apInfo.h"
# include "alps/libalps.h"
extern char *alpsGetMyNid(int *nid);
}

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#if BIT64
typedef Elf64_Shdr myElf_Shdr;
typedef Elf64_Dyn myElf_Dyn;
typedef Elf64_Addr myElf_Addr;
const char *DFLT_LIB_PATH = "/lib64:/usr/lib64";
# define myelf_getshdr elf64_getshdr
#else
typedef Elf32_Shdr myElf_Shdr;
typedef Elf32_Dyn myElf_Dyn;
typedef Elf32_Addr myElf_Addr;
const char *DFLT_LIB_PATH = "/lib:/usr/lib";
# define myelf_getshdr elf32_getshdr
#endif

#define ALPS_STRING_MAX 1024

struct option lopts[] = {
  {"apid", 1, 0, 'a'},
  {"be_starter", 1, 0, 'b'},
  {"daemon", 1, 0, 'p'},
  {0, 0, 0, 0}
};

enum CLE_version_e {
  cle_ver_2_2,
  cle_ver_3_1,
  cle_ver_unknown
}; 

const char XTRELEASE_FILE_PATH[] = "/etc/opt/cray/release/xtrelease";
const char DSO_CONFIG_FILE[] = "CLE-dso.conf";
const char MSGPREFIX[] = "ALPS FE COLOCATOR";

static void
ALPS_say_msg ( const char* m, bool is_err, const char* output, ... )
{
  va_list ap;
  char timelog[PATH_MAX];
  char log[PATH_MAX];
  const char* format = "%b %d %T";
  time_t t;
  const char *ei_str = is_err ? "ERROR" : "INFO";

  time(&t);
  strftime ( timelog, PATH_MAX, format, localtime(&t));
  snprintf(log, PATH_MAX, "<%s> %s (%s): %s\n", timelog, m, "INFO", output);

  va_start(ap, output);
  vfprintf(stdout, log, ap);
  va_end(ap);
}


static void 
sighandler (int sig)
{
  if (sig == SIGINT)
    {
      ALPS_say_msg(MSGPREFIX, false, "SIGINT received, exiting..." );
      exit(EXIT_SUCCESS);
    }

  exit(EXIT_FAILURE);
}


//
// get_CLE_ver return an enumerator indicating
// what DSO list file you sould use at run-time 
// As other CLE lists are added, the logic of 
// detecting CLE versions need to be added here.
//
static CLE_version_e 
get_CLE_ver()
{
  try 
    {
      CLE_version_e rc = cle_ver_unknown;
      std::ifstream xtrelease_file(XTRELEASE_FILE_PATH);

      if ( !xtrelease_file )
        {
          rc = cle_ver_2_2;
        }
      else 
        {
          std::ostringstream oss;
          oss << xtrelease_file.rdbuf();  
          std::string::size_type pos = oss.str().find_first_of("=", 0);
          const std::string vstr("3.1.");
          if (oss.str().compare(pos+1, 4, vstr) == 0)
            {
              rc = cle_ver_3_1;
            }
        }

      return rc;
    }
    catch (std::out_of_range e) {
      return cle_ver_unknown;
    }
}


static 
void
build_DSO_mask (std::map<std::string, int> &dontShipIt, 
                std::ifstream &listFstream)
{
  char apath[PATH_MAX];

  //note new line is not stored in apath
  while (!listFstream.getline(apath, PATH_MAX).eof()) 
    {
      std::map<std::string, int>::iterator iter; 
      iter = dontShipIt.find(apath);
      if (iter == dontShipIt.end())      
        {
          dontShipIt[apath] = 1;  
        }
      else
        {
          iter->second++;
        } 
    }
}  


//
// Function that actually launches daemons using APLS tool helper 
// services 
//
//
static int
launch_daemons(int apid, 
  std::vector<std::string> &dlibs, std::string &bestarter,
  std::string &daemonpath, std::string &dargs)
{
  int rc = 0;
  int my_nid;
  const char *err_str = NULL;

  if ( (err_str = alpsGetMyNid(&my_nid)) )
    {
      ALPS_say_msg(MSGPREFIX, true, "error in alpsGetMyNid %s", err_str);
      return -1;
    }

  appInfo_t appinfo;
  cmdDetail_t *cmdDetail;
  placeList_t *places;

  int my_apid = alps_get_apid(my_nid, apid);
  if (!my_apid)
  {
     ALPS_say_msg(MSGPREFIX, true, "error in alps_get_apid");
     return -1;
  }

  if (alps_get_appinfo(my_apid, &appinfo, &cmdDetail, &places) < 0) 
    {	
      ALPS_say_msg(MSGPREFIX, true, "error in alps_get_appinfo");
      return -1;
    }

  int pe0Node = places[0].nid;

  //
  // building dontshipit map 
  //
  std::map<std::string, int> dontShipIt;
  std::string dsoListPath;
  std::string dsoListDir;
  char *pref = getenv("LMON_PREFIX");
  if (pref)
    {
      dsoListDir = std::string(pref) + std::string("/etc/");
      dsoListPath = dsoListDir + DSO_CONFIG_FILE;
    }
  else
    {
      dsoListDir = std::string("./");
      dsoListPath = dsoListDir + DSO_CONFIG_FILE;
      ALPS_say_msg(MSGPREFIX, 
                   false, 
                   "LMON_PREFIX envVar not found");
    }

  std::ifstream dsofstream(dsoListPath.c_str());
  if (!dsofstream)
    {
      ALPS_say_msg(MSGPREFIX, 
                   false, 
                   "%s not found... selective DSO bcast will not work", 
                   dsoListPath.c_str());
    }
  else
    {
      char aLine[PATH_MAX];
      std::map<std::string, std::string> confMap;
      while (!dsofstream.getline(aLine, PATH_MAX).eof())
        {
          if ((aLine[0] != '-') && (aLine[1] != 'i'))
            {
              continue;
            } 

          std::string aLineStr(aLine);
          std::string::size_type pos = aLineStr.find_first_of(" \t", 0); 
          pos = aLineStr.find_first_not_of(" \t", pos);
          std::string::size_type lastpos = aLineStr.find_first_of(" \t", pos);
          std::string verstr = aLineStr.substr(pos, lastpos-pos);
          pos = aLineStr.find_first_not_of(" \t", lastpos);
          lastpos = aLineStr.find_first_of(" \t", pos);
          std::string filename;

          if (lastpos == std::string::npos)
            {
              filename = aLineStr.substr(pos);
            }
          else
            {
              filename = aLineStr.substr(pos, lastpos-pos);
            }

          confMap[verstr] = filename; 
        }

      CLE_version_e ver = get_CLE_ver();
      switch (ver)
        {
        case cle_ver_2_2:
          {
            std::map<std::string, std::string>::const_iterator citer 
              = confMap.find(std::string("2.2"));
            if (citer != confMap.end())
              { 
                std::string confPath = dsoListDir + citer->second;
                std::ifstream confFile(confPath.c_str());
                if (!confFile)
                  {
                    ALPS_say_msg(MSGPREFIX,
                      true,
                      "can't open %s", 
                      confPath.c_str());
                  }
                else
                  {
                    build_DSO_mask(dontShipIt,confFile);
                  }
              }
             break;
          }
        case cle_ver_3_1:
          {
            std::map<std::string, std::string>::const_iterator citer 
              = confMap.find(std::string("3.1"));
            if (citer != confMap.end())
              { 
                std::string confPath = dsoListDir + citer->second;
                std::ifstream confFile(confPath.c_str());
                if (!confFile)
                  {
                    ALPS_say_msg(MSGPREFIX,
                      true,
                      "can't open %s", 
                      confPath.c_str());
                  }
                else
                  {
                    build_DSO_mask(dontShipIt,confFile);
                  }
              }
             break;
          }
        case cle_ver_unknown:
          {
            //
	    // In this case, DSOs from combined list are used 
            //
            std::map<std::string, std::string>::const_iterator citer;
            for (citer = confMap.begin(); citer != confMap.end(); ++citer)
              {
                std::string confPath = dsoListDir + citer->second;
                std::ifstream confFile(confPath.c_str());
                if (!confFile)
                  {
                    ALPS_say_msg(MSGPREFIX,
                      true,
                      "can't open %s", 
                      confPath.c_str());
                  }
                else
                  {
                    build_DSO_mask(dontShipIt,confFile);
                  } 
              }
            break;
          }
        default:
          break;
        }
    }

  //
  // library Distribution
  //
  std::vector<std::string>::iterator iter;
  for(iter = dlibs.begin(); iter != dlibs.end(); ++iter)
    {
      if (access((*iter).c_str(), R_OK | X_OK) < 0)
        {
          ALPS_say_msg(MSGPREFIX, true, "Can't access %s", (*iter).c_str());
	  rc = -1;
          // we will still try to launch daemons even if a library trasfer fails
          continue;
        }

      if (dontShipIt.find((*iter)) != dontShipIt.end())
        {
          // this library already exists in CN and so no need to broadcast
          continue;
        }

      char *libname = strdup((*iter).c_str());

      //
      // Transfering a shared library here
      // 
      err_str = alps_launch_tool_helper(my_apid,pe0Node,true,false,1,&libname);
      if (err_str)
        {
          ALPS_say_msg(MSGPREFIX, true, "error in alps_launch_tool_helper: %s", err_str);
          rc = -1;
          if (libname)
	    free(libname);

          // we will still try to launch daemons even if a library trasfer fails
          continue;
	}
      if (libname)
        free(libname);
    }

  char lcmdtmp[ALPS_STRING_MAX];
  char *baseexec = strdup(daemonpath.c_str());
  err_str = alps_launch_tool_helper(my_apid,pe0Node,true,false,1,&baseexec);
  free(baseexec);
  if (err_str)
    {
      ALPS_say_msg(MSGPREFIX, true, "error in alps_launch_tool_helper: %s", err_str);
      //
      // If the executable relocation failed, we should use the original
      // be daemon path
      //
      snprintf(lcmdtmp, ALPS_STRING_MAX, "%s %d %s %s", 
      bestarter.c_str(), my_apid, daemonpath.c_str(), dargs.c_str());  
    }
  else
    {
      //
      // Otherwise, the relocated daemon path is passed to ALPS tool helper
      //  
      char reloc_exec[PATH_MAX];
      char *bncp = strdup(daemonpath.c_str());
      char *bn = basename(bncp);
      
      snprintf (reloc_exec, PATH_MAX,
         "/var/spool/alps/%d/toolhelper%d/%s",
         my_apid, my_apid, bn);

      snprintf(lcmdtmp, ALPS_STRING_MAX, "%s %d %s %s", 
      bestarter.c_str(), my_apid, reloc_exec, dargs.c_str());  
      free(bncp);
    }

  char *lcmd = lcmdtmp;
  //
  // Launching backend daemons here 
  // 
  ALPS_say_msg(MSGPREFIX, false, "%s", lcmd );
  err_str = alps_launch_tool_helper(my_apid, pe0Node,true, true, 1, &lcmd);
  if (err_str)
    {
      ALPS_say_msg(MSGPREFIX, true, "error in alps_launch_tool_helper: %s", err_str);
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
      ALPS_say_msg(MSGPREFIX, true, "error in elf_version");
      return -1; 
    }

  if ( ( fd = open (daemonpath.c_str(), O_RDONLY)) == -1 ) 
    {
      ALPS_say_msg(MSGPREFIX, true, "error in openning %s", daemonpath.c_str());
      return -1; 
    }

  if ( ( arf = elf_begin (fd, ELF_C_READ, NULL)) == NULL ) 
    {
      ALPS_say_msg(MSGPREFIX, true, "error in elf_begin");
      return -1; 
    }

  if ( ( kind = elf_kind (arf)) != ELF_K_ELF )
    {
      ALPS_say_msg(MSGPREFIX, true, "error in elf_kind");
      return -1; 
    }

  if ( ( elf_handler = elf_begin (fd, ELF_C_READ, arf)) == NULL )
    {
      ALPS_say_msg(MSGPREFIX, true, "error in elf_begin");
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
          int ix;
          Elf_Data *sectdata;
	  if (!(sectdata = elf_getdata ( sect, NULL)) )
    	    {
              ALPS_say_msg(MSGPREFIX, true, "elf_data returned null");
              return -1; 
            }

          for (ix=0; ix < (shdr->sh_size/shdr->sh_entsize); ++ix) 
            {
              myElf_Dyn *dyndata = (myElf_Dyn *) sectdata->d_buf;
              dyndata += ix;
              switch (dyndata->d_tag) 
                {
                case DT_NEEDED:
		  dynstrtab_offsets_libs.push_back(dyndata->d_un.d_ptr);
		  break;

		case DT_RPATH:
		  dynstrtab_offsets_rpaths.push_back(dyndata->d_un.d_ptr);
		  break;

		case DT_STRTAB:
		  dynstrtab = (char *) dyndata->d_un.d_ptr;
		  break;
	
		default:
		  break;
		}
	     }
         }
       else if ( shdr->sh_type == SHT_STRTAB && shdr->sh_flags == 0)
         {
	   Elf_Data *strsectd;
	   if (!(strsectd = elf_getdata (sect, NULL)))
             {
               ALPS_say_msg(MSGPREFIX, true, "elf_data returned null");
               return -1;
             }

	   std::string shname((char*)strsectd->d_buf+shdr->sh_name); 
	   if (std::string(".shstrtab") == shname)
            sh_strtab = (char*) strsectd->d_buf;
         }
    }

  if (!sh_strtab) 
    {
      ALPS_say_msg(MSGPREFIX, true, "section header string table is not found");
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
          if (std::string(".interp") == std::string(sh_strtab + shdr->sh_name)) 
            {
              Elf_Data *elfdata;
	      if (!(elfdata = elf_getdata(sect, NULL)))
                {
                  ALPS_say_msg(MSGPREFIX, true, "elf_data returned null");
                  return -1;
                }

              std::string interppath ((char *) elfdata->d_buf);
              if (access (interppath.c_str(), R_OK | X_OK) >= 0 )
                dlibs.push_back(interppath);
            }            
         }
       else if (shdr->sh_type == SHT_STRTAB && shdr->sh_flags == SHF_ALLOC)
         {
	   //
	   // Looking for the dynamic symbol table that is loaded into
	   // memory so sh_addr != 0 check
           //
	   Elf_Data *strsectd;
	   if (!(strsectd = elf_getdata (sect, NULL)))
             {
               ALPS_say_msg(MSGPREFIX, true, "elf_data returned null");
               return -1;
             }

	   std::string shname((char*)sh_strtab+shdr->sh_name); 
	   if (std::string(".dynstr") == shname)
            {
              dynstrtab = (char*) strsectd->d_buf;
            }
         }
    }

  //
  // No interpreter? No dynamic string table? No DT_NEEDED entries?
  //
  if ( !dynstrtab || dynstrtab_offsets_libs.empty() )
    {
      ALPS_say_msg(MSGPREFIX, true, "no dynamic library dependency?");
      //
      // Maybe no libraries need to be shipped returning 0 
      //
      return 0; 
    }

  // 
  // Library search path order is RPATH, LD_LIBRARY_PATH and DFLT_LIB_PATH
  //
  std::vector<std::string> ld_lib_path;

  //
  // So RPATH
  //
  char *llp, *tllp, *t, *tok; 
  std::vector<myElf_Addr>::iterator iter;
  for (iter = dynstrtab_offsets_rpaths.begin(); iter != dynstrtab_offsets_rpaths.end(); ++iter)
    {
      tllp = strdup((char *)dynstrtab + (*iter)); 
      t = tllp;
      while ( (tok = strtok(t, ":")) != NULL )
        {
          std::string apath((char *)tok);  
          if (std::find(ld_lib_path.begin(), ld_lib_path.end(), apath) == ld_lib_path.end())
            ld_lib_path.push_back(apath);
	  t = NULL;
        }
      free(tllp); 
    }
  
  //
  // So LD_LIBRARY_PATH
  //
  if ( (llp = getenv("LD_LIBRARY_PATH")) != NULL) 
    {
      tllp = strdup(llp);
      t = tllp;
      while ( (tok = strtok(t, ":")) != NULL )
       {
         std::string apath(tok);
         if (std::find(ld_lib_path.begin(), ld_lib_path.end(), apath) == ld_lib_path.end())
           ld_lib_path.push_back(apath);
         t = NULL; 
       } 
      free(tllp); 
    }	

  //
  // Next DFLT_LIB_PATH
  //
  tllp = strdup(DFLT_LIB_PATH);  
  t = tllp;
  while ( (tok = strtok(t, ":")) != NULL)
    {
      std::string apath(tok);
      if (std::find(ld_lib_path.begin(), ld_lib_path.end(), apath) == ld_lib_path.end())
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
      if (alib.find('/') == alib.npos)
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
          ALPS_say_msg(MSGPREFIX, true, "Couldn't resolve %s", alib.c_str());
        }
    }

  if (elf_end(elf_handler) < 0 )
    {
      ALPS_say_msg(MSGPREFIX, true, "error in close");
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
  char optstring[] = "";
   
  opterr = 0; 
  while ( (c=getopt_long(argc, argv, optstring, lopts, &optix)) != -1)
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

          case '?':
          default:
            dargs += std::string(argv[optind-1]) + std::string(" "); 
            break;
        }
    }

  int ix;
  for (ix=optind; ix < argc; ix++)
    {
      dargs = argv[ix] + std::string(" ") + dargs;
    } 

  if (access(bestarter.c_str(), R_OK | X_OK) < 0)
    {
      ALPS_say_msg(MSGPREFIX, true, "Can't access %s", bestarter.c_str());
      return EXIT_FAILURE; 
    }

  if (access(daemonpath.c_str(), R_OK | X_OK) < 0)
    {
      ALPS_say_msg(MSGPREFIX, true, "Can't access %s", daemonpath.c_str());
      return EXIT_FAILURE; 
    }

  if (get_dep_DSO (daemonpath, dlibs) < 0)
    {
      ALPS_say_msg(MSGPREFIX, true, "Error in get_dep_DSO");
      return EXIT_FAILURE; 
    }

  if (launch_daemons(apid, dlibs, bestarter, daemonpath, dargs) != 0)
    {
      ALPS_say_msg(MSGPREFIX, true, "Error in launch_daemons");
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

