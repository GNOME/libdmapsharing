/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Implmentation of DPAP (e.g., iPhoto Picture) sharing
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

#include <glib/gi18n.h>
#include <glib/gslist.h>
#include <glib/gmappedfile.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <libsoup/soup.h>
#include <libsoup/soup-address.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-uri.h>
#include <libsoup/soup-server.h>

#include <libdmapsharing/dmap.h>
#include <libdmapsharing/dmap-utils.h>
#include <libdmapsharing/dmap-structure.h>

static void dpap_share_set_property  (GObject *object,
					 guint prop_id,
					 const GValue *value,
					 GParamSpec *pspec);
static void dpap_share_get_property  (GObject *object,
					 guint prop_id,
					 GValue *value,
				 	 GParamSpec *pspec);
static void dpap_share_dispose	(GObject *object);
guint dpap_share_get_desired_port (DMAPShare *share);
const char *dpap_share_get_type_of_service (DMAPShare *share);
void dpap_share_server_info (DMAPShare         *share,
			     SoupServer        *server,
	  		     SoupMessage       *message,
			     const char        *path,
			     GHashTable        *query,
			     SoupClientContext *context);
void dpap_share_message_add_standard_headers (DMAPShare *share,
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

#define DPAP_TYPE_OF_SERVICE "_dpap._tcp"
#define DPAP_PORT 8770

struct DPAPSharePrivate {
	gchar unused;
};

/* Mmap'ed full image file. Global so that it may be free'ed in a different
 * function call that the one that set it up.
 */
static GMappedFile *mapped_file = NULL;

#define DPAP_SHARE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DPAP_TYPE_SHARE, DPAPSharePrivate))

enum {
	PROP_0,
};

G_DEFINE_TYPE (DPAPShare, dpap_share, DMAP_TYPE_SHARE)

static void
dpap_share_class_init (DPAPShareClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	DMAPShareClass *parent_class = DMAP_SHARE_CLASS (object_class);

	object_class->get_property = dpap_share_get_property;
	object_class->set_property = dpap_share_set_property;
	object_class->dispose = dpap_share_dispose;

	parent_class->get_desired_port     = dpap_share_get_desired_port;
	parent_class->get_type_of_service  = dpap_share_get_type_of_service;
	parent_class->message_add_standard_headers = dpap_share_message_add_standard_headers;
	parent_class->get_meta_data_map     = get_meta_data_map;
	parent_class->add_entry_to_mlcl    = add_entry_to_mlcl;
	parent_class->databases_browse_xxx = databases_browse_xxx;
	parent_class->databases_items_xxx  = databases_items_xxx;
	parent_class->server_info          = dpap_share_server_info;

	g_type_class_add_private (klass, sizeof (DPAPSharePrivate));
}

static void
dpap_share_init (DPAPShare *share)
{
	share->priv = DPAP_SHARE_GET_PRIVATE (share);
	/* FIXME: do I need to manually call parent _init? */
}

static void
dpap_share_set_property (GObject *object,
			    guint prop_id,
			    const GValue *value,
			    GParamSpec *pspec)
{
	// DPAPShare *share = DPAP_SHARE (object);

	switch (prop_id) {
	/* FIXME: */
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
dpap_share_get_property (GObject *object,
			    guint prop_id,
			    GValue *value,
			    GParamSpec *pspec)
{
	// DPAPShare *share = DPAP_SHARE (object);

	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
dpap_share_dispose (GObject *object)
{
	/* FIXME: implement in parent */
}

/* FIXME: trancode_mimetype currently not used for DPAP, only DAAP. 
 *        Threrfore, it is not passed to g_object_new.
 */
DPAPShare *
dpap_share_new (const char *name,
		const char *password,
		gpointer db,
		gpointer container_db,
		gchar *transcode_mimetype)
{
	DPAPShare *share;

	share = DPAP_SHARE (g_object_new (DPAP_TYPE_SHARE,
					     "name", name,
					     "password", password,
					     "db", db,
					     "container-db", container_db,
					     NULL));

	_dmap_share_server_start (DMAP_SHARE (share));
	_dmap_share_publish_start (DMAP_SHARE (share));

	return share;
}

void
dpap_share_message_add_standard_headers (DMAPShare *share, SoupMessage *message)
{
	soup_message_headers_append (message->response_headers, "DPAP-Server", "libdmapsharing" VERSION);
}

#define DMAP_VERSION 2.0
#define DPAP_VERSION 1.1
#define DPAP_TIMEOUT 1800

guint
dpap_share_get_desired_port (DMAPShare *share)
{
	return DPAP_PORT;
}

const char *
dpap_share_get_type_of_service (DMAPShare *share)
{
	return DPAP_TYPE_OF_SERVICE;
}

void
dpap_share_server_info (DMAPShare *share,
		SoupServer        *server,
	  	SoupMessage       *message,
		const char        *path,
		GHashTable        *query,
		SoupClientContext *context)
{
/* MSRV	server info response
 * 	MSTT status
 * 	MPRO dpap version
 * 	PPRO dpap version
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
	dmap_structure_add (msrv, DMAP_CC_MPRO, (gdouble) DMAP_VERSION);
	dmap_structure_add (msrv, DMAP_CC_PPRO, (gdouble) DPAP_VERSION);
	dmap_structure_add (msrv, DMAP_CC_MINM, nameprop);
	/*dmap_structure_add (msrv, DMAP_CC_MSAU, _dmap_share_get_auth_method (share));*/
	/* authentication method
	 * 0 is nothing
	 * 1 is name & password
	 * 2 is password only
	 */
	dmap_structure_add (msrv, DMAP_CC_MSLR, 0);
	dmap_structure_add (msrv, DMAP_CC_MSTM, (gint32) DPAP_TIMEOUT);
	dmap_structure_add (msrv, DMAP_CC_MSAL, (gchar) 0);
	/*dmap_structure_add (msrv, DMAP_CC_MSUP, (gchar) 1);
	 *dmap_structure_add (msrv, DMAP_CC_MSPI, (gchar) 0);
	 *dmap_structure_add (msrv, DMAP_CC_MSEX, (gchar) 0);
	 *dmap_structure_add (msrv, DMAP_CC_MSBR, (gchar) 0);
	 *dmap_structure_add (msrv, DMAP_CC_MSQY, (gchar) 0); */
	dmap_structure_add (msrv, DMAP_CC_MSIX, (gchar) 0);
	/* dmap_structure_add (msrv, DMAP_CC_MSRS, (gchar) 0); */
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
	PHOTO_ASPECTRATIO,
	PHOTO_CREATIONDATE,
	PHOTO_IMAGEFILENAME,
	PHOTO_IMAGEFORMAT,
	PHOTO_IMAGEFILESIZE,
	PHOTO_IMAGELARGEFILESIZE,
	PHOTO_IMAGEPIXELHEIGHT,
	PHOTO_IMAGEPIXELWIDTH,
	PHOTO_IMAGERATING,
	PHOTO_HIRES,
	PHOTO_THUMB,
	PHOTO_FILEDATA,
	PHOTO_IMAGECOMMENTS
} DPAPMetaData;

static struct DMAPMetaDataMap meta_data_map[] = {
	{"dmap.itemid",			ITEM_ID},
    	{"dmap.itemname",		ITEM_NAME},
    	{"dmap.itemkind",		ITEM_KIND},
    	{"dmap.persistentid",		PERSISTENT_ID},
	{"dmap.containeritemid",	CONTAINER_ITEM_ID},
    	{"dpap.aspectratio",		PHOTO_ASPECTRATIO},
    	{"dpap.creationdate",		PHOTO_CREATIONDATE},
	{"dpap.imagefilename",		PHOTO_IMAGEFILENAME},
	{"dpap.imageformat",		PHOTO_IMAGEFORMAT},
	{"dpap.imagefilesize",		PHOTO_IMAGEFILESIZE},
	{"dpap.imagelargefilesize",	PHOTO_IMAGELARGEFILESIZE},
	{"dpap.imagepixelheight",	PHOTO_IMAGEPIXELHEIGHT},
	{"dpap.imagepixelwidth",	PHOTO_IMAGEPIXELWIDTH},
	{"dpap.imagerating",		PHOTO_IMAGERATING},
	{"dpap.thumb",			PHOTO_THUMB},
	{"dpap.hires",			PHOTO_HIRES},
	{"dpap.filedata",		PHOTO_FILEDATA},
	{"dpap.imagecomments",		PHOTO_IMAGECOMMENTS},
	{ NULL,				0}};

#define DPAP_ITEM_KIND_PHOTO 3 /* This is the constant that dpap-sharp uses. */

static GMappedFile *
file_to_mmap (const char *location)
{
        GFile *file;
        GMappedFile *mapped_file = NULL;
        char *path;
        GError *error = NULL;

        file = g_file_new_for_uri (location);
	/* NOTE: this is broken if original filename contains "%20" etc. This
	 * is because g_file_get_path() will translate this to " ", etc. But
	 * the filename really may have used "%20" (not " ").
	 */
        path = g_file_get_path (file);
        if (path == NULL) {
                g_warning ("Couldn't mmap %s: couldn't get path", path);
                g_object_unref (file);
                return mapped_file;
        }
        g_object_unref (file);

        mapped_file = g_mapped_file_new (path, FALSE, &error);
        if (mapped_file == NULL) {
                g_warning ("Unable to map file %s: %s", path, error->message);
        }

        g_free (path);
	return mapped_file;
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
		dmap_structure_add (mlit, DMAP_CC_MIKD, (gchar) DPAP_ITEM_KIND_PHOTO);
	if (_dmap_share_client_requested (mb->bits, ITEM_ID))
		dmap_structure_add (mlit, DMAP_CC_MIID, GPOINTER_TO_UINT (id));
	if (_dmap_share_client_requested (mb->bits, ITEM_NAME)) {
		gchar *filename = NULL;
		g_object_get (record, "filename", &filename, NULL);
		if (filename) {
			dmap_structure_add (mlit, DMAP_CC_MINM, filename);
			g_free (filename);
		} else
			g_warning ("Filename requested but not available");
	}
	if (_dmap_share_client_requested (mb->bits, PERSISTENT_ID))
		dmap_structure_add (mlit, DMAP_CC_MPER, GPOINTER_TO_UINT (id));
	if (TRUE) {
		/* dpap-sharp claims iPhoto '08 will not show thumbnails without PASP
		 * and this does seem to be the case when testing. */
		gchar *aspect_ratio = NULL;
		g_object_get (record, "aspect-ratio", &aspect_ratio, NULL);
		if (aspect_ratio) {
			dmap_structure_add (mlit, DMAP_CC_PASP, aspect_ratio);
			g_free (aspect_ratio);
		} else
			g_warning ("Aspect ratio requested but not available");
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_CREATIONDATE)) {
		gint creation_date = 0;
		g_object_get (record, "creation-date", &creation_date, NULL);
		dmap_structure_add (mlit, DMAP_CC_PICD, creation_date);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGEFILENAME)) {
		gchar *filename = NULL;
		g_object_get (record, "filename", &filename, NULL);
		if (filename) {
			dmap_structure_add (mlit, DMAP_CC_PIMF, filename);
			g_free (filename);
		} else
			g_warning ("Filename requested but not available");
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGEFORMAT)) {
		gchar *format = NULL;
		g_object_get (record, "format", &format, NULL);
		if (format) {
			dmap_structure_add (mlit, DMAP_CC_PFMT, format);
			g_free (format);
		} else
			g_warning ("Format requested but not available");
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGEFILESIZE)) {
		gint filesize = 0;
		g_object_get (record, "filesize", &filesize, NULL);
		dmap_structure_add (mlit, DMAP_CC_PIFS, filesize);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGELARGEFILESIZE)) {
		gint large_filesize = 0;
		g_object_get (record, "large-filesize", &large_filesize, NULL);
		dmap_structure_add (mlit, DMAP_CC_PLSZ, large_filesize);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGEPIXELHEIGHT)) {
		gint pixel_height = 0;
		g_object_get (record, "pixel-height", &pixel_height, NULL);
		dmap_structure_add (mlit, DMAP_CC_PHGT, pixel_height);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGEPIXELWIDTH)) {
		gint pixel_width = 0;
		g_object_get (record, "pixel-width", &pixel_width, NULL);
		dmap_structure_add (mlit, DMAP_CC_PWTH, pixel_width);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGERATING)) {
		gint rating = 0;
		g_object_get (record, "rating", &rating, NULL);
		dmap_structure_add (mlit, DMAP_CC_PRAT, rating);
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_IMAGECOMMENTS)) {
		gchar *comments = NULL;
		g_object_get (record, "comments", &comments, NULL);
		if (comments) {
			dmap_structure_add (mlit, DMAP_CC_PCMT, comments);
			g_free (comments);
		} else
			g_warning ("Comments requested but not available");
	}
	if (_dmap_share_client_requested (mb->bits, PHOTO_FILEDATA)) {
		size_t size = 0;
		unsigned char *data = NULL;
		if (_dmap_share_client_requested (mb->bits, PHOTO_THUMB)) {
			g_object_get (record, "thumbnail", &data, NULL);
			g_object_get (record, "filesize", &size, NULL);
		} else {
			/* Should be PHOTO_HIRES */
			char *location = NULL;
			g_object_get (record, "location", &location, NULL);
			if (mapped_file) {
				/* Free any previously mapped image */
				g_mapped_file_free (mapped_file);
				mapped_file = NULL;
			}

			mapped_file = file_to_mmap (location);
			if (mapped_file == NULL) {
				g_warning ("Error opening %s", location);
				data = NULL;
				size = 0;
			} else {
				data = (unsigned char *) g_mapped_file_get_contents (mapped_file);
				size = g_mapped_file_get_length (mapped_file);
			}
			g_free (location);
		}
		dmap_structure_add (mlit, DMAP_CC_PFDT, data, size);
	}
}

static void
databases_browse_xxx (DMAPShare *share,
                      SoupServer *server,
                      SoupMessage *msg,
                      const char *path,
                      GHashTable *query,
                      SoupClientContext *context)
{
	g_warning ("Unhandled: %s\n", path);
}

static void
send_chunked_file (SoupServer *server, SoupMessage *message, DPAPRecord *record, guint64 filesize)
{
	GInputStream *stream;
	const char *location;
	GError *error = NULL;
	ChunkData *cd = g_new (ChunkData, 1);

	g_object_get (record, "location", &location, NULL);

	cd->server = server;

	stream = G_INPUT_STREAM (dpap_record_read (record, &error));

	if (error != NULL) {
		g_warning ("Couldn't open %s: %s.", location, error->message);
		g_error_free (error);
		soup_message_set_status (message, SOUP_STATUS_INTERNAL_SERVER_ERROR);
		g_free (cd);
		return;
	}

	cd->stream = stream;

	if (cd->stream == NULL) {
		g_warning ("Could not set up input stream");
		g_free (cd);
		return;
	}

	soup_message_headers_set_encoding (message->response_headers, SOUP_ENCODING_CONTENT_LENGTH);
	soup_message_headers_set_content_length (message->response_headers, filesize);

	soup_message_headers_append (message->response_headers, "Connection", "Close");
	soup_message_headers_append (message->response_headers, "Content-Type", "application/x-dmap-tagged");

	g_signal_connect (message, "wrote_headers", G_CALLBACK (dmap_write_next_chunk), cd);
	g_signal_connect (message, "wrote_chunk", G_CALLBACK (dmap_write_next_chunk), cd);
	g_signal_connect (message, "finished", G_CALLBACK (dmap_chunked_message_finished), cd);
	/* NOTE: cd g_free'd by chunked_message_finished(). */
}

static void
databases_items_xxx  (DMAPShare *share,
                      SoupServer *server,
                      SoupMessage *msg,
                      const char *path,
                      GHashTable *query,
                      SoupClientContext *context)
{
	DMAPDb *db;
	const gchar *rest_of_path;
	const gchar *id_str;
	guint id;
	guint64 filesize;
	DPAPRecord *record;

	rest_of_path = strchr (path + 1, '/');
	id_str = rest_of_path + 9;
	id = strtoul (id_str, NULL, 10);

	g_object_get (share, "db", &db, NULL);
	record = DPAP_RECORD (dmap_db_lookup_by_id (db, id));
	g_object_get (record, "large-filesize", &filesize, NULL);

	DMAP_SHARE_GET_CLASS (share)->message_add_standard_headers
				(share, msg);
	soup_message_set_status (msg, SOUP_STATUS_OK);

	send_chunked_file (server, msg, record, filesize);
	
	g_object_unref (record);
}

static struct DMAPMetaDataMap *
get_meta_data_map (DMAPShare *share)
{
        return meta_data_map;
}
