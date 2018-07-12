/* Copyright (C) Alexandre Rosenfeld 2010 <alexandre.rosenfeld@gmail.com>
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

#include <libdmapsharing/dmap-control-player.h>
#include <libdmapsharing/dmap-enums.h>
#include <libdmapsharing/dmap-av-record.h>

static void
dmap_control_player_default_init (DmapControlPlayerInterface * iface)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		initialized = TRUE;

		g_object_interface_install_property (iface,
						     g_param_spec_ulong
						     ("playing-time",
						      "Playing time",
						      "Playing time (ms)", 0,
						      G_MAXULONG, 0,
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_boolean
						     ("shuffle-state",
						      "Shuffle state",
						      "Shufle state", FALSE,
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_enum
						     ("repeat-state",
						      "Repeat state",
						      "Repeat state",
						      DMAP_TYPE_DMAP_CONTROL_REPEAT_STATE,
						      DMAP_CONTROL_REPEAT_NONE,
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_enum
						     ("play-state",
						      "Play state",
						      "Play state",
						      DMAP_TYPE_DMAP_CONTROL_PLAY_STATE,
						      DMAP_CONTROL_PLAY_STOPPED,
						      G_PARAM_READWRITE));

		g_object_interface_install_property (iface,
						     g_param_spec_ulong
						     ("volume", "Volume",
						      "Volume", 0, 100, 0,
						      G_PARAM_READWRITE));
	}
}

G_DEFINE_INTERFACE(DmapControlPlayer, dmap_control_player, G_TYPE_OBJECT)

DmapAvRecord *
dmap_control_player_now_playing_record (DmapControlPlayer * player)
{
	return DMAP_CONTROL_PLAYER_GET_INTERFACE (player)->
		now_playing_record (player);
}

gchar *
dmap_control_player_now_playing_artwork (DmapControlPlayer * player, guint width,
				 guint height)
{
	return DMAP_CONTROL_PLAYER_GET_INTERFACE (player)->
		now_playing_artwork (player, width, height);
}

void
dmap_control_player_play_pause (DmapControlPlayer * player)
{
	DMAP_CONTROL_PLAYER_GET_INTERFACE (player)->play_pause (player);
}

void
dmap_control_player_pause (DmapControlPlayer * player)
{
	DMAP_CONTROL_PLAYER_GET_INTERFACE (player)->pause (player);
}

void
dmap_control_player_next_item (DmapControlPlayer * player)
{
	DMAP_CONTROL_PLAYER_GET_INTERFACE (player)->next_item (player);
}

void
dmap_control_player_prev_item (DmapControlPlayer * player)
{
	DMAP_CONTROL_PLAYER_GET_INTERFACE (player)->prev_item (player);
}

void
dmap_control_player_cue_clear (DmapControlPlayer * player)
{
	DMAP_CONTROL_PLAYER_GET_INTERFACE (player)->cue_clear (player);
}

void
dmap_control_player_cue_play (DmapControlPlayer * player, GList * records, guint index)
{
	DMAP_CONTROL_PLAYER_GET_INTERFACE (player)->cue_play (player, records, index);
}
