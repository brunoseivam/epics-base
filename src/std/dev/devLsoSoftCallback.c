/*************************************************************************\
* Copyright (c) 2012 UChicago Argonne LLC, as Operator of Argonne
*     National Laboratory.
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/* $Revision-Id$ */
/*
 *      Author: Andrew Johnson
 *      Date:   30 Nov 2012
 */

#include <string.h>

#define EPICS_DBCA_PRIVATE_API
#include "alarm.h"
#include "dbAccess.h"
#include "recGbl.h"
#include "lsoRecord.h"
#include "epicsExport.h"

static long write_string(lsoRecord *prec)
{
    struct link *plink = &prec->out;
    int dtyp = dbGetLinkDBFtype(plink);
    long len = prec->len;
    long status;

    if (prec->pact || dtyp < 0)
        return 0;

    if (dtyp != DBR_CHAR && dtyp != DBF_UCHAR) {
        dtyp = DBR_STRING;
        len = 1;
    }

    if (plink->type != CA_LINK || strcmp(plink->value.pv_link.backend,"ca")!=0)
        return dbPutLink(plink, dtyp, prec->val, len);

    status = dbCaPutLinkCallback(plink, dtyp, prec->val, len,
        dbCaCallbackProcess, plink);
    if (status) {
        recGblSetSevr(prec, LINK_ALARM, INVALID_ALARM);
        return status;
    }

    prec->pact = TRUE;
    return 0;
}

lsodset devLsoSoftCallback = {
    5, NULL, NULL, NULL, NULL, write_string
};
epicsExportAddress(dset, devLsoSoftCallback);
