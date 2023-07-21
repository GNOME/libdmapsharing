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

#ifndef _DMAP_CONTROL_PLAYER_H_
#define _DMAP_CONTROL_PLAYER_H_

#include <glib-object.h>

#include "dmap-av-record.h"

G_BEGIN_DECLS
/**
 * SECTION: dmap-control-player
 * @short_description: An interface for media controllers.
 *
 * #DmapControlPlayer provides an interface for controlling the playback of media.
 */

/**
 * DMAP_TYPE_CONTROL_PLAYER:
 *
 * The type for #DmapControlPlayer.
 */
#define DMAP_TYPE_CONTROL_PLAYER             (dmap_control_player_get_type ())
/**
 * DMAP_CONTROL_PLAYER:
 * @o: Object which is subject to casting.
 *
 * Casts a #DmapControlPlayer or derived pointer into a (DmapControlPlayer *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DMAP_CONTROL_PLAYER(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), DMAP_TYPE_CONTROL_PLAYER, DmapControlPlayer))
/**
 * DMAP_IS_CONTROL_PLAYER:
 * @o: Instance to check for being a %DMAP_TYPE_CONTROL_PLAYER.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DMAP_TYPE_CONTROL_PLAYER.
 */
#define DMAP_IS_CONTROL_PLAYER(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), DMAP_TYPE_CONTROL_PLAYER))
/**
 * DMAP_CONTROL_PLAYER_GET_INTERFACE:
 * @o: a #DmapControlPlayer instance.
 *
 * Get the insterface structure associated to a #DmapControlPlayer instance.
 *
 * Returns: pointer to object interface structure.
 */
#define DMAP_CONTROL_PLAYER_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
                                        DMAP_TYPE_CONTROL_PLAYER, DmapControlPlayerInterface))
typedef struct _DmapControlPlayerInterface DmapControlPlayerInterface;
typedef struct _DmapControlPlayer DmapControlPlayer;

typedef enum {
	DMAP_CONTROL_REPEAT_NONE = 0,
	DMAP_CONTROL_REPEAT_SINGLE = 1,
	DMAP_CONTROL_REPEAT_ALL = 2
} DmapControlRepeatState;

typedef enum {
	DMAP_CONTROL_PLAY_STOPPED = 2,
	DMAP_CONTROL_PLAY_PAUSED = 3,
	DMAP_CONTROL_PLAY_PLAYING = 4
} DmapControlPlayState;

struct _DmapControlPlayerInterface
{
	GTypeInterface parent_class;

	DmapAvRecord *(*now_playing_record) (DmapControlPlayer * player);
	gchar *(*now_playing_artwork) (DmapControlPlayer * player,
	                               guint width, guint height);
	void (*play_pause) (DmapControlPlayer * player);
	void (*pause) (DmapControlPlayer * player);
	void (*next_item) (DmapControlPlayer * player);
	void (*prev_item) (DmapControlPlayer * player);

	void (*cue_clear) (DmapControlPlayer * player);
	void (*cue_play) (DmapControlPlayer * player, GList * records, guint index);
};

GType dmap_control_player_get_type (void);

/**
 * dmap_control_player_now_playing_record:
 * @player: a player
 *
 * Returns: (transfer none): the currently playing record.
 */
DmapAvRecord *dmap_control_player_now_playing_record (DmapControlPlayer * player);

/**
 * dmap_control_player_now_playing_artwork:
 * @player: a player
 * @width: width
 * @height: height
 *
 * Returns: (transfer none): artwork for the currently playing record.
 */
gchar *dmap_control_player_now_playing_artwork (DmapControlPlayer * player,
                                                guint width, guint height);

/**
 * dmap_control_player_play_pause:
 * @player: a player
 */
void dmap_control_player_play_pause (DmapControlPlayer * player);

/**
 * dmap_control_player_pause:
 * @player: a player
 */
void dmap_control_player_pause (DmapControlPlayer * player);

/**
 * dmap_control_player_next_item:
 * @player: a player
 */
void dmap_control_player_next_item (DmapControlPlayer * player);

/**
 * dmap_control_player_now_prev_item:
 * @player: a player
 */
void dmap_control_player_prev_item (DmapControlPlayer * player);

/**
 * dmap_control_player_cue_clear:
 * @player: a player
 */
void dmap_control_player_cue_clear (DmapControlPlayer * player);

/**
 * dmap_control_player_cue_play:
 * @player: a player
 * @records: (element-type DmapRecord): a list of records
 * @index: an index
 */
void dmap_control_player_cue_play (DmapControlPlayer * player, GList * records, guint index);

G_END_DECLS
#endif /* _DMAP_CONTROL_PLAYER_H_ */
