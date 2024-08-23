#pragma once


/**
 * A type wide enough to hold the output of "socket()" or "accept()".  On
 * Windows, this is an intptr_t; elsewhere, it is an int. */
#ifdef _WIN32
//#include <windows.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif
typedef intptr_t IOHandle;
#else
typedef int IOHandle;
#endif


#ifdef _WIN32

    #if defined(SMP_DLLEXPORT)
        #define SMP_UTIL_API __declspec(dllexport)
    #elif defined(SMP_DLLIMPORT)
        #define SMP_UTIL_API __declspec(dllimport)
    #else
        #define SMP_UTIL_API
    #endif
#else
    #define SMP_UTIL_API
#endif


