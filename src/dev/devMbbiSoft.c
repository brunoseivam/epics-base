/* devMbbiSoft.c */
/* share/src/dev @(#)devMbbiSoft.c	1.2     11/30/90 */

/* devMbbiSoft.c - Device Support Routines for  Soft Multibit Binary Input*/
/*
 *      Original Author: Bob Dalesio
 *      Current Author:  Marty Kraimer
 *      Date:            6-1-90
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01  11-11-91        jba     Moved set of alarm stat and sevr to macros
 * .02	03-13-92	jba	ANSI C changes
 * .03  10-10-92        jba     replaced code with recGblGetLinkValue call
 *      ...
 */


#include	<vxWorks.h>
#include	<types.h>
#include	<stdioLib.h>
#include	<string.h>

#include	<alarm.h>
#include	<dbDefs.h>
#include	<dbAccess.h>
#include        <recSup.h>
#include	<devSup.h>
#include	<module_types.h>
#include	<mbbiRecord.h>
/* Added for Channel Access Links */
long dbCaAddInlink();
long dbCaGetLink();


/* Create the dset for devMbbiSoft */
static long init_record();
static long read_mbbi();

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_mbbi;
}devMbbiSoft={
	5,
	NULL,
	NULL,
	init_record,
	NULL,
	read_mbbi};


static long init_record(pmbbi)
    struct mbbiRecord	*pmbbi;
{
    long status;

    /* mbbi.inp must be a CONSTANT or a PV_LINK or a DB_LINK or a CA_LINK*/
    switch (pmbbi->inp.type) {
    case (CONSTANT) :
        pmbbi->val = pmbbi->inp.value.value;
	pmbbi->udf = FALSE;
        break;
    case (PV_LINK) :
        status = dbCaAddInlink(&(pmbbi->inp), (void *) pmbbi, "VAL");
        if(status) return(status);
        break;
    case (DB_LINK) :
        break;
    case (CA_LINK) :
        break;
    default :
	recGblRecordError(S_db_badField,(void *)pmbbi,
		"devMbbiSoft (init_record) Illegal INP field");
	return(S_db_badField);
    }
    return(0);
}

static long read_mbbi(pmbbi)
    struct mbbiRecord	*pmbbi;
{
    long status,options=0,nRequest=1;

    status = recGblGetLinkValue(&(pmbbi->inp),(void *)pmbbi,DBR_USHORT,&(pmbbi->val),
              &options,&nRequest);

    if(RTN_SUCCESS(status)) pmbbi->udf=FALSE;

    return(2);
}
