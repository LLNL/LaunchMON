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
 *        Sep 12 2011 DHA: Added be_fail_detection_supported support
 *                         for the orphaned alps_fe_colocat problem
 *                         (ID: 3408210).
 *        Nov 22 2010 DHA: Includes limits.h
 *        Oct 22 2010 DHA: Added support for launching of tool daemons
 *                         into a subset of an allocation under SLURM.
 *                         (ID: 2871927).
 *        Jun 28 2010 DHA: Moved the rm_catalogue_e defintition into the
 *                         LMON API standard header. 
 *        Jun 09 2010 DHA: Created file.
 */

#ifndef SDBG_RM_MAP_HXX
#define SDBG_RM_MAP_HXX 1

#include "sdbg_std.hxx"

#include <cstdio>
#if HAVE_SIGNAL_H
# include <signal.h>
#else
# error signal.h is required
#endif
#if HAVE_LIMITS_H
# include <limits.h>
#else
# error limits.h is required
#endif


#include <string>
#include <lmon_api/lmon_api_std.h>

struct coloc_str_param_t
{
  int nnodes;
  int ndaemons;
  char *sharedsec;
  char *randomid; 
  int resourceid; 
  int pmgrsize;
  char *pmgrip;
  char *pmgrport;
  int pmgrjobid; 
  char *hnfilename;
};

class rc_rm_t 
{
public:

  // ctor
  rc_rm_t()
   {
     rm_type             = RC_none; 	
     paramset.nnodes     = -1;
     paramset.ndaemons   = -1;
     paramset.sharedsec  = NULL;
     paramset.randomid   = NULL; 
     paramset.resourceid = -1; 
     paramset.pmgrsize   = -1;
     paramset.pmgrip     = NULL;
     paramset.pmgrport   = NULL;
     paramset.pmgrjobid  = -1;       
     paramset.hnfilename = NULL;
   }

  // don't use operations requiring a copy ctor

  // dtor
  ~rc_rm_t() 
    {
      if (paramset.sharedsec)
        free(paramset.sharedsec);
      if (paramset.randomid)
        free(paramset.randomid);
      if (paramset.pmgrip)
        free(paramset.pmgrip);
      if (paramset.pmgrport)
	free(paramset.pmgrport);
      if (paramset.hnfilename)
	free(paramset.hnfilename);
    }
  
  // init must be called after ctor
  // 1 arg: launcher path (lmon's debug target)
  // 2 arg: daemon path 
  // 3 arg: daemon opt and args
  // 4 arg: be daemon stub path for a system that needs it
  bool init(std::string &lnchrpath, std::string &tdpath, std::string &tdopts, std::string bestubpath="")
    {
      using std::string;

      if (!set_rmtype_on_lncher(lnchrpath.c_str())) 
        return false;

      bool rc = true;

      switch (rm_type) 
        {
        case RC_mchecker_rm:
	  rmname = "RC_mchecker_rm";
  	  has_mpir_coloc = true; 
          rid_supported = false;
          be_fail_detection_supported = false;
	  has_mpir_attach_fifo = false;
	  rm_attach_fifo_path = "";
          has_rm_so = false;
          rm_so_name = "";
   	  rm_daemon_path = tdpath;
          rm_daemon_stub = bestubpath; 
  	  rm_coloc_cmd = "coloc";
          rm_coloc_str = tdopts; 
          break;
 
        case RC_slurm:
  	  rmname = "RC_slurm";
  	  has_mpir_coloc = false; 
          rid_supported = true;
          be_fail_detection_supported = true;
	  has_mpir_attach_fifo = false;
	  rm_attach_fifo_path = "";
          has_rm_so = false;
          rm_so_name = "";
	  rm_daemon_path = tdpath;
          rm_daemon_stub = bestubpath; 
  	  rm_coloc_cmd = lnchrpath;
#ifdef TVCMD
          if ( getenv("LMON_DEBUG_BES") != NULL)
            rm_coloc_cmd = TVCMD + std::string(" ") + rm_coloc_cmd + std::string(" -a");        
#endif
          rm_coloc_str = rm_coloc_cmd;
	  rm_coloc_str += std::string(" --input=none --jobid=%d --nodes=%d --ntasks=%d --nodelist=%s ")
            + rm_daemon_path + std::string(" ") + tdopts
#if PMGR_BASED 
            + std::string(" --pmgrsize=%d --pmgrip=%s --pmgrport=%s --pmgrjobid=%d --pmgrlazyrank=1 --pmgrlazysize=1")
#endif
            + std::string(" --lmonsharedsec=%s --lmonsecchk=%s");
          break;

        case RC_bglrm:
  	case RC_bgprm:
        case RC_bgqrm:
	case RC_bgrm:
	  rmname = "RC_bgrm";
  	  has_mpir_coloc = true; 
          rid_supported = false;
          be_fail_detection_supported = false;
	  has_mpir_attach_fifo = false;
	  rm_attach_fifo_path = "";
          has_rm_so = false;
          rm_so_name = "";
	  rm_daemon_path = tdpath;
          rm_daemon_stub = bestubpath; 
  	  rm_coloc_cmd = "coloc";
          rm_coloc_str = tdopts
#if PMGR_BASED // TODO: check why this string doesn't have --pmgrsize=%d?
            + std::string(" --pmgrip=%s --pmgrport=%s --pmgrjobid=%d --pmgrlazyrank=1 --pmgrlazysize=1")
#endif
	    + std::string(" --lmonsharedsec=%s --lmonsecchk=%s");
          break;

        case RC_alps:
	  rmname = "RC_alps";
  	  has_mpir_coloc = false; 
          rid_supported = false;
          be_fail_detection_supported = false;
	  has_mpir_attach_fifo = false;
	  rm_attach_fifo_path = "";
          has_rm_so = false;
          rm_so_name = "";
	  rm_daemon_path = tdpath;
	  rm_daemon_stub = bestubpath;
  	  rm_coloc_cmd = lnchrpath;
          rm_coloc_str = rm_coloc_cmd + std::string(" --be_starter=") + rm_daemon_stub
            + (" --apid=%d --daemon=") + rm_daemon_path + std::string(" ") + tdopts 
#if PMGR_BASED 
            + std::string(" --pmgrip=%s --pmgrport=%s --pmgrjobid=%d --pmgrlazyrank=1 --pmgrlazysize=1")
#endif
	    + std::string(" --lmonsharedsec=%s --lmonsecchk=%s ");
          break;

	case RC_orte:
	  rmname = "RC_orte";
  	  has_mpir_coloc = true; 
          rid_supported = false;
          be_fail_detection_supported = false; 
	  has_mpir_attach_fifo = true;
          has_rm_so = true;
          rm_so_name = "libopen-rte.so";
	  rm_attach_fifo_path = "";
	  rm_daemon_path = tdpath;
	  rm_daemon_stub = bestubpath;
  	  rm_coloc_cmd = "coloc";
          rm_coloc_str = tdopts
#if PMGR_BASED 
            + std::string (" --pmgrip=%s --pmgrport=%s --pmgrjobid=%d --pmgrlazyrank=1 --pmgrlazysize=1")
#endif
	    + std::string(" --lmonsharedsec=%s --lmonsecchk=%s");
          break;

	case RC_none:
        default:
	  rc = false;
          break;
        } // switch (rm_type)

      return rc;
    }

  //
  // setting param set w/o pmgr
  //
  bool set_paramset (int nn, int nd, char *ss, char *ri, int rid, char *hns)
    {
      if (!ss || !ri)
        return false;
 
      paramset.nnodes = nn;
      paramset.ndaemons = nd;
      paramset.sharedsec = strdup(ss);
      paramset.randomid = strdup(ri);
      paramset.resourceid = rid;
      if (hns)
        {
          paramset.hnfilename= strdup(hns);
        }
  
      return true;
    }
  
  //
  // setting param set w pmgr
  //
  bool set_paramset (int nn, int nd, char *ss, char *ri, int rid, char *hns, 
                     int pmsize, char *pmip, char *pmport, int pmjid)
    {
      if (!pmip || !pmport)
        return false;

      if (!set_paramset (nn, nd, ss, ri, rid, hns))
        return false;

      paramset.pmgrsize = pmsize;
      paramset.pmgrip = strdup(pmip);
      paramset.pmgrport = strdup(pmport);
      paramset.pmgrjobid = pmjid; 

      return true;
    }

  // 
  // expand the colocation string and return the expanded string 
  // 
  std::string expand_coloc_str()
    {
      bool rc = true;
      char expanded_string[PATH_MAX];
      rm_coloc_exp_str = "";

      switch (rm_type) 
        {
        case RC_mchecker_rm:
	  /* need nothing */	
          break;
 
        case RC_slurm:
	  snprintf(expanded_string, 
		   PATH_MAX,
		   rm_coloc_str.c_str(),
		   paramset.resourceid,
		   paramset.nnodes,
		   paramset.ndaemons,
                   paramset.hnfilename,
#if PMGR_BASED
		   paramset.pmgrsize, 
		   paramset.pmgrip,
		   paramset.pmgrport,
		   paramset.pmgrjobid,
#endif
		   paramset.sharedsec,
		   paramset.randomid);	

          rm_coloc_exp_str = expanded_string;
          break;

        case RC_bglrm:
  	case RC_bgprm:
        case RC_bgqrm:
	case RC_bgrm:
	  snprintf(expanded_string, 
		   PATH_MAX,
		   rm_coloc_str.c_str(),
#if PMGR_BASED 
		   paramset.pmgrip,
		   paramset.pmgrport,
		   paramset.pmgrjobid,
#endif
		   paramset.sharedsec,
		   paramset.randomid);	

          rm_coloc_exp_str = expanded_string;
          break;

        case RC_alps:
	  snprintf(expanded_string, 
		   PATH_MAX,
		   rm_coloc_str.c_str(),
		   paramset.resourceid,
#if PMGR_BASED 
		   paramset.pmgrip,
		   paramset.pmgrport,
		   paramset.pmgrjobid,
#endif
		   paramset.sharedsec,
		   paramset.randomid);	

          rm_coloc_exp_str = expanded_string;
          break;

	case RC_orte:
	  snprintf(expanded_string, 
		   PATH_MAX,
		   rm_coloc_str.c_str(),
#if PMGR_BASED 
		   paramset.pmgrip,
		   paramset.pmgrport,
		   paramset.pmgrjobid,
#endif
		   paramset.sharedsec,
		   paramset.randomid);	

          rm_coloc_exp_str = expanded_string;
          break;

	case RC_none:
        default:
	  rc = false;
          break;
        } // switch (rm_type)
 
      return rm_coloc_exp_str;
    }

  bool graceful_rmkill(int pid)
    {
       if (pid < 0) 
	 return false;

       bool rc = true;	
       switch (rm_type) 
        {
        case RC_mchecker_rm:
	  /* need nothing */	
          break;
 
        case RC_slurm:
	  usleep (GracePeriodBNSignals);
          kill ( pid, SIGINT);
          usleep (GracePeriodBNSignals);
          kill ( pid, SIGINT);
          usleep (GracePeriodBNSignals);
	  break;

	case RC_bglrm:
  	case RC_bgprm:
        case RC_bgqrm:
	case RC_bgrm:
	  usleep (GracePeriodBNSignals);
          kill ( pid, SIGINT);
	  usleep (GracePeriodBNSignals);
	  break;

	case RC_alps:
	  usleep (GracePeriodBNSignals);
          kill ( pid, SIGINT);
	  usleep (GracePeriodBNSignals);
	  break;

	case RC_orte:
	  usleep (GracePeriodBNSignals);
          kill ( pid, SIGINT);
	  usleep (GracePeriodBNSignals);
	  break;

        case RC_none:
	default:
          rc = false;
	  break;
	}

      return rc;
    }

  static rm_catalogue_e get_configured_rmtype ()
    {
      rm_catalogue_e rc = RC_none;
#ifdef RM_SLURM_SRUN
      rc = RC_slurm;
#elif RM_ORTE_ORTERUN
      rc = RC_orte;
#elif RM_BG_MPIRUN
      rc = RC_bgrm;
#elif RM_ALPS_APRUN
      rc = RC_alps;
#endif
      return rc;
    }

  define_gset(bool, has_mpir_coloc)
  define_gset(bool, rid_supported)
  define_gset(bool, be_fail_detection_supported) 
  define_gset(bool, has_mpir_attach_fifo)
  define_gset(std::string, rm_daemon_path)
  define_gset(rm_catalogue_e, rm_type)
  define_gset(std::string, rm_attach_fifo_path)
  define_gset(bool, has_rm_so)
  define_gset(std::string, rm_so_name)
  
  const char * get_hostnames_fn()
    {
      return paramset.hnfilename;
    }

private:

  bool set_rmtype_on_lncher(const char *lnchrpath)
    {
      using std::string;

      bool rc = true;	

      // dt must not null  
      char *bnbuf = strdup(lnchrpath);
      char *dt = basename(bnbuf);

      //
      // dt must be a basename
      // 
      if (dt == string("srun") || dt == string("lt-srun")) 
        {
          rm_type = RC_slurm;
        }
#ifdef RM_FE_COLOC_CMD
      else if (dt == string (RM_FE_COLOC_CMD))
#else
      else if (dt == string ("alps_fe_colocator"))
#endif
        {
          rm_type = RC_alps;
        }
      else if (dt == string ("mpirun") 
               || dt == string ("mpirun32") 
               || dt == string("mpirun64"))
        {
          rm_type = RC_bgrm;
        }
      else if (dt == string("orterun"))
        {
          rm_type = RC_orte;
        }
      else if (dt == string ("LE_model_checker.debug") 
               || dt == string ("LE_model_checker"))
	{
	  rm_type = RC_mchecker_rm;
	}
      else
        {
	  rc = false;
	}

      free(bnbuf);
      return rc;
    }

  //
  // private members
  //
  //

  // RM type
  rm_catalogue_e rm_type;

  // RM name
  std::string rmname;

  // Does the RM support native MPIR colocation service 
  bool has_mpir_coloc; 

  // Some RM supports resource ID	
  bool rid_supported;

  // back-end failure detection supported
  bool be_fail_detection_supported;

  // for MPIR_attach_fifo supported RM
  bool has_mpir_attach_fifo;

  // daemon path
  std::string rm_daemon_path;
  
  // daemon stub 
  std::string rm_daemon_stub;

  // command for colocation service for non-MPIR colocation cases
  // typically launcher itself but can be a different command
  std::string rm_coloc_cmd; 

  // option/args string to rm_coloc_cmd or MPIR colocation 
  std::string rm_coloc_str;

  // fully expanded string of rm_coloc_str
  std::string rm_coloc_exp_str;

  // fifo path
  std::string rm_attach_fifo_path;

  // any RM SO? (FLAG string for now... later vector might be needed
  bool has_rm_so;
  std::string rm_so_name;

  // parameter set
  coloc_str_param_t paramset;
};

#endif // SDBG_RM_MAP_HXX

