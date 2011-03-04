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
/**
 * DMAP_TYPE_RECORD_FACTORY:
 *
 * The type for #DMAPRecordFactory.
 */
#define DMAP_TYPE_RECORD_FACTORY (dmap_record_factory_get_type ())
/**
 * DMAP_RECORD_FACTORY:
 * @o: Object which is subject to casting.
 *
 * Casts a #DMAPRecordFactory or derived pointer into a (DMAPRecordFactory *)
 * pointer. Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_RECORD_FACTORY(o)	 (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				  DMAP_TYPE_RECORD_FACTORY, DMAPRecordFactory))
/**
 * IS_DMAP_RECORD_FACTORY:
 * @o: Instance to check for being a %DMAP_TYPE_RECORD_FACTORY.
 *
 * Checks whether a valid #GTypeInstance pointer is of type
 * %DMAP_TYPE_RECORD_FACTORY.
 */
#define IS_DMAP_RECORD_FACTORY(o)(G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				  DMAP_TYPE_RECORD_FACTORY))
/**
 * DMAP_RECORD_FACTORY_GET_INTERFACE:
 * @o: a #DMAPRecordFactory instance.
 *
 * Get the interface structure associated to a #DMAPRecordFactory instance.
 *
 * Returns: pointer to object interface structure.
 */
#define DMAP_RECORD_FACTORY_GET_INTERFACE(o) \
				 (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
				  DMAP_TYPE_RECORD_FACTORY, \
				  DMAPRecordFactoryIface))
typedef struct _DMAPRecordFactory DMAPRecordFactory;
typedef struct _DMAPRecordFactoryIface DMAPRecordFactoryIface;

struct _DMAPRecordFactoryIface
{
	GTypeInterface parent;

	DMAPRecord *(*create) (DMAPRecordFactory * factory,
			       gpointer user_data);
};

GType dmap_record_factory_get_type (void);

/**
 * dmap_record_factory_create:
 * @factory: A DMAPRecordFactory.
 * @user_data: Some piece of data that may be used to initialize return value.
 *
 * Returns: a new DMAPRecord as read from path.
 */
DMAPRecord *dmap_record_factory_create (DMAPRecordFactory * factory,
					gpointer user_data);

#endif /* __DMAP_RECORD_FACTORY_H */

G_END_DECLS
