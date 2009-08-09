noinst_PROGRAMS = test-dmap-client test-dmap-server

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
	$(SOUP_LIBS) \
	$(IMAGEMAGICK_LIBS) \
	$(MDNS_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	$(GLIB_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(SOUP_CFLAGS) \
	$(MDNS_CFLAGS)

INCLUDES = \
	$(IMAGEMAGICK_CFLAGS)

noinst_HEADERS = \
	test-dmap-container-record.h \
	test-dmap-container-db.h \
	test-dmap-db.h \
	test-daap-record.h \
	test-daap-record-factory.h \
	test-dpap-record.h \
	test-dpap-record-factory.h
