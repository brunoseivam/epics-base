/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* SPDX-License-Identifier: EPICS
* EPICS Base is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/
/* for a pure posix implementation no osdMutex.h definitions are needed*/
#ifndef osdMutexh
#define osdMutexh

#include <pthread.h>

#include "shareLib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {posixMutexDefault = 0, posixMutexRecursive = 1} EpicsPosixMutexProperty;

/* Returns NULL if requested set of properties is not supported */
epicsShareFunc pthread_mutexattr_t * epicsShareAPI epicsPosixMutexAttrGet (EpicsPosixMutexProperty);
epicsShareFunc int                   epicsShareAPI epicsPosixMutexInit    (pthread_mutex_t *,EpicsPosixMutexProperty);

#ifdef __cplusplus
}
#endif

#endif /* osdMutexh */
