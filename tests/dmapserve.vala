/*   FILE: dmapserve.vala -- Serve media using DMAP
 * AUTHOR: W. Michael Petullo <mike@flyn.org>
 *   DATE: 21 December 2010 
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

private class DPAPServe {
	ValaImageRecord record;
	ValaDmapDb db;
	ValaDmapContainerDb container_db;
	Dmap.ImageShare share;

	public DPAPServe () throws GLib.Error {
		record = new ValaImageRecord ();
		db = new ValaDmapDb ();
		db.add (record);
		container_db = new ValaDmapContainerDb ();
		share = new Dmap.ImageShare ("dmapserve", null, db, container_db, null);
		try {
			share.serve();
			share.publish();
		} catch (GLib.Error e) {
			GLib.error("Error starting server: %s", e.message);
		}
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

	GLib.Log.set_handler ("libdmapsharing", GLib.LogLevelFlags.LEVEL_DEBUG, debug_printf);
	GLib.Log.set_handler (null, GLib.LogLevelFlags.LEVEL_DEBUG, debug_printf);

	var dmapcopy = new DPAPServe ();

	loop.run ();

	return 0;
}
