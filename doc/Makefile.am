AUTOMAKE_OPTIONS = 1.6

DOC_MODULE = libdmapsharing-4.0

DOC_MAIN_SGML_FILE = $(DOC_MODULE)-docs.xml

DOC_SOURCE_DIR = $(top_srcdir)/libdmapsharing

SCAN_OPTIONS =

MKDB_OPTIONS = --sgml-mode --output-format=xml

HFILE_GLOB =

CFILE_GLOB =

IGNORE_HFILES = \
	dmap-config.h \
	dmap-connection-private.h \
	dmap-gst-mp3-input-stream.h \
	dmap-gst-qt-input-stream.h \
	dmap-gst-wav-input-stream.h \
	dmap.h \
	dmap-mdns-avahi.h \
	dmap-private-utils.h \
	dmap-share-private.h \
	dmap-structure.h \
	gst-util.h \
	test-dmap-av-record-factory.h \
	test-dmap-av-record.h \
	test-dmap-container-db.h \
	test-dmap-container-record.h \
	test-dmap-db.h \
	test-dmap-image-record-factory.h \
	test-dmap-image-record.h

HTML_IMAGES =

GTKDOC_CFLAGS = \
	-I$(top_srcdir) \
	$(SOUP_CFLAGS) \
	$(GSTREAMERAPP_CFLAGS) \
	$(MDNS_CFLAGS)

GTKDOC_LIBS = \
	$(top_builddir)/libdmapsharing/libdmapsharing-4.0.la \
	$(CHECK_LIBS) \
	$(GLIB_LIBS) \
	$(GTHREAD_LIBS) \
	$(GOBJECT_LIBS) \
	$(MDNS_LIBS) \
	$(GSTREAMERAPP_LIBS) \
	$(SOUP_LIBS)

include $(top_srcdir)/gtk-doc.make

DISTCLEANFILES = \
	libdmapsharing-4.0-overrides.txt \
	libdmapsharing-4.0-sections.txt \
	libdmapsharing-4.0.types

EXTRA_DIST = \
	authenticatecb.xml \
	browserapi.xml \
	client-internals.xml \
	coding-standards.xml \
	connectedcb.xml \
	containerdb.xml \
	errorcb.xml \
	libdmapsharing-4.0-docs.xml \
	mediadb.xml \
	server-internals.xml \
	serviceaddedcb.xml \
	shareapi.xml
