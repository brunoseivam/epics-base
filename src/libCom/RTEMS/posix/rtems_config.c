/*************************************************************************\
* Copyright (c) 2002 The University of Saskatchewan
* Copyright (c) 2017 Fritz-Haber-Institut der Max-Planck-Gesellschafto* EPICS
* BASE is distributed subject to a Software License Agreement found
* * in file LICENSE that is included with this distribution.
\*************************************************************************/
/*
 * RTEMS configuration for EPICS
 *      Author: W. Eric Norum
 *              Heinz Junkes
 *
 * Version for RTEMS-4.12
 */

#include <rtems.h>

/*
 ***********************************************************************
 *                         RTEMS CONFIGURATION                         *
 ***********************************************************************
 */

extern void *EPICS_WITH_POSIX_Init(void *argument);
 
#define CONFIGURE_POSIX_INIT_THREAD_TABLE
#define CONFIGURE_POSIX_INIT_THREAD_ENTRY_POINT EPICS_WITH_POSIX_Init
/*
 * nfs is using rtems tasks
 */
#define CONFIGURE_MAXIMUM_TASKS rtems_resource_unlimited(5) 
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES    rtems_resource_unlimited(5)
#define CONFIGURE_MAXIMUM_SEMAPHORES  rtems_resource_unlimited(5)
#define CONFIGURE_MAXIMUM_EVENTS    rtems_resource_unlimited(5)

#define CONFIGURE_MAXIMUM_POSIX_MUTEXES     rtems_resource_unlimited(500)
#define CONFIGURE_MAXIMUM_POSIX_THREADS     rtems_resource_unlimited(30)
#define CONFIGURE_MAXIMUM_POSIX_KEYS        rtems_resource_unlimited(200)
#define CONFIGURE_MAXIMUM_POSIX_KEY_VALUE_PAIRS rtems_resource_unlimited(200)
#define CONFIGURE_MAXIMUM_POSIX_SPINLOCKS   rtems_resource_unlimited(10)
#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES  rtems_resource_unlimited(500)
#define CONFIGURE_MAXIMUM_POSIX_TIMERS      rtems_resource_unlimited(20)
#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES    rtems_resource_unlimited(5)
#define CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES rtems_resource_unlimited(10)
 
#define CONFIGURE_UNIFIED_WORK_AREAS

#define CONFIGURE_MAXIMUM_PERIODS rtems_resource_unlimited(3)
#define CONFIGURE_MICROSECONDS_PER_TICK 10000
#define CONFIGURE_MALLOC_STATISTICS     1

#define CONFIGURE_EXTRA_TASK_STACKS         (50 * RTEMS_MINIMUM_STACK_SIZE)
 
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
 
#define CONFIGURE_FILESYSTEM_DEVFS
#define CONFIGURE_FILESYSTEM_TFTPFS
#define CONFIGURE_FILESYSTEM_NFS
#define CONFIGURE_FILESYSTEM_IMFS
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 150
 
#define CONFIGURE_MAXIMUM_NFS_MOUNTS rtems_resource_unlimited(3)
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS 5

#define CONFIGURE_POSIX_INIT_THREAD_STACK_SIZE  (32*1024)
 
#define CONFIGURE_MAXIMUM_DRIVERS 8

//#define CONFIGURE_INITIAL_EXTENSIONS { .fatal = fatal_extension }

#define CONFIGURE_INIT


/*
 * This should be made BSP dependent, not CPU dependent but I know of no
 * appropriate conditionals to use.
 * The new general time support makes including the RTC driverr less important.
 */
#if !defined(mpc604) && !defined(__mc68040__) && !defined(__mcf5200__) && \
    !defined(mpc7455) && !defined(__arm__)  && !defined(__nios2__)
    /* don't have RTC code */
#define CONFIGURE_APPLICATION_NEEDS_RTC_DRIVER
#endif


#include <bsp.h>
#include <rtems/confdefs.h>
