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

#ifndef _DACP_PLAYER_H_
#define _DACP_PLAYER_H_

#include <glib-object.h>

#include "daap-record.h"

G_BEGIN_DECLS
/**
 * DACP_TYPE_PLAYER:
 *
 * The type for #DACPPlayer.
 */
#define DACP_TYPE_PLAYER               (dacp_player_get_type ())
/**
 * DACP_PLAYER:
 * @o: Object which is subject to casting.
 *
 * Casts a #DACPPlayer or derived pointer into a (DACPPlayer *) pointer.
 * Depending on the current debugging level, this function may invoke
 * certain runtime checks to identify invalid casts.
 */
#define DACP_PLAYER(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), DACP_TYPE_PLAYER, DACPPlayer))
/**
 * IS_DACP_PLAYER:
 * @o: Instance to check for being a %DACP_TYPE_PLAYER.
 *
 * Checks whether a valid #GTypeInstance pointer is of type %DACP_TYPE_PLAYER.
 */
#define IS_DACP_PLAYER(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), DACP_TYPE_PLAYER))
/**
 * DACP_PLAYER_GET_INTERFACE:
 * @o: a #DACPPlayer instance.
 *
 * Get the insterface structure associated to a #DACPPlayer instance.
 *
 * Returns: pointer to object interface structure.
 */
#define DACP_PLAYER_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), \
                                        DACP_TYPE_PLAYER, DACPPlayerIface))
typedef struct _DACPPlayerIface DACPPlayerIface;
typedef struct _DACPPlayer DACPPlayer;

typedef enum
{
	DACP_REPEAT_NONE = 0,
	DACP_REPEAT_SINGLE = 1,
	DACP_REPEAT_ALL = 2
} DACPRepeatState;

typedef enum
{
	DACP_PLAY_STOPPED = 2,
	DACP_PLAY_PAUSED = 3,
	DACP_PLAY_PLAYING = 4
} DACPPlayState;

struct _DACPPlayerIface
{
	GTypeInterface parent_class;

	DAAPRecord *(*now_playing_record) (DACPPlayer * player);
	guchar *(*now_playing_artwork) (DACPPlayer * player,
	                                guint width, guint height);
	void (*play_pause) (DACPPlayer * player);
	void (*pause) (DACPPlayer * player);
	void (*next_item) (DACPPlayer * player);
	void (*prev_item) (DACPPlayer * player);

	void (*cue_clear) (DACPPlayer * player);
	void (*cue_play) (DACPPlayer * player, GList * records, guint index);
};

GType dacp_player_get_type (void);

/**
 * dacp_player_now_playing_record:
 * @player: a player
 */
DAAPRecord *dacp_player_now_playing_record (DACPPlayer * player);

/**
 * dacp_player_now_playing_artwork:
 * @player: a player
 * @width: width
 * @height: height
 */
guchar *dacp_player_now_playing_artwork (DACPPlayer * player,
                                         guint width, guint height);

/**
 * dacp_player_play_pause:
 * @player: a player
 */
void dacp_player_play_pause (DACPPlayer * player);

/**
 * dacp_player_pause:
 * @player: a player
 */
void dacp_player_pause (DACPPlayer * player);

/**
 * dacp_player_next_item:
 * @player: a player
 */
void dacp_player_next_item (DACPPlayer * player);

/**
 * dacp_player_now_prev_item:
 * @player: a player
 */
void dacp_player_prev_item (DACPPlayer * player);

/**
 * dacp_player_cue_clear:
 * @player: a player
 */
void dacp_player_cue_clear (DACPPlayer * player);

/**
 * dacp_player_cue_play:
 * @player: a player
 * @records : a list of records
 * @index: an index
 */
void dacp_player_cue_play (DACPPlayer * player, GList * records, guint index);

G_END_DECLS
#endif /* _DACP_PLAYER_H_ */
