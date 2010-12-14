SUBDIRS = docs libdmapsharing tests vala
DIST_SUBDIRS = $(SUBDIRS) m4 media

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
	libdmapsharing-2.2.vapi		\
	libdmapsharing.pc.in		\
	README-Memory

CLEANFILES = $(pcfiles)
