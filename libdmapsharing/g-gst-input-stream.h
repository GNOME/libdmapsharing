/*
 * GGstInputStream class: Open a URI using g_gst_input_stream_new ().
 * Data is decoded using GStreamer and is then made available by the class's
 * read operations.
 *
 * Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __G_GST_INPUT_STREAM
#define __G_GST_INPUT_STREAM

#include <gio/gio.h>
#include <gst/gst.h>

G_BEGIN_DECLS

#define TYPE_G_GST_INPUT_STREAM         (g_gst_input_stream_get_type ())
#define G_GST_INPUT_STREAM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				               TYPE_G_GST_INPUT_STREAM, \
					       GGstInputStream))
#define G_GST_INPUT_STREAM_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				               TYPE_G_GST_INPUT_STREAM, \
					       GGstInputStreamClass))
#define IS_G_GST_INPUT_STREAM(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				               TYPE_G_GST_INPUT_STREAM))
#define IS_G_GST_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
				               TYPE_G_GST_INPUT_STREAM_CLASS))
#define G_GST_INPUT_STREAM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				               TYPE_G_GST_INPUT_STREAM, \
					       GGstInputStreamClass))
#define G_GST_INPUT_STREAM_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
					   TYPE_G_GST_INPUT_STREAM, \
					   GGstInputStreamPrivate))

typedef struct GGstInputStreamPrivate GGstInputStreamPrivate;

typedef struct {
	GInputStream parent;
	GGstInputStreamPrivate *priv;
} GGstInputStream;

typedef struct {
	GInputStreamClass parent;

	void (*kill_pipeline) (GGstInputStream *);
} GGstInputStreamClass;

GType         g_gst_input_stream_get_type (void);

/* Supported transcode target formats (data read from GGstInputStream
 * will be in one of these formats): */
enum {
	RAW, /* No transcoding performed. */
        WAV16,
	MP3
};

GInputStream* g_gst_input_stream_new             (const gchar *uri);

void g_gst_input_stream_new_buffer_cb		 (GstElement *element,
						  GGstInputStream *stream);

gchar *dmapd_input_stream_strdup_format_extension (const gint format_code);

G_END_DECLS

#endif /* __G_GST_INPUT_STREAM */
