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

#define TYPE_DPAP_CONNECTION         (dpap_connection_get_type ())
#define DPAP_CONNECTION(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_DPAP_CONNECTION, DPAPConnection))
#define DPAP_CONNECTION_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TYPE_DPAP_CONNECTION, DPAPConnectionClass))
#define IS_DPAP_CONNECTION(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_DPAP_CONNECTION))
#define IS_DPAP_CONNECTION_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_DPAP_CONNECTION))
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
                                          const gchar *password,
					  DMAPDb      *db,
				          DMAPRecordFactory *factory);

G_END_DECLS

#endif
