/*
 * Copyright (C) 2004,2005 Charles Schmidt <cschmidt2@emich.edu>
 * Copyright (C) 2006 INDT
 *  Andre Moreira Magalhaes <andre.magalhaes@indt.org.br>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*
 */

#ifndef _DMAP_STRUCTURE_H
#define _DMAP_STRUCTURE_H

#include <glib.h>
#include <glib-object.h>

#include <libdmapsharing/dmap-cc.h>

typedef struct _DmapStructureItem DmapStructureItem;

struct _DmapStructureItem
{
	DmapContentCode content_code;
	GValue content;
	guint32 size;
};

GNode *dmap_structure_add (GNode * parent, DmapContentCode cc, ...);
gchar *dmap_structure_serialize (GNode * structure, guint * length);
GNode *dmap_structure_parse (const guint8 * buf, gsize buf_length, GError **error);
DmapStructureItem *dmap_structure_find_item (GNode * structure,
					     DmapContentCode code);
GNode *dmap_structure_find_node (GNode * structure, DmapContentCode code);
void dmap_structure_print (GNode * structure);
void dmap_structure_destroy (GNode * structure);
guint dmap_structure_get_size (GNode * structure);
void dmap_structure_increase_by_predicted_size (GNode * structure,
						guint size);

typedef enum {
	DMAP_TYPE_BYTE = 0x0001,
	DMAP_TYPE_SIGNED_INT = 0x0002,
	DMAP_TYPE_SHORT = 0x0003,
	DMAP_TYPE_INT = 0x0005,
	DMAP_TYPE_INT64 = 0x0007,
	DMAP_TYPE_STRING = 0x0009,
	DMAP_TYPE_DATE = 0x000A,
	DMAP_TYPE_VERSION = 0x000B,
	DMAP_TYPE_CONTAINER = 0x000C,
	DMAP_TYPE_POINTER = 0x002A,
	DMAP_TYPE_INVALID = 0xFFFF
} DmapType;

typedef struct {
	DmapContentCode code;
	gint32 int_code;
	const gchar *name;
	const gchar *string;
	DmapType type;
} DmapContentCodeDefinition;

const DmapContentCodeDefinition * dmap_structure_content_codes (guint * number);
gint32 dmap_structure_cc_string_as_int32 (const gchar * str);

G_END_DECLS
#endif
