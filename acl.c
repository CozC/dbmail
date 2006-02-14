/*
 Copyright (C) 2004 IC & S  dbmail@ic-s.nl

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

#include "dbmail.h"

#define NR_ACL_FLAGS 9

static const char *acl_right_strings[] = {
	"lookup_flag",
	"read_flag",
	"seen_flag",
	"write_flag",
	"insert_flag",
	"post_flag",
	"create_flag",
	"delete_flag",
	"administer_flag"
};

static const char acl_right_chars[] = "lrswipcda";
//{'l','r','s','w','i','p','c','d','a'};

/* local functions */
static ACLRight_t acl_get_right_from_char(char right_char);
static int acl_change_rights(u64_t userid, u64_t mboxid,
			     const char *rightsstring, int set);
static int acl_replace_rights(u64_t userid, u64_t mboxid,
			      const char *rightsstring);
static int acl_set_one_right(u64_t userid, u64_t mboxid,
			     ACLRight_t right, int set);
static int acl_get_rightsstring_identifier(char *identifier, u64_t mboxid,
					   /*@out@*/ char *rightsstring);
static int acl_get_rightsstring(u64_t userid, u64_t mboxid,
				/*@out@*/ char *rightsstring);

int acl_has_right(u64_t userid, u64_t mboxid, ACLRight_t right)
{
	u64_t anyone_userid;
	int auth_result;
	int user_acl_result, anyone_acl_result;

	const char *right_flag = acl_right_strings[right];

	/* first try if the user has the right */
	user_acl_result = db_acl_has_right(userid, mboxid, right_flag);
	if (user_acl_result != 0)
		return user_acl_result;

	/* if the user does not have the right, perhaps the 'anyone' 
	   user has it */
	auth_result = auth_user_exists(DBMAIL_ACL_ANYONE_USER, &anyone_userid);
	if (auth_result == -1) {
		trace(TRACE_ERROR, "%s,%s: error getting user_idnr of "
		      "ACL anyone user", __FILE__, __func__);
		return -1;
	}
	if (auth_result != 0)
		anyone_acl_result = db_acl_has_right(anyone_userid, mboxid,
						     right_flag);
	else
		anyone_acl_result = 0;
	
	return anyone_acl_result;
}

int acl_set_rights(u64_t userid, u64_t mboxid, const char *rightsstring)
{
	if (rightsstring[0] == '-')
		return acl_change_rights(userid, mboxid, rightsstring, 0);
	if (rightsstring[0] == '+')
		return acl_change_rights(userid, mboxid, rightsstring, 1);
	return acl_replace_rights(userid, mboxid, rightsstring);
}

ACLRight_t acl_get_right_from_char(char right_char)
{
	switch (right_char) {
	case 'l':
		return ACL_RIGHT_LOOKUP;
	case 'r':
		return ACL_RIGHT_READ;
	case 's':
		return ACL_RIGHT_SEEN;
	case 'w':
		return ACL_RIGHT_WRITE;
	case 'i':
		return ACL_RIGHT_INSERT;
	case 'p':
		return ACL_RIGHT_POST;
	case 'c':
		return ACL_RIGHT_CREATE;
	case 'd':
		return ACL_RIGHT_DELETE;
	case 'a':
		return ACL_RIGHT_ADMINISTER;
	default:
		trace(TRACE_ERROR,
		      "%s,%s: error wrong acl character. This "
		      "error should have been caught earlier!", __FILE__,
		      __func__);
		return ACL_RIGHT_NONE;
	}
}

int
acl_change_rights(u64_t userid, u64_t mboxid, const char *rightsstring,
		  int set)
{
	size_t i;
	char rightchar;

	for (i = 1; i < strlen(rightsstring); i++) {
		rightchar = rightsstring[i];
		if (acl_set_one_right(userid, mboxid,
				      acl_get_right_from_char(rightchar),
				      set) < 0)
			return -1;
	}
	return 1;
}

int
acl_replace_rights(u64_t userid, u64_t mboxid, const char *rightsstring)
{
	unsigned i;
	int set;

	trace(TRACE_DEBUG, "%s,%s: replacing rights for user [%llu], "
	      "mailbox [%llu] to %s", __FILE__, __func__,
	      userid, mboxid, rightsstring);
	for (i = ACL_RIGHT_LOOKUP; i < ACL_RIGHT_NONE; i++) {

		if (strchr(rightsstring, (int) acl_right_chars[i]))
			set = 1;
		else
			set = 0;
		if (db_acl_set_right
		    (userid, mboxid, acl_right_strings[i], set) < 0) {
			trace(TRACE_ERROR, "%s, %s: error replacing ACL",
			      __FILE__, __func__);
			return -1;
		}
	}
	return 1;

}

int
acl_set_one_right(u64_t userid, u64_t mboxid, ACLRight_t right, int set)
{
	return db_acl_set_right(userid, mboxid, acl_right_strings[right],
				set);
}



int acl_delete_acl(u64_t userid, u64_t mboxid)
{
	return db_acl_delete_acl(userid, mboxid);
}

char *acl_get_acl(u64_t mboxid)
{
	u64_t userid;
	char *username;
	size_t acl_string_size = 0;
	size_t acl_strlen;
	char *acl_string;	/* return string */
	char *identifier;	/* one identifier */
	char rightsstring[NR_ACL_FLAGS + 1];
	int result;
	struct dm_list identifier_list;
	struct element *identifier_elm;
	unsigned nr_identifiers = 0;

	result = db_acl_get_identifier(mboxid, &identifier_list);

	if (result < 0) {
		trace(TRACE_ERROR, "%s,%s: error when getting identifier "
		      "list for mailbox [%llu].", __FILE__, __func__,
		      mboxid);
		dm_list_free(&identifier_list.start);
		return NULL;
	}

	/* add the current user to the list if this user is the owner
	 * of the mailbox
	 */
	if (db_get_mailbox_owner(mboxid, &userid) < 0) {
		trace(TRACE_ERROR, "%s,%s: error querying ownership of "
		      "mailbox", __FILE__, __func__);
		dm_list_free(&identifier_list.start);
		return NULL;
	}

	if ((username = auth_get_userid(userid)) == NULL) {
		trace(TRACE_ERROR, "%s,%s: error getting username for "
		      "user [%llu]", __FILE__, __func__, userid);
		dm_list_free(&identifier_list.start);
		return NULL;
	}
	if (dm_list_nodeadd(&identifier_list, username, 
			 strlen(username) + 1) == NULL) { 
		trace(TRACE_ERROR, "%s,%s: error adding username to list",
		      __FILE__, __func__);
		dm_list_free(&identifier_list.start);
		dm_free(username);
		return NULL;
	}
	dm_free(username);

	identifier_elm = dm_list_getstart(&identifier_list);
	trace(TRACE_DEBUG, "%s,%s: before looping identifiers!",
	      __FILE__, __func__);
	while (identifier_elm) {
		nr_identifiers++;
		acl_string_size += strlen((char *) identifier_elm->data)
		    + NR_ACL_FLAGS + 2;
		identifier_elm = identifier_elm->nextnode;
	}

	trace(TRACE_DEBUG, "%s,%s: acl_string size = %zd",
	      __FILE__, __func__, acl_string_size);

	if (!
	    (acl_string =
	     dm_malloc((acl_string_size + 1) * sizeof(char)))) {
		dm_list_free(&identifier_list.start);
		trace(TRACE_FATAL, "%s,%s: error allocating memory",
		      __FILE__, __func__);
		return NULL;
	}
	// initialise list to length 0
	acl_string[0] = '\0';
	memset((void *) acl_string, '\0', acl_string_size + 1);
	identifier_elm = dm_list_getstart(&identifier_list);
	while (identifier_elm) {
		identifier = (char *) identifier_elm->data;
		if (acl_get_rightsstring_identifier(identifier, mboxid,
						    rightsstring) < 0) {
			trace(TRACE_ERROR, "%s,%s: error getting string "
			      "rights for user with name [%s].",
			      __FILE__, __func__, identifier);
			dm_list_free(&identifier_list.start);
			dm_free(acl_string);
			return NULL;
		}
		trace(TRACE_DEBUG, "%s,%s: %s", __FILE__, __func__,
		      rightsstring);
		if (strlen(rightsstring) > 0) {
			acl_strlen = strlen(acl_string);
			(void) snprintf(&acl_string[acl_strlen], 
					acl_string_size - acl_strlen,
					"%s %s ", identifier, rightsstring);
		}
		identifier_elm = identifier_elm->nextnode;

	}
	dm_list_free(&identifier_list.start);
	return acl_string;
}

char *acl_listrights(u64_t userid, u64_t mboxid)
{
	int result;
	result = db_user_is_mailbox_owner(userid, mboxid);


	if (result < 0) {
		trace(TRACE_ERROR, "%s,%s: error checking if user "
		      "is owner of a mailbox", __FILE__, __func__);
		return NULL;
	}

	if (result == 0) {
		/* user is not owner. User will never be granted any right
		   by default, but may be granted any right by setting the
		   right ACL */
		return dm_strdup("\"\" l r s w i p c d a");
	}

	/* user is owner, User will always be granted all rights */
	return dm_strdup(acl_right_chars);
}

char *acl_myrights(u64_t userid, u64_t mboxid)
{
	char *rightsstring;

	if (!(rightsstring = dm_malloc((NR_ACL_FLAGS + 1) * sizeof(char)))) {
		trace(TRACE_ERROR, "%s,%s: error allocating memory for "
		      "rightsstring", __FILE__, __func__);
		return NULL;
	}

	if (acl_get_rightsstring(userid, mboxid, rightsstring) < 0) {
		trace(TRACE_ERROR, "%s,%s: error getting rightsstring.",
		      __FILE__, __func__);
		dm_free(rightsstring);
		return NULL;
	}

	return rightsstring;
}


int
acl_get_rightsstring_identifier(char *identifier,
				u64_t mboxid, char *rightsstring)
{
	u64_t userid;

	memset(rightsstring, '\0', NR_ACL_FLAGS + 1);
	if (auth_user_exists(identifier, &userid) < 0) {
		trace(TRACE_ERROR, "%s,%s: error finding user id for "
		      "user with name [%s]", __FILE__, __func__,
		      identifier);
		return -1;
	}

	return acl_get_rightsstring(userid, mboxid, rightsstring);
}

int acl_get_rightsstring(u64_t userid, u64_t mboxid, char *rightsstring)
{
	unsigned i;
	unsigned rightsstring_idx = 0;
	int result;

	assert(rightsstring != NULL);
	memset(rightsstring, '\0', NR_ACL_FLAGS + 1);

	for (i = ACL_RIGHT_LOOKUP; i <= ACL_RIGHT_ADMINISTER; i++) {
		result = acl_has_right(userid, mboxid, i);
		if (result < 0) {
			trace(TRACE_ERROR, "%s,%s: error checking rights "
			      "for user [%llu], mailbox [%llu].",
			      __FILE__, __func__, userid, mboxid);
			return -1;
		}

		if (result == 1) {
			rightsstring[rightsstring_idx] =
			    acl_right_chars[i];
			rightsstring_idx++;
			trace(TRACE_DEBUG, "%s,%s: i = %u. char is %c, "
			      "str = %s",
			      __FILE__, __func__, i,
			      acl_right_chars[i], rightsstring);
		}
		trace(TRACE_DEBUG, "%s,%s rightsstring currently is %s",
		      __FILE__, __func__, rightsstring);
	}
	return 1;
}
