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

/*
 * $Id: config.c 1820 2005-07-17 08:23:37Z aaron $
 * \file config.c
 * \brief read configuration values from a config file
 */

#include "dbmail.h"

/** dictionary which holds the configuration */
static dictionary *config_dict = NULL;

static int configured = 0;
/**
 * read the configuration file and stores the configuration
 * directives in an internal structure.
 */
int config_read(const char *config_filename)
{
        char *config_filename_copy;
        int result = 0;
	
	if (configured++) 
		return 0;
        
	
	assert(config_filename != NULL);
        
        config_filename_copy = g_strdup(config_filename);
        config_dict = iniparser_load(config_filename_copy);
        if (config_dict == NULL) {
                trace(CONFIG_ERROR_LEVEL, "%s,%s: error reading "
                      "config file %s", __FILE__, __func__,
                      config_filename);
                result = -1;
        } 
        
        g_free(config_filename_copy);
        return result;
}

/**
 * free all memory related to config 
 */
void config_free(void) 
{
	if (--configured) 
		return;
	
	
	iniparser_freedict(config_dict);
}

/* FIXME: Always returns 0, which is dandy for debugging. */
int config_get_value(const field_t field_name,
                     const char * const service_name,
                     field_t value) {
        char *dict_value;
        char *key;

        assert(service_name);
        assert(config_dict);
        
        key = g_strdup_printf("%s:%s", service_name, field_name);
        dict_value = iniparser_getstring(config_dict, key, NULL);
        if (dict_value)
                g_strlcpy(value, dict_value, FIELDSIZE);
        else
                value[0] = '\0';

	g_free(key);
        return 0;
}

void SetTraceLevel(const char *service_name)
{
	field_t val;

	if (config_get_value("trace_level", service_name, val) < 0)
		trace(TRACE_FATAL, "%s,%s: error getting config!",
		      __FILE__, __func__);
	if (strlen(val) == 0)
		configure_debug(TRACE_ERROR, 1, 0);
	else 
		configure_debug(atoi(val), 1, 0);
}

void GetDBParams(db_param_t * db_params)
{
	field_t port_string, sock_string, serverid_string;
	
	if (config_get_value("host", "DBMAIL", db_params->host) < 0)
		trace(TRACE_FATAL, "%s,%s: error getting config!",
		      __FILE__, __func__);
	if (config_get_value("db", "DBMAIL", db_params->db) < 0) 
		trace(TRACE_FATAL, "%s,%s: error getting config!",
		      __FILE__, __func__);
	if (config_get_value("user", "DBMAIL", db_params->user) < 0) 
		trace(TRACE_FATAL, "%s,%s: error getting config!",
		      __FILE__, __func__);
	if (config_get_value("pass", "DBMAIL", db_params->pass) < 0)
		trace(TRACE_FATAL, "%s,%s: error getting config!",
		      __FILE__, __func__);
	if (config_get_value("sqlport", "DBMAIL", port_string) < 0)
		trace(TRACE_FATAL, "%s,%s: error getting config!",
		      __FILE__, __func__);
	if (config_get_value("sqlsocket", "DBMAIL", sock_string) < 0)
		trace(TRACE_FATAL, "%s,%s: error getting config!",
		      __FILE__, __func__);
	if (config_get_value("serverid", "DBMAIL", serverid_string) < 0)
		trace(TRACE_FATAL, "%s,%s: error getting config!",
		      __FILE__, __func__);

	

	if (config_get_value("table_prefix", "DBMAIL", db_params->pfx) < 0)
		trace(TRACE_FATAL, "%s,%s: error getting config!",
		      __FILE__, __func__);
	if (strcmp(db_params->pfx, "\"\"") == 0) {
		/* FIXME: It appears that when the empty string is quoted
		 * that the quotes themselves are returned as the value. */
		g_strlcpy(db_params->pfx, "", FIELDSIZE);
	} else if (strlen(db_params->pfx) == 0) {
		/* If it's not "" but is zero length, set the default. */
		g_strlcpy(db_params->pfx, DEFAULT_DBPFX, FIELDSIZE);
	}

	/* check if port_string holds a value */
	if (strlen(port_string) != 0) {
		db_params->port =
		    (unsigned int) strtoul(port_string, NULL, 10);
		if (errno == EINVAL || errno == ERANGE)
			trace(TRACE_FATAL,
			      "%s,%s: wrong value for sqlport in "
			      "config file", __FILE__, __func__);
	} else
		db_params->port = 0;

	/* same for sock_string */
	if (strlen(sock_string) != 0)
		g_strlcpy(db_params->sock, sock_string, FIELDSIZE);
	else
		db_params->sock[0] = '\0';

	/* and serverid */
	if (strlen(serverid_string) != 0) {
		db_params->serverid = (unsigned int) strtol(serverid_string, NULL, 10);
		if (errno == EINVAL || errno == ERANGE)
			trace(TRACE_FATAL, "%s,%s: serverid invalid in config file",
					__FILE__, __func__);
	} else {
		db_params->serverid = 1;
	}
}

