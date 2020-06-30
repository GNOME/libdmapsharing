/*
 * Database class for DAAP sharing
 *
 * Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
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

#ifndef __TEST_DPAP_RECORD
#define __TEST_DPAP_RECORD

#include <libdmapsharing/dmap.h>

G_BEGIN_DECLS

#define TYPE_TEST_DPAP_RECORD		(test_dpap_record_get_type ())
#define TEST_DPAP_RECORD(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), \
					 TYPE_TEST_DPAP_RECORD, TestDPAPRecord))
#define TEST_DPAP_RECORD_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), \
					 TYPE_TEST_DPAP_RECORD, \
					 TestDPAPRecordClass))
#define IS_TEST_DPAP_RECORD(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), \
					 TYPE_TEST_DPAP_RECORD))
#define IS_TEST_DPAP_RECORD_CLASS (k)	(G_TYPE_CHECK_CLASS_TYPE ((k), \
					 TYPE_TEST_DPAP_RECORD_CLASS))
#define TEST_DPAP_RECORD_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), \
				         TYPE_TEST_DPAP_RECORD, \
				         TestDPAPRecordClass))
#define TEST_DPAP_RECORD_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
					 TYPE_TEST_DPAP_RECORD, \
					 TestDPAPRecordPrivate))

typedef struct TestDPAPRecordPrivate TestDPAPRecordPrivate;

typedef struct {
	GObject parent;
	TestDPAPRecordPrivate *priv;
} TestDPAPRecord;

typedef struct {
	GObjectClass parent;
} TestDPAPRecordClass;

GType           test_dpap_record_get_type          (void);

TestDPAPRecord *test_dpap_record_new               (void);

#endif /* __TEST_DPAP_RECORD */

G_END_DECLS
