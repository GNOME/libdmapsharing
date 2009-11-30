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

#ifndef __DPAP_CONNECTION_H__
#define __DPAP_CONNECTION_H__

#include <glib.h>
#include <glib-object.h>

#include <libdmapsharing/dmap-connection.h>

G_BEGIN_DECLS

/**
 * TYPE_DPAP_CONNECTION:
 *
 * The type for #DPAPConnection.
 */
#define TYPE_DPAP_CONNECTION         (dpap_connection_get_type ())
/**
 * DPAP_CONNECTION:
 * @o: Object which is subject to casting.
 *
 * Casts a #DPAPConnection or derived pointer into a (DPAPConnection*) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DPAP_CONNECTION(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_DPAP_CONNECTION, DPAPConnection))
/**
 * DPAP_CONNECTION_CLASS:
 * @k: a valid #DPAPConnectionClass
 *
 * Casts a derived #DPAPConnectionClass structure into a #DPAPConnectionClass 
 * structure.
 */
#define DPAP_CONNECTION_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TYPE_DPAP_CONNECTION, DPAPConnectionClass))
/**
 * IS_DPAP_CONNECTION:
 * @o: Instance to check for being a %TYPE_DPAP_CONNECTION.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %TYPE_DPAP_CONNECTION.
 */
#define IS_DPAP_CONNECTION(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_DPAP_CONNECTION))
/**
 * IS_DPAP_CONNECTION_CLASS:
 * @k: a #DPAPConnectionClass
 *
 * Checks whether @k "is a" valid #DPAPConnectionClass structure of type
 * %DPAP_CONNECTION_CLASS or derived.
 */
#define IS_DPAP_CONNECTION_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_DPAP_CONNECTION))
/**
 * DPAP_CONNECTION_GET_CLASS:
 * @o: a #DPAPConnection instance.
 *
 * Get the class structure associated to a #DPAPConnection instance.
 *
 * Returns: pointer to object class structure.
 */
#define DPAP_CONNECTION_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_DPAP_CONNECTION, DPAPConnectionClass))

typedef struct _DPAPConnection        DPAPConnection;
typedef struct _DPAPConnectionClass   DPAPConnectionClass;
typedef struct _DPAPConnectionPrivate DPAPConnectionPrivate;

struct _DPAPConnectionClass {
    DMAPConnectionClass parent;
};

struct _DPAPConnection {
    DMAPConnection parent;
};

GType           dpap_connection_get_type (void);

DPAPConnection *dpap_connection_new      (const gchar *name,
                                          const gchar *host,
                                          gint         port,
                                          gboolean     password_protected,
					  DMAPDb      *db,
				          DMAPRecordFactory *factory);

G_END_DECLS

#endif
