SUBDIRS = docs libdmapsharing tests
DIST_SUBDIRS = docs libdmapsharing m4 media tests

### all of the standard pc files we need to generate
# pcfiles = libdmapsharing-@LIBDMAPSHARING_MAJORMINOR@.pc
pcfiles = libdmapsharing-@API_VERSION@.pc

all-local: $(pcfiles)

### how to generate pc files
# %-@LIBDMAPSHARING_MAJORMINOR@.pc: %.pc
%-@API_VERSION@.pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pcfiles)

EXTRA_DIST =				\
	autogen.sh			\
	libdmapsharing.pc.in		\
	README-Memory

CLEANFILES = $(pcfiles)
