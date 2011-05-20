SUBDIRS = libdmapsharing vala tests doc
DIST_SUBDIRS = $(SUBDIRS) m4 media

# pcfiles = libdmapsharing-@LIBDMAPSHARING_MAJORMINOR@.pc
pcfiles = libdmapsharing-@API_VERSION@.pc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pcfiles)

EXTRA_DIST =					\
	autogen.sh				\
	README-Memory
