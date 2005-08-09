/*
  $Id: rfcmsg.h 1756 2005-04-19 07:37:27Z paul $

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

/*
 * rfcmsg.h
 * 
 * function prototypes to enable message parsing
 */

#ifndef _RFCMSG_H
#define _RFCMSG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dbmailtypes.h"

void db_free_msg(mime_message_t * msg);
void db_reverse_msg(mime_message_t * msg);

int db_fetch_headers(u64_t msguid, mime_message_t * msg);
int db_add_mime_children(struct dm_list *brothers, char *splitbound,
			 int *level, int maxlevel);
int db_start_msg(mime_message_t * msg, char *stopbound, int *level,
		 int maxlevel);
int db_parse_as_text(mime_message_t * msg);
int db_msgdump(mime_message_t * msg, u64_t msguid, int level);

#endif
