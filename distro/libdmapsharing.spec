Name: libdmapsharing
Version: 3.1.0
Release: 1%{?dist}
License: LGPLv2+
Source: http://www.flyn.org/projects/libdmapsharing/%{name}-%{version}.tar.gz
URL: http://www.flyn.org/projects/libdmapsharing/
Summary: A DMAP client and server library
Group: Development/Libraries
BuildRequires: pkgconfig, glib2-devel, libsoup-devel >= 2.32
BuildRequires: gdk-pixbuf2-devel, gstreamer1-plugins-base-devel
BuildRequires: pkgconfig(avahi-client) pkgconfig(avahi-glib)
BuildRequires: vala-tools libgee-devel

%description 
libdmapsharing implements the DMAP protocols. This includes support for
DAAP and DPAP.

%files 
%{_libdir}/libdmapsharing-3.2.so.*
%doc AUTHORS COPYING ChangeLog README

%package devel
Summary: Libraries/include files for libdmapsharing
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
libdmapsharing implements the DMAP protocols. This includes support for
DAAP and DPAP.  This package provides the libraries, include files, and
other resources needed for developing applications using libdmapsharing.

%files devel
%{_libdir}/pkgconfig/libdmapsharing-3.2.pc
%{_includedir}/libdmapsharing-3.2/
%{_libdir}/libdmapsharing-3.2.so
%{_libdir}/girepository-1.0/DMAP-3.2.typelib
%{_libdir}/girepository-1.0/DAAP-3.2.typelib
%{_datadir}/gtk-doc/html/libdmapsharing-3.2
%{_datadir}/gir-1.0/DMAP-3.2.gir
%{_datadir}/gir-1.0/DAAP-3.2.gir

%package vala
Summary: Vala language bindings for libdmapsharing
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description vala
libdmapsharing implements the DMAP protocols. This includes support for
DAAP and DPAP.  This package provides the Vala language bindings for
libdmapsharing.

%files vala
%{_datadir}/vala/vapi/libdmapsharing-3.2.vapi

%prep
%setup -q

%build
%configure --disable-static --disable-tests --disable-check
make %{?_smp_mflags}

%install
make install DESTDIR=$RPM_BUILD_ROOT INSTALL="install -p"
rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdmapsharing-3.2.la

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%changelog
* Mon Aug 01 2016 W. Michael Petullo <mike[@]flyn.org> - 2.9.36-1
- new upstream version to fix Bugzilla #1158652

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 2.9.30-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.9.30-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Mon Mar 16 2015 Than Ngo <than@redhat.com> - 2.9.30-2
- bump release and rebuild so that koji-shadow can rebuild it
  against new gcc on secondary arch

* Sat Feb 07 2015 W. Michael Petullo <mike[@]flyn.org> - 2.9.30-1
- new upstream version

* Mon Sep 01 2014 W. Michael Petullo <mike[@]flyn.org> - 2.9.29-2
- add DMAP-3.0.typelib and DMAP-3.0.gir

* Mon Sep 01 2014 W. Michael Petullo <mike[@]flyn.org> - 2.9.29-1
- new upstream version
- do not build tests

* Mon Sep 01 2014 W. Michael Petullo <mike[@]flyn.org> - 2.9.28-3
- require libgee-devel in order to build

* Mon Sep 01 2014 W. Michael Petullo <mike[@]flyn.org> - 2.9.28-2
- package Vala support

* Mon Sep 01 2014 W. Michael Petullo <mike[@]flyn.org> - 2.9.28-1
- new upstream version

* Sun Aug 17 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.9.24-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.9.24-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sun Dec 22 2013 Ville Skyttä <ville.skytta@iki.fi> - 2.9.24-2
- Drop empty NEWS from docs.

* Thu Nov 07 2013 W. Michael Petullo <mike[@]flyn.org> - 2.9.24-1
- new upstream version

* Thu Sep 26 2013 Rex Dieter <rdieter@fedoraproject.org> 2.9.18-3
- add explicit avahi build deps

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.9.18-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Fri Jul 05 2013 W. Michael Petullo <mike[@]flyn.org> - 2.9.18-1
- new upstream version

* Sun Apr 07 2013 Kalev Lember <kalevlember@gmail.com> - 2.9.16-1
- Update to 2.9.16

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.9.14-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.9.14-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Fri Jan 13 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.9.14-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Mon Dec 05 2011 W. Michael Petullo <mike[@]flyn.org> - 2.9.14-1
- new upstream version
- Remove patch from previous release (upstreamed)

* Tue Nov 08 2011 Adam Jackson <ajax@redhat.com> 2.9.12-2
- libdmapsharing-2.9.12-glib.patch: Fix FTBFS against new glib

* Mon Aug 22 2011 Adam Williamson <awilliam@redhat.com> - 2.9.12-1
- new upstream version

* Mon Mar 21 2011 W. Michael Petullo <mike[@]flyn.org> - 2.9.6-1
- New upstream version.

* Mon Feb 07 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.9.5-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Mon Feb 07 2011 W. Michael Petullo <mike[@]flyn.org> - 2.9.5-1
- New upstream version, fixes problem compiling without libgee.

* Mon Feb 07 2011 W. Michael Petullo <mike[@]flyn.org> - 2.9.4-1
- New upstream version (API 3 series).

* Sun Dec 19 2010 W. Michael Petullo <mike[@]flyn.org> - 2.1.13-1
- New upstream version.

* Sun Nov 28 2010 W. Michael Petullo <mike[@]flyn.org> - 2.1.12-1
- New upstream version.

* Sun Oct 31 2010 W. Michael Petullo <mike[@]flyn.org> - 2.1.8-1
- New upstream version.
- Update Source and URL.
- BuildRequire gdk-pixbuf2-devel for DACP.
- BuildRequire libsoup-devel >= 2.32 for DACP.
- BuildRequire gstreamer-plugins-base-devel >= for transcoding.

* Fri Jun 04 2010 W. Michael Petullo <mike[@]flyn.org> - 1.9.0.21-1
- New upstream version.

* Fri May 28 2010 W. Michael Petullo <mike[@]flyn.org> - 1.9.0.18-1
- New upstream version.

* Fri Aug 28 2009 W. Michael Petullo <mike[@]flyn.org> - 1.9.0.13-1
- New upstream version.

* Thu Aug 27 2009 W. Michael Petullo <mike[@]flyn.org> - 1.9.0.12-1
- New upstream version.

* Sat Aug 15 2009 W. Michael Petullo <mike[@]flyn.org> - 1.9.0.11-1
- New upstream version.
- Add gtk-doc documentation to devel package.

* Wed Jul 29 2009 W. Michael Petullo <mike[@]flyn.org> - 1.9.0.10-1
- New upstream version.

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.9.0.9-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Thu Jul 23 2009 W. Michael Petullo <mike[@]flyn.org> - 1.9.0.9-1
- New upstream version.

* Tue Mar 10 2009 W. Michael Petullo <mike[@]flyn.org> - 1.9.0.4-1
- New upstream version.

* Fri Mar 06 2009 W. Michael Petullo <mike[@]flyn.org> - 1.9.0.3-1
- New upstream version.
- Use "-p /sbin/ldconfig."
- Remove requires that are already known by RPM.
- libdmapsharing-devel package now requires pkgconfig.
- Remove irrelevant INSTALL documentation.

* Sun Feb 22 2009 W. Michael Petullo <mike[@]flyn.org> - 1.9.0.1-3
- Require libsoup >= 2.25.92, as this version supports SOUP_ENCODING_EOF
message encoding, required for HTTP 1.0 clients.

* Sat Feb 07 2009 W. Michael Petullo <mike[@]flyn.org> - 1.9.0.1-2
- Fix BuildRequires.

* Sun Dec 28 2008 W. Michael Petullo <mike[@]flyn.org> - 1.9.0.1-1
- Initial package
