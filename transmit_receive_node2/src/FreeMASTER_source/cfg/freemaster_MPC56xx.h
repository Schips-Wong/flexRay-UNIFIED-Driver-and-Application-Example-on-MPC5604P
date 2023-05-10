/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2006 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
****************************************************************************//*!
*
* @file   freemaster_MPC56xx.h
*
* @brief  FreeMASTER Driver hardware dependent stuff
*
* @version 1.0.4.0
* 
* @date May-22-2008
* 
*******************************************************************************/

#ifndef __FREEMASTER_MPC56xx_H
#define __FREEMASTER_MPC56xx_H

/******************************************************************************
* platform-specific default configuration
******************************************************************************/

/* use 32-bit (EX) commands by default */
#ifndef FMSTR_USE_EX_CMDS
#define FMSTR_USE_EX_CMDS 1
#endif

/* do not use 16-bit (no-EX) commands by default */
#ifndef FMSTR_USE_NOEX_CMDS
#define FMSTR_USE_NOEX_CMDS 0
#endif

/* at least one of EX or no-EX command handling must be enabled */
#if !FMSTR_USE_EX_CMDS && !FMSTR_USE_NOEX_CMDS
    #error At least one of EX or no-EX command handling must be enabled (please set FMSTR_USE_EX_CMDS)
    #undef  FMSTR_USE_EX_CMDS
    #define FMSTR_USE_EX_CMDS 1
#endif

/*****************************************************************************
* Board configuration information 
******************************************************************************/

#define FMSTR_PROT_VER           3      /* protocol version 3 */
#define FMSTR_CFG_FLAGS  FMSTR_CFGFLAG_BIGENDIAN /* board info flags */
#define FMSTR_CFG_BUS_WIDTH      1      /* data bus width */
#define FMSTR_GLOB_VERSION_MAJOR 2      /* driver version */
#define FMSTR_GLOB_VERSION_MINOR 0
#define FMSTR_IDT_STRING "MPC56xx FreeMASTER Drv."
#define FMSTR_TSA_FLAGS          0

/******************************************************************************
* platform-specific types
******************************************************************************/

typedef unsigned char  FMSTR_U8;       /* smallest memory entity */
typedef unsigned short FMSTR_U16;      /* 16bit value */
typedef unsigned long  FMSTR_U32;      /* 32bit value */

typedef signed char    FMSTR_S8;       /* signed 8bit value */
typedef signed short   FMSTR_S16;      /* signed 16bit value */
typedef signed long    FMSTR_S32;      /* signed 32bit value */

typedef unsigned char  FMSTR_FLAGS;    /* type to be union-ed with flags (at least 8 bits) */
typedef unsigned char  FMSTR_SIZE8;    /* one-byte size value */
typedef signed short   FMSTR_INDEX;    /* general for-loop index (must be signed) */

typedef unsigned char  FMSTR_BCHR;     /* type of a single character in comm.buffer */
typedef unsigned char* FMSTR_BPTR;     /* pointer within a communication buffer */

typedef unsigned long  FMSTR_SCISR;    /* data type to store SCI status register */

/******************************************************************************
* communication buffer access functions 
******************************************************************************/

void FMSTR_CopyMemory(FMSTR_ADDR nDestAddr, FMSTR_ADDR nSrcAddr, FMSTR_SIZE8 nSize);
FMSTR_BPTR FMSTR_CopyToBuffer(FMSTR_BPTR pDestBuff, FMSTR_ADDR nSrcAddr, FMSTR_SIZE8 nSize);
FMSTR_BPTR FMSTR_CopyFromBuffer(FMSTR_ADDR nDestAddr, FMSTR_BPTR pSrcBuff, FMSTR_SIZE8 nSize);
void FMSTR_CopyFromBufferWithMask(FMSTR_ADDR nDestAddr, FMSTR_BPTR pSrcBuff, FMSTR_SIZE8 nSize);

/* mixed EX and non-EX commands may occur */
#if FMSTR_USE_EX_CMDS && FMSTR_USE_NOEX_CMDS
void FMSTR_SetExAddr(FMSTR_BOOL bNextAddrIsEx);
#else
/* otherwise, we always know what addresses are used, (ignore FMSTR_SetExAddr) */
#define FMSTR_SetExAddr(bNextAddrIsEx) 
#endif

/*********************************************************************************
* communication buffer access functions. Most of them are trivial simple on HC12
*********************************************************************************/

#define FMSTR_ValueFromBuffer8(pDest, pSrc) \
    ( (*((FMSTR_U8*)(pDest)) = *(FMSTR_U8*)(pSrc)), (((FMSTR_BPTR)(pSrc))+1) )

#define FMSTR_ValueFromBuffer16(pDest, pSrc) \
    ( (*((FMSTR_U16*)(pDest)) = *(FMSTR_U16*)(pSrc)), (((FMSTR_BPTR)(pSrc))+2) )

#define FMSTR_ValueFromBuffer32(pDest, pSrc) \
    ( (*((FMSTR_U32*)(pDest)) = *(FMSTR_U32*)(pSrc)), (((FMSTR_BPTR)(pSrc))+4) )


#define FMSTR_ValueToBuffer8(pDest, src) \
    ( (*((FMSTR_U8*)(pDest)) = (FMSTR_U8)(src)), (((FMSTR_BPTR)(pDest))+1) )

#define FMSTR_ValueToBuffer16(pDest, src) \
    ( (*((FMSTR_U16*)(pDest)) = (FMSTR_U16)(src)), (((FMSTR_BPTR)(pDest))+2) )

#define FMSTR_ValueToBuffer32(pDest, src) \
    ( (*((FMSTR_U32*)(pDest)) = (FMSTR_U32)(src)), (((FMSTR_BPTR)(pDest))+4) )


#define FMSTR_SkipInBuffer(pDest, nSize) \
    ( ((FMSTR_BPTR)(pDest)) + (nSize) )


#define FMSTR_ConstToBuffer8  FMSTR_ValueToBuffer8
#define FMSTR_ConstToBuffer16 FMSTR_ValueToBuffer16

/* EX address used only: fetching 32bit word */
#if FMSTR_USE_EX_CMDS && !FMSTR_USE_NOEX_CMDS
    #define FMSTR_AddressFromBuffer(pDest, pSrc) \
        FMSTR_ValueFromBuffer32(pDest, pSrc)
    #define FMSTR_AddressToBuffer(pDest, nAddr) \
        FMSTR_ValueToBuffer32(pDest, nAddr)
        
/* no-EX address used only: fetching 16bit word  */
#elif !FMSTR_USE_EX_CMDS && FMSTR_USE_NOEX_CMDS
    #define FMSTR_AddressFromBuffer(pDest, pSrc) \
        FMSTR_ValueFromBuffer16(pDest, pSrc)
    #define FMSTR_AddressToBuffer(pDest, nAddr) \
        FMSTR_ValueToBuffer16(pDest, nAddr)
        
/* mixed addresses used, need to process it programatically */
#else
    FMSTR_BPTR FMSTR_AddressFromBuffer(FMSTR_ADDR* pAddr, FMSTR_BPTR pSrc);
    FMSTR_BPTR FMSTR_AddressToBuffer(FMSTR_BPTR pDest, FMSTR_ADDR nAddr);
#endif

#define FMSTR_GetS8(addr)  ( *(FMSTR_S8*)(addr) )
#define FMSTR_GetU8(addr)  ( *(FMSTR_U8*)(addr) )
#define FMSTR_GetS16(addr) ( *(FMSTR_S16*)(addr) )
#define FMSTR_GetU16(addr) ( *(FMSTR_U16*)(addr) )
#define FMSTR_GetS32(addr) ( *(FMSTR_S32*)(addr) )
#define FMSTR_GetU32(addr) ( *(FMSTR_U32*)(addr) )

/****************************************************************************************
* Other helper macros
*****************************************************************************************/

/* This macro assigns C pointer to FMSTR_ADDR-typed variable */
#define FMSTR_PTR2ADDR(tmpAddr,ptr) ( tmpAddr = (FMSTR_ADDR) (FMSTR_U8*) ptr )
#define FMSTR_ARR2ADDR FMSTR_PTR2ADDR

/****************************************************************************************
* Platform-specific configuration check
*****************************************************************************************/

#ifndef FMSTR_USE_SCI
#define FMSTR_USE_SCI 1
#endif

#if FMSTR_USE_SCI
    /* user must select what SCI to use */
    #ifndef FMSTR_SCI_BASE
    #error You have to define FMSTR_SCI_BASE as a base address of SCI register space
    #endif
#endif

/****************************************************************************************
* General peripheral space access macros
*****************************************************************************************/

#define FMSTR_SETBIT32(base, offset, bit)     ((*(volatile FMSTR_U32*)(((FMSTR_U8*)(base))+(offset))) |= bit)
#define FMSTR_CLRBIT32(base, offset, bit)     ((*(volatile FMSTR_U32*)(((FMSTR_U8*)(base))+(offset))) &= ~(bit))
#define FMSTR_SETREG32(base, offset, value)   ((*(volatile FMSTR_U32*)(((FMSTR_U8*)(base))+(offset))) = value)
#define FMSTR_GETREG32(base, offset)          (*(volatile FMSTR_U32*)(((FMSTR_U8*)(base))+(offset)))

#define FMSTR_SETBIT16(base, offset, bit)     ((*(volatile FMSTR_U16*)(((FMSTR_U8*)(base))+(offset))) |= bit)
#define FMSTR_CLRBIT16(base, offset, bit)     ((*(volatile FMSTR_U16*)(((FMSTR_U8*)(base))+(offset))) &= ~(bit))    
    
#define FMSTR_SETREG16(base, offset, value)   ((*(volatile FMSTR_U16*)(((FMSTR_U8*)(base))+(offset))) = value)
#define FMSTR_GETREG16(base, offset)          (*(volatile FMSTR_U16*)(((FMSTR_U8*)(base))+(offset)))

#define FMSTR_SETREG8(base, offset, value)   ((*(volatile FMSTR_U8*)(((FMSTR_U8*)(base))+(offset))) = value)
#define FMSTR_GETREG8(base, offset)          (*(volatile FMSTR_U8*)(((FMSTR_U8*)(base))+(offset)))    
/****************************************************************************************
* SCI module constants
*****************************************************************************************/

/* (UART)SCI module registers */
#define FMSTR_UARTCR_OFFSET 0x12  /* 16bit register */
#define FMSTR_LINIER_OFFSET 0x6   /* 16bit register */
#define FMSTR_LINCR1_OFFSET 0x2   /* 16bit register */
#define FMSTR_UARTSR_OFFSET 0x16  /* 16bit register */
#define FMSTR_BDRL_DATA0_OFFSET   0x3B  /* 32bit register output*/
#define FMSTR_BDRM_DATA4_OFFSET   0x3F  /* 32bit register input*/

/* UARTCR Control Register 1 bits */
#define FMSTR_UARTCR_TXEN        0x0010
#define FMSTR_UARTCR_RXEN        0x0020

/* LINIER Control Register 1 bits */    
#define FMSTR_LINIER_DRIE       0x0004
#define FMSTR_LINIER_DTIE       0x0002
    

/* UART(SCI) Status registers bits */

#define FMSTR_SCISR_RDRF       0x0004	//DRF
#define FMSTR_SCISR_TDRE       0x0002	//DTF


/*******************************************************************************************
* SCI access macros
*****************************************************************************************/

/* transmitter enable/disable */
#define FMSTR_SCI_TE() FMSTR_SETBIT16(FMSTR_SCI_BASE, FMSTR_UARTCR_OFFSET, FMSTR_UARTCR_TXEN)
#define FMSTR_SCI_TD() FMSTR_CLRBIT16(FMSTR_SCI_BASE, FMSTR_UARTCR_OFFSET, FMSTR_UARTCR_TXEN)

/* receiver enable/disable */
#define FMSTR_SCI_RE() FMSTR_SETBIT16(FMSTR_SCI_BASE, FMSTR_UARTCR_OFFSET, FMSTR_UARTCR_RXEN)
#define FMSTR_SCI_RD() FMSTR_CLRBIT16(FMSTR_SCI_BASE, FMSTR_UARTCR_OFFSET, FMSTR_UARTCR_RXEN)

/* Transmitter-empty interrupt enable/disable */
#define FMSTR_SCI_ETXI() FMSTR_SETBIT16(FMSTR_SCI_BASE, FMSTR_LINIER_OFFSET, FMSTR_LINIER_DTIE)
#define FMSTR_SCI_DTXI() FMSTR_CLRBIT16(FMSTR_SCI_BASE, FMSTR_LINIER_OFFSET, FMSTR_LINIER_DTIE)

/* Receiver-full interrupt enable/disable */
#define FMSTR_SCI_ERXI() FMSTR_SETBIT16(FMSTR_SCI_BASE, FMSTR_LINIER_OFFSET, FMSTR_LINIER_DRIE)
#define FMSTR_SCI_DRXI() FMSTR_CLRBIT16(FMSTR_SCI_BASE, FMSTR_LINIER_OFFSET, FMSTR_LINIER_DRIE)

/* Tranmsit character */
#define FMSTR_SCI_PUTCHAR(ch) FMSTR_SETREG8(FMSTR_SCI_BASE, FMSTR_BDRL_DATA0_OFFSET, (FMSTR_U16) (ch))

/* Get received character */
#define FMSTR_SCI_GETCHAR() ((FMSTR_U8) FMSTR_GETREG8(FMSTR_SCI_BASE, FMSTR_BDRM_DATA4_OFFSET))

/* read status register */
#define FMSTR_SCI_GETSR()   FMSTR_GETREG16(FMSTR_SCI_BASE, FMSTR_UARTSR_OFFSET)

/* read & clear status register  */  
inline FMSTR_SCISR FMSTR_SCI_RDCLRSR(void)
{
	FMSTR_SCISR sr = FMSTR_GETREG16(FMSTR_SCI_BASE, FMSTR_UARTSR_OFFSET);
	FMSTR_SETREG16(FMSTR_SCI_BASE, FMSTR_UARTSR_OFFSET, sr & (FMSTR_SCISR_RDRF|FMSTR_SCISR_TDRE));
	return sr;
}

#endif /* __FREEMASTER_MPC56xx_H */
