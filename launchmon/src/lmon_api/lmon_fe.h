/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/lmon_api/lmon_fe.h,v 1.5.2.7 2008/02/21 19:34:32 dahn Exp $
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
 *
 *  Update Log:
 *        May  19 2008 DHA: Added LMON_fe_regErrorCB ( int (*func) (char *msg))
 *                          support.
 *        Mar  13 2008 DHA: Changed parameter data type to unsigned integer in 
 *                          LMON_fe_getProctableSize and LMON_fe_getProctable
 *                          to support extreme proctable sizes.
 *        Jun  06 2008 DHA: Remove description comment; the man pages 
 *                          now contain most up-to-date info.   
 *        Feb  09 2008 DHA: Added LLNS Copyright 
 *        Oct  29 2007 DHA: Rewrite API descriptions
 *        Sep  24 2007 DHA: Change FEBESessionHandle to sessionHandle
 *                          in anticipation for adding middleware API support.
 *                          Along the same line, fe_shutdownBe changed to 
 *                          fe_shutdownDaemons.
 *                          Expanding LMON_fe_sendUsrData into
 *                          LMON_fe_sendUsrDataBe and LMON_fe_sendUsrDataMw
 *                          vice versa for LMON_fe_recvUsrDataBe and 
 *                          LMON_fe_recvUsrDataMw
 *        Aug  15 2007 DHA: LMON_fe_kill, LMON_fe_shutdownBe added      
 *        Aug  13 2007 DHA: LMON_fe_sendUsrData added
 *        Aug  10 2007 DHA: LMON_fe_detach added                          
 *        Jul  27 2007 DHA: Format change
 *        Dec  15 2006 DHA: Created file.          
 */


#ifndef LMON_API_LMON_FE_H
#define LMON_API_LMON_FE_H


#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <lmon_api/lmon_api_std.h>

BEGIN_C_DECLS

lmon_rc_e LMON_fe_init ( int ver );

lmon_rc_e LMON_fe_createSession ( int *sessionHandle );

lmon_rc_e LMON_fe_regPackForFeToBe (
		int sessionHandle, 
		int (*packFebe) 
		(void* udata,void* msgbuf,int msgbufmax,int* msgbuflen));

lmon_rc_e LMON_fe_regUnpackForBeToFe (
	        int sessionHandle, 
	        int (*unpackBefe)
	        ( void* udatabuf,int udatabuflen, void* udata ));

lmon_rc_e LMON_fe_regPackForFeToMw (
		int sessionHandle, 
		int (*packFemw) 
		(void* udata,void* msgbuf,int msgbufmax,int* msgbuflen));

lmon_rc_e LMON_fe_regUnpackForMwToFe (
	        int sessionHandle, 
	        int (*unpackMwfe)
	        ( void* udatabuf,int udatabuflen, void* udata ));

lmon_rc_e LMON_fe_putToBeDaemonEnv (
                int sessionHandle, 
                lmon_daemon_env_t* dmonEnv, 
                int numElem );

lmon_rc_e LMON_fe_putToMwDaemonEnv (
                int sessionHandle, 
                lmon_daemon_env_t* dmonEnv, 
                int numElem);

lmon_rc_e LMON_fe_sendUsrDataBe ( int sessionHandle, void* febe_data );

lmon_rc_e LMON_fe_sendUsrDataMw ( int sessionHandle, void* femw_data );

lmon_rc_e LMON_fe_recvUsrDataBe ( int sessionHandle, void* befe_data );

lmon_rc_e LMON_fe_recvUsrDataMw ( int sessionHandle, void* mwfe_data );

lmon_rc_e LMON_fe_detach ( int sessionHandle );                

lmon_rc_e LMON_fe_kill ( int sessionHandle );                

lmon_rc_e LMON_fe_shutdownDaemons ( int sessionHandle );

lmon_rc_e LMON_fe_getStatus ( int sessionHandle, int *status );

lmon_rc_e LMON_fe_regStatusCB ( int (*func) (int *status) );

lmon_rc_e LMON_fe_regErrorCB ( int (*errorCB) (const char *format, va_list ap) );

lmon_rc_e LMON_fe_getProctable (
                int sessionHandle, 
                MPIR_PROCDESC_EXT* proctable, 
                unsigned int* size, 
                unsigned int maxlen);

lmon_rc_e LMON_fe_getProctableSize (
                int sessionHandle,
                unsigned int* size );

lmon_rc_e LMON_fe_getResourceHandle ( 
                int sessionHandle, 
                char* handle, 
                int* size, 
                int maxstring );

lmon_rc_e LMON_fe_launchAndSpawnDaemons ( 
		int sessionHandle, 
		const char* hostname,
		const char* launcher, 
		char* l_argv[], 
		const char* toolDaemon, 
		char* d_argv[], 
		void* febe_data, 
		void* befe_data );

lmon_rc_e LMON_fe_attachAndSpawnDaemons ( 
                int sessionHandle, 
		const char* hostname,
                pid_t launcherPid, 
                const char* toolDaemon, 
                char* d_argv[], 
                void* febe_data, 
                void* befe_data );

lmon_rc_e LMON_fe_launchMwDaemons (
                int sessionHandle, 
                int numNodes,
                int numDaemons,
                const char* commDaemon,
                char* d_argv[],
                void* femw_data,
                void* mwfe_data );

END_C_DECLS

#endif /* LMON_API_LMON_FE_H */
