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

#if RM_ALPS_APRUN
extern "C" {
#include "apInfo.h"
#include "libalps.h"
extern char *alpsGetMyNid(int *nid);
}
#endif

#include <cstring>
#include <string>

#include "sdbg_self_trace.hxx"
#include "sdbg_base_symtab.hxx" 
#include "sdbg_base_symtab_impl.hxx"
#include "sdbg_base_mach.hxx"
#include "sdbg_base_mach_impl.hxx"
#include "sdbg_base_tracer.hxx"
#include "sdbg_base_ttracer.hxx"
#include "sdbg_base_launchmon.hxx"
#include "sdbg_base_launchmon_impl.hxx"
#include "sdbg_opt.hxx"

#include "sdbg_linux_std.hxx"
#include "sdbg_linux_bp.hxx"
#include "sdbg_linux_mach.hxx"
#include "sdbg_linux_launchmon.hxx"
#include "sdbg_linux_ptracer.hxx"
#include "sdbg_linux_ptracer_impl.hxx"
#include "sdbg_linux_ttracer.hxx"
#include "sdbg_linux_ttracer_impl.hxx"

#if MEASURE_TRACING_COST
static double accum = 0.0;
static int countHandler = 0;
static double beginTS;
static double endTS;
#endif

////////////////////////////////////////////////////////////////////
//
// static functions
//
//

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

  ret_pc = (auxvBuf.a_type == AT_BASE)? auxvBuf.a_un.a_val : T_UNINIT_HEX;

  return ret_pc;
}
////////////////////////////////////////////////////////////////////
//
// PRIVATE METHODS (class linux_launchmon_t<>)
//
////////////////////////////////////////////////////////////////////

launchmon_rc_e
linux_launchmon_t::init_API(opts_args_t *opt)
{ 
  char *tokenize = NULL;
  char *FEip = NULL;
  char *FEport = NULL;
  int clientsockfd;
  struct sockaddr_in servaddr;

  if (!opt->get_my_opt()->remote) 
    {
      //
      // if this isn't API mode, apparently you don't have to 
      // do anything.
      //
      return LAUNCHMON_OK;
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
      return LAUNCHMON_FAILED;
    }

  free(tokenize);

  if ( ( clientsockfd = socket ( AF_INET, SOCK_STREAM, 0 )) < 0 )
    {
      self_trace_t::trace ( LEVELCHK(level1),
        MODULENAME,1,
        "socket failed in the engine init handler.");
        return LAUNCHMON_FAILED;
    }

  int optval = 1;
  int optlen = sizeof(optval);
  if( setsockopt(clientsockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0 )
    {
      self_trace_t::trace ( LEVELCHK(level1),
        MODULENAME,1,
        "setting socket keepalive failed.");
      return LAUNCHMON_FAILED;
    }

  if ( ( connect ( clientsockfd, 
	 	   (struct sockaddr *)&servaddr,
                   sizeof(servaddr) )) < 0 )
    {
      self_trace_t::trace ( LEVELCHK(level1),
        MODULENAME,1,
        "connect failed in PLST init handler.");
      return LAUNCHMON_FAILED;
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

  return LAUNCHMON_OK;
}


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class linux_launchmon_t<>)
//
///////////////////////////////////////////////////////////////////

//!  PUBLIC: 
/*! linux_launchmon_t<> constructors & destructor


*/
linux_launchmon_t::linux_launchmon_t () 
  : continue_method(normal_continue),
    MODULENAME(self_trace_t::launchmon_module_trace.module_name)
{
  /* any more initialization here */
}


//!  PUBLIC: 
/*! linux_launchmon_t<> constructors & destructor


*/
linux_launchmon_t::linux_launchmon_t ( const linux_launchmon_t& l )
{
  /* any more initialization here */
  self_trace_t::trace ( LEVELCHK(level1), 
     MODULENAME, 1, 
    "launchmon object shouldn't be copied, exiting.");
}


//!  PUBLIC: 
/*! linux_launchmon_t<> constructors & destructor


*/
linux_launchmon_t::~linux_launchmon_t ()
{
  /* nothing to delete in this layer */
  

 
  printf(" i am here\n");


}


//! PUBLIC: init
/*!
    Method that registers platform specific process/thread tracers 
    into the platform indepdent layer. It also initializes the FE-engine
    connection for API mode.
*/
launchmon_rc_e 
linux_launchmon_t::init ( opts_args_t *opt )
{
  using namespace std;

  launchmon_rc_e lrc;

  //
  // registering a linux process tracer
  //
  set_tracer(new linux_ptracer_t<SDBG_LINUX_DFLT_INSTANTIATION>());

  //
  // registering a linux thread tracer
  //
  set_ttracer (new linux_thread_tracer_t<T_VA,T_WT,T_IT,T_GRS,T_FRS>());

  lrc = init_API(opt);

  set_last_seen (gettimeofdayD()); 

  return lrc;
}


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
      bool use_cxt = false;

      get_tracer()->tracer_attach(p, use_cxt, -1);

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


//! PUBLIC: handle_bp_prologue
/*!
    performs the breakpoint event prologue. This is nothing 
    but stepping over the target breakpoint.
*/
launchmon_rc_e 
linux_launchmon_t::handle_bp_prologue ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION> &p,
		 breakpoint_base_t<T_VA,T_IT> *bp)
{
  try 
    {
      using namespace std;

      T_VA pc = T_UNINIT_HEX;
      T_VA  adjusted_pc = T_UNINIT_HEX;    
      bool use_cxt = true;
      T_VA retaddr;

      get_tracer()->tracer_getregs(p, use_cxt);
      pc = p.get_gprset(use_cxt)->get_pc();

      //
      // We must keep track of the "return address" 
      // in case the following single step causing the caller
      // to return from the target function,  
      // to simply execute the very next instruction. 
      //
      if ( (retaddr = p.get_gprset(use_cxt)->get_ret_addr())
           == T_UNINIT_HEX )
        {
          T_VA mem_retaddr;
          mem_retaddr = p.get_gprset(use_cxt)->get_memloc_for_ret_addr();
          get_tracer()->tracer_read ( p,
                                      mem_retaddr,
                                      &retaddr,
                                      sizeof(retaddr),
                                      use_cxt );
        }

      bp->set_return_addr(retaddr);

      get_tracer()->pullout_breakpoint  (p, *bp, use_cxt); 
      adjusted_pc = bp->get_address_at(); 

      p.get_gprset(use_cxt)->set_pc(adjusted_pc);
      get_tracer()->tracer_setregs(p, use_cxt);

      get_tracer()->tracer_singlestep (p, use_cxt);

      {
	self_trace_t::trace ( LEVELCHK(level3), 
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


//! PUBLIC: is_bp_prologue_done
/*!
    checks if the prologue for the breakpoint event has been 
    performed. If not, it performs the prologue.
*/
launchmon_rc_e 
linux_launchmon_t::is_bp_prologue_done ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION> &p,
		 breakpoint_base_t<T_VA,T_IT> *bp )
{
  try 
    {
      using namespace std;

      T_VA pc = T_UNINIT_HEX;    
      bool use_cxt = true;

      get_tracer()->tracer_getregs(p,use_cxt);
      pc = p.get_gprset(use_cxt)->get_pc();  

      if ( bp->status == breakpoint_base_t<T_VA,T_IT>::disabled ) 
	{
	  get_tracer()->insert_breakpoint ( p, *bp, use_cxt);

	  {
	    self_trace_t::trace ( LEVELCHK(level3), 
	      MODULENAME,0,
	      "breakpoint event prologue was already done,  time for bp event epilogue. [pc=0x%x]",
	      pc);
	  }

	  return LAUNCHMON_OK;
	}  

      return ( handle_bp_prologue(p, bp)); 
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
      T_VA where_is_rid;
      unsigned long long i, maxcount;   // proctable index can be large!
      int local_pcount;      
      assert ( (main_im  = p.get_myimage()) != NULL );

      //
      // fetching the RPDTAB size
      //
      const symbol_base_t<T_VA>& debug_ps 
	    = main_im->get_a_symbol ( p.get_launch_proctable_size() );
      T_VA procsize_addr = debug_ps.get_relocated_address();  

//      printf("reloc addr is  :%p\n", procsize_addr);


      get_tracer()->tracer_read( p, 
				 procsize_addr, 
				 &(local_pcount), 
				 sizeof(local_pcount), 
				 use_cxt );     
      set_pcount (local_pcount);
      assert(get_pcount() > 0 );


#if MEASURE_TRACING_COST   
    double c_start_ts;
    double c_end_ts;
    c_start_ts = gettimeofdayD();
#endif
      //
      // launcher_proctable holds the MPIR_PROCDESC array. Note that
      // it only contains scalars and pointer addresses. To fetch 
      // the strings pointed by those pointer addresses, we must
      // perform separate read operations using those addresses.
      //
      MPIR_PROCDESC* launcher_proctable 
	    = (MPIR_PROCDESC *) malloc (sizeof (MPIR_PROCDESC) * get_pcount());   
      const symbol_base_t<T_VA>& debug_pt 
	    = main_im->get_a_symbol ( p.get_launch_proctable() );
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
      // fetching each RPDTAB entry including strings pointed by 
      // C pointers.
      //
      maxcount = (unsigned long long) get_pcount();
      for ( i = 0; i < get_pcount(); ++i ) 
	{
	  MPIR_PROCDESC_EXT* an_entry 
	         = (MPIR_PROCDESC_EXT* ) malloc(sizeof(MPIR_PROCDESC_EXT));

	  /*
	   * allocating storages for "an_entry"
	   */	
	  an_entry->pd.host_name 
                 = (char*) malloc(MAX_STRING_SIZE);
	  an_entry->pd.executable_name 
                 = (char*) malloc(MAX_STRING_SIZE);
	  an_entry->pd.pid 
                 = launcher_proctable[i].pid;      
	  an_entry->mpirank = i; /* The mpi rank is the index into the global tab */

	  /*
	   * memory-fetching to get the "host_name" 
           */	
	  get_tracer()->tracer_read_string ( p, 
					     (T_VA) launcher_proctable[i].host_name,    
					     (void*) (an_entry->pd.host_name),
					     MAX_STRING_SIZE,
					     use_cxt );   
   
	  /*
    	   * memory-fetching to get the "executable name" 
	   */	 
          
	  get_tracer()->tracer_read_string ( p, 
					     (T_VA)launcher_proctable[i].executable_name,
					     (void*)an_entry->pd.executable_name,
					     MAX_STRING_SIZE,
					     use_cxt );
//          printf("entry %d read2 \n",i);


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

#if MEASURE_TRACING_COST
     c_end_ts = gettimeofdayD();
     fprintf(stdout, "PROCTAB(%d) Fetching: %f \n", get_pcount(), (c_end_ts - c_start_ts));
#endif

      //
      //
      // fetching the resource ID
      //
      const symbol_base_t<T_VA>& rid 
	= main_im->get_a_symbol (p.get_resource_handler_sym());

/*
#if !RM_BG_MPIRUN
      //
      // DHA 3/4/3009, reviewed. Looks fine for BGP
      // Changed RM_BGL_MPIRUN to RM_BG_MPIRUN to genericize BlueGene Support
      //
//      printf("final read 1\n");
      get_tracer()->tracer_read ( p, 
				  rid.get_relocated_address(),
				  (void*) &where_is_rid,
				  sizeof(T_VA),
				  use_cxt);    
      printf("final read 2\n");
      get_tracer()->tracer_read_string ( 
				  p, where_is_rid,
				  (void*) resource_id,
				  MAX_STRING_SIZE,
				  use_cxt);
       
      printf("final read\n");

      set_resid ( atoi(resource_id) );
      p.set_rid ( get_resid () );

      // -1 is the init value that SLURM sets internally 
      // for "totalview_jobid"
      if ( get_resid() == -1 ) 
	{
	  self_trace_t::trace ( LEVELCHK(level1), 
	     MODULENAME, 1, 
	     "resource ID is not valid!");  

	  return LAUNCHMON_FAILED;
	}
#endif
*/
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

// XT version for launching tool daemons

int launch_daemons(char *daemon_path,char* daemon_opts, int aprun_pid,const char* launchstring,char *pmgr_info,char *pmgr_sec_info)
{
  int my_nid;
  int my_apid;
  int pe0Node;
  int result; 
  char *err_str;
 
  appInfo_t appinfo;
  cmdDetail_t *cmdDetail;
  placeList_t *places;
 
  char expanded_string[MAX_STRING_SIZE];
  char *t;
  t = expanded_string;
 
  err_str = alpsGetMyNid(&my_nid); 
  if (err_str)
  {
     fprintf(stderr, "alpsGetMyNid: %s\n", err_str);
     return -1;
  }
  
  my_apid = alps_get_apid(my_nid, aprun_pid);
  if (!my_apid)
  {
     fprintf(stderr, "alps_get_apid error\n");
     return -1;
  }


     char *tokenize = strdup(pmgr_info);
     char *mip = strtok ( tokenize, ":" );
     char *mport = strtok ( NULL, ":" );
     char *tokenize2 = strdup(pmgr_sec_info);
     
     char* sharedsecret = strdup(strtok (tokenize2, ":"));
     char* randomID = strdup(strtok (NULL, ":"));

  #if PMGR_BASED
     sprintf ( expanded_string,
         launchstring,
         mip,
         mport,
         my_apid,
         sharedsecret,
         randomID);
  #endif
  
  #if COBO_BASED 
  printf("expanded string is %s\n" , expanded_string);
  printf("launch string is %s\n", launchstring);
  sprintf ( expanded_string,
         launchstring,
         sharedsecret,
         randomID);
   #endif
   


  result = alps_get_appinfo(my_apid, &appinfo, &cmdDetail, &places);
  if (result < 0) {	
       fprintf(stderr, "alps_get_appinfo error\n");
     return -1;
  }

/*
  char launchcmd_temp[300];
  sprintf(launchcmd_temp,"/ccs/home/ramya/LMON/launchmon/launchmon/src/linux/daemon_launcher %d %s %s",my_apid,daemon_path,expanded_string);
  char* launchcmd=launchcmd_temp;
  printf("Passing %s to alps tool launcher\n",launchcmd);
*/

 
  char* daemon_launcher;
  daemon_launcher = getenv("LMON_DAEMON_LAUNCHER");

  char launchcmd_temp[300];
  sprintf(launchcmd_temp,"%s %d %s %s",daemon_launcher,my_apid,daemon_path,expanded_string);
  char* launchcmd=launchcmd_temp;
  printf("Passing %s to alps tool launcher\n",launchcmd);
  

  pe0Node = places[0].nid;

/*** ------------------------------------------------------------------------------------------------------------------------------------------------***/
/*** STAGING LIBRARIES AT THE BACKEND - reading STAGE_LIBRARY_PATH environment variable                                                              ***/
/*** ------------------------------------------------------------------------------------------------------------------------------------------------***/

  const char* c_err_str_cplusplus;
  char* save_ptr;
  char** cplusplus_lib=(char**)malloc(sizeof(char*));
  cplusplus_lib[0]=(char*)malloc(sizeof(char*));

  char* stage_lib_path;
  stage_lib_path = getenv("STAGE_LIBRARY_PATH");
  char* copy=(char*)malloc(strlen(stage_lib_path)+1);
  strcpy(copy,stage_lib_path);

  cplusplus_lib[0] = strtok_r(copy, ":", &save_ptr);
  c_err_str_cplusplus=alps_launch_tool_helper(my_apid,pe0Node,true,false,1,cplusplus_lib);
  if (c_err_str_cplusplus!=NULL)
  {
    fprintf(stderr, "Error %s in %s lib staging\n", c_err_str_cplusplus,cplusplus_lib[0]);
    return -1;
  }

  while (cplusplus_lib[0] = strtok_r(NULL,":",&save_ptr))
  {
    c_err_str_cplusplus=alps_launch_tool_helper(my_apid,pe0Node,true,false,1,cplusplus_lib);
    if (c_err_str_cplusplus!=NULL)
    {
      fprintf(stderr, "Error %s in %s lib staging\n", c_err_str_cplusplus,cplusplus_lib[0]);
      return -1;
    }
  }

  const char *c_err_str;
 // printf("calling alps tool helper\n");
  c_err_str = alps_launch_tool_helper(my_apid, pe0Node,true, true, 1, &launchcmd);
//  printf("after call to tool helper\n");
  if (c_err_str!=NULL)
  {
     fprintf(stderr, "Error %s\n", c_err_str);
     return -1;
  }

  return 0;
}

//! PRIVATE: linux_launchmon_t::launch_tool_daemons
/*!
    launches the target tool deamons.

*/
bool
linux_launchmon_t::launch_tool_daemons ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  using namespace std;

  assert ( !get_proctable_copy().empty() );

#if !RM_BG_MPIRUN
  //
  // DHA 3/4/3009, reviewed. Looks fine for BGP. BG mpirun
  // doesn't implement totalview_jobid
  // Changed RM_BGL_MPIRUN to RM_BG_MPIRUN to genericize BlueGene Support
  //
  assert ( get_resid() > 0 );
#endif

  if ( !get_API_mode() 
       && !(p.get_myopts()->get_my_opt()->modelchecker))
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
      char* execname = NULL;

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

	  if ( !(p.get_myopts()->get_my_opt()->modelchecker) )
	    {
	      // 
	      // envVar looks like LAUNCHMON_alc0=12376:23452
	      //
	      string envname = string("LAUNCHMON_") + string(pos->first);
	      setenv(envname.c_str(), pidlist.c_str(), 1);
	    }
	}
    }
 
  if ( p.get_myopts()->get_my_opt()->modelchecker )
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

#if RM_BG_MPIRUN
  //
  // there isn't much you want to do here,
  // because BGLRM does co-spawning of daemons as part of
  // its APAI extension. 
  //
  // TODO: We may not want to release the mpirun process until the 
  // tool set up is done...
  //
  //
  // DHA 3/4/2009, reviewed and looks fine for BGP
  // Changed RM_BGL_MPIRUN to RM_BG_MPIRUN to genericize BlueGene Support
  //
#else

  set_toollauncherpid  (fork());
  if ( !get_toollauncherpid ())
    {
      //
      // The child process
      //
      char expanded_string[MAX_STRING_SIZE];
      char *t;
      char *tmp;
      int i;
      int n = 128;
      char **av = (char**) malloc (n*sizeof(char*));

      if (av == NULL) 
        {
          self_trace_t::trace ( true,
                                MODULENAME,1,
                                "malloc returned null");
          perror("");
          exit(1);
        }

      t = expanded_string;
      i=0;

      if (p.get_myopts()) 
	{
	  char *tokenize2 = strdup(p.get_myopts()->get_my_opt()->lmon_sec_info.c_str());
	  char *sharedsecret = strtok (tokenize2, ":");
	  char *randomID = strtok (NULL, ":");
#if PMGR_BASED
          char *tokenize = strdup(p.get_myopts()->get_my_opt()->pmgr_info.c_str());
	  char *mip = strtok ( tokenize, ":" );
	  char *mport = strtok ( NULL, ":" );

	  sprintf ( expanded_string,
		p.get_myopts()->get_my_opt()->launchstring.c_str(), 
		get_resid(),
		get_proctable_copy().size(),
		get_proctable_copy().size(),
		get_proctable_copy().size(),
		mip,
		mport,
		get_resid(),
		sharedsecret,
		randomID);
#else
	  sprintf ( expanded_string, 
		p.get_myopts()->get_my_opt()->launchstring.c_str(), 
		get_resid(),
		get_proctable_copy().size(),
		get_proctable_copy().size(),
		sharedsecret,
		randomID);
#endif
	}
	
      {
	self_trace_t::trace ( LEVELCHK(level1), 
			      MODULENAME,0,
			      "launching daemons with: %s",
			      expanded_string);
      }

      while ( ( tmp = strtok ( t, " " )) != NULL  )
	{
	  av[i] = strdup ( tmp );
	  t = NULL;
	  if ( i > n )
	    {
	      av = (char**) realloc ( av, 2*n*sizeof(char*));
	      n += n;
	    }
	  i++;
	}
      av[i] = NULL;
      i=0;

      if ( execvp ( av[0], av) < 0 )
	{
	  self_trace_t::trace ( true, 
				MODULENAME,1,
				"execvp to launch tool daemon failed");
	  perror("");
	  exit(1);
	}
    }
#endif

  return LAUNCHMON_OK;
}


//! PUBLIC: handle_launch_bp_event
/*!
    The event handler for a launch breakpoint hit event.
    Most RM APAI implementations use MPIR_Breakpoint for this.
*/
launchmon_rc_e 
linux_launchmon_t::handle_launch_bp_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  try 
    {
      
      using namespace std;
      printf("inside handle_launch_bp_event\n");
#if MEASURE_TRACING_COST
      beginTS = gettimeofdayD ();
#endif
      launchmon_rc_e lrc = LAUNCHMON_OK;
      bool use_cxt = true;
      image_base_t<T_VA,elf_wrapper>* main_im;
      T_VA debug_state_addr;
      char pmgr_info[50];
      char pmgr_sec_info[50];
      int bdbg, bdbgp;
  
      if ( is_bp_prologue_done(p, p.get_launch_hidden_bp()) != LAUNCHMON_OK ) {
#if MEASURE_TRACING_COST
        endTS = gettimeofdayD ();
        accum += endTS - beginTS;
	countHandler++;
#endif
	return LAUNCHMON_OK;
      }

      self_trace_t::trace ( LEVELCHK(level2),
	MODULENAME,0,
	"launch-breakpoint hit event handler invoked.");

      main_im  = p.get_myimage();  
      assert(main_im != NULL);

      //
      // looking up MPIR_debug_state
      //
      const symbol_base_t<T_VA> &debug_state_var 
	    = main_im->get_a_symbol (p.get_launch_debug_state());
      debug_state_addr = debug_state_var.get_relocated_address(); 
      get_tracer()->tracer_read ( p, 
				  debug_state_addr,
				  &bdbg,
				  sizeof(bdbg),
				  use_cxt );
      const symbol_base_t<T_VA> &debug_state
            = main_im->get_a_symbol (p.get_launch_being_debug());

#if MEASURE_TRACING_COST
      endTS = gettimeofdayD ();
      accum += endTS - beginTS;
      countHandler++;
#endif

      switch ( bdbg )
	{
	case MPIR_DEBUG_SPAWNED:
          {
	   
            printf("inside handle_launch_bp_event\n"); 
            /*
	     * Apparently, MPI tasks have just been spawned.
	     *   We want to acquire RPDTAB and the resource ID, 
	     *   and to pass those along to the FE client. 
	     *   Subsequently, we want to launch the specified tool 
             *   daemons before let go of the RM process.
	     */
	    {
	      self_trace_t::trace ( LEVELCHK(level2), 
		MODULENAME,0,
	        "launch-breakpoint hit event handler completing with MPIR_DEBUG_SPAWNED");
	    }
	    acquire_proctable ( p, use_cxt );
	    ship_proctab_msg ( lmonp_proctable_avail );
	    ship_resourcehandle_msg ( lmonp_resourcehandle_avail, get_resid() );
	    say_fetofe_msg ( lmonp_stop_at_launch_bp_spawned );
            
            #if RM_ALPS_APRUN

            pid_t temppid=p.get_master_thread_pid();
            int ramya=(int)temppid;

            std::string daemon_path=p.get_myopts()->get_my_opt()->tool_daemon;
            std::string daemon_opts=p.get_myopts()->get_my_opt()->tool_daemon_opts;

            if (p.get_myopts())
            {
                
                 opt_struct_t* tmpopt=(p.get_myopts())->get_my_opt();

    
                #if PMGR_BASED
                   std::string pmgr_info_str=tmpopt->pmgr_info;
                  //sprintf(pmgr_info,"%s",p.get_myopts()->get_my_opt()->pmgr_info.c_str());
                  sprintf(pmgr_info,"%s",pmgr_info_str.c_str());
                #endif
  
                  sprintf(pmgr_sec_info,"%s",p.get_myopts()->get_my_opt()->lmon_sec_info.c_str());
                   
                //   printf("pmgr_info is %s\n", pmgr_info);
                 printf("pmgr sec info is %s\n", pmgr_sec_info);  


               //#endif

            }


            const char* launchstring=p.get_myopts()->get_my_opt()->launchstring.c_str();
            int aprun_pid=(int)p.get_myopts()->get_my_opt()->launcher_pid;
            launch_daemons((char*)daemon_path.c_str(),(char*)daemon_opts.c_str(),ramya,launchstring,pmgr_info,pmgr_sec_info);
            #else
            launch_tool_daemons(p);
            #endif

	    get_tracer()->tracer_continue (p, use_cxt);


	    break;
          }
	case MPIR_DEBUG_ABORTING:
          {
	    /*
	     * Apparently, MPI tasks have just been aborted, either normally or abnormally.
             *   We want to pass this along to the FE client, 
	     *   to notify the RM launcher of the upcoming detach via 
             *   the MPIR_being_debugged, to disinsert all breakpoints, and to 
	     *   actually issue a detach command to the RM process.
	     */
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
            const symbol_base_t<T_VA>& being_debugged
              = p.get_myimage()->get_a_symbol (p.get_launch_being_debug());

            get_tracer()->tracer_write ( p,
                                  being_debugged.get_relocated_address(),
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

            self_trace_t::trace ( LEVELCHK(level1),
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

            self_trace_t::trace ( LEVELCHK(level1),
              MODULENAME,
              0,
              "detached from all the RM process...");

            say_fetofe_msg(lmonp_stop_at_launch_bp_abort);

 	    //
 	    // this return code will cause the engine to exit.
 	    // but it should leave its children RM_daemon process
 	    // in a running state.
 	    //
 	    lrc = LAUNCHMON_MPIR_DEBUG_ABORT;

	    {
	      self_trace_t::trace ( LEVELCHK(level2),
		MODULENAME,0,
	        "launch-breakpoint hit event handler completing with MPIR_DEBUG_ABORTING");
	    }
	    break; 
	  }
	default:
	  {
	    {
	      self_trace_t::trace ( LEVELCHK(level2), 
		MODULENAME,0,
	       "launch-breakpoint hit event handler completing with unknown debug state");
	    }
	    break;
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
      const symbol_base_t<T_VA>& being_debugged
        = p.get_myimage()->get_a_symbol (p.get_launch_being_debug());

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
          //
          // Please hide this "srun" specific code
          //
          if ( dt == std::string("srun")
               || dt == std::string("lt-srun"))
            {
              usleep (GracePeriodBNSignals);
              kill ( get_toollauncherpid(), SIGINT);
              usleep (GracePeriodBNSignals);
              kill ( get_toollauncherpid(), SIGINT);
              usleep (GracePeriodBNSignals);
            }
          say_fetofe_msg ( lmonp_detach_done );	
          break;
        case FE_disconnected:
	  //usleep (GracePeriodFEDisconnection);
          //
          // Please hide this "srun" specific code
          //
          if ( dt == std::string("srun")
               || dt == std::string("lt-srun"))
            {
              usleep (GracePeriodBNSignals);
              kill ( get_toollauncherpid(), SIGINT);
              usleep (GracePeriodBNSignals);
              kill ( get_toollauncherpid(), SIGINT);
              usleep (GracePeriodBNSignals);
            }
          break;
        case ENGINE_dying_wsignal:
          say_fetofe_msg (lmonp_stop_tracing);
          break;
        case reserved_for_rent:
        case FE_requested_kill:
          {
	    self_trace_t::trace ( LEVELCHK(level1), 
			          MODULENAME,1,
	    "Reason for the detach is unclear (reserved_for_rent or FE_requested_kill)  ");
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
      const symbol_base_t<T_VA>& debug_state
        = p.get_myimage()->get_a_symbol (p.get_launch_being_debug());
 
      debug_state_flag = debug_state.get_relocated_address();
      get_tracer()->tracer_write ( p,
        debug_state_flag,
        &bdbg,
        sizeof(bdbg),
        false );

      //
      // detach from all the main thread
      //
      get_tracer()->tracer_detach ( p, false );

       self_trace_t::trace ( LEVELCHK(level1),
         MODULENAME,
         0,
         "detached from the RM process ...");

      std::string dt = basename ( strdup (p.get_myopts()->get_my_opt()->debugtarget.c_str()));

      //
      // Please hide this "srun" specific code
      //
      if ( dt == std::string("srun") 
              || dt == std::string("lt-srun"))
        {
          usleep (GracePeriodBNSignals);
          kill ( p.get_pid(false), SIGINT);
          usleep (GracePeriodBNSignals);
          kill ( p.get_pid(false), SIGINT);
          usleep (GracePeriodBNSignals);

          kill ( get_toollauncherpid(), SIGINT );
	  usleep (GracePeriodBNSignals);
          kill ( get_toollauncherpid(), SIGINT );
	  usleep (GracePeriodBNSignals);
         }
       else if ( dt == std::string ("mpirun32") 
                    || dt == std::string ("mpirun64") 
		    || dt == std::string("mpirun") )
         {
           //
           // Please hide this "mpirun" specific code
           //
	   //
	   // NOTE: Coded for BlueGene, so make sure if the
	   // following "kill support" works for mpiruns on other platforms.
  	   // If not, conditionals will be needed below.
	   //	
           usleep (GracePeriodBNSignals);
           kill ( p.get_pid(false), SIGINT);
	   usleep (GracePeriodBNSignals);
	   std::cout << "SIGINT sent" << std::endl;
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
	    "RM_BE_daemon_exited or its equivalents should not kill the job!");
          }
          break;
        case FE_requested_kill:
          say_fetofe_msg ( lmonp_kill_done );	
          break;
        case reserved_for_rent:
          {
	    self_trace_t::trace ( LEVELCHK(level1), 
			          MODULENAME,1,
	    "Reason for the kill is unclear (reserved_for_rent!)");
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

      bool use_cxt = true;
      image_base_t<T_VA,elf_wrapper> *dynloader_im = NULL;
      image_base_t<T_VA,elf_wrapper> *main_im = NULL;  
      breakpoint_base_t<T_VA, T_IT> *lo_bp = NULL;
      breakpoint_base_t<T_VA, T_IT> *la_bp = NULL;
      int bdbg = 1;
      //char* pmgr_info;
      //char* pmgr_sec_info;
      char pmgr_info[50];
      char pmgr_sec_info[100];   
      T_VA lpc = T_UNINIT_HEX;
      T_VA debug_state_flag = T_UNINIT_HEX;
      T_VA debug_state_addr = T_UNINIT_HEX;
      T_VA addr_dl_bp = T_UNINIT_HEX;

      {
	self_trace_t::trace ( LEVELCHK(level2), 
			      MODULENAME,0,
			      " trap after attach event handler invoked. ");
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

      lpc = get_va_from_procfs ( p.get_master_thread_pid(), 
				 dynloader_im->get_base_image_name() );

      if ( (lpc = get_auxv (p.get_master_thread_pid()) ) == T_UNINIT_HEX)
      {
	 lpc = get_va_from_procfs ( p.get_master_thread_pid(), 
		 dynloader_im->get_base_image_name() );
      }
      
      if ( lpc == T_UNINIT_HEX )
      {
	self_trace_t::trace ( LEVELCHK(level1), 
			      MODULENAME,1,
			      "can't resolve the base address of the loader.");
			      
	return LAUNCHMON_FAILED;
      }	
      
      dynloader_im->set_image_base_address(lpc);
      dynloader_im->compute_reloc();

      // registering p.launch_hidden_bp: because launch_hidden_bp 
      // comes from the base image, it doesn't need to be relocating. 
      const symbol_base_t<T_VA>& launch_bp_sym 
	    = main_im->get_a_symbol (p.get_launch_breakpoint_sym());

      la_bp = new linux_breakpoint_t();
      la_bp->set_address_at(launch_bp_sym.get_relocated_address());
#if PPC_ARCHITECTURE
      la_bp->set_use_indirection();
#endif
      la_bp->status 
	    = breakpoint_base_t<T_VA, T_IT>::set_but_not_inserted;

      p.set_launch_hidden_bp(la_bp);
      get_tracer()->insert_breakpoint ( p, 
					(*p.get_launch_hidden_bp()),
					use_cxt );
   

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
      lo_bp->status 
	    = breakpoint_base_t<T_VA, T_IT>::set_but_not_inserted;

      p.set_loader_hidden_bp(lo_bp);
      get_tracer()->insert_breakpoint ( p, 
					(*p.get_loader_hidden_bp()),
					use_cxt );


      // setting MPIR_being_debugged.
      //
      const symbol_base_t<T_VA>& debug_state 
	= main_im->get_a_symbol (p.get_launch_being_debug());

      debug_state_flag = debug_state.get_relocated_address();  

      get_tracer()->tracer_write ( p, 
				   debug_state_flag, 
				   &bdbg,
				   sizeof(bdbg), 
				   use_cxt );

#if RM_BG_MPIRUN
      //
      // Always check correctness of BG_SERVERARG_LENGTH 
      // and BG_EXECPATH_LENGTH
      //
#define BG_SERVERARG_LENGTH 1024
#define BG_EXECPATH_LENGTH 256
      //
      // To deal with BGL's APAI extension
      //
      // setting MPIR_executable_path
      //
      // DHA 3/4/2009, reviewed and this looks fine for BGP
      // BGP's mpirun implements: char MPIR_executable_path[256]
      // and char MPIR_server_arguments[1024] as well.
      // Chaged the macro to RM_BG_MPIRUN from RM_BGL_MPIRUN to
      // genericize BlueGene support.
      //
#if PMGR_BASED || COBO_BASED
      char *tokenize2 = strdup(p.get_myopts()->get_my_opt()->lmon_sec_info.c_str());
      char *sharedsecret = strtok (tokenize2, ":");
      char *randomID = strtok (NULL, ":");
      char serverargstmp[BG_SERVERARG_LENGTH];
      char serverargs[BG_SERVERARG_LENGTH] = {0};
      char *curptr = NULL;
      char *token = NULL;
#if PMGR_BASED
      char *tokenize = strdup(p.get_myopts()->get_my_opt()->pmgr_info.c_str());
      char *mip = strtok ( tokenize, ":" );
      char *mport = strtok ( NULL, ":" );
#endif


      const symbol_base_t<T_VA>& executablepath
        = main_im->get_a_symbol (p.get_launch_exec_path ());

      T_VA ep_addr = executablepath.get_relocated_address();

      //
      // daemon_path length cannot exceed 256
      //
      get_tracer()->tracer_write ( p,
                                   ep_addr,
                                   p.get_myopts()->get_my_opt()->tool_daemon.c_str(),
                                   p.get_myopts()->get_my_opt()->tool_daemon.size()+1,
                                   use_cxt );

      const symbol_base_t<T_VA>& sa
        = main_im->get_a_symbol (p.get_launch_server_args ());

      T_VA sa_addr = sa.get_relocated_address();

      sprintf ( serverargstmp,
                p.get_myopts()->get_my_opt()->launchstring.c_str(),
#if PMGR_BASED
                mip,
                mport,
                24689, /* just a random number for pmgrjobid on BGL */
#endif
                sharedsecret,
                randomID);

      curptr = serverargs;
      token = strtok (serverargstmp, " ");
      int tlen = strlen(token);
      while ( curptr != NULL && ((curptr-serverargs+tlen+1) < BG_SERVERARG_LENGTH))
        {
          memcpy ( curptr, token, tlen);
          *(curptr + tlen) = '\0';
          curptr += tlen + 1;
          token = strtok (NULL, " ");
          if (!token)
            break;
          tlen = strlen(token);
        }

      if ( (curptr - serverargs) > (BG_SERVERARG_LENGTH-1))
        {
          self_trace_t::trace ( LEVELCHK(level2),
                                MODULENAME,1,
                                "Daemon arg list too long");
        }

      (*curptr) = '\0';
      curptr += 1;
      get_tracer()->tracer_write ( p,
                                   sa_addr,
                                   serverargs,
                                   BG_SERVERARG_LENGTH,
                                   use_cxt );
#endif /* PMGR_BASED || COBO_BASED */
#endif /* RM_BG_MPIRUN */

      //	
      // checking to see if the pthread library has been loaded, 
      // and if so, initializing the thread tracing.
      //
      chk_pthread_libc_and_init(p);
      p.set_never_trapped(false); 

#if !RM_BG_MPIRUN
      acquire_proctable ( p, use_cxt );
      ship_proctab_msg ( lmonp_proctable_avail );
      ship_resourcehandle_msg ( lmonp_resourcehandle_avail, get_resid() );
      say_fetofe_msg ( lmonp_stop_at_first_attach );
	  
	  /*
	   *
	   * OKAY, we are ready to launch tool daemons for this attach event
	   *
	   *
	   */
      #if RM_ALPS_APRUN

     pid_t temppid=p.get_master_thread_pid();
     std::string daemon_path=p.get_myopts()->get_my_opt()->tool_daemon;
     std::string daemon_opts=p.get_myopts()->get_my_opt()->tool_daemon_opts;
     
     //sleep(10);  
     int aprun_pid=(int)p.get_myopts()->get_my_opt()->launcher_pid;
     if(use_cxt)
    printf("value of ue_cxt is 1\n");
    else
    printf("value of use_cxt is 0\n");
 
    printf("p value is %p\n", &p); 

     printf("pid here1 p is  %d\n", p.get_pid(use_cxt));

     if (p.get_myopts())
     {

      opt_struct_t* tmpopt=(p.get_myopts())->get_my_opt();

#if PMGR_BASED
     std::string pmgr_info_str=tmpopt->pmgr_info;
      sprintf(pmgr_info,"%s",pmgr_info_str.c_str());

#endif


     std::string lmon_sec_info_str=tmpopt->lmon_sec_info;

     //sprintf(pmgr_info,"%s",p.get_myopts()->get_my_opt()->pmgr_info.c_str());
     //sprintf(pmgr_sec_info,"%s",p.get_myopts()->get_my_opt()->lmon_sec_info.c_str());
     sprintf(pmgr_sec_info, "%s", lmon_sec_info_str.c_str()); 

     printf("pmgr info is %s\n", pmgr_info);
     printf("pmgr sec info is %s\n", pmgr_sec_info);
     //printf("p value is %p\n", &p);
     //printf("pid here2 p is  %d\n", p.get_pid(use_cxt));


     }
  
 
    //if(use_cxt)
    //printf("value of ue_cxt is 1\n");
    //else
    //printf("value of use_cxt is 0\n"); 
    const char* launchstring=p.get_myopts()->get_my_opt()->launchstring.c_str();

    //int aprun_pid=(int)p.get_myopts()->get_my_opt()->launcher_pid;

    launch_daemons((char*)daemon_path.c_str(),(char*)daemon_opts.c_str(),aprun_pid,launchstring,pmgr_info,pmgr_sec_info);
    //printf("pid here3 p is  %d\n", p.get_pid(use_cxt));
#else
    launch_tool_daemons(p);
#endif
#endif

      get_tracer()->tracer_continue (p, use_cxt);

      {
	self_trace_t::trace ( LEVELCHK(level2), 
			      MODULENAME,0,
	"trap after attach event handler completed.");
      }
  
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

      printf("Inside handle_trap_after_exec_event\n");
      using namespace std;

#if MEASURE_TRACING_COST
     beginTS = gettimeofdayD ();
#endif

      bool use_cxt = true;
      image_base_t<T_VA,elf_wrapper> *main_im, *dynloader_im;
      breakpoint_base_t<T_VA, T_IT> *lo_bp, *la_bp;
      T_VA addr_dl_start, dl_linked_addr, addr_dl_bp;
      T_VA debug_state_flag;
      int bdbg = 1; 

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
      // registering p.launch_hidden_bp 
      //
      const symbol_base_t<T_VA>& launch_bp_sym 
	= main_im->get_a_symbol (p.get_launch_breakpoint_sym());

      la_bp = new linux_breakpoint_t();
      la_bp->set_address_at(launch_bp_sym.get_relocated_address());
      
#if RM_BG_MPIRUN
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

      la_bp->status 
	= breakpoint_base_t<T_VA, T_IT>::set_but_not_inserted;

      p.set_launch_hidden_bp(la_bp);
      get_tracer()->insert_breakpoint ( p, 
					(*p.get_launch_hidden_bp()),
					use_cxt );

      //
      // setting MPIR_debing_debugged
      //
      const symbol_base_t<T_VA>& debug_state 
	= main_im->get_a_symbol (p.get_launch_being_debug());
      debug_state_flag = debug_state.get_relocated_address();  
      get_tracer()->tracer_write ( p, 
				   debug_state_flag, 
				   &bdbg,
				   sizeof(bdbg), 
				   use_cxt );
 
#if RM_BG_MPIRUN
      // DHA 3/4/2009, reviewed and this looks fine for BGP
      // BGP's mpirun implements: char MPIR_executable_path[256]
      // and char MPIR_server_arguments[1024] as well.
      // Chaged the macro to RM_BG_MPIRUN from RM_BGL_MPIRUN to
      // genericize BlueGene support.
      //
#if PMGR_BASED || COBO_BASED
      char *tokenize2 = strdup(p.get_myopts()->get_my_opt()->lmon_sec_info.c_str());
      char *sharedsecret = strtok (tokenize2, ":");
      char *randomID = strtok (NULL, ":");
      char serverargstmp[BG_SERVERARG_LENGTH];
      char serverargs[BG_SERVERARG_LENGTH] = {0};
      char *curptr = NULL;
      char *token = NULL;
#if PMGR_BASED
      char *tokenize = strdup(p.get_myopts()->get_my_opt()->pmgr_info.c_str());
      char *mip = strtok ( tokenize, ":" );
      char *mport = strtok ( NULL, ":" );
#endif

      const symbol_base_t<T_VA>& executablepath
	= main_im->get_a_symbol (p.get_launch_exec_path ());
      T_VA ep_addr = executablepath.get_relocated_address();  

      //
      // daemon_path length cannot exceed 256
      //
      get_tracer()->tracer_write ( p, 
				   ep_addr, 
				   p.get_myopts()->get_my_opt()->tool_daemon.c_str(),
				   p.get_myopts()->get_my_opt()->tool_daemon.size()+1, 
				   use_cxt );

      const symbol_base_t<T_VA>& sa
	= main_im->get_a_symbol (p.get_launch_server_args ());
      T_VA sa_addr = sa.get_relocated_address();  

      sprintf ( serverargstmp,
                p.get_myopts()->get_my_opt()->launchstring.c_str(),
#if PMGR_BASED
                mip,
                mport,
                24689, /* just a random number */
#endif
                sharedsecret,
                randomID);

      curptr = serverargs;
      token = strtok (serverargstmp, " ");
      int tlen = strlen(token);
      while ( curptr != NULL && ((curptr-serverargs+tlen+1) < BG_SERVERARG_LENGTH))
        {
          memcpy ( curptr, token, tlen);
          *(curptr + tlen) = '\0';
          curptr += tlen + 1;
	  token = strtok (NULL, " "); 	
          if (!token)
            break;
	  tlen = strlen(token);
        }	

      if ( (curptr - serverargs) > (BG_SERVERARG_LENGTH-1)) 
        {
          self_trace_t::trace ( LEVELCHK(level2),
                                MODULENAME,1,
                                "Daemon arg list too long");
        }

      (*curptr) = '\0';
      curptr += 1; 
      get_tracer()->tracer_write ( p,
				   sa_addr,
				   serverargs,
				   BG_SERVERARG_LENGTH,
				   use_cxt );
#endif /* PMGR_BASED || COBO_BASED */
#endif
 
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

      lo_bp->status 
	= breakpoint_base_t<T_VA, T_IT>::set_but_not_inserted;

      p.set_loader_hidden_bp(lo_bp);
      get_tracer()->insert_breakpoint ( p,
					(*p.get_loader_hidden_bp()),
					use_cxt );

      // now this process has get past the first fork/exec
      //
      p.set_never_trapped ( false );
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

      if ( is_bp_prologue_done(p, p.get_loader_hidden_bp()) != LAUNCHMON_OK ) {
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

      chk_pthread_libc_and_init(p);    
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
linux_launchmon_t::handle_fork_bp_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  try 
    {
      using namespace std;

#if MEASURE_TRACING_COST
     beginTS = gettimeofdayD ();
#endif

      bool use_cxt = true; 
  
      if ( is_bp_prologue_done(p, p.get_fork_hidden_bp()) != LAUNCHMON_OK ) {
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
			      "fork event handler invoked.");
      }

      disable_all_BPs(p, use_cxt);  
      get_tracer()->tracer_syscall(p, use_cxt);
      continue_method = syscall_continue;

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

	  /*
	   * LMON API SUPPORT: a message via a pipe to the watchdog thread
	   *
	   */
	  say_fetofe_msg(lmonp_exited);	 

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

      if (rc == LAUNCHMON_MAINPROG_EXITED) { 
        fprintf (stdout, "COUNT: %d\n", countHandler);
        fprintf (stdout, "ACCUM TIME: %f\n", accum);
      } 
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
	
  set_last_seen (gettimeofdayD ());
  return LAUNCHMON_OK;
}


//! PUBLIC: handle_thrcreate_bp_event 
/*!
    handle a thread-creation event 
*/
launchmon_rc_e 
linux_launchmon_t::handle_thrcreate_bp_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p )
{
  try 
    {
      using namespace std;

#if MEASURE_TRACING_COST
     beginTS = gettimeofdayD (); 
#endif
      
      bool use_cxt = true; 
      
      if ( is_bp_prologue_done(p, p.get_thread_creation_hidden_bp()) != LAUNCHMON_OK ) {
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
	  "thread creation event handler invoked");
      }     

      get_ttracer()->ttracer_attach(p);
      get_tracer()->tracer_continue(p,use_cxt);

      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0,
	  "thread creation event handler completed");
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
  catch ( thread_tracer_exception_t e ) 
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


//! PUBLIC: handle_thrdeath_bp_event 
/*!
   handle a thread-death event 
*/
launchmon_rc_e 
linux_launchmon_t::handle_thrdeath_bp_event ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p ) 
{
  try 
    {
      bool use_cxt = true; 

      if ( is_bp_prologue_done(p, p.get_thread_death_hidden_bp()) != LAUNCHMON_OK )
	return LAUNCHMON_OK;

      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0,
	  "thread death event handler invoked");      
      }

      //
      // UNIMPLEMENTED!!
      //
      self_trace_t::trace ( true, 
	  MODULENAME,1,
	  "UNIMPLEMENTED %d : %d", p.get_pid(true),  p.get_pid(false));
      get_tracer()->tracer_continue(p,use_cxt);
      {
	self_trace_t::trace ( LEVELCHK(level2), 
	  MODULENAME,0,
	  "thread death event handler completed");
      }   

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
  catch ( thread_tracer_exception_t e ) 
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

      switch (continue_method) 
	{
	case normal_continue:
	  {
	    get_tracer()->tracer_continue (p, use_cxt);
	    break;
	  }
	case syscall_continue:
	  {
	    continue_method = in_between;
	    get_tracer()->tracer_syscall (p, use_cxt);
	    break;
	  }
	case in_between:
	  {
	    continue_method = normal_continue;
	    enable_all_BPs(p, use_cxt);
	    get_tracer()->tracer_continue (p, use_cxt);
	    break;
	  }
	default:
	  break;
	}

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

////////////////////////////////////////////////////////////////////
//
// PRIVATE METHODS (class linux_launchmon_t<>)
//
//

//!  PRIVATE: linux_launchmon_t::disable_all_BPs
/*! 
     disables all the hidden breakpoints    
*/
bool 
linux_launchmon_t::disable_all_BPs ( 
                 process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p, 
		 bool use_cxt )
{
  try 
    {

      tracer_base_t<SDBG_LINUX_DFLT_INSTANTIATION> *tr = get_tracer(); 
 
      if ( p.get_launch_hidden_bp() ) 
	{
	 tr->pullout_breakpoint ( 
           p, 
	   *(p.get_launch_hidden_bp()), 
	   use_cxt);
	}

      if ( p.get_loader_hidden_bp() ) 
	{
	 tr->pullout_breakpoint ( 
           p, 
           *(p.get_loader_hidden_bp()), 
	   use_cxt);
	}

      if ( p.get_thread_creation_hidden_bp() ) 
	{
	  tr->pullout_breakpoint ( 
            p,
	    *(p.get_thread_creation_hidden_bp()), 
	    use_cxt);
	}  

      if ( p.get_thread_death_hidden_bp() ) 
	{ 
	  tr->pullout_breakpoint ( 
            p, 
	    *(p.get_thread_death_hidden_bp()),
	    use_cxt);	
	}

      if ( p.get_fork_hidden_bp() ) 
	{ 
	  tr->pullout_breakpoint ( 
            p, 
	    *(p.get_fork_hidden_bp()), 
            use_cxt);
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


//!  PRIVATE: linux_launchmon_t::enable_all_BPs
/*!   
     enables all the hidden breakpoints
*/
bool 
linux_launchmon_t::enable_all_BPs ( 
	         process_base_t<SDBG_LINUX_DFLT_INSTANTIATION>& p, 
		 bool use_cxt )
{
  try 
    {
      tracer_base_t<SDBG_LINUX_DFLT_INSTANTIATION>* tr = get_tracer();
 
      if (p.get_launch_hidden_bp()) 
	{
	  tr->insert_breakpoint ( 
            p, 
	    *(p.get_launch_hidden_bp()), 
	    use_cxt);
	}

      if (p.get_loader_hidden_bp()) 
	{
	  tr->insert_breakpoint ( 
            p, 
	    *(p.get_loader_hidden_bp()), 
	    use_cxt);
	}

      if (p.get_thread_creation_hidden_bp()) 
	{
	  tr->insert_breakpoint ( 
            p, 
	    *(p.get_thread_creation_hidden_bp()), 
	    use_cxt);
	}

      if (p.get_thread_death_hidden_bp()) 
	{ 
	  tr->insert_breakpoint ( 
            p, 
	    *(p.get_thread_death_hidden_bp()), 
	    use_cxt);
	}

      if (p.get_fork_hidden_bp()) 
	{
	  tr->insert_breakpoint ( 
            p, 
            *(p.get_fork_hidden_bp()), 
	    use_cxt);
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


//!  PRIVATE: linux_launchmon_t::chk_pthread_libc_and_init
/*!
     checks to see if libpthread is linked, and if so
     initializes the thread tracer
*/
bool 
linux_launchmon_t::chk_pthread_libc_and_init ( 
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
      breakpoint_base_t<T_VA, T_IT> *frbp;
      T_VA where_to_read;
      struct r_debug r_debug_buf;
      struct link_map a_map;
      char lname[MAX_STRING_SIZE];

      assert( (thr_im = p.get_mythread_lib_image()) != NULL );

      if ( thr_im->get_image_base_address() !=  SYMTAB_UNINIT_ADDR ) 
	{
	  {
	    self_trace_t::trace ( LEVELCHK(level3), 
	      MODULENAME, 0,
	      "libpthread base already found. No need for dynamic load tracing. [base addr=0x%x]",
	      thr_im->get_image_base_address());
	  }

	  return false;
	}

      assert( (libc_im = p.get_mylibc_image()) != NULL );
      assert( (dynloader_im = p.get_mydynloader_image()) != NULL );  

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

	    if ( (strncmp(LIBPTHREAD_IDEN,bn,strlen(LIBPTHREAD_IDEN)) == 0 ) 
                  && (a_map.l_addr)) 
              {
                /*
                 * The linked map for the POSIX thread library is found
                 *
                 */
                thr_im->init(slname);
		thr_im->read_linkage_symbols();
	        thr_im->set_image_base_address(a_map.l_addr);
		thr_im->compute_reloc(); 
		get_ttracer()->ttracer_init(p, get_tracer()); 
		rc = true;
	      } 
	    else if ( (strncmp(LIBC_IDEN,bn,strlen(LIBC_IDEN)) == 0 ) 
                  && (a_map.l_addr)) 
              {
                /*
                 * The linked map for the C runtime library is found
                 *
                 */
                libc_im->init(slname);
                libc_im->read_linkage_symbols();
                libc_im->set_image_base_address(a_map.l_addr);	 	
		libc_im->compute_reloc();
		const symbol_base_t<T_VA>& fork_bp_sym 
		  = libc_im->get_a_symbol (p.get_fork_sym());
		frbp = new linux_breakpoint_t();
		frbp->set_address_at(fork_bp_sym.get_relocated_address());
#if PPC_ARCHITECTURE
                frbp->set_use_indirection();
#endif
		frbp->status 
		  = breakpoint_base_t<T_VA, T_IT>::set_but_not_inserted;
		p.set_fork_hidden_bp(frbp);
		get_tracer()->insert_breakpoint ( p,
		  (*p.get_fork_hidden_bp()),
		  use_cxt );
              }

            free(cplname1);
            free(cplname2);
	  }

	where_to_read =  (T_VA) a_map.l_next;

      } while (where_to_read  && where_to_read != T_UNINIT_HEX );
 
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
