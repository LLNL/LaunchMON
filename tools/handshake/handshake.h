/*
 * $Header: Exp $
 *--------------------------------------------------------------------------------
 * Copyright (c) 2014, Lawrence Livermore National Security, LLC. Produced at 
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
 *        Mar 10 2014 MPL: Add secure handshake
 */

#include <stdio.h>
#include <stdint.h>

#if !defined(handshake_h_)
#define handshake_h_

#if defined(__cplusplus)
extern "C" {
#endif

/* Error returns from handshake session */

#define HSHAKE_SUCCESS 0
#define HSHAKE_INTERNAL_ERROR -1
#define HSHAKE_DROP_CONNECTION -2
#define HSHAKE_CONNECTION_REFUSED -3
#define HSHAKE_ABORT -4

typedef enum {
   hs_none,         //No security validation in handshake
   hs_munge,        //Use munge 
   hs_key_in_file,  //Use gcrypt with key from file
   hs_explicit_key  //Use gcrypt with provided key
} handshake_security_t;

typedef struct {
   handshake_security_t mechanism;
   union {
      struct {
         //No data needed for none
      } none;
      struct {
         //No data needed for munge
      } munge;
      struct {
         char *key_filepath;
         int key_length_bytes;
      } key_in_file;
      struct {
         unsigned char *key;
         int key_length_bytes;
      } explicit_key;
   } data;
} handshake_protocol_t;

int handshake_server(int sockfd, handshake_protocol_t *hdata, uint64_t session_id);
int handshake_client(int sockfd, handshake_protocol_t *hdata, uint64_t session_id);
int handshake_is_security_type_enabled(handshake_security_t sectype);
char *handshake_last_error_str();

void handshake_enable_read_timeout(int timeout_sec);
void handshake_enable_debug_prints(FILE *debug_output);

void handshake_log_sec_error(const char *msg);

#if defined(__cplusplus)
}
#endif

#endif
