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

typedef struct FilterData
{
	DMAPDb *db;
	GSList *filter_def;
	GHashTable *ht;
} FilterData;

static gint dmap_db_init_count = 0;

static void
dmap_db_init (DMAPDbIface * iface)
{
	dmap_db_init_count++;
}

static void
dmap_db_finalize (DMAPDbIface * iface)
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
			sizeof (DMAPDbIface),
			(GBaseInitFunc) dmap_db_init,
			(GBaseFinalizeFunc) dmap_db_finalize
		};
		object_type =
			g_type_register_static (G_TYPE_INTERFACE,
						"DMAPDb", &object_info, 0);
		g_type_interface_add_prerequisite (object_type,
						   G_TYPE_OBJECT);
	}
	return object_type;
}

DMAPRecord *
dmap_db_lookup_by_id (const DMAPDb * db, guint id)
{
	return DMAP_DB_GET_INTERFACE (db)->lookup_by_id (db, id);
}

guint
dmap_db_lookup_id_by_location (const DMAPDb * db, const gchar * location)
{
	return DMAP_DB_GET_INTERFACE (db)->lookup_id_by_location (db,
								  location);
}

void
dmap_db_foreach (const DMAPDb * db, DMAPIdRecordFunc func, gpointer data)
{
	DMAP_DB_GET_INTERFACE (db)->foreach (db, func, data);
}

guint
dmap_db_add (DMAPDb * db, DMAPRecord * record)
{
	return DMAP_DB_GET_INTERFACE (db)->add (db, record);
}

guint
dmap_db_add_with_id (DMAPDb * db, DMAPRecord * record, guint id)
{
	return DMAP_DB_GET_INTERFACE (db)->add_with_id (db, record, id);
}

guint
dmap_db_add_path (DMAPDb * db, const gchar * path)
{
	return DMAP_DB_GET_INTERFACE (db)->add_path (db, path);
}

gulong
dmap_db_count (const DMAPDb * db)
{
	return DMAP_DB_GET_INTERFACE (db)->count (db);
}

gchar **
_dmap_db_strsplit_using_quotes (const gchar * str)
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
			if (token[strlen (token) - 1] == '\\') {
				token = g_strconcat (fnval[i], "'",
						     fnval[i + 1], NULL);
				g_free (fnval[i]);
				g_free (fnval[i + 1]);
				i++;
			}

			fnval[j++] = token;

		}

		fnval[j] = 0x00;
	}

	return fnval;
}

static gboolean
compare_record_property (DMAPRecord * record, const gchar * property_name,
			 const gchar * property_value)
{
	GParamSpec *pspec;
	GValue value = { 0, };
	/* Note that this string belongs to value and will not be freed explicitely. */
	const gchar *str_value;
	gboolean accept;

	pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (record),
					      property_name);

	if (pspec == NULL)
		// Can't find the property in this record, so don't accept it.
		return FALSE;

	// Get the property value as a GValue set to the type of this
	// property.
	g_value_init (&value, G_PARAM_SPEC_VALUE_TYPE (pspec));
	g_object_get_property (G_OBJECT (record), property_name, &value);

	if (G_VALUE_HOLDS_STRING (&value)) {
		str_value = g_value_get_string (&value);
	} else if (G_VALUE_HOLDS_BOOLEAN (&value)) {
		g_debug ("Compare %s (boolean): %d %s", property_name, g_value_get_boolean (&value), property_value);
		accept = (g_value_get_boolean (&value) &&
			  g_strcmp0 (property_value, "1") == 0);
		g_value_unset (&value);
		return accept;
	} else if (g_value_type_transformable
		   (G_VALUE_TYPE (&value), G_TYPE_LONG)) {
		// Prefer integer conversion.
		GValue dest = { 0, };
		g_value_init (&dest, G_TYPE_LONG);
		if (!g_value_transform (&value, &dest)) {
			g_warning
				("Failed to convert value into long for property %s",
				 property_name);
			g_value_unset (&value);
			return FALSE;
		}
		g_debug ("Compare %s (long): %ld %s", property_name, g_value_get_long (&dest), property_value);
		accept = (g_value_get_long (&dest) ==
			  strtol (property_value, NULL, 10));
		g_value_unset (&value);
		return accept;
	} else if (g_value_type_transformable
		   (G_VALUE_TYPE (&value), G_TYPE_STRING)) {
		// Use standard transform functions from GLib (note that these
		// functions are unreliable and known cases should be handled
		// above).
		GValue dest;

		g_value_init (&dest, G_TYPE_STRING);
		if (!g_value_transform (&value, &dest)) {
			g_warning
				("Failed to convert value into string for property %s",
				 property_name);
			g_value_unset (&value);
			return FALSE;
		}
		str_value = g_value_dup_string (&dest);
		g_value_reset (&value);
		//Sets the string to value so that it will be freed later.
		g_value_take_string (&value, (gchar *) str_value);
		g_value_unset (&dest);
	} else {
		g_warning ("Attempt to compare unhandled type");
		g_value_unset (&value);
		return FALSE;
	}

	// Only arrive here if we are handling strings.
	g_debug ("Compare %s (string): %s %s", property_name, str_value, property_value);
	if (str_value != NULL && property_value != NULL &&
	    g_ascii_strcasecmp (str_value, property_value) == 0) {
		accept = TRUE;
	} else if (str_value == NULL && property_value == NULL) {
		accept = TRUE;
	} else {
		accept = FALSE;
	}

	// This will destroy str_value since it belongs to value.
	g_value_unset (&value);

	return accept;
}

static void
apply_filter (guint id, DMAPRecord * record, gpointer data)
{
	FilterData *fd;
	gboolean accept = FALSE;

	const gchar *query_key;
	const gchar *query_value;

	g_return_if_fail (record != NULL);
	g_return_if_fail (G_IS_OBJECT (record));

	fd = data;
	if (fd->filter_def == NULL) {
		g_hash_table_insert (fd->ht, GUINT_TO_POINTER (id),
				     g_object_ref (record));
		return;
	}

	GSList *list, *filter;

	for (list = fd->filter_def; list != NULL; list = list->next) {
		for (filter = list->data; filter != NULL;
		     filter = filter->next) {
			DMAPDbFilterDefinition *def = filter->data;
			const gchar *property_name;

			query_key = def->key;
			query_value = def->value;

			if (g_strcmp0 (query_key, "dmap.itemid") == 0) {
				if (GPOINTER_TO_UINT (id) ==
				    strtoul (query_value, NULL, 10)) {
					accept = TRUE;
					break;
				}
			};

			// Use only the part after the last dot.
			// For instance, dmap.songgenre becomes songgenre.
			property_name = strrchr (query_key, '.');
			if (property_name == NULL)
				property_name = query_key;
			else
				//Don't include the dot in the property name.
				property_name++;

			accept = compare_record_property (record,
							  property_name,
							  query_value);

			if (def->negate)
				accept = !accept;

			// If we accept this value, then quit looking at this 
			// group (groups are always OR)
			if (accept)
				break;
		}
		// Don't look any further, because groups are AND between 
		// each other, the first FALSE means FALSE at the end.
		if (!accept)
			break;
	}
	if (accept) {
		g_hash_table_insert (fd->ht, GUINT_TO_POINTER (id),
				     g_object_ref (record));
	}
}

GHashTable *
dmap_db_apply_filter (DMAPDb * db, GSList * filter_def)
{
	GHashTable *ht;
	FilterData data;

	ht = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
				    g_object_unref);
	data.db = db;
	data.filter_def = filter_def;
	data.ht = ht;

	dmap_db_foreach (db, (DMAPIdRecordFunc) apply_filter, &data);

	return data.ht;
}
