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

#ifndef _DACP_PLAYER_H_
#define _DACP_PLAYER_H_

#include <glib-object.h>

#include "daap-record.h"

G_BEGIN_DECLS

#define DACP_TYPE_PLAYER               (dacp_player_get_type ())
#define DACP_PLAYER(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), DACP_TYPE_PLAYER, DACPPlayer))
#define IS_DACP_PLAYER(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DACP_TYPE_PLAYER))
#define DACP_PLAYER_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), \
                                        DACP_TYPE_PLAYER, DACPPlayerIface))

typedef struct _DACPPlayerIface DACPPlayerIface;
typedef struct _DACPPlayer DACPPlayer;

/**
 * DACPRepeatState:
 */
typedef enum {
	REPEAT_NONE = 0,
	REPEAT_SINGLE = 1,
	REPEAT_ALL = 2
} DACPRepeatState;

/**
 * DACPPlayState:
 */
typedef enum {
	PLAY_STOPPED = 2,
	PLAY_PAUSED = 3,
	PLAY_PLAYING = 4
} DACPPlayState;

struct _DACPPlayerIface
{
	GTypeInterface parent_class;

	DAAPRecord *(*now_playing_record)  (DACPPlayer *player);
	gchar *(*now_playing_artwork)      (DACPPlayer *player, guint width, guint height);
	void (*play_pause)                 (DACPPlayer *player);
	void (*pause)                      (DACPPlayer *player);
	void (*next_item)                  (DACPPlayer *player);
	void (*prev_item)                  (DACPPlayer *player);

	void (*cue_clear)                  (DACPPlayer *player);
	void (*cue_play)                   (DACPPlayer *player, GList *records, guint index);
};

GType dacp_player_get_type (void);

DAAPRecord *dacp_player_now_playing_record  (DACPPlayer *player);
gchar      *dacp_player_now_playing_artwork (DACPPlayer *player, guint width, guint height);
void        dacp_player_play_pause          (DACPPlayer *player);
void        dacp_player_pause               (DACPPlayer *player);
void        dacp_player_next_item           (DACPPlayer *player);
void        dacp_player_prev_item           (DACPPlayer *player);

void        dacp_player_cue_clear           (DACPPlayer *player);
void        dacp_player_cue_play            (DACPPlayer *player, GList *records, guint index);

G_END_DECLS

#endif /* _DACP_PLAYER_H_ */
