/*
 * Header for DACP (e.g., iTunes Remote) sharing
 *
 * Copyright (C) 2010 Alexandre Rosenfeld <airmind@gmail.com>
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
 *
 */

#ifndef __DACP_SHARE_H
#define __DACP_SHARE_H

#include <glib-object.h>

#include <libdmapsharing/dacp-player.h>
#include <libdmapsharing/dmap-share.h>
#include <libdmapsharing/dmap-db.h>
#include <libdmapsharing/dmap-container-db.h>
#include <libdmapsharing/daap-share.h>

G_BEGIN_DECLS
/**
 * DACP_TYPE_SHARE:
 *
 * The type for #DACPShare.
 */
#define DACP_TYPE_SHARE         (dacp_share_get_type ())
/**
 * DACP_SHARE:
 * @o: Object which is subject to casting.
 * 
 * Casts a #DACPShare or derived pointer into a (DACPShare*) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DACP_SHARE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				 DACP_TYPE_SHARE, DACPShare))
/**
 * DACP_SHARE_CLASS:
 * @k: a valid #DACPShareClass
 *
 * Casts a derived #DACPShareClass structure into a #DACPShareClass structure.
 */
#define DACP_SHARE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				 DACP_TYPE_SHARE, DACPShareClass))
/**
 * IS_DACP_SHARE:
 * @o: Instance to check for being a %DACP_TYPE_SHARE.
 * 
 * Checks whether a valid #GTypeInstance pointer is of type %DACP_TYPE_SHARE.
 */
#define IS_DACP_SHARE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				 DACP_TYPE_SHARE))
/**
 * IS_DACP_SHARE_CLASS:
 * @k: a #DACPShareClass
 * 
 * Checks whether @k "is a" valid #DACPShareClass structure of type
 * %DACP_SHARE or derived.
 */
#define IS_DACP_SHARE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
				 DACP_TYPE_SHARE))
/**
 * DACP_SHARE_GET_CLASS:
 * @o: a #DACPShare instance.
 * 
 * Get the class structure associated to a #DACPShare instance.
 *
 * Returns: pointer to object class structure.
 */
#define DACP_SHARE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				 DACP_TYPE_SHARE, DACPShareClass))
typedef struct DACPSharePrivate DACPSharePrivate;

typedef struct
{
	DAAPShare daap_share_instance;
	DACPSharePrivate *priv;
} DACPShare;

typedef struct
{
	DAAPShareClass daap_share_class;

	  gboolean (*lookup_guid) (DACPShare * share, gchar * guid);
	void (*add_guid) (DACPShare * share, gchar * guid);

	void (*remote_found) (DACPShare * share,
			      gchar * service_name, gchar * remote_name);
	void (*remote_lost) (DACPShare * share, gchar * service_name);
	void (*remote_paired) (DACPShare * share,
			       gchar * service_name, gboolean connected);
} DACPShareClass;

GType dacp_share_get_type (void);

/**
 * dacp_share_new:
 * @library_name: The library name that will be shown in the remote.
 * @player: A #DACPPlayer instance, used to retrieve information from a player
 *          implementation.
 * @db: a media database represented by a #DMAPDb instance.
 * @container_db: a container (album) database represented by a #DMAPContainerDb
 *                instance.
 * 
 * Creates a new DACP share and publishes it using mDNS.
 *
 * Returns: a pointer to a #DACPShare.
 */
DACPShare *dacp_share_new (const gchar * library_name, DACPPlayer * player,
			   DMAPDb * db, DMAPContainerDb * container_db);

/**
 * dacp_share_pair:
 * @share: a #DACPShare 
 * @service_name: DACP client (remote) service identifier.
 * @passcode: 4-Digit PIN code entered by the user.
 * 
 * Pairs a DACP client (Remote) with this server. If the passcode is 
 * correct (the same as shown on the remote), the remote will start connecting
 * to this server.
 */
void dacp_share_pair (DACPShare * share, gchar * service_name,
		      gchar passcode[4]);

/**
 * dacp_share_start_lookup:
 * @share: A #DACPShare.
 *     
 * Start looking up for DACP remotes. Connect to #DACPShare::remote-found signal
 * to detect new remotes. Be aware that when a #DACPShare is created, only 
 * after calling this function is that it starts looking up for Remotes on the
 * network.
 */
void dacp_share_start_lookup (DACPShare * share);

/**
 * dacp_share_stop_lookup:
 * @share: A #DACPShare.
 *     
 * Stop looking up for DACP remotes.
 */
void dacp_share_stop_lookup (DACPShare * share);

/**
 * dacp_share_player_update:
 * @share: A #DACPShare.
 * 
 * Signals that the player has been updated (different track playing, playing
 * state changed, suffle state changed, etc).
 */
void dacp_share_player_updated (DACPShare * share);

#endif /* __DACP_SHARE_H */

G_END_DECLS
