/*
 * Copyright (C) 2006 INDT
 *  Andre Moreira Magalhaes <andre.magalhaes@indt.org.br>
 * Copyright (C) 2004,2005 Charles Schmidt <cschmidt2@emich.edu>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*
 */

#ifndef _DMAP_MD5_H__
#define _DMAP_MD5_H__

#include <glib.h>

#define DMAP_HASH_SIZE 16

G_BEGIN_DECLS

typedef struct DmapHashContext
{
        guint32 buf[4];
        guint32 bits[2];
        unsigned char in[64];
        gint version;
} DmapHashContext;

void dmap_md5_progressive_init      (DmapHashContext *context);

void dmap_md5_progressive_update    (DmapHashContext *context,
                                     unsigned char const *buffer,
                                     unsigned int length);

void dmap_md5_progressive_final     (DmapHashContext *context,
                                     unsigned char digest[16]);

void dmap_md5_progressive_to_string (const unsigned char *digest, gchar * string);

void dmap_md5_generate              (short version_major,
                                     const guchar *url,
                                     guchar hash_select,
                                     guchar *out,
                                     gint request_id);

G_END_DECLS
#endif
