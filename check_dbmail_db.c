/*
 *  Copyright (C) 2006  Aaron Stone  <aaron@serendipity.cx>
 *  Copyright (c) 2006 NFG Net Facilities Group BV support@nfg.nl
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later
 *   version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *  $Id: check_dbmail_dsn.c 1598 2005-02-23 08:41:02Z paul $ 
 *
 *
 *  
 *
 *   Basic unit-test framework for dbmail (www.dbmail.org)
 *
 *   See http://check.sf.net for details and docs.
 *
 *
 *   Run 'make check' to see some action.
 *
 */ 

// Test dsnuser_resolve through all of its pathways.
#include <check.h>
#include "check_dbmail.h"

extern char *configFile;
extern db_param_t _db_params;
extern int quiet;
extern int reallyquiet;
extern char *query;

u64_t useridnr = 0;
u64_t useridnr_domain = 0;

char *testuser = "testuser1";
u64_t testidnr = 0;

char *username = "testfaildsn";
char *username_domain = "testfailuser@nonexistantdomain";
char *username_mailbox = "testfailuser+foomailbox@nonexistantdomain";

char *alias = "testfailalias@nonexistantdomain";
char *alias_mailbox = "testfailalias+foomailbox@nonexistantdomain";

char *userpart_catchall = "testfailcatchall@";
char *domain_catchall = "@nonexistantdomain";

/*
 *
 * the test fixtures
 *
 */
void setup(void)
{
	configure_debug(5,0);
	if (config_read(configFile) < 0) {
		printf( "Config file not found: %s\n", configFile );
		exit(1);
	}
	GetDBParams(&_db_params);
	db_connect();
	auth_connect();

	GList *alias_add = NULL;
	alias_add = g_list_append(alias_add, alias);
	alias_add = g_list_append(alias_add, userpart_catchall);
	alias_add = g_list_append(alias_add, domain_catchall);

	if (! (auth_user_exists(testuser, &testidnr))) {
		do_add(testuser, "test", "md5-hash", 0, 0, NULL, NULL);
		auth_user_exists(testuser, &testidnr);
	}
	if (! (auth_user_exists(username, &useridnr))) {
		do_add(username, "testpass", "md5-hash", 0, 0, alias_add, NULL);
		auth_user_exists(username, &useridnr);
	}
	if (! (auth_user_exists(username_domain, &useridnr_domain))) {
		do_add(username_domain, "testpass", "md5-hash", 0, 0, NULL, NULL);
		auth_user_exists(username_domain, &useridnr_domain);
	}
}

void teardown(void)
{
	u64_t mailbox_id=0;
	if (db_findmailbox("testcreatebox",testidnr,&mailbox_id))
		db_delete_mailbox(mailbox_id,0,0);
			
	if (db_findmailbox("testpermissionbox",testidnr,&mailbox_id)) {
		db_mailbox_set_permission(mailbox_id, IMAPPERM_READWRITE);
		db_delete_mailbox(mailbox_id,0,0);
	}

	db_disconnect();
	auth_disconnect();
}

/*
 * make sure we're running against a current database layout 
 */

//int db_check_version(void);


/**
 * \brief check database connection. If it is dead, reconnect
 * \return
 *    - -1 on failure (no connection to db possible)
 *    -  0 on success
 */
//int db_check_connection(void);

/**
 * \brief check database for existence of usermap table
 * \return
 * -  0 on table not found
 * -  1 on table found
 */
//int db_use_usermap(void);

/**
 * \brief check if username exists in the usermap table
 * \param ci clientinfo_t of connected client
 * \param userid login to lookup
 * \param real_username contains userid after successful lookup
 * \return
 *  - -1 on db failure
 *  -  0 on success
 *  -  1 on not found
 */
//int db_usermap_resolve(clientinfo_t *ci, const char *userid, char * real_username);

/**
 * \brief disconnect from database server
 * \return 0
 */
//int db_disconnect(void);

/**
 * \brief execute a database query
 * \param the_query the SQL query to execute
 * \return 
 *         - 0 on success
 *         - 1 on failure
 */
//int db_query(const char *the_query);
START_TEST(test_db_query)
{
	GString *q = g_string_new("");
	int result;

	g_string_printf(q,"SELECT foo FROM bar");
	result = db_query(q->str);
	fail_unless(result==-1, "db_query should have failed");

	g_string_printf(q,"SELECT 1=1");
	result = db_query(q->str);
	fail_unless(result==0, "db_query should have succeeded");

	g_string_free(q,TRUE);
	db_free_result();
}
END_TEST
START_TEST(test_db_mailbox_set_permission)
{

	int result;
	u64_t mailbox_id;
	result = db_find_create_mailbox("testpermissionbox", BOX_DEFAULT, testidnr, &mailbox_id);
	fail_unless(mailbox_id, "db_find_create_mailbox [testpermissionbox] owned by [%llu] failed [%llu].", testidnr, mailbox_id);

	result = db_mailbox_set_permission(mailbox_id, IMAPPERM_READ);
	fail_unless(result==0,"db_mailbox_set_permission failed");

	result = db_delete_mailbox(mailbox_id,0,0);
	fail_unless(result!=0,"db_delete_mailbox should have failed on readonly mailbox");

	result = db_mailbox_set_permission(mailbox_id, IMAPPERM_READWRITE);
	fail_unless(result==0,"db_mailbox_set_permission failed");

	result = db_delete_mailbox(mailbox_id,0,0);
	fail_unless(result==0,"db_delete_mailbox should have succeeded on readwrite mailbox");


	return;
}
END_TEST
/**
 * \brief get number of rows in result set.
 * \return 
 *     - 0 on failure or no rows
 *     - number of rows otherwise
 */
//unsigned db_num_rows(void);

/**
 * \brief get number of fields in result set.
 * \return
 *      - 0 on failure or no fields
 *      - number of fields otherwise
 */
//unsigned db_num_fields(void);

/**
 * \brief clear the result set of a query
 */
//void db_free_result(void);

/** 
 * \brief get a single result field row from the result set
 * \param row result row to get it from
 * \param field field (=column) to get result from
 * \return
 *     - pointer to string holding result. 
 *     - NULL if no such result
 */
//const char *db_get_result(unsigned row, unsigned field);

/** \ingroup db_get_result_group
 * \brief Returns the result as an Integer
 */
//int db_get_result_int(unsigned row, unsigned field);

/** \ingroup db_get_result_group
 * \brief Returns the result as an Integer from 0 to 1
 */
//int db_get_result_bool(unsigned row, unsigned field);

/** \ingroup db_get_result_group
 * \brief Returns the result as an Unsigned 64 bit Integer
 */
//u64_t db_get_result_u64(unsigned row, unsigned field);

/**
 * \brief return the ID generated for an AUTO_INCREMENT column
 *        by the previous column.
 * \param sequence_identifier sequence identifier
 * \return 
 *       - 0 if no such id (if for instance no AUTO_INCREMENT
 *          value was generated).
 *       - the id otherwise
 */
//u64_t db_insert_result(const char *sequence_identifier);

/**
 * \brief escape a string for use in query
 * \param to string to copy escaped string to. Must be allocated by caller
 * \param from original string
 * \param length of orginal string
 * \return length of escaped string
 * \attention behaviour is undefined if to and from overlap
 */
//unsigned long db_escape_string(char *to,
//			       const char *from, unsigned long length);

/**
 * \brief escape a binary data for use in query
 * \param to string to copy escaped string to. Must be allocated by caller
 * \param from original string
 * \param length of orginal string
 * \return length of escaped string
 * \attention behaviour is undefined if to and from overlap
 */
//unsigned long db_escape_binary(char *to,
//			       const char *from, unsigned long length);

/**
 * \brief get length in bytes of a result field in a result set.
 * \param row row of field
 * \param field field number (0..nfields)
 * \return 
 *      - -1 if invalid field
 *      - length otherwise
 */
//u64_t db_get_length(unsigned row, unsigned field);

/**
 * \brief get number of rows affected by a query
 * \return
 *    -  -1 on error (e.g. no result set)
 *    -  number of affected rows otherwise
 */
//u64_t db_get_affected_rows(void);

/**
 * \brief switch from the normal result set to the msgbuf
 * result set from hereon. A call to db_store_msgbuf_result()
 * will reverse this situation again 
 */
//void db_use_msgbuf_result(void);

/**
 * \brief switch from msgbuf result set to the normal result
 * set for all following database operations. This function
 * should be called after db_use_msgbuf_result() when the
 * msgbuf result has to be used later on.
 */
//void db_store_msgbuf_result(void);

/**
 * \brief switch from normal result set to the authentication result
 * set
 */
//void db_use_auth_result(void);

/**
 * \brief switch from authentication result set to normal result set
 */
//void db_store_auth_result(void);

/**
 * \brief get void pointer to result set.
 * \return a pointer to a result set
 * \bug this is really ugly and should be dealt with differently!
 */
//void *db_get_result_set(void);

/**
 * \brief set the new result set 
 * \param res the new result set
 * \bug this is really ugly and should be dealt with differently!
 */
//void db_set_result_set(void *res);

/**
 * begin transaction
 * \return 
 *     - -1 on error
 *     -  0 otherwise
 */
//int db_begin_transaction(void);

/**
 * commit transaction
 * \return
 *      - -1 on error
 *      -  0 otherwise
 */
//int db_commit_transaction(void);

/**
 * rollback transaction
 * \return 
 *     - -1 on error
 *     -  0 otherwise
 */
//int db_rollback_transaction(void);

/* shared implementattion from hereon */
/**
 * \brief get the physmessage_id from a message_idnr
 * \param message_idnr
 * \param physmessage_id will hold physmessage_id on return. Must hold a valid
 * pointer on call.
 * \return 
 *     - -1 on error
 *     -  0 if a physmessage_id was found
 *     -  1 if no message with this message_idnr found
 * \attention function will fail and halt program if physmessage_id is
 * NULL on call.
 */
//int db_get_physmessage_id(u64_t message_idnr, /*@out@*/ u64_t * physmessage_id);

/**
 * \brief return number of bytes used by user identified by userid
 * \param user_idnr id of the user (from users table)
 * \param curmail_size will hold current mailsize used (should hold a valid 
 * pointer on call).
 * \return
 *          - -1 on failure<BR>
 *          -  1 otherwise
 */
//int db_get_quotum_used(u64_t user_idnr, u64_t * curmail_size);

/**
 * \brief finds all users which need to have their curmail_size (amount
 * of space used by user) updated. Then updates this number in the
 * database
 * \return 
 *     - -2 on memory error
 *     - -1 on database error
 *     -  0 on success
 */
//int db_calculate_quotum_all(void);

/**
 * \brief performs a recalculation of used quotum of a user and puts
 *  this value in the users table.
 * \param user_idnr
 * \return
 *   - -1 on db_failure
 *   -  0 on success
 */
//int db_calculate_quotum_used(u64_t user_idnr);

/**
 * \brief get auto-notification address for a user
 * \param user_idnr user id
 * \param notify_address pointer to string that will hold the address
 * \return
 *        - -2 on failure of allocating memory for string
 *        - -1 on database failure
 *        - 0 on success
 * \attention caller should free deliver_to address
 */
//int db_get_notify_address(u64_t user_idnr, char **notify_address);
/** 
 * \brief get reply body for auto-replies
 * \param user_idnr user idnr 
 * \param body pointer to string that will hold reply body
 * \return 
 *        - -2 on failure of allocating memory for string
 *        - -1 on database failure
 *        - 0 on success
 * \attention caller should free reply_body
 */
//int db_get_reply_body(u64_t user_idnr, char **body);

/**
 * \brief get user idnr of a message. 
 * \param message_idnr idnr of message
 * \return 
 *              - -1 on failure
 * 		- 0 if message is located in a shared mailbox.
 * 		- user_idnr otherwise
 */
//u64_t db_get_useridnr(u64_t message_idnr);

/**
 * \brief insert a new physmessage. This inserts only an new record in the
 * physmessage table with a timestamp
 * \param physmessage_id will hold the id of the physmessage on return. Must 
 * hold a valid pointer on call.
 * \return 
 *     - -1 on failure
 *     -  1 on success
 */
//int db_insert_physmessage(/*@out@*/ u64_t * physmessage_id);

/**
 * \brief insert a physmessage with an internal date.
 * \param internal_date the internal date in "YYYY-MM-DD HH:Mi:SS"
 * \param physmessage_id will hold the id of the physmessage on return. Must
 * hold a valid pointer on call.
 * \return 
 *    - -1 on failure
 *    -  1 on success
 */
//int db_insert_physmessage_with_internal_date(timestring_t internal_date,
//					     u64_t * physmessage_id);

/**
 * \brief update unique_id, message_size and rfc_size of
 *        a message identified by message_idnr
 * \param message_idnr
 * \param unique_id unique id of message
 * \param message_size size of message
 * \param rfc_size
 * \return 
 *      - -1 on database error
 *      - 0 on success
 */
//int db_update_message(u64_t message_idnr, const char *unique_id,
//		      u64_t message_size, u64_t rfc_size);
/**
 * \brief set unique id of a message 
 * \param message_idnr
 * \param unique_id unique id of message
 * \return 
 *     - -1 on database error
 *     -  0 on success
 */
//int db_message_set_unique_id(u64_t message_idnr, const char *unique_id);

/**
 * \brief set messagesize and rfcsize of a message
 * \param physmessage_id 
 * \param message_size
 * \param rfc_size
 * \return
 *    - -1 on failure
 *    -  0 on succes
 */
//int db_physmessage_set_sizes(u64_t physmessage_id, u64_t message_size,
//			     u64_t rfc_size);

/**
 * \brief insert a messageblock for a specific physmessage
 * \param block the block
 * \param block_size size of block
 * \param physmessage_id id of physmessage to which the block belongs.
 * \param messageblock_idnr will hold id of messageblock after call. Should
 * hold a valid pointer on call.
 * \return
 *      - -1 on failure
 *      -  0 on success
 */
//int db_insert_message_block_physmessage(const char *block,
//					u64_t block_size,
//					u64_t physmessage_id,
//					u64_t * messageblock_idnr,
//					unsigned is_header);
/**
* \brief insert a message block into the message block table
* \param block the message block (which is a string)
* \param block_size length of the block
* \param message_idnr id of the message the block belongs to
* \param messageblock_idnr will hold id of messageblock after call. Should
* be a valid pointer on call.
* \return 
*        - -1 on failure
*        - 0 otherwise
*/
//int db_insert_message_block(const char *block, u64_t block_size,
//			    u64_t message_idnr, 
//			    /*@out@*/ u64_t * messageblock_idnr,
//			    unsigned is_header);
/**
 * \brief log IP-address for POP/IMAP_BEFORE_SMTP. If the IP-address
 *        is already logged, it's timestamp is renewed.
 * \param ip the ip (xxx.xxx.xxx.xxx)
 * \return
 *        - -1 on database failure
 *        - 0 on success
 * \bug timestamp!
 */
//int db_log_ip(const char *ip);

/**
* \brief clean up ip log
* \param lasttokeep all ip log entries before this timestamp will
*                     deleted
* \return 
*       - -1 on database failure
*       - 0 on success
*/
//int db_cleanup_iplog(const char *lasttokeep, u64_t *affected_rows);
//int db_count_iplog(const char *lasttokeep, u64_t *affected_rows);

/**
 * \brief cleanup database tables
 * \return 
 *       - -1 on database failure
 *       - 0 on success
 */
//int db_cleanup(void);

/**
 * \brief execute cleanup of database tables (have no idea why there
 *        are two functions for this..)
 * \param tables array of char* holding table names 
 * \param num_tables number of tables in tables
 * \return 
 *    - -1 on db failure
 *    - 0 on success
 */
//int db_do_cleanup(const char **tables, int num_tables);

/**
* \brief remove all mailboxes for a user
* \param user_idnr
* \return 
*      - -2 on memory failure
*      - -1 on database failure
*      - 0 on success
*/
//int db_empty_mailbox(u64_t user_idnr);

/**
* \brief check for all message blocks  that are not connected to
*        a physmessage. This can only occur when in use with a 
*        database that does not check for foreign key constraints.
* \param lost_list pointer to a list which will contain all lost blocks.
*        this list needs to be empty on call to this function.
* \return 
*      - -2 on memory error
*      - -1 on database error
*      - 0 on success
* \attention caller should free this memory
*/
//int db_icheck_messageblks(struct dm_list *lost_list);

/**
 * \brief check for all messages that are not connected to
 *        mailboxes
 * \param lost_list pointer to a list which will contain all lost messages
 *        this list needs to be empty on call to this function.
 * \return 
 *      - -2 on memory error
 *      - -1 on database error
 *      - 0 on success
 * \attention caller should free this memory
 */
//int db_icheck_messages(struct dm_list *lost_list);

/**
 * \brief check for all mailboxes that are not connected to
 *        users
 * \param lost_list pointer to a list which will contain all lost mailboxes
 *        this list needs to be empty on call to this function.
 * \return 
 *      - -2 on memory error
 *      - -1 on database error
 *      - 0 on success
 * \attention caller should free this memory
 */
//int db_icheck_mailboxes(struct dm_list *lost_list);

/**
 * \brief check for all messages that are not connected to physmessage
 * records. This function is not nessecary when using foreign key
 * constraints. 
 * \param lost_list pointer to a list which will contain all message_idnr
 *        of messages that are not connected to physmessage records.
 *        this list needs to be empty on call to this function.
 * \return 
 *      - -2 on memory error
 *      - -1 on database error
 *      - 0 on success
 * \attention caller should free this memory
 */
//int db_icheck_null_messages(struct dm_list *lost_list);

/**
 * \brief check for all physmessage records that have no messageblks 
 * associated with them.
 * \param null_list pointer to a list which will contain all physmessage_ids
 * of messages that are not connected to physmessage records.
 * this list needs to be empty on call to this function.
 * \return
 *     - -2 on memory error
 *     - -1 on database error
 *     -  0 on success.
 * \attention caller should free this memory
 */
//int db_icheck_null_physmessages(struct dm_list *lost_list);

/**
 * \brief check for is_header flag on messageblks
 *
 */
//int db_icheck_isheader(GList  **lost);
//int db_set_isheader(GList *lost);

/** 
 * \brief check for cached header values
 *
 */
//int db_icheck_headercache(GList **lost);
//int db_set_headercache(GList *lost);

/**
 * \brief check for rfcsize in physmessage table
 *
 */
//int db_icheck_rfcsize(GList **lost);
//int db_update_rfcsize(GList *lost);

/**
 * \brief set status of a message
 * \param message_idnr
 * \param status new status of message
 * \return result of db_query()
 */
//int db_set_message_status(u64_t message_idnr, MessageStatus_t status);

/**
* \brief delete a message block
* \param messageblk_idnr
* \return result of db_query()
*/
//int db_delete_messageblk(u64_t messageblk_idnr);
/**
 * \brief delete a physmessage
 * \param physmessage_id
 * \return
 *      - -1 on error
 *      -  1 on success
 */
//int db_delete_physmessage(u64_t physmessage_id);

/**
 * \brief delete a message 
 * \param message_idnr
 * \return 
 *     - -1 on error
 *     -  1 on success
 */
//int db_delete_message(u64_t message_idnr);
START_TEST(test_db_delete_message)
{

}
END_TEST
/**
 * \brief write lines of message to fstream. Does not write the header
 * \param fstream the stream to write to
 * \param message_idnr idrn of message to write
 * \param lines number of lines to write. If <PRE>lines == -2</PRE>, then
 *              the whole message (excluding the header) is written.
 * \param no_end_dot if 
 *                    - 0 \b do write the final "." signalling
 *                    the end of the message
 *                    - otherwise do \b not write the final "."
 * \return 
 * 		- 0 on failure
 * 		- 1 on success
 */
//int db_send_message_lines(void *fstream, u64_t message_idnr,
//			  long lines, int no_end_dot);
/**
 * \brief create a new POP3 session. (was createsession() in dbmysql.c)
 * \param user_idnr user idnr 
 * \param session_ptr pointer to POP3 session 
 */
//int db_createsession(u64_t user_idnr, PopSession_t * session_ptr);
/** 
 * \brief Clean up a POP3 Session
 * \param session_ptr pointer to POP3 session
 */
//void db_session_cleanup(PopSession_t * session_ptr);
/**
 * \brief update POP3 session
 * \param session_ptr pointer to POP3 session
 * \return
 *     - -1 on failure
 *     - 0 on success
 * \attention does not update shared mailboxes (which should
 * not be nessecary, because the shared mailboxes are not 
 * touched by POP3
 */
//int db_update_pop(PopSession_t * session_ptr);
/**
 * \brief set deleted status (=3) for all messages that are marked for
 *        delete (=2)
 * \param affected_rows will hold the number of affected messages on return. 
 * must hold a valid pointer on call.
 * \return
 *    - -1 on database failure;
 *    - 1 otherwise
 */
//int db_set_deleted(u64_t * affected_rows);
/**
 * \brief purge all messages from the database with a "delete"-status 
 * (status = 3)
 * \param affected_rows will hold the number of affected rows on return. Must
 * hold a valid pointer on call
 * \return 
 *     - -2 on memory failure
 *     - -1 on database failure
 *     - 0 if no messages deleted (affected_rows will also hold '0')
 *     - 1 if a number of messages deleted (affected_rows will hold the number
 *       of deleted messages.
 */
//int db_deleted_purge(u64_t * affected_rows);
//int db_deleted_count(u64_t * affected_rows);
/**
 * \brief check if a block of a certain size can be inserted.
 * \param addblocksize size of added blocks (UNUSED)
 * \param message_idnr 
 * \param *user_idnr pointer to user_idnr. This will be set to the
 *        idnr of the user associated with the message
 * \return
 *      - -2 on database failure after a limit overrun (if this
 *           occurs the DB might be inconsistent and dbmail-util
 *           needs to be run)
 *      - -1 on database failure
 *      - 0 on success
 *      - 1 on limit overrun.
 * \attention when inserting a block would cause a limit run-overrun.
 *            the message insert is automagically rolled back
 */
//u64_t db_check_sizelimit(u64_t addblocksize, u64_t message_idnr,
//			 u64_t * user_idnr);
/**
 * \brief insert a message into the database
 * \param msgdata the message
 * \param datalen length of message
 * \param mailbox_idnr mailbox to put it in
 * \param user_idnr
 * \param internal_date internal date of message. May be '\0'.
 * \return 
 *     - -1 on failure
 *     - 0 on success
 *     - 1 on invalid message
 *     - 2 on mail quotum exceeded
 */
//int db_imap_append_msg(const char *msgdata, u64_t datalen,
//		       u64_t mailbox_idnr, u64_t user_idnr,
//		       timestring_t internal_date, u64_t * msg_idnr);

/**
 * Produces a regexp that will case-insensitively match the mailbox name
 * according to the modified UTF-7 rules given in section 5.1.3 of IMAP.
 * \param column name of the name column.
 * \param mailbox name of the mailbox.
 * \param filter use /% for children or "" for just the box.
 * \return pointer to a newly allocated string.
 */
START_TEST(test_db_imap_utf7_like)
{
	char *trythese[] = {
		"INBOX/&BekF3AXVBd0-",
		"Inbox",
		"Listen/F% %/Users",
		NULL };
	char *getthese[] = {
		"name LIKE BINARY '______&BekF3AXVBd0_/%' AND name LIKE 'INBOX/____________-/%'",
		"name LIKE 'Inbox/%'",
		"name LIKE 'Listen/F% %/Users/%'",
		NULL };
	int i;

	for (i = 0; trythese[i] != NULL; i++) {
		char *result = db_imap_utf7_like("name", trythese[i], "/%");

		fail_unless(strcmp(result, getthese[i])==0, "Failed to make db_imap_utf7_like string for [%s]", trythese[i]);

		dm_free(result);
	}
}
END_TEST

/* mailbox functionality */
/** 
 * \brief find mailbox "name" for a user
 * \param name name of mailbox
 * \param user_idnr
 * \param mailbox_idnr will hold mailbox_idnr after return. Must hold a valid
 * pointer on call.
 * \return
 *      - -1 on failure
 *      - 0 if mailbox not found
 *      - 1 if found
 */
//int db_findmailbox(const char *name, u64_t user_idnr,
//		   /*@out@*/ u64_t * mailbox_idnr);
/**
 * \brief finds all the mailboxes owned by owner_idnr who match 
 *        the pattern.
 * \param owner_idnr
 * \param pattern pattern
 * \param children pointer to a list of mailboxes conforming to
 *        pattern. This will be filled when the function
 *        returns and needs to be free-d by caller
 * \param nchildren number of mailboxes in children
 * \param only_subscribed only search in subscribed mailboxes.
 * \return 
 *      - -1 on failure
 *      - 0 on success
 */
//int db_findmailbox_by_regex(u64_t owner_idnr, const char *pattern,
//			    u64_t ** children, unsigned *nchildren,
//			    int only_subscribed);
/**
 * \brief get info on a mailbox. Info is filled in in the
 *        mailbox_t struct.
 * \param mb the mailbox_t to fill in. (mb->uid needs to be
 *        set already!
 * \return
 *     - -1 on failure
 *     - 0 on success
 */
//int db_getmailbox(mailbox_t * mb);

/**
 * \brief find owner of a mailbox
 * \param mboxid id of mailbox
 * \param owner_id will hold owner of mailbox after return
 * \return
 *    - -1 on db error
 *    -  0 if owner not found
 *    -  1 if owner found
 */
//int db_get_mailbox_owner(u64_t mboxid, /*@out@*/ u64_t * owner_id);

/**
 * \brief check if a user is owner of the specified mailbox 
 * \param userid id of user
 * \param mboxid id of mailbox
 * \return
 *     - -1 on db error
 *     -  0 if not user
 *     -  1 if user
 */
//int db_user_is_mailbox_owner(u64_t userid, u64_t mboxid);

/**
 * \brief create a new mailbox
 * \param name name of mailbox
 * \param owner_idnr owner of mailbox
 * \return 
 *    - -1 on failure
 *    -  0 on success
 */
//int db_createmailbox(const char *name, u64_t owner_idnr,
//		     u64_t * mailbox_idnr);

START_TEST(test_db_createmailbox)
{
	u64_t owner_id=99999999;
	u64_t mailbox_id=0;
	int result;

	result = db_createmailbox("INBOX", owner_id, &mailbox_id);
	fail_unless(result == -1, "db_createmailbox should have failed");

	result = db_createmailbox("testcreatebox", testidnr, &mailbox_id);
	fail_unless(result==0,"db_createmailbox failed");
	
}
END_TEST

/** Create a mailbox, recursively creating its parents.
 * \param mailbox Name of the mailbox to create
 * \param owner_idnr Owner of the mailbox
 * \param mailbox_idnr Fills the pointer with the mailbox id
 * \param message Returns a static pointer to the return message
 * \return
 *    0 Everything's good
 *    1 Cannot create mailbox
 *   -1 Database error
 */
START_TEST(test_db_mailbox_create_with_parents)
{
	u64_t mailbox_idnr = 0;
	const char *message;
	int result;
	int only_empty = 0, update_curmail_size = 0 ;

	result = db_mailbox_create_with_parents("INBOX/Foo/Bar/Baz",
			useridnr, &mailbox_idnr, &message);
	fail_unless(result == 0 && mailbox_idnr != 0,
			"Failed at db_mailbox_create_with_parents: [%s]", message);

	/* At this point, useridnr should have and be subscribed to several boxes... */
	result = db_findmailbox("inbox/foo/BAR/baz", useridnr, &mailbox_idnr);
	fail_unless(result == 1 && mailbox_idnr != 0, "Failed at db_findmailbox(\"inbox/foo/BAR/baz\")");
	result = db_delete_mailbox(mailbox_idnr, only_empty, update_curmail_size);
	fail_unless(result == 0, "Failed at db_findmailbox or db_delete_mailbox");

	result = db_findmailbox("InBox/Foo/bar", useridnr, &mailbox_idnr);
	fail_unless(result == 1 && mailbox_idnr != 0, "Failed at db_findmailbox(\"InBox/Foo/bar\")");
	result = db_delete_mailbox(mailbox_idnr, only_empty, update_curmail_size);
	fail_unless(result == 0, "Failed at db_findmailbox or db_delete_mailbox");

	result = db_findmailbox("INBOX/FOO", useridnr, &mailbox_idnr);
	fail_unless(result == 1 && mailbox_idnr != 0, "Failed at db_findmailbox(\"INBOX/FOO\")");
	result = db_delete_mailbox(mailbox_idnr, only_empty, update_curmail_size);
	fail_unless(result == 0, "Failed at db_findmailbox or db_delete_mailbox");

	result = db_findmailbox("INBox", useridnr, &mailbox_idnr);
	fail_unless(result == 1 && mailbox_idnr != 0, "Failed at db_findmailbox(\"INBox\")");
	result = db_delete_mailbox(mailbox_idnr, only_empty, update_curmail_size);
	fail_unless(result == 0, "We just deleted inbox. Something silly.");
	/* Cool, we've cleaned up after ourselves. */
}
END_TEST

/**
 * \brief delete a mailbox. 
 * \param mailbox_idnr
 * \param only_empty if non-zero the mailbox will only be emptied,
 *        i.e. all messages in it will be deleted.
 * \param update_curmail_size if non-zero the curmail_size of the
 *        user will be updated.
* \return 
*    - -1 on database failure
*    - 0 on success
* \attention this function is unable to delete shared mailboxes
*/
//int db_delete_mailbox(u64_t mailbox_idnr, int only_empty,
//		      int update_curmail_size);
START_TEST(test_db_delete_mailbox)
{
	u64_t mailbox_id = 999999999;
	int result;

	result = db_delete_mailbox(mailbox_id, 0, 0);
	fail_unless(result != DM_SUCCESS, "db_delete_mailbox should have failed");

	result = db_createmailbox("testdeletebox",testidnr, &mailbox_id);
	fail_unless(result == 0,"db_createmailbox failed");

	result = db_delete_mailbox(mailbox_id,0,0);
	fail_unless(result == 0,"db_delete_mailbox failed");
	
}
END_TEST

/**
 * \brief find a mailbox, create if not found
 * \param name name of mailbox
 * \param owner_idnr owner of mailbox
 * \return 
 *    - -1 on failure
 *    -  0 on success
 */
//int db_find_create_mailbox(const char *name, mailbox_source_t source,
//		u64_t owner_idnr, /*@out@*/ u64_t * mailbox_idnr);
/**
 * \brief produce a list containing the UID's of the specified
 *        mailbox' children matching the search criterion
 * \param mailbox_idnr id of parent mailbox
 * \param user_idnr
 * \param children will hold list of children
 * \param nchildren length of list of children
 * \param filter search filter
 * \return 
 *    - -1 on failure
 *    -  0 on success
 */
//int db_listmailboxchildren(u64_t mailbox_idnr, u64_t user_idnr,
//			   u64_t ** children, int *nchildren,
//			   const char *filter);

/**
 * \brief check if mailbox is selectable
 * \param mailbox_idnr
 * \return 
 *    - -1 on failure
 *    - 0 if not selectable
 *    - 1 if selectable
 */
//int db_isselectable(u64_t mailbox_idnr);
/**
 * \brief check if mailbox has no_inferiors flag set.
 * \param mailbox_idnr
 * \return
 *    - -1 on failure
 *    - 0 flag not set
 *    - 1 flag set
 */
//int db_noinferiors(u64_t mailbox_idnr);

/**
 * \brief set selectable flag of a mailbox on/of
 * \param mailbox_idnr
 * \param select_value 
 *            - 0 for not select
 *            - 1 for select
 * \return
 *     - -1 on failure
 *     - 0 on success
 */
//int db_setselectable(u64_t mailbox_idnr, int select_value);
/** 
 * \brief remove all messages from a mailbox
 * \param mailbox_idnr
 * \return 
 * 		- -1 on failure
 * 		- 0 on success
 */
//int db_removemsg(u64_t user_idnr, u64_t mailbox_idnr);
/**
 * \brief move all messages from one mailbox to another.
 * \param mailbox_to idnr of mailbox to move messages from.
 * \param mailbox_from idnr of mailbox to move messages to.
 * \return 
 * 		- -1 on failure
 * 		- 0 on success
 */
//int db_movemsg(u64_t mailbox_to, u64_t mailbox_from);
/**
 * \brief copy a message to a mailbox
 * \param msg_idnr
 * \param mailbox_to mailbox to copy to
 * \param user_idnr user to copy the messages for.
 * \return 
 * 		- -2 if the quotum is exceeded
 * 		- -1 on failure
 * 		- 0 on success
 */
//int db_copymsg(u64_t msg_idnr, u64_t mailbox_to,
//	       u64_t user_idnr, u64_t * newmsg_idnr);
/**
 * \brief get name of mailbox
 * \param mailbox_idnr
 * \param name will hold the name
 * \return 
 * 		- -1 on failure
 * 		- 0 on success
 * \attention name should be large enough to hold the name 
 * (preferably of size IMAP_MAX_MAILBOX_NAMELEN + 1)
 */
//int db_getmailboxname(u64_t mailbox_idnr, u64_t user_idnr, char *name);
/**
 * \brief set name of mailbox
 * \param mailbox_idnr
 * \param name new name of mailbox
 * \return 
 * 		- -1 on failure
 * 		- 0 on success
 */
//int db_setmailboxname(u64_t mailbox_idnr, const char *name);
/**
 * \brief remove all messages from a mailbox that have the delete flag
 *        set. Remove is done by setting status to 2. A list holding
 *        messages_idnrs of all removed messages is made in msg_idnrs.
 *        nmsgs will hold the number of deleted messages.
 * \param mailbox_idnr
 * \param msg_idnrs pointer to a list of msg_idnrs. If NULL, or nmsg
 *        is NULL, no list will be made.
 * \param nmsgs will hold the number of memebers in the list. If NULL,
 *        or msg_idnrs is NULL, no list will be made.
 * \return 
 * 		- -1 on failure
 * 		- 0 on success
 * \attention caller should free msg_idnrs and nmsg
 */
//int db_expunge(u64_t mailbox_idnr,
//	       u64_t user_idnr, u64_t ** msg_idnrs, u64_t * nmsgs);
/**
 * \brief get first unseen message in a mailbox
 * \param mailbox_idnr
 * \return 
 *     - -1 on failure
 *     - msg_idnr of first unseen message in mailbox otherwise
 */
//u64_t db_first_unseen(u64_t mailbox_idnr);
/**
 * \brief subscribe to a mailbox.
 * \param mailbox_idnr mailbox to subscribe to
 * \param user_idnr user to subscribe
 * \return 
 * 		- -1 on failure
 * 		- 0 on success
 */
//int db_subscribe(u64_t mailbox_idnr, u64_t user_idnr);
/**
 * \brief unsubscribe from a mailbox
 * \param mailbox_idnr
 * \param user_idnr
 * \return
 * 		- -1 on failure
 * 		- 0 on success
 */
//int db_unsubscribe(u64_t mailbox_idnr, u64_t user_idnr);



Suite *dbmail_db_suite(void)
{
	Suite *s = suite_create("Dbmail Basic Database Functions");

	TCase *tc_db = tcase_create("DB");
	suite_add_tcase(s, tc_db);
	tcase_add_checked_fixture(tc_db, setup, teardown);
	tcase_add_test(tc_db, test_db_query);
	tcase_add_test(tc_db, test_db_createmailbox);
	tcase_add_test(tc_db, test_db_mailbox_create_with_parents);
	tcase_add_test(tc_db, test_db_delete_mailbox);
	tcase_add_test(tc_db, test_db_mailbox_set_permission);
	tcase_add_test(tc_db, test_db_imap_utf7_like);
	return s;
}

int main(void)
{
	int nf;
	Suite *s = dbmail_db_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

