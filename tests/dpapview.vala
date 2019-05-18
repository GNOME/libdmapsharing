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

private class DpapViewer {
	private Dmap.MdnsBrowser browser;
	private Dmap.Connection connection;
	private Gtk.ListStore liststore;
	private ValaDmapDb db;
	private ValaImageRecordFactory factory;

	private void connected_cb (Dmap.Connection connection, bool result, string? reason) {
		GLib.debug ("%" + int64.FORMAT + " entries\n", db.count ());

		db.foreach ((k, v) => {
			Gdk.Pixbuf pixbuf = null;
			if (((ValaImageRecord) v).thumbnail != null) {
				string path;
				int fd = GLib.FileUtils.open_tmp ("dpapview.XXXXXX", out path);
				GLib.FileUtils.set_data (path, (uint8[]) ((ValaImageRecord) v).thumbnail.data);
				GLib.FileUtils.close (fd);
				pixbuf = new Gdk.Pixbuf.from_file (path);
				GLib.FileUtils.unlink (path);
			}

			Gtk.TreeIter iter;
			liststore.append (out iter);
			liststore.set (iter, 0, pixbuf, 1, ((ValaImageRecord) v).filename);
		});
	}

	public DpapViewer (Gtk.Builder builder) throws GLib.Error {
		builder.connect_signals (this);

		Gtk.Widget widget = builder.get_object ("window") as Gtk.Widget;
		Gtk.IconView iconview = builder.get_object ("iconview") as Gtk.IconView;
		liststore = builder.get_object ("liststore") as Gtk.ListStore;
		db = new ValaDmapDb ();
		factory = new ValaImageRecordFactory ();

		iconview.set_pixbuf_column (0);
		iconview.set_text_column (1);

		widget.show_all ();

		browser = new Dmap.MdnsBrowser (Dmap.MdnsServiceType.DPAP);
		browser.service_added.connect ((browser, service) =>  {
			connection = (Dmap.Connection) new Dmap.ImageConnection (service.service_name, service.host, service.port, db, factory);
			connection.start (connected_cb);
		});
		browser.start ();
	}
}

int main (string[] args) {     
	Gtk.init (ref args);

	try {
		var builder = new Gtk.Builder ();
		builder.add_from_file ("tests/dpapview.ui");

		var dpapviewer = new DpapViewer (builder);

		Gtk.main ();

	} catch (GLib.Error e) {
		stderr.printf ("Error: %s\n", e.message);
		return 1;
	} 

	return 0;
}
