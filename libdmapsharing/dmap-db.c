/*
 *  Database interface for DMAP sharing
 *
 *  Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
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

#include <stdlib.h>
#include <string.h>
#include <libdmapsharing/dmap-db.h>

typedef struct FilterData {
	DMAPDb *db;
	GSList *filter_def;
	GHashTable *ht;
} FilterData;

static gint dmap_db_init_count = 0;

static void
dmap_db_init (DMAPDbInterface *iface)
{
	dmap_db_init_count++;
}

static void
dmap_db_finalize (DMAPDbInterface *iface)
{
	dmap_db_init_count--;
}

/* FIXME: No G_DEFINE_INTERFACE available in GObject headers: */
GType
dmap_db_get_type (void)
{
	static GType object_type = 0;
	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof(DMAPDbInterface),
			(GBaseInitFunc) dmap_db_init,
			(GBaseFinalizeFunc) dmap_db_finalize
		};
		object_type =
		    g_type_register_static(G_TYPE_INTERFACE,
					   "DMAPDb",
					   &object_info, 0);
	}
	return object_type;
}

DMAPRecord *
dmap_db_lookup_by_id (DMAPDb *db, guint id)
{
	return DMAP_DB_GET_INTERFACE (db)->lookup_by_id (db, id);
}

void
dmap_db_foreach	(const DMAPDb *db,
		 GHFunc func,
		 gpointer data)
{
	DMAP_DB_GET_INTERFACE (db)->foreach (db, func, data);
}

gint
dmap_db_add (DMAPDb *db, DMAPRecord *record)
{
	return DMAP_DB_GET_INTERFACE (db)->add (db, record);
}

gulong
dmap_db_count (const DMAPDb *db)
{
	return DMAP_DB_GET_INTERFACE (db)->count (db);
}

/* Change "\'" to "'". */
static gchar *
unescape (const gchar *str)
{
	gchar *fnval;
	gsize j, i, size;

	size = strlen (str) + 1;
	fnval = g_new0 (gchar, size);

	j = 0;
	for (i = 0; i < size; i++)
		if (str[i] != '\\' || str[i+1] != '\'')
			fnval[j++] = str[i];

	return fnval;
}

gchar **
_dmap_db_strsplit_using_quotes (const gchar *str)
{
	/* What we are splitting looks something like this:
	 * 'foo'text to ignore'bar'.
	 */

	gchar **fnval = NULL;

	if (str != NULL) {
		int i, j;
		fnval = g_strsplit (str, "\'", 0);

		for (i = j = 0; fnval[i]; i++) {
			gchar *token = fnval[i];

			/* Handle areas around ':
			 * 'foo' 'bar'
			 * ^
			 * 'foo' 'bar'
			 *      ^
			 * 'foo'+'bar'
			 *      ^
			 */
			if (*token == '\0' || *token == ' ' || *token == '+')
				continue;

			/* Handle mistaken split at escaped '. */
			if (token[strlen(token) - 1] == '\\') {
				token = g_strconcat (fnval[i], "'", fnval[i+1], NULL);
				g_free (fnval[i]);
				g_free (fnval[i+1]);
				i++;
			}

			fnval[j++] = token;

		}

		fnval[j] = 0x00;
	}

        return fnval;
}

static void
apply_filter (gpointer id, DMAPRecord *record, gpointer data)
{
	FilterData *fd;
	
	fd = data;
	if (fd->filter_def == NULL) {
		g_hash_table_insert (fd->ht, GUINT_TO_POINTER (id), record);
	} else {
		GSList *ptr1, *ptr2;
		gboolean accepted = TRUE;
		for (ptr1 = fd->filter_def; ptr1 != NULL; ptr1 = ptr1->next) {
			for (ptr2 = ptr1->data; ptr2 != NULL; ptr2 = ptr2->next) {
				gchar *value = unescape (((FilterDefinition *) ptr2->data)->value);
				if (((FilterDefinition *) ptr2->data)->record_get_value == NULL) {
					if (GPOINTER_TO_UINT (id) == atoi (value))
						g_hash_table_insert (fd->ht, id, dmap_db_lookup_by_id (fd->db, GPOINTER_TO_UINT (id)));
					accepted = FALSE; /* Not really, but we've already added if required. */
				} else if (g_strcasecmp (value, ((FilterDefinition *) ptr2->data)->record_get_value (record)) != 0)
					accepted = FALSE;
				g_free (value);
			}
		}
		if (accepted == TRUE)
			g_hash_table_insert (fd->ht, GUINT_TO_POINTER (id), record);
	}
}

GHashTable *
dmap_db_apply_filter (DMAPDb *db, GSList *filter_def)
{
	GHashTable *ht;
	FilterData data;

	ht = g_hash_table_new (g_direct_hash, g_direct_equal);
	data.db = db;
	data.filter_def = filter_def;
	data.ht = ht;

	dmap_db_foreach (db, (GHFunc) apply_filter, &data);

	return data.ht;
}
