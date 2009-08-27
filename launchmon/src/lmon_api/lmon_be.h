/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/lmon_api/lmon_be.h,v 1.7.2.4 2008/02/21 09:26:38 dahn Exp $
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
 *        Aug  26 2009 DHA: lmon-config.h support
 *        Jun  06 2008 DHA: Remove description comments; the man pages
 *                          now contain most up-to-date info.
 *        Feb  09 2008 DHA: Added LLNS Copyright 
 *        Nov  07 2007 DHA: Rewrite API descriptions
 *        Aug  10 2007 DHA: LMON_be_recvUsrData added         
 *        Dec  20 2006 DHA: Created file.          
 */

#ifndef LMON_API_LMON_BE_H
#define LMON_API_LMON_BE_H 1

#if LAUNCHMON_HAVE_UNISTD_H
# include <unistd.h>
#else
# error unistd.h is required
#endif

#if LAUNCHMON_HAVE_NETDB_H
# include <netdb.h>
#else
# error netdb.h is required
#endif

#include <lmon_api/lmon_api_std.h>
#include <lmon_api/lmon_proctab.h>

BEGIN_C_DECLS

lmon_rc_e LMON_be_init ( int ver, int* argc, char*** argv );

lmon_rc_e LMON_be_amIMaster ();

lmon_rc_e LMON_be_getMyRank ( int *rank );

lmon_rc_e LMON_be_getSize ( int* size );

lmon_rc_e LMON_be_barrier ();

lmon_rc_e LMON_be_broadcast ( 
                void* buf, 
                int numbyte );

lmon_rc_e LMON_be_gather ( 
                void *sendbuf,
                int numbyte_per_elem,
                void* recvbuf );

lmon_rc_e LMON_be_scatter ( 
                void *sendbuf,
                int numbyte_per_element,
                void* recvbuf );

lmon_rc_e LMON_be_finalize ();

lmon_rc_e LMON_be_handshake ( void* udata );

lmon_rc_e LMON_be_ready ( void* udata );

lmon_rc_e LMON_be_getMyProctab ( 
                MPIR_PROCDESC_EXT *proctabbuf, 
                int *size, 
                int proctab_num_elem );

lmon_rc_e LMON_be_getMyProctabSize (
		int *size );

lmon_rc_e LMON_be_regPackForBeToFe (
                int (*packBefe) 
                ( void* udata,void* msgbuf,int msgbufmax,int* msgbuflen ) );

lmon_rc_e LMON_be_regUnpackForFeToBe (
                int (*unpackFebe) 
                ( void* udatabuf,int udatabuflen, void* udata  ) );

lmon_rc_e LMON_be_recvUsrData ( void* udata );

lmon_rc_e LMON_be_sendUsrData ( void* udata );

lmon_rc_e LMON_be_regErrorCB ( int (*errorCB) (const char *format, va_list ap) );

END_C_DECLS

#endif /* LMON_API_LMON_BE_H */
