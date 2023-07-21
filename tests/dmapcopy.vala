/*   FILE: dmapcopy.vala -- Copy files from a DMAP server
 * AUTHOR: W. Michael Petullo <mike@flyn.org>
 *   DATE: 20 December 2010 
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

private class DPAPCopy {
	private Dmap.MdnsBrowser browser;
	private Dmap.Connection connection;
	private ValaDmapDb db;
	private ValaImageRecordFactory factory;

	private void connected_cb (Dmap.Connection connection, bool result, string? reason) {
		GLib.debug ("%" + int64.FORMAT + " entries\n", db.count ());

		db.foreach ((k, v) => {
			stdout.printf ("%s\n", ((ValaImageRecord) v).location);

			/* Uncomment to copy the data:
			var session = new Soup.SessionAsync ();
			var message = new Soup.Message ("GET", ((ValaDPAPRecord) v).location);
			message.request_headers = connection.get_headers (((ValaDPAPRecord) v).location);

			GLib.debug ("GET %s", ((ValaDPAPRecord) v).location);
			session.send_message (message);

			var file = File.new_for_path (((int) k).to_string ());
			var file_stream = file.create (FileCreateFlags.NONE);

			// Test for the existence of file
			if (file.query_exists ()) {
				stdout.printf ("File successfully created.\n");
			}

			// Write text data to file
			var data_stream = new DataOutputStream (file_stream);
			data_stream.write (message.response_body.data, (size_t) message.response_body.length, null);
			*/
		});
	}

	public DPAPCopy () throws GLib.Error {
		db = new ValaDmapDb ();
		factory = new ValaImageRecordFactory ();

		browser = new Dmap.MdnsBrowser (Dmap.MdnsServiceType.DPAP);
		browser.service_added.connect ((browser, service) => {
			connection = (Dmap.Connection) new Dmap.ImageConnection (service.service_name, service.host, service.port, db, factory);
			connection.start (connected_cb);
		});
		browser.start ();
	}
}

void debug_printf (string? log_domain,
		   GLib.LogLevelFlags log_level,
		   string? message)
{
	stdout.printf ("%s\n", message);
}

void debug_null (string? log_domain,
		 GLib.LogLevelFlags log_level,
		 string? message)
{
}

int main (string[] args) {     
	var loop = new GLib.MainLoop ();

	GLib.Log.set_handler ("libdmapsharing", GLib.LogLevelFlags.LEVEL_DEBUG, debug_null);
	GLib.Log.set_handler (null, GLib.LogLevelFlags.LEVEL_DEBUG, debug_null);

	try {
		var dmapcopy = new DPAPCopy ();
		loop.run ();
	} catch (Error e) {
		stderr.printf("Error: %s\n", e.message);
	}

	return 0;
}
