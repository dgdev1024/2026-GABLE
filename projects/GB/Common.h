/**
 * @file    GB/Common.h
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-15
 * 
 * @brief   Contains includes and definitions commonly used by the Game Boy
 *          Emulation Core library, and its client libraries and applications.
 */

#pragma once

/* Public Includes ************************************************************/

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Public Constant Macros - Import/Export Symbols *****************************/

#if defined(GB_BUILD_STATIC)
    #define GB_API
#else
    #if defined(_WIN32) || defined(__CYGWIN__)
        #if defined(GB_BUILD_SHARED)
            #define GB_API __declspec(dllexport)
        #else
            #define GB_API __declspec(dllimport)
        #endif
    #else
        #if __GNUC__ >= 4
            #define GB_API __attribute__((visibility("default")))
        #else
            #define GB_API
        #endif
    #endif
#endif

/* Public Constant Macros - Attributes ****************************************/

#if defined(__GNUC__) || defined(__clang__)
    #define GB_UNUSED __attribute__((unused))
    #define GB_PACKED __attribute__((packed))
#elif defined(_MSC_VER)
    #define GB_UNUSED
    #define GB_PACKED __pragma(pack(push, 1))
#else
    #define GB_UNUSED
    #define GB_PACKED
#endif

/* Public Function Macros - Logging *******************************************/

#define gbLog(stream, ...) \
    do \
    { \
        fprintf((stream), "[%s] ", __FUNCTION__); \
        fprintf((stream), __VA_ARGS__); \
        fprintf((stream), "\n"); \
        fflush((stream)); \
    } while (0)
#define gbLogInfo(...)  gbLog(stdout, "[INFO] " __VA_ARGS__)
#define gbLogWarn(...)  gbLog(stderr, "[WARN] " __VA_ARGS__)
#define gbLogError(...) gbLog(stderr, "[ERROR] " __VA_ARGS__)
#define gbLogErrno(...) \
    do \
    { \
        fprintf(stderr, "[%s] [ERROR] ", __FUNCTION__); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, ": '%s'\n", strerror(errno)); \
        fflush(stderr); \
    } while (0)

/* Public Function Macros - Memory ********************************************/

#define gbCreate(count, type) ((type*) malloc((count) * sizeof(type)))
#define gbCreateZero(count, type) ((type*) calloc((count), sizeof(type)))
#define gbResize(ptr, count, type) ((type*) realloc((ptr), (count) * sizeof(type)))
#define gbDestroy(ptr) \
    do \
    { \
        if (ptr != nullptr) \
        { \
            free(ptr); \
            ptr = nullptr; \
        } \
    } while (0)
#define gbFallback(ptr, fallback) \
    do \
    { \
        if ((ptr) == nullptr) \
        { \
            (ptr) = (fallback); \
        } \
    } while (0)

/* Public Function Macros - Checks and Assertion ******************************/

#if defined(GB_DEBUG) || defined(GB_ENABLE_ASSERTS)
    #include <assert.h>
    #define gbAssert(clause) assert(clause)
#else
    #define gbAssert(clause) ((void)0)
#endif

#define gbCheck(clause, ...) \
    do \
    { \
        if (!(clause)) \
        { \
            gbLogError(__VA_ARGS__); \
            return; \
        } \
    } while (0)
#define gbCheckv(clause, val, ...) \
    do \
    { \
        if (!(clause)) \
        { \
            gbLogError(__VA_ARGS__); \
            return (val); \
        } \
    } while (0)
#define gbCheckp(clause, ...) \
    do \
    { \
        if (!(clause)) \
        { \
            gbLogErrno(__VA_ARGS__); \
            return; \
        } \
    } while (0)
#define gbCheckpv(clause, val, ...) \
    do \
    { \
        if (!(clause)) \
        { \
            gbLogErrno(__VA_ARGS__); \
            return (val); \
        } \
    } while (0)
#define gbCheckq(clause) \
    do \
    { \
        if (!(clause)) \
        { \
            return; \
        } \
    } while (0)
#define gbCheckqv(clause, val) \
    do \
    { \
        if (!(clause)) \
        { \
            return (val); \
        } \
    } while (0)

/* Public Function Macros - Bit Checking & Manipulation ***********************/

#define gbGetBit(value, bit) ((((value) >> (bit)) & 0b1) != 0)
#define gbSetBit(value, bit) ((value) |= (1 << (bit)))
#define gbClearBit(value, bit) ((value) &= ~(1 << (bit)))
#define gbToggleBit(value, bit) ((value) ^= (1 << (bit)))
#define gbAssignBit(value, bit, on) \
    do \
    { \
        if (on) { gbSetBit((value), (bit)); } \
        else    { gbClearBit((value), (bit)); } \
    } while (0)

#define gbGetMaskAll(value, mask) (((value) & (mask)) == (mask))
#define gbGetMaskAny(value, mask) (((value) & (mask)) != 0)
#define gbSetMask(value, mask) ((value) |= (mask))
#define gbClearMask(value, mask) ((value) &= ~(mask))
#define gbToggleMask(value, mask) ((value) ^= (mask))
#define gbAssignMask(value, mask, on) \
    do \
    { \
        if (on) { gbSetMask((value), (mask)); } \
        else    { gbClearMask((value), (mask)); } \
    } while (0)
