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

#define DMAP_SHARE_CHUNK_SIZE 16384

void
dmap_write_next_chunk (SoupMessage * message, ChunkData * cd)
{
	gssize read_size;
	GError *error = NULL;
	gchar *chunk = g_malloc (DMAP_SHARE_CHUNK_SIZE);

	read_size = g_input_stream_read (cd->stream,
					 chunk,
					 DMAP_SHARE_CHUNK_SIZE, NULL, &error);
	if (read_size > 0) {
		soup_message_body_append (message->response_body,
					  SOUP_MEMORY_TAKE, chunk, read_size);
	} else {
		if (error != NULL) {
			g_warning ("Error reading from input stream: %s",
				   error->message);
			g_error_free (error);
		}
		g_free (chunk);
		g_debug ("Wrote 0 bytes, sending message complete.");
		soup_message_body_complete (message->response_body);
	}
	soup_server_unpause_message (cd->server, message);
}

void
dmap_chunked_message_finished (SoupMessage * message, ChunkData * cd)
{
	g_debug ("Finished sending chunked file.");
	g_input_stream_close (cd->stream, NULL, NULL);
	g_free (cd);
}
