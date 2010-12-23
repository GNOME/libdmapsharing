/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Implmentation of DAAP (e.g., iTunes Music) sharing
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
#include <libsoup/soup-address.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-uri.h>
#include <libsoup/soup-server.h>

#include <libdmapsharing/dmap.h>
#include <libdmapsharing/dmap-structure.h>
#include <libdmapsharing/dmap-utils.h>

#ifdef HAVE_GSTREAMERAPP
#include <libdmapsharing/dmap-gst-input-stream.h>
#endif /* HAVE_GSTREAMERAPP */

static void daap_share_set_property  (GObject *object,
					 guint prop_id,
					 const GValue *value,
					 GParamSpec *pspec);
static void daap_share_get_property  (GObject *object,
					 guint prop_id,
					 GValue *value,
				 	 GParamSpec *pspec);
static void daap_share_dispose	(GObject *object);
guint daap_share_get_desired_port (DMAPShare *share);
const char *daap_share_get_type_of_service (DMAPShare *share);
void daap_share_server_info (DMAPShare         *share,
			     SoupServer        *server,
	  		     SoupMessage       *message,
			     const char        *path,
			     GHashTable        *query,
			     SoupClientContext *context);
void daap_share_message_add_standard_headers (DMAPShare *share,
					      SoupMessage *message);
static void databases_browse_xxx (DMAPShare *share,
				  SoupServer *server,
				  SoupMessage *msg,
				  const char *path,
				  GHashTable *query,
				  SoupClientContext *context);
static void databases_items_xxx (DMAPShare *share,
				 SoupServer *server,
				 SoupMessage *msg,
				 const char *path,
				 GHashTable *query,
				 SoupClientContext *context);
static struct DMAPMetaDataMap *get_meta_data_map (DMAPShare *share);
static void add_entry_to_mlcl (gpointer id,
			       DMAPRecord *record,
			       gpointer mb);

#define DAAP_TYPE_OF_SERVICE "_daap._tcp"
#define DAAP_PORT 3689

struct DAAPSharePrivate {
	gchar unused;
};

#define DAAP_SHARE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DAAP_TYPE_SHARE, DAAPSharePrivate))

enum {
	PROP_0,
};

G_DEFINE_TYPE (DAAPShare, daap_share, DMAP_TYPE_SHARE)

static void
daap_share_class_init (DAAPShareClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	DMAPShareClass *parent_class = DMAP_SHARE_CLASS (object_class);

	object_class->get_property = daap_share_get_property;
	object_class->set_property = daap_share_set_property;
	object_class->dispose = daap_share_dispose;

	parent_class->get_desired_port     = daap_share_get_desired_port;
	parent_class->get_type_of_service  = daap_share_get_type_of_service;
	parent_class->message_add_standard_headers = daap_share_message_add_standard_headers;
	parent_class->get_meta_data_map    = get_meta_data_map;
	parent_class->add_entry_to_mlcl    = add_entry_to_mlcl;
	parent_class->databases_browse_xxx = databases_browse_xxx;
	parent_class->databases_items_xxx  = databases_items_xxx;
	parent_class->server_info          = daap_share_server_info;

	g_type_class_add_private (klass, sizeof (DAAPSharePrivate));
}

static void
daap_share_init (DAAPShare *share)
{
	share->priv = DAAP_SHARE_GET_PRIVATE (share);
	/* FIXME: do I need to manually call parent _init? */
}

static void
daap_share_set_property (GObject *object,
			    guint prop_id,
			    const GValue *value,
			    GParamSpec *pspec)
{
	// DAAPShare *share = DAAP_SHARE (object);

	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
daap_share_get_property (GObject *object,
			    guint prop_id,
			    GValue *value,
			    GParamSpec *pspec)
{
	// DAAPShare *share = DAAP_SHARE (object);

	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
daap_share_dispose (GObject *object)
{
	/* FIXME: implement in parent */
}

static gchar *mime_to_format (const gchar *transcode_mimetype)
{
	if (! transcode_mimetype) {
		return NULL;
	} else if (! strcmp (transcode_mimetype, "audio/wav")) {
		return g_strdup ("wav");
	} else if ( ! strcmp (transcode_mimetype, "audio/mp3")) {
		return g_strdup ("mp3");
	} else
		return NULL;
}

DAAPShare *
daap_share_new (const char *name,
		const char *password,
		DMAPDb *db,
		DMAPContainerDb *container_db,
		gchar *transcode_mimetype)
{
	DAAPShare *share;

	g_object_ref (db);
	g_object_ref (container_db);

	share = DAAP_SHARE (g_object_new (DAAP_TYPE_SHARE,
					     "name", name,
					     "password", password,
					     "db", db,
					     "container-db", container_db,
					     "transcode-mimetype", transcode_mimetype,
					     NULL));

	_dmap_share_server_start (DMAP_SHARE (share));
	_dmap_share_publish_start (DMAP_SHARE (share));

	return share;
}

void
daap_share_message_add_standard_headers (DMAPShare *share, SoupMessage *message)
{
	soup_message_headers_append (message->response_headers, "DMAP-Server", "libdmapsharing" VERSION);
}

#define DMAP_VERSION 2.0
#define DAAP_VERSION 3.0
#define DAAP_TIMEOUT 1800

guint
daap_share_get_desired_port (DMAPShare *share)
{
	return DAAP_PORT;
}

const char *
daap_share_get_type_of_service (DMAPShare *share)
{
	return DAAP_TYPE_OF_SERVICE;
}

void
daap_share_server_info (DMAPShare *share,
		SoupServer        *server,
	  	SoupMessage       *message,
		const char        *path,
		GHashTable        *query,
		SoupClientContext *context)
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
	dmap_structure_add (msrv, DMAP_CC_MSTT, (gint32) DMAP_STATUS_OK);
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
	dmap_structure_add (msrv, DMAP_CC_MSAU, _dmap_share_get_auth_method (share));
	/* authentication method
	 * 0 is nothing
	 * 1 is name & password
	 * 2 is password only
	 */
	dmap_structure_add (msrv, DMAP_CC_MSLR, 0);
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

	_dmap_share_message_set_from_dmap_structure (share, message, msrv);
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

static struct DMAPMetaDataMap meta_data_map[] = {
	{"dmap.itemid",			ITEM_ID},
    	{"dmap.itemname",		ITEM_NAME},
    	{"dmap.itemkind",		ITEM_KIND},
    	{"dmap.persistentid",		PERSISTENT_ID},
	{"dmap.containeritemid",	CONTAINER_ITEM_ID},
    	{"daap.songalbum",		SONG_ALBUM},
    	{"daap.songartist",		SONG_ARTIST},
    	{"daap.songbitrate",		SONG_BITRATE},
    	{"daap.songbeatsperminute",	SONG_BPM},
    	{"daap.songcomment",		SONG_COMMENT},
    	{"daap.songcompilation",	SONG_COMPILATION},
    	{"daap.songcomposer",		SONG_COMPOSER},
    	{"daap.songdatakind",		SONG_DATA_KIND},
    	{"daap.songdataurl",		SONG_DATA_URL},
    	{"daap.songdateadded",		SONG_DATE_ADDED},
    	{"daap.songdatemodified",	SONG_DATE_MODIFIED},
    	{"daap.songdescription",	SONG_DESCRIPTION},
    	{"daap.songdisabled",		SONG_DISABLED},
    	{"daap.songdisccount",		SONG_DISC_COUNT},
    	{"daap.songdiscnumber",		SONG_DISC_NUMBER},
    	{"daap.songeqpreset",		SONG_EQ_PRESET},
    	{"daap.songformat",		SONG_FORMAT},
    	{"daap.songgenre",		SONG_GENRE},
    	{"daap.songgrouping",		SONG_GROUPING},
    	{"daap.songrelativevolume",	SONG_RELATIVE_VOLUME},
    	{"daap.songsamplerate",		SONG_SAMPLE_RATE},
    	{"daap.songsize",		SONG_SIZE},
    	{"daap.songstarttime",		SONG_START_TIME},
    	{"daap.songstoptime",		SONG_STOP_TIME},
   	{"daap.songtime",		SONG_TIME},
    	{"daap.songtrackcount",		SONG_TRACK_COUNT},
    	{"daap.songtracknumber",	SONG_TRACK_NUMBER},
    	{"daap.songuserrating",		SONG_USER_RATING},
    	{"daap.songyear",		SONG_YEAR},
	{"daap.sortalbum",		SONG_SORT_ALBUM},
	{"daap.sortartist",		SONG_SORT_ARTIST},
	{"com.apple.itunes.has-video",	SONG_HAS_VIDEO},
	{"com.apple.itunes.smart-playlist",		SONG_SMART_PLAYLIST},
	{"com.apple.itunes.is-podcast-playlist",	SONG_IS_PODCAST_PLAYLIST},
	{"com.apple.itunes.special-playlist",		SONG_SPECIAL_PLAYLIST},
	{"com.apple.itunes.saved-genius",		SONG_SAVED_GENIUS},
	{"com.apple.itunes.mediakind",			SONG_MEDIAKIND},
	{"dmap.haschildcontainers",			HAS_CHILD_CONTAINERS},
	{"dmap.parentcontainerid",			PARENT_CONTAINER_ID},
	{ NULL,				0}};

#define DAAP_ITEM_KIND_AUDIO 2
#define DAAP_SONG_DATA_KIND_NONE 0

static void
send_chunked_file (SoupServer *server, SoupMessage *message, DAAPRecord *record, guint64 filesize, guint64 offset, const gchar *transcode_mimetype)
{
	gchar *format = NULL;
	GInputStream *stream;
	gboolean has_video;
	const char *location;
	GError *error = NULL;
	ChunkData *cd = g_new (ChunkData, 1);

	g_object_get (record, "location", &location, NULL);

	/* FIXME: This crashes on powerpc-440fp-linux-gnu:
	 * g_debug ("Sending %s chunked from offset %" G_GUINT64_FORMAT ".", location, offset);
	 */

	cd->server = server;

	stream = G_INPUT_STREAM (daap_record_read (record, &error));

	if (error != NULL) {
		g_warning ("Couldn't open %s: %s.", location, error->message);
		g_error_free (error);
		soup_message_set_status (message, SOUP_STATUS_INTERNAL_SERVER_ERROR);
		g_free (cd);
		return;
	}

	g_object_get (record, "format", &format, NULL);
	if (transcode_mimetype == NULL || ! strcmp (format, mime_to_format (transcode_mimetype))) {
		g_debug ("Not transcoding");
		cd->stream = stream;
#ifdef HAVE_GSTREAMERAPP
	} else {
		cd->stream = dmap_gst_input_stream_new (transcode_mimetype, stream);
	}
#else
	} else {
		g_warning ("Transcode format %s not supported", transcode_mimetype);
		cd->stream = stream;
	}
#endif /* HAVE_GSTREAMERAPP */

	if (cd->stream == NULL) {
		g_warning ("Could not set up input stream");
		g_free (cd);
		return;
	}

	if (offset != 0) {
		if (g_seekable_seek (G_SEEKABLE (cd->stream), offset, G_SEEK_SET, NULL, &error) == FALSE) {
			g_warning ("Error seeking: %s.", error->message);
			g_input_stream_close (cd->stream, NULL, NULL);
			soup_message_set_status (message, SOUP_STATUS_INTERNAL_SERVER_ERROR);
			g_free (cd);
			return;
	 	}
		filesize -= offset;
	}

	/* Free memory after each chunk sent out over network. */
	soup_message_body_set_accumulate (message->response_body, FALSE);

	g_object_get (record, "has-video", &has_video, NULL);
	if (has_video
		/* NOTE: iTunes seems to require this or it stops reading 
		 * video data after about 2.5MB. Perhaps this is so iTunes
		 * knows how much data to buffer.
		 */
	    || transcode_mimetype == NULL) {
	    	/* NOTE: iTunes 8 (and other versions?) will not seek
		 * properly without a Content-Length header.
		 */
		g_debug ("Using HTTP 1.1 content length encoding.");
		soup_message_headers_set_encoding (message->response_headers,
						  SOUP_ENCODING_CONTENT_LENGTH);
		g_debug ("Content length is %" G_GUINT64_FORMAT ".", filesize);
		soup_message_headers_set_content_length (
			message->response_headers, filesize);
	} else if (soup_message_get_http_version (message) == SOUP_HTTP_1_0) {
		/* NOTE: Roku clients support only HTTP 1.0. */
#ifdef HAVE_ENCODING_EOF
		g_debug ("Using HTTP 1.0 encoding.");
		soup_message_headers_set_encoding (message->response_headers,
						   SOUP_ENCODING_EOF);
#else
		g_warning ("Received HTTP 1.0 request, but not built with HTTP 1.0 support");
		soup_message_headers_set_encoding (message->response_headers,
						   SOUP_ENCODING_CHUNKED);
#endif
	} else {
		/* NOTE: Can not provide Content-Length when performing
		 * real-time transcoding.
		 */
		g_debug ("Using HTTP 1.1 chunked encoding.");
		soup_message_headers_set_encoding (message->response_headers,
						   SOUP_ENCODING_CHUNKED);
	}

	soup_message_headers_append (message->response_headers, "Connection", "Close");
	soup_message_headers_append (message->response_headers, "Content-Type", "application/x-dmap-tagged");

	g_signal_connect (message, "wrote_headers", G_CALLBACK (dmap_write_next_chunk), cd);
	g_signal_connect (message, "wrote_chunk", G_CALLBACK (dmap_write_next_chunk), cd);
	g_signal_connect (message, "finished", G_CALLBACK (dmap_chunked_message_finished), cd);
	/* NOTE: cd g_free'd by chunked_message_finished(). */
}

static void
add_entry_to_mlcl (gpointer id,
		   DMAPRecord *record,
		   gpointer _mb)
{
	GNode *mlit;
	struct MLCL_Bits *mb = (struct MLCL_Bits *) _mb;
	mlit = dmap_structure_add (mb->mlcl, DMAP_CC_MLIT);

	if (_dmap_share_client_requested (mb->bits, ITEM_KIND))
		dmap_structure_add (mlit, DMAP_CC_MIKD, (gchar) DAAP_ITEM_KIND_AUDIO);
	if (_dmap_share_client_requested (mb->bits, ITEM_ID))
		dmap_structure_add (mlit, DMAP_CC_MIID, GPOINTER_TO_UINT (id));
	if (_dmap_share_client_requested (mb->bits, ITEM_NAME)) {
		gchar *title = NULL;
		g_object_get (record, "title", &title, NULL);
		if (title) {
			dmap_structure_add (mlit, DMAP_CC_MINM, title);
			g_free (title);
		} else
			g_warning ("Title requested but not available");
	}
	if (_dmap_share_client_requested (mb->bits, PERSISTENT_ID))
		dmap_structure_add (mlit, DMAP_CC_MPER, GPOINTER_TO_UINT (id));
	if (_dmap_share_client_requested (mb->bits, CONTAINER_ITEM_ID))
		dmap_structure_add (mlit, DMAP_CC_MCTI, GPOINTER_TO_UINT (id));
	if (_dmap_share_client_requested (mb->bits, SONG_DATA_KIND))
		dmap_structure_add (mlit, DMAP_CC_ASDK, (gchar) DAAP_SONG_DATA_KIND_NONE);
	/* FIXME: Any use for this?
	if (_dmap_share_client_requested (mb->bits, SONG_DATA_URL))
		dmap_structure_add (mlit, DMAP_CC_ASUL, "daap://192.168.0.100:%u/databases/1/items/%d.%s?session-id=%s", data->port, *id, daap_record_get_format (DAAP_RECORD (record)), data->session_id);
	*/
	if (_dmap_share_client_requested (mb->bits, SONG_ALBUM)) {
		gchar *album = NULL;
		g_object_get (record, "songalbum", &album, NULL);
		if (album) {
			dmap_structure_add (mlit, DMAP_CC_ASAL, album);
			g_free (album);
		} else
			g_warning ("Album requested but not available");
	}
	if (_dmap_share_client_requested (mb->bits, SONG_GROUPING))
		dmap_structure_add (mlit, DMAP_CC_AGRP, "");
	if (_dmap_share_client_requested (mb->bits, SONG_ARTIST)) {
		gchar *artist = NULL;
		g_object_get (record, "songartist", &artist, NULL);
		if (artist) {
			dmap_structure_add (mlit, DMAP_CC_ASAR, artist);
			g_free (artist);
		} else
			g_warning ("Artist requested but not available");
	}
	if (_dmap_share_client_requested (mb->bits, SONG_BITRATE)) {
		gint32 bitrate = 0;
		g_object_get (record, "bitrate", &bitrate, NULL);
		if (bitrate != 0)
			dmap_structure_add (mlit, DMAP_CC_ASBR, (gint32) bitrate);
	}
	if (_dmap_share_client_requested (mb->bits, SONG_BPM))
		dmap_structure_add (mlit, DMAP_CC_ASBT, (gint32) 0);
	if (_dmap_share_client_requested (mb->bits, SONG_COMMENT))
		dmap_structure_add (mlit, DMAP_CC_ASCM, "");
	if (_dmap_share_client_requested (mb->bits, SONG_COMPILATION))
		dmap_structure_add (mlit, DMAP_CC_ASCO, (gchar) FALSE);
	if (_dmap_share_client_requested (mb->bits, SONG_COMPOSER))
		dmap_structure_add (mlit, DMAP_CC_ASCP, "");
	if (_dmap_share_client_requested (mb->bits, SONG_DATE_ADDED)) {
		gint32 firstseen = 0;
		g_object_get (record, "firstseen", &firstseen, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASDA, firstseen);
	}
	if (_dmap_share_client_requested (mb->bits, SONG_DATE_MODIFIED)) {
		gint32 mtime = 0;
		g_object_get (record, "mtime", &mtime, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASDM, mtime);
	}
	if (_dmap_share_client_requested (mb->bits, SONG_DISC_COUNT))
		dmap_structure_add (mlit, DMAP_CC_ASDC, (gint32) 0);
	if (_dmap_share_client_requested (mb->bits, SONG_DISC_NUMBER)) {
		gint32 disc = 0;
		g_object_get (record, "disc", &disc, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASDN, disc);
	}
	if (_dmap_share_client_requested (mb->bits, SONG_DISABLED))
		dmap_structure_add (mlit, DMAP_CC_ASDB, (gchar) FALSE);
	if (_dmap_share_client_requested (mb->bits, SONG_EQ_PRESET))
		dmap_structure_add (mlit, DMAP_CC_ASEQ, "");
	if (_dmap_share_client_requested (mb->bits, SONG_FORMAT)) {
		gchar *format = NULL;
		gchar *transcode_mimetype = NULL;
		g_object_get (record, "transcode-mimetype", &transcode_mimetype, NULL);
		if (transcode_mimetype)
			format = g_strdup (mime_to_format (transcode_mimetype));
		else
			g_object_get (record, "format", &format, NULL);
		if (format) {
			dmap_structure_add (mlit, DMAP_CC_ASFM, format);
			g_free (format);
		} else
			g_warning ("Format requested but not available");
	}
	if (_dmap_share_client_requested (mb->bits, SONG_GENRE)) {
		gchar *genre = NULL;
		g_object_get (record, "songgenre", &genre, NULL);
		if (genre) {
			dmap_structure_add (mlit, DMAP_CC_ASGN, genre);
			g_free (genre);
		} else
			g_warning ("Genre requested but not available");
	}
	if (_dmap_share_client_requested (mb->bits, SONG_DESCRIPTION))
		dmap_structure_add (mlit, DMAP_CC_ASDT, ""); /* FIXME: e.g., wav audio file */
	if (_dmap_share_client_requested (mb->bits, SONG_RELATIVE_VOLUME))
		dmap_structure_add (mlit, DMAP_CC_ASRV, 0);
	if (_dmap_share_client_requested (mb->bits, SONG_SAMPLE_RATE))
		dmap_structure_add (mlit, DMAP_CC_ASSR, 0);
	if (_dmap_share_client_requested (mb->bits, SONG_SIZE)) {
		guint64 filesize = 0;
		g_object_get (record, "filesize", &filesize, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASSZ, (gint32) filesize);
	}
	if (_dmap_share_client_requested (mb->bits, SONG_START_TIME))
		dmap_structure_add (mlit, DMAP_CC_ASST, 0);
	if (_dmap_share_client_requested (mb->bits, SONG_STOP_TIME))
		dmap_structure_add (mlit, DMAP_CC_ASSP, 0);
	if (_dmap_share_client_requested (mb->bits, SONG_TIME)) {
		gint32 duration;
		g_object_get (record, "duration", &duration, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASTM, (1000 * duration));
	}
	if (_dmap_share_client_requested (mb->bits, SONG_TRACK_COUNT))
		dmap_structure_add (mlit, DMAP_CC_ASTC, 0);
	if (_dmap_share_client_requested (mb->bits, SONG_TRACK_NUMBER)) {
		gint32 track = 0;
		g_object_get (record, "track", &track, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASTN, track);
	}
	if (_dmap_share_client_requested (mb->bits, SONG_USER_RATING)) {
		gint32 rating = 0;
		g_object_get (record, "rating", &rating, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASUR, rating);
	}
	if (_dmap_share_client_requested (mb->bits, SONG_YEAR)) {
		gint32 year = 0;
		g_object_get (record, "year", &year, NULL);
		dmap_structure_add (mlit, DMAP_CC_ASYR, year);
	}
	if (_dmap_share_client_requested (mb->bits, SONG_HAS_VIDEO)) {
		gboolean has_video = 0;
		g_object_get (record, "has-video", &has_video, NULL);
		dmap_structure_add (mlit, DMAP_CC_AEHV, has_video);
	}
	if (_dmap_share_client_requested (mb->bits, SONG_SORT_ARTIST)) {
		gchar *sort_artist = NULL;
		g_object_get (record, "sort-artist", &sort_artist, NULL);
		if (sort_artist)
			dmap_structure_add (mlit, DMAP_CC_ASSA, sort_artist);
		else
			g_warning ("Sort artist requested but not available");
	}
	if (_dmap_share_client_requested (mb->bits, SONG_SORT_ALBUM)) {
		gchar *sort_album = NULL;
		g_object_get (record, "sort-album", &sort_album, NULL);
		if (sort_album)
			dmap_structure_add (mlit, DMAP_CC_ASSU, sort_album);
		else
			g_warning ("Sort album requested but not available");
	}
	if (_dmap_share_client_requested (mb->bits, SONG_MEDIAKIND)) {
		gint mediakind = 0;
		g_object_get (record, "mediakind", &mediakind, NULL);
		dmap_structure_add (mlit, DMAP_CC_AEMK, mediakind);
	}
}

static void
genre_tabulator (gpointer id, DMAPRecord *record, GHashTable *ht)
{
	const gchar *genre;
	g_object_get (record, "songgenre", &genre, NULL);
	if (!genre)
		return;
	if (! g_hash_table_lookup (ht, genre))
		g_hash_table_insert (ht, (gchar *) genre, NULL);
}

static void
artist_tabulator (gpointer id, DMAPRecord *record, GHashTable *ht)
{
	const gchar *artist;
	g_object_get (record, "songartist", &artist, NULL);
	if (!artist)
		return;
	if (! g_hash_table_lookup (ht, artist))
		g_hash_table_insert (ht, (gchar *) artist, NULL);
}

static void
album_tabulator (gpointer id, DMAPRecord *record, GHashTable *ht)
{
	const gchar *album;
	g_object_get (record, "songalbum", &album, NULL);
	if (!album)
		return;
	if (! g_hash_table_lookup (ht, album))
		g_hash_table_insert (ht, (gchar *) album, NULL);
}

static void
add_to_category_listing (gpointer key, gpointer user_data)
{
	GNode *mlit;
	GNode *node = (GNode *) user_data;

	mlit = dmap_structure_add (node, DMAP_CC_MLIT);
	dmap_structure_add (mlit, DMAP_RAW, (char *) key);
}

static void
databases_browse_xxx (DMAPShare *share,
		      SoupServer *server,
		      SoupMessage *msg,
		      const char *path,
		      GHashTable *query,
		      SoupClientContext *context)
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
	DMAPDb *db;
	const gchar *rest_of_path;
	GNode *abro, *node;
	gchar *filter;
	GSList *filter_def;
	GHashTable *filtered;
	guint num_genre;
	const gchar *browse_category;
	GHashTable *category_items;
	DMAPContentCode category_cc;
	GList *values;

	rest_of_path = strchr (path + 1, '/');
	browse_category = rest_of_path + 10;
	category_items = g_hash_table_new (g_str_hash, g_str_equal);

	filter = g_hash_table_lookup (query, "filter");
	filter_def = _dmap_share_build_filter (filter);
	g_object_get (share, "db", &db, NULL);
	filtered = dmap_db_apply_filter (db, filter_def);

	if (g_ascii_strcasecmp (browse_category, "genres") == 0) {
		g_hash_table_foreach (filtered, (GHFunc) genre_tabulator, category_items);
		category_cc = DMAP_CC_ABGN;
	} else if (g_ascii_strcasecmp (browse_category, "artists") == 0) {
		g_hash_table_foreach (filtered, (GHFunc) artist_tabulator, category_items);
		category_cc = DMAP_CC_ABAR;
	} else if (g_ascii_strcasecmp (browse_category, "albums") == 0) {
		g_hash_table_foreach (filtered, (GHFunc) album_tabulator, category_items);
		category_cc = DMAP_CC_ABAL;
	} else {
		g_warning ("Unsupported browse category: %s",
			   browse_category);
		goto _bad_category;
	}

	abro = dmap_structure_add (NULL, DMAP_CC_ABRO);
	dmap_structure_add (abro, DMAP_CC_MSTT, (gint32) DMAP_STATUS_OK);
	dmap_structure_add (abro, DMAP_CC_MUTY, 0);

	num_genre = g_hash_table_size (category_items);
	dmap_structure_add (abro, DMAP_CC_MTCO, (gint32) num_genre);
	dmap_structure_add (abro, DMAP_CC_MRCO, (gint32) num_genre);

	node = dmap_structure_add (abro, category_cc);

	values = g_hash_table_get_keys (category_items);
	if (values && g_hash_table_lookup (query, "include-sort-headers")) {
		g_debug ("Sorting...");
		values = g_list_sort (values, (GCompareFunc) g_ascii_strcasecmp);
	}

	g_list_foreach (values, add_to_category_listing, node);
	
	g_list_free (values);

	_dmap_share_message_set_from_dmap_structure (share, msg, abro);
	dmap_structure_destroy (abro);
_bad_category:
	dmap_share_free_filter (filter_def);
	/* Free's hash table but not data (points into real DB): */
	g_hash_table_destroy (filtered);
	g_hash_table_destroy (category_items);
}

static void
databases_items_xxx (DMAPShare *share,
		     SoupServer *server,
		     SoupMessage *msg,
		     const char *path,
		     GHashTable *query,
		     SoupClientContext *context)
{
	DMAPDb *db;
	const gchar *transcode_mimetype;
	const gchar *rest_of_path;
	const gchar *id_str;
	guint id;
	const gchar *range_header;
	guint64 filesize;
	guint64 offset = 0;
	DAAPRecord *record;

	rest_of_path = strchr (path + 1, '/');
	id_str = rest_of_path + 9;
	id = strtoul (id_str, NULL, 10);

	g_object_get (share, "db", &db, NULL);
	record = DAAP_RECORD (dmap_db_lookup_by_id (db, id));
	g_object_get (record, "filesize", &filesize, NULL);

	DMAP_SHARE_GET_CLASS (share)->message_add_standard_headers
				(share, msg);
	soup_message_headers_append (msg->response_headers, "Accept-Ranges", "bytes");

	range_header = soup_message_headers_get (msg->request_headers, "Range");
	if (range_header) {
		const gchar *s;
		gchar *content_range;

		s = range_header + 6; /* bytes= */
		offset = atoll (s);

		content_range = g_strdup_printf ("bytes %" G_GUINT64_FORMAT "-%" G_GUINT64_FORMAT "/%" G_GUINT64_FORMAT, offset, filesize, filesize);
		soup_message_headers_append (msg->response_headers, "Content-Range", content_range);
		g_debug ("Content range is %s.", content_range);
		g_free (content_range);
		soup_message_set_status (msg, SOUP_STATUS_PARTIAL_CONTENT);
	} else {
		soup_message_set_status (msg, SOUP_STATUS_OK);
	}
	g_object_get (share, "transcode-mimetype", &transcode_mimetype, NULL);
	send_chunked_file (server, msg, record, filesize, offset, transcode_mimetype);
	
	g_object_unref (record);
}

static struct DMAPMetaDataMap *
get_meta_data_map (DMAPShare *share)
{
	return meta_data_map;
}
