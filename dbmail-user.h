/*
 Copyright (c) 2005-2006 NFG Net Facilities Group BV support@nfg.nl

 This program is free software; you can redistribute it and/or 
 modify it under the terms of the GNU General Public License 
 as published by the Free Software Foundation; either 
 version 2 of the License, or (at your option) any later 
 version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* $Id: dbmail-user.h 2270 2006-09-17 05:04:54Z aaron $ 
 */
#ifndef _DBMAIL_USER_H
#define _DBMAIL_USER_H

#include "dbmail.h"

#define qverbosef(fmt, args...) (!verbose ? 0 : printf(fmt, ##args) )
#define qprintf(fmt, args...) ((quiet||reallyquiet) ? 0 : printf(fmt, ##args) )
#define qerrorf(fmt, args...) (reallyquiet ? 0 : fprintf(stderr, fmt, ##args) )

#define null_strncpy(dst, src, len) (src ? strncpy(dst, src, len) : 0 )
#define null_crypt(src, dst) (src ? crypt(src, dst) : "" )


/* These are the available password types. */
typedef enum {
	PLAINTEXT = 0, PLAINTEXT_RAW, CRYPT, CRYPT_RAW,
	MD5_HASH, MD5_HASH_RAW, MD5_DIGEST, MD5_DIGEST_RAW,
	MD5_BASE64, MD5_BASE64_RAW, SHADOW, PWTYPE_NULL
} pwtype_t;

int mkpassword(const char * const user, const char * const passwd,
               const char * const passwdtype, const char * const passwdfile,
               char ** password, char ** enctype);

struct change_flags {
	unsigned int newuser         : 1;
	unsigned int newmaxmail      : 1;
	unsigned int newclientid     : 1;
	unsigned int newpasswd       : 1;
	unsigned int newpasswdfile   : 1;
	unsigned int newpasswdstdin  : 1;
	unsigned int newpasswdshadow : 1;
};

/* The prodigious use of const ensures that programming
 * mistakes inside of these functions don't cause us to
 * use incorrect values when calling auth_ and db_ internals.
 * */

/* Core operations */
int do_add(const char * const user,
           const char * const password,
           const char * const enctype,
           const u64_t maxmail, const u64_t clientid,
	   GList * alias_add,
	   GList * alias_del);
int do_delete(const u64_t useridnr, const char * const user);
int do_show(const char * const user);
int do_empty(const u64_t useridnr);
/* Change operations */
int do_username(const u64_t useridnr, const char *newuser);
int do_maxmail(const u64_t useridnr, const u64_t maxmail);
int do_clientid(const u64_t useridnr, const u64_t clientid);
int do_password(const u64_t useridnr,
                const char * const password,
                const char * const enctype);
int do_aliases(const u64_t useridnr,
               GList * alias_add,
               GList * alias_del);
/* External forwards */
int do_forwards(const char *alias, const u64_t clientid,
                GList * fwds_add,
                GList * fwds_del);

/* Helper functions */
u64_t strtomaxmail(const char * const str);

#endif
