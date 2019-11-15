/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/

#ifndef epicsMathh
#define epicsMathh

#include <math.h>
#include <libComAPI.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef isfinite
#  undef finite
#  define finite(x) isfinite((double)(x))
#endif

LIBCOM_API extern float epicsNAN;
LIBCOM_API extern float epicsINF;

#ifdef __cplusplus
}
#endif

#endif /* epicsMathh */
