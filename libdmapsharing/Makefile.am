lib_LTLIBRARIES = libdmapsharing.la

BUILT_SOURCES = dmap-marshal.c dmap-marshal.h

libdmapsharing_la_SOURCES = \
	$(BUILT_SOURCES) \
	daap-connection.c \
	daap-record.c \
	daap-share.c \
	dmap-connection.c \
	dmap-container-db.c \
	dmap-container-record.c \
	dmap-db.c \
	dmap-md5.c \
	dmap-record.c \
	dmap-record-factory.c \
	dmap-share.c \
	dmap-structure.c \
	dpap-connection.c \
	dpap-record.c \
	dpap-share.c

if USE_HOWL
libdmapsharing_la_SOURCES += \
	dmap-mdns-browser-howl.c
else
if USE_DNS_SD
libdmapsharing_la_SOURCES += \
	dmap-mdns-browser-dnssd.c \
	dmap-mdns-publisher-dnssd.c
else
libdmapsharing_la_SOURCES += \
	dmap-mdns-avahi.c \
	dmap-mdns-browser-avahi.c \
	dmap-mdns-publisher-avahi.c
endif
endif

if USE_GSTREAMERAPP
libdmapsharing_la_SOURCES += \
	g-gst-input-stream.c \
	g-gst-mp3-input-stream.c \
	g-gst-wav-input-stream.c \
	gst-util.c
endif

libdmapsharing_la_LDFLAGS = \
	-version-info @SO_VERSION@ \
	$(GLIB_LIBS) \
	$(GTHREAD_LIBS) \
	$(GOBJECT_LIBS) \
	$(MDNS_LIBS) \
	$(GSTREAMERAPP_LIBS) \
	$(SOUP_LIBS)

libdmapsharingincludedir = \
	$(includedir)/libdmapsharing-@LIBDMAPSHARING_MAJORMINOR@/libdmapsharing

libdmapsharinginclude_HEADERS = \
	daap-connection.h \
	daap-record.h \
	daap-share.h \
	dmap.h \
	dmap-config.h \
	dmap-connection.h \
	dmap-container-db.h \
	dmap-container-record.h \
	dmap-db.h \
	dmap-md5.h \
	dmap-mdns-browser.h \
	dmap-mdns-publisher.h \
	dmap-record.h \
	dmap-record-factory.h \
	dmap-share.h \
	dmap-structure.h \
	dmap-utils.h \
	dpap-connection.h \
	dpap-record.h \
	dpap-share.h

noinst_HEADERS = \
	dmap-marshal.h \
	dmap-config.h \
	dmap-mdns-avahi.h \
	dmap-mdns-browser.h \
	g-gst-input-stream.h \
	g-gst-mp3-input-stream.h \
	g-gst-wav-input-stream.h \
	gst-util.h

INCLUDES = \
	-I$(top_srcdir) \
	$(SOUP_CFLAGS) \
	$(GSTREAMERAPP_CFLAGS) \
	$(MDNS_CFLAGS)

dmap-marshal.c: dmap-marshal.list
	@glib-genmarshal --prefix=dmap_marshal $(srcdir)/dmap-marshal.list --header --body > dmap-marshal.c

dmap-marshal.h: dmap-marshal.list
	@glib-genmarshal --prefix=dmap_marshal $(srcdir)/dmap-marshal.list --header > dmap-marshal.h

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = dmap-marshal.list
