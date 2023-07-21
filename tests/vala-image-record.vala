/*   FILE: vala-dmap-image-record.vala -- A DMAP.ImageRecord implementation in Vala
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

private class ValaImageRecord : GLib.Object, Dmap.Record, Dmap.ImageRecord {
	private string _location;
	private string _filename;
	private string _aspect_ratio;
	private string _format;
	private GLib.Array _hash;
	private string _comments;
	private GLib.Array _thumbnail;
	private int _large_filesize;
	private int _pixel_height;
	private int _pixel_width;
	private int _rating;
	private int _creation_date;

	public string location {
		owned get { return _location; }
		set { _location = value; }
	}

	public string filename {
		owned get { return _filename; }
		set { _filename = value; }
	}

	public string aspect_ratio {
		owned get { return _aspect_ratio; }
		set { _aspect_ratio = value; }
	}

	public string format {
		owned get { return _format; }
		set { _format = value; }
	}

	public  GLib.Array<void *> hash {
		owned get { return _hash; }
		set {
			_hash = new GLib.Array<void *> (false, false, 1);
			_hash.append_vals (value.data, value.data.length);
		}
	}

	public GLib.Array<void *> thumbnail {
		owned get { return _thumbnail; }
		set {
			_thumbnail = new GLib.Array<void *> (false, false, 1);
			_thumbnail.append_vals (value.data, value.data.length);
		}
	}

	public string comments {
		owned get { return _comments; }
		set { _comments = value; }
	}

	public int large_filesize {
		get { return _large_filesize; }
		set { _large_filesize = value; }
	}

	public int pixel_height {
		get { return _pixel_height; }
		set { _pixel_height = value; }
	}

	public int pixel_width {
		get { return _pixel_width; }
		set { _pixel_width = value; }
	}

	public int rating {
		get { return _rating; }
		set { _rating = value; }
	}

	public int creation_date {
		get { return _creation_date; }
		set { _creation_date = value; }
	}

	public GLib.InputStream read () throws GLib.Error {
		GLib.error ("read not implemented");
	}

	public unowned bool set_from_blob (GLib.Array<uint8> blob) {
		GLib.error ("set_from_blob not implemented");
	}

	public GLib.Array<uint8> to_blob () {
		GLib.error ("to_blob not implemented");
	}

	public ValaImageRecord () {
		_location = "file://" + GLib.Environment.get_current_dir () + "/media/test.jpeg";
		_aspect_ratio = "1.333";
		_filename = GLib.Path.get_basename (_location);
		_format = "JPEG";
		_comments = "Comments";
		_large_filesize = 13953;
		_pixel_height = 480;
		_pixel_width = 640;
		_rating = 5;
		_creation_date = 0;

		string path = GLib.Environment.get_current_dir () + "/media/test.jpeg";
		uint8[] data;

		try {
			GLib.FileUtils.get_data (path, out data);
		} catch (Error e) {
			stderr.printf("Error: %s\n", e.message);
		}

		_thumbnail = new GLib.Array<uint8> (false, false, 1);
		_thumbnail.append_vals (data, data.length);
	}
}

private class ValaImageRecordFactory : GLib.Object, Dmap.RecordFactory {
	public Dmap.Record create () {
		return new ValaImageRecord ();
	}
}
