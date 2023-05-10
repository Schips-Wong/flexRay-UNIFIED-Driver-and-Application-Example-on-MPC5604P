/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2005 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***************************************************************************//*!
*
* @file      Fr_UNIFIED_types.h.backup
*
* @author    rg003c
* 
* @version   1.0.11.0
* 
* @date      Apr-7-2009
* 
* @brief     FlexRay UNIFIED Driver implementation
*            This file can be changed according to compiler requirements
*
******************************************************************************/

#ifndef FR_UNIFIED_TYPES
#define FR_UNIFIED_TYPES

/******************************************************************************
* Includes
******************************************************************************/


/******************************************************************************
* Code portability macros
******************************************************************************/
/* Compiler nonANSI extensions portability is ensured by uncommenting parts of this section */




/* FlexRay module specific pointer class "far", this forces to compiler to use G-instructions
for accessing the global addresses on the MC9S12 microcontrollers */

/*************** Use this for CodeWarrior Compiler for MC9S12X ***************/
/* Uncomment one from this subsection - REGISTER access*/
//#define PTR2FARREG *                     /* "far" modifier is NOT used for an access to FlexRay registers */
//#define PTR2FARREG * __far               /* "far" modifier is used for an access to FlexRay registers */

/* Uncomment one from this subsection - MEMORY acces */
//#define PTR2FARDATA *                   /* "far" modifier is NOT used for an access to FlexRay memory */
//define PTR2FARDATA * __far              /* "far" modifier is used for an access to FlexRay memory */


/******************** Use this for Cosmic compilerfor MC9S12X ****************/
/* Uncomment one from this subsection - REGISTER access*/
//#define PTR2FARREG *                     /* "far" modifier is NOT used for an access to FlexRay registers */
//#define PTR2FARREG @far @gpage *         /* "far" modifier is used for an access to FlexRay registers */

/* Uncomment one from this subsection - MEMORY acces */
//#define PTR2FARDATA *                    /* "far" modifier is NOT used for an access to FlexRay memory */
//#define PTR2FARDATA @far @gpage *        /* "far" modifier is used for an access to FlexRay memory */


/************** Use this for other platforms than MC9S12X ********************/
/* Uncomment both from this section */
#define PTR2FARDATA *                   /* No modifier used for an access to FlexRay registers */
#define PTR2FARREG *                    /* No modifier used for an access to FlexRay memory */


/******************************************************************************
* Types
******************************************************************************/

typedef unsigned char       boolean;

typedef signed char         sint8;
typedef unsigned char       uint8;
typedef signed short int    sint16;
typedef unsigned short int  uint16;
typedef signed long int     sint32;
typedef unsigned long int   uint32;
typedef unsigned short int  uint8_least;
typedef unsigned int        uint16_least;
typedef unsigned long int   uint32_least;
typedef signed short int    sint8_least;
typedef signed int          sint16_least;
typedef signed long int     sint32_least;

typedef float               float32;
typedef double              float64;

typedef uint8 Fr_return_type;

#define FR_NOT_SUCCESS  0U
#define FR_SUCCESS      1U

#ifndef TRUE
    #define TRUE ((boolean) 1)
#endif

#ifndef FALSE
    #define FALSE ((boolean) 0)
#endif

#ifndef NULL
    #define NULL (void *)(0)
#endif

#endif /* FR_UNIFIED_TYPES */
