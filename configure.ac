AC_INIT(libdmapsharing, 2.9.35)

dnl when going to/from release please set the nano (fourth number) right !
dnl releases only do Wall, cvs and prerelease does Werror too
AS_VERSION(libdmapsharing, LIBDMAPSHARING, 2, 9, 35, LIBDMAPSHARING_CVS="no", LIBDMAPSHARING_CVS="yes")

# Enable silent build when available (Automake 1.11)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

dnl FIXME:
dnl would like to automate this off the above definitions, but API might be 2.2 with version 2.1.
dnl see also Makefile.am, libdmapsharing/Makefile.am and libdmapsharing.pc.in
API_VERSION=3.0
#AC_DEFINE(API_VERSION, $API_VERSION, [library API version])
AC_SUBST(API_VERSION)

AM_INIT_AUTOMAKE

LIBDMAPSHARING_MAJORMINOR=$LIBDMAPSHARING_MAJOR_VERSION.$LIBDMAPSHARING_MINOR_VERSION
AC_SUBST(LIBDMAPSHARING_MAJORMINOR)

SO_VERSION=`expr $LIBDMAPSHARING_MINOR_VERSION + $LIBDMAPSHARING_MAJOR_VERSION`:$LIBDMAPSHARING_MICRO_VERSION:$LIBDMAPSHARING_MINOR_VERSION
AC_SUBST(SO_VERSION)

AM_CONFIG_HEADER(config.h)

AM_DISABLE_STATIC

dnl AM_MAINTAINER_MODE provides the option to enable maintainer mode
AM_MAINTAINER_MODE
dnl make aclocal work in maintainer mode
AC_SUBST(ACLOCAL_AMFLAGS, "-I m4")

dnl check for tools
dnl Make sure CFLAGS is defined to stop AC_PROC_CC adding -g
CFLAGS="$CFLAGS "
AC_PROG_CC
AC_PROG_CPP
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_LIBTOOL

GOBJECT_INTROSPECTION_CHECK([1.30.0])

GTK_DOC_CHECK(1.0)
AC_CONFIG_MACRO_DIR(m4)

dnl Test if --disable-tests given
AC_ARG_ENABLE(tests, [AC_HELP_STRING([--disable-tests],[do not build tests])], tests=$enableval, tests=yes)
if test x$tests = xyes; then
	AC_DEFINE(WITH_TESTS,1,[Defined when tests are to be built])
	AM_CONDITIONAL(WITH_TESTS, true)
else
	AM_CONDITIONAL(WITH_TESTS, false)
fi

dnl Test if --enable-debug given
AC_ARG_ENABLE(debug, [AC_HELP_STRING([--enable-debug],[enable debugging mode])])
if test "x$enable_debug" = "xyes" ; then
      CFLAGS="$CFLAGS -g"
fi          

dnl optimisation flag
CFLAGS="$CFLAGS -O2"

dnl decide on error flags
AS_COMPILER_FLAG(-Wall, LIBDMAPSHARING_WALL="yes", LIBDMAPSHARING_WALL="no")

if test "x$LIBDMAPSHARING_WALL" = "xyes"; then
    CFLAGS="$CFLAGS -Wall"

    if test "x$LIBDMAPSHARING_CVS" = "xyes"; then
       AS_COMPILER_FLAG(-Werror,CFLAGS="$CFLAGS -Werror",)
    fi
fi
                  
dnl Now check required packages

dnl Check for pkgconfig
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, yes, no)
dnl Give error and exit if we don't have pkgconfig
if test "x$HAVE_PKGCONFIG" = "xno"; then
  AC_MSG_ERROR(you need to have pkgconfig installed !)
fi

dnl Check for Glib2.0
PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.36, HAVE_GLIB=yes,HAVE_GLIB=no)

dnl Give error and exit if we don't have glib
if test "x$HAVE_GLIB" = "xno"; then
  AC_MSG_ERROR(you need glib-2.0 installed)
fi

dnl make GLIB_CFLAGS and GLIB_LIBS available
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

AC_DEFINE(HAVE_GLIB,1,[Defined when glib-2.0 was found])

dnl Check for GThread2.0
PKG_CHECK_MODULES(GTHREAD, 
  gthread-2.0, 
  HAVE_GTHREAD=yes, HAVE_GTHREAD=no)

dnl Give error and exit if we don't have gthread
if test "x$HAVE_GTHREAD" = "xno"; then
  AC_MSG_ERROR(you need gthread-2.0 installed)
fi

dnl make GOBJECT_CFLAGS and GOBJECT_LIBS available
AC_SUBST(GOBJECT_CFLAGS)
AC_SUBST(GOBJECT_LIBS)

dnl Check for GObject2.0
PKG_CHECK_MODULES(GOBJECT, 
  gobject-2.0, 
  HAVE_GOBJECT=yes, HAVE_GOBJECT=no)

dnl Give error and exit if we don't have gobject
if test "x$HAVE_GOBJECT" = "xno"; then
  AC_MSG_ERROR(you need gobject-2.0 installed)
fi

dnl make GOBJECT_CFLAGS and GOBJECT_LIBS available
AC_SUBST(GOBJECT_CFLAGS)
AC_SUBST(GOBJECT_LIBS)

dnl Check for gdk-pixbuf, needed for DACP Now Playing Artwork
PKG_CHECK_MODULES(GDKPIXBUF, gdk-pixbuf-2.0,
  HAVE_GDKPIXBUF=yes,
  HAVE_GDKPIXBUF=no)

if test x"$HAVE_GDKPIXBUF" = "xyes"; then
  AC_DEFINE(HAVE_GDKPIXBUF, 1, [Define if gdk-pixbuf support is enabled])
else
  AC_WARN(Gdk-pixbuf library not present, Now Playing artwork might be affected.)
fi

AM_CONDITIONAL(USE_GDKPIXBUF, test x"$HAVE_GDKPIXBUF" = "xyes")

AC_SUBST(GDKPIXBUF_CFLAGS)
AC_SUBST(GDKPIXBUF_LIBS)

AC_ARG_WITH(mdns,
   AC_HELP_STRING([--with-mdns=auto|howl|avahi|dns_sd],
   [Select the mDNS/DNS-SD implementation to use (default auto)]),
   WITH_MDNS=$withval,
   WITH_MDNS=auto)

HAVE_HOWL=no
HAVE_AVAHI=no
HAVE_DNS_SD=no # Apple's DNSSD library.
HAVE_MDNS=no
USE_HOWL=no
USE_AVAHI=no
USE_DNS_SD=no

PKG_CHECK_MODULES(AVAHI,
   avahi-client >= 0.6
   avahi-glib >= 0.6,
   HAVE_AVAHI_06=yes,
   HAVE_AVAHI_06=no)
if test x$HAVE_AVAHI_06 != xyes; then
   PKG_CHECK_MODULES(AVAHI,
      avahi-client >= 0.5
      avahi-glib >= 0.5,
      HAVE_AVAHI_05=yes,
      HAVE_AVAHI_05=no)
fi
if test x$HAVE_AVAHI_06 = xyes || test x$HAVE_AVAHI_05 = xyes; then
   HAVE_AVAHI=yes
else
   HAVE_AVAHI=no
fi

PKG_CHECK_MODULES(HOWL,
   howl,
   HAVE_HOWL=yes,
   HAVE_HOWL=no)

AC_CHECK_HEADERS(dns_sd.h, HAVE_DNS_SD=yes)
if test x"$HAVE_DNS_SD" = xyes; then
  AC_CHECK_LIB(dns_sd, DNSServiceRegister, DNS_SD_LIBS="-ldns_sd",)
fi

if test x"$WITH_MDNS" = xauto; then
  if test x"$HAVE_DNS_SD" = xyes; then
    MDNS_CFLAGS=$DNS_SD_CFLAGS
    MDNS_LIBS=$DNS_SD_LIBS
    AC_DEFINE(WITH_DNS_SD, 1, [Define if mDNS/DNS-SD implementation uses Apple's DNSSD])
    USE_DNS_SD=yes
    AC_MSG_NOTICE([Detected Apple's DNSSD, using it for mDNS/DNS-SD])
    HAVE_MDNS=yes
  elif test x"$HAVE_AVAHI" = xyes; then
    MDNS_CFLAGS=$AVAHI_CFLAGS
    MDNS_LIBS=$AVAHI_LIBS
    AC_DEFINE(WITH_AVAHI, 1, [Define if mDNS/DNS-SD implementation uses Avahi])
    USE_AVAHI=yes
    AC_MSG_NOTICE([Detected Avahi, using it for mDNS/DNS-SD])
    if test x$HAVE_AVAHI_06 = xyes; then
      AC_DEFINE(HAVE_AVAHI_0_6, 1, [Define if mDNS/DNS-SD implementation uses Avahi 0.6])
    else
      AC_DEFINE(HAVE_AVAHI_0_5, 1, [Define if mDNS/DNS-SD implementation uses Avahi 0.5])
    fi

    HAVE_MDNS=yes
  elif test x"$HAVE_HOWL" = xyes; then
    GNOME_VFS_REQS=2.7.4
    PKG_CHECK_MODULES(GNOME_VFS, 
      gnome-vfs-2.0 >= $GNOME_VFS_REQS gnome-vfs-module-2.0,
      HAVE_GNOME_VFS=yes,
      HAVE_GNOME_VFS=no)

    if test x"$HAVE_GNOME_VFS" = xno; then
      AC_MSG_ERROR([You need gnome-vfs to compile libdmapsharing with howl support. Install it or try --with-mdns=avahi])
    fi

    MDNS_CFLAGS="$HOWL_CFLAGS $GNOME_VFS_CFLAGS"
    MDNS_LIBS="$HOWL_LIBS $GNOME_VFS_LIBS"
    AC_DEFINE(WITH_HOWL, 1, [Define if mDNS/DNS-SD implementation uses Howl])
    USE_HOWL=yes
    AC_MSG_NOTICE([Detected Howl, using it for mDNS/DNS-SD])

    HAVE_MDNS=yes
  else
    AC_MSG_ERROR([You need avahi, howl or dns_sd.h development files installed.])
  fi
fi

if test x"$WITH_MDNS" = xhowl; then
  if test x"$HAVE_HOWL" = xno; then
    AC_MSG_ERROR([Howl explicitly requested but not found.  Install Howl or try --with-mdns=avahi])
  fi

  GNOME_VFS_REQS=2.7.4
  PKG_CHECK_MODULES(GNOME_VFS,
    gnome-vfs-2.0 >= $GNOME_VFS_REQS gnome-vfs-module-2.0,
    HAVE_GNOME_VFS=yes,
    HAVE_GNOME_VFS=no)

  if test x"$HAVE_GNOME_VFS" = xno; then
    AC_MSG_ERROR([You need gnome-vfs to compile libdmapsharing with howl support. Install it or try --with-mdns=avahi])
  fi

  MDNS_CFLAGS="$HOWL_CFLAGS $GNOME_VFS_CFLAGS"
  MDNS_LIBS="$HOWL_LIBS $GNOME_VFS_LIBS"
  AC_DEFINE(WITH_HOWL, 1, [Define if mDNS/DNS-SD implementation uses Howl])
  USE_HOWL=yes
  AC_MSG_NOTICE([Using Howl for mDNS/DNS-SD])
  HAVE_MDNS=yes
fi

if test x"$WITH_MDNS" = xavahi; then
  if test x"$HAVE_AVAHI" = xno; then
    AC_MSG_ERROR([Avahi explicitly requested but not found.  Install Avahi or try --with-mdns=howl])
  fi

  MDNS_CFLAGS=$AVAHI_CFLAGS
  MDNS_LIBS=$AVAHI_LIBS
  AC_DEFINE(WITH_AVAHI, 1, [Define if mDNS/DNS-SD implementation uses Avahi])
  USE_AVAHI=yes
  AC_MSG_NOTICE([Using Avahi for mDNS/DNS-SD])

  if test x$HAVE_AVAHI_06 = xyes; then
    AC_DEFINE(HAVE_AVAHI_0_6, 1, [Define if mDNS/DNS-SD implementation uses Avahi 0.6])
  else
    AC_DEFINE(HAVE_AVAHI_0_5, 1, [Define if mDNS/DNS-SD implementation uses Avahi 0.5])
  fi

  HAVE_MDNS=yes
fi

if test x"$WITH_MDNS" = xdns_sd; then
  if test x"$HAVE_DNS_SD" = xno; then
    AC_MSG_ERROR([Apple DNSSD explicitly requested but not found.  Install Apple DNSSD or try --with-mdns=avahi])
  fi

  MDNS_CFLAGS=$DNS_SD_CFLAGS
  MDNS_LIBS=$DNS_SD_LIBS
  AC_DEFINE(WITH_DNS_SD, 1, [Define if mDNS/DNS-SD implementation uses Apple's DNSSD])
  USE_DNS_SD=yes
  AC_MSG_NOTICE([Using Apple's DNSSD for mDNS/DNS-SD])

  HAVE_MDNS=yes
fi

AM_CONDITIONAL(USE_HOWL, test "x$USE_HOWL" = "xyes")
AM_CONDITIONAL(USE_AVAHI, test "x$USE_AVAHI" = "xyes")
AM_CONDITIONAL(USE_DNS_SD, test "x$USE_DNS_SD" = "xyes")

AC_SUBST(MDNS_CFLAGS)
AC_SUBST(MDNS_LIBS)

dnl Check for libsoup, needed for DAAP and audioscrobbler
dnl 2.48.0 is required so we can use the new SoupServer API.
PKG_CHECK_MODULES(SOUP, libsoup-2.4 >= 2.48.0,
  HAVE_LIBSOUP=yes,
  HAVE_LIBSOUP=no)

if test x"$HAVE_LIBSOUP" = "xyes"; then
  AC_DEFINE(HAVE_LIBSOUP, 1, [Define if libsoup support is enabled])
fi

AM_CONDITIONAL(USE_LIBSOUP, test x"$HAVE_LIBSOUP" = "xyes")

dnl daap support
if test x"$HAVE_LIBSOUP" = "xno"; then
  AC_MSG_ERROR([libsoup not found.  Install libsoup])
fi

AC_SUBST(SOUP_CFLAGS)
AC_SUBST(SOUP_LIBS)

dnl Check for GStreamer App, needed for transcoding
dnl gstreamer-plugins-base >= 0.10.23.2 for GNOME Bugzilla #588205 & #587896.
PKG_CHECK_MODULES(GSTREAMERAPP,
    gstreamer-app-1.0
    gstreamer-plugins-base-1.0 >= 0.10.23.2,
    HAVE_GSTREAMERAPP=yes,
    HAVE_GSTREAMERAPP=no)

if test x"$HAVE_GSTREAMERAPP" = "xyes"; then
  AC_DEFINE(HAVE_GSTREAMERAPP, 1, [Define if GStreamer App support is enabled])
  GSTLIB=gstreamer-1.0
  AC_SUBST(GSTLIB) # For libdmapsharing-3.0.pc.in.
else
  AC_WARN(GStreamer app element not present, transcoding will not be supported.)
fi

AM_CONDITIONAL(USE_GSTREAMERAPP, test x"$HAVE_GSTREAMERAPP" = "xyes")

AC_SUBST(GSTREAMERAPP_CFLAGS)
AC_SUBST(GSTREAMERAPP_LIBS)

AM_PROG_VALAC([0.11.4])
AM_CONDITIONAL(HAVE_VALAC, test -x "$VALAC")

AC_PATH_PROG(VALA_GEN_INTROSPECT, vala-gen-introspect)
AC_PATH_PROG(VAPIGEN, vapigen)
dnl GOBJECT_INTROSPECTION_CHECK sets found_introspection.
AM_CONDITIONAL(BUILD_VAPI, test -n "$VALA_GEN_INTROSPECT" -a -n "$VAPIGEN" -a xyes = x"$found_introspection")
if ! test -n "$VALA_GEN_INTROSPECT" -a -n "$VAPIGEN" -a xyes = x"$found_introspection"; then
	AC_WARN([will not build vapi])
fi

PKG_CHECK_MODULES(GTK, gtk+-2.0, HAVE_GTK=yes, HAVE_GTK=no)

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES(GEE, gee-0.8, HAVE_GEE=yes, HAVE_GEE=no)
AM_CONDITIONAL(HAVE_GEE, test "x$HAVE_GEE" = "xyes")

AC_SUBST(GEE_CFLAGS)
AC_SUBST(GEE_LIBS)

AM_CONDITIONAL(BUILD_DPAPVIEW, test "x$HAVE_VALA" = "xyes" -a "x$HAVE_GEE" = "xyes" -a "x$HAVE_GTK" = "xyes")
if test -n "$BUILD_DPAPVIEW" ; then
	AC_WARN([will not build dpapview])
fi

PKG_CHECK_MODULES([CHECK], [check >= 0.9.4],have_check=yes,have_check=no)
AM_CONDITIONAL(HAVE_CHECK, test x"$have_check" = "xyes")
if test "x$have_check" = "xyes"; then
	AC_DEFINE(HAVE_CHECK, 1, [Define if check support is enabled])
fi

AC_CHECK_LIB(z, uncompress)

dnl common/m4/dmap-arch.m4
dnl check for unaligned access
AS_UNALIGNED_ACCESS
dnl create a configure variable for libdmapsharing/dmap-config.h
if test x${as_cv_unaligned_access} = xyes ; then
  DMAP_HAVE_UNALIGNED_ACCESS_DEFINE="#define DMAP_HAVE_UNALIGNED_ACCESS 1"
else
  DMAP_HAVE_UNALIGNED_ACCESS_DEFINE="#define DMAP_HAVE_UNALIGNED_ACCESS 0"
fi
AC_SUBST(DMAP_HAVE_UNALIGNED_ACCESS_DEFINE)

AC_OUTPUT([
Makefile
doc/Makefile
libdmapsharing-${API_VERSION}-uninstalled.pc
libdmapsharing-${API_VERSION}.pc
libdmapsharing/Makefile
libdmapsharing/dmap-config.h
tests/Makefile
m4/Makefile
media/Makefile
vala/Makefile
vala/libdmapsharing-${API_VERSION}-daap/libdmapsharing-${API_VERSION}-uninstalled.files
vala/libdmapsharing-${API_VERSION}-dacp/libdmapsharing-${API_VERSION}-uninstalled.files
vala/libdmapsharing-${API_VERSION}-dmap/libdmapsharing-${API_VERSION}-uninstalled.files
vala/libdmapsharing-${API_VERSION}-dpap/libdmapsharing-${API_VERSION}-uninstalled.files
])
