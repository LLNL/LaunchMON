/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_opt.cxx,v 1.9.2.4 2008/03/06 00:13:57 dahn Exp $
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
 *
 *  Update Log:
 *        Mar 11 2009 DHA: Added 2009 to copyright
 *        Mar 17 2008 DHA: Added PMGR Collective support.
 *        Feb 09 2008 DHA: Added LLNS Copyright.
 *        Dec 05 2007 DHA: Fixed some unitialized option field, which 
 *                         crashed the LaunchMON under the control
 *                         of TotalView's memory debugging, specifically
 *                         memory painting. 
 *        Dec 05 2007 DHA: Added mpirun model checker support
 *        Jul 04 2006 DHA: Added self tracing support
 *        Jul 03 2006 DHA: Added a logic to catch an invalid pid 
 *                         for attaching case.
 *        Jun 08 2006 DHA: Added attach-to-a-running-job support
 *        Jun 06 2006 DHA: File created
 */ 

#include <lmon_api/common.h>
#include "sdbg_opt.hxx"
#include "sdbg_self_trace.hxx"

const std::string software_name 
   = PACKAGE_NAME;
const std::string version 
   = PACKAGE_VERSION;
const std::string copyright 
   = "Copyright (C) 2008-2009, Lawrence Livermore National Security, LLC.";
const std::string produced 
   = "Produced at Lawrence Livermore National Laboratory.";
const std::string right 
   = "LLNL-CODE-409469 All rights reserved.";
const std::string LAUNCHMON_COPYRIGHT 
   = software_name + " " +  version + "\n" + copyright + "\n" + produced + "\n" + right + "\n";

//!
/*!  opt_args_t constructor
      
    
*/
opts_args_t::opts_args_t ()
{
  my_opt = new opt_struct_t();
  my_opt->verbose = 0;
  my_opt->modelchecker=0;
  my_opt->attach = false;
  my_opt->remote = false;
  my_opt->remaining = NULL;
  my_opt->tool_daemon = "";
  my_opt->tool_daemon_opts = "";
  my_opt->remote_info = "";
#if PMGR_BASED
  my_opt->pmgr_info = "";
  my_opt->pmgr_sec_info = "";
#endif
  my_opt->debugtarget = "";
  my_opt->launchstring = "";
  my_opt->copyright = LAUNCHMON_COPYRIGHT;
  my_opt->launcher_pid = -1;

  MODULENAME = self_trace_t::opt_module_trace.module_name;
}


//!
/*!  opt_args_t copy constructor
     the only member it doesn't do the deep copy is "remaining"
    
*/
opts_args_t::opts_args_t ( const opts_args_t& o )
{
  if (o.my_opt != NULL) {
    my_opt = new opt_struct_t();
    my_opt->verbose = o.my_opt->verbose;
    my_opt->modelchecker = o.my_opt->modelchecker;
    my_opt->attach = o.my_opt->attach;
    my_opt->remote = o.my_opt->remote;
    my_opt->remaining = o.my_opt->remaining;
    my_opt->tool_daemon = o.my_opt->tool_daemon;
    my_opt->tool_daemon_opts = o.my_opt->tool_daemon_opts;
    my_opt->remote_info = o.my_opt->remote_info;
#if PMGR_BASED
    my_opt->pmgr_info = o.my_opt->pmgr_info;
    my_opt->pmgr_sec_info = o.my_opt->pmgr_sec_info;
#endif
    my_opt->debugtarget = o.my_opt->debugtarget;
    my_opt->launchstring = o.my_opt->launchstring;
    my_opt->copyright = o.my_opt->copyright;
    my_opt->launcher_pid = o.my_opt->launcher_pid;

    MODULENAME = o.MODULENAME;
  }  
}


//!
/*!  opt_args_t destructor
      
    
*/
opts_args_t::~opts_args_t()
{
  if(my_opt) {
    delete my_opt;
  }
  my_opt = NULL;
}


//!
/*!  opt_args_t::process_args
      
    
*/
bool 
opts_args_t::process_args ( int* argc, char*** argv )
{ 
  using namespace std;

  bool fin_parsing = false;
  bool rv = true;
  char* debugtarget = NULL;
  char** nargv = *argv;
  char* tracingmodule;
  char c;
  string modulename;
  string bspth;
  int level;
  int i = 1;
  self_trace_verbosity ver = quiet;

  if ((*argc) < 2) 
    {
      print_usage();
      return false;
    }

  my_opt->remote = false;
  my_opt->modelchecker = false;

  for (i=1; (i < (*argc)) && !fin_parsing ; i++) 
    {    
      if (nargv[i][0] == '-') 
	{
	  c = nargv[i][1];
	  if ( c == '-') 
	    {
	      if ( string(&nargv[i][2]) == string("verbose")) 
		c = 'v';		
	      else if ( string(&nargv[i][2]) == string("help")) 		
		c = 'h';	    
	      else if ( string(&nargv[i][2]) == string("daemonpath")) 		
		c = 'd';	       
	      else if ( string(&nargv[i][2]) == string("selftrace")) 		
		c = 'x';	       
	      else if ( string(&nargv[i][2]) == string("daemonopts")) 		
		c = 't';		
	      else if ( string(&nargv[i][2]) == string("pid")) 		
		c = 'p';		
	      else if ( string(&nargv[i][2]) == string("traceout"))		
		c = 'o';
	      else if ( string(&nargv[i][2]) == string("remote"))
		c = 'r';
#if PMGR_BASED
	      else if ( string(&nargv[i][2]) == string("pmgr"))
	        c = 'm';  	
	      else if ( string(&nargv[i][2]) == string("pmgrsec"))	
	        c = 's';
#endif
	    }
            
	  switch (c) 
	    {
      
	    case 'a':  
	      if(debugtarget)
		nargv[i] = debugtarget;	      
	      else
		exit(1);  

	      my_opt->remaining = &(nargv[i]);
	      fin_parsing = true;
	      break;
	      
	    case 'o':
	      self_trace_t::tracefptr = fopen (nargv[i+1], "w+");	     
	      assert ( self_trace_t::tracefptr != NULL);
	      i++;
	      break;

	    case 'v':
	      my_opt->verbose = atoi(nargv[i+1]);
	      
	      if (my_opt->verbose == 0)
		ver = quiet;
	      else if (my_opt->verbose == 1 )
		ver = level1;       
	      else if (my_opt->verbose == 2 )
		ver = level2;	   
	      else 
		my_opt->verbose = 1;	  
	
	      self_trace_t::launchmon_module_trace.verbosity_level = ver;
	      self_trace_t::tracer_module_trace.verbosity_level = ver;
	      self_trace_t::symtab_module_trace.verbosity_level = ver;
	      self_trace_t::thread_tracer_module_trace.verbosity_level = ver;
	      self_trace_t::event_module_trace.verbosity_level = ver;
	      self_trace_t::driver_module_trace.verbosity_level = ver;
	      self_trace_t::machine_module_trace.verbosity_level = ver;
	      self_trace_t::opt_module_trace.verbosity_level = ver;
	      
	      i++;
	      break;
	      
	    case 'h':
	      print_usage();
	      break;
       
	    case 'd':	     	
	      my_opt->tool_daemon =  nargv[i+1];
	      bspth = nargv[i+1];
	      if (!check_path(bspth, my_opt->tool_daemon))
  	        exit(1);
	   
	      i++;
	      break;
	
	    case 't':
	      my_opt->tool_daemon_opts = nargv[i+1];
	      i++;
	      break;

	    case 'p':
	      my_opt->launcher_pid = (pid_t)atoi(nargv[i+1]);
	      my_opt->attach = true;	   
	      i++;
	      break;
	      
	    case 'r':
	      my_opt->remote = true;
	      my_opt->remote_info = nargv[i+1]; // it should have hostname:port 
	      i++;
	      break;
#if PMGR_BASED

 	    case 'm':
	      my_opt->pmgr_info = nargv[i+1]; // should also be hostname:port	
	      i++;
	      break;	

 	    case 's':
	      my_opt->pmgr_sec_info = nargv[i+1]; 	
	      i++;
	      break;	

#endif
	    case 'x':
	      //
	      // this is a hidden option for self-tracing
	      //
	      tracingmodule = strdup(nargv[i+1]);
	      modulename = strtok(tracingmodule, ":");
	      level = atoi(strtok(NULL, ":"));;

	      if (level == 0)
		ver = quiet;	      
	      else if (level == 1 )
		ver = level1;       
	      else if (level == 2 )
		ver = level2;	     
	      else if (level == 3 )
		ver = level3;	   
	      else if (level == 4 )
		ver = level4;	   
	      else 
		ver = level1;

	      if ( modulename== self_trace_t::launchmon_module_trace.module_symbol ) 
		self_trace_t::launchmon_module_trace.verbosity_level = ver;       
	      else if ( modulename 
			== self_trace_t::tracer_module_trace.module_symbol )
		self_trace_t::tracer_module_trace.verbosity_level = ver;	      
	      else if ( modulename
			== self_trace_t::symtab_module_trace.module_symbol )
		self_trace_t::symtab_module_trace.verbosity_level = ver;       
	      else if ( modulename 
			== self_trace_t::thread_tracer_module_trace.module_symbol )
		self_trace_t::thread_tracer_module_trace.verbosity_level = ver;       
	      else if ( modulename 
			== self_trace_t::machine_module_trace.module_symbol )
		self_trace_t::machine_module_trace.verbosity_level = ver;
	      else if ( modulename 
			== self_trace_t::event_module_trace.module_symbol )
		self_trace_t::event_module_trace.verbosity_level = ver;
	      else if ( modulename 
			== self_trace_t::driver_module_trace.module_symbol )
		self_trace_t::driver_module_trace.verbosity_level = ver;
	      else if ( modulename 
			== self_trace_t::opt_module_trace.module_symbol )
		self_trace_t::opt_module_trace.verbosity_level = ver;
	      else if ( modulename 
			== self_trace_t::sighandler_module_trace.module_symbol )
		self_trace_t::sighandler_module_trace.verbosity_level = ver;

	      i++;
	      break;
	      
	    default:
	      print_usage();
	      rv = false;
	      break;  
	    }
	}
      else 
	{
	  debugtarget = nargv[i];
	  my_opt->debugtarget = nargv[i];
	  bspth = debugtarget;
	  if (!check_path(bspth, my_opt->debugtarget))
	    exit(1);     
	}
    }

  if ( !option_sanity_check() )
    {
      if ( my_opt->remote && (my_opt->verbose == 0 ))
        exit (1);

      print_usage();
    }

  if ( !construct_launch_string() )
    {
      if ( my_opt->remote && (my_opt->verbose == 0 ))
        exit (1);
    
      print_usage();
    }

  if ( !(my_opt->remote && (my_opt->verbose == 0 )) )
    print_copyright();
  
  return rv;
}


//!  opts_args_t::construct_launchstring()
/*!        
    
*/
bool 
opts_args_t::construct_launch_string ()
{
  using namespace std;

  string dt;
  bool rc = true;;
  char tar[PATH_MAX];
  char path[PATH_MAX];  
  int cnt;

  if (!my_opt) 
    {
      {
	self_trace_t::trace ( LEVELCHK(quiet), 
	  MODULENAME,
	  1,
	  "options and arguments haven't been parsed ");
      }

      return false;
    }
  
  if ( my_opt->attach ) 
    {    
      sprintf(path, "/proc/%d/exe", my_opt->launcher_pid);   
      cnt = readlink(path, tar, PATH_MAX);
      if ( cnt < 0 )
	{
	  {
	    self_trace_t::trace ( LEVELCHK(quiet), 
	    MODULENAME,
	    1,
	    "pid[%d] isn't valid.",
	    my_opt->launcher_pid);
	  }	  
	  return false;
	}
      tar[cnt] = '\0';
      my_opt->debugtarget = tar;
    }
  
  char *bnbuf = strdup(my_opt->debugtarget.c_str()); 
  dt = basename(bnbuf);
  if ( dt == string("srun") || dt == string("lt-srun")) 
    {
      my_opt->launchstring 
	= my_opt->debugtarget + string(" --input=none --jobid=%d --nodes=%d --ntasks=%d ")
	+ my_opt->tool_daemon + string(" ")
	+ my_opt->tool_daemon_opts
#if PMGR_BASED
	+ string (" --pmgrsize=%d --pmgrip=%s --pmgrport=%s --pmgrjobid=%d --pmgrsharedsec=%s --pmgrsecchk=%s --pmgrlazyrank=1 --pmgrlazysize=1") 
#endif	
	;

      if ( getenv("LMON_DEBUG_BES") != NULL)
	{
	  my_opt->launchstring = string("totalview ") + my_opt->debugtarget +string(" -a ") 
	    + string(" --input=none --jobid=%d --nodes=%d --ntasks=%d ")	   
	    + my_opt->tool_daemon + string(" ")
	    + my_opt->tool_daemon_opts
#if PMGR_BASED
            + string (" --pmgrsize=%d --pmgrip=%s --pmgrport=%s --pmgrjobid=%d --pmgrsharedsec=%s --pmgrsecchk=%s --pmgrlazyrank=1 --pmgrlazysize=1")
#endif
            ;
	}

      //cout << my_opt->launchstring << endl;
      
      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,
	  0,
	  "constructed launchstring: %s",
	  my_opt->launchstring.c_str() );
      }	  
    }
  else if ( (dt == string ("mpirun")) 
            || (dt == string ("mpirun32"))
            || (dt == string("mpirun64")))
    {
      my_opt->launchstring 
        = my_opt->tool_daemon_opts  
#if PMGR_BASED
        + string (" --pmgrip=%s --pmgrport=%s --pmgrjobid=%d --pmgrsharedsec=%s --pmgrsecchk=%s --pmgrlazyrank=1 --pmgrlazysize=1")
#endif     
      ;
    } 
  else if ( (dt == string ("LE_model_checker.debug") )
	    || (dt == string ("LE_model_checker")) )
    {
      // 
      // mpirun model checker support
      //

      my_opt->launchstring 
        = my_opt->tool_daemon_opts  
#if PMGR_BASED
        + string (" --pmgrip=%s --pmgrport=%s --pmgrlazyrank=1 --pmgrlazysize=1")
#endif     
      ;
      my_opt->modelchecker = true;

    }
  else 
    {    
      {
	self_trace_t::trace ( LEVELCHK(quiet), 
			      MODULENAME,
			      1,
			      "unknown launcher: %s ",
			      dt.c_str());
      }
      
      rc = false;    
    }

  //
  // NOTE: are there any platforms that free on bnbuf is dangerous?
  //
  free (bnbuf);

  return rc;
}


//!  opts_args_t::print_usage()
/*!  
     prints the usage
*/
void 
opts_args_t::print_usage()
{
  using namespace std;

  cerr << "Usage: " << endl;
  cerr << "  launchmon <options> srun -a <srun options>" << endl; 
  cerr << "      or " << endl;
  cerr << "  launchmon <options> -p srun_pid " << endl << endl;
  cerr << "options:" << endl;
  cerr << "\t\t-v, --verbose 0~2           sets the verbosity level." << endl;
  cerr << "\t\t-h, --help                  prints this message." << endl;
  cerr << "\t\t-r, --remote ip:port        invokes launchmon in API mode." << endl;
#if PMGR_BASED
  cerr << "\t\t-r, --pmgr ip:port          ip/port pair FEN pmgr created." << endl;
#endif
  cerr << "\t\t-d, --daemonpath path       sets the tool daemon path." << endl;
  cerr << "\t\t-t, --daemonopts \"opts\"     sets the tool daemon option set." << endl;
  cerr << "\t\t-p, --pid pid               attaches to pid, the pid of running parallel "
       << "job launcher process." << endl; 
  cerr << "\t\t-a  args                    pass all subsequent arguments to the parallel job launcher. "
       << endl << endl;
  cerr << "example: launchmon -v 1 -d tooldaemon -t \"-k -r\" "
       << "srun -a -n 4 -N 2 -ppdebug ./code"
       << endl;
  cerr << "         launchmon --daemonpath tooldaemon -pid 1234" 
       << endl << endl;

  exit(-1); 
}


//!  opts_args_t::print_copyright()
/*!  
     print the copyright
*/
void 
opts_args_t::print_copyright()
{
  using namespace std;
  {
     self_trace_t::trace ( LEVELCHK(level1),
        MODULENAME,
        0,
        my_opt->copyright.c_str());
  }
  std::cout << my_opt->copyright << std::endl;
}


//!  opts_args_t::option_sanity_check()
/*!  
     sanity check for options and argument
*/
bool
opts_args_t::option_sanity_check()
{ 

  if (!my_opt) 
    {
      {
	self_trace_t::trace ( LEVELCHK(quiet), 
	  MODULENAME,
	  1,
	  "options and arguments haven't been parsed ");
      }

      return false;
    }

  if ( my_opt->tool_daemon == std::string(""))
    {
      {
	self_trace_t::trace ( LEVELCHK(quiet), 
	  MODULENAME,
	  1,
	  "a tool daemon path must be specified with -d option");
      }

      return false;
    }

  return true;
}


//!  opts_args_t::check_path
/*!  
          
*/
bool 
opts_args_t::check_path ( std::string& base, std::string& path )
{
  using namespace std;

  char* pth = NULL;
  char* mypath = NULL;
  char* mypathstart = NULL;
  bool rc = true;
  struct stat pathchk;

  mypath = strdup(getenv("PATH"));
  mypathstart = mypath;

  while ( stat( path.c_str(), &pathchk ) != 0 ) 
    {		  
      pth = strtok(mypath, ":");
      if ( (base[0] != '/') && pth != NULL ) 
	{
	  string dt = string(pth) + string("/") +  string(base);
	  path = dt;
	}
    else 
      {
	{
	  self_trace_t::trace ( LEVELCHK(quiet), 
	  MODULENAME,
	  1,
	  "the path[%s] does not exit.",
	  base.c_str());
	}
	rc = false;
	break;
      }		
      mypath = NULL;	
    }
  
  free(mypathstart);

  return rc;
}
