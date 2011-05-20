## Process this file with automake to produce Makefile.in

# We require automake 1.6 at least.
AUTOMAKE_OPTIONS = 1.6

# This is a blank Makefile.am for using gtk-doc.
# Copy this to your project's API docs directory and modify the variables to
# suit your project. See the GTK+ Makefiles in gtk+/docs/reference for examples
# of using the various options.

# The name of the module, e.g. 'glib'.
DOC_MODULE=libdmapsharing-3.0

# Uncomment for versioned docs and specify the version of the module, e.g. '2'.
#DOC_MODULE_VERSION=2

# The top-level SGML file. You can change this if you want to.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.xml

# The directory containing the source code. Relative to $(srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting the functions and macros.
# e.g. DOC_SOURCE_DIR=../../../gtk
DOC_SOURCE_DIR=../libdmapsharing

# Extra options to pass to gtkdoc-scangobj. Not normally needed.
SCANGOBJ_OPTIONS=

# Extra options to supply to gtkdoc-scan.
# e.g. SCAN_OPTIONS=--deprecated-guards="GTK_DISABLE_DEPRECATED"
SCAN_OPTIONS=

# Extra options to supply to gtkdoc-mkdb.
# e.g. MKDB_OPTIONS=--sgml-mode --output-format=xml
MKDB_OPTIONS=--sgml-mode --output-format=xml

# Extra options to supply to gtkdoc-mktmpl
# e.g. MKTMPL_OPTIONS=--only-section-tmpl
MKTMPL_OPTIONS=

# Extra options to supply to gtkdoc-mkhtml
MKHTML_OPTIONS=

# Extra options to supply to gtkdoc-fixref. Not normally needed.
# e.g. FIXXREF_OPTIONS=--extra-dir=../gdk-pixbuf/html --extra-dir=../gdk/html
FIXXREF_OPTIONS=

# Used for dependencies. The docs will be rebuilt if any of these change.
# e.g. HFILE_GLOB=$(top_srcdir)/gtk/*.h
# e.g. CFILE_GLOB=$(top_srcdir)/gtk/*.c
HFILE_GLOB=
CFILE_GLOB=

# Extra header to include when scanning, which are not under DOC_SOURCE_DIR
# e.g. EXTRA_HFILES=$(top_srcdir}/contrib/extra.h
EXTRA_HFILES=

# Header files to ignore when scanning. Use base file name, no paths
# e.g. IGNORE_HFILES=gtkdebug.h gtkintl.h
IGNORE_HFILES= \
	dmap-marshal.h \
	dmap-config.h \
	dmap-md5.h \
	dmap-mdns-avahi.h \
	dmap-mdns-publisher.h \
	dmap-structure.h \
	dmap-utils.h \
	g-gst-input-stream.h \
	g-gst-mp3-input-stream.h \
	g-gst-wav-input-stream.h \
	gst-util.h

# Images to copy into HTML directory.
# e.g. HTML_IMAGES=$(top_srcdir)/gtk/stock-icons/stock_about_24.png
HTML_IMAGES=

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
# e.g. content_files=running.sgml building.sgml changes-2.0.sgml
content_files=

# SGML files where gtk-doc abbrevations (#GtkWidget) are expanded
# These files must be listed here *and* in content_files
# e.g. expand_content_files=running.sgml
expand_content_files=

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
# Only needed if you are using gtkdoc-scangobj to dynamically query widget
# signals and properties.
# e.g. GTKDOC_CFLAGS=-I$(top_srcdir) -I$(top_builddir) $(GTK_DEBUG_FLAGS)
# e.g. GTKDOC_LIBS=$(top_builddir)/gtk/$(gtktargetlib)
GTKDOC_CFLAGS= \
	-I$(top_srcdir) \
	$(SOUP_CFLAGS) \
	$(GSTREAMERAPP_CFLAGS) \
	$(MDNS_CFLAGS)

GTKDOC_LIBS= \
	$(top_builddir)/libdmapsharing/libdmapsharing-3.0.la \
	$(GLIB_LIBS) \
	$(GTHREAD_LIBS) \
	$(GOBJECT_LIBS) \
	$(MDNS_LIBS) \
	$(GSTREAMERAPP_LIBS) \
	$(SOUP_LIBS)

# This includes the standard gtk-doc make rules, copied by gtkdocize.
include $(top_srcdir)/gtk-doc.make

# Other files to distribute
# e.g. EXTRA_DIST += version.xml.in
EXTRA_DIST +=

# Files not to distribute
# for --rebuild-types in $(SCAN_OPTIONS), e.g. $(DOC_MODULE).types
# for --rebuild-sections in $(SCAN_OPTIONS) e.g. $(DOC_MODULE)-sections.txt
#DISTCLEANFILES +=

# Comment this out if you want your docs-status tested during 'make check'
if ENABLE_GTK_DOC
#TESTS_ENVIRONMENT = cd $(srcsrc)
#TESTS = $(GTKDOC_CHECK)
endif

-include $(top_srcdir)/git.mk

# FIXME: this target is host-specific!
# NOTE: flyn.org serves using "Content-Type: text/html;
# charset=ISO-8859-1." This breaks HTML that contains UTF-8.
# To get GTK-Doc to output ISO-8859-1, edit
# /usr/share/gtk-doc/data/gtk-doc.xsl and set "chunker.output.encoding"
# to "ISO-8859-1."
publish:
	sed 's/images\/logo_head.png/\.\.\/\.\.\/\.\.\/images\/logo_head.png/g' ~/Public/www.flyn.org/head.html.in > tmpl/head.html
# Add header and point stylesheet to flyn.org stylesheet
	for h in html/*.html; do \
		sed '/<body/ r tmpl/head.html' $$h | sed 's/style.css/\.\.\/\.\.\/\.\.\/flyn.org.css/g' \
			> ~/Public/www.flyn.org/projects/libdmapsharing/docs/`basename $$h`; \
	done
	cp html/*.png ~/Public/www.flyn.org/projects/libdmapsharing/docs/
