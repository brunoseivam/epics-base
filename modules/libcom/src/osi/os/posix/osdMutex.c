/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* SPDX-License-Identifier: EPICS
* EPICS Base is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/
/* osi/os/posix/osdMutex.c */

/* Author:  Marty Kraimer Date:    13AUG1999 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>

#include "epicsMutex.h"
#include "cantProceed.h"
#include "epicsTime.h"
#include "errlog.h"
#include "epicsAssert.h"
#include "envDefs.h"

#define checkStatus(status,message) \
    if((status)) { \
        errlogPrintf("epicsMutex %s failed: error %s\n", \
            (message), strerror((status))); \
    }
#define checkStatusQuit(status,message,method) \
    if(status) { \
        errlogPrintf("epicsMutex %s failed: error %s\n", \
            (message), strerror((status))); \
        cantProceed((method)); \
    }

/* Until these can be demonstrated to work leave them undefined*/
/* On solaris 8 _POSIX_THREAD_PRIO_INHERIT fails*/
#if defined(DONT_USE_POSIX_THREAD_PRIORITY_SCHEDULING)
#undef _POSIX_THREAD_PRIO_INHERIT
#endif

#if defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE)>=500
#define HAVE_RECURSIVE_MUTEX
#else
#undef  HAVE_RECURSIVE_MUTEX
#endif

/* Global var - pthread_once does not support passing args but it is more efficient
 * then epicsThreadOnce which always acquires a mutex.
 */
static pthread_mutexattr_t globalAttrDefault;
#ifdef HAVE_RECURSIVE_MUTEX
static pthread_mutexattr_t globalAttrRecursive;
#endif
static pthread_once_t      globalAttrInitOnce = PTHREAD_ONCE_INIT;

static void setAttrDefaults(pthread_mutexattr_t *a)
{
    int status;

    status = pthread_mutexattr_init(a);
    checkStatusQuit(status,"pthread_mutexattr_init","setAttrDefaults");

    {
        const char *p = envGetConfigParamPtr(&EPICS_MUTEX_USE_PRIORITY_INHERITANCE);
        char        c = p ? toupper(p[0]) : 'N';
        if ( 'T' == c || 'Y' == c || '1' == c ) {
#if defined _POSIX_THREAD_PRIO_INHERIT
            status = pthread_mutexattr_setprotocol(a, PTHREAD_PRIO_INHERIT);
            if (errVerbose) checkStatus(status, "pthread_mutexattr_setprotocol(PTHREAD_PRIO_INHERIT)");
#ifndef HAVE_RECURSIVE_MUTEX
            /* The implementation based on a condition variable below does not support 
             * priority-inheritance!
             */
            fprintf(stderr,"WARNING: PRIORITY-INHERITANCE UNAVAILABLE for epicsMutex\n");
#endif
#else
            fprintf(stderr,"WARNING: PRIORITY-INHERITANCE UNAVAILABLE OR NOT COMPILED IN\n");
#endif
        }
    }
}

static void globalAttrInit()
{
    int status;

    setAttrDefaults( &globalAttrDefault );

#ifdef HAVE_RECURSIVE_MUTEX
    setAttrDefaults( &globalAttrRecursive );
    status = pthread_mutexattr_settype(&globalAttrRecursive, PTHREAD_MUTEX_RECURSIVE);
    checkStatusQuit(status, "pthread_mutexattr_settype(PTHREAD_MUTEX_RECURSIVE)", "globalAttrInit");
#endif
}

epicsShareFunc pthread_mutexattr_t * epicsShareAPI epicsPosixMutexAttrGet (EpicsPosixMutexProperty p)
{
    int status;

    status = pthread_once( &globalAttrInitOnce, globalAttrInit );
    checkStatusQuit(status,"pthread_once","epicsPosixMutexAttrGet");
    switch ( p ) {
        default:
        case posixMutexDefault:
            break;
        case posixMutexRecursive:
#ifdef HAVE_RECURSIVE_MUTEX
            return &globalAttrRecursive;
#else
            return 0;
#endif
    }
    return &globalAttrDefault;
}

epicsShareFunc int epicsShareAPI epicsPosixMutexInit (pthread_mutex_t *m, EpicsPosixMutexProperty p)
{
    pthread_mutexattr_t *atts = epicsPosixMutexAttrGet( p );

    if ( ! atts )
        return ENOTSUP;
    return pthread_mutex_init(m, atts);
}

epicsShareFunc void epicsShareAPI epicsPosixMutexMustInit (pthread_mutex_t *m, EpicsPosixMutexProperty p)
{
    int status;

    status = epicsPosixMutexInit(m, p);
    checkStatusQuit(status,"pthread_mutex_init","epicsMustInitPosixMutex");
}


static int mutexLock(pthread_mutex_t *id)
{
    int status;

    while ((status = pthread_mutex_lock(id)) == EINTR) {
        errlogPrintf("pthread_mutex_lock returned EINTR. Violates SUSv3\n");
    }
    return status;
}

/* Two completely different implementations are provided below
 * If support is available for PTHREAD_MUTEX_RECURSIVE then
 *      only pthread_mutex is used.
 * If support is not available for PTHREAD_MUTEX_RECURSIVE then
 *      a much more complicated solution is required
 */


#ifdef HAVE_RECURSIVE_MUTEX
typedef struct epicsMutexOSD {
    pthread_mutex_t     lock;
} epicsMutexOSD;

epicsMutexOSD * epicsMutexOsdCreate(void) {
    epicsMutexOSD *pmutex;
    int status;

    pmutex = calloc(1, sizeof(*pmutex));
    if(!pmutex)
        return NULL;

    status = epicsPosixMutexInit(&pmutex->lock, posixMutexRecursive);
    if (!status)
        return pmutex;

    free(pmutex);
    return NULL;
}

void epicsMutexOsdDestroy(struct epicsMutexOSD * pmutex)
{
    int status;

    status = pthread_mutex_destroy(&pmutex->lock);
    checkStatus(status, "pthread_mutex_destroy");
    free(pmutex);
}

void epicsMutexOsdUnlock(struct epicsMutexOSD * pmutex)
{
    int status;

    status = pthread_mutex_unlock(&pmutex->lock);
    checkStatus(status, "pthread_mutex_unlock epicsMutexOsdUnlock");
}

epicsMutexLockStatus epicsMutexOsdLock(struct epicsMutexOSD * pmutex)
{
    int status;

    status = mutexLock(&pmutex->lock);
    if (status == EINVAL) return epicsMutexLockError;
    if(status) {
        errlogMessage("epicsMutex pthread_mutex_lock failed: error epicsMutexOsdLock\n");
        return epicsMutexLockError;
    }
    return epicsMutexLockOK;
}

epicsMutexLockStatus epicsMutexOsdTryLock(struct epicsMutexOSD * pmutex)
{
    int status;

    if (!pmutex) return epicsMutexLockError;
    status = pthread_mutex_trylock(&pmutex->lock);
    if (status == EINVAL) return epicsMutexLockError;
    if (status == EBUSY) return epicsMutexLockTimeout;
    if(status) {
        errlogMessage("epicsMutex pthread_mutex_trylock failed: error epicsMutexOsdTryLock");
        return epicsMutexLockError;
    }
    return epicsMutexLockOK;
}

void epicsMutexOsdShow(struct epicsMutexOSD * pmutex, unsigned int level)
{
    /* GLIBC w/ NTPL is passing the &lock.__data.__lock as the first argument (UADDR)
     * of the futex() syscall.  __lock is at offset 0 of the enclosing structures.
     */
    printf("    pthread_mutex_t* uaddr=%p\n", &pmutex->lock);
}

#else /* #ifdef HAVE_RECURSIVE_MUTEX */

/* The standard EPICS implementation of a recursive mutex (in absence of native support)
 * does not allow for priority-inheritance:
 * a low priority thread may hold the ('soft-') mutex, i.e., may be preempted in the
 * critical section without the high-priority thread noticing (because the HP-thread is
 * sleeping on the condvar and not waiting for the mutex).
 *
 * A better implementation could be:
 *
 *  struct epicsMutexOSD {
 *    pthread_mutex_t   mtx;
 *    atomic<pthread_t> owner;
 *    unsigned          count;
 *  };
 *
 * void mutexLock(struct epicsMutexOSD *m)
 * {
 *   pthread_t currentOwner = atomic_load(&m->owner, acquire);   
 *
 *   if ( pthread_equal(currentOwner, pthread_self()) ) {
 *     m->count++;
 *     return;
 *   }
 *
 *   pthread_mutex_lock(&m->mtx);
 *   // ordering of this write to 'owner' is irrelevant since it doesn't matter
 *   // if another thread performing the test above sees the 'invalid' or already
 *   // 'our' TID. 
 *   atomic_store(&m->owner, pthread_self(), relaxed);
 *   // 'count' is only ever accessed with 'mtx' held
 *   m->count = 1;
 * }
 *
 * void mutexUnlock(struct epicsMutexOSD *o)
 * {
 *   o->count--;
 *   if ( o->count == 0 ) {
 *     // acquire-release ordering between here and 'mutexLock' above'!
 *     // Theoretically (but extremely unlikely) the executing thread
 *     // may go away and a newly created thread with the same (recycled)
 *     // TID on a different CPU could still see the old TID in mutexLock
 *     // and believe it already owns the mutex...
 *     atomic_store(&m->owner, invalid_thread_id, release);
 *     pthread_mutex_unlock( &o->mtx );
 *   }
 *
 * The 'invalid_thread_id' could be an ID of a permanently suspended dummy thread
 * (pthread does not define a 'NULL' ID and you don't want to get into an 'ABA'-sort
 * of situation where 'mutexLock' believes to be the current owner because the 'invalid'
 * ID is a 'recycled' thread id).
 *
 * Without atomic operations we'd have to introduce a second mutex to protect the 'owner'
 * member ('count' is only ever accessed with the mutex held). But that would then
 * lead to two extra lock/unlock pairs in 'mutexLock'. A dirty version would ignore that
 * and rely on pthread_t fitting in a CPU word and the acquire/release corner-case mentioned
 * above to never happen. Plus, some CPUs (x86) are more strongly (acq/rel) ordered implicitly.
 *
 *   Here the corner case again:
 *
 *        CPU1                  CPU2
 *     owner = 1234
 *     ...
 *     owner = invalid_tid
 *     mutex_unlock()
 *
 *     thread 1234 dies         new thread with recycled TID 1234
 *                              enters osdMutexLock
 *                              'owner=invalid_tid' assignment not yet visible on this CPU
 *                              if ( pthread_equal( owner, pthread_self() ) ) {
 *                                    ==> ERRONEOUSLY ENTERS HERE
 *                              }
 */

typedef struct epicsMutexOSD {
    pthread_mutex_t     lock;
    pthread_cond_t      waitToBeOwner;
    int                 count;
    int                 owned;  /* TRUE | FALSE */
    pthread_t           ownerTid;
} epicsMutexOSD;

epicsMutexOSD * epicsMutexOsdCreate(void) {
    epicsMutexOSD *pmutex;
    int           status;

    pmutex = calloc(1, sizeof(*pmutex));
    if(!pmutex)
        return NULL;

    epicsPosixMutexMustInit(&pmutex->lock, posixMutexDefault);

    status = pthread_cond_init(&pmutex->waitToBeOwner, 0);
    if(!status)
        return pmutex;

    pthread_mutex_destroy(&pmutex->lock);
    free(pmutex);
    return NULL;
}

void epicsMutexOsdDestroy(struct epicsMutexOSD * pmutex)
{
    int   status;

    status = pthread_cond_destroy(&pmutex->waitToBeOwner);
    checkStatus(status, "pthread_cond_destroy");
    status = pthread_mutex_destroy(&pmutex->lock);
    checkStatus(status, "pthread_mutex_destroy");
    free(pmutex);
}

void epicsMutexOsdUnlock(struct epicsMutexOSD * pmutex)
{
    int status;

    status = mutexLock(&pmutex->lock);
    checkStatus(status, "pthread_mutex_lock epicsMutexOsdUnlock");
    if(status)
        return;

    if ((pmutex->count <= 0) || (pmutex->ownerTid != pthread_self())) {
        pthread_mutex_unlock(&pmutex->lock);
        checkStatus(status, "pthread_mutex_unlock epicsMutexOsdUnlock");
        errlogPrintf("epicsMutexOsdUnlock but caller is not owner\n");
        cantProceed("epicsMutexOsdUnlock but caller is not owner");
        return;
    }

    pmutex->count--;
    if (pmutex->count == 0) {
        pmutex->owned = 0;
        pmutex->ownerTid = 0;
        status = pthread_cond_signal(&pmutex->waitToBeOwner);
        checkStatusQuit(status, "pthread_cond_signal epicsMutexOsdUnlock", "epicsMutexOsdUnlock");
    }

    status = pthread_mutex_unlock(&pmutex->lock);
    checkStatus(status, "pthread_mutex_unlock epicsMutexOsdUnlock");
}

static int condWait(pthread_cond_t *condId, pthread_mutex_t *mutexId)
{
    int status;

    while ((status = pthread_cond_wait(condId, mutexId)) == EINTR) {
        errlogPrintf("pthread_cond_wait returned EINTR. Violates SUSv3\n");
    }
    return status;
}

epicsMutexLockStatus epicsMutexOsdLock(struct epicsMutexOSD * pmutex)
{
    pthread_t tid = pthread_self();
    int status;

    if (!pmutex || !tid) return epicsMutexLockError;
    status = mutexLock(&pmutex->lock);
    if (status == EINVAL) return epicsMutexLockError;
    checkStatus(status, "pthread_mutex_lock epicsMutexOsdLock");
    if(status)
        return epicsMutexLockError;

    while (pmutex->owned && !pthread_equal(pmutex->ownerTid, tid))
        condWait(&pmutex->waitToBeOwner, &pmutex->lock);
    pmutex->ownerTid = tid;
    pmutex->owned = 1;
    pmutex->count++;

    status = pthread_mutex_unlock(&pmutex->lock);
    checkStatus(status, "pthread_mutex_unlock epicsMutexOsdLock");
    if(status)
        return epicsMutexLockError;
    return epicsMutexLockOK;
}

epicsMutexLockStatus epicsMutexOsdTryLock(struct epicsMutexOSD * pmutex)
{
    pthread_t tid = pthread_self();
    epicsMutexLockStatus result;
    int status;

    status = mutexLock(&pmutex->lock);
    if (status == EINVAL) return epicsMutexLockError;
    checkStatus(status, "pthread_mutex_lock epicsMutexOsdTryLock");
    if(status)
        return epicsMutexLockError;

    if (!pmutex->owned || pthread_equal(pmutex->ownerTid, tid)) {
        pmutex->ownerTid = tid;
        pmutex->owned = 1;
        pmutex->count++;
        result = epicsMutexLockOK;
    } else {
        result = epicsMutexLockTimeout;
    }

    status = pthread_mutex_unlock(&pmutex->lock);
    checkStatus(status, "pthread_mutex_unlock epicsMutexOsdTryLock");
    if(status)
        return epicsMutexLockError;
    return result;
}

void epicsMutexOsdShow(struct epicsMutexOSD *pmutex,unsigned int level)
{
    printf("ownerTid %p count %d owned %d\n",
        (void *)pmutex->ownerTid, pmutex->count, pmutex->owned);
}
#endif /* #ifdef HAVE_RECURSIVE_MUTEX */
