/*   FILE: dacplisten.vala -- Listen to DACP remotes
 * AUTHOR: W. Michael Petullo <mike@flyn.org>
 *   DATE: 06 January 2011 
 *
 * Copyright (c) 2011 W. Michael Petullo <new@flyn.org>
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

private class ValaDacpPlayer : GLib.Object, Dmap.ControlPlayer {

	unowned Dmap.AvRecord now_playing_record () {
		stdout.printf ("Now playing record request received\n");
		return null;
	}

	unowned string now_playing_artwork (uint width, uint height) {
		stdout.printf ("Now playing artwork request received\n");
		return null;
	}

	void play_pause () {
		stdout.printf ("Play/pause request received\n");
	}

	void pause () {
		stdout.printf ("Pause request received\n");
	}

	void next_item () {
		stdout.printf ("Next item request received\n");
	}

	void prev_item () {
		stdout.printf ("Previous item request received\n");
	}

	void cue_clear () {
		stdout.printf ("Cue clear request received\n");
	}

	void cue_play (GLib.List records, uint index) {
		stdout.printf ("Cue play request received\n");
	}
}

private class DacpListener : GLib.Object {
	private Dmap.Db db;
	private Dmap.ContainerDb container_db;
	private Dmap.ControlPlayer player;
	private Dmap.ControlShare share;

	public DacpListener () {
		db = new ValaDmapDb ();
		container_db = new ValaDmapContainerDb ();
		player = new ValaDacpPlayer ();
		share = new Dmap.ControlShare ("dacplisten", player, db, container_db);

		share.remote_found.connect ((service_name, display_name) => {
			stdout.printf ("Found remote: %s, %s\n", service_name, display_name);
		});

		share.add_guid.connect ((guid) => {
			stdout.printf ("Add GUID request received\n");
		});

		share.start_lookup ();
	}
}

int main (string[] args) {     
	var loop = new GLib.MainLoop ();

	var dacplistener = new DacpListener ();

	loop.run ();

	return 0;
}
