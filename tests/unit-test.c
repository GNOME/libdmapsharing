/*   FILE: unit-test.c -- Unit tests
 * AUTHOR: W. Michael Petullo <mike@flyn.org>
 *   DATE: 24 May 2011 
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

#include <check.h>
#include <glib.h>
#include <stdlib.h>
#include <libdmapsharing/dmap.h>

Suite *dmap_test_dmap_md5_suite (void);
Suite *dmap_test_daap_connection_suite (void);

static void
debug_null (const char *log_domain,
            GLogLevelFlags log_level,
            const gchar *message,
            gpointer user_data)
{
}

void run_suite (Suite *s)
{
	int nf;
	SRunner *sr;

	sr = srunner_create(s);
	srunner_run_all(sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	if (nf != 0)
		exit (EXIT_FAILURE);
}

int main(void)
{
	g_log_set_handler ("libdmapsharing", G_LOG_LEVEL_DEBUG, debug_null, NULL);
	g_log_set_handler (NULL, G_LOG_LEVEL_DEBUG, debug_null, NULL);

	run_suite (dmap_test_daap_connection_suite ());
	run_suite (dmap_test_dmap_md5_suite ());

	exit (EXIT_SUCCESS);
}
