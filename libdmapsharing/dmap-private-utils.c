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

#include "dmap-private-utils.h"

void
dmap_private_utils_write_next_chunk (SoupServerMessage * message, ChunkData * cd)
{
	gssize read_size;
	GError *error = NULL;
	gchar *chunk = g_malloc (DMAP_SHARE_CHUNK_SIZE);

	g_debug ("Trying to read %d bytes.", DMAP_SHARE_CHUNK_SIZE);
	read_size = g_input_stream_read (cd->stream,
					 chunk,
					 DMAP_SHARE_CHUNK_SIZE, NULL, &error);
	if (read_size > 0) {
		soup_message_body_append (soup_server_message_get_response_body(message),
					  SOUP_MEMORY_TAKE, chunk, read_size);
		g_debug ("Read/wrote %"G_GSSIZE_FORMAT" bytes.", read_size);
	} else {
		if (error != NULL) {
			g_warning ("Error reading from input stream: %s",
				   error->message);
			g_error_free (error);
		}
		g_free (chunk);
		g_debug ("Wrote 0 bytes, sending message complete.");
		soup_message_body_complete (soup_server_message_get_response_body(message));
	}
	soup_server_message_unpause (message);
}

void
dmap_private_utils_chunked_message_finished (G_GNUC_UNUSED SoupServerMessage * message, ChunkData * cd)
{
	g_debug ("Finished sending chunked file.");
	g_input_stream_close (cd->stream, NULL, NULL);

	if (cd->original_stream) {
		g_input_stream_close (cd->original_stream, NULL, NULL);
	}

	g_free (cd);
}
