noinst_PROGRAMS = test-dmap-client test-dmap-server dpapview

test_dmap_client_SOURCES = \
	test-dmap-db.c \
	test-daap-record.c \
	test-dpap-record.c \
	test-daap-record-factory.c \
	test-dpap-record-factory.c \
	test-dmap-client.c

test_dmap_client_LDADD = \
	$(top_builddir)/libdmapsharing/libdmapsharing.la \
	$(GLIB_LIBS) \
	$(GTHREAD_LIBS) \
	$(GOBJECT_LIBS) \
	$(GDKPIXBUF_LIBS) \
	$(SOUP_LIBS) \
	$(IMAGEMAGICK_LIBS) \
	$(MDNS_LIBS)

test_dmap_server_SOURCES = \
	test-dmap-db.c \
	test-dmap-container-record.c \
	test-dmap-container-db.c \
	test-dmap-server.c \
	test-daap-record.c \
	test-daap-record-factory.c \
	test-dpap-record.c \
	test-dpap-record-factory.c

test_dmap_server_LDADD = \
	$(top_builddir)/libdmapsharing/libdmapsharing.la \
	$(GLIB_LIBS) \
	$(GTHREAD_LIBS) \
	$(GOBJECT_LIBS) \
	$(GDKPIXBUF_LIBS) \
	$(SOUP_LIBS) \
	$(IMAGEMAGICK_LIBS) \
	$(MDNS_LIBS)

dpapview_VALASOURCES = \
	dpapview.vala

dpapview_VALABUILTSOURCES = $(dpapview_VALASOURCES:.vala=.c)

if MAINTAINER_MODE
PACKAGES = --pkg gee-1.0 --pkg gtk+-2.0 --pkg gstreamer-0.10 --pkg libdmapsharing-2.2 --pkg libsoup-2.4 --pkg glib-2.0
BUILT_SOURCES = vala.stamp

vala.stamp: $(dpapview_VALASOURCES)
	$(VALAC) --vapidir=$(srcdir) $(PACKAGES) $^ -C
	touch $@
endif

dpapview_SOURCES = \
	$(dpapview_VALABUILTSOURCES)

dpapview_LDADD = \
	$(top_builddir)/libdmapsharing/libdmapsharing.la \
	$(GTK_LIBS) \
	$(GEE_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	$(GLIB_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GDKPIXBUF_CFLAGS) \
	$(SOUP_CFLAGS) \
	$(MDNS_CFLAGS)

INCLUDES = \
	$(IMAGEMAGICK_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GEE_CFLAGS)

noinst_HEADERS = \
	test-dmap-container-record.h \
	test-dmap-container-db.h \
	test-dmap-db.h \
	test-daap-record.h \
	test-daap-record-factory.h \
	test-dpap-record.h \
	test-dpap-record-factory.h

EXTRA_DIST = \
	$(dpapview_VALASOURCES) \
	$(dpapview_VALABUILTSOURCES)

if MAINTAINER_MODE
CLEANFILES = \
	$(BUILT_SOURCES) \
	$(dpapview_VALABUILTSOURCES)
endif

