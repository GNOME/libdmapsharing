/* Implmentation of DAAP (e.g., iTunes Music) sharing
 *
 * Copyright (C) 2005 Charles Schmidt <cschmidt2@emich.edu>
 *
 * Modifications Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
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
 *
 */

#include "config.h"

#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <libsoup/soup.h>

#include <libdmapsharing/dmap.h>
#include <libdmapsharing/dmap-share-private.h>
#include <libdmapsharing/dmap-structure.h>
#include <libdmapsharing/dmap-private-utils.h>
#include <libdmapsharing/dmap-utils.h>

#ifdef HAVE_GSTREAMERAPP
#include <libdmapsharing/dmap-transcode-stream.h>
#endif /* HAVE_GSTREAMERAPP */

static guint _get_desired_port (DmapShare * share);
static const char *_get_type_of_service (DmapShare * share);
static void _server_info (DmapShare * share,
                          SoupServerMessage * message,
                          const char *path);
static void _message_add_standard_headers (DmapShare * share,
                                           SoupServerMessage * message);
static void _databases_browse_xxx (DmapShare * share,
                                   SoupServerMessage * msg,
                                   const char *path,
                                   GHashTable * query);
static void _databases_items_xxx (DmapShare * share,
                                  SoupServer * server,
                                  SoupServerMessage * msg,
                                  const char *path);
static struct DmapMetaDataMap *_get_meta_data_map (DmapShare * share);
static void _add_entry_to_mlcl (guint id, DmapRecord * record, gpointer mb);

#define DAAP_TYPE_OF_SERVICE "_daap._tcp"
#define DAAP_PORT 3689

G_DEFINE_TYPE (DmapAvShare, dmap_av_share, DMAP_TYPE_SHARE);

static void
dmap_av_share_class_init (DmapAvShareClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	DmapShareClass *parent_class = DMAP_SHARE_CLASS (object_class);

	parent_class->get_desired_port = _get_desired_port;
	parent_class->get_type_of_service = _get_type_of_service;
	parent_class->message_add_standard_headers = _message_add_standard_headers;
	parent_class->get_meta_data_map = _get_meta_data_map;
	parent_class->add_entry_to_mlcl = _add_entry_to_mlcl;
	parent_class->databases_browse_xxx = _databases_browse_xxx;
	parent_class->databases_items_xxx = _databases_items_xxx;
	parent_class->server_info = _server_info;
}

static void
dmap_av_share_init (G_GNUC_UNUSED DmapAvShare * share)
{
}

DmapAvShare *
dmap_av_share_new (const char *name,
                   const char *password,
                   DmapDb * db,
                   DmapContainerDb * container_db,
                   gchar * transcode_mimetype)
{
	return DMAP_AV_SHARE (g_object_new (DMAP_TYPE_AV_SHARE,
	                                   "name", name,
	                                   "password", password,
	                                   "db", db,
	                                   "container-db", container_db,
	                                   "transcode-mimetype",
	                                    transcode_mimetype, NULL));
}

static void
_message_add_standard_headers (G_GNUC_UNUSED DmapShare * share,
                               SoupServerMessage * message)
{
	soup_message_headers_append (soup_server_message_get_response_headers(message),
	                             "DMAP-Server",
				     "libdmapsharing" VERSION);
}

#define DMAP_VERSION 2.0
#define DAAP_VERSION 3.0
#define DAAP_TIMEOUT 1800

static guint
_get_desired_port (G_GNUC_UNUSED DmapShare * share)
{
	return DAAP_PORT;
}

static const char *
_get_type_of_service (G_GNUC_UNUSED DmapShare * share)
{
	return DAAP_TYPE_OF_SERVICE;
}

static void
_server_info (DmapShare * share,
              SoupServerMessage * message,
              const char *path)
{
/* MSRV	server info response
 * 	MSTT status
 * 	MPRO daap version
 * 	APRO daap version
 * 	MINM name
 * 	MSAU authentication method
 * 	MSLR login required
 * 	MSTM timeout interval
 * 	MSAL supports auto logout
 * 	MSUP supports update
 * 	MSPI supports persistent ids
 * 	MSEX supports extensions
 * 	MSBR supports browse
 * 	MSQY supports query
 * 	MSIX supports index
 * 	MSRS supports resolve
 * 	MSDC databases count
 */
	gchar *nameprop;
	GNode *msrv;

	g_debug ("Path is %s.", path);

	g_object_get ((gpointer) share, "name", &nameprop, NULL);

	msrv = dmap_structure_add (NULL, DMAP_CC_MSRV);
	dmap_structure_add (msrv, DMAP_CC_MSTT, (gint32) SOUP_STATUS_OK);
	dmap_structure_add (msrv, DMAP_CC_MPRO, (gdouble) DAAP_VERSION);
	dmap_structure_add (msrv, DMAP_CC_APRO, (gdouble) DAAP_VERSION);
	/* 2/3 is for itunes 4.8 (at least).  its determined by the
	 * Client-DAAP-Version header sent, but if we decide not to support
	 * older versions..? anyway
	 *
	 * 1.0 is 1/1
	 * 2.0 is 1/2
	 * 3.0 is 2/3
	 */
	dmap_structure_add (msrv, DMAP_CC_MINM, nameprop);
	dmap_structure_add (msrv, DMAP_CC_MSAU,
			    dmap_share_get_auth_method (share));
	dmap_structure_add (msrv, DMAP_CC_MSLR, DMAP_SHARE_AUTH_METHOD_NONE);
	dmap_structure_add (msrv, DMAP_CC_MSTM, (gint32) DAAP_TIMEOUT);
	dmap_structure_add (msrv, DMAP_CC_MSAL, (gchar) 0);
	dmap_structure_add (msrv, DMAP_CC_MSUP, (gchar) 1);
	dmap_structure_add (msrv, DMAP_CC_MSPI, (gchar) 0);
	dmap_structure_add (msrv, DMAP_CC_MSEX, (gchar) 0);
	dmap_structure_add (msrv, DMAP_CC_MSBR, (gchar) 0);
	dmap_structure_add (msrv, DMAP_CC_MSQY, (gchar) 0);
	dmap_structure_add (msrv, DMAP_CC_MSIX, (gchar) 0);
	dmap_structure_add (msrv, DMAP_CC_MSRS, (gchar) 0);
	dmap_structure_add (msrv, DMAP_CC_MSDC, (gint32) 1);

	dmap_share_message_set_from_dmap_structure (share, message, msrv);
	dmap_structure_destroy (msrv);

	g_free (nameprop);
}

typedef enum {
	ITEM_ID = 0,
	ITEM_NAME,
	ITEM_KIND,
	PERSISTENT_ID,
	CONTAINER_ITEM_ID,
	SONG_ALBUM,
	SONG_GROUPING,
	SONG_ARTIST,
	SONG_BITRATE,
	SONG_BPM,
	SONG_COMMENT,
	SONG_COMPILATION,
	SONG_COMPOSER,
	SONG_DATA_KIND,
	SONG_DATA_URL,
	SONG_DATE_ADDED,
	SONG_DATE_MODIFIED,
	SONG_DISC_COUNT,
	SONG_DISC_NUMBER,
	SONG_DISABLED,
	SONG_EQ_PRESET,
	SONG_FORMAT,
	SONG_GENRE,
	SONG_DESCRIPTION,
	SONG_RELATIVE_VOLUME,
	SONG_SAMPLE_RATE,
	SONG_SIZE,
	SONG_SORT_ALBUM,
	SONG_SORT_ARTIST,
	SONG_START_TIME,
	SONG_STOP_TIME,
	SONG_TIME,
	SONG_TRACK_COUNT,
	SONG_TRACK_NUMBER,
	SONG_USER_RATING,
	SONG_YEAR,
	SONG_HAS_VIDEO,
	SONG_SMART_PLAYLIST,
	SONG_IS_PODCAST_PLAYLIST,
	SONG_SPECIAL_PLAYLIST,
	SONG_SAVED_GENIUS,
	SONG_MEDIAKIND,
	HAS_CHILD_CONTAINERS,
	PARENT_CONTAINER_ID
} DAAPMetaData;

static struct DmapMetaDataMap _meta_data_map[] = {
	{"dmap.itemid", ITEM_ID},
	{"dmap.itemname", ITEM_NAME},
	{"dmap.itemkind", ITEM_KIND},
	{"dmap.persistentid", PERSISTENT_ID},
	{"dmap.containeritemid", CONTAINER_ITEM_ID},
	{"daap.songalbum", SONG_ALBUM},
	{"daap.songartist", SONG_ARTIST},
	{"daap.songbitrate", SONG_BITRATE},
	{"daap.songbeatsperminute", SONG_BPM},
	{"daap.songcomment", SONG_COMMENT},
	{"daap.songcompilation", SONG_COMPILATION},
	{"daap.songcomposer", SONG_COMPOSER},
	{"daap.songdatakind", SONG_DATA_KIND},
	{"daap.songdataurl", SONG_DATA_URL},
	{"daap.songdateadded", SONG_DATE_ADDED},
	{"daap.songdatemodified", SONG_DATE_MODIFIED},
	{"daap.songdescription", SONG_DESCRIPTION},
	{"daap.songdisabled", SONG_DISABLED},
	{"daap.songdisccount", SONG_DISC_COUNT},
	{"daap.songdiscnumber", SONG_DISC_NUMBER},
	{"daap.songeqpreset", SONG_EQ_PRESET},
	{"daap.songformat", SONG_FORMAT},
	{"daap.songgenre", SONG_GENRE},
	{"daap.songgrouping", SONG_GROUPING},
	{"daap.songrelativevolume", SONG_RELATIVE_VOLUME},
	{"daap.songsamplerate", SONG_SAMPLE_RATE},
	{"daap.songsize", SONG_SIZE},
	{"daap.songstarttime", SONG_START_TIME},
	{"daap.songstoptime", SONG_STOP_TIME},
	{"daap.songtime", SONG_TIME},
	{"daap.songtrackcount", SONG_TRACK_COUNT},
	{"daap.songtracknumber", SONG_TRACK_NUMBER},
	{"daap.songuserrating", SONG_USER_RATING},
	{"daap.songyear", SONG_YEAR},
	{"daap.sortalbum", SONG_SORT_ALBUM},
	{"daap.sortartist", SONG_SORT_ARTIST},
	{"com.apple.itunes.has-video", SONG_HAS_VIDEO},
	{"com.apple.itunes.smart-playlist", SONG_SMART_PLAYLIST},
	{"com.apple.itunes.is-podcast-playlist", SONG_IS_PODCAST_PLAYLIST},
	{"com.apple.itunes.special-playlist", SONG_SPECIAL_PLAYLIST},
	{"com.apple.itunes.saved-genius", SONG_SAVED_GENIUS},
	{"com.apple.itunes.mediakind", SONG_MEDIAKIND},
	{"dmap.haschildcontainers", HAS_CHILD_CONTAINERS},
	{"dmap.parentcontainerid", PARENT_CONTAINER_ID},
	{NULL, 0}
};

#define DAAP_ITEM_KIND_AUDIO 2
#define DAAP_SONG_DATA_KIND_NONE 0

static gboolean
_should_transcode (DmapAvShare *share,
                   const gchar *format,
                   const gboolean has_video,
                   const gchar *transcode_mimetype)
{
	gboolean fnval = FALSE;
	char *format2 = NULL;

	// Not presently transcoding videos (see also same comments elsewhere).
	if (TRUE == has_video) {
		goto done;
	}

	if (NULL == transcode_mimetype) {
		goto done;
	}

	format2 = dmap_utils_mime_to_format (transcode_mimetype);
	if (NULL == format2) {
		dmap_share_emit_error(DMAP_SHARE(share), DMAP_STATUS_BAD_FORMAT,
		                     "Configured to transcode, but target format bad");
		goto done;
	}

	if (strcmp (format, format2)) {
		fnval = TRUE;
	}

done:
	g_debug ("    Should%s transcode %s to %s", fnval ? "" : " not", format, format2 ? format2 : "[no target format]");

	g_free(format2);

	return fnval;
}

static void
_send_chunked_file (DmapAvShare *share, SoupServer * server, SoupServerMessage * message,
		   DmapAvRecord * record, guint64 filesize, guint64 offset,
		   const gchar * transcode_mimetype)
{
	gchar *format = NULL;
	gchar *location = NULL;
	GInputStream *stream = NULL;
	gboolean has_video;
	GError *error = NULL;
	ChunkData *cd = NULL;
	gboolean teardown = TRUE;

	cd = g_new0 (ChunkData, 1);

	g_object_get (record, "location", &location, "has-video", &has_video, NULL);
	if (NULL == location) {
		dmap_share_emit_error(DMAP_SHARE(share), DMAP_STATUS_RECORD_MISSING_FIELD,
		                     "Error getting location from record");
		goto done;
	}

	/* FIXME: This crashes on powerpc-440fp-linux-gnu:
	 * g_debug ("Sending %s chunked from offset %" G_GUINT64_FORMAT ".", location, offset);
	 */

	cd->server = server;

	stream = G_INPUT_STREAM (dmap_av_record_read (record, &error));
	if (error != NULL) {
		dmap_share_emit_error(DMAP_SHARE(share), DMAP_STATUS_OPEN_FAILED,
		                     "Cannot open %s", error->message);
		goto done;
	}

	g_object_get (record, "format", &format, NULL);
	if (NULL == format) {
		dmap_share_emit_error(DMAP_SHARE(share), DMAP_STATUS_RECORD_MISSING_FIELD,
		                     "Error getting format from record");
		goto done;
	}

	// Not presently transcoding videos (see also same comments elsewhere).
	if (_should_transcode (share, format, has_video, transcode_mimetype)) {
#ifdef HAVE_GSTREAMERAPP
		cd->original_stream = stream;
		cd->stream = dmap_transcode_stream_new (transcode_mimetype, stream);
#else
		dmap_share_emit_error(DMAP_SHARE(share), DMAP_STATUS_BAD_FORMAT,
		                     "Transcode format %s not supported",
		                      transcode_mimetype);
		cd->original_stream = NULL;
		cd->stream = stream;
#endif /* HAVE_GSTREAMERAPP */
	} else {
		g_debug ("Not transcoding %s", location);
		cd->original_stream = NULL;
		cd->stream = stream;
	}

	if (cd->stream == NULL) {
		dmap_share_emit_error(DMAP_SHARE(share), DMAP_STATUS_OPEN_FAILED,
		                     "Could not setup input stream");
		goto done;
	}

	if (offset != 0) {
		if (g_seekable_seek (G_SEEKABLE (cd->stream), offset, G_SEEK_SET, NULL, &error) == FALSE) {
			dmap_share_emit_error(DMAP_SHARE(share), DMAP_STATUS_SEEK_FAILED,
			                     "Error seeking: %s.", error->message);
			goto done;
		}
		filesize -= offset;
	}

	/* Free memory after each chunk sent out over network. */
	soup_message_body_set_accumulate (soup_server_message_get_response_body(message), FALSE);

	if (! _should_transcode (share, format, has_video, transcode_mimetype)) {
	        /* NOTE: iTunes seems to require this or it stops reading
	         * video data after about 2.5MB. Perhaps this is so iTunes
	         * knows how much data to buffer.
	         */
		g_debug ("Using HTTP 1.1 content length encoding.");
		soup_message_headers_set_encoding (soup_server_message_get_response_headers(message),
		                                   SOUP_ENCODING_CONTENT_LENGTH);

	        /* NOTE: iTunes 8 (and other versions?) will not seek
	         * properly without a Content-Length header.
	         */
		g_debug ("Content length is %" G_GUINT64_FORMAT ".", filesize);
		soup_message_headers_set_content_length (soup_server_message_get_response_headers(message), filesize);
	} else if (soup_server_message_get_http_version (message) == SOUP_HTTP_1_0) {
		/* NOTE: Roku clients support only HTTP 1.0. */
		g_debug ("Using HTTP 1.0 encoding.");
		soup_message_headers_set_encoding (soup_server_message_get_response_headers(message), SOUP_ENCODING_EOF);
	} else {
		/* NOTE: Can not provide Content-Length when performing
		 * real-time transcoding.
		 */
		g_debug ("Using HTTP 1.1 chunked encoding.");
		soup_message_headers_set_encoding (soup_server_message_get_response_headers(message), SOUP_ENCODING_CHUNKED);
	}

	soup_message_headers_append (soup_server_message_get_response_headers(message), "Connection",
				     "Close");
	soup_message_headers_append (soup_server_message_get_response_headers(message),
				     "Content-Type",
				     "application/x-dmap-tagged");

	if (0 == g_signal_connect (message, "wrote_headers",
			           G_CALLBACK (dmap_private_utils_write_next_chunk), cd)) {
		dmap_share_emit_error(DMAP_SHARE(share), DMAP_STATUS_FAILED,
		                     "Error connecting to wrote_headers signal");
		goto done;
	}

	if (0 == g_signal_connect (message, "wrote_chunk",
			  G_CALLBACK (dmap_private_utils_write_next_chunk), cd)) {
		dmap_share_emit_error(DMAP_SHARE(share), DMAP_STATUS_FAILED,
		                     "Error connecting to wrote_chunk signal");
		goto done;
	}

	if (0 == g_signal_connect (message, "finished",
			  G_CALLBACK (dmap_private_utils_chunked_message_finished), cd)) {
		dmap_share_emit_error(DMAP_SHARE(share), DMAP_STATUS_FAILED,
		                     "Error connecting to finished signal");
		goto done;
	}
	/* NOTE: cd g_free'd by chunked_message_finished(). */

	teardown = FALSE;

done:
	if (teardown) {
		gboolean ok;

		soup_server_message_set_status (message, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL);

		if (NULL != cd && NULL != cd->stream) {
			ok = g_input_stream_close (cd->stream, NULL, &error);
			if (!ok) {
				dmap_share_emit_error(DMAP_SHARE(share), DMAP_STATUS_CLOSE_FAILED,
				                     "Error closing transcode stream: %s.",
				                      error->message);
			}
		}

		g_clear_error(&error);

		if (NULL != stream) {
			ok = g_input_stream_close (stream, NULL, &error);
			if (!ok) {
				dmap_share_emit_error(DMAP_SHARE(share), DMAP_STATUS_CLOSE_FAILED,
				                     "Error closing stream: %s.",
				                      error->message);
			}
		}

		g_free (cd);
	}


	g_free (location);
	g_free (format);

	if (NULL != error) {
		g_error_free(error);
	}

	return;
}

static void
_add_entry_to_mlcl (guint id, DmapRecord * record, gpointer _mb)
{
	GNode *mlit;
	gboolean has_video = 0;
	struct DmapMlclBits *mb = (struct DmapMlclBits *) _mb;

	mlit = dmap_structure_add (mb->mlcl, DMAP_CC_MLIT);
	g_object_get (record, "has-video", &has_video, NULL);

	if (dmap_share_client_requested (mb->bits, ITEM_KIND)) {
		dmap_structure_add (mlit, DMAP_CC_MIKD,
				    (gchar) DAAP_ITEM_KIND_AUDIO);
	}

	if (dmap_share_client_requested (mb->bits, ITEM_ID)) {
		dmap_structure_add (mlit, DMAP_CC_MIID, id);
	}

	if (dmap_share_client_requested (mb->bits, ITEM_NAME)) {
		gchar *title = NULL;

		g_object_get (record, "title", &title, NULL);
		if (title) {
			dmap_structure_add (mlit, DMAP_CC_MINM, title);
			g_free (title);
		} else {
			g_debug ("Title requested but not available");
		}
	}

	if (dmap_share_client_requested (mb->bits, PERSISTENT_ID)) {
		dmap_structure_add (mlit, DMAP_CC_MPER, id);
	}

	if (dmap_share_client_requested (mb->bits, CONTAINER_ITEM_ID)) {
		dmap_structure_add (mlit, DMAP_CC_MCTI, id);
	}

	if (dmap_share_client_requested (mb->bits, SONG_DATA_KIND)) {
		dmap_structure_add (mlit, DMAP_CC_ASDK,
				    (gchar) DAAP_SONG_DATA_KIND_NONE);
	}

	/* FIXME: Any use for this?
	 * if (dmap_share_client_requested (mb->bits, SONG_DATA_URL))
	 * dmap_structure_add (mlit, DMAP_CC_ASUL, "daap://192.168.0.100:%u/databases/1/items/%d.%s?session-id=%s", data->port, *id, dmap_av_record_get_format (DMAP_AV_RECORD (record)), data->session_id);
	 */
	if (dmap_share_client_requested (mb->bits, SONG_ALBUM)) {
		gchar *album = NULL;

		g_object_get (record, "songalbum", &album, NULL);
		if (album) {
			dmap_structure_add (mlit, DMAP_CC_ASAL, album);
			g_free (album);
		} else {
			g_debug ("Album requested but not available");
		}
	}

	if (dmap_share_client_requested (mb->bits, SONG_GROUPING)) {
		dmap_structure_add (mlit, DMAP_CC_AGRP, "");
	}

	if (dmap_share_client_requested (mb->bits, SONG_ARTIST)) {
		gchar *artist = NULL;

		g_object_get (record, "songartist", &artist, NULL);
		if (artist) {
			dmap_structure_add (mlit, DMAP_CC_ASAR, artist);
			g_free (artist);
		} else {
			g_debug ("Artist requested but not available");
		}
	}

	if (dmap_share_client_requested (mb->bits, SONG_BITRATE)) {
		gint32 bitrate = 0;

		g_object_get (record, "bitrate", &bitrate, NULL);
		if (bitrate != 0) {
			dmap_structure_add (mlit, DMAP_CC_ASBR,
					    (gint32) bitrate);
		}
	}

	if (dmap_share_client_requested (mb->bits, SONG_BPM)) {
		dmap_structure_add (mlit, DMAP_CC_ASBT, (gint32) 0);
	}

	if (dmap_share_client_requested (mb->bits, SONG_COMMENT)) {
		dmap_structure_add (mlit, DMAP_CC_ASCM, "");
	}

	if (dmap_share_client_requested (mb->bits, SONG_COMPILATION)) {
		dmap_structure_add (mlit, DMAP_CC_ASCO, (gchar) FALSE);
	}

	if (dmap_share_client_requested (mb->bits, SONG_COMPOSER)) {
		dmap_structure_add (mlit, DMAP_CC_ASCP, "");
	}

	if (dmap_share_client_requested (mb->bits, SONG_DATE_ADDED)) {
		gint32 firstseen = 0;

		g_object_get (record, "firstseen", &firstseen, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASDA, firstseen);
	}

	if (dmap_share_client_requested (mb->bits, SONG_DATE_MODIFIED)) {
		gint32 mtime = 0;

		g_object_get (record, "mtime", &mtime, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASDM, mtime);
	}

	if (dmap_share_client_requested (mb->bits, SONG_DISC_COUNT)) {
		dmap_structure_add (mlit, DMAP_CC_ASDC, (gint32) 0);
	}

	if (dmap_share_client_requested (mb->bits, SONG_DISC_NUMBER)) {
		gint32 disc = 0;

		g_object_get (record, "disc", &disc, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASDN, disc);
	}

	if (dmap_share_client_requested (mb->bits, SONG_DISABLED)) {
		dmap_structure_add (mlit, DMAP_CC_ASDB, (gchar) FALSE);
	}

	if (dmap_share_client_requested (mb->bits, SONG_EQ_PRESET)) {
		dmap_structure_add (mlit, DMAP_CC_ASEQ, "");
	}

	if (dmap_share_client_requested (mb->bits, SONG_FORMAT)) {
		gchar *format = NULL;
		gchar *transcode_mimetype = NULL;

		g_object_get (mb->share, "transcode-mimetype",
			      &transcode_mimetype, NULL);
		// Not presently transcoding videos (see also same comments elsewhere).
		if (! has_video && transcode_mimetype) {
			format = dmap_utils_mime_to_format (transcode_mimetype);
			g_free (transcode_mimetype);
		} else {
			g_object_get (record, "format", &format, NULL);
		}
		if (format) {
			dmap_structure_add (mlit, DMAP_CC_ASFM, format);
			g_free (format);
		} else {
			g_debug ("Format requested but not available");
		}
	}

	if (dmap_share_client_requested (mb->bits, SONG_GENRE)) {
		gchar *genre = NULL;

		g_object_get (record, "songgenre", &genre, NULL);
		if (genre) {
			dmap_structure_add (mlit, DMAP_CC_ASGN, genre);
			g_free (genre);
		} else {
			g_debug ("Genre requested but not available");
		}
	}

	if (dmap_share_client_requested (mb->bits, SONG_DESCRIPTION)) {
		dmap_structure_add (mlit, DMAP_CC_ASDT, "");	/* FIXME: e.g., wav audio file */
	}

	if (dmap_share_client_requested (mb->bits, SONG_RELATIVE_VOLUME)) {
		dmap_structure_add (mlit, DMAP_CC_ASRV, 0);
	}

	if (dmap_share_client_requested (mb->bits, SONG_SAMPLE_RATE)) {
		dmap_structure_add (mlit, DMAP_CC_ASSR, 0);
	}

	if (dmap_share_client_requested (mb->bits, SONG_SIZE)) {
		guint64 filesize = 0;

		g_object_get (record, "filesize", &filesize, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASSZ, (gint32) filesize);
	}

	if (dmap_share_client_requested (mb->bits, SONG_START_TIME)) {
		dmap_structure_add (mlit, DMAP_CC_ASST, 0);
	}

	if (dmap_share_client_requested (mb->bits, SONG_STOP_TIME)) {
		dmap_structure_add (mlit, DMAP_CC_ASSP, 0);
	}

	if (dmap_share_client_requested (mb->bits, SONG_TIME)) {
		gint32 duration;

		g_object_get (record, "duration", &duration, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASTM, (1000 * duration));
	}

	if (dmap_share_client_requested (mb->bits, SONG_TRACK_COUNT)) {
		dmap_structure_add (mlit, DMAP_CC_ASTC, 0);
	}

	if (dmap_share_client_requested (mb->bits, SONG_TRACK_NUMBER)) {
		gint32 track = 0;

		g_object_get (record, "track", &track, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASTN, track);
	}

	if (dmap_share_client_requested (mb->bits, SONG_USER_RATING)) {
		gint32 rating = 0;

		g_object_get (record, "rating", &rating, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASUR, rating);
	}

	if (dmap_share_client_requested (mb->bits, SONG_YEAR)) {
		gint32 year = 0;

		g_object_get (record, "year", &year, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASYR, year);
	}

	if (dmap_share_client_requested (mb->bits, SONG_HAS_VIDEO)) {
		dmap_structure_add (mlit, DMAP_CC_AEHV, has_video);
	}

	if (dmap_share_client_requested (mb->bits, SONG_SORT_ARTIST)) {
		gchar *sort_artist = NULL;

		g_object_get (record, "sort-artist", &sort_artist, NULL);
		if (sort_artist) {
			dmap_structure_add (mlit, DMAP_CC_ASSA, sort_artist);
			g_free (sort_artist);
		} else {
			g_debug ("Sort artist requested but not available");
		}
	}

	if (dmap_share_client_requested (mb->bits, SONG_SORT_ALBUM)) {
		gchar *sort_album = NULL;

		g_object_get (record, "sort-album", &sort_album, NULL);
		if (sort_album) {
			dmap_structure_add (mlit, DMAP_CC_ASSU, sort_album);
			g_free (sort_album);
		} else {
			g_debug ("Sort album requested but not available");
		}
	}

	if (dmap_share_client_requested (mb->bits, SONG_MEDIAKIND)) {
		gint mediakind = 0;

		g_object_get (record, "mediakind", &mediakind, NULL);
		dmap_structure_add (mlit, DMAP_CC_AEMK, mediakind);
	}
}

static void
_genre_tabulator (G_GNUC_UNUSED gpointer id, DmapRecord *record, GHashTable *ht)
{
	const gchar *genre;

	g_object_get (record, "songgenre", &genre, NULL);
	if (!genre) {
		return;
	}

	if (!g_hash_table_lookup (ht, genre)) {
		gboolean ok;

		ok = g_hash_table_insert (ht, (gchar *) genre, NULL);
		if (!ok) {
			g_warning("error inserting %s", genre);
		}
	}
}

static void
_artist_tabulator (G_GNUC_UNUSED gpointer id, DmapRecord * record, GHashTable * ht)
{
	const gchar *artist;

	g_object_get (record, "songartist", &artist, NULL);
	if (!artist) {
		return;
	}

	if (!g_hash_table_lookup (ht, artist)) {
		gboolean ok;

		ok = g_hash_table_insert (ht, (gchar *) artist, NULL);
		if (!ok) {
			g_warning("error inserting %s", artist);
		}
	}
}

static void
_album_tabulator (G_GNUC_UNUSED gpointer id, DmapRecord * record, GHashTable * ht)
{
	const gchar *album;

	g_object_get (record, "songalbum", &album, NULL);
	if (!album) {
		return;
	}

	if (!g_hash_table_lookup (ht, album)) {
		gboolean ok;

		ok = g_hash_table_insert (ht, (gchar *) album, NULL);
		if (!ok) {
			g_warning("error inserting %s", album);
		}
	}
}

static void
_add_to_category_listing (gpointer key, gpointer user_data)
{
	GNode *mlit;
	GNode *node = (GNode *) user_data;

	mlit = dmap_structure_add (node, DMAP_CC_MLIT);
	dmap_structure_add (mlit, DMAP_RAW, (char *) key);
}

static void
_databases_browse_xxx (DmapShare * share,
                       SoupServerMessage * msg,
                       const char *path,
                       GHashTable * query)
{
	/* ABRO database browse
	 *      MSTT status
	 *      MUTY update type
	 *      MTCO specified total count
	 *      MRCO returned count
	 *      ABGN genre listing
	 *              MLIT listing item
	 *              ...
	 */
	DmapDb *db;
	const gchar *rest_of_path;
	GNode *abro, *node;
	gchar *filter;
	GSList *filter_def;
	GHashTable *filtered;
	guint num_genre;
	const gchar *browse_category;
	GHashTable *category_items;
	DmapContentCode category_cc;
	GList *values;

	rest_of_path = strchr (path + 1, '/');
	browse_category = rest_of_path + 10;
	category_items = g_hash_table_new (g_str_hash, g_str_equal);

	filter = g_hash_table_lookup (query, "filter");
	filter_def = dmap_share_build_filter (filter);
	g_object_get (share, "db", &db, NULL);
	filtered = dmap_db_apply_filter (db, filter_def);

	if (g_ascii_strcasecmp (browse_category, "genres") == 0) {
		g_hash_table_foreach (filtered, (GHFunc) _genre_tabulator,
				      category_items);
		category_cc = DMAP_CC_ABGN;
	} else if (g_ascii_strcasecmp (browse_category, "artists") == 0) {
		g_hash_table_foreach (filtered, (GHFunc) _artist_tabulator,
				      category_items);
		category_cc = DMAP_CC_ABAR;
	} else if (g_ascii_strcasecmp (browse_category, "albums") == 0) {
		g_hash_table_foreach (filtered, (GHFunc) _album_tabulator,
				      category_items);
		category_cc = DMAP_CC_ABAL;
	} else {
		dmap_share_emit_error(share, DMAP_STATUS_BAD_BROWSE_CATEGORY,
		                     "Unsupported browse category: %s",
		                      browse_category);
		goto _bad_category;
	}

	abro = dmap_structure_add (NULL, DMAP_CC_ABRO);
	dmap_structure_add (abro, DMAP_CC_MSTT, (gint32) SOUP_STATUS_OK);
	dmap_structure_add (abro, DMAP_CC_MUTY, 0);

	num_genre = g_hash_table_size (category_items);
	dmap_structure_add (abro, DMAP_CC_MTCO, (gint32) num_genre);
	dmap_structure_add (abro, DMAP_CC_MRCO, (gint32) num_genre);

	node = dmap_structure_add (abro, category_cc);

	values = g_hash_table_get_keys (category_items);
	if (values && g_hash_table_lookup (query, "include-sort-headers")) {
		g_debug ("Sorting...");
		values = g_list_sort (values,
				      (GCompareFunc) g_ascii_strcasecmp);
	}

	g_list_foreach (values, _add_to_category_listing, node);

	g_list_free (values);

	dmap_share_message_set_from_dmap_structure (share, msg, abro);
	dmap_structure_destroy (abro);
      _bad_category:
	dmap_share_free_filter (filter_def);
	/* Free's hash table but not data (points into real DB): */
	g_hash_table_destroy (filtered);
	g_hash_table_destroy (category_items);
}

static void
_databases_items_xxx (DmapShare * share,
                      SoupServer * server,
                      SoupServerMessage * msg,
                      const char *path)
{
	DmapDb *db = NULL;
	DmapAvRecord *record = NULL;
	gchar *transcode_mimetype = NULL;
	const gchar *rest_of_path;
	const gchar *id_str;
	guint id;
	const gchar *range_header;
	guint64 filesize = 0;
	guint64 offset = 0;

	rest_of_path = strchr (path + 1, '/');
	id_str = rest_of_path + 9;
	id = strtoul (id_str, NULL, 10);

	g_object_get (share, "db", &db, NULL);

	record = DMAP_AV_RECORD (dmap_db_lookup_by_id (db, id));
	if (NULL == record) {
		g_signal_emit_by_name(share, "error",
			g_error_new(DMAP_ERROR,
			            DMAP_STATUS_DB_BAD_ID,
			           "Bad record identifier requested"));
		soup_server_message_set_status (msg, SOUP_STATUS_NOT_FOUND, NULL);
		goto done;
	}

	g_object_get (record, "filesize", &filesize, NULL);

	DMAP_SHARE_GET_CLASS (share)->message_add_standard_headers
		(share, msg);
	soup_message_headers_append (soup_server_message_get_response_headers(msg), "Accept-Ranges",
				     "bytes");

	range_header =
		soup_message_headers_get_one (soup_server_message_get_request_headers(msg), "Range");
	if (range_header) {
		const gchar *s;
		gchar *content_range;

		if (!g_str_has_prefix (range_header, "bytes=")) {
			/* Not starting with "bytes=" ? */
			offset = 0;
		} else {
			s = range_header + strlen ("bytes=");	/* bytes= */
			offset = atoll (s);
		}

		content_range =
			g_strdup_printf ("bytes %" G_GUINT64_FORMAT "-%"
					 G_GUINT64_FORMAT "/%"
					 G_GUINT64_FORMAT, offset, filesize,
					 filesize);
		soup_message_headers_append (soup_server_message_get_response_headers(msg),
					     "Content-Range", content_range);
		g_debug ("Content range is %s.", content_range);
		g_free (content_range);
		soup_server_message_set_status (msg, SOUP_STATUS_PARTIAL_CONTENT, NULL);
	} else {
		soup_server_message_set_status (msg, SOUP_STATUS_OK, NULL);
	}
	g_object_get (share, "transcode-mimetype", &transcode_mimetype, NULL);
	_send_chunked_file (DMAP_AV_SHARE(share), server, msg, record, filesize,
	                    offset, transcode_mimetype);

done:
	if (NULL != record) {
		g_object_unref (record);
	}

	if (NULL != db) {
		g_object_unref (db);
	}

	g_free(transcode_mimetype);
}

static struct DmapMetaDataMap *
_get_meta_data_map (G_GNUC_UNUSED DmapShare * share)
{
	return _meta_data_map;
}

#ifdef HAVE_CHECK

#include <check.h>
#include <libdmapsharing/test-dmap-db.h>
#include <libdmapsharing/test-dmap-av-record.h>
#include <libdmapsharing/test-dmap-container-db.h>
#include <libdmapsharing/test-dmap-container-record.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static DmapShare *
_build_share_test(char *name)
{
	DmapDb *db;
	DmapContainerRecord *container_record;
	DmapContainerDb *container_db;
	DmapRecord *record;
	DmapShare *share;
	struct stat statbuf;

	db = DMAP_DB(test_dmap_db_new());
	container_record = DMAP_CONTAINER_RECORD (test_dmap_container_record_new ());
	container_db = DMAP_CONTAINER_DB(test_dmap_container_db_new(container_record));

	if (-1 == stat("/etc/services", &statbuf)) {
		ck_abort();
	}

	record = DMAP_RECORD(test_dmap_av_record_new());
	g_object_set(record, "songgenre", "genre1", NULL);
	g_object_set(record, "songartist", "artist1", NULL);
	g_object_set(record, "songalbum", "album1", NULL);
	g_object_set(record, "location", "file:///etc/services", NULL);
	g_object_set(record, "filesize", statbuf.st_size, NULL);

	dmap_db_add(db, record, NULL);

	if (-1 == stat("/etc/group", &statbuf)) {
		ck_abort();
	}

	record = DMAP_RECORD(test_dmap_av_record_new());
	g_object_set(record, "songgenre", "genre2", NULL);
	g_object_set(record, "songartist", "artist2", NULL);
	g_object_set(record, "songalbum", "album2", NULL);
	g_object_set(record, "location", "file:///etc/group", NULL);
	g_object_set(record, "filesize", statbuf.st_size, NULL);

	dmap_db_add(db, record, NULL);

	share  = DMAP_SHARE(dmap_av_share_new(name,
	                                   NULL,
	                                   db,
	                                   container_db,
	                                   NULL));

	g_object_unref(db);
	g_object_unref(container_record);
	g_object_unref(container_db);

	return share;
}

START_TEST(_get_meta_data_map_test)
{
	ck_assert_ptr_eq(_meta_data_map, _get_meta_data_map(NULL));
}
END_TEST

START_TEST(_new_test)
{
	DmapDb *db;
	DmapContainerRecord *container_record;
	DmapContainerDb *container_db;
	DmapRecord *record;
	DmapShare *share;
	char *str;

	db = DMAP_DB(test_dmap_db_new());
	container_record = DMAP_CONTAINER_RECORD (test_dmap_container_record_new ());
	container_db = DMAP_CONTAINER_DB(test_dmap_container_db_new(container_record));


	record = DMAP_RECORD(test_dmap_av_record_new());
	g_object_set(record, "songgenre", "genre1", NULL);
	g_object_set(record, "songartist", "artist1", NULL);
	g_object_set(record, "songalbum", "album1", NULL);

	dmap_db_add(db, record, NULL);

	share = DMAP_SHARE(dmap_av_share_new("name",
	                                     "password",
	                                      db,
	                                      container_db,
	                                     "audio/mp3"));

	g_object_get(share, "name", &str, NULL);
	ck_assert_str_eq("name", str);
	g_free(str);

	g_object_get(share, "password", &str, NULL);
	ck_assert_str_eq("password", str);
	g_free(str);

	g_object_get(share, "transcode-mimetype", &str, NULL);
	ck_assert_str_eq("audio/mp3", str);
	g_free(str);

	g_object_unref(db);
	g_object_unref(container_record);
	g_object_unref(container_db);
	g_object_unref(share);
}
END_TEST

START_TEST(_serve_publish_test)
{
	DmapDb *db;
	gboolean ok;
	DmapContainerRecord *container_record;
	DmapContainerDb *container_db;
	DmapRecord *record;
	DmapShare *share;

	db = DMAP_DB(test_dmap_db_new());
	container_record = DMAP_CONTAINER_RECORD (test_dmap_container_record_new ());
	container_db = DMAP_CONTAINER_DB(test_dmap_container_db_new(container_record));


	record = DMAP_RECORD(test_dmap_av_record_new());
	g_object_set(record, "songgenre", "genre1", NULL);
	g_object_set(record, "songartist", "artist1", NULL);
	g_object_set(record, "songalbum", "album1", NULL);

	dmap_db_add(db, record, NULL);

	share = DMAP_SHARE(dmap_av_share_new("name",
	                                     "password",
	                                      db,
	                                      container_db,
	                                     "audio/mp3"));

	ok = dmap_share_serve(share, NULL);
	ck_assert(ok);

	ok = dmap_share_publish(share, NULL);
	ck_assert(ok);

	g_object_unref(db);
	g_object_unref(container_record);
	g_object_unref(container_db);
	g_object_unref(share);
}
END_TEST

START_TEST(_serve_publish_collision_test)
{
	DmapDb *db;
	gboolean ok;
	DmapContainerRecord *container_record;
	DmapContainerDb *container_db;
	DmapRecord *record;
	DmapShare *share1, *share2;

	db = DMAP_DB(test_dmap_db_new());
	container_record = DMAP_CONTAINER_RECORD (test_dmap_container_record_new ());
	container_db = DMAP_CONTAINER_DB(test_dmap_container_db_new(container_record));


	record = DMAP_RECORD(test_dmap_av_record_new());
	g_object_set(record, "songgenre", "genre1", NULL);
	g_object_set(record, "songartist", "artist1", NULL);
	g_object_set(record, "songalbum", "album1", NULL);

	dmap_db_add(db, record, NULL);

	share1 = DMAP_SHARE(dmap_av_share_new("name",
	                                      "password",
	                                       db,
	                                       container_db,
	                                      "audio/mp3"));

	ok = dmap_share_serve(share1, NULL);
	ck_assert(ok);

	ok = dmap_share_publish(share1, NULL);
	ck_assert(ok);

	share2 = DMAP_SHARE(dmap_av_share_new("name",
	                                      "password",
	                                       db,
	                                       container_db,
	                                      "audio/mp3"));

	ok = dmap_share_serve(share2, NULL);
	ck_assert(ok);

	ok = dmap_share_publish(share2, NULL);
	ck_assert(ok);

	g_object_unref(db);
	g_object_unref(container_record);
	g_object_unref(container_db);
	g_object_unref(share1);
	g_object_unref(share2);
}
END_TEST

static void
_tabulator_test(char *property,
                void (*tabulator) (gpointer id, DmapRecord * record, GHashTable * ht))
{
	guint id1, id2;
	DmapRecord *record1, *record2;
	DmapDb *db;
	GHashTable *ht;
	gboolean ok;

	db = DMAP_DB(test_dmap_db_new());

	record1 = DMAP_RECORD(test_dmap_av_record_new());
	g_object_set(record1, property, "str1", NULL);

	id1 = dmap_db_add(db, record1, NULL);

	record2 = DMAP_RECORD(test_dmap_av_record_new());
	g_object_set(record2, property, "str2", NULL);

	id2 = dmap_db_add(db, record2, NULL);

	ht = g_hash_table_new (g_str_hash, g_str_equal);

	tabulator (GINT_TO_POINTER(id1), record1, ht);
	tabulator (GINT_TO_POINTER(id2), record2, ht);

	ok = g_hash_table_contains(ht, "str1");
	ck_assert_int_eq(TRUE, ok);

	ok = g_hash_table_contains(ht, "str2");
	ck_assert_int_eq(TRUE, ok);

	g_object_unref(record1);
	g_object_unref(record2);
	g_object_unref(db);
	g_hash_table_destroy(ht);
}

START_TEST(_genre_tabulator_test)
{
	_tabulator_test("songgenre", _genre_tabulator);
}
END_TEST

START_TEST(_artist_tabulator_test)
{
	_tabulator_test("songartist", _artist_tabulator);
}
END_TEST

START_TEST(_album_tabulator_test)
{
	_tabulator_test("songalbum", _album_tabulator);
}
END_TEST

static int _status = DMAP_STATUS_OK;

static void
_error_cb(G_GNUC_UNUSED DmapShare *share, GError *error, G_GNUC_UNUSED gpointer user_data)
{
	_status = error->code;
}

START_TEST(_should_transcode_test_no)
{
	_status = DMAP_STATUS_OK;
	DmapAvShare *share = dmap_av_share_new("test", NULL, NULL, NULL, NULL);
	g_signal_connect(share, "error", G_CALLBACK(_error_cb), NULL);
	ck_assert_int_eq(FALSE, _should_transcode(share, "mp3", TRUE, "audio/wav"));
	ck_assert_int_eq(DMAP_STATUS_OK, _status);
}
END_TEST

START_TEST(_should_transcode_test_no_trancode_mimetype)
{
	_status = DMAP_STATUS_OK;
	DmapAvShare *share = dmap_av_share_new("test", NULL, NULL, NULL, NULL);
	g_signal_connect(share, "error", G_CALLBACK(_error_cb), NULL);
	ck_assert_int_eq(FALSE, _should_transcode(share, "foo", FALSE, NULL));
	ck_assert_int_eq(DMAP_STATUS_OK, _status);
}
END_TEST

START_TEST(_should_transcode_test_no_trancode_mimetype_unknown_mimetype)
{
	_status = DMAP_STATUS_OK;
	DmapAvShare *share = dmap_av_share_new("test", NULL, NULL, NULL, NULL);
	g_signal_connect(share, "error", G_CALLBACK(_error_cb), NULL);
	ck_assert_int_eq(FALSE, _should_transcode(share, "mp3", FALSE, "foo"));
	ck_assert_int_eq(DMAP_STATUS_BAD_FORMAT, _status);
}
END_TEST

START_TEST(_should_transcode_test_no_trancode_mimetype_already_good)
{
	_status = DMAP_STATUS_OK;
	DmapAvShare *share = dmap_av_share_new("test", NULL, NULL, NULL, NULL);
	g_signal_connect(share, "error", G_CALLBACK(_error_cb), NULL);
	ck_assert_int_eq(FALSE, _should_transcode(share, "mp3", FALSE, "audio/mp3"));
	ck_assert_int_eq(DMAP_STATUS_OK, _status);
}
END_TEST

START_TEST(_should_transcode_test_yes_trancode_mimetype_to_wav)
{
	_status = DMAP_STATUS_OK;
	DmapAvShare *share = dmap_av_share_new("test", NULL, NULL, NULL, NULL);
	g_signal_connect(share, "error", G_CALLBACK(_error_cb), NULL);
	ck_assert_int_eq(TRUE, _should_transcode(share, "mp3", FALSE, "audio/wav"));
	ck_assert_int_eq(DMAP_STATUS_OK, _status);
}
END_TEST

START_TEST(_should_transcode_test_yes_trancode_mimetype_to_mp3)
{
	_status = DMAP_STATUS_OK;
	DmapAvShare *share = dmap_av_share_new("test", NULL, NULL, NULL, NULL);
	g_signal_connect(share, "error", G_CALLBACK(_error_cb), NULL);
	ck_assert_int_eq(TRUE, _should_transcode(share, "wav", FALSE, "audio/mp3"));
	ck_assert_int_eq(DMAP_STATUS_OK, _status);
}
END_TEST

START_TEST(_should_transcode_test_yes_trancode_mimetype_to_mp4)
{
	_status = DMAP_STATUS_OK;
	DmapAvShare *share = dmap_av_share_new("test", NULL, NULL, NULL, NULL);
	g_signal_connect(share, "error", G_CALLBACK(_error_cb), NULL);
	ck_assert_int_eq(TRUE, _should_transcode(share, "wav", FALSE, "video/quicktime"));
	ck_assert_int_eq(DMAP_STATUS_OK, _status);
}
END_TEST

START_TEST(_get_desired_port_test)
{
	DmapShare *share = _build_share_test("_get_desired_port_test");
	ck_assert_int_eq(DAAP_PORT, _get_desired_port(share));
	g_object_unref(share);
}
END_TEST

START_TEST(_get_type_of_service_test)
{
	DmapShare *share = _build_share_test("_get_type_of_service_test");
	ck_assert_str_eq(DAAP_TYPE_OF_SERVICE, _get_type_of_service(share));
	g_object_unref(share);
}
END_TEST

START_TEST(_server_info_test)
{
	char *nameprop = "_server_info_test";
	DmapShare *share;
	SoupServerMessage *message;
	SoupMessageBody *body;
	GBytes *buffer;
	const guint8 *data;
	gsize length;
	GNode *root;
	DmapStructureItem *item;

	share   = _build_share_test(nameprop);
	message = g_object_new (SOUP_TYPE_SERVER_MESSAGE, NULL);

	/* Causes auth. method to be set to DMAP_SHARE_AUTH_METHOD_PASSWORD. */
	g_object_set(share, "password", "password", NULL);

	_server_info(share, message, "/");

	body = soup_server_message_get_response_body(message);
	buffer = soup_message_body_flatten(body);
	data = g_bytes_get_data(buffer, &length);

	root = dmap_structure_parse(data, length, NULL);

	item = dmap_structure_find_item(root, DMAP_CC_MSTT);
	ck_assert_int_eq(SOUP_STATUS_OK, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MPRO);
	ck_assert_int_eq(DAAP_VERSION, item->content.data->v_double);

	item = dmap_structure_find_item(root, DMAP_CC_APRO);
	ck_assert_int_eq(DAAP_VERSION, item->content.data->v_double);

	item = dmap_structure_find_item(root, DMAP_CC_MINM);
	ck_assert_str_eq(nameprop, item->content.data->v_pointer);

	item = dmap_structure_find_item(root, DMAP_CC_MSAU);
	ck_assert_int_eq(DMAP_SHARE_AUTH_METHOD_PASSWORD, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MSLR);
	ck_assert_int_eq(0, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MSTM);
	ck_assert_int_eq(DAAP_TIMEOUT, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MSAL);
	ck_assert_int_eq(0, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MSUP);
	ck_assert_int_eq(1, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MSPI);
	ck_assert_int_eq(0, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MSEX);
	ck_assert_int_eq(0, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MSBR);
	ck_assert_int_eq(0, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MSQY);
	ck_assert_int_eq(0, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MSIX);
	ck_assert_int_eq(0, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MSRS);
	ck_assert_int_eq(0, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MSDC);
	ck_assert_int_eq(1, item->content.data->v_int);

	g_object_unref(share);
}
END_TEST

START_TEST(_message_add_standard_headers_test)
{
	const char *header;
	DmapShare *share;
	SoupMessage *message;
	SoupMessageHeaders *headers;

	share = _build_share_test("_message_add_standard_headers_test");
	message = soup_message_new(SOUP_METHOD_GET, "http://test/");

	soup_message_headers_append(soup_message_get_response_headers(message),
	                           "DMAP-Server",
	                           "libdmapsharing" VERSION);

	headers = soup_message_get_response_headers(message);
	header = soup_message_headers_get_one(headers, "DMAP-Server");

	ck_assert_str_eq("libdmapsharing" VERSION, header);

	g_object_unref(share);
}
END_TEST

START_TEST(_databases_browse_xxx_test)
{
	char *nameprop = "databases_browse_xxx_test";
	DmapShare *share;
	SoupServerMessage *message;
	GHashTable *query;
	SoupMessageBody *body;
	GBytes *buffer;
	const guint8 *data;
	gsize length;
	GNode *root;
	DmapStructureItem *item;

	share   = _build_share_test(nameprop);
	message = g_object_new (SOUP_TYPE_SERVER_MESSAGE, NULL);
	query = g_hash_table_new(g_str_hash, g_str_equal);

	g_hash_table_insert(query, "filter", "");

	_databases_browse_xxx(share, message, "/db/1/browse/genres", query);

	body = soup_server_message_get_response_body(message);
	buffer = soup_message_body_flatten(body);
	data = g_bytes_get_data(buffer, &length);

	root = dmap_structure_parse(data, length, NULL);

	item = dmap_structure_find_item(root, DMAP_CC_MSTT);
	ck_assert_int_eq(SOUP_STATUS_OK, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MUTY);
	ck_assert_int_eq(0, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MTCO);
	ck_assert_int_eq(2, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_MRCO);
	ck_assert_int_eq(2, item->content.data->v_int);

	item = dmap_structure_find_item(root, DMAP_CC_ABGN);
	ck_assert(NULL != item);

	root = dmap_structure_find_node(root, DMAP_CC_MLIT);
	ck_assert(NULL != root);

	ck_assert_str_eq("genre2",
                        ((DmapStructureItem *) root->children->data)->content.data->v_pointer);
	ck_assert_str_eq("genre1",
                        ((DmapStructureItem *) root->next->children->data)->content.data->v_pointer);

	g_object_unref(share);
	g_hash_table_destroy(query);
}
END_TEST

START_TEST(_databases_browse_xxx_artists_test)
{
	char *nameprop = "databases_browse_xxx_artists_test";
	DmapShare *share;
	SoupServerMessage *message;
	GHashTable *query;
	SoupMessageBody *body;
	GBytes *buffer;
	const guint8 *data;
	gsize length;
	GNode *root;

	share   = _build_share_test(nameprop);
	message = g_object_new (SOUP_TYPE_SERVER_MESSAGE, NULL);
	query = g_hash_table_new(g_str_hash, g_str_equal);

	g_hash_table_insert(query, "filter", "");

	_databases_browse_xxx(share, message, "/db/1/browse/artists", query);

	body = soup_server_message_get_response_body(message);
	buffer = soup_message_body_flatten(body);
	data = g_bytes_get_data(buffer, &length);

	root = dmap_structure_parse(data, length, NULL);

	root = dmap_structure_find_node(root, DMAP_CC_MLIT);
	ck_assert(NULL != root);

	ck_assert_str_eq("artist1",
                        ((DmapStructureItem *) root->children->data)->content.data->v_pointer);
	ck_assert_str_eq("artist2",
                        ((DmapStructureItem *) root->next->children->data)->content.data->v_pointer);

	g_object_unref(share);
	g_hash_table_destroy(query);
}
END_TEST

START_TEST(_databases_browse_xxx_albums_test)
{
	char *nameprop = "databases_browse_xxx_albums_test";
	DmapShare *share;
	SoupServerMessage *message;
	GHashTable *query;
	SoupMessageBody *body;
	GBytes *buffer;
	const guint8 *data;
	gsize length;
	GNode *root;

	share   = _build_share_test(nameprop);
	message = g_object_new (SOUP_TYPE_SERVER_MESSAGE, NULL);
	query = g_hash_table_new(g_str_hash, g_str_equal);

	g_hash_table_insert(query, "filter", "");

	_databases_browse_xxx(share, message, "/db/1/browse/albums", query);

	body = soup_server_message_get_response_body(message);
	buffer = soup_message_body_flatten(body);
	data = g_bytes_get_data(buffer, &length);

	root = dmap_structure_parse(data, length, NULL);

	root = dmap_structure_find_node(root, DMAP_CC_MLIT);
	ck_assert(NULL != root);

	ck_assert_str_eq("album1",
                        ((DmapStructureItem *) root->children->data)->content.data->v_pointer);
	ck_assert_str_eq("album2",
                        ((DmapStructureItem *) root->next->children->data)->content.data->v_pointer);

	g_object_unref(share);
	g_hash_table_destroy(query);
}
END_TEST

START_TEST(_databases_browse_xxx_bad_category_test)
{
	char *nameprop = "databases_browse_xxx_bad_category_test";
	DmapShare *share;
	SoupServerMessage *message;
	GHashTable *query;
	SoupMessageBody *body;
	GBytes *buffer;
	const guint8 *data;
	gsize length;
	GNode *root;

	share   = _build_share_test(nameprop);
	message = g_object_new (SOUP_TYPE_SERVER_MESSAGE, NULL);
	query = g_hash_table_new(g_str_hash, g_str_equal);

	g_hash_table_insert(query, "filter", "");

	_databases_browse_xxx(share, message, "/db/1/browse/bad_category", query);

	body = soup_server_message_get_response_body(message);
	buffer = soup_message_body_flatten(body);
	data = g_bytes_get_data(buffer, &length);

	root = dmap_structure_parse(data, length, NULL);
	ck_assert(NULL == root);

	g_object_unref(share);
	g_hash_table_destroy(query);
}
END_TEST

START_TEST(_databases_items_xxx_test)
{
	char *nameprop = "databases_items_xxx_test";
	DmapShare *share;
	SoupServer *server;
	SoupServerMessage *message;
	SoupMessageBody *body = NULL;
	GBytes *buffer;
	char path[PATH_MAX + 1];
	DmapDb *db = NULL;
	DmapRecord *record = NULL;
	gsize size1 = 0, size2 = 0;
	const guint8 *contents1;
	char *location, *contents2, *etag_out;
	GFile *file;
	GError *error = NULL;
	gboolean ok;
	guint64 i;

	share   = _build_share_test(nameprop);
	server  = soup_server_new(NULL, NULL);
	message = g_object_new (SOUP_TYPE_SERVER_MESSAGE, NULL);

	g_snprintf(path, sizeof path, "/db/1/items/%d", G_MAXINT);

	_databases_items_xxx(share, server, message, path);

	g_object_get(share, "db", &db, NULL);
	ck_assert(NULL != db);

	record = dmap_db_lookup_by_id(db, G_MAXINT);
	ck_assert(NULL != record);

	g_object_get(record, "filesize", &size1, "location", &location, NULL);
	ck_assert(0 != size1);
	ck_assert(NULL != location);

	g_signal_emit_by_name(message, "wrote_headers", NULL);

	for (i = 0; i < size1 / DMAP_SHARE_CHUNK_SIZE + 1; i++) {
		g_signal_emit_by_name(message, "wrote_chunk", NULL);
	}

	g_signal_emit_by_name(message, "finished", NULL);

	body = soup_server_message_get_response_body(message);
	ck_assert(NULL != body);

	soup_message_body_set_accumulate (body, TRUE);
	buffer = soup_message_body_flatten(body);
	contents1 = g_bytes_get_data(buffer, &size1);

	file = g_file_new_for_uri(location);
	ck_assert(NULL != file);

	ok = g_file_load_contents(file, NULL, &contents2, &size2, &etag_out, &error);
	ck_assert(ok);

	ck_assert(size1 == size2);
	ck_assert(0 == memcmp(contents1, contents2, size1));

	g_object_unref(record);
	g_object_unref(db);
	g_object_unref(share);
}
END_TEST

START_TEST(_databases_items_xxx_test_bad_id)
{
	char *nameprop = "databases_items_xxx_test";
	DmapShare *share;
	SoupServer *server;
	SoupServerMessage *message;
	char path[PATH_MAX + 1];

	share   = _build_share_test(nameprop);
	server  = soup_server_new(NULL, NULL);
	message = g_object_new (SOUP_TYPE_SERVER_MESSAGE, NULL);

	/* IDs go from G_MAXINT down, so 0 does not exist. */
	g_snprintf(path, sizeof path, "/db/1/items/%d", 0);

	_status = DMAP_STATUS_OK;
	g_signal_connect(share, "error", G_CALLBACK(_error_cb), NULL);
	_databases_items_xxx(share, server, message, path);
	ck_assert_int_eq(DMAP_STATUS_DB_BAD_ID, _status);

	g_object_unref(share);
}
END_TEST

#include "dmap-av-share-suite.c"

#endif
