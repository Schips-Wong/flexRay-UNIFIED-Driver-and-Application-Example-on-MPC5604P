/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2006 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
****************************************************************************//*!
*
* @file   freemaster_tsa.c
*
* @brief  FreeMASTER TSA implementation
*
* @version 1.0.1.0
* 
* @date May-16-2008
* 
*******************************************************************************
*
* This file implements a new FreeMASTER feature called Targer-side address
* translation.
* 
*******************************************************************************/

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"

#if FMSTR_USE_TSA

/**************************************************************************//*!
*
* @brief    TSA Initialization
*
******************************************************************************/

void FMSTR_InitTsa(void)
{   
}

/**************************************************************************//*!
*
* @brief    Handling GETTSAINFO and GETTSAINFO_EX command
*
* @param    pMessageIO - original command (in) and response buffer (out) 
*
* @return   As all command handlers, the return value should be the buffer 
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

FMSTR_BPTR FMSTR_GetTsaInfo(FMSTR_BPTR pMessageIO)
{
    FMSTR_BPTR pResponse = pMessageIO;
    const FMSTR_TSA_ENTRY* pTbl;
    FMSTR_TSA_TINDEX nTblIndex;
    FMSTR_TSA_TSIZE nTblSize = 0;
    FMSTR_ADDR tmpAddr;
    FMSTR_U16 tmp16;

    /* get index of table the PC is requesting   */
    pMessageIO = FMSTR_SkipInBuffer(pMessageIO, 2);
    pMessageIO = FMSTR_ValueFromBuffer16(&nTblIndex, pMessageIO);
    
    /* TSA flags */
    tmp16 = FMSTR_TSA_VERSION | FMSTR_TSA_FLAGS;

    /* sizeof TSA table entry items */
    if((sizeof(FMSTR_TSA_ENTRY)/4) == 4)
        tmp16 |= FMSTR_TSA_INFO_32BIT;  /* 32bit TSA entries */
    
    /* get the table (or NULL if no table on given index) */
    pTbl = FMSTR_TsaGetTable(nTblIndex, &nTblSize);

    /* success, flags */
    pResponse = FMSTR_ConstToBuffer8(pResponse, FMSTR_STS_OK);
    pResponse = FMSTR_ValueToBuffer16(pResponse, tmp16);

    /* table size in bytes */
    nTblSize *= FMSTR_CFG_BUS_WIDTH;
    pResponse = FMSTR_ValueToBuffer16(pResponse, nTblSize);
    
    /* table address */
    FMSTR_PTR2ADDR(tmpAddr, pTbl);
    return FMSTR_AddressToBuffer(pResponse, tmpAddr);
}

/**************************************************************************//*!
*
* @brief    Private inline implementation of "strlen"
*
******************************************************************************/

FMSTR_U16 FMSTR_StrLen(FMSTR_ADDR nAddr)
{
    const char* pStr;
    FMSTR_U16 nLen = 0;

    #ifdef __HCS12X__
    /* convert from logical to global if needed */
    nAddr = FMSTR_FixHcs12xAddr(nAddr);
    #endif

    pStr = (const char*) nAddr;
    
    while(*pStr++)
        nLen++;
        
    return nLen;
}

/**************************************************************************//*!
*
* @brief    Handling GETSTRLEN and GETSTRLEN_EX commands
*
* @param    pMessageIO - original command (in) and response buffer (out) 
*
* @return   As all command handlers, the return value should be the buffer 
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

FMSTR_BPTR FMSTR_GetStringLen(FMSTR_BPTR pMessageIO)
{
    FMSTR_BPTR pResponse = pMessageIO;
    FMSTR_ADDR nAddr;
    FMSTR_U16 nLen = 0;
    
    pMessageIO = FMSTR_SkipInBuffer(pMessageIO, 1);
    pMessageIO = FMSTR_AddressFromBuffer(&nAddr, pMessageIO);

    nLen = FMSTR_StrLen(nAddr);

    /* return strign size in bytes (even on 16bit DSP) */
    nLen *= FMSTR_CFG_BUS_WIDTH ;
        
    pResponse = FMSTR_ConstToBuffer8(pResponse, FMSTR_STS_OK);
    return FMSTR_ValueToBuffer16(pResponse, nLen);
}

/**************************************************************************//*!
*
* @brief    Helper (inline) function for TSA memory region check
*
* @param    nAddrUser - address of region to be checked
* @param    nSizeUser - size of region to be checked
* @param    nAddrSafe - address of known "safe" region
* @param    wSizeSafe - size of safe region
*
* @return   This function returns non-zero if given user space is safe 
*           (i.e. it lies in given safe space)
*
******************************************************************************/

#if defined(FMSTR_PLATFORM_56F8xxx) || defined(FMSTR_PLATFORM_56F8xx)
/* make inline */
inline
#else
/* declare function prototype */
FMSTR_BOOL FMSTR_CheckMemSpace(FMSTR_ADDR nAddrUser, FMSTR_SIZE8 nSizeUser, 
	FMSTR_ADDR nAddrSafe, FMSTR_SIZE wSizeSafe);
#endif

FMSTR_BOOL FMSTR_CheckMemSpace(FMSTR_ADDR nAddrUser, FMSTR_SIZE8 nSizeUser, 
                               FMSTR_ADDR nAddrSafe, FMSTR_SIZE wSizeSafe)
{
    #ifdef __HCS12X__
    /* convert from logical to global if needed */
    nAddrUser = FMSTR_FixHcs12xAddr(nAddrUser);
    nAddrSafe = FMSTR_FixHcs12xAddr(nAddrSafe);
    #endif

    if(nAddrUser < nAddrSafe)
        return 0;
    
    return (nAddrUser + nSizeUser) <= (nAddrSafe + wSizeSafe);
}

/**************************************************************************//*!
*
* @brief    Check wether given memory region is "safe" (covered by TSA)
*
* @param    dwAddr - address of the memory to be checked
* @param    nSize  - size of the memory to be checked
* @param    bWriteAccess - write access is required
*
* @return   This function returns non-zero if user space is safe
*
******************************************************************************/

FMSTR_BOOL FMSTR_CheckTsaSpace(FMSTR_ADDR dwAddr, FMSTR_SIZE8 nSize, FMSTR_BOOL bWriteAccess)
{
    const FMSTR_TSA_ENTRY* pte;
    FMSTR_TSA_TINDEX nTableIndex;
    FMSTR_TSA_TSIZE i, cnt;
    FMSTR_SIZE nInfo;

#if FMSTR_CFG_BUS_WIDTH >= 2
    /* TSA tables use sizeof() operator which returns size in "bus-widths" (e.g. 56F8xx) */
    nSize = (nSize + 1) / FMSTR_CFG_BUS_WIDTH;
#endif
    
    /* to be as fast as possible during normal opearaion, 
       check variable entries in all tables first */
    for(nTableIndex=0; (pte=FMSTR_TsaGetTable(nTableIndex, &cnt)) != NULL; nTableIndex++)
    {
        /* number of items in a table */
        cnt /= sizeof(FMSTR_TSA_ENTRY);

        /* all table entries */
        for(i=0; i<cnt; i++, pte++)
        {
            nInfo = (FMSTR_SIZE) pte->info.p;
            
            /* variable entry only (also check read-write flag) */
            if((nInfo & FMSTR_TSA_INFO_VAR_FLAG) && (!bWriteAccess || (nInfo & FMSTR_TSA_INFO_RWV_FLAG)))
            {
                /* need to take the larger of the two in union (will be optimized by compiler anyway) */
                if(sizeof(pte->addr.p) < sizeof(pte->addr.n))
                {
                    if(FMSTR_CheckMemSpace(dwAddr, nSize, (FMSTR_ADDR) pte->addr.n, (FMSTR_SIZE) (nInfo >> 2)))
                        return 1; /* access granted! */
                }
                else
                {
                    if(FMSTR_CheckMemSpace(dwAddr, nSize, (FMSTR_ADDR) pte->addr.p, (FMSTR_SIZE) (nInfo >> 2)))
                        return 1; /* access granted! */
                }
            }
        }
    }

    /* no more writeable memory chunks available */
    if(bWriteAccess)
        return 0;

    /* allow reading of recorder buffer */
#if FMSTR_USE_RECORDER
    if(FMSTR_IsInRecBuffer(dwAddr, nSize))
        return 1;
#endif

    /* allow reading of any C-constant string referenced in TSA tables */
    for(nTableIndex=0; (pte=FMSTR_TsaGetTable(nTableIndex, &cnt)) != NULL; nTableIndex++)
    {
        /* allow reading of the TSA table itself */
        if(FMSTR_CheckMemSpace(dwAddr, nSize, (FMSTR_ADDR)(FMSTR_U8*)pte, cnt))
            return 1;
        
        /* number of items in a table */
        cnt /= sizeof(FMSTR_TSA_ENTRY);

        /* all table entries */
        for(i=0; i<cnt; i++, pte++)
        {
            /* system strings are always accessible at C-pointers */
            if(pte->name.p && FMSTR_CheckMemSpace(dwAddr, nSize, (FMSTR_ADDR) pte->name.p, FMSTR_StrLen((FMSTR_ADDR) pte->name.p)))
                return 1;
            if(pte->type.p && FMSTR_CheckMemSpace(dwAddr, nSize, (FMSTR_ADDR) pte->type.p, FMSTR_StrLen((FMSTR_ADDR) pte->type.p)))
                return 1;
        }
    }

    /* no valid TSA entry found => not-safe to access the memory */
    return 0;
} 

#endif /* FMSTR_USE_TSA */

