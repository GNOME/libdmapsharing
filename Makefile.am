SUBDIRS = libdmapsharing tests
DIST_SUBDIRS = common libdmapsharing m4 media tests

### all of the standard pc files we need to generate
pcfiles = libdmapsharing-@LIBDMAPSHARING_MAJORMINOR@.pc

all-local: $(pcfiles)

### how to generate pc files
%-@LIBDMAPSHARING_MAJORMINOR@.pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pcfiles)

EXTRA_DIST =				\
	autogen.sh				\
	libdmapsharing.pc.in

CLEANFILES = $(pcfiles)
