/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2006 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
****************************************************************************//*!
*
* @file   freemaster_scope.c
*
* @brief  FreeMASTER Oscilloscope implementation
*
* @version 1.0.1.0
* 
* @date May-16-2008
* 
*******************************************************************************/

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"

#if FMSTR_USE_SCOPE

/***********************************
*  local variables 
***********************************/

static FMSTR_U8  pcm_nScopeVarCount;        /* number of active scope variables */
static FMSTR_ADDR  pcm_pScopeVarAddr[FMSTR_MAX_SCOPE_VARS]; /* addresses of scope variables */
static FMSTR_SIZE8 pcm_pScopeVarSize[FMSTR_MAX_SCOPE_VARS]; /* sizes of scope variables */

/**************************************************************************//*!
*
* @brief    Scope Initialization
*
******************************************************************************/

void FMSTR_InitScope(void)
{   
}

/**************************************************************************//*!
*
* @brief    Handling SETUPSCOPE and SETUPSCOPE_EX command
*
* @param    pMessageIO - original command (in) and response buffer (out) 
*
* @return   As all command handlers, the return value should be the buffer 
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

FMSTR_BPTR FMSTR_SetUpScope(FMSTR_BPTR pMessageIO)
{
    FMSTR_BPTR pResponse = pMessageIO;
    FMSTR_U8 i, sz, nVarCnt;

    /* uninitialize scope */
    pcm_nScopeVarCount = 0;

    /* seek the setup data */
    pMessageIO = FMSTR_SkipInBuffer(pMessageIO, 2);
    
    /* scope variable count  */
    pMessageIO = FMSTR_ValueFromBuffer8(&nVarCnt, pMessageIO);

    /* scope variable information must fit into our buffers */
    if(!nVarCnt || nVarCnt > FMSTR_MAX_SCOPE_VARS)
        return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_INVBUFF);

    /* get all addresses and sizes */
    for(i=0; i<nVarCnt; i++)
    {
        /* variable size */
        pMessageIO = FMSTR_ValueFromBuffer8(&sz, pMessageIO);
        pcm_pScopeVarSize[i] = sz;
        
        /* variable address */
        pMessageIO = FMSTR_AddressFromBuffer(&pcm_pScopeVarAddr[i], pMessageIO);

        /* valid numeric variable sizes only */
        if(sz == 0 || sz > 8)
            return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_INVSIZE);

#if FMSTR_CFG_BUS_WIDTH > 1
        /* even sizes only */
        if(sz & 1)
            return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_INVSIZE);
#endif
        
#if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
        if(!FMSTR_CheckTsaSpace(pcm_pScopeVarAddr[i], sz, 0))
            return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_EACCESS);
#endif
        
    }

    /* activate scope */
    pcm_nScopeVarCount = nVarCnt;

    /* return just a status */
    return FMSTR_ConstToBuffer8(pResponse, FMSTR_STS_OK);
}

/**************************************************************************//*!
*
* @brief    Handling READSCOPE command
*
* @param    pMessageIO - original command (in) and response buffer (out) 
*
* @return   As all command handlers, the return value should be the buffer 
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

FMSTR_BPTR FMSTR_ReadScope(FMSTR_BPTR pMessageIO)
{
    FMSTR_U8 i;
    
    if(!pcm_nScopeVarCount)
        return FMSTR_ConstToBuffer8(pMessageIO, FMSTR_STC_NOTINIT);
    
    /* success */
    pMessageIO = FMSTR_ConstToBuffer8(pMessageIO, FMSTR_STS_OK);
    
    for (i=0; i<pcm_nScopeVarCount; i++)
        pMessageIO = FMSTR_CopyToBuffer(pMessageIO, pcm_pScopeVarAddr[i], pcm_pScopeVarSize[i]);
        
    /* return end position */
    return pMessageIO;  
}

#endif /* FMSTR_USE_SCOPE */

