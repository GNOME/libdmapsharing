if BUILD_VAPI

libdmapsharing-@API_VERSION@.vapi: ../libdmapsharing/Dmap-@API_VERSION@.gir
	$(VAPIGEN) --pkg=avahi-gobject --pkg=libsoup-3.0 --pkg=glib-2.0 --pkg=gio-2.0 --pkg=gstreamer-1.0 --library libdmapsharing-@API_VERSION@ $^

all: libdmapsharing-@API_VERSION@.vapi

vapidir = $(datadir)/vala/vapi

dist_vapi_DATA = \
        libdmapsharing-@API_VERSION@.vapi

EXTRA_DIST = \
	libdmapsharing-@API_VERSION@.vapi

CLEANFILES = \
	libdmapsharing-@API_VERSION@.vapi

endif
