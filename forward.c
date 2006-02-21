/*
 Copyright (C) 1999-2004 IC & S  dbmail@ic-s.nl

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

/* $Id: forward.c 1991 2006-02-20 10:41:48Z aaron $
 *
 * takes care of forwarding mail to an external address */

#include "dbmail.h"

/* For each of the addresses or programs in targets,
 * send out a copy of the message pointed to by msgidnr.
 *
 * Returns 0 if all went well, -1 if there was an error.
 * FIXME: there is no detail in the error reporting,
 * so there's no way to tell *which* targets failed...
 * */
int forward(u64_t msgidnr, struct dm_list *targets, const char *from,
	    const char *header, u64_t headersize UNUSED)
{
	struct element *target = NULL;
	char *command = NULL;
	size_t command_len = 0;
	FILE *pipe = NULL;
	int err;
	field_t sendmail;
	char timestr[50];
	time_t td;
	struct tm tm;

	time(&td);		/* get time */
	tm = *localtime(&td);	/* get components */
	strftime(timestr, sizeof(timestr), "%a %b %e %H:%M:%S %Y", &tm);

	config_get_value("SENDMAIL", "SMTP", sendmail);
	if (sendmail[0] == '\0')
		trace(TRACE_FATAL,
		      "%s,%s: SENDMAIL not configured (see config file). "
		      "Stop.", __FILE__, __func__);

	trace(TRACE_INFO,
	      "%s,%s: delivering to [%ld] external addresses",
	      __FILE__, __func__, dm_list_length(targets));

	if (!msgidnr) {
		trace(TRACE_ERROR,
		      "%s,%s: got NULL as message id number",
		      __FILE__, __func__);
		return -1;
	}

	target = dm_list_getstart(targets);

	/* We might get passed a NULL pointer here. */
	if (!from)
		from = "DBMAIL-MAILER";

	while (target != NULL) {
		if ((((char *) target->data)[0] == '|')
		    || (((char *) target->data)[0] == '!')) {

			/* external pipe command */
			command_len = strlen((char *) (target->data)) + 1;
			command = dm_malloc(command_len * sizeof(char));
			if (!command) {
				trace(TRACE_ERROR,
				      "%s,%s: out of memory",
				      __FILE__, __func__);
				return -1;
			}
			/* skip the pipe (|) sign */
			strncpy(command, (char *) (target->data) + 1, command_len);
		} else {
			/* pipe to sendmail */
			command_len = strlen(sendmail) + strlen(" -f ") +
				strlen(from) + strlen (" ") +
				strlen((char *) (target->data)) + 1;
			command = dm_malloc(command_len * sizeof(char));
			if (!command) {
				trace(TRACE_ERROR,
				      "%s,%s: out of memory",
				      __FILE__, __func__);
				return -1;
			}

			trace(TRACE_DEBUG,
			      "%s,%s: allocated memory for external "
			      "command call", __FILE__, __func__);
			snprintf(command, command_len, "%s -f %s %s", sendmail, from, 
				(char *) (target->data));
		}

		trace(TRACE_INFO, "%s,%s: opening pipe to command %s",
		      __FILE__, __func__, command);

		pipe = popen(command, "w");	/* opening pipe */
		dm_free(command);
		command = NULL;

		if (pipe != NULL) {
			trace(TRACE_DEBUG,
			      "%s,%s: call to popen() successfully "
			      "opened pipe [%d]", __FILE__, __func__,
			      fileno(pipe));

			if (((char *) target->data)[0] == '!') {
				/* ! tells us to prepend an mbox style header in this pipe */
				trace(TRACE_DEBUG,
				      "%s,%s: appending mbox style from "
				      "header to pipe returnpath : %s",
				      __FILE__, __func__, from);
				/* format: From<space>address<space><space>Date */
				fprintf(pipe, "From %s  %s\n", from,
					timestr);
			}

			trace(TRACE_INFO,
			      "%s,%s: sending message id number [%llu] "
			      "to forward pipe", __FILE__, __func__,
			      msgidnr);

			err = ferror(pipe);

			trace(TRACE_DEBUG, "%s,%s: ferror reports"
			      " %d, feof reports %d on pipe %d", __FILE__,
			      __func__, err, feof(pipe), fileno(pipe));

			if (!err) {
				if (msgidnr != 0) {
					trace(TRACE_DEBUG,
					      "%s,%s: sending lines from "
					      "message %llu on pipe %d",
					      __FILE__, __func__,
					      msgidnr, fileno(pipe));
					db_send_message_lines(pipe,
							      msgidnr, -2,
							      1);
				} else {
					/* only send the headers if there
					 * is no message to send. */
					trace(TRACE_DEBUG,
					      "%s,%s: writing header to "
					      "pipe", __FILE__,
					      __func__);
					fprintf(pipe, "%s", header);
				}

			}

			trace(TRACE_DEBUG, "%s,%s: closing pipes",
			      __FILE__, __func__);

			if (!ferror(pipe)) {
				pclose(pipe);
				trace(TRACE_DEBUG,
				      "%s,%s: pipe closed",
				      __FILE__, __func__);
			} else {
				trace(TRACE_ERROR,
				      "%s,%s: error on pipe",
				      __FILE__, __func__);
			}
		} else {
			trace(TRACE_ERROR,
			      "%s,%s: Could not open pipe to [%s]",
			      __FILE__, __func__, sendmail);
		}
		target = target->nextnode;
	}
	return 0;
}
