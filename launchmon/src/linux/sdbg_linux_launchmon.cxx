/*
 * $Header: $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008 - 2010, Lawrence Livermore National Security, LLC. Produced at
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
 *        Oct 26 2012 DHA: Removed catch clauses for deprecated thread tracers 
 *                         exceptions.
 *        Jul 31 2012 DHA: Added a fix for a thread race-related hang problem.
 *        Jun 01 2012 DHA: Merged with the middleware-support branch.
 *        Oct 07 2011 DHA: Dynamic resource manager detection support.
 *        Oct 27 2010 DHA: Reorganize methods within this file.
 *        Oct 26 2010 DHA: Mods to use slightly changed abstraction
 *                         for breakpoint_base_t and tracer_base_t.
 *        Oct 01 2010 DHA: Refactor handling of mpir variables setup to
 *                         handle_mpir_variables.
 *        Sep 02 2010 DHA: Added MPIR_attach_fifo support
 *        Jul 02 2010 DHA: Decreased the verbose level for Daemon args too long
 *                         check. Cleaned up that code to enhance the readability.
 *        Jun 30 2010 DHA: Added faster parse error detection support
 *        Jun 28 2010 DHA: Added support to implement LMON_fe_getRMInfo
 *        Jun 10 2010 DHA: Added CRAY XT support and RM MAP support.
 *        Apr 27 2010 DHA: Added more MEASURE_TRACING_COST support.
 *        Dec 22 2009 DHA: Added auxilliary vector support to discover
 *                         the loader's load address accurately. (e.g.,
 *                         without having to rely on the "_start" symbol exported.
 *        Dec 16 2009 DHA: Added COBO support
 *        May 07 2009 DHA: Added a patch to fix the attach-detach-and-reattach failure
 *                         on BlueGene. This requires IBM efix27.
 *        Mar 04 2009 DHA: Added BlueGene/P support.
 *                         In particular, changed RM_BGL_MPIRUN to RM_BG_MPIRUN 
 *                         to genericize BlueGene Support.
 *                         Added indirect breakpoint support.
 *        Sep 24 2008 DHA: Enforced the error handling semantics
 *                         defined in README.ERROR_HANDLIN.
 *        Sep 22 2008 DHA: Added set_last_seen support to enable
 *                         a two-phased polling scheme.
 *        Jun 18 2008 DHA: Added 64 bit mpirun support.
 *        Mar 11 2008 DHA: Added Linux PowerPC/BlueGene support.
 *        Feb 09 2008 DHA: Added LLNS Copyright.
 *        Dec 05 2007 DHA: fixed a scalability bug that was exposed during
 *                         adding the modle checker support. When pcount > 2K,
 *                         the sheer number of environment variables in
 *                         the standalone mode causes the execvp call of the
 *                         child process to fail. I added a check for
 *                         execvp and a note saying the developers should
 *                         use the API mode when higher scalability is
 *                         desired. In addition, I removed envVar exporting
 *                         for the model checking case.
 *        Dec 04 2007 DHA: freed launcher_proctable (TV detects a memory leak there).
 *        Mar 13 2007 DHA: pipe_t support. Better coding for proctab message packing.
 *                         Turned on the PID environment variable support for
 *                         standalone launchmon utility.
 *        Jan 09 2006 DHA: Linux X86/64 support
 *        Jul 03 2006 DHA: Better self tracing support
 *        Jun 30 2006 DHA: Added acquire_protable so that both
 *                         handle_launch_bp_event and
 *                         handle_trap_after_attach_event can user the service.
 *        Jun 29 2006 DHA: Added chk_pthread_libc_and_init private
 *                         method. This is mainly to enhance code
 *                         reusability. It is now used not only by
 *                         handle_loader_bp_event, but also by
 *                         handle_trap_after_attach_event.
 *        Jun 29 2006 DHA: Added get_va_from_procfs support which
 *                         allows fetching the base address of the
 *                         dynamic linker module.
 *                         Only other way that I know of that would
 *                         allow me to do this is by spawning a sample
 *                         process and do some math when it's stopped
 *                         at the first fork/exec. Even so, with modern
 *                         Redhead security features (exec-shield),
 *                         the sample process may not generate an exact
 *                         base address for the dynlinker.
 *        Jun 08 2006 DHA: Added attach-to-a-running job support.
 *                         handle_attach_event method
 *        Mar 31 2006 DHA: Some read operations are now using.
 *                         tracer_string_read instead of tracer_read.
 *        Mar 31 2006 DHA: Added self tracing support.
 *        Mar 30 2006 DHA: Added exception handling support.
 *        Jan 12 2006 DHA: Created file.
 */


#include "sdbg_std.hxx"

#ifndef LINUX_CODE_REQUIRED
#error This source file requires a LINUX OS
#endif

extern "C" {
#if HAVE_SYS_TYPES_H 
# include <sys/types.h>
#else
# error sys/types.h is required
#endif

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#else
# error sys/stat.h is required
#endif

#if HAVE_FCNTL_H
# include <fcntl.h>
#else
# error fcntl.h is required
#endif

#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#else
# error sys/socket.h is required
#endif

#if HAVE_ARPA_INET_H
# include <arpa/inet.h>
#else
# error arpa/inet.h is required
#endif

#if HAVE_LINK_H
# include <link.h>
#else
# error link.h is required
#endif

#if HAVE_THREAD_DB_H
# include <thread_db.h>
#else
# error thread_db.h is required
#endif

#if HAVE_LIBGEN_H
# include <libgen.h>
#else
# error libgen.h is required
#endif

#if HAVE_LIMITS_H
# include <limits.h>
#else
# error limits.h is required 
#endif 
}

#include "sdbg_self_trace.hxx"
#include "sdbg_base_symtab.hxx" 
#include "sdbg_base_symtab_impl.hxx"
#include "sdbg_base_mach.hxx"
#include "sdbg_base_mach_impl.hxx"
#include "sdbg_base_tracer.hxx"

#include "sdbg_base_launchmon.hxx"
#include "sdbg_base_launchmon_impl.hxx"
#include "sdbg_rm_map.hxx"

#include "sdbg_linux_std.hxx"
#include "sdbg_linux_bp.hxx"
#include "sdbg_linux_mach.hxx"
#include "sdbg_linux_launchmon.hxx"
#include "sdbg_linux_ptracer.hxx"
#include "sdbg_linux_ptracer_impl.hxx"

#if MEASURE_TRACING_COST
static double accum = 0.0;
static int countHandler = 0;
static double beginTS;
static double endTS;
#endif

////////////////////////////////////////////////////////////////////
//                                                                //
// STATIC FUNCTIONS                                               //
//                                                                //
////////////////////////////////////////////////////////////////////

//!  File scope  get_va_from_procfs
/*!  get_va_from_procfs

     returns the starting virtual memory address of the given
     shared library using /proc file system. It returns T_UNINIT_HEX
     when fails to find "dynname." This is a hack; I need to get
     the base link address of the dynamic linker from AUX vector.

     NOTE: Dec 23 2009 DHA: This function shouldn't be used if other
     more standard methods are available like get_auxv below
*/
static
T_VA 
get_va_from_procfs ( pid_t pid, const std::string& dynname )
{ 
  using namespace std;

  FILE *fptr = NULL;
  char mapfile[PATH_MAX];
  char aline[MAX_STRING_SIZE];
  char *vir_addr_range = NULL;
  char *lowerpc = NULL;
  char *perm = NULL;
  char *libname = NULL;
  char libnamecp[PATH_MAX];

  T_VA ret_pc = T_UNINIT_HEX;

  sprintf ( mapfile, "/proc/%d/maps", pid );

  if ( ( fptr = fopen(mapfile, "r")) == NULL )   
    return ret_pc;   

  while ( fgets ( aline, MAX_STRING_SIZE, fptr ) ) 
    {
      vir_addr_range = strdup ( strtok ( aline, " " ) );
      perm = strdup ( strtok ( NULL, " " ) );
      strtok ( NULL, " " );
      strtok ( NULL, " " );
      strtok ( NULL, " " );
      libname = strdup ( strtok ( NULL, " " ) );

      // removing the trailing newline character
      //
      libname[strlen(libname)-1] = '\0'; 
      strncpy ( libnamecp, libname, PATH_MAX );
      if ( strcmp(dynname.c_str(), basename(libnamecp)) == 0 )
	{
	  if ( strcmp("r-xp", perm) == 0 ) 
	    {
	      lowerpc = strtok ( vir_addr_range, "-" );
#if BIT64 
	      sscanf ( lowerpc, "%lx", &ret_pc );
#else
	      sscanf ( lowerpc, "%x", &ret_pc );
#endif
	      break;
	    }
	}

      free ( vir_addr_range );
      free ( perm );
      free ( libname );
      vir_addr_range = NULL;
      perm = NULL;
      libname = NULL;
    }

  fclose ( fptr );

  return ret_pc;
}


//!  File scope  get_auxv
/*!  get_auxv

     returns the base address of the loader through the auxvector information 
     that OS provides.
*/
static
T_VA 
get_auxv ( pid_t pid )
{ 
  using namespace std;

  FILE *fptr = NULL;
  char auxvFile[PATH_MAX];
  
#if BIT64
  Elf64_auxv_t auxvBuf;
  int len = sizeof (Elf64_auxv_t);
#else
  Elf32_auxv_t auxvBuf;
  int len = sizeof (Elf32_auxv_t);
#endif

  T_VA ret_pc = T_UNINIT_HEX;

  sprintf ( auxvFile, "/proc/%d/auxv", pid );

  if ( ( fptr = fopen(auxvFile, "r")) == NULL )   
    {
      //
      // In case we see problems with opening up auxv pseudo file,
      // simply return T_UNITNIT_HEX
      //
      return ret_pc;
    }
    
  do {
    if (fread (&auxvBuf, len, 1, fptr) < 0)
      {
	//
	// Should I wrap this call as well w.r.t. EINTR?
	//
        return ret_pc;
      }
  } while (auxvBuf.a_type != AT_BASE);


  fclose(fptr);

  ret_pc = (auxvBuf.a_type == AT_BASE)? auxvBuf.a_un.a_val : T_UNINIT_HEX;

  return ret_pc;
}


////////////////////////////////////////////////////////////////////
//                                                                //
// PRIVATE METHODS (class linux_launchmon_t<>)                    //
//                                                                //
////////////////////////////////////////////////////////////////////

//!  PRIVATE: 
/*! linux_launchmon_t::init_API initialize API support
    within the engine
*/
launchmon_rc_e
linux_launchmon_t::init_API(opts_args_t *opt)
{ 
  char *tokenize = NULL;
  char *FEip = NULL;
  char *FEport = NULL;
  int optval = 1;
  int optlen = sizeof(optval);
  int clientsockfd;
  struct sockaddr_in servaddr;

  if (!opt->get_my_opt()->remote) 
    {
      //
      // if this isn't API mode, apparently you don't have to 
      // do anything.
      //
      goto no_error;
    }

  //
  // parsing ip:port info
  //
  tokenize = strdup(opt->get_my_opt()->remote_info.c_str());
  FEip = strtok ( tokenize, ":" );
  FEport = strtok ( NULL, ":" );

  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons((uint16_t) atoi(FEport));

  //
  // converting the text IP (or hostname) to binary
  //
  if ( inet_pton(AF_INET, (const char*) FEip, &(servaddr.sin_addr)) < 0 )
    {
      self_trace_t::trace ( LEVELCHK(level1),
        MODULENAME,1,
        "inet_pton failed in the engine init handler.");
      goto has_error;
    }

  free(tokenize);

  if ( ( clientsockfd = socket ( AF_INET, SOCK_STREAM, 0 )) < 0 )
    {
      self_trace_t::trace ( LEVELCHK(level1),
        MODULENAME,1,
        "socket failed in the engine init handler.");
      goto has_error;
    }

  if( setsockopt(clientsockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0 )
    {
      self_trace_t::trace ( LEVELCHK(level1),
        MODULENAME,1,
        "setting socket keepalive failed.");
      goto has_error;
    }

  if ( ( connect ( clientsockfd, 
	 	   (struct sockaddr *)&servaddr,
                   sizeof(servaddr) )) < 0 )
    {
      self_trace_t::trace ( LEVELCHK(level1),
        MODULENAME,1,
        "connect failed in the engine's init handler.");
      goto has_error;
    }

  //
  // registering the FD for FE-client and engine connection
  //
  set_FE_sockfd ( clientsockfd );

  //
  // setting API mode flag
  //
  set_API_mode ( true );

  {
    self_trace_t::trace ( LEVELCHK(level2),
      MODULENAME,0,
      "linux_launchmon_t initialized.");
  }

  //
  // communicate to the FE API that there was a parse error
  //
  if (opt->get_has_parse_error())
    {
      say_fetofe_msg ( lmonp_conn_ack_parse_error );
      self_trace_t::trace ( true,
        MODULENAME,1,
        "the engine deteced parsing errors.");
      goto has_error;
    } 
  else
    {
      say_fetofe_msg ( lmonp_conn_ack_no_error );
    }
  
no_error:
  return LAUNCHMON_OK;

has_error:
  return LAUNCHMON_FAILED;
}


//!  PRIVATE: 
/*! linux_launchmon_t::free_engine_resources constructors & destructor
    free resources that the engine created here including
    files.
*/
void
linux_launchmon_t::free_engine_resources (
  process_base_t<SDBG_LINUX_DFLT_INSTANTIATION> &p )
{
  const char *hostnamesfn 
    = p.get_myopts()->get_my_rmconfig()->get_hostnames_fn();

  if (hostnamesfn)
    {
      remove(hostnamesfn);
    }
}


//! PRIVATE: 
/*! linux_launchmon_t<> constructors & destructor
    Making copy Ctor private keep an object from being copied

*/
linux_launchmon_t::linux_launchmon_t ( const linux_launchmon_t& l )
{
  // launchmon object must not be copied, exiting.
}


//! PRIVATE: linux_launchmon_t::launch_tool_daemons
/*!
    launches the target tool deamons.

*/
bool
linux_launchmon_t::launch_tool_daemons ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION> &p )
{
  using namespace std;

  if (get_proctable_copy().empty())
    {
      self_trace_t::trace ( true, 
          MODULENAME,0,
          "Proctab is empty!");

      return false;
    }

  if ( !get_API_mode()
       && (!p.rmgr()->is_modelchecker()))
    {
      //
      // Standalone launchmon. The launchmon session is not
      // driven by LMON APIs...
      //
      // In this case, we must communicate the RPDTAB info via 
      // environment variables.
      // Assuming the system will copy the envVar list of the parallel
      // launcher to remote nodes, following creates envVars
      // of the format,
      // LAUNCHMON_hostname=pid1:pid2:pid3...
      //
      // Each of the tool daemons spawned on to the remote nodes can find 
      // its own target PID list by looking at hostname specific 
      // environment variable.
      //
      // Note that this method won't scale beyond 2K-ish.
      // For example, at 4K, the sheer number of environment variables
      // causes a following execvp to fail, returning to the caller.
      // For higher scalability, the developer should consider
      // using the API mode.
      //

      map<string, vector<MPIR_PROCDESC_EXT*> >::const_iterator pos;
      vector<MPIR_PROCDESC_EXT*>::const_iterator vpos;
      char *execname = NULL;

      for (pos = get_proctable_copy().begin(); pos != get_proctable_copy().end(); pos++) 
	{	
	  string pidlist;
	  for(vpos = pos->second.begin(); vpos != pos->second.end(); vpos++) 
	    {
	      char pidbuf[10];
	      sprintf(pidbuf, "%d:", (*vpos)->pd.pid);
	      pidlist = pidlist + string(pidbuf);

	      if(!execname) 
		execname = strdup((*vpos)->pd.executable_name);
	}				

	// 
	// envVar looks like LAUNCHMON_alc0=12376:23452
	//
	string envname = string("LAUNCHMON_") + string(pos->first);
	setenv(envname.c_str(), pidlist.c_str(), 1);
	}
    }
 
  if ( p.get_myopts()->get_my_rmconfig()->is_modelchecker())
    {
      //
      // mpirun model checker support
      //

      map<string, vector<MPIR_PROCDESC_EXT*> >::const_iterator pos;
      vector<MPIR_PROCDESC_EXT*>::const_iterator vpos;

      for (pos = get_proctable_copy().begin(); 
               pos != get_proctable_copy().end(); pos++) 
	{	
	  for(vpos = pos->second.begin(); vpos != pos->second.end(); vpos++) 
	    {
	      self_trace_t::trace ( 1, 
	       MODULENAME,0,
	       "MODEL CHECKER: %s, %d, %s",
				    (*vpos)->pd.host_name,
				    (*vpos)->pd.pid,
				    (*vpos)->pd.executable_name);
	    }
	}
       return LAUNCHMON_OK;
    }
 
  map<string, string>::const_iterator envListPos;

  for (envListPos = p.get_myopts()->get_my_opt()->envMap.begin(); 
       envListPos !=  p.get_myopts()->get_my_opt()->envMap.end(); envListPos++)
    {
      setenv(envListPos->first.c_str(), envListPos->second.c_str(), 1);	
    }

  //
  // W/ new RM MAP support
  //
  if (p.get_myopts()->get_my_rmconfig()->is_coloc_sup())
    {
      //
      // there isn't much you want to do here,
      // because RM that supports colocation service 
      // co-spawns daemons as part of its MPIR APAI extension. 
      // This includes BGRM

      //
      // TODO: We may not want to release the mpirun process until the 
      // tool set up is done...
      //
      //
      // DHA 3/4/2009, reviewed and looks fine for BGP
      // Changed RM_BGL_MPIRUN to RM_BG_MPIRUN to genericize BlueGene Support
      //
    }
  else
    {
#if MEASURE_TRACING_COST
      {
        self_trace_t::trace ( true, /* print always */
 	   MODULENAME,0,
	  "About to fork a RM process to bulk-launch daemons");
        self_trace_t::trace ( true, /* print always */ 
           MODULENAME,0,
	  "COUNT: %d, ACCUM TIME (except for the proctab fetching): %f", countHandler, accum);
      } 
#endif

      char *tokenize2 = strdup(p.get_myopts()->get_my_opt()->lmon_sec_info.c_str());
      char *sharedsecret = strtok (tokenize2, ":");
      char *randomID = strtok (NULL, ":");
      char hnfn[PATH_MAX] = {'\0'};
      char tmpsuf[128] = {'\0'};

      //
      //
      // Handle the file that contains hostnames of the target nodes
      if (!getcwd(hnfn, PATH_MAX))
        {
          self_trace_t::trace ( true, /* print always */ 
          MODULENAME,1,
	  "getcwd failed" );

          return false;
        }

      strcat(hnfn, "/");
      strcat(hnfn, LMON_HOSTS_FN_BASE);
      snprintf(tmpsuf, 128, ".%d", getpid());
      strcat(hnfn, tmpsuf);
      ofstream hnstream;
      hnstream.open(hnfn, ios::out);

      if (hnstream.fail())
        {
          self_trace_t::trace ( true, 
            MODULENAME,0,
            "open %s failed, trying /tmp/%s instead", hnfn, LMON_HOSTS_FN_BASE );

          snprintf(hnfn, PATH_MAX, "/tmp/%s", LMON_HOSTS_FN_BASE);
          strcat(hnfn, tmpsuf);

          // try open again
          hnstream.open(hnfn, ios::out);
          if (hnstream.fail())
            {
              self_trace_t::trace ( true, 
                MODULENAME,0,
                "open %s also failed!", hnfn);
              return false;
            }
          }

      std::map<std::string, std::vector<MPIR_PROCDESC_EXT *> >::const_iterator pos;
      int count=0;
      for (pos = get_proctable_copy().begin(); pos != get_proctable_copy().end(); ++pos)
        {
          hnstream << pos->first;
          count++;
          if (count < get_proctable_copy().size())
            {
              hnstream << "\n";
            } 
        }

      hnstream.close();

      p.rmgr()->set_paramset(
	    get_proctable_copy().size(),
	    get_proctable_copy().size(),
	    sharedsecret,
	    randomID,
	    get_resid(),
            hnfn
      );

      free(tokenize2);

      std::string expandstr;
      std::list<std::string> alist
        = p.rmgr()->expand_launch_string(expandstr);

      {
        self_trace_t::trace (
          LEVELCHK(level1),
          MODULENAME,0,
          "launching daemons with: %s",
          expandstr.c_str());
       }

      //
      // For non-colocation RM, we need to fork/exec
      //
      set_toollauncherpid  (fork());
      if ( !get_toollauncherpid ())
        {
          char **av = (char **) malloc((alist.size() + 1)*sizeof(char *));
          size_t indx=0;
          std::list<std::string>::iterator iter;
          for (iter = alist.begin(); iter != alist.end(); iter++)
            {
              av[indx] = strdup((*iter).c_str());
              indx++;
            }

          av[indx] = NULL;

          //
          // Sink
          //
          if ( execvp ( av[0], av) < 0 )
            {
              self_trace_t::trace ( true,
                MODULENAME,1,
                "execvp to launch tool daemon failed");
                perror("");
                exit(1);
            }
        } //Child vs. parent
        // Only parent reaches here
    } // Co-location vs. traditional daemon launching

    return LAUNCHMON_OK;
}


//! PRIVATE: handle_bp_prologue
/*!
    performs the breakpoint event prologue. It includes 
    stepping over the target breakpoint.
*/
launchmon_rc_e 
linux_launchmon_t::handle_bp_prologue ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION> &p,
		 breakpoint_base_t<T_VA,T_IT> *bp)
{
  try 
    {
      T_VA pc           = T_UNINIT_HEX;
      T_VA adjusted_pc  = T_UNINIT_HEX;    
      T_VA retaddr      = T_UNINIT_HEX;
      bool use_cxt      = true;

      get_tracer()->tracer_getregs(p, use_cxt);
      pc = p.get_gprset(use_cxt)->get_pc();

      //
      // We must keep track of the "return address" 
      // in case the following single step causing the caller
      // to return from the target function,  
      // to simply execute the very next instruction. 
      //
      retaddr = p.get_gprset(use_cxt)->get_ret_addr();
      if ( retaddr == T_UNINIT_HEX )
        {
          T_VA mem_retaddr
            = p.get_gprset(use_cxt)->get_memloc_for_ret_addr();

          get_tracer()->tracer_read(p,
                                    mem_retaddr,
                                    &retaddr,
                                    sizeof(retaddr),
                                    use_cxt );
        }

      bp->set_return_addr(retaddr);
      get_tracer()->disable_breakpoint(p, *bp, use_cxt); 
      adjusted_pc = bp->get_address_at(); 
      p.get_gprset(use_cxt)->set_pc(adjusted_pc);
      get_tracer()->tracer_setregs(p, use_cxt);
      get_tracer()->tracer_singlestep(p, use_cxt);

      {
	self_trace_t::trace ( 
          LEVELCHK(level3), 
	  MODULENAME,0, 
	  "breakpoint event prologue completed. [pc=0x%x]", 
	  pc);
      }

      return LAUNCHMON_BP_PROLOGUE;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }  
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
}


//! PRIVATE: is_bp_prologue_done
/*!
    checks if the prologue for the breakpoint event has been 
    performed. If not, it calls into the prologue.
*/
bool
linux_launchmon_t::is_bp_prologue_done ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION> &p,
		 breakpoint_base_t<T_VA,T_IT> *bp )
{
  try 
    {
      T_VA pc = T_UNINIT_HEX;    
      bool use_cxt = true;

      get_tracer()->tracer_getregs(p,use_cxt);
      pc = p.get_gprset(use_cxt)->get_pc();  

      if ( bp->is_disabled() ) 
	{
	  get_tracer()->enable_breakpoint ( p, *bp, use_cxt);

	  {
	    self_trace_t::trace ( 
              LEVELCHK(level3), 
	      MODULENAME,0,
	      "breakpoint event prologue was already done. [pc=0x%x]",
	      pc);
	  }

	  return true;
	}  

      return (handle_bp_prologue(p, bp) == LAUNCHMON_BP_PROLOGUE)? false : true; 
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }    
}


//! PRIVATE: linux_launchmon_t::acquire_proctable
/*!
    acquires RPDTAB as well as the resource ID if available. 
    (e.g., totalview_jobid)

*/
bool 
linux_launchmon_t::acquire_proctable (
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p, 
		 bool use_cxt )
{
  try
    {
      using namespace std;

      char resource_id[MAX_STRING_SIZE];
      image_base_t<T_VA,elf_wrapper>* main_im;

      T_VA proctable_loc;
      unsigned long long i, maxcount;   // proctable index can be large!
      int local_pcount;      
#if MEASURE_TRACING_COST   
      double c_start_ts;
      double c_end_ts;
      c_start_ts = gettimeofdayD();
#endif
      main_im  = p.get_myimage();
      if (!main_im)
        {
	  self_trace_t::trace ( 
             true, 
	     MODULENAME, 1, 
	     "main image not processed");  
          return false;
        } 

      //
      // fetching the RPDTAB size
      //
      symbol_base_t<T_VA> debug_ps 
	= main_im->get_a_symbol ( p.get_launch_proctable_size() );

      if (!debug_ps && p.get_myrmso_image())
        {
          debug_ps
            = p.get_myrmso_image()->get_a_symbol(p.get_launch_proctable_size());
        }

      T_VA procsize_addr = debug_ps.get_relocated_address();  

      get_tracer()->tracer_read( p, 
				 procsize_addr, 
				 &(local_pcount), 
				 sizeof(local_pcount), 
				 use_cxt );     
      set_pcount (local_pcount);
      if (get_pcount() <= 0 )
        {
	  self_trace_t::trace ( 
             true, 
	     MODULENAME, 1, 
	     "MPIR_proctable_size is negative");  
          return false;
        }

      //
      // launcher_proctable holds the MPIR_PROCDESC array. Note that
      // it only contains scalars and pointer addresses. To fetch 
      // the strings pointed by those pointer addresses, we must
      // perform separate read operations using those addresses.
      //
      MPIR_PROCDESC* launcher_proctable 
	= (MPIR_PROCDESC *) malloc (sizeof (MPIR_PROCDESC) * get_pcount());   

      if (!launcher_proctable)
        {
	  self_trace_t::trace ( 
             true, 
	     MODULENAME, 
             1, 
	     "Out of memory!");  

          return false;
        }

      symbol_base_t<T_VA> debug_pt 
	    = main_im->get_a_symbol ( p.get_launch_proctable() );
      if (!debug_pt && p.get_myrmso_image())
        {
          debug_pt
            = p.get_myrmso_image()->get_a_symbol(p.get_launch_proctable());
        }

      T_VA proctable_addr = debug_pt.get_relocated_address();
      get_tracer()->tracer_read ( p, 
				  proctable_addr, 
				  &proctable_loc,
				  sizeof(proctable_loc), 
				  use_cxt );
      get_tracer()->tracer_read ( p, 
				  proctable_loc,
				  launcher_proctable,
				  ( sizeof (MPIR_PROCDESC) * get_pcount()), 
				  use_cxt );

 
      //
      // fetching each proctable entry including strings pointed by 
      // C pointers.
      //
      maxcount = (unsigned long long) get_pcount();
      for ( i = 0; i < maxcount; ++i ) 
	{
	  MPIR_PROCDESC_EXT* an_entry 
	    = (MPIR_PROCDESC_EXT* ) malloc(sizeof(MPIR_PROCDESC_EXT));

          if (!an_entry)
            {
	      self_trace_t::trace ( 
                true, 
	        MODULENAME, 1, 
	       "Out of memory!");  

              return false;
            }

	  //
	  // allocating storages for "an_entry"
	  //	
	  an_entry->pd.host_name 
                 = (char*) malloc(MAX_STRING_SIZE);
	  an_entry->pd.executable_name 
                 = (char*) malloc(MAX_STRING_SIZE);
#if SUB_ARCH_BGQ
	  an_entry->cnodeid
                 = launcher_proctable[i].pid;      
#else
	  an_entry->pd.pid 
                 = launcher_proctable[i].pid;      
	  an_entry->cnodeid = -1;      
#endif
	  an_entry->mpirank = i; /* The mpi rank is the index into the global tab */

#if SUB_ARCH_BGQ
	  an_entry->pd.pid = i; /* The mpi rank is the index into the global tab */
#endif

	  //
	  // memory-fetching to get the "host_name" 
          //
	  get_tracer()->tracer_read_string(p, 
			  (T_VA) launcher_proctable[i].host_name,
			  (void*) (an_entry->pd.host_name),
			  MAX_STRING_SIZE,
			  use_cxt );   
   
	  //
    	  // memory-fetching to get the "executable name" 
	  //	 
	  get_tracer()->tracer_read_string(p, 
		          (T_VA)launcher_proctable[i].executable_name,
			  (void*)an_entry->pd.executable_name,
			  MAX_STRING_SIZE,
			  use_cxt );

	  get_proctable_copy()[an_entry->pd.host_name].push_back(an_entry);
	} 

      free ( launcher_proctable );

      if ( get_proctable_copy().empty() )
	{
	  self_trace_t::trace ( LEVELCHK(level1),
	     MODULENAME, 1,
	     "proctable is empty!");

	  return LAUNCHMON_FAILED;
	}

      if (p.get_myopts()->get_my_rmconfig()->is_rid_sup())
        {
          if (p.get_myopts()->get_my_rmconfig()->is_rid_via_symbol())
            {
              resource_manager_t r_mgr;
              r_mgr = p.get_myopts()->get_my_rmconfig()->get_resource_manager();
              std::string rid_sym = r_mgr.get_job_id().id.symbol_name;

              symbol_base_t<T_VA> rid
                = main_im->get_a_symbol(rid_sym);
              if (!rid && p.get_myrmso_image())
                {
                  rid = p.get_myrmso_image()->get_a_symbol(rid_sym);
                }


              if (r_mgr.get_job_id().dtype == cstring)
                {
                  T_VA where_is_rid;

                  get_tracer()->tracer_read(
                                        p,
                                        rid.get_relocated_address(),
                                        (void *) &where_is_rid,
                                        sizeof(T_VA),
                                        use_cxt);

                  get_tracer()->tracer_read_string (
                                        p,
                                        where_is_rid,
                                        (void*) resource_id,
                                        MAX_STRING_SIZE,
                                        use_cxt);

                  set_resid(atoi(resource_id));
                  p.set_rid(get_resid());

                  // -1 is the init value that SLURM sets internally 
                  // for "totalview_jobid"
                  if ( get_resid() == -1 )
	           {
	             self_trace_t::trace ( LEVELCHK(level1),
	               MODULENAME, 1,
	               "resource ID is not valid!");

	             return LAUNCHMON_FAILED;
	           }
                }
              else if (r_mgr.get_job_id().dtype == integer32)
                {
                  uint32_t int_val;

                  get_tracer()->tracer_read(
                                        p,
                                        rid.get_relocated_address(),
                                        (void *) &int_val,
                                        sizeof(int_val),
                                        use_cxt);
                  set_resid(int_val);
                  p.set_rid(get_resid());
                }
          else if (p.get_myopts()->get_my_rmconfig()->is_rid_via_pid())
            {
              set_resid (p.get_pid(false));
              p.set_rid (get_resid());
            }
        }
      }


#if MEASURE_TRACING_COST
      c_end_ts = gettimeofdayD();
      {
        self_trace_t::trace ( true,
           MODULENAME,0,
           "PROCTAB(%d) Fetching: %f ",
           get_pcount(), (c_end_ts - c_start_ts));
      }
#endif

      return LAUNCHMON_OK;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report ();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report ();
      return LAUNCHMON_FAILED;
    }  
  catch ( machine_exception_t e )
    {
      e.report ();
      return LAUNCHMON_FAILED;
    }
}


//!  PRIVATE: linux_launchmon_t::disable_all_BPs
/*! 
     disables all the hidden breakpoints    
*/
bool 
linux_launchmon_t::disable_all_BPs ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p, 
		 bool use_cxt, bool change_state )
{
  try 
    {

      tracer_base_t<SDBG_LINUX_DFLT_INSTANTIATION> *tr = get_tracer(); 
 
      if ( p.get_launch_hidden_bp() ) 
	{
	 tr->disable_breakpoint ( 
           p, 
	   *(p.get_launch_hidden_bp()), 
	   use_cxt,
           change_state);
	}

      if ( p.get_loader_hidden_bp() ) 
	{
	 tr->disable_breakpoint ( 
           p, 
           *(p.get_loader_hidden_bp()), 
	   use_cxt,
           change_state);
	}

      return true;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return false;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return false;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      return false;
    }
}


//!  PRIVATE: linux_launchmon_t::set_mpir_variables
/*!   
     set MPIR variables
*/
bool 
linux_launchmon_t::handle_mpir_variables (
                process_base_t<SDBG_LINUX_DFLT_INSTANTIATION> &p,
                image_base_t<T_VA,elf_wrapper> &image )
{
  try
    {
      bool use_cxt = true;
      breakpoint_base_t<T_VA, T_IT> *la_bp;
      T_VA debug_state_flag;
      int bdbg = 1;

      //
      // registering p.launch_hidden_bp 
      //
      const symbol_base_t<T_VA>& launch_bp_sym
        = image.get_a_symbol (p.get_launch_breakpoint_sym());

      if (!(!launch_bp_sym) && launch_bp_sym.is_defined())
        {
          if ( p.get_launch_hidden_bp() )
            {
               if (p.get_launch_hidden_bp()->is_enabled())
                 {
                   //
                   // If bp already inserted, remove it.
                   //
                   get_tracer()->disable_breakpoint( p,
                                   *(p.get_launch_hidden_bp()),
                                   use_cxt);
                  }
              delete p.get_launch_hidden_bp();
              p.set_launch_hidden_bp(NULL);
            }
          la_bp = new linux_breakpoint_t();
          la_bp->set_address_at(launch_bp_sym.get_relocated_address());

#if PPC_ARCHITECTURE
          //
          // DHA Mar 05 2009
          // PowerPC Linux has begun to change the linking convention
          // such that binaries no longer export direct function
          // symbols. (e.g., .MPIR_Breakpoint). But rather, undotted
          // global data symbols (e.g., MPIR_Breakpoint) contains the
          // address for the corresponding function.
          //
          // Added indirect breakpoint support for that and use this
          // method on all PPC systems across the board including
          // BGL and BGP
          //
          la_bp->set_use_indirection();
#endif
          la_bp->set();

          p.set_launch_hidden_bp(la_bp);
          get_tracer()->enable_breakpoint(p,
                                          *(p.get_launch_hidden_bp()),
                                          use_cxt );
        }

      //
      // setting MPIR_being_debugged
      //
      const symbol_base_t<T_VA>& debug_state
        = image.get_a_symbol (p.get_launch_being_debug());
      if (!(!debug_state))
        {
          debug_state_flag = debug_state.get_relocated_address();
          get_tracer()->tracer_write ( p,
                                       debug_state_flag,
                                       &bdbg,
                                       sizeof(bdbg),
                                       use_cxt );
        }

      if (p.rmgr()->is_coloc_sup())
        {
          //
          // Always check correctness of BG_SERVERARG_LENGTH 
          // and BG_EXECPATH_LENGTH
          //
          const int BG_SERVERARG_LENGTH = 1024;
          const int BG_EXECPATH_LENGTH = 256;
          const symbol_base_t<T_VA>& executablepath
            = image.get_a_symbol (p.get_launch_exec_path ());
          const symbol_base_t<T_VA> &sa
            = image.get_a_symbol (p.get_launch_server_args ());

          if (!(!executablepath) && !(!sa))
            { 
              T_VA ep_addr = executablepath.get_relocated_address();
              std::string daemon_pth;
              daemon_pth = p.rmgr()->get_const_coloc_paramset().rm_daemon_path;
              size_t dpsize = daemon_pth.size() + 1;

              if (dpsize > BG_EXECPATH_LENGTH)
                {
                  self_trace_t::trace ( true,
                    MODULENAME,1,
                    "daemon path(%d) exceeds the buffer length: %d",
                    dpsize,
                    BG_EXECPATH_LENGTH);

                  return LAUNCHMON_FAILED;
                }

              get_tracer()->tracer_write(p,
                                         ep_addr,
                                         daemon_pth.c_str(),
                                         dpsize,
                                         use_cxt);

              char *tokenize2 
                = strdup(p.get_myopts()->get_my_opt()->lmon_sec_info.c_str());
              char *sharedsecret = strtok (tokenize2, ":");
              char *randomID = strtok (NULL, ":");
              p.rmgr()->set_paramset(0,
                                     0,
                                     sharedsecret,
                                     randomID,
                                     -1,
                                     NULL);

              std::string expstr;
              std::list<std::string> alist;
              //
              // Change this for vector<string> interface
              //
              alist = p.rmgr()->expand_launch_string(expstr);
              char serverargs[BG_SERVERARG_LENGTH] = {0};
              T_VA sa_addr = sa.get_relocated_address();

              if (expstr != "")
                {
                  if (expstr.size() >= BG_SERVERARG_LENGTH)
                    {
                      self_trace_t::trace (true,
                        MODULENAME, 1,
                        "Daemon args list too long to fit %s", expstr.c_str());
                    }
                  else
                    {
                      char *traverse = serverargs;
                      std::list<std::string>::iterator iter;
                      for (iter = alist.begin(); iter != alist.end(); iter++)
                        {
                          memcpy((void *)traverse,
                             (const void *)((*iter).c_str()),
                             (size_t)((*iter).size()));
                          traverse += ((*iter).size());
                          (*traverse) = '\0';
                          traverse += 1;
                        }

                      get_tracer()->tracer_write(p,
                                                 sa_addr,
                                                 serverargs,
                                                 BG_SERVERARG_LENGTH,
                                                 use_cxt);
                    }
                }
            }

          if (p.rmgr()->is_attfifo_sup())
            {
              const symbol_base_t<T_VA>& af_sym 
                = image.get_a_symbol(p.get_launch_attach_fifo());

              if (!(!af_sym) && !(p.get_sym_attach_fifo()))
                {
                  symbol_base_t<T_VA> *af_sym_copy
                    = new symbol_base_t<T_VA>(af_sym);
                  p.set_sym_attach_fifo(af_sym_copy);
                }
            } // RM with MPIR_attach_fifo support
        } // With MPIR Colocation service
    }
  catch ( symtab_exception_t e )
    {
      e.report();
      return false;
    }
  catch ( tracer_exception_t e )
    {
      e.report();
      return false;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      return false;
    }
}


//!  PRIVATE: linux_launchmon_t::enable_all_BPs
/*!   
     enables all the hidden breakpoints
*/
bool 
linux_launchmon_t::enable_all_BPs ( 
	         process_base_t<SDBG_LINUX_DFLT_INSTANTIATION> &p, 
		 bool use_cxt,
                 bool change_state )
{
  try 
    {
      tracer_base_t<SDBG_LINUX_DFLT_INSTANTIATION> *tr = get_tracer();
 
      if (p.get_launch_hidden_bp()) 
	{
	  tr->enable_breakpoint ( 
            p, 
	    *(p.get_launch_hidden_bp()), 
	    use_cxt,
            change_state);
	}

      if (p.get_loader_hidden_bp()) 
	{
	  tr->enable_breakpoint ( 
            p, 
	    *(p.get_loader_hidden_bp()), 
	    use_cxt,
            change_state);
	}

      return true;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return false;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return false;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      return false;
    }
}


//!  PRIVATE: linux_launchmon_t::check_dependent_SOs
/*!
     checks to see if libpthread is linked, and if so
     initializes the thread tracer
     Sep 13 2010, we now handle RM SO here as well. 
*/
bool 
linux_launchmon_t::check_dependent_SOs ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  try 
    {
      using namespace std;

      bool use_cxt = true;
      bool rc = false;
      image_base_t<T_VA,elf_wrapper> *dynloader_im;
      image_base_t<T_VA,elf_wrapper> *thr_im;
      image_base_t<T_VA,elf_wrapper> *libc_im;
      image_base_t<T_VA,elf_wrapper> *rmso_im;
      breakpoint_base_t<T_VA, T_IT> *frbp;
      T_VA where_to_read;
      struct r_debug r_debug_buf;
      struct link_map a_map;
      char lname[MAX_STRING_SIZE];

      thr_im = p.get_mythread_lib_image();
      libc_im = p.get_mylibc_image();
      rmso_im = p.get_myrmso_image();

      if (!thr_im || !libc_im || !rmso_im)
        {
	  {
	    self_trace_t::trace (true, 
	      MODULENAME, 0,
	      "objects for key binary images are not allocated");
	  }

          return false;
        }

      if ( (libc_im->get_image_base_address() != SYMTAB_UNINIT_ADDR)
           && (thr_im->get_image_base_address() !=  SYMTAB_UNINIT_ADDR )
           && (!p.rmgr()->need_check_launcher_so()
               || rmso_im->get_image_base_address() !=  SYMTAB_UNINIT_ADDR))
	{
	  {
	    self_trace_t::trace ( LEVELCHK(level3), 
	      MODULENAME, 0,
	      "dependent libraries have been already found.");
	  }

	  return false;
	}

      dynloader_im = p.get_mydynloader_image();
      const symbol_base_t<T_VA>& r_debug_sym
	    = dynloader_im->get_a_symbol (p.get_loader_r_debug_sym());

      get_tracer()->tracer_read ( p, 
	r_debug_sym.get_relocated_address(), 
	&r_debug_buf, 
	sizeof(struct r_debug),
	use_cxt);

      assert ( r_debug_buf.r_map != 0 );  

      where_to_read = (T_VA) r_debug_buf.r_map;

    do 
      {
	get_tracer()->tracer_read ( p, 
				    where_to_read, &a_map, 
				    sizeof(struct link_map),
				    use_cxt);
	if (a_map.l_name) 
	  {
	    string slname;
	    char *cplname1, *cplname2, *dn, *bn;

	    get_tracer()->tracer_read_string ( p, 
	      (T_VA)(a_map.l_name), 
	      lname, 
	      MAX_STRING_SIZE,
	      use_cxt);

	    cplname1 = strdup(lname);
	    cplname2 = strdup(lname);
            dn = dirname(cplname1);
            bn = basename(cplname2);
	    slname = lname;

	    if ( strncmp(LIBPTHREAD_IDEN,bn,strlen(LIBPTHREAD_IDEN)) == 0 ) 
              {
                /*
                 * The linked map for the POSIX thread library is found
                 *
                 */
                if (thr_im->get_image_base_address() == SYMTAB_UNINIT_ADDR)
                  {
                    thr_im->init(slname);
		    thr_im->read_linkage_symbols();
	            thr_im->set_image_base_address(a_map.l_addr);
		    thr_im->compute_reloc(); 

		    rc = true;
                  }
	      } 
	    else if (strncmp(LIBC_IDEN,bn,strlen(LIBC_IDEN)) == 0 ) 
              {
                /*
                 * The linked map for the C runtime library is found
                 *
                 */
                if (libc_im->get_image_base_address() == SYMTAB_UNINIT_ADDR)
                  {
                    libc_im->init(slname);
                    libc_im->read_linkage_symbols();
                    libc_im->set_image_base_address(a_map.l_addr);	 	
		    libc_im->compute_reloc();
                  }
              }
            else if ( p.rmgr()->need_check_launcher_so() )
              {
                /*
                 * The linked map for RM SO is found
                 *
                 */
                std::string rmso = p.rmgr()->get_launcher_so_name();
                if ( (strncmp(rmso.c_str(), bn, rmso.size()) == 0) )
                  {
                    if (rmso_im->get_image_base_address() == SYMTAB_UNINIT_ADDR)
                      {
                        rmso_im->init(slname);
                        rmso_im->read_linkage_symbols();
                        rmso_im->set_image_base_address(a_map.l_addr);
                        rmso_im->compute_reloc();

                        //
                        // we want to reset mpir variables because rmso might
                        // hold the actual storage for those variables 
                        // and thus variables could have been (re)-initialized 
                        // as part of SO init right before this dl breakpoint
                        // is called.
                        //
                        handle_mpir_variables(p, *(p.get_myimage()));
                        p.set_myrmso_image(rmso_im);


                        //
                        // Some RM's base image doesn't "define" MPIR_Breakpoint
                        // In that case, we use the same symbol defined within the RM lib.  
                        //
                        handle_mpir_variables(p, *(p.get_myrmso_image()));
                      }
                  }
              }

            free(cplname1);
            free(cplname2);
	  }

	where_to_read =  (T_VA) a_map.l_next;

      } while (where_to_read && where_to_read != T_UNINIT_HEX );
 
    return rc;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    } 
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
}


////////////////////////////////////////////////////////////////////
//                                                                //
// PUBLIC INTERFACES (class linux_launchmon_t<>)                  //
//                                                                //
////////////////////////////////////////////////////////////////////

//! PUBLIC: 
/*! linux_launchmon_t<> constructors & destructor


*/
linux_launchmon_t::linux_launchmon_t () 
  : MODULENAME(self_trace_t::launchmon_module_trace.module_name)
{
  // more initialization here
}


//! PUBLIC: 
/*! linux_launchmon_t<> constructors & destructor


*/
linux_launchmon_t::~linux_launchmon_t ()
{

}


//! PUBLIC: init
/*!
    Method that registers platform specific process/thread 
    tracers into the platform indepdent layer. It also initializes 
    the FE-engine connection in API mode.
*/
launchmon_rc_e 
linux_launchmon_t::init ( opts_args_t *opt )
{
  launchmon_rc_e lrc;

  //
  // registering a linux process tracer
  //
  set_tracer(new linux_ptracer_t<SDBG_LINUX_DFLT_INSTANTIATION>());

  //
  // registering a linux thread tracer
  //

  //
  // API initialization
  //
  lrc = init_API(opt);

  set_last_seen(gettimeofdayD()); 

  return lrc;
}



//! decipher_an_event:
/*!
    deciphers an event of debug_event_t and returns a 
    corresponding launchmon_event_e code.
*/
launchmon_event_e 
linux_launchmon_t::decipher_an_event 
(process_base_t<SDBG_LINUX_DFLT_INSTANTIATION> &p, const debug_event_t &event)
{

  launchmon_event_e return_ev = LM_INVALID;
  bool use_context = true;

  switch ( event.get_ev () ) {

  //
  // all debug events that come from waitpid have to
  // go through this switch statement.
  //

  case EV_STOPPED:
    {
      //
      // set the FSM state of the focus thread to STOPPED
      // when a race condition occurs, it will simply print
      // out a warning message. But that kind of race condition
      // will be very very low
      //
      p.set_lwp_state (LMON_RM_STOPPED, use_context);
      get_tracer()->tracer_getregs ( p, use_context );
      T_VA pc = p.get_gprset(use_context)->get_pc();

      {
        self_trace_t::trace ( LEVELCHK(level3), 
	  MODULENAME, 0,
	  "converting [pc=0x%x] of tid=%d into an debug event.",
	  pc, p.get_cur_thread_ctx());
      }

      if ( p.get_never_trapped() )
        {
          //
          // This is the first trap ... easy
          //
	  return_ev = (p.get_myopts()->get_my_opt()->attach )
	    ? LM_STOP_AT_FIRST_ATTACH
	    : LM_STOP_AT_FIRST_EXEC;
        }
      else if ( p.get_please_detach()) 
	{
          //
          // This method gives priority to the detach request, thereby
          // eliminating race conditions that can occur: e.g., a BP stop event 
          // of a process/thread, (not a detach-stop event) after a detach request 
          // is considered to be LM_STOP_FOR_DETACH. It doesn't matter 
          // why p/t stopped once a detach request is made; the detach handler
          // should be invoked.  
          //
	  return_ev = LM_STOP_FOR_DETACH;
	}
      else if ( p.get_please_kill ())
	{
          //
          // This method gives priority to the detach request, thereby
          // eliminating race conditions that can occur: e.g., a BP stop event 
          // of a process/thread, (not a detach-stop event) after a kill request 
          // is considered to be LM_STOP_FOR_KILl. It doesn't matter 
          // why p/t stopped once a kill request is made; the kill handler
          // should be invoked.  
          //
	  return_ev = LM_STOP_FOR_KILL;
	}
      else if ( p.get_launch_hidden_bp() 
	        && ( p.get_launch_hidden_bp()->is_pc_part_of_bp_op(pc)))
	{
	  return_ev = LM_STOP_AT_LAUNCH_BP;
	}
      else if ( p.get_loader_hidden_bp() 
                && ( p.get_loader_hidden_bp()->is_pc_part_of_bp_op(pc)))
	{
	  return_ev = LM_STOP_AT_LOADER_BP;
	}
      else if ( event.get_signum () == SIGTRAP )
        {
	  //
	  // Parent gets SIGTRAP when a new thread is created
	  // Used to be: return_ev = LM_STOP_NOT_INTERESTED;
	  int upper16;
          upper16 = event.get_rawstatus() >> 16; 
	  if (upper16 == LINUX_TRACER_EVENT_CLONE) 
            {
              return_ev = LM_STOP_AT_THREAD_CREATION;
            }  
          else
            {
              //
              // SIGTRAP due to fork for example
              //
              return_ev = LM_STOP_NOT_INTERESTED;
            }
        }
      else if ( event.get_signum () == SIGSTOP ) 
        {
          return_ev = LM_RELAY_SIGNAL;

          if (p.get_thrlist().find( p.get_pid(use_context) )
                != p.get_thrlist().end()) 
            {
              if (!(p.get_thrlist()[p.get_pid(use_context)]->get_traced()))
                {
                  return_ev = LM_STOP_NEW_THREAD_TRACE;
                }
            }
          else
            {
              self_trace_t::trace (
                true, // print always 
                MODULENAME,
                0,
               "event decode requires a thread data structure already available!");
            }
        }
      else 
        {
          return_ev = LM_RELAY_SIGNAL;
        }
    }
    break;

  case EV_EXITED:
    //
    // set the FSM state of the focus thread to EXITED
    //
    p.set_lwp_state (LMON_RM_EXITED, use_context);
    return_ev = LM_EXITED;
    break;

  case EV_TERMINATED:
    //
    // set the FSM state of the focus thread to EXITED
    //
    p.set_lwp_state (LMON_RM_EXITED, use_context);
    return_ev = LM_EXITED;
    break; 

  default:
    return_ev = LM_INVALID;
    break;
  } 

  return return_ev;
}


////////////////////////////////////////////////////////////////////
//
// EVENT HANDLERS 
//
//

//! PUBLIC: handle_attach_event 
/*!
    handles an attach event.
*/
launchmon_rc_e 
linux_launchmon_t::handle_attach_event
( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  try 
    {
      get_tracer()->tracer_attach(p, false, -1);
      set_last_seen(gettimeofdayD());
      return LAUNCHMON_OK;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED; 
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
}


//! PUBLIC: handle_trap_after_attach_event
/*!
    handles a trap-after-attach event.
*/
launchmon_rc_e 
linux_launchmon_t::handle_trap_after_attach_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{ 
  try 
    {    
      using namespace std; 

#if MEASURE_TRACING_COST
      beginTS = gettimeofdayD ();
      {
        self_trace_t::trace ( 
          true, // print always 
          MODULENAME,
          0,
          "The RM process has just been trapped due to attach"); 
      }
#endif

      bool use_cxt = true;
      image_base_t<T_VA,elf_wrapper> *dynloader_im = NULL;
      image_base_t<T_VA,elf_wrapper> *main_im = NULL;  
      breakpoint_base_t<T_VA, T_IT> *lo_bp = NULL;
      breakpoint_base_t<T_VA, T_IT> *la_bp = NULL;
      int bdbg = 1;
      T_VA lpc = T_UNINIT_HEX;
      T_VA debug_state_flag = T_UNINIT_HEX;
      T_VA debug_state_addr = T_UNINIT_HEX;
      T_VA addr_dl_bp = T_UNINIT_HEX;

      {
	self_trace_t::trace ( 
          LEVELCHK(level2), 
	  MODULENAME,
          0,
	  " trap after attach event handler invoked. ");
      }

      main_im  = p.get_myimage();
      if ( !main_im )
	{
	  self_trace_t::trace ( 
            LEVELCHK(level1), 
            MODULENAME,
            1,
	    "main image is null.");
				
	  return LAUNCHMON_FAILED;
	}
      
      dynloader_im = p.get_mydynloader_image();
      if ( !dynloader_im )
      {
	self_trace_t::trace ( 
          LEVELCHK(level1), 
	  MODULENAME,
          1,
	  "dynamic loader image is null.");
			      
	return LAUNCHMON_FAILED;
      }      

      main_im->set_image_base_address(0);
      main_im->compute_reloc();

      //
      // handle_mpir_variables method requires main image has been relocated
      // (though this is meaningless almost all platforms
      handle_mpir_variables(p, *main_im); 

      lpc = get_va_from_procfs ( p.get_master_thread_pid(), 
				 dynloader_im->get_base_image_name() );

      if ( (lpc = get_auxv (p.get_master_thread_pid()) ) == T_UNINIT_HEX)
      {
	 lpc = get_va_from_procfs ( p.get_master_thread_pid(), 
		 dynloader_im->get_base_image_name() );
      }
      
      if ( lpc == T_UNINIT_HEX )
      {
	self_trace_t::trace ( 
          LEVELCHK(level1), 
	  MODULENAME,
          1,
	  "can't resolve the base address of the loader.");
			      
	return LAUNCHMON_FAILED;
      }	
      
      dynloader_im->set_image_base_address(lpc);
      dynloader_im->compute_reloc();

      // registering p.loader_hidden_bp. 
      // 
      const symbol_base_t<T_VA>& dynload_sym 
	    = dynloader_im->get_a_symbol (p.get_loader_breakpoint_sym());   

      lo_bp = new linux_breakpoint_t();
      addr_dl_bp = dynload_sym.get_relocated_address();
      lo_bp->set_address_at ( addr_dl_bp );
#if PPC_ARCHITECTURE
      lo_bp->set_use_indirection();
#endif
      lo_bp->set();

      p.set_loader_hidden_bp(lo_bp);
      get_tracer()->enable_breakpoint (p, 
				       (*p.get_loader_hidden_bp()),
				       use_cxt );

      check_dependent_SOs(p);
      p.set_never_trapped(false); 

      if (p.rmgr()->is_coloc_sup())
        {
          if (p.rmgr()->is_attfifo_sup())
            {
              if (p.get_sym_attach_fifo())
                {
                  //
                  // MPIR_attach_fifo support is only applicable to attach mode.
                  // 
                  char fifopathbuf[256]; 
                  T_VA fifo_addr = p.get_sym_attach_fifo()->get_relocated_address();
                  get_tracer()->tracer_read_string(p, 
                                               fifo_addr,
				               (void*) fifopathbuf,
				               256,
				               use_cxt);
                  std::string fip = fifopathbuf;
                  p.rmgr()->set_attach_fifo_path(fip);

	          //
	          // We have to continue the target process before starting FIFO
	          // otherwise open on the FIFO will block
	          //
	          get_tracer()->tracer_continue (p, use_cxt);
                  int fifofd = 0;
                  if ( (fifofd = open(fifopathbuf, O_WRONLY)) >= 0)
                    {
                      char wakeup = (char) 1; 
                      if ( lmon_write_raw(fifofd, &wakeup, 1) != 1 )
                        {
                          self_trace_t::trace(
                            true,  
                            MODULENAME,
                            0,
                            "lmon_write_raw returned a value that is not equal to 1");
                        }
                      close(fifofd);
                    }
                  else
                    {
                      self_trace_t::trace(
                        true,  
                        MODULENAME,
                        0,
                        "Open a FIFO (%s) failed, errno(%d)", fifopathbuf, errno);
                    } 
                } 
              else
                {
                  self_trace_t::trace(
                    true,  
                    MODULENAME,
                    0,
                    "MPIR_attach_fifo symbol not found");
                } 
            } 
          else
            {
	      get_tracer()->tracer_continue (p, use_cxt);
            }
        } // With MPIR Colocation service
      else
        {
           //
           // Without MPIR Colocation service, you would have
           // Proctable available on attach and you would be 
           // ready to launch daemon at this point
           // 
           // 
           acquire_proctable ( p, use_cxt );
           ship_proctab_msg ( lmonp_proctable_avail );
           ship_resourcehandle_msg ( lmonp_resourcehandle_avail, get_resid() );
	   ship_rminfo_msg ( lmonp_rminfo,
			     (int) p.get_pid(false),
			      p.rmgr()->get_resource_manager().get_rm());
           say_fetofe_msg ( lmonp_stop_at_first_attach );
           launch_tool_daemons(p);
	   get_tracer()->tracer_continue (p, use_cxt);
        }

      {
	self_trace_t::trace ( 
          LEVELCHK(level2), 
	  MODULENAME,
          0,
	  "trap after attach event handler completed.");
      }
  
#if MEASURE_TRACING_COST
      endTS = gettimeofdayD ();
      accum += endTS - beginTS;
      countHandler++;
      // accum and countHandler now contain the cost of this handler which 
      // is invoked just once per job
      {
        self_trace_t::trace ( 
          true, // print always 
          MODULENAME, 
          0,
          "Just continued the RM process out of the first trap"); 
      }
#endif

      set_last_seen (gettimeofdayD ());

      return LAUNCHMON_OK;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    } 
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
}


//! PUBLIC: handle_trap_after_exec_event
/*!
    handles the first fork/exec-trap event.  
*/
launchmon_rc_e 
linux_launchmon_t::handle_trap_after_exec_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  try 
    {
      using namespace std;

#if MEASURE_TRACING_COST
     beginTS = gettimeofdayD ();
     {
       self_trace_t::trace ( 
         true, // print always 
         MODULENAME,
         0,
         "The RM process has just been forked and exec'ed.");
     }
#endif

      bool use_cxt = true;
      image_base_t<T_VA,elf_wrapper> *main_im, *dynloader_im;
      breakpoint_base_t<T_VA, T_IT> *lo_bp;
      T_VA addr_dl_start, dl_linked_addr, addr_dl_bp;

      {
	self_trace_t::trace ( LEVELCHK(level2), 
          MODULENAME,0,
	  "trap after first exec event handler invoked.");
      }

      main_im  = p.get_myimage();
      if ( !main_im )
	{
	  self_trace_t::trace ( LEVELCHK(level1), 
	    MODULENAME,1,
	    "main image is null.");
				
	  return LAUNCHMON_FAILED;
	}
      
      dynloader_im = p.get_mydynloader_image();
      if ( !dynloader_im )
      {
	self_trace_t::trace ( LEVELCHK(level1), 
	  MODULENAME,1,
	  "dynamic loader image is null.");
			      
	return LAUNCHMON_FAILED;
      }    

      main_im->set_image_base_address(0);
      main_im->compute_reloc();

      //
      // handle_mpir_variables method requires main image has been relocated
      // (though this is meaningless almost all platforms
      handle_mpir_variables(p, *main_im); 

      //
      // computing where the dynamic loader was linked and 
      // registering p.loader_hidden_bp 
      const symbol_base_t<T_VA>& dynload_sym 
	= dynloader_im->get_a_symbol (p.get_loader_breakpoint_sym());  

      if ( (dl_linked_addr = get_auxv (p.get_pid (true)) ) == T_UNINIT_HEX)
        {
	  //
	  // Corner case; we deal with a heuristics
	  //
#if PPC_ARCHITECTURE
          //
          // DHA Mar 05 2009
	  // There're systems that do not directly 
	  // export function symbols, and the original logic 
	  // to determining the base linked address of the runtime 
	  // linker maps has some problems. 
	  // Specifically, one must know the relative function address of 
	  // _start to compute the linked addrss, but the system 
	  // that only allows indirection to get this function address 
	  // won't provide that without
	  // having to collect that info from the memory. Now,
	  // collecting this from the mem isn't possible 
	  // for symbols within the runtime linker until
	  // the base mapped address for the linker is determined.
	  // Egg-and-Chicken problem. 
	  //
	  // In such a system, we use a hack assuming the runtime
	  // linker is mapped to an address aligned in some multiple pages, 
	  // which insn't too outragous to assume. 
	  //
	  
	  dl_linked_addr
# if BIT64
	    =  p.get_gprset(use_cxt)->get_pc() & 0xffffffffff0000;
# else
	    =  p.get_gprset(use_cxt)->get_pc() & 0xffff0000;
# endif
#else /* PPC_ARCHITECTURE */
          //
          // This requires the actual page size to compute this loader load
          // address implictly. Just using the following bits for now.  
          //
	  dl_linked_addr
#if BIT64
	    =  p.get_gprset(use_cxt)->get_pc() & 0xfffffffffff000;
#else
	    =  p.get_gprset(use_cxt)->get_pc() & 0xfffff000;
#endif
#endif
        }
  
      dynloader_im->set_image_base_address(dl_linked_addr);
      dynloader_im->compute_reloc();

      lo_bp = new linux_breakpoint_t();
      addr_dl_bp = dynload_sym.get_relocated_address();
      lo_bp->set_address_at ( addr_dl_bp );
      
#if PPC_ARCHITECTURE
      lo_bp->set_use_indirection();
      T_VA adjusted_indirect_addr; 
      //
      // This is A special case because the loader hasn't had
      // a chance even to relocate its own symbols... 
      //
      get_tracer()->tracer_read (p,
	addr_dl_bp,
	&adjusted_indirect_addr,
	sizeof(T_VA),
	use_cxt);

      lo_bp->set_indirect_address_at(adjusted_indirect_addr + dl_linked_addr);
#endif
      lo_bp->set();

      p.set_loader_hidden_bp(lo_bp);
      get_tracer()->enable_breakpoint(p,
				      (*p.get_loader_hidden_bp()),
				      use_cxt );

      // now this process has get past the first fork/exec
      //
      p.set_never_trapped ( false );

      get_tracer()->tracer_setoptions (p, use_cxt, -1);

      get_tracer()->tracer_continue (p, use_cxt);
 
      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0,
	  "trap after first exec event handler completed.");
      }

#if MEASURE_TRACING_COST
      endTS = gettimeofdayD ();
      accum += endTS - beginTS;
      countHandler++;
      // accum and countHandler now contain the cost of this handler which 
      // is invoked just once per job
      {
        self_trace_t::trace ( true, // print always 
          MODULENAME,0,
          "Just continued the RM process out of the first trap"); 
      }
#endif

      set_last_seen (gettimeofdayD ());
      return LAUNCHMON_OK;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      abort();
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
}


//! PUBLIC: handle_launch_bp_event
/*!
    The event handler for a launch breakpoint hit event.
    Most RM MPIR APAI implementations use MPIR_Breakpoint for this.
*/
launchmon_rc_e 
linux_launchmon_t::handle_launch_bp_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  try 
    {
      using namespace std;
#if MEASURE_TRACING_COST
      beginTS = gettimeofdayD ();
#endif
      launchmon_rc_e lrc = LAUNCHMON_OK;
      bool use_cxt = true;
      image_base_t<T_VA,elf_wrapper> *main_im;
      T_VA debug_state_addr;
      int bdbg, bdbgp;
  
      if (!(is_bp_prologue_done(p, p.get_launch_hidden_bp()))) {
#if MEASURE_TRACING_COST
        endTS = gettimeofdayD ();
        accum += endTS - beginTS;
	countHandler++;
#endif
	return LAUNCHMON_OK;
      }

#if MEASURE_TRACING_COST
      {
        self_trace_t::trace ( 
          true, /* print always */
	  MODULENAME,
          0,
	  "launch-breakpoint hit event handler invoked.");
      }
#endif
      {
        self_trace_t::trace ( 
          LEVELCHK(level2),
	  MODULENAME,
          0,
	  "launch-breakpoint hit event handler invoked.");
      }

      main_im  = p.get_myimage();  
      if (!main_im)
        {
          self_trace_t::trace ( true, /* print always */
	    MODULENAME,0,
	    "main image has not been processed.");

          return LAUNCHMON_FAILED;
        }

      //
      // looking up MPIR_debug_state
      //
      symbol_base_t<T_VA> debug_state_var 
	    = main_im->get_a_symbol (p.get_launch_debug_state());
      if (!debug_state_var && p.get_myrmso_image())
        {
          debug_state_var 
            = p.get_myrmso_image()->get_a_symbol(p.get_launch_debug_state());
        }

      debug_state_addr = debug_state_var.get_relocated_address(); 
      get_tracer()->tracer_read ( p, 
				  debug_state_addr,
				  &bdbg,
				  sizeof(bdbg),
				  use_cxt );
      symbol_base_t<T_VA> debug_state
            = main_im->get_a_symbol (p.get_launch_being_debug());
      if (!debug_state && p.get_myrmso_image())
        {
          debug_state
            = p.get_myrmso_image()->get_a_symbol(p.get_launch_being_debug());
        }

#if MEASURE_TRACING_COST
      endTS = gettimeofdayD ();
      accum += endTS - beginTS;
      countHandler++;
#endif
      
      //
      // MPIR_debug_state 
      //
      //
      if (!validate_mpir_state_transition(bdbg))
        {
	  self_trace_t::trace ( 
            true, 
	    MODULENAME,0,
	    "MPIR state transition is invalid! continue...");

          //
          // if state transition isn't valid, simply continue
          //
          get_tracer()->tracer_continue (p, use_cxt);
        }
      else
        {
          switch ( bdbg )
	    {
	    case MPIR_DEBUG_SPAWNED:
              {
	        //
	        // Apparently, MPI tasks have just been spawned.
	        //   We want to acquire RPDTAB and the resource ID, 
	        //   and to pass those along to the FE client. 
	        //   Subsequently, we want to launch the specified tool 
                //   daemons before let go of the RM process.
	        //
	        {
	          self_trace_t::trace ( 
                    LEVELCHK(level2), 
		    MODULENAME,0,
	            "launch-breakpoint hit event handler completing with MPIR_DEBUG_SPAWNED");
	        }
	        acquire_proctable ( p, use_cxt );
	        ship_proctab_msg ( lmonp_proctable_avail );
	        ship_resourcehandle_msg ( lmonp_resourcehandle_avail, get_resid() );
	        ship_rminfo_msg ( lmonp_rminfo, 
		  	          (int) p.get_pid(false), 
			          p.rmgr()->get_resource_manager().get_rm());
	        say_fetofe_msg ( lmonp_stop_at_launch_bp_spawned );

    	        launch_tool_daemons(p);

                set_engine_state(bdbg);

                if (get_API_mode())
                  {
                    p.get_thrlist()[p.get_cur_thread_ctx()]->set_event_registered ( true );
                    // shouldn't continue this eventing thread 
                    // if the engine is drived via API 
                    // until all set up is done and FEN sends the "unlock" command.
                  } 
                else
                  { 
	            get_tracer()->tracer_continue (p, use_cxt);
                  }
	        break;
              }
	    case MPIR_DEBUG_ABORTING:
              {
	        //
	        // Apparently, MPI tasks have just been aborted, 
                //   either normally or abnormally.
                //   We want to pass this along to the FE client, 
	        //   to notify the RM launcher of the upcoming detach via 
                //   the MPIR_being_debugged, to disinsert all breakpoints, and to 
	        //   actually issue a detach command to the RM process.
	        //
                int bdbg = 0;
                T_VA debug_state_flag = T_UNINIT_HEX;

	        {
	          self_trace_t::trace ( LEVELCHK(level2), 
		    MODULENAME,0,
	            "launch-breakpoint hit event handler completing with MPIR_DEBUG_ABORTING");
                }

                //
                // disable all the hidden breakpoints. Since we don't     
                // know if the context is slave or main thread, we have to 
                // pass true 
                disable_all_BPs (p, true);

                //
                // unsetting "MPIR_being_debugged."
                //
                //const symbol_base_t<T_VA>& being_debugged
                //  = p.get_myimage()->get_a_symbol (p.get_launch_being_debug());

                get_tracer()->tracer_write ( p,
                                  debug_state.get_relocated_address(),
                                  &bdbg,
                                  sizeof(bdbg),
                                  true );

                //
                // detach from all slave threads.
                //
                for ( p.thr_iter = p.get_thrlist().begin();
                        p.thr_iter != p.get_thrlist().end(); p.thr_iter++ )
                  {
                    if ( !(p.thr_iter->second->is_master_thread()) )
                      {
                        // operates on a slave thread
                        p.make_context ( p.thr_iter->first );
                        if (get_tracer()->status(p, true) == SDBG_TRACE_STOPPED)
                          {
                            get_tracer()->tracer_detach ( p, true );
                          }
                        p.check_and_undo_context ( p.thr_iter->first );
                      }
                  }

                self_trace_t::trace ( 
                  LEVELCHK(level1),
                  MODULENAME,
                  0,
                  "detached from all threads of the RM process...");

                //
                // detach from the main thread
                //
                if (get_tracer()->status(p, false) == SDBG_TRACE_STOPPED)
                  {
                    get_tracer()->tracer_detach ( p, false );
                  }

                self_trace_t::trace ( 
                  LEVELCHK(level1),
                  MODULENAME,
                  0,
                  "detached from all the RM process...");

                set_engine_state(bdbg);
                say_fetofe_msg(lmonp_stop_at_launch_bp_abort);

 	        //
 	        // this return code will cause the engine to exit.
 	        // but it should leave its children RM_daemon process
 	        // in a running state.
 	        //
 	        lrc = LAUNCHMON_MPIR_DEBUG_ABORT;

	        {
	          self_trace_t::trace ( 
                    LEVELCHK(level2),
		    MODULENAME,0,
	            "launch-breakpoint hit event handler "
                    "completing with MPIR_DEBUG_ABORTING");
	        }
	        break; 
	      }
            case MPIR_NULL:
              {
	        {
	          self_trace_t::trace ( 
                    LEVELCHK(level2), 
		    MODULENAME,0,
	            "launch-breakpoint hit event handler "
                    "completing with MPIR_NULL");
	        }
                set_engine_state(bdbg);
	        get_tracer()->tracer_continue (p, use_cxt);
                break;
              }
	    default:
	      {
	        {
	          self_trace_t::trace ( LEVELCHK(level2), 
		    MODULENAME,0,
	            "launch-breakpoint hit event handler "
                    "completing with unknown debug state");
	        }
                set_engine_state(bdbg);
	        get_tracer()->tracer_continue (p, use_cxt);
	        break;
	      }
	    }
        }

      set_last_seen (gettimeofdayD ());

      return lrc;
  }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    } 
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
}


//! PUBLIC: handle_detach_cmd_event 
/*!
    handles "detach-command" event.  
*/
launchmon_rc_e 
linux_launchmon_t::handle_detach_cmd_event
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  try
    {
      int bdbg = 0;
      T_VA debug_state_flag = T_UNINIT_HEX;

      //
      // disable all the hidden breakpoints	
      //
      disable_all_BPs (p, false);      

      //
      // detach from all slave threads.
      //
      for ( p.thr_iter = p.get_thrlist().begin(); 
	      p.thr_iter != p.get_thrlist().end(); p.thr_iter++ )
        {
	  if ( !(p.thr_iter->second->is_master_thread()) ) 
            {
              // operates on a slave thread
              p.make_context ( p.thr_iter->first );
              get_tracer()->tracer_unsetoptions ( p, true, -1 );
              get_tracer()->tracer_detach ( p, true ); 
              p.check_and_undo_context ( p.thr_iter->first );
	    }
	}

       self_trace_t::trace (  LEVELCHK(level1),
         MODULENAME,
         0,
         "detached from all RM threads...");

      //
      // unsetting "MPIR_being_debugged."
      //
      symbol_base_t<T_VA> being_debugged
        = p.get_myimage()->get_a_symbol (p.get_launch_being_debug());
      if (!being_debugged && p.get_myrmso_image())
        {
          being_debugged
            = p.get_myrmso_image()->get_a_symbol(p.get_launch_being_debug());
        }


      get_tracer()->tracer_write ( p,
                                   being_debugged.get_relocated_address(),
                                   &bdbg,
                                   sizeof(bdbg),
                                   false );

      //
      // detach from the main thread
      //
      // calls detach twice in case there are
      // multiple stop event queued up into waitpid
      get_tracer()->tracer_unsetoptions ( p, false, -1 );
      get_tracer()->tracer_detach ( p, false );

       self_trace_t::trace ( LEVELCHK(level1),
         MODULENAME,
         0,
         "detached from the RM process ...");

      char *bnbuf = strdup(p.get_myopts()->get_my_opt()->debugtarget.c_str());
      std::string dt = basename(bnbuf);

      switch ( p.get_reason() )
        {
        //
        // say the job-done to the FE, if I can
        //
        case RM_BE_daemon_exited:
          say_fetofe_msg ( lmonp_bedmon_exited );
          break;
        case RM_MW_daemon_exited:
          say_fetofe_msg ( lmonp_mwdmon_exited );
          break;
	case RM_JOB_mpir_aborting:
	  say_fetofe_msg ( lmonp_stop_at_launch_bp_abort );
	  break;
        case FE_requested_detach:
          say_fetofe_msg ( lmonp_detach_done );	
          break;
        case FE_requested_shutdown_dmon:
          p.rmgr()->graceful_rmkill(get_toollauncherpid());
          say_fetofe_msg ( lmonp_detach_done );	
          break;
        case FE_disconnected:
	  usleep (GracePeriodFEDisconnection);
          p.rmgr()->graceful_rmkill(get_toollauncherpid());
          break;
        case ENGINE_dying_wsignal:
          say_fetofe_msg (lmonp_stop_tracing);
          break;
        case reserved_for_rent:
        case FE_requested_kill:
          {
	    self_trace_t::trace ( LEVELCHK(level1), 
			          MODULENAME,1,
	    "Reason for the detach is unclear "
            "(reserved_for_rent or FE_requested_kill)");
          }
          break;
        default:
          {
	    self_trace_t::trace ( LEVELCHK(level1), 
			          MODULENAME,1,
	    "Reason for the detach is unclear");
          }
          break;
        }

      free(bnbuf);
      free_engine_resources(p);

      //
      // This return code will cause the engine to exit.
      // but it should leave its children RM_daemon process
      // in a running state.
      //
      set_last_seen (gettimeofdayD ());

      return LAUNCHMON_STOP_TRACE;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    } 
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    } 
}


//! PUBLIC: handle_kill_cmd_event 
/*!
    handles "kill-command event." This is an event initiated by 
    the FE client as opposed to an event generated from the 
    RM launcher process.
*/
launchmon_rc_e 
linux_launchmon_t::handle_kill_cmd_event 
                 ( process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  try
    {   
      int bdbg = 0;
      T_VA debug_state_flag = T_UNINIT_HEX;
 
      //
      // disinserting all the breakpoints
      //
      disable_all_BPs (p, false);
 
      //
      // detach from all the slave threads.
      //
      for ( p.thr_iter = p.get_thrlist().begin();
              p.thr_iter != p.get_thrlist().end(); p.thr_iter++ )
        {
          if ( !(p.thr_iter->second->is_master_thread()) )
            {
              // operates on a slave thread
              p.make_context ( p.thr_iter->first );
              get_tracer()->tracer_unsetoptions ( p, true, -1 );
              get_tracer()->tracer_detach( p, true );
              p.check_and_undo_context ( p.thr_iter->first );
            }
        }

       self_trace_t::trace ( LEVELCHK(level1),
         MODULENAME,
         0,
         "detached from all RM threads...");
      //
      // unsetting MPIR_being_debugged.
      //
      symbol_base_t<T_VA> debug_state
        = p.get_myimage()->get_a_symbol (p.get_launch_being_debug());

      if (!debug_state && p.get_myrmso_image())
        {
          debug_state
            = p.get_myrmso_image()->get_a_symbol(p.get_launch_being_debug());
        }
 
      debug_state_flag = debug_state.get_relocated_address();
      get_tracer()->tracer_write ( p,
        debug_state_flag,
        &bdbg,
        sizeof(bdbg),
        false );

      //
      // detach from all the main thread
      //
      get_tracer()->tracer_unsetoptions ( p, false, -1 );
      get_tracer()->tracer_detach ( p, false );
     
      {
         self_trace_t::trace ( 
           LEVELCHK(level1),
           MODULENAME,
           0,
           "detached from the RM process ...");
      }

      //
      // kill both the target RM and tool RM gracefully
      //
      p.rmgr()->graceful_rmkill(p.get_pid(false));	
      if (!p.rmgr()->is_coloc_sup())
        {
          int pid = get_toollauncherpid();
          p.rmgr()->graceful_rmkill(pid);
        }

      switch (p.get_reason())
        {
        //
        // say the job-done to the FE, if I can
        //
        case RM_BE_daemon_exited:
	case RM_MW_daemon_exited:
	case FE_disconnected:
	case FE_requested_shutdown_dmon:
	case FE_requested_detach:
        case ENGINE_dying_wsignal:
          {
	    self_trace_t::trace ( LEVELCHK(level1), 
			          MODULENAME,1,
	    "RM_BE_daemon_exited or "
            "its equivalents should not kill the job!");
          }
          break;
        case FE_requested_kill:
          say_fetofe_msg ( lmonp_kill_done );	
          break;
        case reserved_for_rent:
          {
	    self_trace_t::trace ( LEVELCHK(level1), 
			          MODULENAME,1,
	    "Reason for the kill is unclear "
            "(reserved_for_rent!)");
          }
          break;
        default:
          {
	    self_trace_t::trace ( LEVELCHK(level1), 
			          MODULENAME,1,
	    "Reason for the kill is unclear (default!)");
          }
          break;
        }

      free_engine_resources(p);

      //
      // this return code will cause the engine to exit.
      // daemon should have gotten a kill command if supported
      // at this point. 
      //
      set_last_seen (gettimeofdayD ());

      return LAUNCHMON_STOP_TRACE;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    } 
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    } 
}



//! PUBLIC: handle_loader_bp_event
/*!
    handles the loader event. Whenever the loader notifies 
    the debugger its DSO load/unload, this handler gets 
    invoked, fetching some information (e.g. the base link map for
    the pthread library.) Once it gleans all necessary info, it 
    stops poking the link map to optimize the perf. 
*/
launchmon_rc_e 
linux_launchmon_t::handle_loader_bp_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  try 
    {
      using namespace std;

#if MEASURE_TRACING_COST
     beginTS = gettimeofdayD ();
#endif

      bool use_cxt = true;

      if (!(is_bp_prologue_done(p, p.get_loader_hidden_bp()))) {
#if MEASURE_TRACING_COST
        endTS = gettimeofdayD ();
        accum += endTS - beginTS;
	countHandler++;
#endif
	return LAUNCHMON_OK;
      }

      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0,
	  "loader event handler invoked.");
      }

      check_dependent_SOs(p);

      get_tracer()->tracer_continue (p, use_cxt);

      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0,
	  "loader event handler completed.");
      }

#if MEASURE_TRACING_COST
      endTS = gettimeofdayD ();
      accum += endTS - beginTS;
      countHandler++;
#endif 

      set_last_seen (gettimeofdayD ());

      return LAUNCHMON_OK;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }  
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
}


//! PUBLIC: handle_fork_event 
/*!
    handle a process fork event
*/
launchmon_rc_e 
linux_launchmon_t::handle_newproc_forked_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  try 
    {
      using namespace std;

#if MEASURE_TRACING_COST
      beginTS = gettimeofdayD ();
#endif

      bool use_cxt = true; 
  
      {
	self_trace_t::trace ( LEVELCHK(level2), 
			      MODULENAME,0,
			      "newproc forked event handler invoked.");
      }

      //
      // Because the context would have been set up for the newly
      // forked process, following process control actions will
      // only affect the new process 
      //
      disable_all_BPs (p, use_cxt, false);  
      get_tracer()->tracer_detach(p, use_cxt);

      {
	self_trace_t::trace ( LEVELCHK(level2), 
			      MODULENAME,0,
			      "fork event handler completed.");
      }    

#if MEASURE_TRACING_COST
      endTS = gettimeofdayD ();
      accum += endTS - beginTS;
      countHandler++;
#endif

      set_last_seen (gettimeofdayD ());
      return LAUNCHMON_OK;
  }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }     
}


//! PUBLIC: handle_exit_event 
/*!
    handle an exit event
*/
launchmon_rc_e 
linux_launchmon_t::handle_exit_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{  
  try 
    {
      using namespace std;

      launchmon_rc_e rc;

#if MEASURE_TRACING_COST
      beginTS = gettimeofdayD ();
#endif
 
      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0,
          "thread exit event handler invoked.");
      }

      if (p.get_cur_thread_ctx () != p.get_master_thread_pid()) 
	{
          delete p.get_thrlist()[p.get_cur_thread_ctx()];
	  p.get_thrlist().erase(p.get_cur_thread_ctx());
	  {
	    self_trace_t::trace ( LEVELCHK(level2), 
	      MODULENAME,0,
	      "a slave thread has exited.");
	  }

	  //cout << "slave thread exited: " << p.get_cur_thread_ctx() << endl;
	  rc = LAUNCHMON_OK;
	}
      else 
	{    

	  {
	    self_trace_t::trace ( LEVELCHK(level1), 
	      MODULENAME,0,
	      "a main thread has exited.");
	  }

	  //cout << "main thread exited: " << p.get_cur_thread_ctx() << endl;
	  //
	  // LMON API SUPPORT: a message via a pipe to the watchdog thread
	  // 
	  //
	  say_fetofe_msg(lmonp_exited);	 

          free_engine_resources(p);

          //
          // this return code will cause the engine to exit.
          // daemon should continue running: Enforcing D.1
          // error handling semantics.
          //
	  rc = LAUNCHMON_MAINPROG_EXITED;
	}    

      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0,
          "thread exit event handler completed.");
      }

#if MEASURE_TRACING_COST
      endTS = gettimeofdayD ();
      accum += endTS - beginTS;
      countHandler++;
#endif

      set_last_seen (gettimeofdayD ());
      return rc;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }     
}


//! PUBLIC: handle_term_event 
/*!
    handle a termination event
*/
launchmon_rc_e 
linux_launchmon_t::handle_term_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  using namespace std;

#if MEASURE_TRACING_COST
  beginTS = gettimeofdayD ();
#endif

  {
    self_trace_t::trace ( LEVELCHK(level2), 
      MODULENAME,0,
      "termination event handler invoked.");
  }

  say_fetofe_msg(lmonp_terminated);

  {
    self_trace_t::trace ( LEVELCHK(level2), 
      MODULENAME,0,
      "termination event handler completed.");
  }
	
  cout << "main thread terminated: " << p.get_cur_thread_ctx() << endl;

#if MEASURE_TRACING_COST
  endTS = gettimeofdayD ();
  accum += endTS - beginTS;
  countHandler++;
#endif

  set_last_seen( gettimeofdayD() );

  return LAUNCHMON_OK;
}


//! PUBLIC: handle_thrcreate_request
/*!
    handle a thread-creation event 
*/
launchmon_rc_e 
linux_launchmon_t::handle_thrcreate_request ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p, int newlwpid )
{
  try 
    {
      using namespace std;
#if MEASURE_TRACING_COST
      beginTS = gettimeofdayD (); 
#endif

      my_thrinfo_t tinfo;
      memset(&tinfo, '\0', sizeof(tinfo));
      tinfo.ti_lid = (lwpid_t) newlwpid;

      if ( p.get_thrlist().find( tinfo.ti_lid )
           == p.get_thrlist().end() )
        {
          // this thread has not been seen
#if X86_ARCHITECTURE || X86_64_ARCHITECTURE
          thread_base_t<SDBG_LINUX_DFLT_INSTANTIATION> *thrinfo
            = new linux_x86_thread_t();
#elif PPC_ARCHITECTURE
          thread_base_t<SDBG_LINUX_DFLT_INSTANTIATION> *thrinfo
            = new linux_ppc_thread_t();
#endif
          thrinfo->copy_thread_info(tinfo);
          // if the process doesn't contain tid of the given thread,
          // those threads must be the new threads
          p.get_thrlist().insert (make_pair (tinfo.ti_lid, thrinfo) );
        }

       set_last_seen( gettimeofdayD() );

       return LAUNCHMON_OK;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }     
}


//! PUBLIC: handle_thrcreate_trap_event
/*!
    handle a thread-creation event 
*/
launchmon_rc_e 
linux_launchmon_t::handle_thrcreate_trap_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  try 
    {
      using namespace std;
#if MEASURE_TRACING_COST
      beginTS = gettimeofdayD (); 
#endif

      bool use_cxt = true;
      T_WT newlwpid = -1;

      get_tracer()->tracer_get_event_msg( p, 
                                          0, 
                                          (void *) &newlwpid, 
                                          true);

      if (newlwpid < 0) 
        {
          self_trace_t::trace (true, 
          MODULENAME,1,
          "negative thread id has been returned! Ignoring.");
        }

      //cout << "<" << newlwpid << ">" << endl;

      handle_thrcreate_request(p, (int) newlwpid);

      get_tracer()->tracer_continue(p, use_cxt); 
      set_last_seen( gettimeofdayD() );

      return LAUNCHMON_OK;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }     
}


//! PUBLIC: handle_newthread_trace_event
/*!
    handle a thread-creation event 
*/
launchmon_rc_e 
linux_launchmon_t::handle_newthread_trace_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  try 
    {
      using namespace std;
#if MEASURE_TRACING_COST
     beginTS = gettimeofdayD ();
#endif

      bool use_cxt = true;

      {
        self_trace_t::trace ( LEVELCHK(level2),
          MODULENAME,0,
          "irrelevant stop event handler invoked");
      }

      get_tracer()->tracer_setoptions (p, use_cxt, -1);
      p.get_thrlist()[p.get_cur_thread_ctx()]->set_traced(true);
      //cout << "[" << p.get_cur_thread_ctx() << "]" << endl;
      get_tracer()->tracer_continue (p, use_cxt);

      {
        self_trace_t::trace ( LEVELCHK(level2),
          MODULENAME,0,
          "irrelevant stop event handler completed");
      }

#if MEASURE_TRACING_COST
      endTS = gettimeofdayD ();
      accum += endTS - beginTS;
      countHandler++;
#endif

      set_last_seen (gettimeofdayD ());

      return LAUNCHMON_OK;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }     
}


//! PUBLIC: handle_not_interested_event
/*!
    If the target stopped with no apprent reason, 
    we'd better simply kick it again.
*/
launchmon_rc_e 
linux_launchmon_t::handle_not_interested_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p ) 
{
  try 
    {
      using namespace std;

#if MEASURE_TRACING_COST
     beginTS = gettimeofdayD (); 
#endif

      bool use_cxt = true;  

      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0,
	  "irrelevant stop event handler invoked");
      }

      get_tracer()->tracer_continue (p, use_cxt);

      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0,
	  "irrelevant stop event handler completed");
      }

#if MEASURE_TRACING_COST
      endTS = gettimeofdayD ();
      accum += endTS - beginTS;
      countHandler++;
#endif     

     set_last_seen (gettimeofdayD ());

     return LAUNCHMON_OK;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
}


//! PUBLIC: handle_relay_signal_event
/*!
    If the target stopped with no apprent reason, 
    we'd better simply kick it again.
*/
launchmon_rc_e 
linux_launchmon_t::handle_relay_signal_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p, int sig) 
{
  try 
    {
      using namespace std;
      int what_to_send = SIGCONT;
#if MEASURE_TRACING_COST
     beginTS = gettimeofdayD (); 
#endif

      bool use_cxt = true;  

      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0,
	  "irrelevant stop event handler invoked");
      }

      if ( sig != SIGSTOP )
	what_to_send = sig;

      get_tracer()->tracer_setoptions (p, true, -1);
      get_tracer()->tracer_deliver_signal (p, what_to_send, use_cxt);

      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0,
	  "irrelevant stop event handler completed");
      }

#if MEASURE_TRACING_COST
      endTS = gettimeofdayD ();
      accum += endTS - beginTS;
      countHandler++;
#endif

     set_last_seen (gettimeofdayD ());
     return LAUNCHMON_OK;
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      return LAUNCHMON_FAILED;
    }
}

