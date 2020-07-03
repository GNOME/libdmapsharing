/*
 * Utility functions implemented using GStreamer
 *
 * Copyright (C) 2009 W. Michael Petullo <mike@flyn.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "gst-util.h"
#include "dmap-transcode-stream.h"

gboolean
gst_util_pads_compatible (GstPad * pad1, GstPad * pad2)
{
	gboolean fnval;
	GstCaps *res, *caps1, *caps2;

	caps1 = gst_pad_query_caps (pad1, NULL);
	caps2 = gst_pad_query_caps (pad2, NULL);
	res = gst_caps_intersect (caps1, caps2);
	fnval = res && !gst_caps_is_empty (res);

	gst_caps_unref (res);
	gst_caps_unref (caps2);
	gst_caps_unref (caps1);

	return fnval;
}
