/* dmapcopy.c generated by valac 0.48.6, the Vala compiler
 * generated from dmapcopy.vala, do not modify */

/*   FILE: dmapcopy.vala -- Copy files from a DMAP server
 * AUTHOR: W. Michael Petullo <mike@flyn.org>
 *   DATE: 20 December 2010 
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

#include <glib-object.h>
#include <libdmapsharing/dmap.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gobject/gvaluecollector.h>

#define TYPE_DPAP_COPY (dpap_copy_get_type ())
#define DPAP_COPY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_DPAP_COPY, DPAPCopy))
#define DPAP_COPY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DPAP_COPY, DPAPCopyClass))
#define IS_DPAP_COPY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_DPAP_COPY))
#define IS_DPAP_COPY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DPAP_COPY))
#define DPAP_COPY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DPAP_COPY, DPAPCopyClass))

typedef struct _DPAPCopy DPAPCopy;
typedef struct _DPAPCopyClass DPAPCopyClass;
typedef struct _DPAPCopyPrivate DPAPCopyPrivate;

#define TYPE_VALA_DMAP_DB (vala_dmap_db_get_type ())
#define VALA_DMAP_DB(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_VALA_DMAP_DB, ValaDMAPDb))
#define VALA_DMAP_DB_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_VALA_DMAP_DB, ValaDMAPDbClass))
#define IS_VALA_DMAP_DB(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_VALA_DMAP_DB))
#define IS_VALA_DMAP_DB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_VALA_DMAP_DB))
#define VALA_DMAP_DB_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_VALA_DMAP_DB, ValaDMAPDbClass))

typedef struct _ValaDMAPDb ValaDMAPDb;
typedef struct _ValaDMAPDbClass ValaDMAPDbClass;

#define TYPE_VALA_DPAP_RECORD_FACTORY (vala_dpap_record_factory_get_type ())
#define VALA_DPAP_RECORD_FACTORY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_VALA_DPAP_RECORD_FACTORY, ValaDPAPRecordFactory))
#define VALA_DPAP_RECORD_FACTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_VALA_DPAP_RECORD_FACTORY, ValaDPAPRecordFactoryClass))
#define IS_VALA_DPAP_RECORD_FACTORY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_VALA_DPAP_RECORD_FACTORY))
#define IS_VALA_DPAP_RECORD_FACTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_VALA_DPAP_RECORD_FACTORY))
#define VALA_DPAP_RECORD_FACTORY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_VALA_DPAP_RECORD_FACTORY, ValaDPAPRecordFactoryClass))

typedef struct _ValaDPAPRecordFactory ValaDPAPRecordFactory;
typedef struct _ValaDPAPRecordFactoryClass ValaDPAPRecordFactoryClass;
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

#define TYPE_VALA_DPAP_RECORD (vala_dpap_record_get_type ())
#define VALA_DPAP_RECORD(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_VALA_DPAP_RECORD, ValaDPAPRecord))
#define VALA_DPAP_RECORD_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_VALA_DPAP_RECORD, ValaDPAPRecordClass))
#define IS_VALA_DPAP_RECORD(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_VALA_DPAP_RECORD))
#define IS_VALA_DPAP_RECORD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_VALA_DPAP_RECORD))
#define VALA_DPAP_RECORD_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_VALA_DPAP_RECORD, ValaDPAPRecordClass))

typedef struct _ValaDPAPRecord ValaDPAPRecord;
typedef struct _ValaDPAPRecordClass ValaDPAPRecordClass;
#define _dpap_copy_unref0(var) ((var == NULL) ? NULL : (var = (dpap_copy_unref (var), NULL)))
typedef struct _ParamSpecDPAPCopy ParamSpecDPAPCopy;
#define _g_main_loop_unref0(var) ((var == NULL) ? NULL : (var = (g_main_loop_unref (var), NULL)))

struct _DPAPCopy {
	GTypeInstance parent_instance;
	volatile int ref_count;
	DPAPCopyPrivate * priv;
};

struct _DPAPCopyClass {
	GTypeClass parent_class;
	void (*finalize) (DPAPCopy *self);
};

struct _DPAPCopyPrivate {
	DMAPMdnsBrowser* browser;
	DMAPConnection* connection;
	ValaDMAPDb* db;
	ValaDPAPRecordFactory* factory;
};

struct _ParamSpecDPAPCopy {
	GParamSpec parent_instance;
};

static gint DPAPCopy_private_offset;
static gpointer dpap_copy_parent_class = NULL;

gpointer dpap_copy_ref (gpointer instance);
void dpap_copy_unref (gpointer instance);
GParamSpec* param_spec_dpap_copy (const gchar* name,
                                  const gchar* nick,
                                  const gchar* blurb,
                                  GType object_type,
                                  GParamFlags flags);
void value_set_dpap_copy (GValue* value,
                          gpointer v_object);
void value_take_dpap_copy (GValue* value,
                           gpointer v_object);
gpointer value_get_dpap_copy (const GValue* value);
GType dpap_copy_get_type (void) G_GNUC_CONST;
G_DEFINE_AUTOPTR_CLEANUP_FUNC (DPAPCopy, dpap_copy_unref)
GType vala_dmap_db_get_type (void) G_GNUC_CONST;
G_DEFINE_AUTOPTR_CLEANUP_FUNC (ValaDMAPDb, g_object_unref)
GType vala_dpap_record_factory_get_type (void) G_GNUC_CONST;
G_DEFINE_AUTOPTR_CLEANUP_FUNC (ValaDPAPRecordFactory, g_object_unref)
static gboolean dpap_copy_connected_cb (DPAPCopy* self,
                                 DMAPConnection* connection,
                                 gboolean _result_,
                                 const gchar* reason);
static void __lambda4_ (DPAPCopy* self,
                 gconstpointer k,
                 gconstpointer v);
GType vala_dpap_record_get_type (void) G_GNUC_CONST;
G_DEFINE_AUTOPTR_CLEANUP_FUNC (ValaDPAPRecord, g_object_unref)
const gchar* vala_dpap_record_get_location (ValaDPAPRecord* self);
static void ___lambda4__gh_func (gconstpointer key,
                          gconstpointer value,
                          gpointer self);
static void dpap_copy_service_added_cb (DPAPCopy* self,
                                 DMAPMdnsBrowserService* service);
static gboolean _dpap_copy_connected_cb_dmap_connection_callback (DMAPConnection* connection,
                                                           gboolean _result_,
                                                           const gchar* reason,
                                                           gpointer self);
DPAPCopy* dpap_copy_new (GError** error);
DPAPCopy* dpap_copy_construct (GType object_type,
                               GError** error);
ValaDMAPDb* vala_dmap_db_new (void);
ValaDMAPDb* vala_dmap_db_construct (GType object_type);
ValaDPAPRecordFactory* vala_dpap_record_factory_new (void);
ValaDPAPRecordFactory* vala_dpap_record_factory_construct (GType object_type);
static void _dpap_copy_service_added_cb_dmap_mdns_browser_service_added (DMAPMdnsBrowser* _sender,
                                                                  void* service,
                                                                  gpointer self);
static void dpap_copy_finalize (DPAPCopy * obj);
static GType dpap_copy_get_type_once (void);
void debug_printf (const gchar* log_domain,
                   GLogLevelFlags log_level,
                   const gchar* message);
void debug_null (const gchar* log_domain,
                 GLogLevelFlags log_level,
                 const gchar* message);
gint _vala_main (gchar** args,
                 gint args_length1);
static void _debug_null_glog_func (const gchar* log_domain,
                            GLogLevelFlags log_levels,
                            const gchar* message,
                            gpointer self);

static inline gpointer
dpap_copy_get_instance_private (DPAPCopy* self)
{
	return G_STRUCT_MEMBER_P (self, DPAPCopy_private_offset);
}

static void
__lambda4_ (DPAPCopy* self,
            gconstpointer k,
            gconstpointer v)
{
	FILE* _tmp0_;
	const gchar* _tmp1_;
	const gchar* _tmp2_;
	_tmp0_ = stdout;
	_tmp1_ = vala_dpap_record_get_location (G_TYPE_CHECK_INSTANCE_CAST (v, TYPE_VALA_DPAP_RECORD, ValaDPAPRecord));
	_tmp2_ = _tmp1_;
	fprintf (_tmp0_, "%s\n", _tmp2_);
}

static void
___lambda4__gh_func (gconstpointer key,
                     gconstpointer value,
                     gpointer self)
{
	__lambda4_ ((DPAPCopy*) self, key, value);
}

static gboolean
dpap_copy_connected_cb (DPAPCopy* self,
                        DMAPConnection* connection,
                        gboolean _result_,
                        const gchar* reason)
{
	ValaDMAPDb* _tmp0_;
	ValaDMAPDb* _tmp1_;
	gboolean result = FALSE;
	g_return_val_if_fail (self != NULL, FALSE);
	g_return_val_if_fail (connection != NULL, FALSE);
	_tmp0_ = self->priv->db;
	g_debug ("%" G_GINT64_FORMAT " entries\n", dmap_db_count ((DMAPDb*) _tmp0_));
	_tmp1_ = self->priv->db;
	dmap_db_foreach ((DMAPDb*) _tmp1_, ___lambda4__gh_func, self);
	result = TRUE;
	return result;
}

static gboolean
_dpap_copy_connected_cb_dmap_connection_callback (DMAPConnection* connection,
                                                  gboolean _result_,
                                                  const gchar* reason,
                                                  gpointer self)
{
	gboolean result;
	result = dpap_copy_connected_cb ((DPAPCopy*) self, connection, _result_, reason);
	return result;
}

static void
dpap_copy_service_added_cb (DPAPCopy* self,
                            DMAPMdnsBrowserService* service)
{
	const gchar* _tmp0_;
	const gchar* _tmp1_;
	ValaDMAPDb* _tmp2_;
	ValaDPAPRecordFactory* _tmp3_;
	DPAPConnection* _tmp4_;
	DMAPConnection* _tmp5_;
	g_return_if_fail (self != NULL);
	_tmp0_ = service->service_name;
	_tmp1_ = service->host;
	_tmp2_ = self->priv->db;
	_tmp3_ = self->priv->factory;
	_tmp4_ = dpap_connection_new (_tmp0_, _tmp1_, service->port, (DMAPDb*) _tmp2_, (DMAPRecordFactory*) _tmp3_);
	_g_object_unref0 (self->priv->connection);
	self->priv->connection = G_TYPE_CHECK_INSTANCE_CAST (_tmp4_, DMAP_TYPE_CONNECTION, DMAPConnection);
	_tmp5_ = self->priv->connection;
	dmap_connection_connect (_tmp5_, _dpap_copy_connected_cb_dmap_connection_callback, self);
}

static void
_dpap_copy_service_added_cb_dmap_mdns_browser_service_added (DMAPMdnsBrowser* _sender,
                                                             void* service,
                                                             gpointer self)
{
	dpap_copy_service_added_cb ((DPAPCopy*) self, service);
}

DPAPCopy*
dpap_copy_construct (GType object_type,
                     GError** error)
{
	DPAPCopy* self = NULL;
	ValaDMAPDb* _tmp0_;
	ValaDPAPRecordFactory* _tmp1_;
	DMAPMdnsBrowser* _tmp2_;
	DMAPMdnsBrowser* _tmp3_;
	DMAPMdnsBrowser* _tmp4_;
	GError* _inner_error0_ = NULL;
	self = (DPAPCopy*) g_type_create_instance (object_type);
	_tmp0_ = vala_dmap_db_new ();
	_g_object_unref0 (self->priv->db);
	self->priv->db = _tmp0_;
	_tmp1_ = vala_dpap_record_factory_new ();
	_g_object_unref0 (self->priv->factory);
	self->priv->factory = _tmp1_;
	_tmp2_ = dmap_mdns_browser_new (DMAP_MDNS_BROWSER_SERVICE_TYPE_DPAP);
	_g_object_unref0 (self->priv->browser);
	self->priv->browser = _tmp2_;
	_tmp3_ = self->priv->browser;
	g_signal_connect (_tmp3_, "service-added", (GCallback) _dpap_copy_service_added_cb_dmap_mdns_browser_service_added, self);
	_tmp4_ = self->priv->browser;
	dmap_mdns_browser_start (_tmp4_, &_inner_error0_);
	if (G_UNLIKELY (_inner_error0_ != NULL)) {
		g_propagate_error (error, _inner_error0_);
		_dpap_copy_unref0 (self);
		return NULL;
	}
	return self;
}

DPAPCopy*
dpap_copy_new (GError** error)
{
	return dpap_copy_construct (TYPE_DPAP_COPY, error);
}

static void
value_dpap_copy_init (GValue* value)
{
	value->data[0].v_pointer = NULL;
}

static void
value_dpap_copy_free_value (GValue* value)
{
	if (value->data[0].v_pointer) {
		dpap_copy_unref (value->data[0].v_pointer);
	}
}

static void
value_dpap_copy_copy_value (const GValue* src_value,
                            GValue* dest_value)
{
	if (src_value->data[0].v_pointer) {
		dest_value->data[0].v_pointer = dpap_copy_ref (src_value->data[0].v_pointer);
	} else {
		dest_value->data[0].v_pointer = NULL;
	}
}

static gpointer
value_dpap_copy_peek_pointer (const GValue* value)
{
	return value->data[0].v_pointer;
}

static gchar*
value_dpap_copy_collect_value (GValue* value,
                               guint n_collect_values,
                               GTypeCValue* collect_values,
                               guint collect_flags)
{
	if (collect_values[0].v_pointer) {
		DPAPCopy * object;
		object = collect_values[0].v_pointer;
		if (object->parent_instance.g_class == NULL) {
			return g_strconcat ("invalid unclassed object pointer for value type `", G_VALUE_TYPE_NAME (value), "'", NULL);
		} else if (!g_value_type_compatible (G_TYPE_FROM_INSTANCE (object), G_VALUE_TYPE (value))) {
			return g_strconcat ("invalid object type `", g_type_name (G_TYPE_FROM_INSTANCE (object)), "' for value type `", G_VALUE_TYPE_NAME (value), "'", NULL);
		}
		value->data[0].v_pointer = dpap_copy_ref (object);
	} else {
		value->data[0].v_pointer = NULL;
	}
	return NULL;
}

static gchar*
value_dpap_copy_lcopy_value (const GValue* value,
                             guint n_collect_values,
                             GTypeCValue* collect_values,
                             guint collect_flags)
{
	DPAPCopy ** object_p;
	object_p = collect_values[0].v_pointer;
	if (!object_p) {
		return g_strdup_printf ("value location for `%s' passed as NULL", G_VALUE_TYPE_NAME (value));
	}
	if (!value->data[0].v_pointer) {
		*object_p = NULL;
	} else if (collect_flags & G_VALUE_NOCOPY_CONTENTS) {
		*object_p = value->data[0].v_pointer;
	} else {
		*object_p = dpap_copy_ref (value->data[0].v_pointer);
	}
	return NULL;
}

GParamSpec*
param_spec_dpap_copy (const gchar* name,
                      const gchar* nick,
                      const gchar* blurb,
                      GType object_type,
                      GParamFlags flags)
{
	ParamSpecDPAPCopy* spec;
	g_return_val_if_fail (g_type_is_a (object_type, TYPE_DPAP_COPY), NULL);
	spec = g_param_spec_internal (G_TYPE_PARAM_OBJECT, name, nick, blurb, flags);
	G_PARAM_SPEC (spec)->value_type = object_type;
	return G_PARAM_SPEC (spec);
}

gpointer
value_get_dpap_copy (const GValue* value)
{
	g_return_val_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, TYPE_DPAP_COPY), NULL);
	return value->data[0].v_pointer;
}

void
value_set_dpap_copy (GValue* value,
                     gpointer v_object)
{
	DPAPCopy * old;
	g_return_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, TYPE_DPAP_COPY));
	old = value->data[0].v_pointer;
	if (v_object) {
		g_return_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (v_object, TYPE_DPAP_COPY));
		g_return_if_fail (g_value_type_compatible (G_TYPE_FROM_INSTANCE (v_object), G_VALUE_TYPE (value)));
		value->data[0].v_pointer = v_object;
		dpap_copy_ref (value->data[0].v_pointer);
	} else {
		value->data[0].v_pointer = NULL;
	}
	if (old) {
		dpap_copy_unref (old);
	}
}

void
value_take_dpap_copy (GValue* value,
                      gpointer v_object)
{
	DPAPCopy * old;
	g_return_if_fail (G_TYPE_CHECK_VALUE_TYPE (value, TYPE_DPAP_COPY));
	old = value->data[0].v_pointer;
	if (v_object) {
		g_return_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (v_object, TYPE_DPAP_COPY));
		g_return_if_fail (g_value_type_compatible (G_TYPE_FROM_INSTANCE (v_object), G_VALUE_TYPE (value)));
		value->data[0].v_pointer = v_object;
	} else {
		value->data[0].v_pointer = NULL;
	}
	if (old) {
		dpap_copy_unref (old);
	}
}

static void
dpap_copy_class_init (DPAPCopyClass * klass,
                      gpointer klass_data)
{
	dpap_copy_parent_class = g_type_class_peek_parent (klass);
	((DPAPCopyClass *) klass)->finalize = dpap_copy_finalize;
	g_type_class_adjust_private_offset (klass, &DPAPCopy_private_offset);
}

static void
dpap_copy_instance_init (DPAPCopy * self,
                         gpointer klass)
{
	self->priv = dpap_copy_get_instance_private (self);
	self->ref_count = 1;
}

static void
dpap_copy_finalize (DPAPCopy * obj)
{
	DPAPCopy * self;
	self = G_TYPE_CHECK_INSTANCE_CAST (obj, TYPE_DPAP_COPY, DPAPCopy);
	g_signal_handlers_destroy (self);
	_g_object_unref0 (self->priv->browser);
	_g_object_unref0 (self->priv->connection);
	_g_object_unref0 (self->priv->db);
	_g_object_unref0 (self->priv->factory);
}

static GType
dpap_copy_get_type_once (void)
{
	static const GTypeValueTable g_define_type_value_table = { value_dpap_copy_init, value_dpap_copy_free_value, value_dpap_copy_copy_value, value_dpap_copy_peek_pointer, "p", value_dpap_copy_collect_value, "p", value_dpap_copy_lcopy_value };
	static const GTypeInfo g_define_type_info = { sizeof (DPAPCopyClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) dpap_copy_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (DPAPCopy), 0, (GInstanceInitFunc) dpap_copy_instance_init, &g_define_type_value_table };
	static const GTypeFundamentalInfo g_define_type_fundamental_info = { (G_TYPE_FLAG_CLASSED | G_TYPE_FLAG_INSTANTIATABLE | G_TYPE_FLAG_DERIVABLE | G_TYPE_FLAG_DEEP_DERIVABLE) };
	GType dpap_copy_type_id;
	dpap_copy_type_id = g_type_register_fundamental (g_type_fundamental_next (), "DPAPCopy", &g_define_type_info, &g_define_type_fundamental_info, 0);
	DPAPCopy_private_offset = g_type_add_instance_private (dpap_copy_type_id, sizeof (DPAPCopyPrivate));
	return dpap_copy_type_id;
}

GType
dpap_copy_get_type (void)
{
	static volatile gsize dpap_copy_type_id__volatile = 0;
	if (g_once_init_enter (&dpap_copy_type_id__volatile)) {
		GType dpap_copy_type_id;
		dpap_copy_type_id = dpap_copy_get_type_once ();
		g_once_init_leave (&dpap_copy_type_id__volatile, dpap_copy_type_id);
	}
	return dpap_copy_type_id__volatile;
}

gpointer
dpap_copy_ref (gpointer instance)
{
	DPAPCopy * self;
	self = instance;
	g_atomic_int_inc (&self->ref_count);
	return instance;
}

void
dpap_copy_unref (gpointer instance)
{
	DPAPCopy * self;
	self = instance;
	if (g_atomic_int_dec_and_test (&self->ref_count)) {
		DPAP_COPY_GET_CLASS (self)->finalize (self);
		g_type_free_instance ((GTypeInstance *) self);
	}
}

void
debug_printf (const gchar* log_domain,
              GLogLevelFlags log_level,
              const gchar* message)
{
	FILE* _tmp0_;
	_tmp0_ = stdout;
	fprintf (_tmp0_, "%s\n", message);
}

void
debug_null (const gchar* log_domain,
            GLogLevelFlags log_level,
            const gchar* message)
{
}

static void
_debug_null_glog_func (const gchar* log_domain,
                       GLogLevelFlags log_levels,
                       const gchar* message,
                       gpointer self)
{
	debug_null (log_domain, log_levels, message);
}

gint
_vala_main (gchar** args,
            gint args_length1)
{
	GMainLoop* loop = NULL;
	GMainLoop* _tmp0_;
	DPAPCopy* dmapcopy = NULL;
	DPAPCopy* _tmp1_;
	GError* _inner_error0_ = NULL;
	gint result = 0;
	_tmp0_ = g_main_loop_new (NULL, FALSE);
	loop = _tmp0_;
	g_log_set_handler ("libdmapsharing", G_LOG_LEVEL_DEBUG, _debug_null_glog_func, NULL);
	g_log_set_handler (NULL, G_LOG_LEVEL_DEBUG, _debug_null_glog_func, NULL);
	_tmp1_ = dpap_copy_new (&_inner_error0_);
	dmapcopy = _tmp1_;
	if (G_UNLIKELY (_inner_error0_ != NULL)) {
		gint _tmp2_ = -1;
		_g_main_loop_unref0 (loop);
		g_critical ("file %s: line %d: uncaught error: %s (%s, %d)", __FILE__, __LINE__, _inner_error0_->message, g_quark_to_string (_inner_error0_->domain), _inner_error0_->code);
		g_clear_error (&_inner_error0_);
		return _tmp2_;
	}
	g_main_loop_run (loop);
	result = 0;
	_dpap_copy_unref0 (dmapcopy);
	_g_main_loop_unref0 (loop);
	return result;
}

int
main (int argc,
      char ** argv)
{
	return _vala_main (argv, argc);
}

