/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/sdbg_base_launchmon_impl.hxx,v 1.15.2.2 2008/02/20 17:37:56 dahn Exp $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2008~2010, Lawrence Livermore National Security, LLC. Produced at 
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
 *        Jun 28 2010 DHA: Added ship_rminfo_msg 
 *        Aug 07 2009 DHA: Added p.set_lwp_state tracking to decipher_an_event
 *                         method.
 *        Feb 09 2008 DHA: Added LLNS Copyright 
 *        Jul 24 2007 DHA: moved and embellished ship_proctab_msg
 *                         and ship_resourcehandle_msg here
 *                         They really belong to the base class.
 *                         As a matter of fact, all methods communicating
 *                         via FE API lmonp message should belong to 
 *                         the base class
 *        Mar 13 2007 DHA: pipe_t support
 *        Jul 03 2006 DHA: Added self tracing support
 *        Jan 12 2006 DHA: Created file.          
 */ 

#ifndef SDBG_BASE_LAUNCHMON_IMPL_HXX
#define SDBG_BASE_LAUNCHMON_IMPL_HXX 1

#ifndef HAVE_LAUNCHMON_CONFIG_H
#include "config.h"
#endif

#include <lmon_api/lmon_api_std.h>

#if HAVE_POLL_H
# include <poll.h>
#else
# error poll.h is required
#endif 

#if HAVE_SIGNAL_H
# include <signal.h>
#else
# error signal.h is required
#endif

#include "sdbg_base_launchmon.hxx"

////////////////////////////////////////////////////////////////////
//
// PRIVATE METHODS (class launchmon_base_t<>)
//
////////////////////////////////////////////////////////////////////
//!
/*!  launchmon_base_t<> constructor
      
    
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::launchmon_base_t (
                const launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM> &l) 
{
  tracer          = l.tracer;
  ttracer         = l.ttracer;
  resid           = l.resid;
  pcount          = l.pcount;
  toollauncherpid = l.toollauncherpid;
  FE_sockfd       = l.FE_sockfd;
  API_mode        = l.API_mode;
  last_seen       = l.last_seen;
  warm_period     = l.warm_period;
  //
  // This should never be called
  //
}

//!
/*!  launchmon_base_t<> operator= 
      
    
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM> &
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::operator=(
                const launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM> &rhs ) 
{
  tracer          = rhs.tracer;
  ttracer         = rhs.ttracer;
  resid           = rhs.resid;
  pcount          = rhs.pcount;
  toollauncherpid = rhs.toollauncherpid;
  FE_sockfd       = rhs.FE_sockfd;
  API_mode        = rhs.API_mode;
  last_seen       = rhs.last_seen;
  warm_period     = rhs.iwarm_period;
  //
  // This should actually never be called
  //

  return *this;
}


////////////////////////////////////////////////////////////////////
//
// PUBLIC INTERFACES (class launchmon_base_t<>)
//
////////////////////////////////////////////////////////////////////
//!
/*!  launchmon_base_t<> constructor
      
    
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::launchmon_base_t () 
  : engine_state(mpir_start),
    resid(-1), 
    pcount(-1),
    toollauncherpid(-1),
    FE_sockfd(-1),
    API_mode(false),
    MODULENAME(self_trace_t::launchmon_module_trace.module_name)    
{
  char *warm_interval;
  last_seen = gettimeofdayD ();
  warm_period = DefaultWarmPeriods;
  warm_interval = getenv ("LMON_ENGINE_WARM_INTERVAL"); 
  if ( warm_interval ) 
    warm_period = (double) atoi (warm_interval);
}

//!
/*!  launchmon_base_t<> destructor 
      
    
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::~launchmon_base_t ()
{
  if (tracer)
    delete tracer; 
  
  if (ttracer)
    delete ttracer;

  if (!proctable_copy.empty())
    {
      std::map<std::string, std::vector<MPIR_PROCDESC_EXT *> >::iterator iter;
      for(iter = proctable_copy.begin(); iter != proctable_copy.end(); ++iter)
        {
          if (!(iter->second.empty())) 
            {
              std::vector<MPIR_PROCDESC_EXT *>::iterator viter;
              for (viter = iter->second.begin(); viter != iter->second.end(); ++viter)
                {
                  if ((*viter)->pd.host_name)
                    {
                      free((*viter)->pd.host_name);
                    }

                  if ((*viter)->pd.executable_name)
                    {
                      free((*viter)->pd.executable_name);
                    }
                  free(*viter);
                }
            }
        }
      proctable_copy.clear();
    }
}

//!
/*!  launchmon_base_t<> request_detach
     stops all threads and mark the detach flag to the process 
    
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
bool
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::request_detach
(process_base_t<SDBG_DEFAULT_TEMPLPARAM>& p, pcont_req_reason reason)
{
  if (p.get_please_detach() || p.get_please_kill())
    {
      //
      // If either request has already been registered, we
      // must not put additional request. Redundant requests
      // would confuse actual detach/kill command handler.
      //
      return false;
    }
 
  for ( p.thr_iter = p.get_thrlist().begin();
        p.thr_iter != p.get_thrlist().end(); p.thr_iter++ )
    {
      p.make_context ( p.thr_iter->first );
      if (get_tracer()->status(p, true) == SDBG_TRACE_RUNNING)
        {
          get_tracer()->tracer_stop(p, true);
          usleep (GracePeriodBNSignals);
        }
      p.check_and_undo_context ( p.thr_iter->first );
    }

  p.set_please_detach ( true );
  p.set_reason (reason);

  return true;
}

//!
/*!  launchmon_base_t<> request_kill
     stops all threads and mark the kill flag to the process 
    
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
bool
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::request_kill
(process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, pcont_req_reason reason)
{
  if (p.get_please_detach() || p.get_please_kill())
    {
      //
      // If either request has already been registered, we
      // must not put additional request. Redundant requests
      // would confuse actual detach/kill command handler.
      //
      return false;
    }

  for ( p.thr_iter = p.get_thrlist().begin();
        p.thr_iter != p.get_thrlist().end(); p.thr_iter++ )
    {
      p.make_context ( p.thr_iter->first );
      if (get_tracer()->status(p, true) == SDBG_TRACE_RUNNING)
        {
          get_tracer()->tracer_stop(p, true);
          usleep (GracePeriodBNSignals);
        }
      p.check_and_undo_context ( p.thr_iter->first );
    }

  p.set_please_kill ( true );
  p.set_reason (reason);

  return true;
}

//!
/*!  launchmon_base_t<> accessors

 
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void 
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_tracer ( 
                tracer_base_t<SDBG_DEFAULT_TEMPLPARAM> *t ) 
{
  tracer = t;  
}

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
tracer_base_t<SDBG_DEFAULT_TEMPLPARAM> * 
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_tracer ()
{
  return tracer;
}

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void 
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_ttracer ( 
                thread_tracer_base_t<SDBG_DEFAULT_TEMPLPARAM> *t )
{
  ttracer = t;
}

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
thread_tracer_base_t<SDBG_DEFAULT_TEMPLPARAM> * 
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::get_ttracer()
{
  return ttracer;
}

template <SDBG_DEFAULT_TEMPLATE_WIDTH>
void 
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::set_engine_state (int s)
{
  switch (s)
    {
    case MPIR_DEBUG_SPAWNED:
      engine_state = mpir_spawned;
      break;
    case MPIR_DEBUG_ABORTING:
      engine_state = mpir_abort;
      break;
    case MPIR_NULL:
      engine_state = mpir_null;
      break;
    default:
      engine_state = mpir_unknown;
      break;
    }

  return;
}

//! decipher_an_event:
/*!
    deciphers an event of debug_event_t and returns a 
    corresponding launchmon_event_e code.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
launchmon_event_e 
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::decipher_an_event 
(process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, const debug_event_t &event)
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
      tracer->tracer_getregs ( p, use_context );
      VA pc = p.get_gprset(use_context)->get_pc();

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
      else if ( p.get_thread_creation_hidden_bp() 
                && ( p.get_thread_creation_hidden_bp()->is_pc_part_of_bp_op(pc)))
	{
	  return_ev = LM_STOP_AT_THREAD_CREATION;  
	}
      else if ( p.get_thread_death_hidden_bp() 
                && ( p.get_thread_death_hidden_bp()->is_pc_part_of_bp_op(pc)))
	{
	  return_ev = LM_STOP_AT_THREAD_DEATH;
	}
      else if ( p.get_fork_hidden_bp() 
                && ( p.get_fork_hidden_bp()->is_pc_part_of_bp_op(pc)))
	{
	  return_ev = LM_STOP_AT_FORK_BP;
	}
      else if ( event.get_signum () != SIGTRAP )
        {
          //
          // Other event are signals; we resend a signal unless it's TRAP
          //
          return_ev = LM_RELAY_SIGNAL;
        }
      else  
        {
          //
          // P/T stopped because of an event that we don't care about much
          //
	  return_ev = LM_STOP_NOT_INTERESTED;
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


//! invoke_handler:
/*! launchmon_base_t<> invoke_handler
    dispatches a corresponding event handler.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
launchmon_rc_e
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::invoke_handler ( 
                process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p, 
	        const launchmon_event_e ev, 
	        const int data)
{
  launchmon_rc_e rc = LAUNCHMON_FAILED;

  switch ( ev ) {

  //
  // handles a first-exec-trap event.
  // 
  case LM_STOP_AT_FIRST_EXEC:
    rc = handle_trap_after_exec_event (p);
    break;

  //
  // handles an attach-trap event.
  //
  case LM_STOP_AT_FIRST_ATTACH:
    rc = handle_trap_after_attach_event (p);
    break;

  //
  // handles a please_detach command.
  //
  case LM_STOP_FOR_DETACH:
    rc = handle_detach_cmd_event (p);
    break;

  //
  // handles a please_kill command.
  //
  case LM_STOP_FOR_KILL:
    rc = handle_kill_cmd_event (p);
    break;

  //
  // handles a launch-breakpoint event.
  //
  case LM_STOP_AT_LAUNCH_BP:
    rc = handle_launch_bp_event (p);
    break;

  //
  // handles a loader-breakpoint event.
  //
  case LM_STOP_AT_LOADER_BP:
    rc = handle_loader_bp_event (p);
    break;

  //
  // handles a thread-creation event.
  //
  case LM_STOP_AT_THREAD_CREATION:
    rc = handle_thrcreate_bp_event (p);
    break;

  //
  // handles a thread-death event.
  //
  case LM_STOP_AT_THREAD_DEATH:
    rc = handle_thrdeath_bp_event (p);
    break;

  //
  // handles a fork event.
  //
  case LM_STOP_AT_FORK_BP:
    rc = handle_fork_bp_event(p);
    break;

  //
  // handles a signal-relay event.
  //
  case LM_RELAY_SIGNAL:
    rc = handle_relay_signal_event (p, data);
    break;

  //
  // handles an unknown stop event.
  //
  case LM_STOP_NOT_INTERESTED:
    rc = handle_not_interested_event (p);
    break;

  //
  // handles a termination event, 
  // including a thread termination.
  //
  case LM_TERMINATED:
    rc = handle_term_event (p);
    break;

  //
  // handles an exit event.
  //
  case LM_EXITED:
    rc = handle_exit_event (p);
    break;

  //
  // Oh well...
  //
  default:
    rc = LAUNCHMON_FAILED;
    break;
  }

  return rc;
}


//! say_fetofe_msg:
/*! launchmon_base_t<>::say_fetofe_msg

    dispatches a corresponding event handler.
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
launchmon_rc_e
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::say_fetofe_msg (
                lmonp_fe_to_fe_msg_e msg_type )
{
  lmonp_t msgheader;
  
  if ( !get_API_mode() ) 
    return LAUNCHMON_FAILED;

  set_msg_header ( &msgheader,
  	           lmonp_fetofe,
	           (int) msg_type,
	           0,0,0,0,0,0,0 );
  
  if ( (write_lmonp_long_msg ( get_FE_sockfd(), 
                               &msgheader, 
                               sizeof ( msgheader ) )) < 0 )
    {
      return LAUNCHMON_FAILED; 
    }

  return LAUNCHMON_OK; 
}


//! launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::ship_proctab_msg 
/*!
    sends an lmonp_t packet with {msgclass=lmonp_fetofe: 
    type.fetofe_type=lmonp_febe_proctab:
    lmon_payload_length=size of the proctable}
    to the FE API stub.  
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
launchmon_rc_e
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::ship_proctab_msg ( 
                lmonp_fe_to_fe_msg_e t )
{
  using namespace std;

  //
  // If the engine is not driven via the FE API, this method
  // returns LAUNCHMON_FAILED
  //
  if ( !get_API_mode() )
    {    
      self_trace_t::trace ( LEVELCHK(level3), 
	     MODULENAME, 0, 
	     "standalone mode does not ship proctab via LMONP messages");  
      return LAUNCHMON_FAILED;
    }
  
  map<string, unsigned int> execHostName;
  vector<char *> orderedEHName;
  map<string, vector<MPIR_PROCDESC_EXT *> >::const_iterator pos;
  vector<MPIR_PROCDESC_EXT *>::const_iterator vpos;
  vector<char *>::const_iterator EHpos;
  unsigned int offset = 0;
  unsigned int num_unique_exec = 0;
  unsigned int num_unique_hn = 0;
  int msgsize;

  //
  // Establishing a map and an ordered vector to pack a string table
  //
  for ( pos = proctable_copy.begin(); pos != proctable_copy.end(); ++pos )
    {
      for( vpos = pos->second.begin(); vpos != pos->second.end(); ++vpos )
        {
          map<string, unsigned int>::const_iterator finditer;
          finditer = execHostName.find ( string((*vpos)->pd.executable_name) );
	  if ( finditer == execHostName.end () )
	    {
	      execHostName[string((*vpos)->pd.executable_name)] = offset;
	      orderedEHName.push_back( (*vpos)->pd.executable_name );
	      num_unique_exec++;
	      offset += ( strlen ( (*vpos)->pd.executable_name ) + 1 );
	    }

          finditer = execHostName.find (string((*vpos)->pd.host_name));
	  if ( finditer == execHostName.end())
	    {
	      execHostName[string((*vpos)->pd.host_name)] = offset;
	      orderedEHName.push_back ((*vpos)->pd.host_name);
	      num_unique_hn++;
	      offset += ( strlen ( (*vpos)->pd.host_name ) + 1 );
	    }
	}
    }

  //
  // This message can be rather long as the size is
  // an lmonp header size + 16B per-task entry for each task
  // + the string table size.
  // The fixed 16 Byte per-task entry consists of 
  // exec index, hostname index, pid, and rank, each of which
  // is 4 Byte.
  //
  msgsize = sizeof(lmonp_t) 
            + 4*sizeof(int)*pcount + offset;
  lmonp_t *sendbuf = (lmonp_t *) malloc ( msgsize );
  memset ( sendbuf, 0, msgsize );
  if ( pcount < LMON_NTASKS_THRE) 
    {
      set_msg_header ((lmonp_t*) sendbuf, 
		  lmonp_fetofe, 
		  (int)t, 
		  pcount, 
		  0,
		  num_unique_exec,
		  num_unique_hn,
		  0,
		  (4*sizeof(int)*pcount)+offset,
		  0);
    }
  else
    {
      set_msg_header ((lmonp_t*) sendbuf, 
		  lmonp_fetofe, 
		  (int)t, 
		  LMON_NTASKS_THRE, 
		  0,
		  num_unique_exec,
		  num_unique_hn,
		  pcount,
		  (4*sizeof(int)*pcount)+offset,
		  0);
    }

  char *payload_cp_ptr = get_lmonpayload_begin (sendbuf);

  //
  // serializing the process table into a send buffer.
  //
  for ( pos = proctable_copy.begin(); 
            pos != proctable_copy.end(); ++pos )
    {
      for(vpos = pos->second.begin(); 
               vpos != pos->second.end(); ++vpos )
        {
	  memcpy ( payload_cp_ptr,
		   &( execHostName[string((*vpos)->pd.host_name)] ),
		   sizeof ( unsigned int ) );
	  payload_cp_ptr += sizeof ( unsigned int );
	  
	  memcpy ( payload_cp_ptr,
		   &( execHostName[string((*vpos)->pd.executable_name)] ),
		   sizeof ( unsigned int ) );
	  payload_cp_ptr += sizeof ( unsigned int );
	  
	  memcpy ( payload_cp_ptr,
		   &( (*vpos)->pd.pid ),
		   sizeof (int) );
	  payload_cp_ptr += sizeof ( int );
	  
	  memcpy ( payload_cp_ptr,
		   &( (*vpos)->mpirank ),
		   sizeof ( int ) );
	  payload_cp_ptr += sizeof ( int ); 
	}
    }
 

  //
  // serializing the string table into the send buffer. 
  //
  for ( EHpos = orderedEHName.begin(); 
              EHpos != orderedEHName.end(); ++EHpos )
    {
      int leng = strlen ((*EHpos)) + 1;
      memcpy ( payload_cp_ptr, ( *EHpos ),
	       leng );
      payload_cp_ptr += leng;
    }
  
  write_lmonp_long_msg ( get_FE_sockfd(),
			 (lmonp_t*) sendbuf, 
			 msgsize );

  {    
    self_trace_t::trace ( LEVELCHK(level2), 
			  MODULENAME, 0, 
     "a proctable message shipped out"); 
  }
  
  free(sendbuf);

  return LAUNCHMON_OK;
}


//! ship_resourcehandle_msg
/*!
    sends an lmonp_t packet with { msgclass=lmonp_fetofe, 
    type.fetofe_type=lmonp_febe_proctab,
    lmon_payload_length=size of the proctable }
    to the FE API stub.  
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
launchmon_rc_e 
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::ship_resourcehandle_msg ( 
                lmonp_fe_to_fe_msg_e t, int rid )
{
  using namespace std;

  lmonp_t msg;
  int msgsize;
  char *sendbuf;
  char *payload_cp_ptr;  

  //
  // If the engine is not driven via the FE API, this method
  // returns LAUNCHMON_FAILED
  //  
  if ( !get_API_mode() )
    {    
      self_trace_t::trace ( LEVELCHK(level3), 
	     MODULENAME, 0, 
	     "standalone mode does not ship resource handle via LMONP");
      return LAUNCHMON_FAILED;
    }

  init_msg_header (&msg);
  msg.msgclass = lmonp_fetofe;
  msg.type.fetofe_type = t;
  msg.lmon_payload_length = sizeof(rid);
  msgsize = sizeof(msg) + msg.lmon_payload_length + msg.usr_payload_length; 
  sendbuf = (char*) malloc (msgsize); 
  
  payload_cp_ptr = sendbuf;
  memcpy (payload_cp_ptr, &msg, sizeof(msg));  
  payload_cp_ptr += sizeof(msg);
  memcpy(payload_cp_ptr, &rid, sizeof(rid));
  
  write_lmonp_long_msg ( get_FE_sockfd(),
			 (lmonp_t*)sendbuf,
			 msgsize );  

  {    
    self_trace_t::trace ( LEVELCHK(level2), 
			  MODULENAME, 0, 
     "a reshandle message shipped out"); 
  }

  free(sendbuf);  

  return LAUNCHMON_OK;
}


//! ship_resourcehandle_msg
/*!
    sends an lmonp_t packet with { msgclass=lmonp_fetofe, 
    type.fetofe_type=lmonp_febe_proctab,
    lmon_payload_length=size of the proctable }
    to the FE API stub.  
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
launchmon_rc_e 
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::ship_rminfo_msg ( 
                lmonp_fe_to_fe_msg_e t, int rmpid, rm_catalogue_e rmtype)
{
  lmonp_t msg;
  int msgsize;
  uint32_t rminfo_pair[2];
  char *sendbuf;
  char *payload_cp_ptr;  

  //
  // If the engine is not driven via the FE API, this method
  // returns LAUNCHMON_FAILED
  //  
  if ( !get_API_mode() )
    {    
      self_trace_t::trace ( LEVELCHK(level3), 
	     MODULENAME, 0, 
	     "standalone mode does not ship resource handle via LMONP");
      return LAUNCHMON_FAILED;
    }

  rminfo_pair[0] = (uint32_t) rmpid;
  rminfo_pair[1] = (uint32_t) rmtype;

  init_msg_header (&msg);
  msg.msgclass = lmonp_fetofe;
  msg.type.fetofe_type = t;
  msg.lmon_payload_length = sizeof(rminfo_pair);
  msgsize = sizeof(msg) + msg.lmon_payload_length + msg.usr_payload_length; 
  sendbuf = (char*) malloc (msgsize); 
  
  payload_cp_ptr = sendbuf;
  memcpy (payload_cp_ptr, &msg, sizeof(msg));  
  payload_cp_ptr += sizeof(msg);
  memcpy(payload_cp_ptr, rminfo_pair, sizeof(rminfo_pair));
  
  write_lmonp_long_msg ( get_FE_sockfd(),
			 (lmonp_t *)sendbuf,
			 msgsize );  

  {    
    self_trace_t::trace ( LEVELCHK(level2), 
			  MODULENAME, 0, 
     "a reshandle message shipped out"); 
  }

  free(sendbuf);  

  return LAUNCHMON_OK;

}


//! handle_incoming_socket_event
/*!
  handles an lmonp_t packet received from the FE API stub. 
*/
template <SDBG_DEFAULT_TEMPLATE_WIDTH>
launchmon_rc_e 
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::handle_incoming_socket_event (
                process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p )
{  
  try
    {
      struct pollfd fds[1];
      int pollret;
      int timeout = 0;
      double timestamp;

      if ( !get_API_mode() )
	{    
	  self_trace_t::trace ( LEVELCHK(level3), 
	     MODULENAME, 0, 
	     "standalone launchmon does not have to poll the FE socket"); 
 
	  return LAUNCHMON_OK;
	}

      //
      // poll should return positive ret code only when one or more messages 
      // reported onto this file descriptor.
      // 
      fds[0].fd = get_FE_sockfd();
      fds[0].events = POLLIN;
      //fds[0].events = POLLRDBAND;

      timestamp = gettimeofdayD ();
      if ( timestamp > (last_seen + warm_period) )     
        timeout = 10; /* 10 milisecond blocking if no event */

      do {
	pollret = poll ( fds, 1, timeout );
      } while ( (pollret < 0) && (errno == EINTR) );
	  
      if ( pollret > 0 )
	{
#if VERBOSE
	   self_trace_t::trace ( LEVELCHK(level1), 
	   MODULENAME, 0, 
	   "poll returned an event ");  
#endif
          if ( fds[0].revents & POLLIN) { 
	    int numbytes;
	    lmonp_t msg; 

	    init_msg_header (&msg);
	    numbytes = read_lmonp_msgheader ( get_FE_sockfd(), &msg);

            //
	    // First handle socket disconnection and oddly formed message
	    //
            if ( numbytes == -1)
              {
		//
		// A.C.1. If the tool FE fails, the engine first detects the socket 
                // disconnection, at which point it tries to kill  the  RM_daemon  
		// process and detaches  from  the  RM_job  process.  However, 
	        // if for some reason the engine also gets into trouble, the engine 
	        // would perform  A.1  instead; obviously  in  this  case,  the failing 
		// launchmon engine will keep the RM_daemon process running, and 
                // won't be able to do A.1.3.
		//
	        // This attempts to stop all of the threads
                // and mark "detach." If a kill/detach request has been already registered, 
                // don't bother
                self_trace_t::trace ( LEVELCHK(level1),
                  MODULENAME,
                  0,
                  "The channel with front-end disconnected. Starting cleanup...");

                request_detach(p, FE_disconnected); 
                goto ret_ok;
              } // if ( numbytes == -1)
	    else if ( (numbytes != sizeof (msg))
		      || ((numbytes == sizeof (msg))
		          && (msg.msgclass != lmonp_fetofe)) )
	      {
	        self_trace_t::trace ( LEVELCHK(level1), 
	          MODULENAME, 1, 
		  "read_lmonp_msgheader pulled out a ill-formed message");  

	        // FAILED is a misnomer in this case
	        goto ret_fail;
	      }

 	    //
	    // OK, FE socket is connected and we received a legit message 
	    //
	    //
	    switch (msg.type.fetofe_type)
              {
	      case lmonp_detach:
                {
	          // This attempts to stop all of the threads
                  // and mark "kill/detach." If a kill/detach request has been already registered, 
                  // don't bother
                  self_trace_t::trace ( LEVELCHK(level1),
                    MODULENAME,
                    0,
                    "front-end requested detach...");
                  request_detach(p, FE_requested_detach);
                  break;
		}
	      case lmonp_kill:
	        {
	          // This attempts to stop all of the threads
                  // and mark "kill" If a kill/detach request has been already registered, 
                  // don't bother
                  self_trace_t::trace ( LEVELCHK(level1),
                    MODULENAME,
                    0,
                    "front-end requested kill...");
                  request_kill(p, FE_requested_kill); 
                  break;
                }  
              case lmonp_shutdownbe:
                {
	          // This attempts to stop all of the threads
                  // and mark "detach" If a kill/detach request has been already registered, 
                  // don't bother
                  self_trace_t::trace ( LEVELCHK(level1),
                    MODULENAME,
                    0,
                    "front-end requested deamon shutdown...");
                  request_detach(p, FE_requested_shutdown_dmon);
                  break;
		}
             default:
	       {
                  self_trace_t::trace ( LEVELCHK(level1),
                    MODULENAME, 1,
                    "ill-formed msg");

		 goto ret_fail; 
	       }
            } // switch	
          }
        } // if (pollret 

ret_ok:
      //
      // this should cause the event loop to continue
      //
      return LAUNCHMON_OK;
ret_fail:
      //
      // this should cause the event loop to exit
      //
      return LAUNCHMON_FAILED;	
    }
  catch ( symtab_exception_t e ) 
    {
      e.report();
      //
      // when a symtab exception is thrown, we catch and report it  
      // and return the failed code. So the caller must handle 
      // this code.
      //
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e ) 
    {
      e.report();
      //
      // when a symtab exception is thrown, we catch and report it  
      // and return the failed code. So the caller must handle 
      // this code.
      //
      return LAUNCHMON_FAILED;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      //
      // when a symtab exception is thrown, we catch and report it  
      // and return the failed code. So the caller must handle 
      // this code.
      //
      return LAUNCHMON_FAILED;
    }
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
launchmon_rc_e
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::handle_daemon_exit_event
                 ( process_base_t<SDBG_DEFAULT_TEMPLPARAM> &p )
{
  try
    {
      // This attempts to stop all of the threads
      // and mark "detach" If a kill/detach request has been already registered, 
      // don't bother
      self_trace_t::trace ( LEVELCHK(level1),
        MODULENAME,
        0,
        "daemon exited...");
      request_detach(p, RM_BE_daemon_exited);
      set_last_seen (gettimeofdayD ());
      return LAUNCHMON_OK;
    }
  catch ( symtab_exception_t e )
    {
      e.report();
      //
      // when a symtab exception is thrown, we catch and report it  
      // and return the failed code. So the caller must handle 
      // this code.
      //
      return LAUNCHMON_FAILED;
    }
  catch ( tracer_exception_t e )
    {
      e.report();
      //
      // when a symtab exception is thrown, we catch and report it  
      // and return the failed code. So the caller must handle 
      // this code.
      //
      return LAUNCHMON_FAILED;
    }
  catch ( machine_exception_t e )
    {
      e.report();
      //
      // when a symtab exception is thrown, we catch and report it  
      // and return the failed code. So the caller must handle 
      // this code.
      //
      return LAUNCHMON_FAILED;
    }
}


template <SDBG_DEFAULT_TEMPLATE_WIDTH>
bool
launchmon_base_t<SDBG_DEFAULT_TEMPLPARAM>::validate_mpir_state_transition(int s)
{
  bool rc = false;
  switch(s)
    {
    case MPIR_DEBUG_SPAWNED:
      if ( (engine_state == mpir_start) 
           || (engine_state == mpir_null) )
        {
          rc = true;
        }
      break;
    case MPIR_DEBUG_ABORTING: 
      if ( (engine_state == mpir_start) 
           || (engine_state == mpir_null) 
           || (engine_state == mpir_spawned) )
        {
          rc = true;
        }
      break;  
    case MPIR_NULL:
      if ( (engine_state == mpir_start) 
           || (engine_state == mpir_null) 
           || (engine_state == mpir_spawned) )
        {
          rc = true;
        }
      break;
    default:
      break;
    }

  return rc;
}

#endif // SDBG_BASE_LAUNCHMON_IMPL_HXX
