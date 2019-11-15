/*************************************************************************\
* Copyright (c) 2009 UChicago Argonne LLC, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/

/* epicsStdio.h */

#ifndef epicsStdioh
#define epicsStdioh

#include <stdio.h>
#include <stdarg.h>

#include "libComAPI.h"
#include "compilerDependencies.h"
#include "epicsTempFile.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef epicsStdioStdStreams
#  undef stdin
#  define stdin epicsGetStdin()
#  undef stdout
#  define stdout epicsGetStdout()
#  undef stderr
#  define stderr epicsGetStderr()
#endif

/* Make printf, puts and putchar use *our* version of stdout */

#ifndef epicsStdioStdPrintfEtc
#  ifdef printf
#    undef printf
#  endif
#  define printf epicsStdoutPrintf

#  ifdef puts
#    undef puts
#  endif
#  define puts epicsStdoutPuts

#  ifdef putchar
#    undef putchar
#  endif
#  define putchar epicsStdoutPutchar
#endif

LIBCOM_API int LIBCOMSTD_API epicsSnprintf(
    char *str, size_t size, const char *format, ...) EPICS_PRINTF_STYLE(3,4);
LIBCOM_API int LIBCOMSTD_API epicsVsnprintf(
    char *str, size_t size, const char *format, va_list ap);

/*
 * truncate to specified size (we dont use truncate()
 * because it is not portable)
 *
 * pFileName - name (and optionally path) of file
 * size - the new file size (if file is curretly larger)
 * 
 * returns TF_OK if the file is less than size bytes
 * or if it was successfully truncated. Returns
 * TF_ERROR if the file could not be truncated.
 */
enum TF_RETURN {TF_OK=0, TF_ERROR=1};
LIBCOM_API enum TF_RETURN truncateFile ( const char *pFileName, unsigned long size );

/* The following are for redirecting stdin,stdout,stderr */
LIBCOM_API FILE * LIBCOMSTD_API epicsGetStdin(void);
LIBCOM_API FILE * LIBCOMSTD_API epicsGetStdout(void);
LIBCOM_API FILE * LIBCOMSTD_API epicsGetStderr(void);
/* These are intended for iocsh only */
LIBCOM_API FILE * LIBCOMSTD_API epicsGetThreadStdin(void);
LIBCOM_API FILE * LIBCOMSTD_API epicsGetThreadStdout(void);
LIBCOM_API FILE * LIBCOMSTD_API epicsGetThreadStderr(void);
LIBCOM_API void  LIBCOMSTD_API epicsSetThreadStdin(FILE *);
LIBCOM_API void  LIBCOMSTD_API epicsSetThreadStdout(FILE *);
LIBCOM_API void  LIBCOMSTD_API epicsSetThreadStderr(FILE *);

LIBCOM_API int LIBCOMSTD_API epicsStdoutPrintf(
    const char *pformat, ...) EPICS_PRINTF_STYLE(1,2);
LIBCOM_API int LIBCOMSTD_API epicsStdoutPuts(const char *str);
LIBCOM_API int LIBCOMSTD_API epicsStdoutPutchar(int c);

#ifdef  __cplusplus
}

/* Also pull functions into the std namespace (see lp:1786927) */
#if !defined(__GNUC__) || (__GNUC__ > 2)
namespace std {
using ::epicsGetStdin;
using ::epicsGetStdout;
using ::epicsGetStderr;
using ::epicsStdoutPrintf;
using ::epicsStdoutPuts;
using ::epicsStdoutPutchar;
}
#endif /* __GNUC__ > 2 */

#endif /* __cplusplus */

#endif /* epicsStdioh */
