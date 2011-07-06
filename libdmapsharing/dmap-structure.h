/*
 * Copyright (C) 2004,2005 Charles Schmidt <cschmidt2@emich.edu>
 * Copyright (C) 2006 INDT
 *  Andre Moreira Magalhaes <andre.magalhaes@indt.org.br>
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

#ifndef __DMAP_STRUCTURE_H__
#define __DMAP_STRUCTURE_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS typedef enum
{
	DMAP_CC_INVALID = 0,
	DMAP_RAW,		/* The RAW type does not use a content code.
				 * Instead of:
				 * CCCC BYTECOUNT DATA
				 * RAW sends:
				 * DATA
				 */
	DMAP_CC_MDCL,
	DMAP_CC_MSTT,
	DMAP_CC_MIID,
	DMAP_CC_MINM,
	DMAP_CC_MIKD,
	DMAP_CC_MPER,
	DMAP_CC_MCON,
	DMAP_CC_MCTI,
	DMAP_CC_MPCO,
	DMAP_CC_MSTS,
	DMAP_CC_MIMC,
	DMAP_CC_MCTC,
	DMAP_CC_MRCO,
	DMAP_CC_MTCO,
	DMAP_CC_MLCL,
	DMAP_CC_MLIT,
	DMAP_CC_MBCL,
	DMAP_CC_MSRV,
	DMAP_CC_MSAU,
	DMAP_CC_MSLR,
	DMAP_CC_MPRO,
	DMAP_CC_MSAL,
	DMAP_CC_MSUP,
	DMAP_CC_MSPI,
	DMAP_CC_MSEX,
	DMAP_CC_MSBR,
	DMAP_CC_MSQY,
	DMAP_CC_MSIX,
	DMAP_CC_MSRS,
	DMAP_CC_MSTM,
	DMAP_CC_MSDC,
	DMAP_CC_MCCR,
	DMAP_CC_MCNM,
	DMAP_CC_MCNA,
	DMAP_CC_MCTY,
	DMAP_CC_MLOG,
	DMAP_CC_MLID,
	DMAP_CC_MUPD,
	DMAP_CC_MUSR,
	DMAP_CC_MUTY,
	DMAP_CC_MUDL,
	DMAP_CC_MSMA,
	DMAP_CC_FQUESCH,

	DMAP_CC_APRO,
	DMAP_CC_AVDB,
	DMAP_CC_ABRO,
	DMAP_CC_ABAL,
	DMAP_CC_ABAR,
	DMAP_CC_ABCP,
	DMAP_CC_ABGN,
	DMAP_CC_ADBS,
	DMAP_CC_ASAL,
	DMAP_CC_ASAI,
	DMAP_CC_ASAA,
	DMAP_CC_ASAR,
	DMAP_CC_ASBT,
	DMAP_CC_ASBR,
	DMAP_CC_ASCM,
	DMAP_CC_ASCO,
	DMAP_CC_ASDA,
	DMAP_CC_ASDM,
	DMAP_CC_ASDC,
	DMAP_CC_ASDN,
	DMAP_CC_ASDB,
	DMAP_CC_ASEQ,
	DMAP_CC_ASFM,
	DMAP_CC_ASGN,
	DMAP_CC_ASDT,
	DMAP_CC_ASRV,
	DMAP_CC_ASSR,
	DMAP_CC_ASSZ,
	DMAP_CC_ASST,
	DMAP_CC_ASSP,
	DMAP_CC_ASTM,
	DMAP_CC_ASTC,
	DMAP_CC_ASTN,
	DMAP_CC_ASUR,
	DMAP_CC_ASYR,
	DMAP_CC_ASDK,
	DMAP_CC_ASUL,
	DMAP_CC_ASSU,
	DMAP_CC_ASSA,
	DMAP_CC_APLY,
	DMAP_CC_ABPL,
	DMAP_CC_APSO,
	DMAP_CC_PRSV,
	DMAP_CC_ARIF,
	DMAP_CC_MSAS,
	DMAP_CC_AGRP,
	DMAP_CC_AGAL,
	DMAP_CC_ASCP,
	DMAP_CC_PPRO,
	DMAP_CC_PASP,
	DMAP_CC_PFDT,
	DMAP_CC_PICD,
	DMAP_CC_PIMF,
	DMAP_CC_PFMT,
	DMAP_CC_PIFS,
	DMAP_CC_PLSZ,
	DMAP_CC_PHGT,
	DMAP_CC_PWTH,
	DMAP_CC_PRAT,
	DMAP_CC_PCMT,
	DMAP_CC_PRET,

	/* iTunes 6.02+ */
	DMAP_CC_AESV,
	DMAP_CC_AEHV,

	DMAP_CC_AESP,
	DMAP_CC_AEPP,
	DMAP_CC_AEPS,
	DMAP_CC_AESG,
	DMAP_CC_AEMK,
	DMAP_CC_AEFP,

	/* DACP */
	DMAP_CC_CMPA,
	DMAP_CC_CMNM,
	DMAP_CC_CMTY,
	DMAP_CC_CMPG,

	DMAP_CC_CACI,
	DMAP_CC_CAPS,
	DMAP_CC_CASH,
	DMAP_CC_CARP,
	DMAP_CC_CAAS,
	DMAP_CC_CAAR,
	DMAP_CC_CAIA,
	DMAP_CC_CANP,
	DMAP_CC_CANN,
	DMAP_CC_CANA,
	DMAP_CC_CANL,
	DMAP_CC_CANG,
	DMAP_CC_CANT,
	DMAP_CC_CASP,
	DMAP_CC_CASS,
	DMAP_CC_CAST,
	DMAP_CC_CASU,
	DMAP_CC_CASG,
	DMAP_CC_CACR,

	DMAP_CC_CMCP,
	DMAP_CC_CMGT,
	DMAP_CC_CMIK,
	DMAP_CC_CMSP,
	DMAP_CC_CMST,
	DMAP_CC_CMSV,
	DMAP_CC_CMSR,
	DMAP_CC_CMMK,
	DMAP_CC_CMVO,

	DMAP_CC_CMPR,
	DMAP_CC_CAPR,
	DMAP_CC_AEFR,
	DMAP_CC_CAOV,
	DMAP_CC_CMRL,
	DMAP_CC_CAHP,
	DMAP_CC_CAIV,
	DMAP_CC_CAVC
} DMAPContentCode;

typedef struct _DMAPStructureItem DMAPStructureItem;

struct _DMAPStructureItem
{
	DMAPContentCode content_code;
	GValue content;
	guint32 size;
};

GNode *dmap_structure_add (GNode * parent, DMAPContentCode cc, ...);
gchar *dmap_structure_serialize (GNode * structure, guint * length);
GNode *dmap_structure_parse (const gchar * buf, gint buf_length);
DMAPStructureItem *dmap_structure_find_item (GNode * structure,
					     DMAPContentCode code);
GNode *dmap_structure_find_node (GNode * structure, DMAPContentCode code);
void dmap_structure_print (GNode * structure);
void dmap_structure_destroy (GNode * structure);
guint dmap_structure_get_size (GNode * structure);
void dmap_structure_increase_by_predicted_size (GNode * structure,
						guint size);

typedef enum
{
	DMAP_TYPE_BYTE = 0x0001,
	DMAP_TYPE_SIGNED_INT = 0x0002,
	DMAP_TYPE_SHORT = 0x0003,
	DMAP_TYPE_INT = 0x0005,
	DMAP_TYPE_INT64 = 0x0007,
	DMAP_TYPE_STRING = 0x0009,
	DMAP_TYPE_DATE = 0x000A,
	DMAP_TYPE_VERSION = 0x000B,
	DMAP_TYPE_CONTAINER = 0x000C,
	DMAP_TYPE_POINTER = 0x002A
} DMAPType;

typedef struct _DMAPContentCodeDefinition DMAPContentCodeDefinition;

struct _DMAPContentCodeDefinition
{
	DMAPContentCode code;
	gint32 int_code;
	const gchar *name;
	const gchar *string;
	DMAPType type;
};

const DMAPContentCodeDefinition * dmap_content_codes (guint * number);
gint32 dmap_content_code_string_as_int32 (const gchar * str);
const gchar *dmap_content_code_name (DMAPContentCode code);
DMAPType dmap_content_code_dmap_type (DMAPContentCode code);
const gchar *dmap_content_code_string (DMAPContentCode code);

DMAPContentCode dmap_content_code_read_from_buffer (const gchar * buf);

G_END_DECLS
#endif
