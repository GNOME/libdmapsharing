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

#include "dmap-error.h"
#include "dmap-structure.h"
#include "dmap-private-utils.h"

#include <glib.h>
#include <glib-object.h>
#include <gobject/gvaluecollector.h>

#include <string.h>
#include <stdarg.h>

#define MAKE_CONTENT_CODE(ch0, ch1, ch2, ch3) \
    (( (gint32)(gchar)(ch0) | ( (gint32)(gchar)(ch1) << 8 ) | \
    ( (gint32)(gchar)(ch2) << 16 ) | \
    ( (gint32)(gchar)(ch3) << 24 ) ))

static const DmapContentCodeDefinition _cc_defs[] = {
	{DMAP_RAW, 0, "", "", DMAP_TYPE_STRING},
	{DMAP_CC_MDCL, MAKE_CONTENT_CODE ('m', 'd', 'c', 'l'),
	 "dmap.dictionary", "mdcl", DMAP_TYPE_CONTAINER},
	{DMAP_CC_MEDS, MAKE_CONTENT_CODE ('m', 'e', 'd', 's'),
	 "dmap.editcommandssupported", "meds", DMAP_TYPE_INT},
	{DMAP_CC_MSTT, MAKE_CONTENT_CODE ('m', 's', 't', 't'), "dmap.status",
	 "mstt", DMAP_TYPE_INT},
	{DMAP_CC_MIID, MAKE_CONTENT_CODE ('m', 'i', 'i', 'd'), "dmap.itemid",
	 "miid", DMAP_TYPE_INT},
	{DMAP_CC_MINM, MAKE_CONTENT_CODE ('m', 'i', 'n', 'm'),
	 "dmap.itemname", "minm", DMAP_TYPE_STRING},
	{DMAP_CC_MIKD, MAKE_CONTENT_CODE ('m', 'i', 'k', 'd'),
	 "dmap.itemkind", "mikd", DMAP_TYPE_BYTE},
	{DMAP_CC_MPER, MAKE_CONTENT_CODE ('m', 'p', 'e', 'r'),
	 "dmap.persistentid", "mper", DMAP_TYPE_INT64},
	{DMAP_CC_MCON, MAKE_CONTENT_CODE ('m', 'c', 'o', 'n'),
	 "dmap.container", "mcon", DMAP_TYPE_CONTAINER},
	{DMAP_CC_MCTI, MAKE_CONTENT_CODE ('m', 'c', 't', 'i'),
	 "dmap.containeritemid", "mcti", DMAP_TYPE_INT},
	{DMAP_CC_MPCO, MAKE_CONTENT_CODE ('m', 'p', 'c', 'o'),
	 "dmap.parentcontainerid", "mpco", DMAP_TYPE_INT},
	{DMAP_CC_MSTS, MAKE_CONTENT_CODE ('m', 's', 't', 's'),
	 "dmap.statusstring", "msts", DMAP_TYPE_STRING},
	{DMAP_CC_MIMC, MAKE_CONTENT_CODE ('m', 'i', 'm', 'c'),
	 "dmap.itemcount", "mimc", DMAP_TYPE_INT},
	{DMAP_CC_MCTC, MAKE_CONTENT_CODE ('m', 'c', 't', 'c'),
	 "dmap.containercount", "mctc", DMAP_TYPE_INT},
	{DMAP_CC_MRCO, MAKE_CONTENT_CODE ('m', 'r', 'c', 'o'),
	 "dmap.returnedcount", "mrco", DMAP_TYPE_INT},
	{DMAP_CC_MTCO, MAKE_CONTENT_CODE ('m', 't', 'c', 'o'),
	 "dmap.specifiedtotalcount", "mtco", DMAP_TYPE_INT},
	{DMAP_CC_MLCL, MAKE_CONTENT_CODE ('m', 'l', 'c', 'l'), "dmap.listing",
	 "mlcl", DMAP_TYPE_CONTAINER},
	{DMAP_CC_MLIT, MAKE_CONTENT_CODE ('m', 'l', 'i', 't'),
	 "dmap.listingitem", "mlit", DMAP_TYPE_CONTAINER},
	{DMAP_CC_MBCL, MAKE_CONTENT_CODE ('m', 'b', 'c', 'l'), "dmap.bag",
	 "mbcl", DMAP_TYPE_CONTAINER},
	{DMAP_CC_MSRV, MAKE_CONTENT_CODE ('m', 's', 'r', 'v'),
	 "dmap.serverinforesponse", "msrv", DMAP_TYPE_CONTAINER},
	{DMAP_CC_MSAU, MAKE_CONTENT_CODE ('m', 's', 'a', 'u'),
	 "dmap.authenticationmethod", "msau", DMAP_TYPE_BYTE},
	{DMAP_CC_MSLR, MAKE_CONTENT_CODE ('m', 's', 'l', 'r'),
	 "dmap.loginrequired", "mslr", DMAP_TYPE_BYTE},
	{DMAP_CC_MPRO, MAKE_CONTENT_CODE ('m', 'p', 'r', 'o'),
	 "dmap.protocolversion", "mpro", DMAP_TYPE_VERSION},
	{DMAP_CC_MSAL, MAKE_CONTENT_CODE ('m', 's', 'a', 'l'),
	 "dmap.supportsautologout", "msal", DMAP_TYPE_BYTE},
	{DMAP_CC_MSUP, MAKE_CONTENT_CODE ('m', 's', 'u', 'p'),
	 "dmap.supportsupdate", "msup", DMAP_TYPE_BYTE},
	{DMAP_CC_MSPI, MAKE_CONTENT_CODE ('m', 's', 'p', 'i'),
	 "dmap.supportspersistenids", "mspi", DMAP_TYPE_BYTE},
	{DMAP_CC_MSEX, MAKE_CONTENT_CODE ('m', 's', 'e', 'x'),
	 "dmap.supportsextensions", "msex", DMAP_TYPE_BYTE},
	{DMAP_CC_MSBR, MAKE_CONTENT_CODE ('m', 's', 'b', 'r'),
	 "dmap.supportsbrowse", "msbr", DMAP_TYPE_BYTE},
	{DMAP_CC_MSQY, MAKE_CONTENT_CODE ('m', 's', 'q', 'y'),
	 "dmap.supportsquery", "msqy", DMAP_TYPE_BYTE},
	{DMAP_CC_MSIX, MAKE_CONTENT_CODE ('m', 's', 'i', 'x'),
	 "dmap.supportsindex", "msix", DMAP_TYPE_BYTE},
	{DMAP_CC_MSRS, MAKE_CONTENT_CODE ('m', 's', 'r', 's'),
	 "dmap.supportsresolve", "msrs", DMAP_TYPE_BYTE},
	{DMAP_CC_MSTM, MAKE_CONTENT_CODE ('m', 's', 't', 'm'),
	 "dmap.timeoutinterval", "mstm", DMAP_TYPE_INT},
	{DMAP_CC_MSDC, MAKE_CONTENT_CODE ('m', 's', 'd', 'c'),
	 "dmap.databasescount", "msdc", DMAP_TYPE_INT},
	{DMAP_CC_MCCR, MAKE_CONTENT_CODE ('m', 'c', 'c', 'r'),
	 "dmap.contentcodesresponse", "mccr", DMAP_TYPE_CONTAINER},
	{DMAP_CC_MCNM, MAKE_CONTENT_CODE ('m', 'c', 'n', 'm'),
	 "dmap.contentcodesnumber", "mcnm", DMAP_TYPE_INT},
	{DMAP_CC_MCNA, MAKE_CONTENT_CODE ('m', 'c', 'n', 'a'),
	 "dmap.contentcodesname", "mcna", DMAP_TYPE_STRING},
	{DMAP_CC_MCTY, MAKE_CONTENT_CODE ('m', 'c', 't', 'y'),
	 "dmap.contentcodestype", "mcty", DMAP_TYPE_SHORT},
	{DMAP_CC_MLOG, MAKE_CONTENT_CODE ('m', 'l', 'o', 'g'),
	 "dmap.loginresponse", "mlog", DMAP_TYPE_CONTAINER},
	{DMAP_CC_MLID, MAKE_CONTENT_CODE ('m', 'l', 'i', 'd'),
	 "dmap.sessionid", "mlid", DMAP_TYPE_INT},
	{DMAP_CC_MUPD, MAKE_CONTENT_CODE ('m', 'u', 'p', 'd'),
	 "dmap.updateresponse", "mupd", DMAP_TYPE_CONTAINER},
	{DMAP_CC_MUSR, MAKE_CONTENT_CODE ('m', 'u', 's', 'r'),
	 "dmap.serverrevision", "musr", DMAP_TYPE_INT},
	{DMAP_CC_MUTY, MAKE_CONTENT_CODE ('m', 'u', 't', 'y'),
	 "dmap.updatetype", "muty", DMAP_TYPE_BYTE},
	{DMAP_CC_MUDL, MAKE_CONTENT_CODE ('m', 'u', 'd', 'l'),
	 "dmap.deletedidlisting", "mudl", DMAP_TYPE_CONTAINER},
	{DMAP_CC_MSMA, MAKE_CONTENT_CODE ('m', 's', 'm', 'a'),
	 "dmap.speakermachineaddress", "msma", DMAP_TYPE_INT},
	{DMAP_CC_FQUESCH, MAKE_CONTENT_CODE ('f', '?', 'c', 'h'),
	 "dmap.haschildcontainers", "f?ch", DMAP_TYPE_BYTE},
	{DMAP_CC_MDBK, MAKE_CONTENT_CODE ('m', 'd', 'b', 'k'),
	 "dmap.databasekind", "mdbk", DMAP_TYPE_INT},

	{DMAP_CC_APRO, MAKE_CONTENT_CODE ('a', 'p', 'r', 'o'),
	 "daap.protocolversion", "apro", DMAP_TYPE_VERSION},
	{DMAP_CC_AVDB, MAKE_CONTENT_CODE ('a', 'v', 'd', 'b'),
	 "daap.serverdatabases", "avdb", DMAP_TYPE_CONTAINER},
	{DMAP_CC_ABRO, MAKE_CONTENT_CODE ('a', 'b', 'r', 'o'),
	 "daap.databasebrowse", "abro", DMAP_TYPE_CONTAINER},
	{DMAP_CC_ABAL, MAKE_CONTENT_CODE ('a', 'b', 'a', 'l'),
	 "daap.browsealbumlisting", "abal", DMAP_TYPE_CONTAINER},
	{DMAP_CC_ABAR, MAKE_CONTENT_CODE ('a', 'b', 'a', 'r'),
	 "daap.browseartistlisting", "abar", DMAP_TYPE_CONTAINER},
	{DMAP_CC_ABCP, MAKE_CONTENT_CODE ('a', 'b', 'c', 'p'),
	 "daap.browsecomposerlisting", "abcp", DMAP_TYPE_CONTAINER},
	{DMAP_CC_ABGN, MAKE_CONTENT_CODE ('a', 'b', 'g', 'n'),
	 "daap.browsegenrelisting", "abgn", DMAP_TYPE_CONTAINER},
	{DMAP_CC_ADBS, MAKE_CONTENT_CODE ('a', 'd', 'b', 's'),
	 "daap.returndatabasesongs", "adbs", DMAP_TYPE_CONTAINER},
	{DMAP_CC_ASAL, MAKE_CONTENT_CODE ('a', 's', 'a', 'l'),
	 "daap.songalbum", "asal", DMAP_TYPE_STRING},
	{DMAP_CC_ASAI, MAKE_CONTENT_CODE ('a', 's', 'a', 'i'),
	 "daap.songalbumid", "asai", DMAP_TYPE_INT},
	{DMAP_CC_ASAA, MAKE_CONTENT_CODE ('a', 's', 'a', 'a'),
	 "daap.songalbumartist", "asaa", DMAP_TYPE_STRING},
	{DMAP_CC_ASAR, MAKE_CONTENT_CODE ('a', 's', 'a', 'r'),
	 "daap.songartist", "asar", DMAP_TYPE_STRING},
	{DMAP_CC_ASBT, MAKE_CONTENT_CODE ('a', 's', 'b', 't'),
	 "daap.songsbeatsperminute", "asbt", DMAP_TYPE_SHORT},
	{DMAP_CC_ASBR, MAKE_CONTENT_CODE ('a', 's', 'b', 'r'),
	 "daap.songbitrate", "asbr", DMAP_TYPE_SHORT},
	{DMAP_CC_ASCM, MAKE_CONTENT_CODE ('a', 's', 'c', 'm'),
	 "daap.songcomment", "ascm", DMAP_TYPE_STRING},
	{DMAP_CC_ASCO, MAKE_CONTENT_CODE ('a', 's', 'c', 'o'),
	 "daap.songcompliation", "asco", DMAP_TYPE_BYTE},
	{DMAP_CC_ASDA, MAKE_CONTENT_CODE ('a', 's', 'd', 'a'),
	 "daap.songdateadded", "asda", DMAP_TYPE_DATE},
	{DMAP_CC_ASDM, MAKE_CONTENT_CODE ('a', 's', 'd', 'm'),
	 "daap.songdatemodified", "asdm", DMAP_TYPE_DATE},
	{DMAP_CC_ASDC, MAKE_CONTENT_CODE ('a', 's', 'd', 'c'),
	 "daap.songdisccount", "asdc", DMAP_TYPE_SHORT},
	{DMAP_CC_ASDN, MAKE_CONTENT_CODE ('a', 's', 'd', 'n'),
	 "daap.songdiscnumber", "asdn", DMAP_TYPE_SHORT},
	{DMAP_CC_ASDB, MAKE_CONTENT_CODE ('a', 's', 'd', 'b'),
	 "daap.songdisabled", "asdb", DMAP_TYPE_BYTE},
	{DMAP_CC_ASEQ, MAKE_CONTENT_CODE ('a', 's', 'e', 'q'),
	 "daap.songeqpreset", "aseq", DMAP_TYPE_STRING},
	{DMAP_CC_ASFM, MAKE_CONTENT_CODE ('a', 's', 'f', 'm'),
	 "daap.songformat", "asfm", DMAP_TYPE_STRING},
	{DMAP_CC_ASGN, MAKE_CONTENT_CODE ('a', 's', 'g', 'n'),
	 "daap.songgenre", "asgn", DMAP_TYPE_STRING},
	{DMAP_CC_ASDT, MAKE_CONTENT_CODE ('a', 's', 'd', 't'),
	 "daap.songdescription", "asdt", DMAP_TYPE_STRING},
	{DMAP_CC_ASRV, MAKE_CONTENT_CODE ('a', 's', 'r', 'v'),
	 "daap.songrelativevolume", "asrv", DMAP_TYPE_SIGNED_INT},
	{DMAP_CC_ASSR, MAKE_CONTENT_CODE ('a', 's', 's', 'r'),
	 "daap.songsamplerate", "assr", DMAP_TYPE_INT},
	{DMAP_CC_ASSZ, MAKE_CONTENT_CODE ('a', 's', 's', 'z'),
	 "daap.songsize", "assz", DMAP_TYPE_INT},
	{DMAP_CC_ASST, MAKE_CONTENT_CODE ('a', 's', 's', 't'),
	 "daap.songstarttime", "asst", DMAP_TYPE_INT},
	{DMAP_CC_ASSP, MAKE_CONTENT_CODE ('a', 's', 's', 'p'),
	 "daap.songstoptime", "assp", DMAP_TYPE_INT},
	{DMAP_CC_ASTM, MAKE_CONTENT_CODE ('a', 's', 't', 'm'),
	 "daap.songtime", "astm", DMAP_TYPE_INT},
	{DMAP_CC_ASTC, MAKE_CONTENT_CODE ('a', 's', 't', 'c'),
	 "daap.songtrackcount", "astc", DMAP_TYPE_SHORT},
	{DMAP_CC_ASTN, MAKE_CONTENT_CODE ('a', 's', 't', 'n'),
	 "daap.songtracknumber", "astn", DMAP_TYPE_SHORT},
	{DMAP_CC_ASUR, MAKE_CONTENT_CODE ('a', 's', 'u', 'r'),
	 "daap.songuserrating", "asur", DMAP_TYPE_BYTE},
	{DMAP_CC_ASYR, MAKE_CONTENT_CODE ('a', 's', 'y', 'r'),
	 "daap.songyear", "asyr", DMAP_TYPE_SHORT},
	{DMAP_CC_ASDK, MAKE_CONTENT_CODE ('a', 's', 'd', 'k'),
	 "daap.songdatakind", "asdk", DMAP_TYPE_BYTE},
	{DMAP_CC_ASUL, MAKE_CONTENT_CODE ('a', 's', 'u', 'l'),
	 "daap.songdataurl", "asul", DMAP_TYPE_STRING},
	{DMAP_CC_ASSU, MAKE_CONTENT_CODE ('a', 's', 's', 'u'),
	 "daap.sortalbum", "assu", DMAP_TYPE_STRING},
	{DMAP_CC_ASSA, MAKE_CONTENT_CODE ('a', 's', 's', 'a'),
	 "daap.sortartist", "assa", DMAP_TYPE_STRING},
	{DMAP_CC_APLY, MAKE_CONTENT_CODE ('a', 'p', 'l', 'y'),
	 "daap.databaseplaylists", "aply", DMAP_TYPE_CONTAINER},
	{DMAP_CC_ABPL, MAKE_CONTENT_CODE ('a', 'b', 'p', 'l'),
	 "daap.baseplaylist", "abpl", DMAP_TYPE_BYTE},
	{DMAP_CC_APSO, MAKE_CONTENT_CODE ('a', 'p', 's', 'o'),
	 "daap.playlistsongs", "apso", DMAP_TYPE_CONTAINER},
	{DMAP_CC_PRSV, MAKE_CONTENT_CODE ('p', 'r', 's', 'v'), "daap.resolve",
	 "prsv", DMAP_TYPE_CONTAINER},
	{DMAP_CC_ARIF, MAKE_CONTENT_CODE ('a', 'r', 'i', 'f'),
	 "daap.resolveinfo", "arif", DMAP_TYPE_CONTAINER},
	{DMAP_CC_MSAS, MAKE_CONTENT_CODE ('m', 's', 'a', 's'),
	 "daap.authentication.schemes", "msas", DMAP_TYPE_BYTE},
	{DMAP_CC_AGRP, MAKE_CONTENT_CODE ('a', 'g', 'r', 'p'),
	 "daap.songgrouping", "agrp", DMAP_TYPE_STRING},
	{DMAP_CC_AGAL, MAKE_CONTENT_CODE ('a', 'g', 'a', 'l'),
	 "daap.albumgrouping", "agal", DMAP_TYPE_CONTAINER},
	{DMAP_CC_ASCP, MAKE_CONTENT_CODE ('a', 's', 'c', 'p'),
	 "daap.songcomposer", "ascp", DMAP_TYPE_STRING},
	{DMAP_CC_PPRO, MAKE_CONTENT_CODE ('p', 'p', 'r', 'o'),
	 "dpap.protocolversion", "ppro", DMAP_TYPE_VERSION},
	{DMAP_CC_PASP, MAKE_CONTENT_CODE ('p', 'a', 's', 'p'),
	 "dpap.aspectratio", "pasp", DMAP_TYPE_STRING},
	{DMAP_CC_PFDT, MAKE_CONTENT_CODE ('p', 'f', 'd', 't'),
	 "dpap.filedata", "pfdt", DMAP_TYPE_POINTER},
	{DMAP_CC_PICD, MAKE_CONTENT_CODE ('p', 'i', 'c', 'd'),
	 "dpap.creationdate", "picd", DMAP_TYPE_INT},
	{DMAP_CC_PIMF, MAKE_CONTENT_CODE ('p', 'i', 'm', 'f'),
	 "dpap.imagefilename", "pimf", DMAP_TYPE_STRING},
	{DMAP_CC_PFMT, MAKE_CONTENT_CODE ('p', 'f', 'm', 't'),
	 "dpap.imageformat", "pfmt", DMAP_TYPE_STRING},
	{DMAP_CC_PIFS, MAKE_CONTENT_CODE ('p', 'i', 'f', 's'),
	 "dpap.imagefilesize", "pifs", DMAP_TYPE_INT},
	{DMAP_CC_PLSZ, MAKE_CONTENT_CODE ('p', 'l', 's', 'z'),
	 "dpap.imagelargefilesize", "plsz", DMAP_TYPE_INT},
	{DMAP_CC_PHGT, MAKE_CONTENT_CODE ('p', 'h', 'g', 't'),
	 "dpap.imagepixelheight", "phgt", DMAP_TYPE_INT},
	{DMAP_CC_PWTH, MAKE_CONTENT_CODE ('p', 'w', 't', 'h'),
	 "dpap.imagepixelwidth", "pwth", DMAP_TYPE_INT},
	{DMAP_CC_PRAT, MAKE_CONTENT_CODE ('p', 'r', 'a', 't'),
	 "dpap.imagerating", "prat", DMAP_TYPE_INT},
	{DMAP_CC_PCMT, MAKE_CONTENT_CODE ('p', 'c', 'm', 't'),
	 "dpap.imagecomments", "pcmt", DMAP_TYPE_STRING},
	{DMAP_CC_PRET, MAKE_CONTENT_CODE ('p', 'r', 'e', 't'), "dpap.pret",
	 "pret", DMAP_TYPE_STRING},
	{DMAP_CC_AECS, MAKE_CONTENT_CODE ('a', 'e', 'C', 's'),
	 "com.apple.itunes.artworkchecksum", "aeCs", DMAP_TYPE_INT},
	{DMAP_CC_AESV, MAKE_CONTENT_CODE ('a', 'e', 'S', 'V'),
	 "com.apple.itunes.music-sharing-version", "aesv", DMAP_TYPE_INT},
	{DMAP_CC_AEHV, MAKE_CONTENT_CODE ('a', 'e', 'H', 'V'),
	 "com.apple.itunes.has-video", "aeHV", DMAP_TYPE_BYTE},
	{DMAP_CC_AESP, MAKE_CONTENT_CODE ('a', 'e', 'S', 'P'),
	 "com.apple.itunes.smart-playlist", "aeSP", DMAP_TYPE_BYTE},
	{DMAP_CC_AEPP, MAKE_CONTENT_CODE ('a', 'e', 'P', 'P'),
	 "com.apple.itunes.is-podcast-playlist", "aePP", DMAP_TYPE_BYTE},
	{DMAP_CC_AEPS, MAKE_CONTENT_CODE ('a', 'e', 'P', 'S'),
	 "com.apple.itunes.special-playlist", "aePS", DMAP_TYPE_BYTE},
	{DMAP_CC_AESG, MAKE_CONTENT_CODE ('a', 'e', 'S', 'G'),
	 "com.apple.itunes.saved-genius", "aeSG", DMAP_TYPE_BYTE},
	{DMAP_CC_AEMK, MAKE_CONTENT_CODE ('a', 'e', 'M', 'K'),
	 "com.apple.itunes.mediakind", "aeMK", DMAP_TYPE_BYTE},
	{DMAP_CC_AEMK2, MAKE_CONTENT_CODE ('a', 'e', 'M', 'k'),
	 "com.apple.itunes.extended-media-kind", "aeMk", DMAP_TYPE_INT},
	{DMAP_CC_AEFP, MAKE_CONTENT_CODE ('a', 'e', 'F', 'P'),
	 "com.apple.itunes.req-fplay", "aeFP", DMAP_TYPE_BYTE},
	{DMAP_CC_ATED, MAKE_CONTENT_CODE ('a', 't', 'e', 'd'),
	 "daap.supportsextradata", "ated", DMAP_TYPE_SHORT},
	{DMAP_CC_ASGR, MAKE_CONTENT_CODE ('a', 's', 'g', 'r'),
	 "daap.supportsgroups", "asgr", DMAP_TYPE_SHORT},
	{DMAP_CC_AEMQ, MAKE_CONTENT_CODE ('a', 'e', 'M', 'Q'),
	 "com.apple.itunes.unknown-MQ", "aeMQ", DMAP_TYPE_BYTE},
	{DMAP_CC_AESL, MAKE_CONTENT_CODE ('a', 'e', 'S', 'L'),
	 "com.apple.itunes.unknown-SL", "aeSL", DMAP_TYPE_BYTE},
	{DMAP_CC_AESR, MAKE_CONTENT_CODE ('a', 'e', 'S', 'R'),
	 "com.apple.itunes.unknown-SR", "aeSR", DMAP_TYPE_BYTE},
	{DMAP_CC_AETR, MAKE_CONTENT_CODE ('a', 'e', 'T', 'r'),
	 "com.apple.itunes.unknown-Tr", "aeTr", DMAP_TYPE_BYTE},
	{DMAP_CC_MSED, MAKE_CONTENT_CODE ('m', 's', 'e', 'd'),
	 "com.apple.itunes.unknown-ed", "msed", DMAP_TYPE_BYTE},

	/* DACP */
	{DMAP_CC_CMPA, MAKE_CONTENT_CODE ('c', 'm', 'p', 'a'),
	 "dacp.contentcontainer", "cmpa", DMAP_TYPE_CONTAINER},
	{DMAP_CC_CMNM, MAKE_CONTENT_CODE ('c', 'm', 'n', 'm'),
	 "dacp.contentname", "cmnm", DMAP_TYPE_STRING},
	{DMAP_CC_CMTY, MAKE_CONTENT_CODE ('c', 'm', 't', 'y'),
	 "dacp.contentvalue", "cmty", DMAP_TYPE_STRING},
	{DMAP_CC_CMPG, MAKE_CONTENT_CODE ('c', 'm', 'p', 'g'),
	 "dacp.passguid", "cmpg", DMAP_TYPE_INT64},

	{DMAP_CC_CACI, MAKE_CONTENT_CODE ('c', 'a', 'c', 'i'),
	 "dacp.controlint", "caci", DMAP_TYPE_CONTAINER},
	{DMAP_CC_CAPS, MAKE_CONTENT_CODE ('c', 'a', 'p', 's'),
	 "dacp.playstatus", "caps", DMAP_TYPE_BYTE},
	{DMAP_CC_CASH, MAKE_CONTENT_CODE ('c', 'a', 's', 'h'),
	 "dacp.shufflestate", "cash", DMAP_TYPE_BYTE},
	{DMAP_CC_CARP, MAKE_CONTENT_CODE ('c', 'a', 'r', 'p'),
	 "dacp.repeatstate", "carp", DMAP_TYPE_BYTE},
	{DMAP_CC_CAAS, MAKE_CONTENT_CODE ('c', 'a', 'a', 's'),
	 "dacp.albumshuffle", "caas", DMAP_TYPE_INT},
	{DMAP_CC_CAAR, MAKE_CONTENT_CODE ('c', 'a', 'a', 'r'),
	 "dacp.albumrepeat", "caar", DMAP_TYPE_INT},
	{DMAP_CC_CAIA, MAKE_CONTENT_CODE ('c', 'a', 'i', 'a'),
	 "dacp.isavailiable", "caia", DMAP_TYPE_BYTE},
	{DMAP_CC_CANP, MAKE_CONTENT_CODE ('c', 'a', 'n', 'p'),
	 "dacp.nowplaying", "canp", DMAP_TYPE_INT64},
	{DMAP_CC_CANN, MAKE_CONTENT_CODE ('c', 'a', 'n', 'n'),
	 "dacp.nowplayingtrack", "cann", DMAP_TYPE_STRING},
	{DMAP_CC_CANA, MAKE_CONTENT_CODE ('c', 'a', 'n', 'a'),
	 "dacp.nowplayingartist", "cana", DMAP_TYPE_STRING},
	{DMAP_CC_CANL, MAKE_CONTENT_CODE ('c', 'a', 'n', 'l'),
	 "dacp.nowplayingalbum", "canl", DMAP_TYPE_STRING},
	{DMAP_CC_CANG, MAKE_CONTENT_CODE ('c', 'a', 'n', 'g'),
	 "dacp.nowplayinggenre", "cang", DMAP_TYPE_STRING},
	{DMAP_CC_CANT, MAKE_CONTENT_CODE ('c', 'a', 'n', 't'),
	 "dacp.remainingtime", "cant", DMAP_TYPE_INT},
	{DMAP_CC_CASP, MAKE_CONTENT_CODE ('c', 'a', 's', 'p'),
	 "dacp.speakers", "casp", DMAP_TYPE_CONTAINER},
	{DMAP_CC_CASS, MAKE_CONTENT_CODE ('c', 'a', 's', 's'), "dacp.ss",
	 "cass", DMAP_TYPE_BYTE},
	{DMAP_CC_CAST, MAKE_CONTENT_CODE ('c', 'a', 's', 't'),
	 "dacp.tracklength", "cast", DMAP_TYPE_INT},
	{DMAP_CC_CASU, MAKE_CONTENT_CODE ('c', 'a', 's', 'u'), "dacp.su",
	 "casu", DMAP_TYPE_BYTE},
	{DMAP_CC_CASG, MAKE_CONTENT_CODE ('c', 'a', 's', 'g'), "dacp.sg",
	 "caSG", DMAP_TYPE_BYTE},
	{DMAP_CC_CACR, MAKE_CONTENT_CODE ('c', 'a', 'c', 'r'), "dacp.cacr",
	 "cacr", DMAP_TYPE_CONTAINER},

	{DMAP_CC_CMCP, MAKE_CONTENT_CODE ('c', 'm', 'c', 'p'),
	 "dmcp.controlprompt", "cmcp", DMAP_TYPE_CONTAINER},
	{DMAP_CC_CMGT, MAKE_CONTENT_CODE ('c', 'm', 'g', 't'),
	 "dmcp.getpropertyresponse", "cmgt", DMAP_TYPE_CONTAINER},
	{DMAP_CC_CMIK, MAKE_CONTENT_CODE ('c', 'm', 'i', 'k'), "dmcp.ik",
	 "cmik", DMAP_TYPE_BYTE},
	{DMAP_CC_CMSP, MAKE_CONTENT_CODE ('c', 'm', 's', 'p'), "dmcp.ik",
	 "cmsp", DMAP_TYPE_BYTE},
	{DMAP_CC_CMST, MAKE_CONTENT_CODE ('c', 'm', 's', 't'), "dmcp.status",
	 "cmst", DMAP_TYPE_CONTAINER},
	{DMAP_CC_CMSV, MAKE_CONTENT_CODE ('c', 'm', 's', 'v'), "dmcp.sv",
	 "cmsv", DMAP_TYPE_BYTE},
	{DMAP_CC_CMSR, MAKE_CONTENT_CODE ('c', 'm', 's', 'r'),
	 "dmcp.mediarevision", "cmsr", DMAP_TYPE_INT},
	{DMAP_CC_CMMK, MAKE_CONTENT_CODE ('c', 'm', 'm', 'k'),
	 "dmcp.mediakind", "cmmk", DMAP_TYPE_INT},
	{DMAP_CC_CMVO, MAKE_CONTENT_CODE ('c', 'm', 'v', 'o'), "dmcp.volume",
	 "cmvo", DMAP_TYPE_INT},
	 
	{DMAP_CC_CMPR, MAKE_CONTENT_CODE ('c', 'm', 'p', 'r'), "dacp.unknown",
	 "cmpr", DMAP_TYPE_INT},
	{DMAP_CC_CAPR, MAKE_CONTENT_CODE ('c', 'a', 'p', 'r'), "dacp.unknown",
	 "capr", DMAP_TYPE_INT},
	{DMAP_CC_AEFR, MAKE_CONTENT_CODE ('a', 'e', 'F', 'R'), "dacp.unknown",
	 "aeFR", DMAP_TYPE_BYTE},
	{DMAP_CC_CAOV, MAKE_CONTENT_CODE ('c', 'a', 'o', 'v'), "dacp.unknown",
	 "caov", DMAP_TYPE_BYTE},
	{DMAP_CC_CMRL, MAKE_CONTENT_CODE ('c', 'm', 'r', 'l'), "dacp.unknown",
	 "cmrl", DMAP_TYPE_BYTE},
	{DMAP_CC_CAHP, MAKE_CONTENT_CODE ('c', 'a', 'h', 'p'), "dacp.unknown",
	 "cahp", DMAP_TYPE_BYTE},
	{DMAP_CC_CAIV, MAKE_CONTENT_CODE ('c', 'a', 'i', 'v'), "dacp.unknown",
	 "caiv", DMAP_TYPE_BYTE},
	{DMAP_CC_CAVC, MAKE_CONTENT_CODE ('c', 'a', 'v', 'c'), "dacp.unknwon",
	 "cavc", DMAP_TYPE_BYTE},

};

static const gchar *
_cc_name (DmapContentCode code)
{
	return _cc_defs[code - 1].name;
}

static DmapType
_cc_dmap_type (DmapContentCode code, GError **error)
{
	DmapType type = DMAP_TYPE_INVALID;

	if (code < sizeof _cc_defs / sizeof(DmapContentCodeDefinition)) {
		type = _cc_defs[code - 1].type;
	} else {
		g_set_error(error, DMAP_ERROR, DMAP_STATUS_INVALID_CONTENT_CODE,
			   "Invalid content code: %d", code);
	}

	return type;
}

static const gchar *
_cc_string (DmapContentCode code)
{
	return _cc_defs[code - 1].string;
}

static GType
_cc_gtype (DmapContentCode code, GError **error)
{
	GType type = G_TYPE_NONE;

	switch (_cc_dmap_type (code, error)) {
	case DMAP_TYPE_BYTE:
	case DMAP_TYPE_SIGNED_INT:
		type = G_TYPE_CHAR;
		break;
	case DMAP_TYPE_SHORT:
	case DMAP_TYPE_INT:
	case DMAP_TYPE_DATE:
		type = G_TYPE_INT;
		break;
	case DMAP_TYPE_INT64:
		type = G_TYPE_INT64;
		break;
	case DMAP_TYPE_VERSION:
		type = G_TYPE_DOUBLE;
		break;
	case DMAP_TYPE_STRING:
		type = G_TYPE_STRING;
		break;
	case DMAP_TYPE_POINTER:
		type = G_TYPE_POINTER;
		break;
	case DMAP_TYPE_CONTAINER:
	case DMAP_TYPE_INVALID:
	default:
		type = G_TYPE_NONE;
		break;
	}

	return type;
}

static gboolean
_node_serialize (GNode * node, GByteArray * array)
{
	DmapStructureItem *item = node->data;
	DmapType dmap_type;
	guint32 size = GINT32_TO_BE (item->size);

	if (item->content_code != DMAP_RAW) {
		g_byte_array_append (array,
				     (const guint8 *)
				     _cc_string (item->
							       content_code),
				     4);
		g_byte_array_append (array, (const guint8 *) &size, 4);
	}

	dmap_type = _cc_dmap_type (item->content_code, NULL);

	switch (dmap_type) {
	case DMAP_TYPE_BYTE:
	case DMAP_TYPE_SIGNED_INT:{
			gchar c = g_value_get_schar (&(item->content));

			g_byte_array_append (array, (const guint8 *) &c, 1);

			break;
		}
	case DMAP_TYPE_SHORT:{
			gint32 i = g_value_get_int (&(item->content));
			gint16 s = GINT16_TO_BE ((gint16) i);

			g_byte_array_append (array, (const guint8 *) &s, 2);

			break;
		}
	case DMAP_TYPE_DATE:
	case DMAP_TYPE_INT:{
			gint32 i = g_value_get_int (&(item->content));
			gint32 s = GINT32_TO_BE (i);

			g_byte_array_append (array, (const guint8 *) &s, 4);

			break;
		}
	case DMAP_TYPE_VERSION:{
			gdouble v = g_value_get_double (&(item->content));
			gint16 major;
			gint8 minor;
			gint8 patch = 0;

			major = (gint16) v;
			minor = (gint8) (v - ((gdouble) major));

			major = GINT16_TO_BE (major);

			g_byte_array_append (array, (const guint8 *) &major,
					     2);
			g_byte_array_append (array, (const guint8 *) &minor,
					     1);
			g_byte_array_append (array, (const guint8 *) &patch,
					     1);

			break;
		}
	case DMAP_TYPE_INT64:{
			gint64 i = g_value_get_int64 (&(item->content));
			gint64 s = GINT64_TO_BE (i);

			g_byte_array_append (array, (const guint8 *) &s, 8);

			break;
		}
	case DMAP_TYPE_STRING:{
			const gchar *s =
				g_value_get_string (&(item->content));

			g_byte_array_append (array, (const guint8 *) s,
					     strlen (s));

			break;
		}
	case DMAP_TYPE_POINTER:{
			const gpointer *data =
				g_value_get_pointer (&(item->content));

			g_byte_array_append (array, (const guint8 *) data,
					     item->size);

			break;
		}
	case DMAP_TYPE_CONTAINER:
	case DMAP_TYPE_INVALID:
	default:
		break;
	}

	return FALSE;
}

gchar *
dmap_structure_serialize (GNode * structure, guint * length)
{
	GByteArray *array;
	gchar *data;

	array = g_byte_array_new ();

	if (structure) {
		g_node_traverse (structure, G_PRE_ORDER, G_TRAVERSE_ALL, -1,
				 (GNodeTraverseFunc)
				 _node_serialize, array);
	}

	data = (gchar *) array->data;
	*length = array->len;
	g_byte_array_free (array, FALSE);

	return data;
}

static DmapContentCode
_cc_read_from_buffer (const gchar * buf, GError **error)
{
	DmapContentCode cc = DMAP_CC_INVALID;

	gint32 c = MAKE_CONTENT_CODE (buf[0], buf[1], buf[2], buf[3]);
	guint i;

	for (i = 0; i < G_N_ELEMENTS (_cc_defs); i++) {
		if (_cc_defs[i].int_code == c) {
			cc = _cc_defs[i].code;
			goto done;
		}
	}

	g_set_error(error, DMAP_ERROR, DMAP_STATUS_INVALID_CONTENT_CODE,
		   "Invalid content code: %4s", buf);

done:
	return cc;
}

static gchar *
_read_string (const guint8 * buf, gsize size)
{
	gchar *str;

	if (g_utf8_validate ((char *) buf, size, NULL) == TRUE) {
		str = g_strndup ((char *) buf, size);
	} else {
		str = g_strdup ("");
	}

	return str;
}

static void
_parse_container_buffer (GNode * parent, const guint8 * buf,
                         gsize buf_length, GError **error)
{
	gsize l = 0;

	while (l < buf_length) {
		DmapContentCode cc;
		gsize codesize = 0;
		DmapStructureItem *item = NULL;
		GNode *node = NULL;
		GType gtype;

		if (parent
		 && parent->parent
		 && ((DmapStructureItem *)parent->parent->data)
		 && (DMAP_CC_ABGN == ((DmapStructureItem *) parent->parent->data)->content_code
		 ||  DMAP_CC_ABAR == ((DmapStructureItem *) parent->parent->data)->content_code
		 ||  DMAP_CC_ABAL == ((DmapStructureItem *) parent->parent->data)->content_code)) {
			/* Assume DMAP_RAW, as grandparent is ABGN or similar. */
			item = g_new0 (DmapStructureItem, 1);
			item->content_code = DMAP_RAW;
			node = g_node_new (item);
			g_node_append (parent, node);
			gchar *s = _read_string (buf, buf_length);
			item->size = strlen (s);
			g_value_init (&(item->content), G_TYPE_STRING);
			g_value_take_string (&(item->content), s);

			goto done;
		}

		/*
		 * Except in cases above, we need at least 8 bytes (4 of
		 * content_code and 4 of size) is odd.
		 */
		if (buf_length - l < 8) {
			g_set_error(error, DMAP_ERROR, DMAP_STATUS_RESPONSE_TOO_SHORT,
				   "Malformed response received");
			goto done;
		}

		cc = _cc_read_from_buffer ((const gchar *) &(buf[l]), error);
		if (cc == DMAP_CC_INVALID) {
			goto done;
		}
		l += 4;

		codesize = DMAP_READ_UINT32_BE (&(buf[l]));
		/* CCCCSIZECONTENT
		 * if the buffer length (minus 8 for the content code & size)
		 * is smaller than the read codesize (ie, someone sent us
		 * a codesize that is larger than the remaining data)
		 * then get out before we start processing it
		 */
		if (codesize > buf_length - l - 4) {
			g_set_error(error, DMAP_ERROR,
			            DMAP_STATUS_INVALID_CONTENT_CODE_SIZE,
				   "Invalid codesize %"G_GSIZE_FORMAT" "
			           "received in buffer of length %"G_GSIZE_FORMAT,
			            codesize, buf_length);
			goto done;
		}
		l += 4;

		item = g_new0 (DmapStructureItem, 1);
		item->content_code = cc;
		node = g_node_new (item);
		g_node_append (parent, node);

		gtype = _cc_gtype (item->content_code, error);

		if (gtype != G_TYPE_NONE) {
			g_value_init (&(item->content), gtype);
		}
// FIXME USE THE G_TYPE CONVERTOR FUNCTION dmap_type_to_gtype
		switch (_cc_dmap_type (item->content_code, error)) {
		case DMAP_TYPE_SIGNED_INT:
		case DMAP_TYPE_BYTE:{
				gchar c = 0;

				if (codesize == 1) {
					c = (gchar)
						DMAP_READ_UINT8 (&(buf[l]));
				}

				item->size = 1;
				g_value_set_schar (&(item->content), c);
				break;
			}
		case DMAP_TYPE_SHORT:{
				gint16 s = 0;

				if (codesize == 2) {
					s = DMAP_READ_UINT16_BE (&(buf[l]));
				}

				item->size = 2;
				g_value_set_int (&(item->content),
						 (gint32) s);
				break;
			}
		case DMAP_TYPE_DATE:
		case DMAP_TYPE_INT:{
				gint32 i = 0;

				if (codesize == 4) {
					i = DMAP_READ_UINT32_BE (&(buf[l]));
				}

				item->size = 4;
				g_value_set_int (&(item->content), i);
				break;
			}
		case DMAP_TYPE_INT64:{
				gint64 i = 0;

				if (codesize == 8) {
					i = DMAP_READ_UINT64_BE (&(buf[l]));
				}

				item->size = 8;
				g_value_set_int64 (&(item->content), i);
				break;
			}
		case DMAP_TYPE_STRING:{
				gchar *s = _read_string (&(buf[l]), codesize);

				item->size = strlen (s);
				g_value_take_string (&(item->content), s);
				break;
			}
		case DMAP_TYPE_POINTER:{
				gpointer *data =
					g_memdup2 ((const gchar *) &(buf[l]),
						   codesize);

				item->size = codesize;
				g_value_set_pointer (&(item->content), data);

				break;
			}
		case DMAP_TYPE_VERSION:{
				gint16 major = 0;
				gint16 minor = 0;
				gint16 patch = 0;
				gdouble v = 0;

				if (codesize == 4) {
					major = DMAP_READ_UINT16_BE (&
								     (buf
								      [l]));
					minor = DMAP_READ_UINT8 (&(buf[l]) +
								 2);
					patch = DMAP_READ_UINT8 (&(buf[l]) +
								 3);
				}

				v = (gdouble) major;
				v += (gdouble) (minor * 0.1);
				v += (gdouble) (patch * 0.01);

				item->size = 4;
				g_value_set_double (&(item->content), v);
				break;
			}
		case DMAP_TYPE_CONTAINER:{
				_parse_container_buffer (node, &(buf[l]), codesize, error);
				break;
			}
		case DMAP_TYPE_INVALID:
		default:
			/*
			 * Bad type should have been caught as bad content code
			 * by _cc_read_from_buffer()
			 */
			g_assert_not_reached();
		}

		l += codesize;
	}

done:
	return;
}

GNode *
dmap_structure_parse (const guint8 * buf, gsize buf_length, GError **error)
{
	GNode *root  = NULL;
	GNode *child = NULL;

	root = g_node_new (NULL);

	_parse_container_buffer (root, (guchar *) buf, buf_length, error);

	child = root->children;
	if (child) {
		g_node_unlink (child);
	}
	g_node_destroy (root);

	return child;
}

struct NodeFinder
{
	DmapContentCode code;
	GNode *node;
};

static gboolean
_gnode_find_node (GNode * node, gpointer data)
{
	gboolean found = FALSE;

	struct NodeFinder *finder = (struct NodeFinder *) data;
	DmapStructureItem *item = node->data;

	if (item->content_code == finder->code) {
		finder->node = node;
		found = TRUE;
	}

	return found;
}

DmapStructureItem *
dmap_structure_find_item (GNode * structure, DmapContentCode code)
{
	DmapStructureItem *item = NULL;
	GNode *node = NULL;

	node = dmap_structure_find_node (structure, code);
	if (node) {
		item = node->data;
	}

	return item;
}

GNode *
dmap_structure_add (GNode * parent, DmapContentCode cc, ...)
{
	DmapType dmap_type;
	GType gtype;
	DmapStructureItem *item;
	va_list list;
	GNode *node;
	gchar *error = NULL;

	va_start (list, cc);

	dmap_type = _cc_dmap_type (cc, NULL);
	gtype = _cc_gtype (cc, NULL);

	item = g_new0 (DmapStructureItem, 1);
	item->content_code = cc;

	if (gtype != G_TYPE_NONE) {
		g_value_init (&(item->content), gtype);
	}

	if (dmap_type != DMAP_TYPE_STRING && dmap_type != DMAP_TYPE_CONTAINER
	    && dmap_type != DMAP_TYPE_POINTER) {
		G_VALUE_COLLECT (&(item->content), list,
				 G_VALUE_NOCOPY_CONTENTS, &error);
		if (error) {
			g_warning ("%s", error);
			g_free (error);
		}
	}

	switch (dmap_type) {
	case DMAP_TYPE_BYTE:
	case DMAP_TYPE_SIGNED_INT:
		item->size = 1;
		break;
	case DMAP_TYPE_SHORT:
		item->size = 2;
		break;
	case DMAP_TYPE_DATE:
	case DMAP_TYPE_INT:
	case DMAP_TYPE_VERSION:
		item->size = 4;
		break;
	case DMAP_TYPE_INT64:
		item->size = 8;
		break;
	case DMAP_TYPE_STRING:{
			gchar *s = va_arg (list, gchar *);

			g_value_set_string (&(item->content), s);

			/* we dont use G_VALUE_COLLECT for this because we also
			 * need the length */
			item->size = strlen (s);
			break;
		}
	case DMAP_TYPE_POINTER:{
			gpointer p = va_arg (list, gpointer);
			gint s = va_arg (list, gint);

			g_value_set_pointer (&(item->content), p);

			/* we dont use G_VALUE_COLLECT for this because we also
			 * need the size */
			item->size = s;
			break;

		}
	case DMAP_TYPE_CONTAINER:
	default:
		break;
	}

	node = g_node_new (item);

	if (parent) {
		g_node_append (parent, node);

		while (parent) {
			DmapStructureItem *parent_item = parent->data;

			if (cc == DMAP_RAW) {
				parent_item->size += item->size;
			} else {
				parent_item->size += (4 + 4 + item->size);
			}

			parent = parent->parent;
		}
	}

	return node;
}

GNode *
dmap_structure_find_node (GNode * structure, DmapContentCode code)
{
	struct NodeFinder *finder;
	GNode *node = NULL;

	finder = g_new0 (struct NodeFinder, 1);

	finder->code = code;

	g_node_traverse (structure, G_IN_ORDER, G_TRAVERSE_ALL, -1,
			 _gnode_find_node, finder);

	node = finder->node;
	g_free (finder);
	finder = NULL;

	return node;
}

static void
_dmap_item_free (DmapStructureItem * item)
{
	DmapType type = _cc_dmap_type (item->content_code, NULL);

	if (DMAP_TYPE_INVALID != type && DMAP_TYPE_CONTAINER != type) {
		g_value_unset (&(item->content));
	}

	g_free (item);
}

static gboolean
_gnode_free_dmap_item (GNode * node, G_GNUC_UNUSED gpointer data)
{
	_dmap_item_free ((DmapStructureItem *) node->data);

	return FALSE;
}

void
dmap_structure_destroy (GNode * structure)
{
	if (structure) {
		g_node_traverse (structure, G_IN_ORDER, G_TRAVERSE_ALL, -1,
				 _gnode_free_dmap_item, NULL);

		g_node_destroy (structure);

		structure = NULL;
	}
}

const DmapContentCodeDefinition *
dmap_structure_content_codes (guint * number)
{
	*number = G_N_ELEMENTS (_cc_defs);

	return _cc_defs;
}

gint32
dmap_structure_cc_string_as_int32 (const gchar * str)
{
	union
	{
		gint32 i;
		gchar str[5];
	} u;

	strncpy (u.str, str, 4);
	u.str[4] = 0x00;

	return g_htonl (u.i);
}

static gboolean
_print_dmap_item (GNode * node, G_GNUC_UNUSED gpointer data)
{
	DmapStructureItem *item;
	const gchar *name;
	gchar *value;
	guint i;

	for (i = 1; i < g_node_depth (node); i++) {
		g_print ("\t");
	}

	item = node->data;

	name = _cc_name (item->content_code);

	if (G_IS_VALUE (&(item->content))) {
		value = g_strdup_value_contents (&(item->content));
	} else {
		value = g_strdup ("");
	}

	g_print ("%d, %s = %s (%d)\n", g_node_depth (node), name, value,
		 item->size);
	g_free (value);

	return FALSE;
}

void
dmap_structure_print (GNode * structure)
{
	if (structure) {
		g_node_traverse (structure, G_PRE_ORDER, G_TRAVERSE_ALL, -1,
				 (GNodeTraverseFunc) _print_dmap_item, NULL);
	}
}

guint
dmap_structure_get_size (GNode * structure)
{
	DmapStructureItem *item = (DmapStructureItem *) structure->data;

	g_assert (strlen(_cc_defs[item->content_code].string) == 4);
	g_assert (sizeof(item->size) == 4);

	return item->size + strlen(_cc_defs[item->content_code].string) + sizeof(item->size);
}

void
dmap_structure_increase_by_predicted_size (GNode * structure, guint size)
{
	((DmapStructureItem *) structure->data)->size += size;
}
