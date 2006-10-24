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

#include "dbmail.h"


struct cidrfilter * cidr_new(const char *str)
{
	struct cidrfilter *self;
	char *addr, *port, *mask;
	char *haddr, *hport;
	unsigned i;
	size_t l;

	assert(str != NULL);
	
	self = (struct cidrfilter *)malloc(sizeof(struct cidrfilter));
	self->sock_str = strdup(str);
	self->socket = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
	self->mask = 32;

	addr = g_strdup(str);
	haddr = addr;
	while (*addr && *addr != ':')
		addr++;
	if (*addr == ':')
		addr++;
	
	port = g_strdup(addr);
	hport = port;
	while (*port && *port != ':')
		port++;
	if (*port == ':')
		port++;

	/* chop port */
	l = strlen(addr);
	for (i=0; i<l; i++) {
		if (addr[i] == ':') {
			addr[i]='\0';
			break;
		}
	}
	
	mask = index(addr,'/');
	if (mask && mask[1] != '\0') {
		mask++;
		self->mask = atoi(mask);

		/* chop mask */
		l = strlen(addr);
		for(i=0; i<l; i++) {
			if (addr[i] == '/') {
				addr[i]='\0';
				break;
			}
		}
	}
	

	self->socket->sin_family = AF_INET;
	self->socket->sin_port = strtol(port,NULL,10);
	if (! inet_aton(addr,&self->socket->sin_addr)) {
		free(haddr);
		free(hport);
		cidr_free(self);
		return NULL;
	}
		
	free(haddr);
	free(hport);
	
	return self;
}

int cidr_repr(struct cidrfilter *self) 
{
	return printf("struct cidrfilter {\n"
			"\tsock_str: %s;\n"
			"\tsocket->sin_addr: %s;\n"
			"\tsocket->sin_port: %d;\n"
			"\tmask: %d;\n"
			"};\n",
			self->sock_str,
			inet_ntoa(self->socket->sin_addr), 
			self->socket->sin_port,
			self->mask
			);
}

int cidr_match(struct cidrfilter *base, struct cidrfilter *test)
{
	char *fullmask = "255.255.255.255";
	struct in_addr match_addr, base_addr, test_addr;
	inet_aton(fullmask, &base_addr);
	inet_aton(fullmask, &test_addr);
	unsigned result = 0;

	if (base->mask)
		base_addr.s_addr = ~((unsigned long)base_addr.s_addr >> (32-base->mask));
	if (test->mask)
		test_addr.s_addr = ~((unsigned long)test_addr.s_addr >> (32-test->mask));

	base_addr.s_addr = (base->socket->sin_addr.s_addr | base_addr.s_addr);
	test_addr.s_addr = (test->socket->sin_addr.s_addr | test_addr.s_addr);
	match_addr.s_addr = (base_addr.s_addr & test_addr.s_addr);

	/* only match ports if specified */
	if (test->socket->sin_port > 0 && (base->socket->sin_port != test->socket->sin_port))
		return 0;
	
	if (test_addr.s_addr == match_addr.s_addr) 
		result = base->mask ? base->mask : 32;
			
	return result;
	
}

void cidr_free(struct cidrfilter *self)
{
	if (! self)
		return;

	if (self->socket)
		free(self->socket);
	if (self->sock_str)
		free(self->sock_str);
	if (self)
		free(self);
}

