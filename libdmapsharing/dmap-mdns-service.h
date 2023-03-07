/*
 * Copyright (C) 2014 W. Michael Petullo <mike@flyn.org>
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

#ifndef _DMAP_MDNS_SERVICE_H
#define _DMAP_MDNS_SERVICE_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS
/**
 * SECTION: dmap-mdns-service
 * @short_description: An mDNS service.
 *
 * #DmapMdnsService objects advertise DMAP shares.
 */

/**
 * DMAP_TYPE_MDNS_SERVICE:
 *
 * The type for #DmapMdnsService.
 */
#define DMAP_TYPE_MDNS_SERVICE         (dmap_mdns_service_get_type ())
/**
 * DMAP_MDNS_SERVICE:
 * @o: Object which is subject to casting.
 *
 * Casts a #DmapMdnsService or derived pointer into a (DmapMdnsService *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_MDNS_SERVICE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), DMAP_TYPE_MDNS_SERVICE, DmapMdnsService))
/**
 * DMAP_MDNS_SERVICE_CLASS:
 * @k: a valid #DmapMdnsServiceClass
 *
 * Casts a derived #DmapMdnsServiceClass structure into a #DmapMdnsServiceClass structure.
 */
#define DMAP_MDNS_SERVICE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), DMAP_TYPE_MDNS_SERVICE, DmapMdnsServiceClass))
/**
 * DMAP_IS_MDNS_SERVICE:
 * @o: Instance to check for being a %DMAP_TYPE_MDNS_SERVICE.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DAAP_TYPE_MDNS_SERVICE.
 */
#define DMAP_IS_MDNS_SERVICE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), DMAP_TYPE_MDNS_SERVICE))
/**
 * DMAP_IS_MDNS_SERVICE_CLASS:
 * @k: a #DmapMdnsServiceClass
 *
 * Checks whether @k "is a" valid #DmapMdnsServiceClass structure of type
 * %DMAP_MDNS_SERVICE or derived.
 */
#define DMAP_IS_MDNS_SERVICE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), DMAP_TYPE_MDNS_SERVICE))
/**
 * DMAP_MDNS_SERVICE_GET_CLASS:
 * @o: a #DmapMdnsService instance.
 *
 * Get the class structure associated to a #DmapMdnsService instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_MDNS_SERVICE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), DMAP_TYPE_MDNS_SERVICE, DmapMdnsServiceClass))

typedef struct _DmapMdnsServicePrivate DmapMdnsServicePrivate;

/**
 * DmapMdnsServiceType:
 * @DMAP_MDNS_SERVICE_TYPE_INVALID: an invalid service type
 * @DMAP_MDNS_SERVICE_TYPE_DAAP: a DAAP service type
 * @DMAP_MDNS_SERVICE_TYPE_DPAP: a DPAP service type
 * @DMAP_MDNS_SERVICE_TYPE_DACP: a DACP service type
 * @DMAP_MDNS_SERVICE_TYPE_RAOP: a RAOP service type
 * @DMAP_MDNS_SERVICE_TYPE_LAST: an invalid service type
 *
 * Enum values used to specify the service type.
 *
 */
typedef enum {
	DMAP_MDNS_SERVICE_TYPE_INVALID = 0,
	DMAP_MDNS_SERVICE_TYPE_DAAP,
	DMAP_MDNS_SERVICE_TYPE_DPAP,
	DMAP_MDNS_SERVICE_TYPE_DACP,
	DMAP_MDNS_SERVICE_TYPE_RAOP,
	DMAP_MDNS_SERVICE_TYPE_LAST = DMAP_MDNS_SERVICE_TYPE_RAOP
} DmapMdnsServiceType;

// FIXME: this is only for RAOP and corresponds to the "tp" txt record.
// This should be in a sub-class.
typedef enum {
	DMAP_MDNS_SERVICE_TRANSPORT_PROTOCOL_TCP = 0,
	DMAP_MDNS_SERVICE_TRANSPORT_PROTOCOL_UDP,
	DMAP_MDNS_SERVICE_TRANSPORT_PROTOCOL_LAST = DMAP_MDNS_SERVICE_TRANSPORT_PROTOCOL_UDP
} DmapMdnsServiceTransportProtocol;

static const char * const _service_type_name[] = {
	NULL,
	"_daap._tcp",
	"_dpap._tcp",
	"_touch-remote._tcp",
	"_raop._tcp"
};

typedef struct {
	GObjectClass parent_class;
} DmapMdnsServiceClass;

typedef struct {
	GObject object;

	DmapMdnsServicePrivate *priv;
} DmapMdnsService;

GType dmap_mdns_service_get_type (void);

G_END_DECLS
#endif
