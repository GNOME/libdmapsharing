/*
 * DmapTranscodeQtStream class: Open a URI using dmap_transcode_qt_stream_new ().
 * Data is decoded using GStreamer and is then reencoded as a QuickTime video
 * stream by the class's read operations.
 *
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include "dmap-transcode-qt-stream.h"
#include "dmap-transcode-stream-private.h"
#include "gst-util.h"

#define GST_APP_MAX_BUFFERS 1024

struct DmapTranscodeQtStreamPrivate
{
	GstElement *pipeline;
	GstElement *src;
	GstElement *decode;
	GstElement *convert;
	GstElement *audio_encode;
	GstElement *mux;
	GstElement *sink;
};

static void
_pad_added_cb (G_GNUC_UNUSED GstElement * element,
               GstPad * pad,
               GstElement *convert)
{
	/* Link remaining pad after decodebin2 does its magic. */
	GstPad *conv_pad;

	conv_pad = gst_element_get_static_pad (convert, "sink");
	g_assert (conv_pad != NULL);

	if (gst_util_pads_compatible (pad, conv_pad)) {
		g_assert (!GST_PAD_IS_LINKED
			  (gst_element_get_static_pad
			   (convert, "sink")));

		gst_pad_link (pad, conv_pad);
	} else {
		g_warning ("Could not link GStreamer pipeline.");
	}
}

GInputStream *
dmap_transcode_qt_stream_new (GInputStream * src_stream)
{
	GstStateChangeReturn sret;
	GstState state;
	DmapTranscodeQtStream *stream = NULL;

	GstElement *pipeline = NULL;
        GstElement *src = NULL;
        GstElement *decode = NULL;
        GstElement *convert = NULL;
        GstElement *audio_encode = NULL;
	GstElement *mux = NULL;
        GstElement *sink = NULL;

	g_assert (G_IS_INPUT_STREAM (src_stream));

	pipeline = gst_pipeline_new ("pipeline");
        if (NULL == pipeline) {
                g_warning ("Could not create GStreamer pipeline");
                goto done;
        }

	src = gst_element_factory_make ("giostreamsrc", "src");
        if (NULL == src) {
                g_warning ("Could not create GStreamer giostreamsrc element");
                goto done;
        }

	decode = gst_element_factory_make ("decodebin", "decode");
        if (NULL == decode) {
                g_warning ("Could not create GStreamer decodebin element");
                goto done;
        }

	convert = gst_element_factory_make ("audioconvert", "convert");
        if (NULL == convert) {
                g_warning ("Could not create GStreamer audioconvert element");
                goto done;
        }

	audio_encode = gst_element_factory_make ("avenc_aac", "audioencode");
        if (NULL == audio_encode) {
                g_warning ("Could not create GStreamer avenc_aac element");
                goto done;
        }

	mux = gst_element_factory_make ("qtmux", "mux");
        if (NULL == audio_encode) {
                g_warning ("Could not create GStreamer qtmux element");
                goto done;
        }

	sink = gst_element_factory_make ("appsink", "sink");
        if (NULL == sink) {
                g_warning ("Could not create GStreamer appsink element");
                goto done;
        }

	gst_bin_add_many (GST_BIN (pipeline), src, decode, convert, audio_encode, mux, sink, NULL);

	if (FALSE == gst_element_link (src, decode)) {
		g_warning ("Error linking source and decode elements");
		goto done;
	}

	if (FALSE == gst_element_link_many (convert, audio_encode, mux, sink, NULL)) {
		g_warning ("Error linking convert through sink elements");
		goto done;
	}

	g_object_set (G_OBJECT (src), "stream", src_stream, NULL);

	g_object_set (G_OBJECT (sink), "emit-signals", TRUE, "sync", FALSE, NULL);
	gst_app_sink_set_max_buffers (GST_APP_SINK (sink), GST_APP_MAX_BUFFERS);
	gst_app_sink_set_drop (GST_APP_SINK (sink), FALSE);

	g_signal_connect (decode, "pad-added", G_CALLBACK (_pad_added_cb), convert);

	/* FIXME: this technique is shared with dmapd-dmap-av-share.c */
	sret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
	if (GST_STATE_CHANGE_ASYNC == sret) {
		if (GST_STATE_CHANGE_SUCCESS !=
		    gst_element_get_state (GST_ELEMENT (pipeline), &state, NULL, 5 * GST_SECOND)) {
			g_warning ("State change failed for stream.");
			goto done;
		}
	} else if (sret != GST_STATE_CHANGE_SUCCESS) {
		g_warning ("Could not read stream.");
		goto done;
	}

	stream = DMAP_TRANSCODE_QT_STREAM (g_object_new (DMAP_TYPE_GST_QT_INPUT_STREAM, NULL));
	if (NULL == stream) {
                goto done;
        }
        g_assert (G_IS_SEEKABLE (stream));

	g_signal_connect (sink, "new-sample", G_CALLBACK (dmap_transcode_stream_private_new_buffer_cb), stream);

	stream->priv->pipeline = gst_object_ref (pipeline);
        stream->priv->src = gst_object_ref (src);
        stream->priv->decode = gst_object_ref (decode);
        stream->priv->convert = gst_object_ref (convert);
        stream->priv->audio_encode = gst_object_ref (audio_encode);
        stream->priv->mux = gst_object_ref (mux);
        stream->priv->sink = gst_object_ref (sink);

done:
	if (pipeline) {
                gst_object_unref (pipeline);
        }

        if (src) {
                gst_object_unref (src);
        }

        if (decode) {
                gst_object_unref (decode);
        }

        if (convert) {
                gst_object_unref (convert);
        }

        if (audio_encode) {
                gst_object_unref (audio_encode);
        }

        if (mux) {
                gst_object_unref (mux);
        }

        if (sink) {
                gst_object_unref (sink);
        }

	return G_INPUT_STREAM (stream);
}

static void
_kill_pipeline (DmapTranscodeStream * stream)
{
	DmapTranscodeQtStream *qt_stream =
		DMAP_TRANSCODE_QT_STREAM (stream);

	// FIXME: It seems that I need to send an EOS, because QuickTime writes
	// its headers after encoding the streams, but this does not yet work.
	gst_element_send_event(qt_stream->priv->pipeline, gst_event_new_eos());
	
	gst_element_set_state (qt_stream->priv->pipeline, GST_STATE_NULL);
	gst_object_unref (GST_OBJECT (qt_stream->priv->pipeline));
}

G_DEFINE_TYPE_WITH_PRIVATE (DmapTranscodeQtStream,
                            dmap_transcode_qt_stream,
                            DMAP_TYPE_TRANSCODE_STREAM);

static void
dmap_transcode_qt_stream_class_init (DmapTranscodeQtStreamClass * klass)
{
	DmapTranscodeStreamClass *parent_class =
		DMAP_TRANSCODE_STREAM_CLASS (klass);

	parent_class->kill_pipeline = _kill_pipeline;
}

static void
dmap_transcode_qt_stream_init (DmapTranscodeQtStream * stream)
{
	stream->priv = dmap_transcode_qt_stream_get_instance_private(stream);
}
