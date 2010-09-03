/*
 * GGstWAVInputStream class: Open a URI using g_gst_wav_input_stream_new ().
 * Data is decoded using GStreamer and is then reencoded as a MP3
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

#ifndef __G_GST_WAV_INPUT_STREAM
#define __G_GST_WAV_INPUT_STREAM

#include <gio/gio.h>

#include "g-gst-input-stream.h"

G_BEGIN_DECLS

#define TYPE_G_GST_WAV_INPUT_STREAM         (g_gst_wav_input_stream_get_type ())
#define G_GST_WAV_INPUT_STREAM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				               TYPE_G_GST_WAV_INPUT_STREAM, \
					       GGstWAVInputStream))
#define G_GST_WAV_INPUT_STREAM_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				               TYPE_G_GST_WAV_INPUT_STREAM, \
					       GGstWAVInputStreamClass))
#define IS_G_GST_WAV_INPUT_STREAM(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				               TYPE_G_GST_WAV_INPUT_STREAM))
#define IS_G_GST_WAV_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
				               TYPE_G_GST_WAV_INPUT_STREAM_CLASS))
#define G_GST_WAV_INPUT_STREAM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				               TYPE_G_GST_WAV_INPUT_STREAM, \
					       GGstWAVInputStreamClass))
#define G_GST_WAV_INPUT_STREAM_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
					   TYPE_G_GST_WAV_INPUT_STREAM, \
					   GGstWAVInputStreamPrivate))

typedef struct GGstWAVInputStreamPrivate GGstWAVInputStreamPrivate;

typedef struct {
	GGstInputStream parent;
	GGstWAVInputStreamPrivate *priv;
} GGstWAVInputStream;

typedef struct {
	GGstInputStreamClass parent;
} GGstWAVInputStreamClass;

GType         g_gst_wav_input_stream_get_type (void);

GInputStream* g_gst_wav_input_stream_new         (GInputStream *stream);

G_END_DECLS

#endif /* __G_GST_WAV_INPUT_STREAM */
