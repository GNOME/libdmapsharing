if TESTS_ENABLED
noinst_PROGRAMS = test-dmap-client test-dmap-server

if BUILD_VALATESTS
noinst_PROGRAMS += dacplisten dmapcopy dmapserve
endif

if BUILD_DPAPVIEW
noinst_PROGRAMS += dpapview
endif

# Automake documentation mentions LDADD, but it does not work;
# LIBS does.
LIBS += $(top_builddir)/libdmapsharing/libdmapsharing-4.0.la

if HAVE_CHECK
noinst_PROGRAMS += unit-test
endif

test_dmap_client_SOURCES = \
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
	test-dmap-server.c

test_dmap_server_LDADD = \
	$(GLIB_LIBS) \
	$(GTHREAD_LIBS) \
	$(GSTREAMERAPP_LIBS) \
	$(GOBJECT_LIBS) \
	$(GDKPIXBUF_LIBS) \
	$(SOUP_LIBS) \
	$(IMAGEMAGICK_LIBS) \
	$(MDNS_LIBS)

dacplisten.c: $(dacplisten_VALASOURCES)
	$(VALAC) --vapidir=../vala --pkg gee-0.8 --pkg gstreamer-1.0 --pkg libdmapsharing-4.0 --pkg libsoup-3.0 --pkg gio-2.0 --pkg avahi-gobject  $^ -C

dpapview.c: $(dpapview_VALASOURCES)
	$(VALAC) --vapidir=../vala --pkg gee-0.8 --pkg gtk+-3.0 --pkg gstreamer-1.0 --pkg libdmapsharing-4.0 --pkg libsoup-3.0 --pkg gio-2.0 --pkg avahi-gobject $^ -C

dmapcopy.c: $(dmapcopy_VALASOURCES)
	$(VALAC) --vapidir=../vala --pkg gee-0.8 --pkg gstreamer-1.0 --pkg libdmapsharing-4.0 --pkg libsoup-3.0 --pkg gio-2.0 --pkg avahi-gobject $^ -C

dmapserve.c: $(dmapserve_VALASOURCES)
	$(VALAC) --vapidir=../vala --pkg gee-0.8 --pkg gstreamer-1.0 --pkg libdmapsharing-4.0 --pkg libsoup-3.0 --pkg gio-2.0 --pkg avahi-gobject $^ -C

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
	vala-image-record.vala

dpapview_VALABUILTSOURCES = $(dpapview_VALASOURCES:.vala=.c)

dpapview_SOURCES = \
	$(dpapview_VALABUILTSOURCES)

dpapview_LDADD = \
	$(GTK_LIBS) \
	$(GEE_LIBS)

dmapcopy_VALASOURCES = \
	dmapcopy.vala \
	vala-dmap-db.vala \
	vala-image-record.vala

dmapcopy_VALABUILTSOURCES = $(dmapcopy_VALASOURCES:.vala=.c)

dmapcopy_SOURCES = \
	$(dmapcopy_VALABUILTSOURCES)

dmapcopy_LDADD = \
	$(GEE_LIBS)

dmapserve_VALASOURCES = \
	dmapserve.vala \
	vala-dmap-db.vala \
	vala-image-record.vala \
	vala-dmap-container-db.vala

dmapserve_VALABUILTSOURCES = $(dmapserve_VALASOURCES:.vala=.c)

dmapserve_SOURCES = \
	$(dmapserve_VALABUILTSOURCES)

dmapserve_LDADD = \
	$(GEE_LIBS)

unit_test_SOURCES = \
	unit-test.c

unit_test_LDADD = \
	$(CHECK_LIBS) \
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	$(GDKPIXBUF_CFLAGS) \
	$(GEE_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GSTREAMERAPP_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(GTK_CFLAGS) \
	$(IMAGEMAGICK_CFLAGS) \
	$(MDNS_CFLAGS) \
	$(SOUP_CFLAGS)

EXTRA_DIST = \
	$(dacplisten_VALABUILTSOURCES) \
	$(dacplisten_VALASOURCES) \
	$(dmapcopy_VALABUILTSOURCES) \
	$(dmapcopy_VALASOURCES) \
	$(dmapserve_VALABUILTSOURCES) \
	$(dmapserve_VALASOURCES) \
	$(dpapview_VALABUILTSOURCES) \
	$(dpapview_VALASOURCES) \
	dpapview.ui

if HAVE_VALAC
CLEANFILES = \
	$(dacplisten_VALABUILTSOURCES) \
	$(dmapcopy_VALABUILTSOURCES) \
	$(dmapserve_VALABUILTSOURCES) \
	$(dpapview_VALABUILTSOURCES) \
	$(dpapview_VALABUILTSOURCES)
endif
endif
