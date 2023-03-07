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

#ifndef _DMAP_CONTROL_SHARE_H
#define _DMAP_CONTROL_SHARE_H

#include <glib-object.h>

#include <libdmapsharing/dmap-control-player.h>
#include <libdmapsharing/dmap-share.h>
#include <libdmapsharing/dmap-db.h>
#include <libdmapsharing/dmap-container-db.h>
#include <libdmapsharing/dmap-av-share.h>

G_BEGIN_DECLS
/**
 * SECTION: dmap-control-share
 * @short_description: A DACP share.
 *
 * #DmapControlShare objects encapsulate a DACP share.
 */

/**
 * DMAP_TYPE_CONTROL_SHARE:
 *
 * The type for #DmapControlShare.
 */
#define DMAP_TYPE_CONTROL_SHARE         (dmap_control_share_get_type ())
/**
 * DMAP_CONTROL_SHARE:
 * @o: Object which is subject to casting.
 * 
 * Casts a #DmapControlShare or derived pointer into a (DmapControlShare*) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_CONTROL_SHARE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), \
				 DMAP_TYPE_CONTROL_SHARE, DmapControlShare))
/**
 * DMAP_CONTROL_SHARE_CLASS:
 * @k: a valid #DmapControlShareClass
 *
 * Casts a derived #DmapControlShareClass structure into a #DmapControlShareClass structure.
 */
#define DMAP_CONTROL_SHARE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
				 DMAP_TYPE_CONTROL_SHARE, DmapControlShareClass))
/**
 * DMAP_IS_CONTROL_SHARE:
 * @o: Instance to check for being a %DMAP_TYPE_CONTROL_SHARE.
 * 
 * Checks whether a valid #GTypeInstance pointer is of type %DMAP_TYPE_CONTROL_SHARE.
 */
#define DMAP_IS_CONTROL_SHARE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
				 DMAP_TYPE_CONTROL_SHARE))
/**
 * DMAP_IS_CONTROL_SHARE_CLASS:
 * @k: a #DmapControlShareClass
 * 
 * Checks whether @k "is a" valid #DmapControlShareClass structure of type
 * %DMAP_CONTROL_SHARE or derived.
 */
#define DMAP_IS_CONTROL_SHARE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), \
				 DMAP_TYPE_CONTROL_SHARE))
/**
 * DMAP_CONTROL_SHARE_GET_CLASS:
 * @o: a #DmapControlShare instance.
 * 
 * Get the class structure associated to a #DmapControlShare instance.
 *
 * Returns: pointer to object class structure.
 */
#define DMAP_CONTROL_SHARE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
				 DMAP_TYPE_CONTROL_SHARE, DmapControlShareClass))
typedef struct DmapControlSharePrivate DmapControlSharePrivate;

typedef struct {
	DmapAvShare dmap_av_share_instance;
	DmapControlSharePrivate *priv;
} DmapControlShare;

typedef struct {
	DmapAvShareClass dmap_av_share_class;

	  gboolean (*lookup_guid) (DmapControlShare * share, gchar * guid);
	void (*add_guid) (DmapControlShare * share, gchar * guid);

	void (*remote_found) (DmapControlShare * share,
			      gchar * service_name, gchar * remote_name);
	void (*remote_lost) (DmapControlShare * share, gchar * service_name);
	void (*remote_paired) (DmapControlShare * share,
			       gchar * service_name, gboolean connected);
} DmapControlShareClass;

GType dmap_control_share_get_type (void);

/**
 * dmap_control_share_new:
 * @library_name: The library name that will be shown in the remote.
 * @player: A #DmapControlPlayer instance, used to retrieve information from a player
 *          implementation.
 * @db: a media database represented by a #DmapDb instance.
 * @container_db: a container (album) database represented by a #DmapContainerDb
 *                instance.
 * 
 * Creates a new DACP share and publishes it using mDNS.
 *
 * Returns: a pointer to a #DmapControlShare.
 */
DmapControlShare *dmap_control_share_new (const gchar * library_name, DmapControlPlayer * player,
			   DmapDb * db, DmapContainerDb * container_db);

/**
 * dmap_control_share_pair:
 * @share: a #DmapControlShare 
 * @service_name: DACP client (remote) service identifier.
 * @passcode: 4-Digit PIN code entered by the user.
 * 
 * Pairs a DACP client (Remote) with this server. If the passcode is 
 * correct (the same as shown on the remote), the remote will start connecting
 * to this server.
 */
void dmap_control_share_pair (DmapControlShare * share, gchar * service_name,
		      gchar passcode[4]);

/**
 * dmap_control_share_start_lookup:
 * @share: A #DmapControlShare.
 * @error: A #GError.
 *     
 * Start looking up for DACP remotes. Connect to #DmapControlShare::remote-found signal
 * to detect new remotes. Be aware that when a #DmapControlShare is created, only 
 * after calling this function is that it starts looking up for Remotes on the
 * network.
 *
 * Returns: TRUE on success, else FALSE with error set.
 */
gboolean dmap_control_share_start_lookup (DmapControlShare * share, GError **error);

/**
 * dmap_control_share_stop_lookup:
 * @share: A #DmapControlShare.
 * @error: A #GError.
 *     
 * Stop looking up for DACP remotes.
 */
gboolean dmap_control_share_stop_lookup (DmapControlShare * share, GError **error);

/**
 * dmap_control_share_player_update:
 * @share: A #DmapControlShare.
 * 
 * Signals that the player has been updated (different track playing, playing
 * state changed, suffle state changed, etc).
 */
void dmap_control_share_player_updated (DmapControlShare * share);

#endif /* _DMAP_CONTROL_SHARE_H */

G_END_DECLS
