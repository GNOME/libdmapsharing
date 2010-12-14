if HAVE_VALA
libdmapsharing-@API_VERSION@.gi: libdmapsharing-@API_VERSION@.files libdmapsharing-@API_VERSION@.namespace
	PKG_CONFIG_PATH=$(top_builddir):${PKG_CONFIG_PATH} $(VALA_GEN_INTROSPECT) libdmapsharing-@API_VERSION@ .

libdmapsharing-@API_VERSION@.vapi: libdmapsharing-@API_VERSION@.gi libdmapsharing-@API_VERSION@.metadata
	$(VAPIGEN) --pkg=libsoup-2.4 --pkg=glib-2.0 --library libdmapsharing-@API_VERSION@ libdmapsharing-@API_VERSION@.gi
	
all: libdmapsharing-@API_VERSION@.vapi
endif

EXTRA_DIST = \
	libdmapsharing-2.2.files \
	libdmapsharing-2.2.metadata \
	libdmapsharing-2.2.namespace

CLEANFILES = \
	libdmapsharing-2.2.gi \
	libdmapsharing-2.2.vapi
