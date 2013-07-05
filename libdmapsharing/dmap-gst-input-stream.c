/*
 * DMAPGstInputStream class: Open a URI using dmap_gst_input_stream_new ().
 * Data is decoded using GStreamer and is then made available by the class's
 * read operations.
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

#include <string.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include "dmap-gst-input-stream.h"
#include "dmap-gst-mp3-input-stream.h"
#include "dmap-gst-wav-input-stream.h"
#include "dmap-gst-qt-input-stream.h"
#include "gst-util.h"

#define GST_APP_MAX_BUFFERS 1024
#define DECODED_BUFFER_SIZE 1024 * 128
#define QUEUE_PUSH_WAIT_SECONDS 10
#define QUEUE_POP_WAIT_SECONDS 1

struct dmap_gst_format
{
	gchar *id;		/* E.g., used as command line arguments. */
	gchar *extension;	/* E.g., iTunes uses URI extension to
				 * determine stream format.
				 */
};

/* NOTE: Roku clients require lower case extension. */
static const struct dmap_gst_format dmap_gst_formats[] = {
	{"raw", "raw"},
	{"wav16", "wav"},
	{"mp3", "mp3"},
	{NULL, NULL}
};

struct DMAPGstInputStreamPrivate
{
	GQueue *buffer;
	gsize read_request;	/* Size of data asked for */
	gsize write_request;	/* Number of bytes that must be read
				 * to make room for write */
	GCond *buffer_read_ready;	/* Signals when buffer >= read_req. */
	GCond *buffer_write_ready;	/* Signals when buffer not full. */
	GMutex *buffer_mutex;	/* Protects buffer and read_request */
	gboolean buffer_closed;	/* May close before decoding complete */
};

#define DMAP_GST_INPUT_STREAM_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
					   DMAP_TYPE_GST_INPUT_STREAM, \
					   DMAPGstInputStreamPrivate))

static goffset
dmap_gst_input_stream_tell (GSeekable * seekable)
{
	/* FIXME: implement return current position in stream. */
	g_error ("Not implemented");
	return 0;
}

static gboolean
dmap_gst_input_stream_can_seek (GSeekable * seekable)
{
	return TRUE;
}

static gboolean
dmap_gst_input_stream_seek (GSeekable * seekable,
			    goffset offset,
			    GSeekType type,
			    GCancellable * cacellable, GError ** error)
{
	// FIXME: implement: DMAPGstInputStream *stream;
	// FIXME: implement: goffset absolute;

	// FIXME: implement: stream = DMAP_GST_INPUT_STREAM (seekable);

	switch (type) {
		/* FIXME: implement:
		 * case G_SEEK_CUR:
		 *      absolute = stream->priv->pos + offset;
		 *      break;
		 *
		 * case G_SEEK_END:
		 *      absolute = stream->priv->len + offset;
		 *      break;
		 */

	case G_SEEK_SET:
		// FIXME: implement: absolute = offset;
		break;

	default:
		g_set_error (error,
			     G_IO_ERROR,
			     G_IO_ERROR_INVALID_ARGUMENT,
			     "Invalid GSeekType supplied");

		return FALSE;
	}

	/* FIXME: implement:
	 * if (absolute < 0 || absolute > stream->priv->len) {
	 *      g_set_error_literal (error,
	 *              G_IO_ERROR,
	 *              G_IO_ERROR_INVALID_ARGUMENT,
	 *              _("Invalid seek request"));
	 *      return FALSE;
	 * }
	 */

	/* FIXME:
	 * if (! gst_element_seek_simple (DMAP_GST_INPUT_STREAM (seekable)->priv->pipeline,
	 * GST_FORMAT_BYTES,
	 * GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
	 * absolute)) {
	 * g_set_error (error,
	 * G_IO_ERROR,
	 * G_IO_ERROR_FAILED,
	 * "Seek failed");
	 * return FALSE;
	 * }
	 */

	return TRUE;
}

static gboolean
dmap_gst_input_stream_can_truncate (GSeekable * seekable)
{
	return FALSE;
}

static gboolean
dmap_gst_input_stream_truncate (GSeekable * seekable,
				goffset offset,
				GCancellable * cancellable, GError ** error)
{
	g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
		     "Cannot truncate DMAPGstInputStream");
	return FALSE;
}

static void
dmap_gst_input_stream_seekable_iface_init (GSeekableIface * iface)
{
	iface->tell = dmap_gst_input_stream_tell;
	iface->can_seek = dmap_gst_input_stream_can_seek;
	iface->seek = dmap_gst_input_stream_seek;
	iface->can_truncate = dmap_gst_input_stream_can_truncate;
	iface->truncate_fn = dmap_gst_input_stream_truncate;
}

void
dmap_gst_input_stream_new_buffer_cb (GstElement * element,
				     DMAPGstInputStream * stream)
{
	gsize i;
	guint8 *ptr;
	gint64 end_time;
	GstSample *sample = NULL;
	GstBuffer *buffer = NULL;
	GstMemory *memory = NULL;
	GstMapInfo info;

	/* FIXME: Is this necessary? I am trying to protect against this
	 * thread manipulating data after the pipeline has been destroyed.
	 * see also dmap_gst_input_stream_close ().
	 */
	g_mutex_lock (stream->priv->buffer_mutex);

	if (stream->priv->buffer_closed) {
		g_warning ("Unread data");
		goto _return;
	}

	end_time = end_time = g_get_monotonic_time () + QUEUE_PUSH_WAIT_SECONDS * G_TIME_SPAN_SECOND;

	sample = gst_app_sink_pull_sample (GST_APP_SINK (element));
	if (NULL == sample) {
		g_warning ("Error getting GStreamer sample");
		goto _return;
	}

	buffer = gst_sample_get_buffer (sample);
	if (NULL == buffer) {
		g_warning ("Error getting GStreamer buffer");
		goto _return;
	}

	memory = gst_buffer_get_memory (buffer, 0);
	if (NULL == memory) {
		g_warning ("Error getting GStreamer memory");
		goto _return;
	}

	if (gst_memory_map (memory, &info, GST_MAP_READ) == FALSE) {
		g_warning ("Error mapping GStreamer memory");
		goto _return;
	}

	/* FIXME: this actually allows buffer to grow larger than max. */
	if (g_queue_get_length (stream->priv->buffer) +
	    info.size > DECODED_BUFFER_SIZE) {
		stream->priv->write_request = info.size;
		if (!g_cond_wait_until (stream->priv->buffer_write_ready,
					stream->priv->buffer_mutex, end_time)) {
			g_warning
				("Timeout waiting for buffer to empty; will drop");
		}
		/* Required again because g_cond_wait_until released mutex. */
		if (stream->priv->buffer_closed) {
			g_warning ("Unread data");
			goto _return;
		}
	} else {
		stream->priv->write_request = 0;
	}

	if (g_queue_get_length (stream->priv->buffer) +
	    info.size <= DECODED_BUFFER_SIZE) {
		ptr = info.data;

		for (i = 0; i < info.size; i++) {
			g_queue_push_tail (stream->priv->buffer,
					   GINT_TO_POINTER ((gint) * ptr++));
		}
	}

	if (g_queue_get_length (stream->priv->buffer)
	    >= stream->priv->read_request) {
		stream->priv->read_request = 0;
		g_cond_signal (stream->priv->buffer_read_ready);
	}

      _return:
	if (NULL != memory) {
		gst_memory_unmap (memory, &info);
	}

	if (NULL != sample) {
		gst_sample_unref (sample);
	}

	g_mutex_unlock (stream->priv->buffer_mutex);
}

GInputStream *
dmap_gst_input_stream_new (const gchar * transcode_mimetype,
			   GInputStream * src_stream)
{
	GInputStream *stream;

	if (!transcode_mimetype) {
		stream = src_stream;
	} else if (!strcmp (transcode_mimetype, "audio/mp3")) {
		stream = G_INPUT_STREAM (dmap_gst_mp3_input_stream_new
					 (src_stream));
	} else if (!strcmp (transcode_mimetype, "audio/wav")) {
		stream = G_INPUT_STREAM (dmap_gst_wav_input_stream_new
					 (src_stream));
	} else if (!strcmp (transcode_mimetype, "video/quicktime")) {
		stream = G_INPUT_STREAM (dmap_gst_qt_input_stream_new
					 (src_stream));
	} else {
		g_warning ("Transcode format %s not supported",
			   transcode_mimetype);
		stream = src_stream;
	}

	return stream;
}

gchar *
dmapd_input_stream_strdup_format_extension (const gint format_code)
{
	return g_strdup (dmap_gst_formats[format_code].extension);
}

static gssize
min (gssize a, gssize b)
{
	return a < b ? a : b;
}

static gssize
dmap_gst_input_stream_read (GInputStream * stream,
			    void *buffer,
			    gsize count,
			    GCancellable * cancellable, GError ** error)
{
	int i;
	DMAPGstInputStream *gst_stream = DMAP_GST_INPUT_STREAM (stream);
	gint64 end_time;

	end_time = end_time = g_get_monotonic_time () + QUEUE_POP_WAIT_SECONDS * G_TIME_SPAN_SECOND;

	g_mutex_lock (gst_stream->priv->buffer_mutex);

	gst_stream->priv->read_request = count;
	if (g_queue_get_length (gst_stream->priv->buffer) < count
	    && !g_cond_wait_until (gst_stream->priv->buffer_read_ready,
				   gst_stream->priv->buffer_mutex, end_time)) {
		/* Timeout: Count is now what's remaining.  Let's hope
		 * we have enough of a lead on encoding so that this one
		 * second timeout will go unnoticed.
		 */
		g_warning ("Timeout waiting for converted data");
		/* Depending on timing, more data may have been written
		 * since check: do not pull more than count:
		 */
		count = min (count, g_queue_get_length (gst_stream->priv->buffer));
	}

	for (i = 0; i < count; i++) {
		((guint8 *) buffer)[i] =
			GPOINTER_TO_INT (g_queue_pop_head (gst_stream->priv->buffer));
	}

	if (gst_stream->priv->write_request > count)
		gst_stream->priv->write_request -= count;
	else
		gst_stream->priv->write_request = 0;

	if (gst_stream->priv->write_request <= 0) {
		g_cond_signal (gst_stream->priv->buffer_write_ready);
	}

	g_mutex_unlock (gst_stream->priv->buffer_mutex);

	return count;
}

static gssize
dmap_gst_input_stream_skip (GInputStream * stream,
			    gsize count,
			    GCancellable * cancellable, GError ** error)
{
	g_error ("Not implemented");
	return 0;
}

static void
dmap_gst_input_stream_kill_pipeline (DMAPGstInputStream * stream)
{
	DMAP_GST_INPUT_STREAM_GET_CLASS (stream)->kill_pipeline (stream);
}

static gboolean
dmap_gst_input_stream_close (GInputStream * stream,
			     GCancellable * cancellable, GError ** error)
{
	DMAPGstInputStream *gst_stream = DMAP_GST_INPUT_STREAM (stream);

	dmap_gst_input_stream_kill_pipeline (gst_stream);

	g_mutex_lock (gst_stream->priv->buffer_mutex);

	g_queue_free (gst_stream->priv->buffer);
	gst_stream->priv->buffer_closed = TRUE;

	g_mutex_unlock (gst_stream->priv->buffer_mutex);

	return TRUE;
}

static gssize
dmap_gst_input_stream_read_finish (GInputStream * stream,
				   GAsyncResult * result, GError ** error)
{
	g_error ("Not implemented");
	return 0;
}

static gssize
dmap_gst_input_stream_skip_finish (GInputStream * stream,
				   GAsyncResult * result, GError ** error)
{
	g_error ("Not implemented");
	return 0;
}

static void
dmap_gst_input_stream_close_async (GInputStream * stream,
				   int io_priority,
				   GCancellable * cancellabl,
				   GAsyncReadyCallback callback,
				   gpointer data)
{
}

static void
dmap_gst_input_stream_read_async (GInputStream * stream,
				  void *buffer,
				  gsize count,
				  int io_priority,
				  GCancellable * cancellable,
				  GAsyncReadyCallback callback,
				  gpointer user_data)
{
}

static void
dmap_gst_input_stream_skip_async (GInputStream * stream,
				  gsize count,
				  int io_priority,
				  GCancellable * cancellabl,
				  GAsyncReadyCallback callback,
				  gpointer datae)
{
}

static gboolean
dmap_gst_input_stream_close_finish (GInputStream * stream,
				    GAsyncResult * result, GError ** error)
{
	g_error ("Not implemented");
	return FALSE;
}

static void
dmap_gst_input_stream_class_init (DMAPGstInputStreamClass * klass)
{
	GInputStreamClass *istream_class;

	g_type_class_add_private (klass, sizeof (DMAPGstInputStreamPrivate));

	istream_class = G_INPUT_STREAM_CLASS (klass);
	istream_class->read_fn = dmap_gst_input_stream_read;
	istream_class->skip = dmap_gst_input_stream_skip;
	istream_class->close_fn = dmap_gst_input_stream_close;
	istream_class->read_async = dmap_gst_input_stream_read_async;
	istream_class->read_finish = dmap_gst_input_stream_read_finish;
	istream_class->skip_async = dmap_gst_input_stream_skip_async;
	istream_class->skip_finish = dmap_gst_input_stream_skip_finish;
	istream_class->close_async = dmap_gst_input_stream_close_async;
	istream_class->close_finish = dmap_gst_input_stream_close_finish;
}

static void
dmap_gst_input_stream_init (DMAPGstInputStream * stream)
{
	stream->priv = DMAP_GST_INPUT_STREAM_GET_PRIVATE (stream);

	stream->priv->buffer = g_queue_new ();
	stream->priv->read_request = 0;
	stream->priv->write_request = 0;
	stream->priv->buffer_closed = FALSE;

	// FIXME: Never g_mutex_clear'ed:
	g_mutex_init (stream->priv->buffer_mutex);

	// FIXME: Never g_cond_clear'ed:
	g_cond_init (stream->priv->buffer_read_ready);
	g_cond_init (stream->priv->buffer_write_ready);
}

G_DEFINE_TYPE_WITH_CODE (DMAPGstInputStream, dmap_gst_input_stream,
			 G_TYPE_INPUT_STREAM,
			 G_IMPLEMENT_INTERFACE (G_TYPE_SEEKABLE,
						dmap_gst_input_stream_seekable_iface_init));
