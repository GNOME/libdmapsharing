/*
 * Copyright (C) 2009 W. Michael Petullo <mike@flyn.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*
 */

#include "config.h"

#include <stdio.h>
#include <glib.h>
#include <dns_sd.h>

#include "dmap-mdns-browser.h"

DMAPMdnsBrowser *
dmap_mdns_browser_new (DMAPMdnsBrowserServiceType type)
{
	g_error ("Not implemented");
	return NULL;
}

gboolean
dmap_mdns_browser_start (DMAPMdnsBrowser *browser,
                         GError **error)
{
	g_error ("Not implemented");
	return FALSE;
}

gboolean
dmap_mdns_browser_stop (DMAPMdnsBrowser *browser,
                        GError **error)
{
	g_error ("Not implemented");
	return FALSE;
}
