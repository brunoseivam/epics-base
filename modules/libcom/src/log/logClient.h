/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/

/* logClient.h,v 1.5.2.1 2003/07/08 00:08:06 jhill Exp */
/*
 *
 *      Author:         Jeffrey O. Hill 
 *      Date:           080791 
 */

#ifndef INClogClienth
#define INClogClienth 1
#include "libComAPI.h"
#include "osiSock.h" /* for 'struct in_addr' */

/* include default log client interface for backward compatibility */
#include "iocLog.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *logClientId;
LIBCOM_API logClientId LIBCOMSTD_API logClientCreate (
    struct in_addr server_addr, unsigned short server_port);
LIBCOM_API void LIBCOMSTD_API logClientSend (logClientId id, const char *message);
LIBCOM_API void LIBCOMSTD_API logClientShow (logClientId id, unsigned level);
LIBCOM_API void LIBCOMSTD_API logClientFlush (logClientId id);
LIBCOM_API void LIBCOMSTD_API iocLogPrefix(const char* prefix);

/* deprecated interface; retained for backward compatibility */
/* note: implementations are in iocLog.c, not logClient.c */
LIBCOM_API logClientId LIBCOMSTD_API logClientInit (void);

#ifdef __cplusplus
}
#endif

#endif /*INClogClienth*/
