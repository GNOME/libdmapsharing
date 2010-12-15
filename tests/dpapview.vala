/*   FILE: dpapviewer.vala -- View DPAP data
 * AUTHOR: W. Michael Petullo <mike@flyn.org>
 *   DATE: 24 November 2010
 *
 * Copyright (c) 2010 W. Michael Petullo <new@flyn.org>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

private class ViewerDMAPDb : GLib.Object, DMAP.Db {
	// A dumb database that stores everything in an array

	private Gee.ArrayList<ViewerDPAPRecord> db = new Gee.ArrayList<ViewerDPAPRecord> ();

	public uint add (DMAP.Record record) {
		db.add (((ViewerDPAPRecord) record));
		return db.size;
	}

	public uint add_path (string path) {
		GLib.error ("add_path not implemented");
	}

	public uint add_with_id (DMAP.Record record, uint id) {
		GLib.error ("add_with_id not implemented");
	}

	public int64 count () {
		return db.size;
	}

	public void @foreach (GLib.HFunc func) {
		int i;
		for (i = 0; i < db.size; i++) {
			func (i.to_pointer (), db[i]);
		}
	}

	public unowned DMAP.Record lookup_by_id (uint id) {
		GLib.error ("lookup_by_id not implemented");
	}


	public uint lookup_id_by_location (string location){
		GLib.error ("lookup_id_by_location not implemented");
	}
}

private class ViewerDPAPRecordFactory : GLib.Object, DMAP.RecordFactory {
	public DMAP.Record create (void* user_data) {
		return new ViewerDPAPRecord ();
	}
}

private class ViewerDPAPRecord : GLib.Object, DMAP.Record, DPAP.Record {
	private string _location;
	private string _filename;
	private string _aspect_ratio;
	private string _format;
	private string _comments;
	private uchar *_thumbnail;
	private int _filesize;
	private int _large_filesize;
	private int _height;
	private int _width;
	private int _rating;
	private int _creation_date;

	public string location {
		get { return _location; }
		set { _location = value; }
	}

	public string filename {
		get { return _filename; }
		set { _filename = value; }
	}

	public string aspect_ratio {
		get { return _aspect_ratio; }
		set { _aspect_ratio = value; }
	}

	public string format {
		get { return _format; }
		set { _format = value; }
	}

	public uchar *thumbnail {
		get { return _thumbnail; }
		set { _thumbnail = value; }
	}

	public string comments {
		get { return _comments; }
		set { _comments = value; }
	}

	public int filesize {
		get { return _filesize; }
		set { _filesize = value; }
	}

	public int large_filesize {
		get { return _large_filesize; }
		set { _large_filesize = value; }
	}

	public int height {
		get { return _height; }
		set { _height = value; }
	}

	public int width {
		get { return _width; }
		set { _width = value; }
	}

	public int rating {
		get { return _rating; }
		set { _rating = value; }
	}

	public int creation_date {
		get { return _creation_date; }
		set { _creation_date = value; }
	}

	public unowned GLib.InputStream read () throws GLib.Error {
		GLib.error ("read not implemented");
	}

	public unowned DMAP.Record set_from_blob (GLib.ByteArray blob) {
		GLib.error ("set_from_blob not implemented");
	}

	public unowned GLib.ByteArray to_blob () {
		GLib.error ("to_blob not implemented");
	}
}

private class DPAPViewer {
	private DMAP.MdnsBrowser browser;
	private DMAP.Connection connection;
	private Gtk.ListStore liststore;
	private ViewerDMAPDb db;
	private ViewerDPAPRecordFactory factory;

	private bool connected_cb (DMAP.Connection connection, bool result, string? reason) {
		GLib.debug ("%lld entries\n", db.count ());

		db.foreach ((k, v) => {
			string path;
			int fd = GLib.FileUtils.open_tmp ("dpapview.XXXXXX", out path);
			GLib.FileUtils.set_contents (path, (string) ((ViewerDPAPRecord) v).thumbnail, ((ViewerDPAPRecord) v).filesize);
			var pixbuf = new Gdk.Pixbuf.from_file (path);
			GLib.FileUtils.close (fd);
			GLib.FileUtils.unlink (path);

			Gtk.TreeIter iter;
			liststore.append (out iter);
			liststore.set (iter, 0, pixbuf, 1, ((ViewerDPAPRecord) v).filename);
		});

		return true;
	}

	private void service_added_cb (void *service) {
		/* FIXME: fix void * argument, see libdmapsharing TODO: */
		DMAP.MdnsBrowserService *FIXME = service;
		/* FIXME: fix int cast: should not be requried: */
		connection = (DMAP.Connection) new DPAP.Connection (FIXME->service_name, FIXME->host, (int) FIXME->port, false, db, factory);
		connection.connect (connected_cb);
	}

	public DPAPViewer (Gtk.Builder builder) throws GLib.Error {
		builder.connect_signals (this);

		Gtk.Widget widget = builder.get_object ("window") as Gtk.Widget;
		Gtk.IconView iconview = builder.get_object ("iconview") as Gtk.IconView;
		liststore = builder.get_object ("liststore") as Gtk.ListStore;
		db = new ViewerDMAPDb ();
		factory = new ViewerDPAPRecordFactory ();

		iconview.set_pixbuf_column (0);
		iconview.set_text_column (1);

		widget.show_all ();

		browser = new DMAP.MdnsBrowser (DMAP.MdnsBrowserServiceType.DPAP);
		browser.service_added.connect (service_added_cb);
		browser.start ();
	}
}

int main (string[] args) {     
	Gtk.init (ref args);

	try {
		var builder = new Gtk.Builder ();
		builder.add_from_file ("dpapview.ui");

		var dpapviewer = new DPAPViewer (builder);

		Gtk.main ();

	} catch (GLib.Error e) {
		stderr.printf ("Error: %s\n", e.message);
		return 1;
	} 

	return 0;
}
