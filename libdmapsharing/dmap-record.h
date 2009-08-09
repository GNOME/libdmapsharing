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

#define TYPE_DMAP_RECORD	     (dmap_record_get_type ())
#define DMAP_RECORD(o)		     (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				      TYPE_DMAP_RECORD, DMAPRecord))
#define IS_DMAP_RECORD(o)	     (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				      TYPE_DMAP_RECORD))
#define DMAP_RECORD_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				      TYPE_DMAP_RECORD, DMAPRecordInterface))

typedef struct _DMAPRecord DMAPRecord;
typedef struct _DMAPRecordInterface DMAPRecordInterface;

struct _DMAPRecordInterface {
	GTypeInterface parent;
};

typedef unsigned long long bitwise;

struct MLCL_Bits {
	GNode *mlcl;
	bitwise bits;
	gpointer pointer;
};

GType       dmap_record_get_type      (void);

#endif /* __DMAP_RECORD_H */

G_END_DECLS
