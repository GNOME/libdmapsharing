if BUILD_VAPI
libdmapsharing-@API_VERSION@.gi: libdmapsharing-@API_VERSION@-uninstalled.files libdmapsharing-@API_VERSION@-uninstalled.namespace
	PKG_CONFIG_PATH=$(top_builddir):${PKG_CONFIG_PATH} $(VALA_GEN_INTROSPECT) libdmapsharing-@API_VERSION@-uninstalled .
	@mv libdmapsharing-@API_VERSION@-uninstalled.gi libdmapsharing-@API_VERSION@.gi

libdmapsharing-@API_VERSION@.vapi: libdmapsharing-@API_VERSION@.gi libdmapsharing-@API_VERSION@.metadata
	$(VAPIGEN) --pkg=avahi-gobject --pkg=libsoup-2.4 --pkg=glib-2.0 --pkg=gio-2.0 --pkg=gstreamer-1.0 --library libdmapsharing-@API_VERSION@ libdmapsharing-@API_VERSION@.gi

all: libdmapsharing-@API_VERSION@.vapi
endif

vapidir = $(datadir)/vala/vapi

dist_vapi_DATA = \
        libdmapsharing-@API_VERSION@.vapi

EXTRA_DIST = \
	libdmapsharing-@API_VERSION@.vapi \
	libdmapsharing-@API_VERSION@-uninstalled.excludes \
	libdmapsharing-@API_VERSION@-uninstalled.files \
	libdmapsharing-@API_VERSION@.metadata \
	libdmapsharing-@API_VERSION@-uninstalled.namespace

CLEANFILES = \
	libdmapsharing-@API_VERSION@.gi \
	libdmapsharing-@API_VERSION@.vapi

DISTCLEANFILES = \
	libdmapsharing-@API_VERSION@-uninstalled.files

if BUILD_VAPI
CLEANFILES += \
	libdmapsharing-@API_VERSION@.vapi
endif
