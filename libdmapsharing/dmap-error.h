/*
 * Error definitions for DMAP sharing
 *
 * Copyright (C) 2018 W. Michael Petullo <mike@flyn.org>
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

#ifndef _DMAP_ERROR_H
#define _DMAP_ERROR_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

GQuark dmap_error_quark ();

/**
 * DMAP_ERROR:
 *
 * Error domain for DMAP library. Errors in this domain will
 * be from the #DmapError enumeration.
 * See #GError for information on error domains.
 */
#define DMAP_ERROR      dmap_error_quark ()

/**
 * DmapError:
 * @GST_CORE_ERROR_FAILED: a general error which doesn't fit in any other
 * category.  Make sure you add a custom message to the error call.
 *
 * Errors inside the libdmapsharing library.
 */
typedef enum
{
	DMAP_ERROR_FAILED = 1,
	DMAP_ERROR_NUM_ERRORS,
} DmapError;

G_END_DECLS

#endif

