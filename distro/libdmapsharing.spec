Name: libdmapsharing
Version: 2.9.13
Release: 1%{?dist}
License: LGPLv2+
Source: http://www.flyn.org/projects/libdmapsharing/libdmapsharing-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
URL: http://www.flyn.org/projects/libdmapsharing/
Summary: A DMAP client and server library
Group: Development/Libraries
BuildRequires: pkgconfig, glib2-devel, libsoup-devel >= 2.32
BuildRequires: avahi-glib-devel, gdk-pixbuf2-devel, gstreamer-plugins-base-devel >= 0.10.23.2

%description 
libdmapsharing implements the DMAP protocols. This includes support for
DAAP and DPAP.

%files 
%defattr(-, root, root, -)
%{_libdir}/libdmapsharing-3.0.so.*
%doc AUTHORS COPYING ChangeLog NEWS README


%package devel
Summary: Files needed to develop applications using libdmapsharing
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}, glib2-devel, libsoup-devel, avahi-glib-devel, pkgconfig

%description devel
libdmapsharing implements the DMAP protocols. This includes support for
DAAP and DPAP.  This package provides the libraries, include files, and
other resources needed for developing applications using libdmapsharing.

%files devel
%defattr(-, root, root, -)
%{_libdir}/pkgconfig/libdmapsharing-3.0.pc
%{_includedir}/libdmapsharing-3.0/
%{_libdir}/libdmapsharing-3.0.so
%{_datadir}/gtk-doc/html/libdmapsharing-3.0

%prep

%setup -q -n libdmapsharing-%{version}

%build
%configure --disable-static
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT INSTALL="install -p"
rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdmapsharing-3.0.la

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%changelog
* Sun Dec 19 2010 W. Michael Petullo <mike[@]flyn.org> - 2.1.13-1
- New upstream version.

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
