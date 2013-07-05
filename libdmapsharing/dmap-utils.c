/*
 * Utility functions used throughout libdmapsharing
 *
 * Copyright (C) 2010 W. Michael Petullo <mike@flyn.org>
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
 */

#include <string.h>

#include "dmap-utils.h"

gchar *
dmap_mime_to_format (const gchar * transcode_mimetype)
{
        if (!transcode_mimetype) {
                return NULL;
        } else if (!strcmp (transcode_mimetype, "audio/wav")) {
                return g_strdup ("wav");
        } else if (!strcmp (transcode_mimetype, "audio/mp3")) {
                return g_strdup ("mp3");
        } else if (!strcmp (transcode_mimetype, "video/quicktime")) {
                return g_strdup ("mp4");
        } else
                return NULL;
}
