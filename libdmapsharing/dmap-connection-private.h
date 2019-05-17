/* Header for DMAP (e.g., iTunes Music or iPhoto Picture) sharing
 *
 * Copyright (C) 2018 W. Michael Petullo <mike@flyn.org>
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

#include <glib-object.h>

#include <libdmapsharing/dmap-connection.h>

#ifndef _DMAP_CONNECTION_PRIVATE_H
#define _DMAP_CONNECTION_PRIVATE_H

typedef void (*DmapResponseHandler) (DmapConnection * connection,
				     guint status,
				     GNode * structure,
                                     gpointer user_data);

gboolean dmap_connection_get (DmapConnection * self,
                              const gchar * path,
                              DmapResponseHandler handler,
                              gpointer user_data);

void dmap_connection_setup (DmapConnection * connection);

#endif
