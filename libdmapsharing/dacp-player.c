/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * libdmapsharing
 * Copyright (C) Alexandre Rosenfeld 2010 <alexandre.rosenfeld@gmail.com>
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

#include <libdmapsharing/dacp-player.h>
#include <libdmapsharing/dmap-enums.h>
#include <libdmapsharing/daap-record.h>

static void
dacp_player_init (DACPPlayerIface *iface)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		initialized = TRUE;

		g_object_interface_install_property (iface,
			g_param_spec_ulong ("playing-time",
			                    "Playing time",
			                    "Playing time (ms)",
			                    0,
			                    G_MAXULONG,
			                    0,
			                    G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
			g_param_spec_boolean ("shuffle-state",
			                      "Shuffle state",
			                      "Shufle state",
			                      FALSE,
			                      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
			g_param_spec_enum ("repeat-state",
			                   "Repeat state",
			                   "Repeat state",
			                   DMAP_TYPE_DACP_REPEAT_STATE,
			                   REPEAT_NONE,
			                   G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
			g_param_spec_enum ("play-state",
			                   "Play state",
			                   "Play state",
			                   DMAP_TYPE_DACP_PLAY_STATE,
			                   PLAY_STOPPED,
			                   G_PARAM_READWRITE));
		
		g_object_interface_install_property (iface,
			g_param_spec_ulong ("volume",
			                    "Volume",
			                    "Volume",
			                    0,
			                    100,
			                    0,
			                    G_PARAM_READWRITE));
	}
}

static void
dacp_player_finalize (DACPPlayerIface *iface)
{
}

GType
dacp_player_get_type (void)
{
	static GType object_type = 0;
	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof(DACPPlayerIface),
			(GBaseInitFunc) dacp_player_init,
			(GBaseFinalizeFunc) dacp_player_finalize
		};
		object_type = g_type_register_static(G_TYPE_INTERFACE,
		                                     "DACPPlayer",
		                                     &object_info, 0);
		g_type_interface_add_prerequisite (object_type, G_TYPE_OBJECT);
	}
	return object_type;
}

DAAPRecord *
dacp_player_now_playing_record (DACPPlayer *player)
{
	return DACP_PLAYER_GET_INTERFACE (player)->now_playing_record (player);
}

const guchar *
dacp_player_now_playing_artwork (DACPPlayer *player, guint width, guint height)
{
	return DACP_PLAYER_GET_INTERFACE (player)->now_playing_artwork (player, width, height);
}

void 
dacp_player_play_pause (DACPPlayer *player)
{
	DACP_PLAYER_GET_INTERFACE (player)->play_pause (player);
}


void 
dacp_player_pause (DACPPlayer *player)
{
	DACP_PLAYER_GET_INTERFACE (player)->pause (player);
}

void 
dacp_player_next_item (DACPPlayer *player)
{
	DACP_PLAYER_GET_INTERFACE (player)->next_item (player);
}

void 
dacp_player_prev_item (DACPPlayer *player)
{
	DACP_PLAYER_GET_INTERFACE (player)->prev_item (player);
}

void 
dacp_player_cue_clear (DACPPlayer *player)
{
	DACP_PLAYER_GET_INTERFACE (player)->cue_clear (player);
}

void 
dacp_player_cue_play (DACPPlayer *player, GList *records, guint index)
{
	DACP_PLAYER_GET_INTERFACE (player)->cue_play (player, records, index);
}
