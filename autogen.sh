#!/bin/sh

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

olddir=`pwd`
cd "$srcdir"

touch ChangeLog

gtkdocize || exit 1
aclocal -I m4 || exit 1
autoconf || exit 1
autoheader || exit 1
libtoolize --force || glibtoolize --force || exit 1
automake -a || exit 1

cd "$olddir"

test -n "$NOCONFIGURE" || "$srcdir/configure" $@ || exit 1

# Now populate ChangeLog.
git log   >  "$srcdir/ChangeLog"
cat <<EOF >> "$srcdir/ChangeLog"

======================== Convert to Git-based ChangeLog ========================
= Please note that there is is a period where some changes were logged in the  =
= ChangeLog and others in Git. This may be present between August 2009 and     =
= November 2010.                                                               =
================================================================================

19 November 2010 W. Michael Petullo <mike@flyn.org>

	* Change Content-Type to consistently be application/x-dmap-tagged
	after getting a bug report from Michael Miceli. I confirmed that
	iTunes uses this content type (not application/x-daap-tagged)
	when sharing music.

	* Move over-anxious g_object_unref in _dmap_share_databases that
	resulted in corrupt DPAP thumbnails when using DmapdDMAPDbDisk.

18 November 2010 W. Michael Petullo <mike@flyn.org>

	* Fix problem sharing to iTunes due to incorrect Content-Length

	* Bump version number.

	* Don't set Connection: Close header in response to /databases/1/items (iTunes)

11 November 2010 W. Michael Petullo <mike@flyn.org>

	* Bump version number.

07 November 2010 W. Michael Petullo <mike@flyn.org>

	* Significant work on an interim solution for the problem
	of memory usage while building the "/1/items" response. We
	previously simply called foreach...add_entry_to_mlcl and later
	serialized the entire structure. This has the disadvantage that
	the entire response must be in memory before libsoup sends it
	to the client. Now, we transmit portions at a time.

31 October 2010 W. Michael Petullo <mike@flyn.org>

	* Bump version number.

24 October 2010 W. Michael Petullo <mike@flyn.org>

	* Fix _dmap_share_build_filter to support commas in values.

23 October 2010 W. Michael Petullo <mike@flyn.org>

	* Provide a configuration-time warning if libsoup < 2.32; this
	is required to interact with Apple Remote App.

21 October 2010 W. Michael Petullo <mike@flyn.org>

	* Advertise support for dmap.supportsupdate even though we don't
	support it. This should fix DACP interaction with Apple Remote
	v2.0.1 (229). Add real support for dmap.supportsupdate to TODO.

16 October 2010 W. Michael Petullo <mike@flyn.org>

	* Add terminating record to meta_data_map in dpap-share.c.

01 October 2010 W. Michael Petullo <mike@flyn.org>

	* Fix missed "daap.songX" to "songX" change.

29 September 2010 W. Michael Petullo <mike@flyn.org>

	* Fix some bugs related to the recent DmapMdnsPublisher singleton
	change.

09 September 2010 W. Michael Petullo <mike@flyn.org>

	* Allow two sub-DMAP protocols (e.g., DAAP and DPAP) in one
	process.

07 September 2010 W. Michael Petullo <mike@flyn.org>

	* Allow dmap-mdns-*-dnssd.c to compile.

06 September 2010 W. Michael Petullo <mike@flyn.org>

	* Fix serving to iTunes 10.

03 September 2010 W. Michael Petullo <mike@flyn.org>

	* Fix license notice on several source files.

	* Fix license on dacp-player.[ch] after receiving confirmation
	from Alexandre.

	* Fix license on test-dmap-client.c after receiving confirmation
	from Andre.

20 August 2010 W. Michael Petullo <mike@flyn.org>

	* Back out Alexandre's addition of itemid property from
	DAAPRecord. The itemid is the DMAPDb ID.

20 August 2010 Alexandre Rosenfeld <alexandre.rosenfeld@gmail.com>

	* Apply Alexandre Rosenfeld's DACP patch from Google Summer of
	Code 2010.

22 July 2010 W. Michael Petullo <mike@flyn.org>

	* Remove redundancy between dmap_connection_build_message()
	and dmap_connection_build_message().

04 June 2010 W. Michael Petullo <mike@flyn.org>

	* Add dmap_db_add_with_id() to DMAPDb interface.

	* dmap_container_record_get_entries() must not return a const
	because the return value should be free'd.

01 June 2010 W. Michael Petullo <mike@flyn.org>

	* Unref value returned by dmap_container_record_get_entries().

	* Free strings returned by g_object_get().

30 May 2010 W. Michael Petullo <mike@flyn.org>

	* Change lookup_by_path() to lookup_by_location().

29 May 2010 W. Michael Petullo <mike@flyn.org>

	* Add blob functions to DMAPRecord interface.

	* Add lookup_by_path() to DMAPDb.

	* Add add_path() to DMAPDb.

	* Ensure dmap_container_db_lookup_by_id(),
	dmap_container_record_get_id() and dmap_db_lookup_by_id() all
	deal with guint ids.

28 May 2010 W. Michael Petullo <mike@flyn.org>

	* Install dmap-md5.h.

27 May 2010 W. Michael Petullo <mike@flyn.org>

	* Fix crash upon finalizing DMAPShare that was caused by misuse of
	avahi_entry_group_free().

	* Increase the reference count of the databases passed to
	d[ap]ap_share_new().

	* dmap_db_lookup_by_id now takes a const DMAPDb *.

	* Add a _dmap_share_build_filter() prototype.

25 May 2010 W. Michael Petullo <mike@flyn.org>

	* Refactor build_filter into dmap_share_build_factor.

	* Begin naming D[AP]APRecord properties after DAAP keywords so
	that direct use of g_object_get can replace if/else or table
	lookups.

	* Fix some declarations in dmap-db.c so that record IDs are
	always guint (vs. gint).

24 May 2010 W. Michael Petullo <mike@flyn.org>

	* Refactor DMAPShare, DAAPShare and DPAPShare to move code
	to DMAPShare and increase code reuse.

22 May 2010 W. Michael Petullo <mike@flyn.org>

	* Add new to_blob/new_from_blob interface to D[AP]PRecord.

20 May 2010 W. Michael Petullo <mike@flyn.org>

	* Fix finalize code for DMAPShare.

18 May 2010 W. Michael Petullo <mike@flyn.org>

	* Add another case to dpap-share.c:build_filter() in an attempt
	to support iTunes '09.

16 May 2010 W. Michael Petullo <mike@flyn.org>

	* Documentation work.

07 May 2010 W. Michael Petullo <mike@flyn.org>

	* Send artist and album sort order to DAAP clients.

17 March 2010 W. Michael Petullo <mike@flyn.org>

	* Make _dmap_share_add_playlist_to_mlcl a proper GHashFunc.

15 March 2010 W. Michael Petullo <mike@flyn.org>

	* Set def. and min. value of track, year and disc to 0 (not 1) in
	DAAPRecord.

	* Use -D instead of dmap-priv.h to define G_LOG_DOMAIN.

11 March 2010 W. Michael Petullo <mike@flyn.org>

	* Add user agent header to requests (Rhythmbox bug #610440).

	* Fix URI handling when using IPv6 IP address (Rhythmbox bug #584244).

27 February 2010 W. Michael Petullo <mike@flyn.org>

	* Change dmap_connection_get_headers so that it no longer takes a
	bytes argument and returns a SoupMessageHeaders *. This is to remain
	compatible with Rhythmbox.

12 December 2009 W. Michael Petullo <mike@flyn.org>

	* Fixed debug statement that caused segfaults of MIPS32 and
	PowerPC.

05 December 2009 W. Michael Petullo <mike@flyn.org>

	* Use our own log domain.

28 November 2009 W. Michael Petullo <mike@flyn.org>

	* Change configure script to require avahi, howl OR DNS_SD.H.

28 August 2009 W. Michael Petullo <mike@flyn.org>

	* Make dmap_record_factory_create more generic, take gpointer
	user_data instead of path.

27 August 2009 W. Michael Petullo <mike@flyn.org>

	* Fix daap_connection_new, take password_required.

	* Use typedef's for function arguments.

	* DMAPDb foreach function now takes GHFunc.

22 August 2009 W. Michael Petullo <mike@flyn.org>

	* Work on dns_sd mDNS backend.

	* Make autogen.sh work with MacPorts.

17 August 2009 W. Michael Petullo <mike@flyn.org>

	* Update tests to include dmap.h.

	* Ensure values g_free'd before set in D[AP]APRecord's
	set_properties.

	* Add dmap-mdns-publisher.h back to installed headers.

16 August 2009 W. Michael Petullo <mike@flyn.org>

	* Add finalize function for TestDAAPRecord and TestDPAPRecord.

15 August 2009 W. Michael Petullo <mike@flyn.org>

	* Documentation.

	* Add progress idle function to dmap-connection.c.

	* Install dmap-mdns-browser.h again.

14 August 2009 W. Michael Petullo <mike@flyn.org>

	* Add documentation overview.

13 August 2009 W. Michael Petullo <mike@flyn.org>

	* Proper reference counting for DMAPRecords in DMAPDbs.

	* Documentation work.

12 August 2009 W. Michael Petullo <mike@flyn.org>

	* Simplify autogen.sh.

30 July 2009 W. Michael Petullo <mike@flyn.org>

	* Modify configure.ac to require gstreamer-plugins-base >=
	0.10.23.2 for GNOME Bugzilla #588205 & #587896.

	* Remove newly obsolete code (daap-item.c, etc.)

	* Auto-generate marshaling code for dmap-connection.c.

29 July 2009 W. Michael Petullo <mike@flyn.org>

	* Port tests to new client API.

28 July 2009 W. Michael Petullo <mike@flyn.org>

	* More rhythmbox-related work.

27 July 2009 W. Michael Petullo <mike@flyn.org>

	* Rhythmbox-related work.

25 July 2009 W. Michael Petullo <mike@flyn.org>

	* Pull DMAPConnection from more modern Rhythmbox in preparation
	of implementing libdmapsharing-base Rhythmbox DAAP plugin.

22 July 2009 W. Michael Petullo <mike@flyn.org>

	* D[AP]APRecord no longer had DMAPRecord as parent.

	* Fixed various runtime warnings.

21 July 2009 W. Michael Petullo <mike@flyn.org>

	* Fix handling of DAAP filesize.

	* Work on seeking.

18 July 2009 W. Michael Petullo <mike@flyn.org>

	* Fix various GLib Warnings.

13 July 2009 W. Michael Petullo <mike@flyn.org>

	* Fix bug where the first stream/pipeline continues after
	a fast forwarding to a second.

11 July 2009 W. Michael Petullo <mike@flyn.org>

	* Change some g_warning's to g_debug.

10 July 2009 W. Michael Petullo <mike@flyn.org>

	* Fix HTTP encoding decision in daap-share.c.

09 July 2009 W. Michael Petullo <mike@flyn.org>

	* Fix use of giostream element (two upstream GStreamer bugs
	fixed in the process).

06 July 2009 W. Michael Petullo <mike@flyn.org>

	* Make all GStreamer / transcoding code optional.

05 July 2009 W. Michael Petullo <mike@flyn.org>

	* Move transcoding to libdmapsharing.

02 July 2009 W. Michael Petullo <mike@flyn.org>

	* Get rid of get methods in daap-record.c and replace with
	GObject properties.

17 June 2009 W. Michael Petullo <mike@flyn.org>

	* Slight API change to support delaying trancoding decisions
	until after a file has been requested.

30 April 2009 W. Michael Petullo <mike@flyn.org>

	* Fix compiler warning.

	* Update RPM specification.

19 April 2009 W. Michael Petullo <mike@flyn.org>

	* Fix memory leaks in dmap-db.c's filter code.

18 April 2009 W. Michael Petullo <mike@flyn.org>

	* Start to implement dmap-mdns-browser-dnssd.c and
	dmap-mdns-publisher-dnssd.c -- still needs work.

11 April 2009 W. Michael Petullo <mike@flyn.org>

	* Add support for Apple's DNSSD to configure.ac.

10 April 2009 W. Michael Petullo <mike@flyn.org>

	* Clean up some compiler warnings.

07 April 2009 W. Michael Petullo <mike@flyn.org>

	* Completed DAAP browsing.

05 April 2009 W. Michael Petullo <mike@flyn.org>

	* Filtering now works.

04 April 2009 W. Michael Petullo <mike@flyn.org>

	* Work on DAAP browsing, start support for filter.

31 March 2009 W. Michael Petullo <mike@flyn.org>

	* Began supporting DAAP browsing (e.g., list genres).

30 March 2009 W. Michael Petullo <mike@flyn.org>

	* Set rating in daap-share.c.

29 March 2009 W. Michael Petullo <mike@flyn.org>

	* Fix compiler warnings related to const return from
	dmap_container_record_get_entries.

28 March 2009 W. Michael Petullo <mike@flyn.org>

	* Fix support for Roku clients; they use query parameter, much
	like iPhoto.

	* Update pkg-config file to support includedir & libdir in terms
	of prefix.

26 March 2009 W. Michael Petullo <mike@flyn.org>

	* Work on seeking.

	* Add new DAAPRecord method: itunes_compat.

24 March 2009 W. Michael Petullo <mike@flyn.org>

	* Make dmap_container_record_get_entries return a const DMAPDb *.

	* Remove commented out mmap code from daap-share.c.

07 March 2009 W. Michael Petullo <mike@flyn.org>

	* Fix Cflags in libdmapsharing.pc.in.

06 March 2009 W. Michael Petullo <mike@flyn.org>

	* Use @libdir@ in libdmapsharing.pc.in to support 64-bit
	architectures.

	* Do not include -<major verion> in SONAME.

	* Set LDFLAGS properly when building libdmapsharing.

	* Update RPM specification file.

02 March 2009 W. Michael Petullo <mike@flyn.org>

	* Fix some issues that broke compiling on 64-bit platforms.

28 February 2009 W. Michael Petullo <mike@flyn.org>

	* Support building with older versions of libsoup, just don't
	build HTTP 1.0 / SOUP_ENCODING_EOF support.

27 February 2009 W. Michael Petullo <mike@flyn.org>

	* Always use content length encoding for video data, as iTunes
	seems to require this.

22 February 2009 W. Michael Petullo <mike@flyn.org>

	* Require libsoup >= 2.25.92, as this version supports
	SOUP_ENCODING_EOF message encoding, required for HTTP 1.0 clients.

	* Decrease DMAP_SHARE_CHUNK_SIZE now that the Roku SoundBridge is
	properly handled as a HTTP 1.0 client.

12 February 2009 W. Michael Petullo <mike@flyn.org>

	* dmap_container_record_get_entries now returns a DMAPDb *
	instead of a GSList *.

	* Increase DMAP_SHARE_CHUNK_SIZE in order to keep Roku SoundBridge
	client from popping.

	* Send proper count to client when providing list of containers.

02 February 2009 W. Michael Petullo <mike@flyn.org>

	* Bump version number to 1.9 in preparation for release.

31 January 2009 W. Michael Petullo <mike@flyn.org>

	* Slight change to DAAPRecord and DPAPRecord interfaces. Add
	read method that returns GInputStream * (instead of simply the
	location/path). This sets the conditions for realtime transcoding
	done by interface implementations.

25 January 2009 W. Michael Petullo <mike@flyn.org>

	* Don't try to mmap large files for DAAP sharing; they may be
	large videos that could cause the server to thrash.

19 January 2009 W. Michael Petullo <mike@flyn.org>

	* Add a DMAP record factory interface.

18 January 2009 W. Michael Petullo <mike@flyn.org>

	* Support sharing video (Quicktime for now).

	* Merge DAAPDb and DPAPDb interfaces into DMAPDb.

10 January 2009 W. Michael Petullo <mike@flyn.org>

	* Update libdmapsharing.spec.

09 January 2009 W. Michael Petullo <mike@flyn.org>

	* Format some code better.

	* Release mmap'ed files before mmap'ing another one.

	* Implement sharing of playlists / albums.

08 January 2009 W. Michael Petullo <mike@flyn.org>

	* Add to README.

	* Format some code better.

	* Update license headings.

07 January 2009 W. Michael Petullo <mike@flyn.org>

	* Add library version to build system.

	* Remove fork from dmap-test-server.c.

	* Consolidate parsing of meta-data portion of query in dmap-share.c.

06 January 2009 W. Michael Petullo <mike@flyn.org>

	* Add Fedora RPM spec file.

05 January 2009 W. Michael Petullo <mike@flyn.org>

	* Fix test code to work with new thumbnail interface.

01 January 2009 W. Michael Petullo <mike@flyn.org>

	* Reengineered database code.

30 December 2008 W. Michael Petullo <mike@flyn.org>

	* DPAP sharing now works with iPhoto 6.

28 December 2008 W. Michael Petullo <mike@flyn.org>

	* DAAP sharing now works with iTunes 8.

26 December 2008 W. Michael Petullo <mike@flyn.org>

	* Split DAAP sharing functionality into two classes: DMAPShare
	and DAAPShare. DAAPShare is a subclass of DMAPShare.

	* Start work on DPAP server code.

21 December 2008 W. Michael Petullo <mike@flyn.org>

	* Moved daap_mdns_publisher* to dmap_mdns_publisher*.

	* Make type_of_service (e.g., _daap._tcp) configurable in
	dmap_mdns_publisher*.

18 December 2008 W. Michael Petullo <mike@flyn.org>

	* Begin implementing server code.

18 December 2008 W. Michael Petullo <mike@flyn.org>

	* Update to compile against libsoup-2.24.
EOF

exit 0
