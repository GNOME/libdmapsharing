/*
 * DmapTranscodeWavStream class: Open a URI using dmap_transcode_wav_stream_new ().
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

#ifndef _DMAP_TRANSCODE_WAV_STREAM_H
#define _DMAP_TRANSCODE_WAV_STREAM_H

#include <gio/gio.h>

#include "dmap-transcode-stream.h"

G_BEGIN_DECLS
#define DMAP_TYPE_GST_WAV_INPUT_STREAM         (dmap_transcode_wav_stream_get_type ())
#define DMAP_TRANSCODE_WAV_STREAM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				               DMAP_TYPE_GST_WAV_INPUT_STREAM, \
					       DmapTranscodeWavStream))
#define DMAP_TRANSCODE_WAV_STREAM_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				               DMAP_TYPE_GST_WAV_INPUT_STREAM, \
					       DmapTranscodeWavStreamClass))
#define DMAP_IS_GST_WAV_INPUT_STREAM(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				               DMAP_TYPE_GST_WAV_INPUT_STREAM))
#define DMAP_IS_GST_WAV_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
				               DMAP_TYPE_GST_WAV_INPUT_STREAM_CLASS))
#define DMAP_TRANSCODE_WAV_STREAM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				               DMAP_TYPE_GST_WAV_INPUT_STREAM, \
					       DmapTranscodeWavStreamClass))
typedef struct DmapTranscodeWavStreamPrivate DmapTranscodeWavStreamPrivate;

typedef struct {
	DmapTranscodeStream parent;
	DmapTranscodeWavStreamPrivate *priv;
} DmapTranscodeWavStream;

typedef struct {
	DmapTranscodeStreamClass parent;
} DmapTranscodeWavStreamClass;

GType dmap_transcode_wav_stream_get_type (void);

GInputStream *dmap_transcode_wav_stream_new (GInputStream * stream);

G_END_DECLS
#endif
