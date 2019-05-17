/* Copyright (C) 2006 William Jon McCann <mccann@jhu.edu>
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

#include "config.h"

#include <glib.h>
#include <avahi-glib/glib-malloc.h>
#include <avahi-glib/glib-watch.h>
#include <avahi-common/error.h>

#include "dmap-mdns-avahi.h"

static AvahiClient *_client = NULL;
static AvahiEntryGroup *_entry_group = NULL;
static gsize _client_init = 0;

static void
_client_cb (G_GNUC_UNUSED AvahiClient * client,
            AvahiClientState state,
            G_GNUC_UNUSED gpointer data)
{
	/* FIXME
	 * check to make sure we're in the _RUNNING state before we publish
	 * check for COLLISION state and remove published information
	 */

	/* Called whenever the client or server state changes */

	switch (state) {
	case AVAHI_CLIENT_S_RUNNING:
		/* The server has startup successfully and registered its host
		 * name on the network, so it's time to create our services
		 */

		break;

	case AVAHI_CLIENT_S_COLLISION:

		/* Let's drop our registered services. When the server is back
		 * in AVAHI_SERVER_RUNNING state we will register them
		 * again with the new host name.
		 */
		if (_entry_group) {
			avahi_entry_group_reset (_entry_group);
		}
		break;

	case AVAHI_CLIENT_FAILURE:
		g_warning ("Client failure: %s",
			   avahi_strerror (avahi_client_errno (_client)));
		break;

	case AVAHI_CLIENT_CONNECTING:
	case AVAHI_CLIENT_S_REGISTERING:
	default:
		break;
	}
}

AvahiClient *
dmap_mdns_avahi_get_client (void)
{
	if (g_once_init_enter (&_client_init)) {
		AvahiClientFlags flags = 0;
		AvahiGLibPoll *apoll;
		int error = 0;

		avahi_set_allocator (avahi_glib_allocator ());

		apoll = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);
		if (apoll == NULL) {
			g_warning
				("Unable to create AvahiGlibPoll object for mDNS");
		}

		_client = avahi_client_new (avahi_glib_poll_get (apoll),
					   flags,
					   (AvahiClientCallback) _client_cb,
					   NULL, &error);
		if (error != 0) {
			g_warning ("Unable to initialize mDNS: %s",
				   avahi_strerror (error));
		}

		g_once_init_leave (&_client_init, 1);
	}

	return _client;
}

void
dmap_mdns_avahi_set_entry_group (AvahiEntryGroup * eg)
{
	/* FIXME: No longer a valid assumption with new multiple-protocol
	 * per process code. Refactor?
	 * g_assert (eg == NULL || entry_group == NULL);
	 */
	g_assert (avahi_entry_group_get_client (eg) == _client);
	_entry_group = eg;
}
