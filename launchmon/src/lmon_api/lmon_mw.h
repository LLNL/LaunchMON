/*
 * $Header: /usr/gapps/asde/cvs-vault/sdb/launchmon/src/lmon_api/Attic/lmon_mw.h,v 1.1.2.3 2008/02/20 17:37:57 dahn Exp $
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
 *        Dec  23 2009 DHA: Removed header file macroes for header files that
 *                          would exit on almost all UNIX based platforms,
 *                               facilitaing binary distribution.
 *        Aug 26 2009 DHA: lmon-config.h support
 *        Feb 09 2008 DHA: Added LLNS Copyright 
 *        Dec 27 2006 DHA: Created file.          
 */

#ifndef LMON_API_LMON_BE_H
#define LMON_API_LMON_BE_H

# include <sys/types.h>
# include <unistd.h>

#include <lmon_api/lmon_api_std.h>
#include <lmon_api/lmon_proctab.h>

BEGIN_C_DECLS

lmon_rc_e LMON_mw_amIMaster ();

lmon_rc_e LMON_mw_init ( int ver, int *argc, char ***argv );

lmon_rc_e LMON_mw_barrier ();

lmon_rc_e LMON_mw_gather ( 
                void *sendbuf,
                int numbyte_per_elem,
                void *recvbuf );

lmon_rc_e LMON_mw_scatter ( 
                void *sendbuf,
                int numbyte_per_element,
                void *recvbuf );

lmon_rc_e LMON_mw_broadcast ( 
                void *buf, 
                int numbyte );

lmon_rc_e LMON_mw_getSize ( int *size );

lmon_rc_e LMON_mw_getMyRank ( int *rank );

lmon_rc_e LMON_mw_regPackForMwToFe (
                int (*packMwfe) 
                ( void *udata,void *msgbuf,int msgbufmax,int *msgbuflen ) );

lmon_rc_e LMON_mw_regUnpackForFeToMw (
                int (*unpackFebe) 
                ( void *udatabuf,int udatabuflen, void *udata  ) );

lmon_rc_e LMON_mw_handshake ( void *udata );

lmon_rc_e LMON_mw_ready ( void *udata );

lmon_rc_e LMON_mw_finalize ();

lmon_rc_e LMON_mw_recvUsrData ( void *udata );

lmon_rc_e LMON_mw_sendUsrData ( void *udata );

lmon_rc_e LMON_mw_getMyProctab ( 
                MPIR_PROCDESC_EXT *proctabbuf, 
                int *size, 
                int proctab_num_elem );

END_C_DECLS

#endif /* LMON_API_LMON_BE_H */
