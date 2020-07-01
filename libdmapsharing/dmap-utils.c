/*
 * Utility functions used throughout libdmapsharing
 *
 * Copyright (C) 2010 W. Michael Petullo <mike@flyn.org>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "dmap-utils.h"

gchar *
dmap_utils_mime_to_format (const gchar * transcode_mimetype)
{
	gchar *format = NULL;

	if (NULL == transcode_mimetype) {
		goto done;
	}

        if (!strcmp (transcode_mimetype, "audio/wav")) {
                format = g_strdup ("wav");
        } else if (!strcmp (transcode_mimetype, "audio/mp3")) {
                format = g_strdup ("mp3");
        } else if (!strcmp (transcode_mimetype, "video/quicktime")) {
                format = g_strdup ("mp4");
        }

done:
	return format;
}

#ifdef HAVE_CHECK

#include <check.h>

START_TEST(_mime_to_format_test_wav)
{
	ck_assert_str_eq(dmap_utils_mime_to_format("audio/wav"), "wav");
}
END_TEST

START_TEST(_mime_to_format_test_mp3)
{
	ck_assert_str_eq(dmap_utils_mime_to_format("audio/mp3"), "mp3");
}
END_TEST

START_TEST(_mime_to_format_test_quicktime)
{
	ck_assert_str_eq(dmap_utils_mime_to_format("video/quicktime"), "mp4");
}
END_TEST

START_TEST(_mime_to_format_test_null)
{
	ck_assert_ptr_eq(dmap_utils_mime_to_format(NULL), NULL);
}
END_TEST

START_TEST(_mime_to_format_test_bad)
{
	ck_assert_ptr_eq(dmap_utils_mime_to_format("bad/mime"), NULL);
}
END_TEST

#include "dmap-utils-suite.c"

#endif
