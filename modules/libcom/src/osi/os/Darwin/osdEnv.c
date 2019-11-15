/*************************************************************************\
* Copyright (c) 2002 The University of Saskatchewan
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/* osdEnv.c */
/*
 * Author: Eric Norum
 *   Date: May 7, 2001
 *
 * Routines to modify/display environment variables and EPICS parameters
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

/*
 * Starting in Mac OS X 10.5 (Leopard) shared libraries and
 * bundles don't have direct access to environ (man environ).
 */
#include <crt_externs.h>
#define environ (*_NSGetEnviron())

#include "epicsStdio.h"
#include "envDefs.h"
#include "iocsh.h"

/*
 * Set the value of an environment variable
 */
LIBCOM_API void LIBCOMSTD_API epicsEnvSet (const char *name, const char *value)
{
    if (!name) return;
    iocshEnvClear(name);
    setenv(name, value, 1);
}

/*
 * Unset an environment variable
 */

LIBCOM_API void LIBCOMSTD_API epicsEnvUnset (const char *name)
{
    iocshEnvClear(name);
    unsetenv(name);
}

/*
 * Show the value of the specified, or all, environment variables
 */
LIBCOM_API void LIBCOMSTD_API epicsEnvShow (const char *name)
{
    if (name == NULL) {
        extern char **environ;
        char **sp;

        for (sp = environ ; (sp != NULL) && (*sp != NULL) ; sp++)
            printf ("%s\n", *sp);
    }
    else {
        const char *cp = getenv (name);
        if (cp == NULL)
            printf ("%s is not an environment variable.\n", name);
        else
            printf ("%s=%s\n", name, cp);
    }
}
