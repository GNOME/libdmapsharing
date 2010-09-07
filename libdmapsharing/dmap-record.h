/*
 *  Database record interface for DMAP sharing
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

#ifndef __DMAP_RECORD_H
#define __DMAP_RECORD_H

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * TYPE_DMAP_RECORD:
 *
 * The type for #DMAPRecord.
 */
#define TYPE_DMAP_RECORD	     (dmap_record_get_type ())
/**
 * DMAP_RECORD:
 * @o: Object which is subject to casting.
 *
 * Casts a #DMAPRecord or derived pointer into a (DMAPRecord *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_RECORD(o)		     (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				      TYPE_DMAP_RECORD, DMAPRecord))
/**
 * IS_DMAP_RECORD:
 * @o: Instance to check for being a %TYPE_DMAP_RECORD.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %TYPE_DMAP_RECORD.
 */
#define IS_DMAP_RECORD(o)	     (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				      TYPE_DMAP_RECORD))
/**
 * DAAP_RECORD_GET_INTERFACE:
 * @o: a #DAAPRecord instance.
 *
 * Get the class structure associated to a #DAAPRecord instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_RECORD_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				      TYPE_DMAP_RECORD, DMAPRecordInterface))

typedef struct _DMAPRecord DMAPRecord;
typedef struct _DMAPRecordInterface DMAPRecordInterface;

struct _DMAPRecordInterface {
	GTypeInterface parent;

	GByteArray * (*to_blob)	      (DMAPRecord *record);
	DMAPRecord * (*set_from_blob) (DMAPRecord *record, GByteArray *blob);
};

typedef unsigned long long bitwise;

struct MLCL_Bits {
	GNode *mlcl;
	bitwise bits;
	gpointer pointer;
};

typedef enum {
	DMAP_MEDIA_KIND_MUSIC = 1,
	DMAP_MEDIA_KIND_MOVIE = 2
} DMAPMediaKind;

GType       dmap_record_get_type      (void);

/**
 * dmap_record_to_blob:
 * @record: A DMAPRecord.
 *
 * Returns: A byte array representation of the record.
 */
GByteArray *dmap_record_to_blob (DMAPRecord *record);

/**
 * dmap_record_from_blob:
 * @blob: A byte array representation of a record.
 *
 * Returns: The record.
 */
DMAPRecord *dmap_record_set_from_blob (DMAPRecord *record, GByteArray *blob);

#endif /* __DMAP_RECORD_H */

G_END_DECLS
