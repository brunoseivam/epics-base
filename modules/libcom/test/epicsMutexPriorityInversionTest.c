#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <features.h>
#include <sched.h>
#include <epicsThread.h>
#include <epicsEvent.h>
#include <epicsMutex.h>
#include <epicsUnitTest.h>
#include <errlog.h>
#include <testMain.h>
#include <time.h>
#include <envDefs.h>
#include <stdio.h>

#define  THE_CPU   0

#define  SPIN_SECS 3000000

#if defined(__GLIBC__) && defined(__GLIBC_PREREQ) && __GLIBC_PREREQ(2,4)
#define HAVE_CPU_AFFINITY
#else
#undef  HAVE_CPU_AFFINITY
#endif

typedef struct ThreadArg_ {
    epicsEventId           sync;
    epicsEventId           done;
    epicsMutexId           mtx;
    volatile unsigned long end;
    unsigned long          lim;
    int                   pri[3];
} ThreadArg;

static unsigned long getClockUs()
{
    struct timespec now;
    if ( clock_gettime( CLOCK_MONOTONIC, &now ) ) {
        testAbort("clock_gettime(CLOCK_MONOTONIC) failed");
    }
    return ((unsigned long)now.tv_sec)*1000000 + ((unsigned long)now.tv_nsec)/1000;
}

/*
 * Set affinity of executing thread to 'cpu'
 */
static void setThreadAffinity(int cpu)
{
#ifdef HAVE_CPU_AFFINITY
    cpu_set_t cset;

    CPU_ZERO( &cset );
    CPU_SET( cpu, &cset );
    testOk1 ( 0 == pthread_setaffinity_np( pthread_self(), sizeof(cset), &cset ) );
#endif
}

/*
 * Ensure only 'cpu' is set in executing thread's affinity mask
 */
static void checkAffinity(int cpu)
{
#ifdef HAVE_CPU_AFFINITY
    cpu_set_t cset;

    if ( pthread_getaffinity_np( pthread_self(), sizeof(cset), &cset) ) {
        testFail("pthread_getaffinity_np FAILED");
        return;
    }
    testOk ( 1 == CPU_COUNT( &cset ) && CPU_ISSET( cpu, &cset ), "Checking CPU affinity mask" );
#endif
}


/*
 * Make sure executing thread uses SCHED_FIFO and
 * store its priority a->pri[idx]
 *
 * RETURNS: 0 if SCHED_FIFO engaged, nonzero otherwise.
 */
static int checkThreadPri(ThreadArg *a, unsigned idx)
{
    int                pol;
    struct sched_param p;

    if ( pthread_getschedparam( pthread_self(), &pol, &p ) ) {
        testFail("pthread_getschedparam FAILED");
        return 1;
    }
    if ( a && idx < sizeof(a->pri)/sizeof(a->pri[0]) ) {
        a->pri[idx] = p.sched_priority;
    }
    testOk1( SCHED_FIFO == pol );
    return (SCHED_FIFO != pol);
}


/*
 * Low-priority thread.
 *
 * Lock mutex and signal to the medium-priority thread
 * that it may proceed.
 *
 * Since all three (high-, medium- and low-priority threads)
 * execute on the same CPU (via affinity) the medium-
 * priority thread will then take over the CPU and prevent
 * (unless priority-inheritance is enabled, that is)
 * the low-priority thread from continueing on to unlock
 * the mutex.
 */
static void loPriThread(void *parm)
{
    ThreadArg *a = (ThreadArg*)parm;

    checkAffinity( THE_CPU );

    checkThreadPri( a, 0 );

    epicsMutexMustLock( a->mtx );

    epicsEventSignal( a->sync );

    /* medium-priority thread takes over CPU and spins
     * while the high-priority thread waits for the mutex.
     * With priority-inheritance enabled this thread's
     * priority will be increased so that the medium-
     * priority thread can be preempted and we proceed
     * to release the mutex.
     */

    epicsMutexUnlock( a->mtx );
}

static void hiPriThread(void *parm)
{
    ThreadArg  *a = (ThreadArg*)parm;

    /* Try to get the mutex */
    epicsMutexMustLock( a->mtx );
    /* Record the time when we obtained the mutex */
    a->end = getClockUs();
    epicsMutexUnlock( a->mtx );
    /* Tell the main thread that the test done */
    epicsEventSignal( a->done );
}

static void miPriThread(void *parm)
{
    ThreadArg  *a = (ThreadArg*)parm;

    /* Basic checks:
     *  - affinity must be set to use single CPU for all threads
     *  - SCHED_FIFO must be in effect
     */
    checkAffinity( THE_CPU );
    checkThreadPri( a, 1 );

    /* Create the low-priority thread. */
    epicsThreadMustCreate("testLo",
                          a->pri[0],
                          epicsThreadGetStackSize( epicsThreadStackMedium ),
                          loPriThread,
                          a);

    /* Wait until low-priority thread has taken the mutex */
    epicsEventMustWait( a->sync );

    /* Compute the end-time for our spinning loop */
    a->lim = getClockUs() + SPIN_SECS;
    a->end = 0;

    /* Create the high-priority thread. The hiPri thread will
     * block for the mutex and
     *  if priority-inheritance is available:
     *    increase the low-priority thread's priority temporarily
     *    so it can proceed to release the mutex and hand it to
     *    the high-priority thread.
     *  if priority-inheritance is not available:
     *    the high-priority thread will have to wait until we are
     *    done spinning and the low-priority thread is scheduled
     *    again.
     */
    epicsThreadMustCreate("testHi",
                          a->pri[2],
                          epicsThreadGetStackSize( epicsThreadStackMedium ),
                          hiPriThread,
                          a);

    /* Spin for some time; the goal is hogging the CPU and thus preventing
     * the low-priority thread from being scheduled.
     * If priority-inheritance is enabled then the low-priority thread's
     * priority is temporarily increased so that we can be preempted. This
     * then causes the high-priority thread to record the 'end' time which
     * tells us that we can terminate early...
     */
    while ( 0 == a->end && getClockUs() < a->lim )
        /* spin */;

    /* w/o priority-inheritance the low-priority thread may proceed at
     * this point and release the mutex to the high-priority thread.
     */
}


#define NUM_TESTS 8

MAIN(epicsMutexPriorityInversionTest)
{
    ThreadArg a;
    long      hiPriStalledTimeUs;
    struct    sched_param p_pri;

    /* This happens too late - i.e., after initialization of libCom
     * user must set in the environment...
    epicsEnvSet("EPICS_MUTEX_USE_PRIORITY_INHERITANCE","YES");
     */

    a.mtx  = epicsMutexMustCreate();
    a.sync = epicsEventMustCreate( epicsEventEmpty );
    a.done = epicsEventMustCreate( epicsEventEmpty );
    a.pri[0] = epicsThreadPriorityLow;
    a.pri[1] = epicsThreadPriorityMedium;
    a.pri[2] = epicsThreadPriorityHigh;

    testPlan(NUM_TESTS);

#ifndef HAVE_CPU_AFFINITY
#warning "glibc too old for this test: pthread_setaffinity_np() not available..."
    testSkip( NUM_TESTS, "glibc too old for this test: pthread_setaffinity_np() not implemented!" );
    return testDone();
#endif

    p_pri.sched_priority = sched_get_priority_min( SCHED_FIFO );
    if ( sched_setscheduler( 0, SCHED_FIFO, &p_pri ) ) {
        testDiag("SCHED_FIFO not engaged - maybe you need to be root to run this test?");
        testFail("sched_setscheduler(SCHED_FIFO)");
        testSkip( NUM_TESTS - 1, "SCHED_FIFO not engaged" );
        return testDone();
    } else {
        testPass("SCHED_FIFO can be used");
    }

    setThreadAffinity( THE_CPU );
    /* created threads should inherit CPU affinity mask */

    epicsThreadMustCreate("testMi",
                          a.pri[1],
                          epicsThreadGetStackSize( epicsThreadStackMedium ),
                          miPriThread,
                          &a);

    epicsEventMustWait( a.done );

    testOk1( (a.pri[0] < a.pri[1]) && (a.pri[1] < a.pri[2]) );

    hiPriStalledTimeUs = a.end - (a.lim - SPIN_SECS);

    testDiag("High-priority thread stalled for %li us\n", hiPriStalledTimeUs);
    testOk1( hiPriStalledTimeUs <  200 );

    epicsEventDestroy( a.done );
    epicsEventDestroy( a.sync );
    epicsMutexDestroy( a.mtx  );

    return testDone();
}
