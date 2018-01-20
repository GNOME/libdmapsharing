/* dacplisten.c generated by valac 0.38.4, the Vala compiler
 * generated from dacplisten.vala, do not modify */

/*   FILE: dacplisten.vala -- Listen to DACP remotes
 * AUTHOR: W. Michael Petullo <mike@flyn.org>
 *   DATE: 06 January 2011 
 *
 * Copyright (c) 2011 W. Michael Petullo <new@flyn.org>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <glib.h>
#include <glib-object.h>
#include <libdmapsharing/dmap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define TYPE_VALA_DACP_PLAYER (vala_dacp_player_get_type ())
#define VALA_DACP_PLAYER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_VALA_DACP_PLAYER, ValaDacpPlayer))
#define VALA_DACP_PLAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_VALA_DACP_PLAYER, ValaDacpPlayerClass))
#define IS_VALA_DACP_PLAYER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_VALA_DACP_PLAYER))
#define IS_VALA_DACP_PLAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_VALA_DACP_PLAYER))
#define VALA_DACP_PLAYER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_VALA_DACP_PLAYER, ValaDacpPlayerClass))

typedef struct _ValaDacpPlayer ValaDacpPlayer;
typedef struct _ValaDacpPlayerClass ValaDacpPlayerClass;
typedef struct _ValaDacpPlayerPrivate ValaDacpPlayerPrivate;
enum  {
	VALA_DACP_PLAYER_0_PROPERTY,
	VALA_DACP_PLAYER_NUM_PROPERTIES
};
static GParamSpec* vala_dacp_player_properties[VALA_DACP_PLAYER_NUM_PROPERTIES];

#define TYPE_DACP_LISTENER (dacp_listener_get_type ())
#define DACP_LISTENER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_DACP_LISTENER, DacpListener))
#define DACP_LISTENER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DACP_LISTENER, DacpListenerClass))
#define IS_DACP_LISTENER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_DACP_LISTENER))
#define IS_DACP_LISTENER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DACP_LISTENER))
#define DACP_LISTENER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DACP_LISTENER, DacpListenerClass))

typedef struct _DacpListener DacpListener;
typedef struct _DacpListenerClass DacpListenerClass;
typedef struct _DacpListenerPrivate DacpListenerPrivate;
enum  {
	DACP_LISTENER_0_PROPERTY,
	DACP_LISTENER_NUM_PROPERTIES
};
static GParamSpec* dacp_listener_properties[DACP_LISTENER_NUM_PROPERTIES];
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

#define TYPE_VALA_DMAP_DB (vala_dmap_db_get_type ())
#define VALA_DMAP_DB(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_VALA_DMAP_DB, ValaDmapDb))
#define VALA_DMAP_DB_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_VALA_DMAP_DB, ValaDmapDbClass))
#define IS_VALA_DMAP_DB(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_VALA_DMAP_DB))
#define IS_VALA_DMAP_DB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_VALA_DMAP_DB))
#define VALA_DMAP_DB_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_VALA_DMAP_DB, ValaDmapDbClass))

typedef struct _ValaDmapDb ValaDmapDb;
typedef struct _ValaDmapDbClass ValaDmapDbClass;

#define TYPE_VALA_DMAP_CONTAINER_DB (vala_dmap_container_db_get_type ())
#define VALA_DMAP_CONTAINER_DB(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_VALA_DMAP_CONTAINER_DB, ValaDmapContainerDb))
#define VALA_DMAP_CONTAINER_DB_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_VALA_DMAP_CONTAINER_DB, ValaDmapContainerDbClass))
#define IS_VALA_DMAP_CONTAINER_DB(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_VALA_DMAP_CONTAINER_DB))
#define IS_VALA_DMAP_CONTAINER_DB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_VALA_DMAP_CONTAINER_DB))
#define VALA_DMAP_CONTAINER_DB_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_VALA_DMAP_CONTAINER_DB, ValaDmapContainerDbClass))

typedef struct _ValaDmapContainerDb ValaDmapContainerDb;
typedef struct _ValaDmapContainerDbClass ValaDmapContainerDbClass;
#define _g_main_loop_unref0(var) ((var == NULL) ? NULL : (var = (g_main_loop_unref (var), NULL)))

struct _ValaDacpPlayer {
	GObject parent_instance;
	ValaDacpPlayerPrivate * priv;
};

struct _ValaDacpPlayerClass {
	GObjectClass parent_class;
};

struct _DacpListener {
	GObject parent_instance;
	DacpListenerPrivate * priv;
};

struct _DacpListenerClass {
	GObjectClass parent_class;
};

struct _DacpListenerPrivate {
	DmapDb* db;
	DmapContainerDb* container_db;
	DmapControlPlayer* player;
	DmapControlShare* share;
};


static gpointer vala_dacp_player_parent_class = NULL;
static DmapControlPlayerInterface * vala_dacp_player_dmap_control_player_parent_iface = NULL;
static gpointer dacp_listener_parent_class = NULL;

GType vala_dacp_player_get_type (void) G_GNUC_CONST;
static DmapAvRecord* vala_dacp_player_real_now_playing_record (DmapControlPlayer* base);
static guchar* vala_dacp_player_real_now_playing_artwork (DmapControlPlayer* base, guint width, guint height, int* result_length1);
static void vala_dacp_player_real_play_pause (DmapControlPlayer* base);
static void vala_dacp_player_real_pause (DmapControlPlayer* base);
static void vala_dacp_player_real_next_item (DmapControlPlayer* base);
static void vala_dacp_player_real_prev_item (DmapControlPlayer* base);
static void vala_dacp_player_real_cue_clear (DmapControlPlayer* base);
static void vala_dacp_player_real_cue_play (DmapControlPlayer* base, GList* records, guint index);
ValaDacpPlayer* vala_dacp_player_new (void);
ValaDacpPlayer* vala_dacp_player_construct (GType object_type);
GType dacp_listener_get_type (void) G_GNUC_CONST;
#define DACP_LISTENER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_DACP_LISTENER, DacpListenerPrivate))
DacpListener* dacp_listener_new (void);
DacpListener* dacp_listener_construct (GType object_type);
ValaDmapDb* vala_dmap_db_new (void);
ValaDmapDb* vala_dmap_db_construct (GType object_type);
GType vala_dmap_db_get_type (void) G_GNUC_CONST;
ValaDmapContainerDb* vala_dmap_container_db_new (void);
ValaDmapContainerDb* vala_dmap_container_db_construct (GType object_type);
GType vala_dmap_container_db_get_type (void) G_GNUC_CONST;
static void __lambda4_ (DacpListener* self, const gchar* service_name, const gchar* display_name);
static void ___lambda4__dmap_control_share_remote_found (DmapControlShare* _sender, const gchar* service_name, const gchar* remote_name, gpointer self);
static void __lambda5_ (DacpListener* self, const gchar* guid);
static void ___lambda5__dmap_control_share_add_guid (DmapControlShare* _sender, const gchar* guid, gpointer self);
static void dacp_listener_finalize (GObject * obj);
gint _vala_main (gchar** args, int args_length1);


static DmapAvRecord* vala_dacp_player_real_now_playing_record (DmapControlPlayer* base) {
	ValaDacpPlayer * self;
	DmapAvRecord* result = NULL;
	FILE* _tmp0_;
	self = (ValaDacpPlayer*) base;
	_tmp0_ = stdout;
	fprintf (_tmp0_, "Now playing record request received\n");
	result = NULL;
	return result;
}


static guchar* vala_dacp_player_real_now_playing_artwork (DmapControlPlayer* base, guint width, guint height, int* result_length1) {
	ValaDacpPlayer * self;
	guchar* result = NULL;
	FILE* _tmp0_;
	guchar* _tmp1_;
	gint _tmp1__length1;
	self = (ValaDacpPlayer*) base;
	_tmp0_ = stdout;
	fprintf (_tmp0_, "Now playing artwork request received\n");
	_tmp1_ = NULL;
	_tmp1__length1 = 0;
	if (result_length1) {
		*result_length1 = _tmp1__length1;
	}
	result = _tmp1_;
	return result;
}


static void vala_dacp_player_real_play_pause (DmapControlPlayer* base) {
	ValaDacpPlayer * self;
	FILE* _tmp0_;
	self = (ValaDacpPlayer*) base;
	_tmp0_ = stdout;
	fprintf (_tmp0_, "Play/pause request received\n");
}


static void vala_dacp_player_real_pause (DmapControlPlayer* base) {
	ValaDacpPlayer * self;
	FILE* _tmp0_;
	self = (ValaDacpPlayer*) base;
	_tmp0_ = stdout;
	fprintf (_tmp0_, "Pause request received\n");
}


static void vala_dacp_player_real_next_item (DmapControlPlayer* base) {
	ValaDacpPlayer * self;
	FILE* _tmp0_;
	self = (ValaDacpPlayer*) base;
	_tmp0_ = stdout;
	fprintf (_tmp0_, "Next item request received\n");
}


static void vala_dacp_player_real_prev_item (DmapControlPlayer* base) {
	ValaDacpPlayer * self;
	FILE* _tmp0_;
	self = (ValaDacpPlayer*) base;
	_tmp0_ = stdout;
	fprintf (_tmp0_, "Previous item request received\n");
}


static void vala_dacp_player_real_cue_clear (DmapControlPlayer* base) {
	ValaDacpPlayer * self;
	FILE* _tmp0_;
	self = (ValaDacpPlayer*) base;
	_tmp0_ = stdout;
	fprintf (_tmp0_, "Cue clear request received\n");
}


static void vala_dacp_player_real_cue_play (DmapControlPlayer* base, GList* records, guint index) {
	ValaDacpPlayer * self;
	FILE* _tmp0_;
	self = (ValaDacpPlayer*) base;
	_tmp0_ = stdout;
	fprintf (_tmp0_, "Cue play request received\n");
}


ValaDacpPlayer* vala_dacp_player_construct (GType object_type) {
	ValaDacpPlayer * self = NULL;
	self = (ValaDacpPlayer*) g_object_new (object_type, NULL);
	return self;
}


ValaDacpPlayer* vala_dacp_player_new (void) {
	return vala_dacp_player_construct (TYPE_VALA_DACP_PLAYER);
}


static void vala_dacp_player_class_init (ValaDacpPlayerClass * klass) {
	vala_dacp_player_parent_class = g_type_class_peek_parent (klass);
}


static void vala_dacp_player_dmap_control_player_interface_init (DmapControlPlayerInterface * iface) {
	vala_dacp_player_dmap_control_player_parent_iface = g_type_interface_peek_parent (iface);
	iface->now_playing_record = (DmapAvRecord* (*) (DmapControlPlayer *)) vala_dacp_player_real_now_playing_record;
	iface->now_playing_artwork = (guchar* (*) (DmapControlPlayer *, guint, guint, int*)) vala_dacp_player_real_now_playing_artwork;
	iface->play_pause = (void (*) (DmapControlPlayer *)) vala_dacp_player_real_play_pause;
	iface->pause = (void (*) (DmapControlPlayer *)) vala_dacp_player_real_pause;
	iface->next_item = (void (*) (DmapControlPlayer *)) vala_dacp_player_real_next_item;
	iface->prev_item = (void (*) (DmapControlPlayer *)) vala_dacp_player_real_prev_item;
	iface->cue_clear = (void (*) (DmapControlPlayer *)) vala_dacp_player_real_cue_clear;
	iface->cue_play = (void (*) (DmapControlPlayer *, GList*, guint)) vala_dacp_player_real_cue_play;
}


static void vala_dacp_player_instance_init (ValaDacpPlayer * self) {
}


GType vala_dacp_player_get_type (void) {
	static volatile gsize vala_dacp_player_type_id__volatile = 0;
	if (g_once_init_enter (&vala_dacp_player_type_id__volatile)) {
		static const GTypeInfo g_define_type_info = { sizeof (ValaDacpPlayerClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) vala_dacp_player_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (ValaDacpPlayer), 0, (GInstanceInitFunc) vala_dacp_player_instance_init, NULL };
		static const GInterfaceInfo dmap_control_player_info = { (GInterfaceInitFunc) vala_dacp_player_dmap_control_player_interface_init, (GInterfaceFinalizeFunc) NULL, NULL};
		GType vala_dacp_player_type_id;
		vala_dacp_player_type_id = g_type_register_static (G_TYPE_OBJECT, "ValaDacpPlayer", &g_define_type_info, 0);
		g_type_add_interface_static (vala_dacp_player_type_id, DMAP_TYPE_CONTROL_PLAYER, &dmap_control_player_info);
		g_once_init_leave (&vala_dacp_player_type_id__volatile, vala_dacp_player_type_id);
	}
	return vala_dacp_player_type_id__volatile;
}


static void __lambda4_ (DacpListener* self, const gchar* service_name, const gchar* display_name) {
	FILE* _tmp0_;
	const gchar* _tmp1_;
	const gchar* _tmp2_;
	g_return_if_fail (service_name != NULL);
	g_return_if_fail (display_name != NULL);
	_tmp0_ = stdout;
	_tmp1_ = service_name;
	_tmp2_ = display_name;
	fprintf (_tmp0_, "Found remote: %s, %s\n", _tmp1_, _tmp2_);
}


static void ___lambda4__dmap_control_share_remote_found (DmapControlShare* _sender, const gchar* service_name, const gchar* remote_name, gpointer self) {
	__lambda4_ ((DacpListener*) self, service_name, remote_name);
}


static void __lambda5_ (DacpListener* self, const gchar* guid) {
	FILE* _tmp0_;
	g_return_if_fail (guid != NULL);
	_tmp0_ = stdout;
	fprintf (_tmp0_, "Add GUID request received\n");
}


static void ___lambda5__dmap_control_share_add_guid (DmapControlShare* _sender, const gchar* guid, gpointer self) {
	__lambda5_ ((DacpListener*) self, guid);
}


DacpListener* dacp_listener_construct (GType object_type) {
	DacpListener * self = NULL;
	ValaDmapDb* _tmp0_;
	ValaDmapContainerDb* _tmp1_;
	ValaDacpPlayer* _tmp2_;
	DmapControlPlayer* _tmp3_;
	DmapDb* _tmp4_;
	DmapContainerDb* _tmp5_;
	DmapControlShare* _tmp6_;
	DmapControlShare* _tmp7_;
	DmapControlShare* _tmp8_;
	DmapControlShare* _tmp9_;
	self = (DacpListener*) g_object_new (object_type, NULL);
	_tmp0_ = vala_dmap_db_new ();
	_g_object_unref0 (self->priv->db);
	self->priv->db = (DmapDb*) _tmp0_;
	_tmp1_ = vala_dmap_container_db_new ();
	_g_object_unref0 (self->priv->container_db);
	self->priv->container_db = (DmapContainerDb*) _tmp1_;
	_tmp2_ = vala_dacp_player_new ();
	_g_object_unref0 (self->priv->player);
	self->priv->player = (DmapControlPlayer*) _tmp2_;
	_tmp3_ = self->priv->player;
	_tmp4_ = self->priv->db;
	_tmp5_ = self->priv->container_db;
	_tmp6_ = dmap_control_share_new ("dacplisten", _tmp3_, _tmp4_, _tmp5_);
	_g_object_unref0 (self->priv->share);
	self->priv->share = _tmp6_;
	_tmp7_ = self->priv->share;
	g_signal_connect_object (_tmp7_, "remote-found", (GCallback) ___lambda4__dmap_control_share_remote_found, self, 0);
	_tmp8_ = self->priv->share;
	g_signal_connect_object (_tmp8_, "add-guid", (GCallback) ___lambda5__dmap_control_share_add_guid, self, 0);
	_tmp9_ = self->priv->share;
	dmap_control_share_start_lookup (_tmp9_);
	return self;
}


DacpListener* dacp_listener_new (void) {
	return dacp_listener_construct (TYPE_DACP_LISTENER);
}


static void dacp_listener_class_init (DacpListenerClass * klass) {
	dacp_listener_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (DacpListenerPrivate));
	G_OBJECT_CLASS (klass)->finalize = dacp_listener_finalize;
}


static void dacp_listener_instance_init (DacpListener * self) {
	self->priv = DACP_LISTENER_GET_PRIVATE (self);
}


static void dacp_listener_finalize (GObject * obj) {
	DacpListener * self;
	self = G_TYPE_CHECK_INSTANCE_CAST (obj, TYPE_DACP_LISTENER, DacpListener);
	_g_object_unref0 (self->priv->db);
	_g_object_unref0 (self->priv->container_db);
	_g_object_unref0 (self->priv->player);
	_g_object_unref0 (self->priv->share);
	G_OBJECT_CLASS (dacp_listener_parent_class)->finalize (obj);
}


GType dacp_listener_get_type (void) {
	static volatile gsize dacp_listener_type_id__volatile = 0;
	if (g_once_init_enter (&dacp_listener_type_id__volatile)) {
		static const GTypeInfo g_define_type_info = { sizeof (DacpListenerClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) dacp_listener_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (DacpListener), 0, (GInstanceInitFunc) dacp_listener_instance_init, NULL };
		GType dacp_listener_type_id;
		dacp_listener_type_id = g_type_register_static (G_TYPE_OBJECT, "DacpListener", &g_define_type_info, 0);
		g_once_init_leave (&dacp_listener_type_id__volatile, dacp_listener_type_id);
	}
	return dacp_listener_type_id__volatile;
}


gint _vala_main (gchar** args, int args_length1) {
	gint result = 0;
	GMainLoop* loop = NULL;
	GMainLoop* _tmp0_;
	DacpListener* dacplistener = NULL;
	DacpListener* _tmp1_;
	_tmp0_ = g_main_loop_new (NULL, FALSE);
	loop = _tmp0_;
	_tmp1_ = dacp_listener_new ();
	dacplistener = _tmp1_;
	g_main_loop_run (loop);
	result = 0;
	_g_object_unref0 (dacplistener);
	_g_main_loop_unref0 (loop);
	return result;
}


int main (int argc, char ** argv) {
	return _vala_main (argv, argc);
}



