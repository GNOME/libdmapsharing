if HAVE_VALA
libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@.gi: libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@.files libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@.namespace
	PKG_CONFIG_PATH=$(top_builddir):${PKG_CONFIG_PATH} $(VALA_GEN_INTROSPECT) libdmapsharing-@API_VERSION@ libdmapsharing-@API_VERSION@-daap

libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@.gi: libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@.files libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@.namespace
	PKG_CONFIG_PATH=$(top_builddir):${PKG_CONFIG_PATH} $(VALA_GEN_INTROSPECT) libdmapsharing-@API_VERSION@ libdmapsharing-@API_VERSION@-dacp

libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@.gi: libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@.files libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@.namespace
	PKG_CONFIG_PATH=$(top_builddir):${PKG_CONFIG_PATH} $(VALA_GEN_INTROSPECT) libdmapsharing-@API_VERSION@ libdmapsharing-@API_VERSION@-dmap

libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@.gi: libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@.files libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@.namespace
	PKG_CONFIG_PATH=$(top_builddir):${PKG_CONFIG_PATH} $(VALA_GEN_INTROSPECT) libdmapsharing-@API_VERSION@ libdmapsharing-@API_VERSION@-dpap

libdmapsharing-@API_VERSION@-daap.vapi: libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@.gi libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@.metadata
	$(VAPIGEN) --pkg=libsoup-2.4 --pkg=glib-2.0 --library libdmapsharing-@API_VERSION@-daap libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@.gi

libdmapsharing-@API_VERSION@-dacp.vapi: libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@.gi libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@.metadata
	$(VAPIGEN) --pkg=libsoup-2.4 --pkg=glib-2.0 --library libdmapsharing-@API_VERSION@-dacp libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@.gi

libdmapsharing-@API_VERSION@-dmap.vapi: libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@.gi libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@.metadata
	$(VAPIGEN) --pkg=libsoup-2.4 --pkg=glib-2.0 --library libdmapsharing-@API_VERSION@-dmap libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@.gi

libdmapsharing-@API_VERSION@-dpap.vapi: libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@.gi libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@.metadata
	$(VAPIGEN) --pkg=libsoup-2.4 --pkg=glib-2.0 --library libdmapsharing-@API_VERSION@-dpap libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@.gi

libdmapsharing-@API_VERSION@.vapi: libdmapsharing-@API_VERSION@-daap.vapi libdmapsharing-@API_VERSION@-dacp.vapi libdmapsharing-@API_VERSION@-dmap.vapi libdmapsharing-@API_VERSION@-dpap.vapi
	cat libdmapsharing-@API_VERSION@-daap.vapi libdmapsharing-@API_VERSION@-dacp.vapi libdmapsharing-@API_VERSION@-dmap.vapi libdmapsharing-@API_VERSION@-dpap.vapi > libdmapsharing-@API_VERSION@.vapi
	rm -f libdmapsharing-@API_VERSION@-daap.vapi libdmapsharing-@API_VERSION@-dacp.vapi libdmapsharing-@API_VERSION@-dmap.vapi libdmapsharing-@API_VERSION@-dpap.vapi
	
all: libdmapsharing-@API_VERSION@.vapi
endif

EXTRA_DIST = \
	libdmapsharing-@API_VERSION@-daap/libdmapsharing-2.2.excludes \
	libdmapsharing-@API_VERSION@-daap/libdmapsharing-2.2.files \
	libdmapsharing-@API_VERSION@-daap/libdmapsharing-2.2.metadata \
	libdmapsharing-@API_VERSION@-daap/libdmapsharing-2.2.namespace \
	libdmapsharing-@API_VERSION@-dacp/libdmapsharing-2.2.excludes \
	libdmapsharing-@API_VERSION@-dacp/libdmapsharing-2.2.files \
	libdmapsharing-@API_VERSION@-dacp/libdmapsharing-2.2.metadata \
	libdmapsharing-@API_VERSION@-dacp/libdmapsharing-2.2.namespace \
	libdmapsharing-@API_VERSION@-dmap/libdmapsharing-2.2.excludes \
	libdmapsharing-@API_VERSION@-dmap/libdmapsharing-2.2.files \
	libdmapsharing-@API_VERSION@-dmap/libdmapsharing-2.2.metadata \
	libdmapsharing-@API_VERSION@-dmap/libdmapsharing-2.2.namespace \
	libdmapsharing-@API_VERSION@-dpap/libdmapsharing-2.2.excludes \
	libdmapsharing-@API_VERSION@-dpap/libdmapsharing-2.2.files \
	libdmapsharing-@API_VERSION@-dpap/libdmapsharing-2.2.metadata \
	libdmapsharing-@API_VERSION@-dpap/libdmapsharing-2.2.namespace

CLEANFILES = \
	libdmapsharing-@API_VERSION@-daap/libdmapsharing-2.2.gi \
	libdmapsharing-@API_VERSION@-dacp/libdmapsharing-2.2.gi \
	libdmapsharing-@API_VERSION@-dmap/libdmapsharing-2.2.gi \
	libdmapsharing-@API_VERSION@-dpap/libdmapsharing-2.2.gi \
	libdmapsharing-2.2-daap.vapi \
	libdmapsharing-2.2-dacp.vapi \
	libdmapsharing-2.2-dmap.vapi \
	libdmapsharing-2.2-dpap.vapi \
	libdmapsharing-2.2.vapi
