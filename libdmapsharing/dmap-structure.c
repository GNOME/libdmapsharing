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

#include "dmap-structure.h"
#include "dmap-utils.h"

#include <glib.h>
#include <glib-object.h>
#include <gobject/gvaluecollector.h>

#include <string.h>
#include <stdarg.h>

#define MAKE_CONTENT_CODE(ch0, ch1, ch2, ch3) \
    (( (gint32)(gchar)(ch0) | ( (gint32)(gchar)(ch1) << 8 ) | \
    ( (gint32)(gchar)(ch2) << 16 ) | \
    ( (gint32)(gchar)(ch3) << 24 ) ))

static const DMAPContentCodeDefinition cc_defs[] = {
    {DMAP_RAW, 0, "", "", DMAP_TYPE_STRING},
    {DMAP_CC_MDCL, MAKE_CONTENT_CODE('m','d','c','l'), "dmap.dictionary", "mdcl", DMAP_TYPE_CONTAINER},
    {DMAP_CC_MSTT, MAKE_CONTENT_CODE('m','s','t','t'), "dmap.status", "mstt", DMAP_TYPE_INT},
    {DMAP_CC_MIID, MAKE_CONTENT_CODE('m','i','i','d'), "dmap.itemid", "miid", DMAP_TYPE_INT},
    {DMAP_CC_MINM, MAKE_CONTENT_CODE('m','i','n','m'), "dmap.itemname", "minm", DMAP_TYPE_STRING},
    {DMAP_CC_MIKD, MAKE_CONTENT_CODE('m','i','k','d'), "dmap.itemkind", "mikd", DMAP_TYPE_BYTE},
    {DMAP_CC_MPER, MAKE_CONTENT_CODE('m','p','e','r'), "dmap.persistentid", "mper", DMAP_TYPE_INT64},
    {DMAP_CC_MCON, MAKE_CONTENT_CODE('m','c','o','n'), "dmap.container", "mcon", DMAP_TYPE_CONTAINER},
    {DMAP_CC_MCTI, MAKE_CONTENT_CODE('m','c','t','i'), "dmap.containeritemid", "mcti", DMAP_TYPE_INT},
    {DMAP_CC_MPCO, MAKE_CONTENT_CODE('m','p','c','o'), "dmap.parentcontainerid", "mpco", DMAP_TYPE_INT},
    {DMAP_CC_MSTS, MAKE_CONTENT_CODE('m','s','t','s'), "dmap.statusstring", "msts", DMAP_TYPE_STRING},
    {DMAP_CC_MIMC, MAKE_CONTENT_CODE('m','i','m','c'), "dmap.itemcount", "mimc", DMAP_TYPE_INT},
    {DMAP_CC_MCTC, MAKE_CONTENT_CODE('m','c','t','c'), "dmap.containercount", "mctc", DMAP_TYPE_INT},
    {DMAP_CC_MRCO, MAKE_CONTENT_CODE('m','r','c','o'), "dmap.returnedcount", "mrco", DMAP_TYPE_INT},
    {DMAP_CC_MTCO, MAKE_CONTENT_CODE('m','t','c','o'), "dmap.specifiedtotalcount", "mtco", DMAP_TYPE_INT},
    {DMAP_CC_MLCL, MAKE_CONTENT_CODE('m','l','c','l'), "dmap.listing", "mlcl", DMAP_TYPE_CONTAINER},
    {DMAP_CC_MLIT, MAKE_CONTENT_CODE('m','l','i','t'), "dmap.listingitem", "mlit", DMAP_TYPE_CONTAINER},
    {DMAP_CC_MBCL, MAKE_CONTENT_CODE('m','b','c','l'), "dmap.bag", "mbcl", DMAP_TYPE_CONTAINER},
    {DMAP_CC_MSRV, MAKE_CONTENT_CODE('m','s','r','v'), "dmap.serverinforesponse", "msrv", DMAP_TYPE_CONTAINER},
    {DMAP_CC_MSAU, MAKE_CONTENT_CODE('m','s','a','u'), "dmap.authenticationmethod", "msau", DMAP_TYPE_BYTE},
    {DMAP_CC_MSLR, MAKE_CONTENT_CODE('m','s','l','r'), "dmap.loginrequired", "mslr", DMAP_TYPE_BYTE},
    {DMAP_CC_MPRO, MAKE_CONTENT_CODE('m','p','r','o'), "dmap.protocolversion", "mpro", DMAP_TYPE_VERSION},
    {DMAP_CC_MSAL, MAKE_CONTENT_CODE('m','s','a','l'), "dmap.supportsautologout", "msal", DMAP_TYPE_BYTE},
    {DMAP_CC_MSUP, MAKE_CONTENT_CODE('m','s','u','p'), "dmap.supportsupdate", "msup", DMAP_TYPE_BYTE},
    {DMAP_CC_MSPI, MAKE_CONTENT_CODE('m','s','p','i'), "dmap.supportspersistenids", "mspi", DMAP_TYPE_BYTE},
    {DMAP_CC_MSEX, MAKE_CONTENT_CODE('m','s','e','x'), "dmap.supportsextensions", "msex", DMAP_TYPE_BYTE},
    {DMAP_CC_MSBR, MAKE_CONTENT_CODE('m','s','b','r'), "dmap.supportsbrowse", "msbr", DMAP_TYPE_BYTE},
    {DMAP_CC_MSQY, MAKE_CONTENT_CODE('m','s','q','y'), "dmap.supportsquery", "msqy", DMAP_TYPE_BYTE},
    {DMAP_CC_MSIX, MAKE_CONTENT_CODE('m','s','i','x'), "dmap.supportsindex", "msix", DMAP_TYPE_BYTE},
    {DMAP_CC_MSRS, MAKE_CONTENT_CODE('m','s','r','s'), "dmap.supportsresolve", "msrs", DMAP_TYPE_BYTE},
    {DMAP_CC_MSTM, MAKE_CONTENT_CODE('m','s','t','m'), "dmap.timeoutinterval", "mstm", DMAP_TYPE_INT},
    {DMAP_CC_MSDC, MAKE_CONTENT_CODE('m','s','d','c'), "dmap.databasescount", "msdc", DMAP_TYPE_INT},
    {DMAP_CC_MCCR, MAKE_CONTENT_CODE('m','c','c','r'), "dmap.contentcodesresponse", "mccr", DMAP_TYPE_CONTAINER},
    {DMAP_CC_MCNM, MAKE_CONTENT_CODE('m','c','n','m'), "dmap.contentcodesnumber", "mcnm", DMAP_TYPE_INT},
    {DMAP_CC_MCNA, MAKE_CONTENT_CODE('m','c','n','a'), "dmap.contentcodesname", "mcna", DMAP_TYPE_STRING},
    {DMAP_CC_MCTY, MAKE_CONTENT_CODE('m','c','t','y'), "dmap.contentcodestype", "mcty", DMAP_TYPE_SHORT},
    {DMAP_CC_MLOG, MAKE_CONTENT_CODE('m','l','o','g'), "dmap.loginresponse", "mlog", DMAP_TYPE_CONTAINER},
    {DMAP_CC_MLID, MAKE_CONTENT_CODE('m','l','i','d'), "dmap.sessionid", "mlid", DMAP_TYPE_INT},
    {DMAP_CC_MUPD, MAKE_CONTENT_CODE('m','u','p','d'), "dmap.updateresponse", "mupd", DMAP_TYPE_CONTAINER},
    {DMAP_CC_MUSR, MAKE_CONTENT_CODE('m','u','s','r'), "dmap.serverrevision", "musr", DMAP_TYPE_INT},
    {DMAP_CC_MUTY, MAKE_CONTENT_CODE('m','u','t','y'), "dmap.updatetype", "muty", DMAP_TYPE_BYTE},
    {DMAP_CC_MUDL, MAKE_CONTENT_CODE('m','u','d','l'), "dmap.deletedidlisting", "mudl", DMAP_TYPE_CONTAINER},
    {DMAP_CC_MSMA, MAKE_CONTENT_CODE('m','s','m','a'), "dmap.speakermachineaddress", "msma", DMAP_TYPE_INT},
    {DMAP_CC_FQUESCH, MAKE_CONTENT_CODE('f','?','c','h'), "dmap.haschildcontainers", "f?ch", DMAP_TYPE_BYTE},

    {DMAP_CC_APRO, MAKE_CONTENT_CODE('a','p','r','o'), "daap.protocolversion", "apro", DMAP_TYPE_VERSION},
    {DMAP_CC_AVDB, MAKE_CONTENT_CODE('a','v','d','b'), "daap.serverdatabases", "avdb", DMAP_TYPE_CONTAINER},
    {DMAP_CC_ABRO, MAKE_CONTENT_CODE('a','b','r','o'), "daap.databasebrowse", "abro", DMAP_TYPE_CONTAINER},
    {DMAP_CC_ABAL, MAKE_CONTENT_CODE('a','b','a','l'), "daap.browsealbumlisting", "abal", DMAP_TYPE_CONTAINER},
    {DMAP_CC_ABAR, MAKE_CONTENT_CODE('a','b','a','r'), "daap.browseartistlisting", "abar", DMAP_TYPE_CONTAINER},
    {DMAP_CC_ABCP, MAKE_CONTENT_CODE('a','b','c','p'), "daap.browsecomposerlisting", "abcp", DMAP_TYPE_CONTAINER},
    {DMAP_CC_ABGN, MAKE_CONTENT_CODE('a','b','g','n'), "daap.browsegenrelisting", "abgn", DMAP_TYPE_CONTAINER},
    {DMAP_CC_ADBS, MAKE_CONTENT_CODE('a','d','b','s'), "daap.returndatabasesongs", "adbs", DMAP_TYPE_CONTAINER},
    {DMAP_CC_ASAL, MAKE_CONTENT_CODE('a','s','a','l'), "daap.songalbum", "asal", DMAP_TYPE_STRING},
    {DMAP_CC_ASAI, MAKE_CONTENT_CODE('a','s','a','i'), "daap.songalbumid", "asai", DMAP_TYPE_INT},
    {DMAP_CC_ASAA, MAKE_CONTENT_CODE('a','s','a','a'), "daap.songalbumartist", "asaa", DMAP_TYPE_STRING},
    {DMAP_CC_ASAR, MAKE_CONTENT_CODE('a','s','a','r'), "daap.songartist", "asar", DMAP_TYPE_STRING},
    {DMAP_CC_ASBT, MAKE_CONTENT_CODE('a','s','b','t'), "daap.songsbeatsperminute", "asbt", DMAP_TYPE_SHORT},
    {DMAP_CC_ASBR, MAKE_CONTENT_CODE('a','s','b','r'), "daap.songbitrate", "asbr", DMAP_TYPE_SHORT},
    {DMAP_CC_ASCM, MAKE_CONTENT_CODE('a','s','c','m'), "daap.songcomment", "ascm", DMAP_TYPE_STRING},
    {DMAP_CC_ASCO, MAKE_CONTENT_CODE('a','s','c','o'), "daap.songcompliation", "asco", DMAP_TYPE_BYTE},
    {DMAP_CC_ASDA, MAKE_CONTENT_CODE('a','s','d','a'), "daap.songdateadded", "asda", DMAP_TYPE_DATE},
    {DMAP_CC_ASDM, MAKE_CONTENT_CODE('a','s','d','m'), "daap.songdatemodified", "asdm", DMAP_TYPE_DATE},
    {DMAP_CC_ASDC, MAKE_CONTENT_CODE('a','s','d','c'), "daap.songdisccount", "asdc", DMAP_TYPE_SHORT},
    {DMAP_CC_ASDN, MAKE_CONTENT_CODE('a','s','d','n'), "daap.songdiscnumber", "asdn", DMAP_TYPE_SHORT},
    {DMAP_CC_ASDB, MAKE_CONTENT_CODE('a','s','d','b'), "daap.songdisabled", "asdb", DMAP_TYPE_BYTE},
    {DMAP_CC_ASEQ, MAKE_CONTENT_CODE('a','s','e','q'), "daap.songeqpreset", "aseq", DMAP_TYPE_STRING},
    {DMAP_CC_ASFM, MAKE_CONTENT_CODE('a','s','f','m'), "daap.songformat", "asfm", DMAP_TYPE_STRING},
    {DMAP_CC_ASGN, MAKE_CONTENT_CODE('a','s','g','n'), "daap.songgenre", "asgn", DMAP_TYPE_STRING},
    {DMAP_CC_ASDT, MAKE_CONTENT_CODE('a','s','d','t'), "daap.songdescription", "asdt", DMAP_TYPE_STRING},
    {DMAP_CC_ASRV, MAKE_CONTENT_CODE('a','s','r','v'), "daap.songrelativevolume", "asrv", DMAP_TYPE_SIGNED_INT},
    {DMAP_CC_ASSR, MAKE_CONTENT_CODE('a','s','s','r'), "daap.songsamplerate", "assr", DMAP_TYPE_INT},
    {DMAP_CC_ASSZ, MAKE_CONTENT_CODE('a','s','s','z'), "daap.songsize", "assz", DMAP_TYPE_INT},
    {DMAP_CC_ASST, MAKE_CONTENT_CODE('a','s','s','t'), "daap.songstarttime", "asst", DMAP_TYPE_INT},
    {DMAP_CC_ASSP, MAKE_CONTENT_CODE('a','s','s','p'), "daap.songstoptime", "assp", DMAP_TYPE_INT},
    {DMAP_CC_ASTM, MAKE_CONTENT_CODE('a','s','t','m'), "daap.songtime", "astm", DMAP_TYPE_INT},
    {DMAP_CC_ASTC, MAKE_CONTENT_CODE('a','s','t','c'), "daap.songtrackcount", "astc", DMAP_TYPE_SHORT},
    {DMAP_CC_ASTN, MAKE_CONTENT_CODE('a','s','t','n'), "daap.songtracknumber", "astn", DMAP_TYPE_SHORT},
    {DMAP_CC_ASUR, MAKE_CONTENT_CODE('a','s','u','r'), "daap.songuserrating", "asur", DMAP_TYPE_BYTE},
    {DMAP_CC_ASYR, MAKE_CONTENT_CODE('a','s','y','r'), "daap.songyear", "asyr", DMAP_TYPE_SHORT},
    {DMAP_CC_ASDK, MAKE_CONTENT_CODE('a','s','d','k'), "daap.songdatakind", "asdk", DMAP_TYPE_BYTE},
    {DMAP_CC_ASUL, MAKE_CONTENT_CODE('a','s','u','l'), "daap.songdataurl", "asul", DMAP_TYPE_STRING},
    {DMAP_CC_ASSU, MAKE_CONTENT_CODE('a','s','s','u'), "daap.sortalbum", "assu", DMAP_TYPE_STRING},
    {DMAP_CC_ASSA, MAKE_CONTENT_CODE('a','s','s','a'), "daap.sortartist", "assa", DMAP_TYPE_STRING},
    {DMAP_CC_APLY, MAKE_CONTENT_CODE('a','p','l','y'), "daap.databaseplaylists", "aply", DMAP_TYPE_CONTAINER},
    {DMAP_CC_ABPL, MAKE_CONTENT_CODE('a','b','p','l'), "daap.baseplaylist", "abpl", DMAP_TYPE_BYTE},
    {DMAP_CC_APSO, MAKE_CONTENT_CODE('a','p','s','o'), "daap.playlistsongs", "apso", DMAP_TYPE_CONTAINER},
    {DMAP_CC_PRSV, MAKE_CONTENT_CODE('p','r','s','v'), "daap.resolve", "prsv", DMAP_TYPE_CONTAINER},
    {DMAP_CC_ARIF, MAKE_CONTENT_CODE('a','r','i','f'), "daap.resolveinfo", "arif", DMAP_TYPE_CONTAINER},
    {DMAP_CC_MSAS, MAKE_CONTENT_CODE('m','s','a','s'), "daap.authentication.schemes", "msas", DMAP_TYPE_BYTE},
    {DMAP_CC_AGRP, MAKE_CONTENT_CODE('a','g','r','p'), "daap.songgrouping", "agrp", DMAP_TYPE_STRING},
    {DMAP_CC_AGAL, MAKE_CONTENT_CODE('a','g','a','l'), "daap.albumgrouping", "agal", DMAP_TYPE_CONTAINER},
    {DMAP_CC_ASCP, MAKE_CONTENT_CODE('a','s','c','p'), "daap.songcomposer", "ascp", DMAP_TYPE_STRING},
    {DMAP_CC_PPRO, MAKE_CONTENT_CODE('p','p','r','o'), "dpap.protocolversion", "ppro", DMAP_TYPE_VERSION},
    {DMAP_CC_PASP, MAKE_CONTENT_CODE('p','a','s','p'), "dpap.aspectratio", "pasp", DMAP_TYPE_STRING},
    {DMAP_CC_PFDT, MAKE_CONTENT_CODE('p','f','d','t'), "dpap.filedata", "pfdt", DMAP_TYPE_POINTER},
    {DMAP_CC_PICD, MAKE_CONTENT_CODE('p','i','c','d'), "dpap.creationdate", "picd", DMAP_TYPE_INT},
    {DMAP_CC_PIMF, MAKE_CONTENT_CODE('p','i','m','f'), "dpap.imagefilename", "pimf", DMAP_TYPE_STRING},
    {DMAP_CC_PFMT, MAKE_CONTENT_CODE('p','f','m','t'), "dpap.imageformat", "pfmt", DMAP_TYPE_STRING},
    {DMAP_CC_PIFS, MAKE_CONTENT_CODE('p','i','f','s'), "dpap.imagefilesize", "pifs", DMAP_TYPE_INT},
    {DMAP_CC_PLSZ, MAKE_CONTENT_CODE('p','l','s','z'), "dpap.imagelargefilesize", "plsz", DMAP_TYPE_INT},
    {DMAP_CC_PHGT, MAKE_CONTENT_CODE('p','h','g','t'), "dpap.imagepixelheight", "phgt", DMAP_TYPE_INT},
    {DMAP_CC_PWTH, MAKE_CONTENT_CODE('p','w','t','h'), "dpap.imagepixelwidth", "pwth", DMAP_TYPE_INT},
    {DMAP_CC_PRAT, MAKE_CONTENT_CODE('p','r','a','t'), "dpap.imagerating", "prat", DMAP_TYPE_INT},
    {DMAP_CC_PCMT, MAKE_CONTENT_CODE('p','c','m','t'), "dpap.imagecomments", "pcmt", DMAP_TYPE_STRING},
    {DMAP_CC_PRET, MAKE_CONTENT_CODE('p','r','e','t'), "dpap.pret", "pret", DMAP_TYPE_STRING},
    {DMAP_CC_AESV, MAKE_CONTENT_CODE('a','e','S','V'), "com.apple.itunes.music-sharing-version", "aesv", DMAP_TYPE_INT},
    {DMAP_CC_AEHV, MAKE_CONTENT_CODE('a','e','H','V'), "com.apple.itunes.has-video", "aeHV", DMAP_TYPE_BYTE},
    {DMAP_CC_AESP, MAKE_CONTENT_CODE('a','e','S','P'), "com.apple.itunes.smart-playlist", "aeSP", DMAP_TYPE_BYTE},
    {DMAP_CC_AEPP, MAKE_CONTENT_CODE('a','e','P','P'), "com.apple.itunes.is-podcast-playlist", "aePP", DMAP_TYPE_BYTE},
    {DMAP_CC_AEPS, MAKE_CONTENT_CODE('a','e','P','S'), "com.apple.itunes.special-playlist", "aePS", DMAP_TYPE_BYTE},
    {DMAP_CC_AESG, MAKE_CONTENT_CODE('a','e','S','G'), "com.apple.itunes.saved-genius", "aeSG", DMAP_TYPE_BYTE},
    {DMAP_CC_AEMK, MAKE_CONTENT_CODE('a','e','M','K'), "com.apple.itunes.mediakind", "aeMK", DMAP_TYPE_BYTE},
    {DMAP_CC_AEFP, MAKE_CONTENT_CODE('a','e','F','P'), "com.apple.itunes.req-fplay", "aeFP", DMAP_TYPE_BYTE},
    
    /* DACP */
    {DMAP_CC_CMPA, MAKE_CONTENT_CODE('c','m','p','a'), "dacp.contentcontainer", "cmpa", DMAP_TYPE_CONTAINER},
    {DMAP_CC_CMNM, MAKE_CONTENT_CODE('c','m','n','m'), "dacp.contentname", "cmnm", DMAP_TYPE_STRING},
    {DMAP_CC_CMTY, MAKE_CONTENT_CODE('c','m','t','y'), "dacp.contentvalue", "cmty", DMAP_TYPE_STRING},
    {DMAP_CC_CMPG, MAKE_CONTENT_CODE('c','m','p','g'), "dacp.passguid", "cmpy", DMAP_TYPE_INT64},
    
    {DMAP_CC_CACI, MAKE_CONTENT_CODE('c','a','c','i'), "dacp.controlint", "caci", DMAP_TYPE_CONTAINER},
    {DMAP_CC_CAPS, MAKE_CONTENT_CODE('c','a','p','s'), "dacp.playstatus", "caci", DMAP_TYPE_BYTE},
    {DMAP_CC_CASH, MAKE_CONTENT_CODE('c','a','s','h'), "dacp.shufflestate", "caci", DMAP_TYPE_BYTE},
    {DMAP_CC_CARP, MAKE_CONTENT_CODE('c','a','r','p'), "dacp.repeatstate", "caci", DMAP_TYPE_BYTE},
    {DMAP_CC_CAAS, MAKE_CONTENT_CODE('c','a','a','s'), "dacp.albumshuffle", "caas", DMAP_TYPE_INT},
    {DMAP_CC_CAAR, MAKE_CONTENT_CODE('c','a','a','r'), "dacp.albumrepeat", "caci", DMAP_TYPE_INT},    
    {DMAP_CC_CAIA, MAKE_CONTENT_CODE('c','a','i','a'), "dacp.isavailiable", "caia", DMAP_TYPE_BYTE},
    {DMAP_CC_CANP, MAKE_CONTENT_CODE('c','a','n','p'), "dacp.nowplaying", "canp", DMAP_TYPE_INT64},
    {DMAP_CC_CANN, MAKE_CONTENT_CODE('c','a','n','n'), "dacp.nowplayingtrack", "cann", DMAP_TYPE_STRING},
    {DMAP_CC_CANA, MAKE_CONTENT_CODE('c','a','n','a'), "dacp.nowplayingartist", "cana", DMAP_TYPE_STRING},
    {DMAP_CC_CANL, MAKE_CONTENT_CODE('c','a','n','l'), "dacp.nowplayingalbum", "canl", DMAP_TYPE_STRING},
    {DMAP_CC_CANG, MAKE_CONTENT_CODE('c','a','n','g'), "dacp.nowplayinggenre", "cang", DMAP_TYPE_STRING},
    {DMAP_CC_CANT, MAKE_CONTENT_CODE('c','a','n','t'), "dacp.remainingtime", "cant", DMAP_TYPE_INT},
    {DMAP_CC_CASP, MAKE_CONTENT_CODE('c','a','s','p'), "dacp.speakers", "casp", DMAP_TYPE_CONTAINER},
    {DMAP_CC_CASS, MAKE_CONTENT_CODE('c','a','s','s'), "dacp.ss", "cass", DMAP_TYPE_BYTE},
    {DMAP_CC_CAST, MAKE_CONTENT_CODE('c','a','s','t'), "dacp.tracklength", "cast", DMAP_TYPE_INT},
    {DMAP_CC_CASU, MAKE_CONTENT_CODE('c','a','s','u'), "dacp.su", "casu", DMAP_TYPE_BYTE},
    {DMAP_CC_CASG, MAKE_CONTENT_CODE('c','a','s','g'), "dacp.sg", "caSG", DMAP_TYPE_BYTE},
    {DMAP_CC_CACR, MAKE_CONTENT_CODE('c','a','c','r'), "dacp.cacr", "cacr", DMAP_TYPE_CONTAINER},
    
    {DMAP_CC_CMCP, MAKE_CONTENT_CODE('c','m','c','p'), "dmcp.controlprompt", "cmcp", DMAP_TYPE_CONTAINER},
    {DMAP_CC_CMGT, MAKE_CONTENT_CODE('c','m','g','t'), "dmcp.getpropertyresponse", "cmgt", DMAP_TYPE_CONTAINER},
    {DMAP_CC_CMIK, MAKE_CONTENT_CODE('c','m','i','k'), "dmcp.ik", "cmik", DMAP_TYPE_BYTE},
    {DMAP_CC_CMSP, MAKE_CONTENT_CODE('c','m','s','p'), "dmcp.ik", "cmsp", DMAP_TYPE_BYTE},
    {DMAP_CC_CMST, MAKE_CONTENT_CODE('c','m','s','t'), "dmcp.status", "cmst", DMAP_TYPE_CONTAINER},
    {DMAP_CC_CMSV, MAKE_CONTENT_CODE('c','m','s','v'), "dmcp.sv", "cmsv", DMAP_TYPE_BYTE},
    {DMAP_CC_CMSR, MAKE_CONTENT_CODE('c','m','s','r'), "dmcp.mediarevision", "cmsr", DMAP_TYPE_INT},
    {DMAP_CC_CMMK, MAKE_CONTENT_CODE('c','m','m','k'), "dmcp.mediakind", "cmmk", DMAP_TYPE_INT},
    {DMAP_CC_CMVO, MAKE_CONTENT_CODE('c','m','v','o'), "dmcp.volume", "cmvo", DMAP_TYPE_INT},
    
};

const gchar * 
dmap_content_code_name (DMAPContentCode code)
{
    return cc_defs[code-1].name;
}

DMAPType 
dmap_content_code_dmap_type (DMAPContentCode code)
{
    return cc_defs[code-1].type;
}

const gchar * 
dmap_content_code_string (DMAPContentCode code)
{
    return cc_defs[code-1].string;
}
            
static GType
dmap_content_code_gtype (DMAPContentCode code)
{
    switch (dmap_content_code_dmap_type (code)) {
        case DMAP_TYPE_BYTE:
        case DMAP_TYPE_SIGNED_INT:
            return G_TYPE_CHAR;
        case DMAP_TYPE_SHORT:
        case DMAP_TYPE_INT:
        case DMAP_TYPE_DATE:
            return G_TYPE_INT;
        case DMAP_TYPE_INT64:
            return G_TYPE_INT64;
        case DMAP_TYPE_VERSION:
            return G_TYPE_DOUBLE;
        case DMAP_TYPE_STRING:
            return G_TYPE_STRING;
        case DMAP_TYPE_POINTER:
            return G_TYPE_POINTER;
        case DMAP_TYPE_CONTAINER:
        default:
            return G_TYPE_NONE;
    }
}

static gboolean 
dmap_structure_node_serialize (GNode *node, 
                  GByteArray *array)
{
    DMAPStructureItem *item = node->data;
    DMAPType dmap_type;
    guint32 size = GINT32_TO_BE (item->size);

    if (item->content_code != DMAP_RAW) {
	    g_byte_array_append (array, (const guint8 *)dmap_content_code_string (item->content_code), 4);
	    g_byte_array_append (array, (const guint8 *)&size, 4);
    }
    
    dmap_type = dmap_content_code_dmap_type (item->content_code);

    switch (dmap_type) {
        case DMAP_TYPE_BYTE: 
        case DMAP_TYPE_SIGNED_INT: {
            gchar c = g_value_get_char (&(item->content));
            
            g_byte_array_append (array, (const guint8 *)&c, 1);
            
            break;
        }
        case DMAP_TYPE_SHORT: {
            gint32 i = g_value_get_int (&(item->content));
            gint16 s = GINT16_TO_BE ((gint16) i);
            
            g_byte_array_append (array, (const guint8 *)&s, 2);

            break;
            }
        case DMAP_TYPE_DATE: 
        case DMAP_TYPE_INT: {
            gint32 i = g_value_get_int (&(item->content));
            gint32 s = GINT32_TO_BE (i);

            g_byte_array_append (array, (const guint8 *)&s, 4);
            
            break;
        }
        case DMAP_TYPE_VERSION: {
            gdouble v = g_value_get_double (&(item->content));
            gint16 major;
            gint8 minor;
            gint8 patch = 0;

            major = (gint16)v;
            minor = (gint8)(v - ((gdouble)major));

            major = GINT16_TO_BE (major);

            g_byte_array_append (array, (const guint8 *)&major, 2);
            g_byte_array_append (array, (const guint8 *)&minor, 1);
            g_byte_array_append (array, (const guint8 *)&patch, 1);
            
            break;
        }        
        case DMAP_TYPE_INT64: {
            gint64 i = g_value_get_int64 (&(item->content));
            gint64 s = GINT64_TO_BE (i);

            g_byte_array_append (array, (const guint8 *)&s, 8);
            
            break;
        }
        case DMAP_TYPE_STRING: {
            const gchar *s = g_value_get_string (&(item->content));

            g_byte_array_append (array, (const guint8 *)s, strlen (s));
            
            break;
        }
        case DMAP_TYPE_POINTER: {
            const gpointer *data = g_value_get_pointer (&(item->content));

            g_byte_array_append (array, (const guint8 *)data, item->size);

            break;
        }
        case DMAP_TYPE_CONTAINER:
        default:
            break;
    }

    return FALSE;
}
    
gchar * 
dmap_structure_serialize (GNode *structure, 
                          guint *length)
{
    GByteArray *array;
    gchar *data;

    array = g_byte_array_new ();

    if (structure) {
        g_node_traverse (structure, G_PRE_ORDER, G_TRAVERSE_ALL, -1, (GNodeTraverseFunc)dmap_structure_node_serialize, array);
    }
    
    data = (gchar *) array->data;
    *length = array->len;
    g_byte_array_free (array, FALSE);
    
    return data;
}

DMAPContentCode 
dmap_content_code_read_from_buffer (const gchar *buf)
{
     gint32 c = MAKE_CONTENT_CODE (buf[0], buf[1], buf[2], buf[3]);
     guint i;

    for (i = 0; i < G_N_ELEMENTS (cc_defs); i++) {
        if (cc_defs[i].int_code == c) {
            return cc_defs[i].code;
        }
    }
    
    g_warning ("Content code %4s is invalid.", buf);

    return DMAP_CC_INVALID;
}

static gchar *
dmap_buffer_read_string (const gchar *buf, gssize size)
{
    if (g_utf8_validate (buf, size, NULL) == TRUE) {
        return g_strndup (buf, size);
    } else {
        return g_strdup ("");
    }
}

static void 
dmap_structure_parse_container_buffer (GNode *parent, 
                      const guchar *buf, 
                      gint buf_length)
{
    gint l = 0;

    while (l < buf_length) {
        DMAPContentCode cc;
        gint codesize = 0;
        DMAPStructureItem *item = NULL;
        GNode *node = NULL;
        GType gtype;
        
        g_debug ("l is %d and buf_length is %d\n", l, buf_length);

        /* we need at least 8 bytes, 4 of content_code and 4 of size */
        if (buf_length - l < 8) {
            g_debug ("Malformed response recieved\n");
            return;
        }
        
        cc = dmap_content_code_read_from_buffer ((const gchar*)&(buf[l]));
        if (cc == DMAP_CC_INVALID) {
            return;
        }
        l += 4;

        codesize = DMAP_READ_UINT32_BE(&(buf[l]));
        /* CCCCSIZECONTENT
         * if the buffer length (minus 8 for the content code & size)
         * is smaller than the read codesize (ie, someone sent us
         * a codesize that is larger than the remaining data)
         * then get out before we start processing it
         */
        if (codesize > buf_length - l - 4 || codesize < 0) {
            g_debug ("Invalid codesize %d recieved in buf_length %d\n", codesize, buf_length);
            return;
        }
        l += 4;

        g_debug ("content_code = %d, codesize is %d, l is %d\n", cc, codesize, l);
        
        item = g_new0 (DMAPStructureItem, 1);
        item->content_code = cc;
        node = g_node_new (item);
        g_node_append (parent, node);
        
        gtype = dmap_content_code_gtype (item->content_code);

        if (gtype != G_TYPE_NONE) {
            g_value_init (&(item->content), gtype);
        }
        
// FIXME USE THE G_TYPE CONVERTOR FUNCTION dmap_type_to_gtype
        switch (dmap_content_code_dmap_type (item->content_code)) {
            case DMAP_TYPE_SIGNED_INT:
            case DMAP_TYPE_BYTE: {
                gchar c = 0;
                
                if (codesize == 1) {
                    c = (gchar) DMAP_READ_UINT8(&(buf[l]));
                }
                
                item->size = 1;
                g_value_set_char (&(item->content), c);
                g_debug ("Code: %s, content (%d): \"%c\"\n", dmap_content_code_string (item->content_code), codesize, (gchar)c);

                break;
            }
            case DMAP_TYPE_SHORT: {
                gint16 s = 0;

                if (codesize == 2) {
                    s = DMAP_READ_UINT16_BE(&(buf[l]));
                }

                item->size = 2;
                g_value_set_int (&(item->content),(gint32)s);
                g_debug ("Code: %s, content (%d): %hi\n", dmap_content_code_string (item->content_code), codesize, s);

                break;
            }
            case DMAP_TYPE_DATE:
            case DMAP_TYPE_INT: {
                gint32 i = 0;

                if (codesize == 4) {
                    i = DMAP_READ_UINT32_BE(&(buf[l]));
                }
                
                item->size = 4;
                g_value_set_int (&(item->content), i);
                g_debug ("Code: %s, content (%d): %d\n", dmap_content_code_string (item->content_code), codesize, i);
                break;
            }
            case DMAP_TYPE_INT64: {
                gint64 i = 0;
        
                if (codesize == 8) {
                    i = DMAP_READ_UINT64_BE(&(buf[l]));
                }
                
                item->size = 8;
                g_value_set_int64 (&(item->content), i);
                g_debug ("Code: %s, content (%d): %"G_GINT64_FORMAT"\n", dmap_content_code_string (item->content_code), codesize, i);

                break;
            }
            case DMAP_TYPE_STRING: {
                gchar *s = dmap_buffer_read_string ((const gchar*)&(buf[l]), codesize);

                item->size = strlen (s);
                g_value_set_string (&(item->content), s);
                g_debug ("Code: %s, content (%d): \"%s\"\n", dmap_content_code_string (item->content_code), codesize, s);

                break;
            }
            case DMAP_TYPE_POINTER: {
                gpointer *data = g_memdup ((const gchar*)&(buf[l]), codesize);
            
                item->size = codesize;
                g_value_set_pointer (&(item->content), data);
                
                break;
            }
            case DMAP_TYPE_VERSION: {
                gint16 major = 0;
                gint16 minor = 0;
                gint16 patch = 0;
                gdouble v = 0;

                if (codesize == 4) {
                    major = DMAP_READ_UINT16_BE(&(buf[l]));
                    minor = DMAP_READ_UINT8(&(buf[l]) + 2);
                    patch = DMAP_READ_UINT8(&(buf[l]) + 3);
                }

                v = (gdouble)major;
                v += (gdouble)(minor * 0.1);
                v += (gdouble)(patch * 0.01);
                
                item->size = 4;
                g_value_set_double (&(item->content), v);
                g_debug ("Code: %s, content: %f\n", dmap_content_code_string (item->content_code), v);

                break;
            }
            case DMAP_TYPE_CONTAINER: {
                g_debug ("Code: %s, container\n", dmap_content_code_string (item->content_code));
                dmap_structure_parse_container_buffer (node,&(buf[l]), codesize);
                break;
            }
        }

        l += codesize;
    }

    return;
}

GNode * 
dmap_structure_parse (const gchar *buf, 
                      gint buf_length)
{
    GNode *root = NULL;
    GNode *child = NULL;

    root = g_node_new (NULL);

    dmap_structure_parse_container_buffer (root, (guchar *)buf, buf_length);

    child = root->children;
    if (child) {
        g_node_unlink (child);
    }
    g_node_destroy (root);
    
    return child;
}

struct NodeFinder {
    DMAPContentCode code;
    GNode *node;
};

static gboolean 
gnode_find_node (GNode *node, 
         gpointer data)
{
    struct NodeFinder *finder = (struct NodeFinder *)data;
    DMAPStructureItem *item = node->data;

    if (item->content_code == finder->code) {
        finder->node = node;
        return TRUE;
    }

    return FALSE;
}

DMAPStructureItem * 
dmap_structure_find_item (GNode *structure, 
                          DMAPContentCode code)
{
    GNode *node = NULL;
    
    node = dmap_structure_find_node (structure, code);

    if (node) {
        return node->data;
    }

    return NULL;
}

GNode *
dmap_structure_add (GNode *parent,
		       DMAPContentCode cc,
		       ...)
{
	DMAPType dmap_type;
	GType gtype;
	DMAPStructureItem *item;
	va_list list;
	GNode *node;
	gchar *error = NULL;

	va_start (list, cc);

	dmap_type = dmap_content_code_dmap_type (cc);
	gtype = dmap_content_code_gtype (cc);

	item = g_new0(DMAPStructureItem, 1);
	item->content_code = cc;

	if (gtype != G_TYPE_NONE) {
		g_value_init (&(item->content), gtype);
	}

	if (dmap_type != DMAP_TYPE_STRING && dmap_type != DMAP_TYPE_CONTAINER && dmap_type != DMAP_TYPE_POINTER) {
		G_VALUE_COLLECT (&(item->content), list, G_VALUE_NOCOPY_CONTENTS, &error);
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
		case DMAP_TYPE_STRING: {
			gchar *s = va_arg (list, gchar *);

			g_value_set_string (&(item->content), s);

			/* we dont use G_VALUE_COLLECT for this because we also
			 * need the length */
			item->size = strlen (s);
			break;
		}
		case DMAP_TYPE_POINTER: {
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
			DMAPStructureItem *parent_item = parent->data;

			if (cc == DMAP_RAW)
				parent_item->size += item->size;
			else
				parent_item->size += (4 + 4 + item->size);

			parent = parent->parent;
		}
	}

	return node;
}

GNode * 
dmap_structure_find_node (GNode *structure, 
                 DMAPContentCode code)
{
    struct NodeFinder *finder;
    GNode *node = NULL;

    finder = g_new0(struct NodeFinder,1);
    finder->code = code;

    g_node_traverse (structure, G_IN_ORDER, G_TRAVERSE_ALL, -1, gnode_find_node, finder);

    node = finder->node;
    g_free (finder);
    finder = NULL;

    return node;
}



static void
dmap_item_free (DMAPStructureItem *item)
{
    if (dmap_content_code_dmap_type (item->content_code) != DMAP_TYPE_CONTAINER) {
        g_value_unset (&(item->content));
    }

    g_free (item);
}

static gboolean
gnode_free_dmap_item (GNode *node,
             gpointer data)
{
    dmap_item_free ((DMAPStructureItem *)node->data);

    return FALSE;
}

void 
dmap_structure_destroy (GNode *structure)
{
    if (structure) {
        g_node_traverse (structure, G_IN_ORDER, G_TRAVERSE_ALL, -1, gnode_free_dmap_item, NULL);

        g_node_destroy (structure);

        structure = NULL;
    }
}

const DMAPContentCodeDefinition *
dmap_content_codes (guint *number)
{
    *number = G_N_ELEMENTS (cc_defs);

    return cc_defs;
}

gint32 
dmap_content_code_string_as_int32 (const gchar *str)
{
    union {
        gint32 i;
        gchar str[4];
    } u;

    strncpy (u.str, str, 4);

    return g_htonl (u.i);
}

static gboolean 
print_dmap_item (GNode *node, 
                 gpointer data)
{
    DMAPStructureItem *item;
    const gchar *name;
    gchar *value;
    gint i;

    for (i = 1; i < g_node_depth (node); i++) {
        g_print ("\t");
    }

    item = node->data;

    name = dmap_content_code_name (item->content_code);

    if (G_IS_VALUE (&(item->content))) {
        value = g_strdup_value_contents (&(item->content));
    } else {
        value = g_strdup ("");
    }

    g_print ("%d, %s = %s (%d)\n", g_node_depth (node), name, value, item->size);
    g_free (value);

    return FALSE;
}

void
dmap_structure_print (GNode *structure)
{
    if (structure) {
        g_node_traverse (structure, G_PRE_ORDER, G_TRAVERSE_ALL, -1, (GNodeTraverseFunc)print_dmap_item, NULL);
    }
}
