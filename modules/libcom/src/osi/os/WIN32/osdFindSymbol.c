/*************************************************************************\
* Copyright (c) 2013 Dirk Zimoch, PSI
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/* osi/os/WIN32/osdFindSymbol.c */


#include <windows.h>

#include "epicsFindSymbol.h"

static int epicsLoadErrorCode = 0;

LIBCOM_API void * epicsLoadLibrary(const char *name)
{
    HMODULE lib;

    epicsLoadErrorCode = 0;
    lib = LoadLibrary(name);
    if (lib == NULL)
    {
        epicsLoadErrorCode = GetLastError();
    }
    return lib;
}

LIBCOM_API const char *epicsLoadError(void)
{
    static char buffer[100];

    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        epicsLoadErrorCode,
        0,
        buffer,
        sizeof(buffer)-1, NULL );
    return buffer;
}

LIBCOM_API void * LIBCOMSTD_API epicsFindSymbol(const char *name)
{
    return GetProcAddress(0, name);
}
