/*
 *  Database interface for a DMAPRecord factory
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

#ifndef __DMAP_RECORD_FACTORY_H
#define __DMAP_RECORD_FACTORY_H

#include <glib-object.h>

#include <libdmapsharing/dmap-record.h>

G_BEGIN_DECLS

#define TYPE_DMAP_RECORD_FACTORY (dmap_record_factory_get_type ())
#define DMAP_RECORD_FACTORY(o)	 (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				  TYPE_DMAP_RECORD_FACTORY, DMAPRecordFactory))
#define IS_DMAP_RECORD_FACTORY(o)(G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				  TYPE_DMAP_RECORD_FACTORY))
#define DMAP_RECORD_FACTORY_GET_INTERFACE(o) \
				 (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				  TYPE_DMAP_RECORD_FACTORY, \
				  DMAPRecordFactoryInterface))

typedef struct _DMAPRecordFactory DMAPRecordFactory;
typedef struct _DMAPRecordFactoryInterface DMAPRecordFactoryInterface;

struct _DMAPRecordFactoryInterface {
	GTypeInterface parent;

	DMAPRecord *(*create) (DMAPRecordFactory *factory, const char *path);
};

GType dmap_record_factory_get_type (void);

DMAPRecord *dmap_record_factory_create (DMAPRecordFactory *factory, const char *path);

#endif /* __DMAP_RECORD_FACTORY_H */

G_END_DECLS
