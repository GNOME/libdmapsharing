/*
 * DMAPGstMP3InputStream class: Open a URI using dmap_gst_mp3_input_stream_new ().
 * Data is decoded using GStreamer and is then reencoded as an MP3
 * stream by the class's read operations.
 *
 * Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
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

#ifndef __DMAP_GST_MP3_INPUT_STREAM
#define __DMAP_GST_MP3_INPUT_STREAM

#include <gio/gio.h>

#include "dmap-gst-input-stream.h"

G_BEGIN_DECLS
#define DMAP_TYPE_GST_MP3_INPUT_STREAM         (dmap_gst_mp3_input_stream_get_type ())
#define DMAP_GST_MP3_INPUT_STREAM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				               DMAP_TYPE_GST_MP3_INPUT_STREAM, \
					       DMAPGstMP3InputStream))
#define DMAP_GST_MP3_INPUT_STREAM_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				               DMAP_TYPE_GST_MP3_INPUT_STREAM, \
					       DMAPGstMP3InputStreamClass))
#define IS_DMAP_GST_MP3_INPUT_STREAM(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				               DMAP_TYPE_GST_MP3_INPUT_STREAM))
#define IS_DMAP_GST_MP3_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
				               DMAP_TYPE_GST_MP3_INPUT_STREAM_CLASS))
#define DMAP_GST_MP3_INPUT_STREAM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				               DMAP_TYPE_GST_MP3_INPUT_STREAM, \
					       DMAPGstMP3InputStreamClass))
#define DMAP_GST_MP3_INPUT_STREAM_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
					   DMAP_TYPE_GST_MP3_INPUT_STREAM, \
					   DMAPGstMP3InputStreamPrivate))
typedef struct DMAPGstMP3InputStreamPrivate DMAPGstMP3InputStreamPrivate;

typedef struct
{
	DMAPGstInputStream parent;
	DMAPGstMP3InputStreamPrivate *priv;
} DMAPGstMP3InputStream;

typedef struct
{
	DMAPGstInputStreamClass parent;
} DMAPGstMP3InputStreamClass;

GType dmap_gst_mp3_input_stream_get_type (void);

GInputStream *dmap_gst_mp3_input_stream_new (GInputStream * stream);

G_END_DECLS
#endif /* __DMAP_GST_MP3_INPUT_STREAM */
