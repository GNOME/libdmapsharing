/*   FILE: vala-dmap-db.vala -- A DmapContainerDb implementation in Vala
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

private class ValaDmapContainerDb : GLib.Object, Dmap.ContainerDb {
	// A dumb database that stores everything in an array

	private Gee.ArrayList<Dmap.ContainerRecord> db = new Gee.ArrayList<Dmap.ContainerRecord> ();

	public int64 count () {
		return db.size;
	}

	public void @foreach (Dmap.IdContainerRecordFunc func) {
		int i;
		for (i = 0; i < db.size; i++) {
			func (i, db[i]);
		}
	}

	public Dmap.ContainerRecord lookup_by_id (uint id) {
		GLib.error ("lookup_by_id not implemented");
	}

	public void add (Dmap.ContainerRecord record) {
		GLib.error ("add not implemented");
	}
}
