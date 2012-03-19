if MAINTAINER_MODE
libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@.gi: libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@-uninstalled.files libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@-uninstalled.namespace
	PKG_CONFIG_PATH=$(top_builddir):${PKG_CONFIG_PATH} $(VALA_GEN_INTROSPECT) libdmapsharing-@API_VERSION@-uninstalled libdmapsharing-@API_VERSION@-daap
	@mv libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@-uninstalled.gi libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@.gi

libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@.gi: libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@-uninstalled.files libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@-uninstalled.namespace
	PKG_CONFIG_PATH=$(top_builddir):${PKG_CONFIG_PATH} $(VALA_GEN_INTROSPECT) libdmapsharing-@API_VERSION@-uninstalled libdmapsharing-@API_VERSION@-dacp
	@mv libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@-uninstalled.gi libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@.gi

libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@.gi: libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@-uninstalled.files libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@-uninstalled.namespace
	PKG_CONFIG_PATH=$(top_builddir):${PKG_CONFIG_PATH} $(VALA_GEN_INTROSPECT) libdmapsharing-@API_VERSION@-uninstalled libdmapsharing-@API_VERSION@-dmap
	@mv libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@-uninstalled.gi libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@.gi

libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@.gi: libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@-uninstalled.files libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@-uninstalled.namespace
	PKG_CONFIG_PATH=$(top_builddir):${PKG_CONFIG_PATH} $(VALA_GEN_INTROSPECT) libdmapsharing-@API_VERSION@-uninstalled libdmapsharing-@API_VERSION@-dpap
	@mv libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@-uninstalled.gi libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@.gi

libdmapsharing-@API_VERSION@-daap.vapi: libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@.gi libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@.metadata
	$(VAPIGEN) --pkg=avahi-gobject --pkg=libsoup-2.4 --pkg=glib-2.0 --pkg=gio-2.0 --pkg=gstreamer-0.10 --library libdmapsharing-@API_VERSION@-daap libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@.gi

libdmapsharing-@API_VERSION@-dacp.vapi: libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@.gi libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@.metadata
	$(VAPIGEN) --pkg=avahi-gobject --pkg=libsoup-2.4 --pkg=glib-2.0 --pkg=gio-2.0 --pkg=gstreamer-0.10 --library libdmapsharing-@API_VERSION@-dacp libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@.gi

libdmapsharing-@API_VERSION@-dmap.vapi: libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@.gi libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@.metadata
	$(VAPIGEN) --pkg=avahi-gobject --pkg=libsoup-2.4 --pkg=glib-2.0 --pkg=gio-2.0 --pkg=gstreamer-0.10 --library libdmapsharing-@API_VERSION@-dmap libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@.gi

libdmapsharing-@API_VERSION@-dpap.vapi: libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@.gi libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@.metadata
	$(VAPIGEN) --pkg=avahi-gobject --pkg=libsoup-2.4 --pkg=glib-2.0 --pkg=gio-2.0 --pkg=gstreamer-0.10 --library libdmapsharing-@API_VERSION@-dpap libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@.gi

# NOTE: Sed is used to fix a problem I don't know how to handle in
# vapigen. I have several namespaces (DMAP, DAAP, ...) in libdmapsharing. In
# order to support this, I generate four Vala API files and then concatenate
# them. But, each has its own definition of DMAP.Db, DAAP.DMAPDb, etc. The
# use of sed fixes this:
libdmapsharing-@API_VERSION@.vapi: libdmapsharing-@API_VERSION@-daap.vapi libdmapsharing-@API_VERSION@-dacp.vapi libdmapsharing-@API_VERSION@-dmap.vapi libdmapsharing-@API_VERSION@-dpap.vapi
	cat libdmapsharing-@API_VERSION@-daap.vapi libdmapsharing-@API_VERSION@-dacp.vapi libdmapsharing-@API_VERSION@-dmap.vapi libdmapsharing-@API_VERSION@-dpap.vapi > libdmapsharing-@API_VERSION@.vapi
	cat libdmapsharing-@API_VERSION@.vapi | \
		sed 's/DAAP\.DMAPDb/DMAP.Db/g' | \
		sed 's/DPAP\.DMAPDb/DMAP.Db/g' | \
		sed 's/DACP\.DMAPDb/DMAP.Db/g' | \
		sed 's/DAAP\.DMAPContainerDb/DMAP.ContainerDb/g' | \
		sed 's/DPAP\.DMAPContainerDb/DMAP.ContainerDb/g' | \
		sed 's/DACP\.DMAPContainerDb/DMAP.ContainerDb/g' | \
		sed 's/DAAP\.DMAPRecordFactory/DMAP.RecordFactory/g' | \
		sed 's/DPAP\.DMAPRecordFactory/DMAP.RecordFactory/g' | \
		sed 's/DACP\.DAAPRecord/DAAP.Record/g' | \
		cat > libdmapsharing-@API_VERSION@.vapi.tmp
	mv libdmapsharing-@API_VERSION@.vapi.tmp libdmapsharing-@API_VERSION@.vapi
	rm -f libdmapsharing-@API_VERSION@-daap.vapi libdmapsharing-@API_VERSION@-dacp.vapi libdmapsharing-@API_VERSION@-dmap.vapi libdmapsharing-@API_VERSION@-dpap.vapi
	
all: libdmapsharing-@API_VERSION@.vapi
endif

EXTRA_DIST = \
	libdmapsharing-@API_VERSION@.vapi \
	libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@-uninstalled.excludes \
	libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@-uninstalled.files \
	libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@.metadata \
	libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@-uninstalled.namespace \
	libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@-uninstalled.excludes \
	libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@-uninstalled.files \
	libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@.metadata \
	libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@-uninstalled.namespace \
	libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@-uninstalled.excludes \
	libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@-uninstalled.files \
	libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@.metadata \
	libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@-uninstalled.namespace \
	libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@-uninstalled.excludes \
	libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@-uninstalled.files \
	libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@.metadata \
	libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@-uninstalled.namespace

CLEANFILES = \
	libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@-uninstalled.files \
	libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@-uninstalled.files \
	libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@-uninstalled.files \
	libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@-uninstalled.files \
	libdmapsharing-@API_VERSION@-daap/libdmapsharing-@API_VERSION@.gi \
	libdmapsharing-@API_VERSION@-dacp/libdmapsharing-@API_VERSION@.gi \
	libdmapsharing-@API_VERSION@-dmap/libdmapsharing-@API_VERSION@.gi \
	libdmapsharing-@API_VERSION@-dpap/libdmapsharing-@API_VERSION@.gi \
	libdmapsharing-@API_VERSION@-daap.vapi \
	libdmapsharing-@API_VERSION@-dacp.vapi \
	libdmapsharing-@API_VERSION@-dmap.vapi \
	libdmapsharing-@API_VERSION@-dpap.vapi

if MAINTAINER_MODE
CLEANFILES += \
	libdmapsharing-@API_VERSION@.vapi
endif
