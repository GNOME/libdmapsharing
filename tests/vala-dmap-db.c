/* vala-dmap-db.c generated by valac 0.22.1, the Vala compiler
 * generated from vala-dmap-db.vala, do not modify */

/*   FILE: vala-dmap-db.vala -- A DMAPDb implementation in Vala
 * AUTHOR: W. Michael Petullo <mike@flyn.org>
 *   DATE: 21 December 2010 
 *
 * Copyright (c) 2010 W. Michael Petullo <new@flyn.org>
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
#include <gee.h>
#include <stdlib.h>
#include <string.h>


#define TYPE_VALA_DMAP_DB (vala_dmap_db_get_type ())
#define VALA_DMAP_DB(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_VALA_DMAP_DB, ValaDMAPDb))
#define VALA_DMAP_DB_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_VALA_DMAP_DB, ValaDMAPDbClass))
#define IS_VALA_DMAP_DB(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_VALA_DMAP_DB))
#define IS_VALA_DMAP_DB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_VALA_DMAP_DB))
#define VALA_DMAP_DB_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_VALA_DMAP_DB, ValaDMAPDbClass))

typedef struct _ValaDMAPDb ValaDMAPDb;
typedef struct _ValaDMAPDbClass ValaDMAPDbClass;
typedef struct _ValaDMAPDbPrivate ValaDMAPDbPrivate;
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

struct _ValaDMAPDb {
	GObject parent_instance;
	ValaDMAPDbPrivate * priv;
};

struct _ValaDMAPDbClass {
	GObjectClass parent_class;
};

struct _ValaDMAPDbPrivate {
	GeeArrayList* db;
};


static gpointer vala_dmap_db_parent_class = NULL;
static DMAPDbIface* vala_dmap_db_dmap_db_parent_iface = NULL;

GType vala_dmap_db_get_type (void) G_GNUC_CONST;
#define VALA_DMAP_DB_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_VALA_DMAP_DB, ValaDMAPDbPrivate))
enum  {
	VALA_DMAP_DB_DUMMY_PROPERTY
};
static guint vala_dmap_db_real_add (DMAPDb* base, DMAPRecord* record);
static guint vala_dmap_db_real_add_path (DMAPDb* base, const gchar* path);
static guint vala_dmap_db_real_add_with_id (DMAPDb* base, DMAPRecord* record, guint id);
static gint64 vala_dmap_db_real_count (DMAPDb* base);
static void vala_dmap_db_real_foreach (DMAPDb* base, GHFunc func, void* func_target);
static DMAPRecord* vala_dmap_db_real_lookup_by_id (DMAPDb* base, guint id);
static guint vala_dmap_db_real_lookup_id_by_location (DMAPDb* base, const gchar* location);
ValaDMAPDb* vala_dmap_db_new (void);
ValaDMAPDb* vala_dmap_db_construct (GType object_type);
static void vala_dmap_db_finalize (GObject* obj);


static guint vala_dmap_db_real_add (DMAPDb* base, DMAPRecord* record) {
	ValaDMAPDb * self;
	guint result = 0U;
	GeeArrayList* _tmp0_ = NULL;
	DMAPRecord* _tmp1_ = NULL;
	GeeArrayList* _tmp2_ = NULL;
	gint _tmp3_ = 0;
	gint _tmp4_ = 0;
	self = (ValaDMAPDb*) base;
	g_return_val_if_fail (record != NULL, 0U);
	_tmp0_ = self->priv->db;
	_tmp1_ = record;
	gee_abstract_collection_add ((GeeAbstractCollection*) _tmp0_, G_TYPE_CHECK_INSTANCE_CAST (_tmp1_, DMAP_TYPE_RECORD, DMAPRecord));
	_tmp2_ = self->priv->db;
	_tmp3_ = gee_abstract_collection_get_size ((GeeCollection*) _tmp2_);
	_tmp4_ = _tmp3_;
	result = (guint) _tmp4_;
	return result;
}


static guint vala_dmap_db_real_add_path (DMAPDb* base, const gchar* path) {
	ValaDMAPDb * self;
	guint result = 0U;
	self = (ValaDMAPDb*) base;
	g_return_val_if_fail (path != NULL, 0U);
	g_error ("vala-dmap-db.vala:35: add_path not implemented");
	return result;
}


static guint vala_dmap_db_real_add_with_id (DMAPDb* base, DMAPRecord* record, guint id) {
	ValaDMAPDb * self;
	guint result = 0U;
	self = (ValaDMAPDb*) base;
	g_return_val_if_fail (record != NULL, 0U);
	g_error ("vala-dmap-db.vala:39: add_with_id not implemented");
	return result;
}


static gint64 vala_dmap_db_real_count (DMAPDb* base) {
	ValaDMAPDb * self;
	gint64 result = 0LL;
	GeeArrayList* _tmp0_ = NULL;
	gint _tmp1_ = 0;
	gint _tmp2_ = 0;
	self = (ValaDMAPDb*) base;
	_tmp0_ = self->priv->db;
	_tmp1_ = gee_abstract_collection_get_size ((GeeCollection*) _tmp0_);
	_tmp2_ = _tmp1_;
	result = (gint64) _tmp2_;
	return result;
}


static void vala_dmap_db_real_foreach (DMAPDb* base, GHFunc func, void* func_target) {
	ValaDMAPDb * self;
	gint i = 0;
	self = (ValaDMAPDb*) base;
	{
		gboolean _tmp0_ = FALSE;
		i = 0;
		_tmp0_ = TRUE;
		while (TRUE) {
			gboolean _tmp1_ = FALSE;
			gint _tmp3_ = 0;
			GeeArrayList* _tmp4_ = NULL;
			gint _tmp5_ = 0;
			gint _tmp6_ = 0;
			GHFunc _tmp7_ = NULL;
			void* _tmp7__target = NULL;
			gint _tmp8_ = 0;
			void* _tmp9_ = NULL;
			GeeArrayList* _tmp10_ = NULL;
			gint _tmp11_ = 0;
			gpointer _tmp12_ = NULL;
			_tmp1_ = _tmp0_;
			if (!_tmp1_) {
				gint _tmp2_ = 0;
				_tmp2_ = i;
				i = _tmp2_ + 1;
			}
			_tmp0_ = FALSE;
			_tmp3_ = i;
			_tmp4_ = self->priv->db;
			_tmp5_ = gee_abstract_collection_get_size ((GeeCollection*) _tmp4_);
			_tmp6_ = _tmp5_;
			if (!(_tmp3_ < _tmp6_)) {
				break;
			}
			_tmp7_ = func;
			_tmp7__target = func_target;
			_tmp8_ = i;
			_tmp9_ = GINT_TO_POINTER (_tmp8_ + 1);
			_tmp10_ = self->priv->db;
			_tmp11_ = i;
			_tmp12_ = gee_abstract_list_get ((GeeAbstractList*) _tmp10_, _tmp11_);
			_tmp7_ (_tmp9_, (DMAPRecord*) _tmp12_, _tmp7__target);
		}
	}
}


static DMAPRecord* vala_dmap_db_real_lookup_by_id (DMAPDb* base, guint id) {
	ValaDMAPDb * self;
	DMAPRecord* result = NULL;
	GeeArrayList* _tmp0_ = NULL;
	guint _tmp1_ = 0U;
	gpointer _tmp2_ = NULL;
	self = (ValaDMAPDb*) base;
	_tmp0_ = self->priv->db;
	_tmp1_ = id;
	_tmp2_ = gee_abstract_list_get ((GeeAbstractList*) _tmp0_, ((gint) _tmp1_) - 1);
	result = (DMAPRecord*) _tmp2_;
	return result;
}


static guint vala_dmap_db_real_lookup_id_by_location (DMAPDb* base, const gchar* location) {
	ValaDMAPDb * self;
	guint result = 0U;
	self = (ValaDMAPDb*) base;
	g_return_val_if_fail (location != NULL, 0U);
	g_error ("vala-dmap-db.vala:63: lookup_id_by_location not implemented");
	return result;
}


ValaDMAPDb* vala_dmap_db_construct (GType object_type) {
	ValaDMAPDb * self = NULL;
	self = (ValaDMAPDb*) g_object_new (object_type, NULL);
	return self;
}


ValaDMAPDb* vala_dmap_db_new (void) {
	return vala_dmap_db_construct (TYPE_VALA_DMAP_DB);
}


static void vala_dmap_db_class_init (ValaDMAPDbClass * klass) {
	vala_dmap_db_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (ValaDMAPDbPrivate));
	G_OBJECT_CLASS (klass)->finalize = vala_dmap_db_finalize;
}


static void vala_dmap_db_dmap_db_interface_init (DMAPDbIface * iface) {
	vala_dmap_db_dmap_db_parent_iface = g_type_interface_peek_parent (iface);
	iface->add = (guint (*)(DMAPDb*, DMAPRecord*)) vala_dmap_db_real_add;
	iface->add_path = (guint (*)(DMAPDb*, const gchar*)) vala_dmap_db_real_add_path;
	iface->add_with_id = (guint (*)(DMAPDb*, DMAPRecord*, guint)) vala_dmap_db_real_add_with_id;
	iface->count = (gint64 (*)(DMAPDb*)) vala_dmap_db_real_count;
	iface->foreach = (void (*)(DMAPDb*, GHFunc, void*)) vala_dmap_db_real_foreach;
	iface->lookup_by_id = (DMAPRecord* (*)(DMAPDb*, guint)) vala_dmap_db_real_lookup_by_id;
	iface->lookup_id_by_location = (guint (*)(DMAPDb*, const gchar*)) vala_dmap_db_real_lookup_id_by_location;
}


static void vala_dmap_db_instance_init (ValaDMAPDb * self) {
	GeeArrayList* _tmp0_ = NULL;
	self->priv = VALA_DMAP_DB_GET_PRIVATE (self);
	_tmp0_ = gee_array_list_new (DMAP_TYPE_RECORD, (GBoxedCopyFunc) g_object_ref, g_object_unref, NULL, NULL, NULL);
	self->priv->db = _tmp0_;
}


static void vala_dmap_db_finalize (GObject* obj) {
	ValaDMAPDb * self;
	self = G_TYPE_CHECK_INSTANCE_CAST (obj, TYPE_VALA_DMAP_DB, ValaDMAPDb);
	_g_object_unref0 (self->priv->db);
	G_OBJECT_CLASS (vala_dmap_db_parent_class)->finalize (obj);
}


GType vala_dmap_db_get_type (void) {
	static volatile gsize vala_dmap_db_type_id__volatile = 0;
	if (g_once_init_enter (&vala_dmap_db_type_id__volatile)) {
		static const GTypeInfo g_define_type_info = { sizeof (ValaDMAPDbClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) vala_dmap_db_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (ValaDMAPDb), 0, (GInstanceInitFunc) vala_dmap_db_instance_init, NULL };
		static const GInterfaceInfo dmap_db_info = { (GInterfaceInitFunc) vala_dmap_db_dmap_db_interface_init, (GInterfaceFinalizeFunc) NULL, NULL};
		GType vala_dmap_db_type_id;
		vala_dmap_db_type_id = g_type_register_static (G_TYPE_OBJECT, "ValaDMAPDb", &g_define_type_info, 0);
		g_type_add_interface_static (vala_dmap_db_type_id, DMAP_TYPE_DB, &dmap_db_info);
		g_once_init_leave (&vala_dmap_db_type_id__volatile, vala_dmap_db_type_id);
	}
	return vala_dmap_db_type_id__volatile;
}



