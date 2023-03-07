/*
 * DmapTranscodeStream class: Open a URI using dmap_transcode_stream_new ().
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

#ifndef _DMAP_TRANSCODE_STREAM_H
#define _DMAP_TRANSCODE_STREAM_H

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: dmap-transcode-stream
 * @short_description: A transcoding #GInputStream wrapper.
 *
 * #DmapTranscodeStream objects wrap a #GInputStream in a way that transcodes the data therein.
 */

#define DMAP_TYPE_TRANSCODE_STREAM         (dmap_transcode_stream_get_type ())
#define DMAP_TRANSCODE_STREAM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
                                            DMAP_TYPE_TRANSCODE_STREAM, \
                                            DmapTranscodeStream))
#define DMAP_TRANSCODE_STREAM_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
                                            DMAP_TYPE_TRANSCODE_STREAM, \
                                            DmapTranscodeStreamClass))
#define DMAP_IS_TRANSCODE_STREAM(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
                                            DMAP_TYPE_TRANSCODE_STREAM))
#define DMAP_IS_TRANSCODE_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
                                            DMAP_TYPE_TRANSCODE_STREAM_CLASS))
#define DMAP_TRANSCODE_STREAM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
                                            DMAP_TYPE_TRANSCODE_STREAM, \
                                            DmapTranscodeStreamClass))
typedef struct DmapTranscodeStreamPrivate DmapTranscodeStreamPrivate;

typedef struct {
	GInputStream parent;
	DmapTranscodeStreamPrivate *priv;
} DmapTranscodeStream;

typedef struct {
	GInputStreamClass parent;

	void (*kill_pipeline) (DmapTranscodeStream *stream);
} DmapTranscodeStreamClass;

GType dmap_transcode_stream_get_type (void);

/* Supported transcode target formats (data read from DmapTranscodeStream
 * will be in one of these formats): */
enum
{
	RAW,			/* No transcoding performed. */
	WAV16,
	MP3
};

GInputStream *dmap_transcode_stream_new (const gchar * transcode_mimetype,
					 GInputStream * src_stream);

G_END_DECLS
#endif /* _DMAP_TRANSCODE_STREAM_H */
