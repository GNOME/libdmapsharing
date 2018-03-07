/* Copyright (C) 2008 Jonathan Matthew  <jonathan@d14n.org>
 *
 * Modifications Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef _DMAP_MDNS_AVAHI_H
#define _DMAP_MDNS_AVAHI_H

#include <avahi-client/client.h>
#include <avahi-client/publish.h>

AvahiClient *dmap_mdns_avahi_get_client (void);

void dmap_mdns_avahi_set_entry_group (AvahiEntryGroup * group);

#endif
