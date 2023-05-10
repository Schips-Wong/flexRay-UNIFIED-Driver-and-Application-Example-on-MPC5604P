/*****************************************************************************
*
* Motorola Inc.
* (c) Copyright 2000 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
******************************************************************************
*
* File Name: types.h
*
* Description: structures, prototypes, definitions for portable code
*              ITU definition of intrinsic functions
*
* Modules Included:
*
*****************************************************************************/


#ifndef __C_TYPES_H__
#define __C_TYPES_H__

/*******************************************************
* Target designation
*******************************************************/

/* MetroWerks defines __POWERPC__ */
/* MetroWerks defines __PPC_EABI__ */

/*******************************************************
* C Constructs
*******************************************************/

#define ITU_INTRINSICS

/*******************************************************
* Basic Types 
*******************************************************/

#ifdef __MWERKS__

    #include <stdint.h>

#endif   /* defined (__MWERKS__) */



#ifdef __DCC__
// This is needed because Diab does not currently have a stdint.h file

    typedef signed   char int8_t;
    typedef unsigned char uint8_t;
    typedef volatile signed char vint8_t;
    typedef volatile unsigned char vuint8_t;

    typedef signed   short int int16_t;
    typedef unsigned short int uint16_t;
    typedef volatile signed short vint16_t;
    typedef volatile unsigned short vuint16_t;

    typedef signed   int int32_t;
    typedef unsigned int uint32_t;
    typedef volatile signed int vint32_t;
    typedef volatile unsigned int vuint32_t;

    typedef signed   long long int64_t;
    typedef unsigned long long uint64_t;
    typedef volatile signed   long long vint64_t;
    typedef volatile unsigned long long vuint64_t;

#endif /* defined (__DCC__) */

#ifdef __ghs__    //GreenHills
    #include <stdint.h>
#endif

/* Generic word types for ITU compatibility */
typedef int16_t             Word16;
typedef uint16_t            UWord16;
typedef int32_t             Word32;
typedef uint32_t            UWord32;

typedef int16_t             Int16;
typedef uint16_t            UInt16;
typedef int32_t             Int32;
typedef uint32_t            UInt32;

/* Generic word types extension */
typedef int8_t             Word8;
typedef uint8_t            UWord8;

typedef int8_t             Int8;
typedef uint8_t            UInt8;

typedef int64_t            Word64;
typedef uint64_t           UWord64;

typedef int64_t            Int64;
typedef uint64_t           UInt64;


/* Fractional data types for portability */
typedef Word16             Frac16;
typedef Word32             Frac32;

/* Floating point types */
typedef float               REAL;
//typedef double              REAL;

/* Miscellaneous types */
typedef UInt16             Flag;
typedef UInt16             flag;

#ifndef COMPILER_HAS_BOOL
typedef int                 bool;
#endif

typedef void (*tPointerFcn)(void); /* pointer to void function */
typedef struct tcd_t sTCD_t;

/*******************************************************
* Constants
*******************************************************/
/* Function Result Values */

#ifndef COMPILER_HAS_BOOL
    #define true            1
    #define false           !true
#endif

#ifndef NULL
    #define NULL            ((void *)0)
    /* or for C++   #define NULL 0    */
#endif


/* limit definitions */
#define MAX_16              (Word16)0x7FFF
#define MIN_16              (Word16)0x8000

#define MAX_32              (Word32)0x7FFFFFFFL
#define MIN_32              (Word32)0x80000000L

/* useful macros */
#define FRAC16(x) ((Frac16)((x) < 0.999969482421875 ? ((x) >= -1 ? (x)*0x8000 : 0x8000) : 0x7FFF))
#define FRAC32(x) ((Frac32)((x) < 1 ? ((x) >= -1 ? (x)*0x80000000 : 0x80000000) : 0x7FFFFFFF))



#endif  /* __C_TYPES_H__ */
