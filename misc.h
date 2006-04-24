/*
 Copyright (C) 1999-2004 IC & S  dbmail@ic-s.nl
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

/* $Id: misc.h 2080 2006-04-24 09:40:11Z aaron $ 
 */

#ifndef _MISC_H
#define _MISC_H

#include "dbmail.h"

#define BUFLEN 2048
 

/**
   \brief drop process privileges. Change change euid and egid to
   uid and gid of newuser and newgroup
   \param newuser user to change to
   \param newgroup group to change to
   \return 
        - -1 on error
	-  0 on success
*/
int drop_privileges(char *newuser, char *newgroup);

/**
 * \brief create a unique id for a message (used for pop, stored per message)
 * \param target target string. Length should be UID_SIZE 
 * \param message_idnr message_idnr of message
 */
void create_unique_id(/*@out@*/ char *target, u64_t message_idnr);

/**
 * \brief create a timestring with the current time.
 * \param timestring an allocated timestring object.
 */
void create_current_timestring(timestring_t * timestring);

/**
 * \brief decorate a mailbox name with a namespace if needed
 * \param mailbox_name name of mailbox
 * \param owner_idnr owner idnr of mailbox
 * \param user_idnr idnr of current user
 * \return
 *     - NULL on error
 *     - fully qualified mailbox name otherwise (e.g. #Users/username/INBOX)
 * \note caller should free returned string
 */
char *mailbox_add_namespace(const char *mailbox_name, u64_t owner_idnr,
			    u64_t user_idnr);

/**
 * \brief remove the namespace from the fully qualified name
 * \param fq_name full name (with possible namespace) of mailbox
 * \return
 *     - NULL on error
 *     - simple name of mailbox
 */
const char *mailbox_remove_namespace(const char *fq_name);

/**
 * write to a client socket. does error checking.
 * \param fd socket to write to
 * \param msg formatstring of message to write
 * \param ... arguments for formatstring.
 * \return 
 *      - -1 on error
 *      -  0 on success
 */
int ci_write(FILE * fd, char * msg, ...);
/**
 * \brief converts an IMAP date to a number (strictly ascending in date)
 * valid IMAP dates:
 *     - d-mon-yyyy
 *     - dd-mon-yyyy  ('-' may be a space)
 * \param date the IMAP date
 * \return integer representation of the date
 */
int num_from_imapdate(const char *date);


char **base64_decode(char *str, size_t len);
void base64_free(char **ret);
int read_from_stream(FILE * instream, char **m_buf, size_t maxlen);
int find_bounded(const char * const value, char left, char right,
		char **retchar, size_t * retsize, size_t * retlast);
int zap_between(const char * const instring, signed char left, signed char right,
		char **outstring, size_t *outlen, size_t *zaplen);
int base64_grow_ret(char ***inchar, size_t ** inint, size_t newcount,
		    size_t newchar);

GString * g_list_join(GList * list, const gchar * sep);
GList * g_string_split(GString * string, const gchar * sep);
GList * g_list_append_printf(GList * list, char * format, ...);

gint ucmp(const u64_t *a, const u64_t *b);
void g_list_destroy(GList *list);
GList * g_tree_keys(GTree *tree);
GList * g_tree_values(GTree *tree);
void tree_dump(GTree *t);
int g_tree_merge(GTree *a, GTree *b, int condition);
	
char * dm_stresc(const char * from);
char * dm_shellesc(const char * command);
void dm_pack_spaces(char *in);
void dm_base_subject(char *subject);
int listex_match(const char *p, const char *s, const char *x, int flags);
u64_t dm_getguid(unsigned int serverid);

sa_family_t dm_get_client_sockaddr(clientinfo_t *ci, struct sockaddr *saddr);

int dm_sock_score(const char *base, const char *test);
int dm_sock_compare(const char *clientsock, const char *sock_allow, const char *sock_deny);
int dm_valid_format(const char *str);

GList * g_tree_keys(GTree *tree);

char *date_sql2imap(const char *sqldate);
char *date_imap2sql(const char *imapdate);

int checkmailboxname(const char *s);
int check_msg_set(const char *s);
int check_date(const char *date);

/**
 * \brief discards all input coming from instream
 * \param instream FILE stream holding input from a client
 * \return 
 *      - -1 on error
 *      -  0 on success
 */
int discard_client_input(FILE * instream);

#endif
