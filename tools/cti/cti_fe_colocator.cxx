/*
 * $Header: Exp $
 *--------------------------------------------------------------------------------
 *
 * Copyright 2015 Cray Inc. All Rights Reserved.
 *
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
 *        Mar 31 2015 andrewg@cray.com: File created
 */

extern "C" {
#include <getopt.h>
#include <signal.h>
#include <stdint.h>
#include <limits.h>
}

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <iostream>
#include <string>
#include <vector>

#include "sdbg_std.hxx"

#include "cray_tools_fe.h"

const char MSGPREFIX[] = "CRAY CTI FE COLOCATOR";

const struct option long_opts[] = {
	{"apid",		required_argument, 0, 'a'},
	{"daemon",		required_argument, 0, 'p'},
	{0, 0, 0, 0}
};

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

int
main (int argc, char *argv[])
{
	int c;
	int opt_ind = 0;
	pid_t aprun_pid;
	uint64_t apid;
	char* eptr;
	std::string daemonpath = "";
	std::vector<char *> dargs;
	char ** c_dargs;
	bool a_arg = false;
	bool p_arg = false;
	// cti variables
	cti_app_id_t myapp;
	cti_session_id_t mysid;
	cti_manifest_id_t mymid;
	
	opterr = 0;
	while ((c = getopt_long(argc, argv, "a:p:", long_opts, &opt_ind)) != -1)
	{
		switch (c)
		{
			case 'a':
			
				if (optarg == NULL)
				{
					ALPS_say_msg(MSGPREFIX, true, "Missing --apid argument.");
					return EXIT_FAILURE; 
				}
				
				// This is the pid of the aprun process
				errno = 0;
				aprun_pid = (pid_t)strtol(optarg, &eptr, 10);
				
				// check for error
				if ((errno == ERANGE && (aprun_pid == LONG_MAX || aprun_pid == LONG_MIN))
						|| (errno != 0 && aprun_pid == 0))
				{
					ALPS_say_msg(MSGPREFIX, true, "strtol: %s", strerror(errno));
					return EXIT_FAILURE; 
				}
				
				// check for invalid input
				if (eptr == optarg || *eptr != '\0')
				{
					ALPS_say_msg(MSGPREFIX, true, "Invalid --apid argument.");
					return EXIT_FAILURE;
				}
				
				a_arg = true;
				
				break;
			
			case 'p':
			
				if (optarg == NULL)
				{
					ALPS_say_msg(MSGPREFIX, true, "Missing --daemon argument.");
					return EXIT_FAILURE; 
				}
				
				// This is the backend daemon
				daemonpath = optarg;
				
				p_arg = true;
				
				break;
				
			default:
				
				break;
		}
	}
	
	// grab extra args
	int ix;
	for (ix=optind; ix < argc; ix++)
	{
		dargs.push_back(strdup(argv[ix]));
	}
	
	// create the args array
	c_dargs = (char **)malloc(sizeof(char *) * dargs.size() + 1);
	if (c_dargs == NULL)
	{
		ALPS_say_msg(MSGPREFIX, true, "malloc failed.");
		return EXIT_FAILURE;
	}
	ix = 0;
	for (std::vector<char *>::iterator iter = dargs.begin(); iter != dargs.end(); iter++)
	{
		c_dargs[ix++] = *iter;
	}
	c_dargs[ix] = NULL;
	
	// ensure all required args were provided
	if (!(p_arg && a_arg))
	{
		if (!a_arg)
		{
			ALPS_say_msg(MSGPREFIX, true, "Missing --apid argument.");
		}
		if (!p_arg)
		{
			ALPS_say_msg(MSGPREFIX, true, "Missing --daemon argument.");
		}
		return EXIT_FAILURE; 
	}
	
	// convert the aprun pid to an apid
	apid = cti_alps_getApid(aprun_pid);
	if (apid == 0)
	{
		ALPS_say_msg(MSGPREFIX, true, "CTI error: %s", cti_error_str());
		return EXIT_FAILURE;
	}
	
	// register the apid with cti
	myapp = cti_alps_registerApid(apid);
	if (myapp == 0)
	{
		ALPS_say_msg(MSGPREFIX, true, "CTI error: %s", cti_error_str());
		return EXIT_FAILURE;
	}
	
	// create a session to stage files to
	if ((mysid = cti_createSession(myapp)) == 0)
	{
		ALPS_say_msg(MSGPREFIX, true, "CTI error: %s", cti_error_str());
		cti_deregisterApp(myapp);
		return EXIT_FAILURE;
	}
	
	// Create a manifest based on the session to put our daemon file into
	if ((mymid = cti_createManifest(mysid)) == 0)
	{
		ALPS_say_msg(MSGPREFIX, true, "CTI error: %s", cti_error_str());
		cti_deregisterApp(myapp);
		return EXIT_FAILURE;
	}
	
	// Transfer and exec the daemon
	if (cti_execToolDaemon(mymid, daemonpath.c_str(), (const char * const *)c_dargs, NULL))
	{
		ALPS_say_msg(MSGPREFIX, true, "CTI error: %s", cti_error_str());
		cti_deregisterApp(myapp);
		return EXIT_FAILURE;
	}
	
	// cleanup
	cti_deregisterApp(myapp);
	
	//
	// TODO: There is no monitoring service in CTI
	// determine the strategy to overcome this constraint
	//
	// For now, sleep with SIGINT signal handler installed
	// This way, launchmon engine won't enforce C.1
	signal(SIGINT, sighandler);
	while (1)
		sleep(3600);

	return EXIT_SUCCESS;
}
