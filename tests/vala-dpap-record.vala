/*   FILE: vala-dpap-record.vala -- A DPAPRecord implementation in Vala
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

private class ValaDPAPRecord : GLib.Object, DMAP.Record, DPAP.Record {
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

private class ValaDPAPRecordFactory : GLib.Object, DMAP.RecordFactory {
	public DMAP.Record create (void* user_data) {
		return new ValaDPAPRecord ();
	}
}
