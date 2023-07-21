m4_define([libdmapsharing_major], [3])
m4_define([libdmapsharing_minor], [9])
m4_define([libdmapsharing_micro], [13])
m4_define([libdmapsharing_version], [libdmapsharing_major.libdmapsharing_minor.libdmapsharing_micro])

AC_INIT([libdmapsharing],[libdmapsharing_version],[https://www.flyn.org/projects/libdmapsharing/])

API_VERSION=4.0
AC_SUBST(API_VERSION)

SO_VERSION=m4_eval(libdmapsharing_minor+libdmapsharing_major):m4_eval(libdmapsharing_micro):m4_eval(libdmapsharing_minor)
AC_SUBST(SO_VERSION)

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CC
AC_PROG_CPP
LT_INIT
AC_PROG_CC

AC_SEARCH_LIBS([floorf], [m])

GOBJECT_INTROSPECTION_CHECK([1.30.0])
GTK_DOC_CHECK(1.0)

# Have pedansee?
AC_PATH_PROG(PEDANSEE, pedansee)
AM_CONDITIONAL(HAVE_PEDANSEE, test -n "$PEDANSEE")

# Select debugging or measuring code coverage?
AC_ARG_ENABLE(debug, [AS_HELP_STRING([--enable-debug],[enable debugging build])])
AC_ARG_ENABLE(coverage, [AS_HELP_STRING([--enable-coverage],[enable code-coverage build])])
if test "x$enable_debug" = "xyes"; then
	CFLAGS="$CFLAGS -O0 -g"
elif test "x$enable_coverage" = "xyes"; then
	CFLAGS="$CFLAGS -O0 -g -fprofile-arcs -ftest-coverage"
fi

# Select more compiler warnings?
AC_ARG_ENABLE(more-warnings, [AS_HELP_STRING([--enable-more-warnings],[maximum compiler warnings])])
if test "$GCC" = "yes" -a x$enable_more_warnings = "xyes"; then
	WARN_CFLAGS="\
		-Wcomment -Wformat -Wnonnull -Wimplicit-int -Wimplicit \
		-Wmain -Wmissing-braces -Wparentheses -Wsequence-point \
		-Wreturn-type -Wswitch -Wtrigraphs -Wunused-function \
		-Wunused-label -Wunused-value \
		-Wchar-subscripts -Wmissing-declarations -Wmissing-prototypes \
		-Wnested-externs -Wpointer-arith \
		-Wformat-security -Wall \
		-Werror -Wno-deprecated-declarations -std=gnu89"

	if echo "$CFLAGS" | grep -q -- '-O' && echo "$CFLAGS" | grep -q -v -- '-O0'; then
		WARN_CFLAGS="-Wuninitialized $WARN_CFLAGS"
	fi

	for option in $WARN_CFLAGS; do
		AS_COMPILER_FLAG($option, has_option=yes, has_option=no)
		if test x$has_option = xyes; then
			OK_CFLAGS="$OK_CFLAGS $option"
		fi
		unset has_option
	done

	WARN_CFLAGS="$OK_CFLAGS"
fi

AC_SUBST(WARN_CFLAGS)

# Build test code?
AC_ARG_ENABLE(tests, [AS_HELP_STRING([--disable-tests],[do not build tests])], tests=$enableval, tests=yes)
if test x$tests = xyes; then
	PKG_CHECK_MODULES([CHECK], [check >= 0.9.4],have_check=yes,have_check=no)
	AM_CONDITIONAL(HAVE_CHECK, test x"$have_check" = "xyes")
	if test "x$have_check" = "xyes"; then
		AC_DEFINE(HAVE_CHECK, 1, [Define if check support is enabled])
	else
		AC_MSG_ERROR([Must have check library present when tests enabled])
	fi

	AC_DEFINE(TESTS_ENABLED,1,[Defined when tests are to be built])
	AM_CONDITIONAL(TESTS_ENABLED, true)
else
	AM_CONDITIONAL(TESTS_ENABLED, false)
	AM_CONDITIONAL(HAVE_CHECK, false)
fi

# Have libz?
AC_CHECK_LIB(z, uncompress)

# Have pkg-config?
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, yes, no)
if test "x$HAVE_PKGCONFIG" = "xno"; then
	AC_MSG_ERROR([Must have pkgconfig installed])
fi

# Have glib?
PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.66, HAVE_GLIB=yes,HAVE_GLIB=no)
if test "x$HAVE_GLIB" = "xno"; then
	AC_MSG_ERROR([Must have glib-2.0 installed])
fi

AC_DEFINE(GLIB_VERSION_MIN_REQUIRED, GLIB_VERSION_2_66, [minimum glib version])
AC_DEFINE(GLIB_VERSION_MAX_ALLOWED, GLIB_VERSION_2_66, [maximum glib version])

AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

# Have gthread?
PKG_CHECK_MODULES(GTHREAD, gthread-2.0, HAVE_GTHREAD=yes, HAVE_GTHREAD=no)
if test "x$HAVE_GTHREAD" = "xno"; then
	AC_MSG_ERROR([Must have gthread-2.0 installed])
fi

AC_SUBST(GOBJECT_CFLAGS)
AC_SUBST(GOBJECT_LIBS)

# Have gobject?
PKG_CHECK_MODULES(GOBJECT, gobject-2.0, HAVE_GOBJECT=yes, HAVE_GOBJECT=no)
if test "x$HAVE_GOBJECT" = "xno"; then
	AC_MSG_ERROR([Must have gobject-2.0 installed])
fi

AC_SUBST(GOBJECT_CFLAGS)
AC_SUBST(GOBJECT_LIBS)

# Have gdkpixbuf?
PKG_CHECK_MODULES(GDKPIXBUF, gdk-pixbuf-2.0, HAVE_GDKPIXBUF=yes, HAVE_GDKPIXBUF=no)
if test x"$HAVE_GDKPIXBUF" = "xyes"; then
	AC_DEFINE(HAVE_GDKPIXBUF, 1, [Define if gdk-pixbuf support is enabled])
else
	AC_MSG_WARN([Gdk-pixbuf library not present; now-playing artwork might be affected])
fi

AM_CONDITIONAL(USE_GDKPIXBUF, test x"$HAVE_GDKPIXBUF" = "xyes")

AC_SUBST(GDKPIXBUF_CFLAGS)
AC_SUBST(GDKPIXBUF_LIBS)

# Have libsoup?
PKG_CHECK_MODULES(SOUP, libsoup-3.0, HAVE_LIBSOUP=yes, HAVE_LIBSOUP=no)
if test x"$HAVE_LIBSOUP" = "xno"; then
	AC_MSG_ERROR([Must have libsoup installed])
fi

AC_SUBST(SOUP_CFLAGS)
AC_SUBST(SOUP_LIBS)

# Have GTK3?
PKG_CHECK_MODULES(GTK, gtk+-3.0, HAVE_GTK=yes, HAVE_GTK=no)

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

# Have libgee?
PKG_CHECK_MODULES(GEE, gee-0.8, HAVE_GEE=yes, HAVE_GEE=no)

AC_SUBST(GEE_CFLAGS)
AC_SUBST(GEE_LIBS)

# Have gstreamer-app?
PKG_CHECK_MODULES(
	GSTREAMERAPP,
	gstreamer-app-1.0
	gstreamer-plugins-base-1.0 >= 0.10.23.2,
	HAVE_GSTREAMERAPP=yes,
	HAVE_GSTREAMERAPP=no
)

if test x"$HAVE_GSTREAMERAPP" = "xyes"; then
	AC_DEFINE(HAVE_GSTREAMERAPP, 1, [Define if GStreamer App support is enabled])
	GSTLIB="gstreamer-1.0 gstreamer-app-1.0"
	AC_SUBST(GSTLIB)
else
	AC_MSG_WARN([GStreamer app element not present; transcoding will not be supported])
fi

AM_CONDITIONAL(USE_GSTREAMERAPP, test x"$HAVE_GSTREAMERAPP" = "xyes")

AC_SUBST(GSTREAMERAPP_CFLAGS)
AC_SUBST(GSTREAMERAPP_LIBS)

# Have Vala compiler?
AM_PROG_VALAC([0.11.4])
AM_CONDITIONAL(HAVE_VALAC, test -x "$VALAC")

# Have vapigen?
AC_PATH_PROG(VAPIGEN, vapigen)
AM_CONDITIONAL(BUILD_VAPI, test -n "$VAPIGEN" -a xyes = x"$found_introspection")
if ! test -n "$VAPIGEN" -a xyes = x"$found_introspection"; then
	AC_MSG_WARN([Will not build vapi])
fi

AM_CONDITIONAL(BUILD_VALATESTS, test -x "$VALAC" -a "x$HAVE_GEE" = "xyes")
if test -n "$BUILD_VALATESTS" ; then
	AC_MSG_WARN([Will not build Vala tests])
fi

AM_CONDITIONAL(BUILD_DPAPVIEW, test -x "$VALAC" -a "x$HAVE_GEE" = "xyes" -a "x$HAVE_GTK" = "xyes")
if test -n "$BUILD_DPAPVIEW" ; then
	AC_MSG_WARN([Will not build dpapview])
fi

if test x${as_cv_unaligned_access} = xyes ; then
	DMAP_HAVE_UNALIGNED_ACCESS_DEFINE="#define DMAP_HAVE_UNALIGNED_ACCESS 1"
else
	DMAP_HAVE_UNALIGNED_ACCESS_DEFINE="#define DMAP_HAVE_UNALIGNED_ACCESS 0"
fi
AC_SUBST(DMAP_HAVE_UNALIGNED_ACCESS_DEFINE)

# Select mDNS library.
AC_ARG_WITH(
	mdns,
	AS_HELP_STRING([--with-mdns=auto|avahi|dns_sd],[Select the mDNS/DNS-SD implementation to use (default auto)]),
	WITH_MDNS=$withval,
	WITH_MDNS=auto
)

HAVE_AVAHI=no
HAVE_DNS_SD=no # Apple's DNSSD library.
USE_AVAHI=no
USE_DNS_SD=no

if test x"$WITH_MDNS" = xauto || test x"$WITH_MDNS" = xavahi; then
	PKG_CHECK_MODULES(
		AVAHI,
		avahi-client >= 0.6
		avahi-glib >= 0.6,
		HAVE_AVAHI=yes,
		HAVE_AVAHI=no
	)
fi

if test x"$HAVE_AVAHI" = xno; then
	if test x"$WITH_MDNS" = xauto || test x"$WITH_MDNS" = xdns_sd; then
		AC_CHECK_HEADERS(dns_sd.h, HAVE_DNS_SD=yes)
		if test x"$HAVE_DNS_SD" = xyes; then
			AC_CHECK_LIB(dns_sd, DNSServiceRegister, DNS_SD_LIBS="-ldns_sd",)
		fi
	fi
fi

if test x"$WITH_MDNS" = xauto; then
	if test x"$HAVE_AVAHI" = xyes; then
		MDNS_CFLAGS=$AVAHI_CFLAGS
		MDNS_LIBS=$AVAHI_LIBS
		USE_AVAHI=yes
		AC_MSG_NOTICE([Detected Avahi; using it for mDNS/DNS-SD])
	elif test x"$HAVE_DNS_SD" = xyes; then
		MDNS_CFLAGS=$DNS_SD_CFLAGS
		MDNS_LIBS=$DNS_SD_LIBS
		USE_DNS_SD=yes
		AC_MSG_NOTICE([Detected Apple's DNSSD; using it for mDNS/DNS-SD])
	else
		AC_MSG_ERROR([Must have avahi or dns_sd.h development files installed])
	fi
elif test x"$WITH_MDNS" = xavahi; then
	if test x"$HAVE_AVAHI" = xno; then
		AC_MSG_ERROR([Avahi explicitly requested but not found. Install Avahi, or try --with-mdns=dns_sd])
	fi

	MDNS_CFLAGS=$AVAHI_CFLAGS
	MDNS_LIBS=$AVAHI_LIBS
	USE_AVAHI=yes
	AC_MSG_NOTICE([Using Avahi for mDNS/DNS-SD])
elif test x"$WITH_MDNS" = xdns_sd; then
	if test x"$HAVE_DNS_SD" = xno; then
		AC_MSG_ERROR([Apple DNSSD explicitly requested but not found. Install Apple DNSSD, or try --with-mdns=avahi])
	fi

	MDNS_CFLAGS=$DNS_SD_CFLAGS
	MDNS_LIBS=$DNS_SD_LIBS
	USE_DNS_SD=yes
	AC_MSG_NOTICE([Using Apple's DNSSD for mDNS/DNS-SD])
fi

AM_CONDITIONAL(USE_AVAHI, test "x$USE_AVAHI" = "xyes")
AM_CONDITIONAL(USE_DNS_SD, test "x$USE_DNS_SD" = "xyes")

AC_SUBST(MDNS_CFLAGS)
AC_SUBST(MDNS_LIBS)

# Enable silent build when available (Automake 1.11)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_INIT_AUTOMAKE

AC_CONFIG_FILES([
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
])

AC_OUTPUT
