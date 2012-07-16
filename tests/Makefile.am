noinst_PROGRAMS = test-dmap-client test-dmap-server

if HAVE_GEE
noinst_PROGRAMS += dacplisten dpapview dmapcopy dmapserve
endif

# Automake documentation mentions LDADD, but it does not work;
# LIBS does.
LIBS = $(top_builddir)/libdmapsharing/libdmapsharing-3.0.la

if HAVE_CHECK
noinst_PROGRAMS += unit-test

LIBS += $(CHECK_LIBS)
endif

test_dmap_client_SOURCES = \
	test-dmap-db.c \
	test-daap-record.c \
	test-dpap-record.c \
	test-daap-record-factory.c \
	test-dpap-record-factory.c \
	test-dmap-client.c

test_dmap_client_LDADD = \
	$(GLIB_LIBS) \
	$(GTHREAD_LIBS) \
	$(GSTREAMERAPP_LIBS) \
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
	$(GLIB_LIBS) \
	$(GTHREAD_LIBS) \
	$(GSTREAMERAPP_LIBS) \
	$(GOBJECT_LIBS) \
	$(GDKPIXBUF_LIBS) \
	$(SOUP_LIBS) \
	$(IMAGEMAGICK_LIBS) \
	$(MDNS_LIBS)

if MAINTAINER_MODE
BUILT_SOURCES = dacplisten.stamp dpapview.stamp dmapcopy.stamp dmapserve.stamp

dacplisten.stamp: $(dacplisten_VALASOURCES)
	$(VALAC) --vapidir=../vala --pkg gee-0.8 --pkg gstreamer-0.10 --pkg libdmapsharing-3.0 --pkg libsoup-2.4 --pkg glib-2.0 --pkg avahi-gobject  $^ -C
	touch $@

dpapview.stamp: $(dpapview_VALASOURCES)
	$(VALAC) --vapidir=../vala --pkg gee-0.8 --pkg gtk+-2.0 --pkg gstreamer-0.10 --pkg libdmapsharing-3.0 --pkg libsoup-2.4 --pkg glib-2.0 --pkg avahi-gobject $^ -C
	touch $@

dmapcopy.stamp: $(dmapcopy_VALASOURCES)
	$(VALAC) --vapidir=../vala --pkg gee-0.8 --pkg gstreamer-0.10 --pkg libdmapsharing-3.0 --pkg libsoup-2.4 --pkg glib-2.0 --pkg avahi-gobject $^ -C
	touch $@

dmapserve.stamp: $(dmapserve_VALASOURCES)
	$(VALAC) --vapidir=../vala --pkg gee-0.8 --pkg gstreamer-0.10 --pkg libdmapsharing-3.0 --pkg libsoup-2.4 --pkg glib-2.0 --pkg avahi-gobject $^ -C
	touch $@
endif

dacplisten_VALASOURCES = \
	dacplisten.vala \
	vala-dmap-db.vala \
	vala-dmap-container-db.vala

dacplisten_VALABUILTSOURCES = $(dacplisten_VALASOURCES:.vala=.c)

dacplisten_SOURCES = \
	$(dacplisten_VALABUILTSOURCES)

dacplisten_LDADD = \
	$(GEE_LIBS)

dpapview_VALASOURCES = \
	dpapview.vala \
	vala-dmap-db.vala \
	vala-dpap-record.vala

dpapview_VALABUILTSOURCES = $(dpapview_VALASOURCES:.vala=.c)

dpapview_SOURCES = \
	$(dpapview_VALABUILTSOURCES)

dpapview_LDADD = \
	$(GTK_LIBS) \
	$(GEE_LIBS)

dmapcopy_VALASOURCES = \
	dmapcopy.vala \
	vala-dmap-db.vala \
	vala-dpap-record.vala

dmapcopy_VALABUILTSOURCES = $(dmapcopy_VALASOURCES:.vala=.c)

dmapcopy_SOURCES = \
	$(dmapcopy_VALABUILTSOURCES)

dmapcopy_LDADD = \
	$(GEE_LIBS)

dmapserve_VALASOURCES = \
	dmapserve.vala \
	vala-dmap-db.vala \
	vala-dpap-record.vala \
	vala-dmap-container-db.vala

dmapserve_VALABUILTSOURCES = $(dmapserve_VALASOURCES:.vala=.c)

dmapserve_SOURCES = \
	$(dmapserve_VALABUILTSOURCES)

dmapserve_LDADD = \
	$(GEE_LIBS)

unit_test_SOURCES = \
	unit-test.c

unit_test_LDADD = \
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	$(GLIB_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(GSTREAMERAPP_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GDKPIXBUF_CFLAGS) \
	$(SOUP_CFLAGS) \
	$(MDNS_CFLAGS)

INCLUDES = \
	$(IMAGEMAGICK_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GEE_CFLAGS)

if HAVE_CHECK
INCLUDES += $(CHECK_CFLAGS)
endif

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
	$(dpapview_VALABUILTSOURCES) \
	$(dacplisten_VALASOURCES) \
	$(dacplisten_VALABUILTSOURCES) \
	$(dmapserve_VALASOURCES) \
	$(dmapserve_VALABUILTSOURCES) \
	dpapview.ui

if MAINTAINER_MODE
CLEANFILES = \
	$(BUILT_SOURCES) \
	$(dpapview_VALABUILTSOURCES)
endif
