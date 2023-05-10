/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2006 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
****************************************************************************//*!
*
* @file   freemaster_appcmd.c
*
* @brief  FreeMASTER Application Commands implementation
*
* @version 1.0.1.0
* 
* @date May-16-2008
* 
*******************************************************************************/

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"

#if FMSTR_USE_APPCMD

/***********************************
*  local variables 
***********************************/

static FMSTR_APPCMD_CODE pcm_nAppCmd;                               /* app.cmd code (to application) */
static FMSTR_APPCMD_DATA pcm_pAppCmdBuff[FMSTR_APPCMD_BUFF_SIZE];   /* app.cmd data buffer  */
static FMSTR_SIZE        pcm_nAppCmdLen;                            /* app.cmd data length */

static FMSTR_APPCMD_RESULT  pcm_nAppCmdResult;                      /* app.cmd result code (from application) */
static FMSTR_SIZE pcm_nAppCmdResultDataLen;

#if FMSTR_MAX_APPCMD_CALLS > 0
static FMSTR_APPCMD_CODE pcm_pAppCmdCallId[FMSTR_MAX_APPCMD_CALLS]; /* registerred callback commands */
static FMSTR_PAPPCMDFUNC pcm_pAppCmdCallFunc[FMSTR_MAX_APPCMD_CALLS];  /* registerred callback handlers */
#endif

/***********************************
*  local functions
***********************************/

static FMSTR_INDEX FMSTR_FindAppCmdCallIndex(FMSTR_APPCMD_CODE nAppcmdCode);
static FMSTR_APPCMD_RESULT FMSTR_SfioHandler(FMSTR_APPCMD_CODE nAppcmdCode, 
    FMSTR_APPCMD_PDATA pData, FMSTR_SIZE nDataLen);

/**************************************************************************//*!
*
* @brief    Initialize app.cmds interface
*
******************************************************************************/

void FMSTR_InitAppCmds(void)
{
    FMSTR_INDEX i;
    
    pcm_nAppCmd = FMSTR_APPCMDRESULT_NOCMD;
    pcm_nAppCmdResult = FMSTR_APPCMDRESULT_NOCMD;
    
#if FMSTR_MAX_APPCMD_CALLS
    for(i=0; i<FMSTR_MAX_APPCMD_CALLS; i++)
    {
        pcm_pAppCmdCallId[i] = FMSTR_APPCMDRESULT_NOCMD;
        pcm_pAppCmdCallFunc[i] = NULL;
    }
#endif      

#if FMSTR_USE_SFIO
    FMSTR_RegisterAppCmdCall(FMSTR_SFIO_APPCMD, FMSTR_SfioHandler);

#endif
}

/**************************************************************************//*!
*
* @brief    Handling SANDAPPCMD command
*
* @param    pMessageIO - original command (in) and response buffer (out) 
*
* @return   As all command handlers, the return value should be the buffer 
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

FMSTR_BPTR FMSTR_StoreAppCmd(FMSTR_BPTR pMessageIO)
{
    FMSTR_BPTR pResponse = pMessageIO;
    FMSTR_U8 nArgsLen;
    FMSTR_U8 nCode;

    /* the previous command not yet processed */
    if(pcm_nAppCmd != FMSTR_APPCMDRESULT_NOCMD)
        return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_SERVBUSY);

    pMessageIO = FMSTR_SkipInBuffer(pMessageIO, 1);
    pMessageIO = FMSTR_ValueFromBuffer8(&nArgsLen, pMessageIO);
    pMessageIO = FMSTR_ValueFromBuffer8(&nCode, pMessageIO);

    /* args len is datalen minus one */
    nArgsLen--;
        
    /* does the application command fit to buffer ? */
    if (nArgsLen > FMSTR_APPCMD_BUFF_SIZE)
        return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_INVBUFF);
    
    /* store command data into dedicated buffer */
    pcm_nAppCmd = nCode;
    pcm_nAppCmdLen = nArgsLen;
    
    /* data copy */
    if(nArgsLen)
        FMSTR_CopyFromBuffer((FMSTR_ADDR)pcm_pAppCmdBuff, pMessageIO, nArgsLen);

    /* mark command as "running" (without any response data) */
    pcm_nAppCmdResult = FMSTR_APPCMDRESULT_RUNNING;
    pcm_nAppCmdResultDataLen = 0;

    return FMSTR_ConstToBuffer8(pResponse, FMSTR_STS_OK);
}

/**************************************************************************//*!
*
* @brief    Handling GETAPPCMDSTS command
*
* @param    pMessageIO - original command (in) and response buffer (out) 
*
* @return   As all command handlers, the return value should be the buffer 
*           pointer where the response output finished (except checksum)
*
* @note The callback-registerred commands are processed at the moment the PC 
*       tries to get the result for the first time. At this moment, we are
*       sure the PC already got the command delivery packet acknowledged.
*
******************************************************************************/

FMSTR_BPTR FMSTR_GetAppCmdStatus(FMSTR_BPTR pMessageIO)
{
#if FMSTR_MAX_APPCMD_CALLS
    FMSTR_PAPPCMDFUNC pFunc = NULL;
    FMSTR_INDEX nIndex;
    
    /* time to execute the command's callback */
    if((nIndex = FMSTR_FindAppCmdCallIndex(pcm_nAppCmd)) >= 0)
        pFunc = pcm_pAppCmdCallFunc[nIndex];

    /* valid callback function found? */
    if(pFunc)
    {
        /* do execute callback, return value is app.cmd result code */
        pcm_nAppCmdResult = pFunc(pcm_nAppCmd, pcm_pAppCmdBuff, pcm_nAppCmdLen);

        /* nothing more to do with this command (i.e. command acknowledged) */
        pcm_nAppCmd = FMSTR_APPCMDRESULT_NOCMD;
    }
#endif

    pMessageIO = FMSTR_ConstToBuffer8(pMessageIO, FMSTR_STS_OK);
    return FMSTR_ValueToBuffer8(pMessageIO, pcm_nAppCmdResult);
}

/**************************************************************************//*!
*
* @brief    Handling GETAPPCMDDATA command
*
* @param    pMessageIO - original command (in) and response buffer (out) 
*
* @return   As all command handlers, the return value should be the buffer 
*           pointer where the response output finished (except checksum)
*
******************************************************************************/

FMSTR_BPTR FMSTR_GetAppCmdRespData(FMSTR_BPTR pMessageIO)
{
    FMSTR_BPTR pResponse = pMessageIO;
    FMSTR_U8 nDataLen;
    FMSTR_U8 nDataOffset;

    /* the previous command not yet processed */
    if(pcm_nAppCmd != FMSTR_APPCMDRESULT_NOCMD)
        return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_SERVBUSY);

    pMessageIO = FMSTR_SkipInBuffer(pMessageIO, 1);
    pMessageIO = FMSTR_ValueFromBuffer8(&nDataLen, pMessageIO);
    pMessageIO = FMSTR_ValueFromBuffer8(&nDataOffset, pMessageIO);

    /* the response would not fit into comm buffer */
    if(nDataLen > FMSTR_COMM_BUFFER_SIZE)
        return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_RSPBUFFOVF);
    
    /* the data would be fetched outside the app.cmd response data */
    if((nDataOffset + nDataLen) > pcm_nAppCmdResultDataLen)
        return FMSTR_ConstToBuffer8(pResponse, FMSTR_STC_INVSIZE);
    
    pResponse = FMSTR_ConstToBuffer8(pResponse, FMSTR_STS_OK);
    return FMSTR_CopyToBuffer(pResponse, (FMSTR_ADDR) pcm_pAppCmdBuff, nDataLen);
}

/**************************************************************************//*!
*
* @brief    Find index of registerred app.cmd callback
*
* @param    nAppcmdCode - App. command ID 
*
* @return   Index of function pointer in our local tables
*
******************************************************************************/

static FMSTR_INDEX FMSTR_FindAppCmdCallIndex(FMSTR_APPCMD_CODE nAppcmdCode)
{
    FMSTR_INDEX i;
    
#if FMSTR_MAX_APPCMD_CALLS > 0
    for(i=0; i<FMSTR_MAX_APPCMD_CALLS; i++)
        if(pcm_pAppCmdCallId[i] == nAppcmdCode)
            return i;
#endif
        
    return -1;
}

/**************************************************************************//*!
*
* @brief    API: Mark the application command is processed by the application
*
* @param    nResultCode - the result code which is returned to a host
*
******************************************************************************/

void FMSTR_AppCmdAck(FMSTR_APPCMD_RESULT nResultCode)
{
    pcm_nAppCmdResult = nResultCode;
    pcm_nAppCmdResultDataLen = 0;
    
    /* waiting for a new command to come */
    pcm_nAppCmd = FMSTR_APPCMDRESULT_NOCMD;
}

/**************************************************************************//*!
*
* @brief    API: Mark the application command is processed by the application
*
* @param    pResultDataAddr - address of data we want to return to the PC
* @param    nResultDataLen - length of return data
*
******************************************************************************/

void FMSTR_AppCmdSetResponseData(FMSTR_ADDR nResultDataAddr, FMSTR_SIZE nResultDataLen)
{
    /* any data supplied by user? */
    if(nResultDataAddr)
    {
        /* response data length is trimmed if response data would not fit into buffer */
        pcm_nAppCmdResultDataLen = nResultDataLen;
        if(pcm_nAppCmdResultDataLen > FMSTR_APPCMD_BUFF_SIZE)
            pcm_nAppCmdResultDataLen = FMSTR_APPCMD_BUFF_SIZE;

        if(pcm_nAppCmdResultDataLen > 0)        
            FMSTR_CopyMemory((FMSTR_ADDR) pcm_pAppCmdBuff, nResultDataAddr, (FMSTR_SIZE8) pcm_nAppCmdResultDataLen);
    }
    else
    {
        /* no data being returned at all (same effect as pure FMSTR_AppCmdAck) */
        pcm_nAppCmdResultDataLen = 0;
    }
}

/**************************************************************************//*!
*
* @brief    API: Fetch the application command code if one is ready for processing
*
* @return   A command code stored in the application cmd buffer.
*           The return value is FMSTR_APPCMDRESULT_NOCMD if there is no
*           new command since the last call to FMSTR_AppCmdAck
*
******************************************************************************/

FMSTR_APPCMD_CODE FMSTR_GetAppCmd(void)
{
#if FMSTR_MAX_APPCMD_CALLS
    /* the user can never see the callback-registerred commands */
    if(FMSTR_FindAppCmdCallIndex(pcm_nAppCmd) >= 0)
        return FMSTR_APPCMDRESULT_NOCMD;
#endif

    /* otherwise, return the appcomand pending */
    return pcm_nAppCmd;     
}
    
/**************************************************************************//*!
*
* @brief    API: Get a pointer to application command data
*
* @param    pDataLen - A pointer to variable which receives the data length
*
* @return   Pointer to the "application command" data 
*
******************************************************************************/

FMSTR_APPCMD_PDATA FMSTR_GetAppCmdData(FMSTR_SIZE* pDataLen)
{
    /* no command, no data */
    if(pcm_nAppCmd == FMSTR_APPCMDRESULT_NOCMD)
        return NULL;
    
#if FMSTR_MAX_APPCMD_CALLS
    /* the user never sees the callback-registerred commands */
    if(FMSTR_FindAppCmdCallIndex(pcm_nAppCmd) >= 0)
        return NULL;
#endif

    /* caller want to know the data length */
    if(pDataLen)
        *pDataLen = pcm_nAppCmdLen;
    
    /* data are saved in out buffer */
    return pcm_nAppCmdLen ? pcm_pAppCmdBuff : (FMSTR_APPCMD_PDATA) NULL;
}

/**************************************************************************//*!
*
* @brief    API: Register or unregister app.cmd callback handler
*
* @param    nAppCmdCode   - App.command ID
* @param    pCallbackFunc - Pointer to handler function (NULL to unregister)
*
* @return   Non-zero if successfull, zero if maximum callbacks already set
*
******************************************************************************/

FMSTR_BOOL FMSTR_RegisterAppCmdCall(FMSTR_APPCMD_CODE nAppCmdCode, FMSTR_PAPPCMDFUNC pCallbackFunc)
{
#if FMSTR_MAX_APPCMD_CALLS > 0

    FMSTR_INDEX nIndex;

    /* keep "void" ID as reserved */
    if(nAppCmdCode == FMSTR_APPCMDRESULT_NOCMD)
        return 0;
    
    /* get index of app.cmd ID (if already set) */
    nIndex = FMSTR_FindAppCmdCallIndex(nAppCmdCode);
    
    /* when not found, get a free slot (only if registerring new callback) */
    if(nIndex < 0 && pCallbackFunc != NULL)
        nIndex = FMSTR_FindAppCmdCallIndex(FMSTR_APPCMDRESULT_NOCMD);
    
    /* failed? */
    if(nIndex < 0)
        return 0;
    
    /* register handler */
    pcm_pAppCmdCallFunc[nIndex] = pCallbackFunc;
    pcm_pAppCmdCallId[nIndex] = (FMSTR_APPCMD_CODE) (pCallbackFunc ? 
        nAppCmdCode : FMSTR_APPCMDRESULT_NOCMD);
    return 1;
    
#else
    /* app.cmd callback not implemented */
    return 0;
        
#endif  
}


/**************************************************************************//*!
*
* @brief  Executing the SFIO command
*
* @param  nAppcmdCode - App.cmd used for SFIO encapsulation
* @param  pData       - App.cmd data (SFIO packet to be delivered to SFIO layer)
* @param  nDataLen    - App.cmd data length
*
* @return As all app.cmd-registerred callbacks, this function returns the
*         result code to be passed up to the PC.
*
* @note  This function handles the SFIO (Matlab/Simulink Interface) command
*        encapsulated into FreeMASTER protocol. It emulates the SFIO serial
*        char-by-char communication.
*
******************************************************************************/

#if FMSTR_USE_SFIO

static FMSTR_APPCMD_RESULT FMSTR_SfioHandler(FMSTR_APPCMD_CODE nAppcmdCode, FMSTR_APPCMD_PDATA pData, FMSTR_SIZE nDataLen)
{
    FMSTR_APPCMD_RESULT nRet = 0;
    FMSTR_U8* pSfioResponse = NULL;
    FMSTR_U8* pCmdData = (FMSTR_U8*) pData;
    FMSTR_U8 nChar;
    FMSTR_INDEX i;
    
    for(i=0; !nRet && i<nDataLen; i++)
    {
        nChar = *pCmdData++;
        nRet = (FMSTR_APPCMD_RESULT) SFIO_ProcessRxChar(nChar, &pSfioResponse);

        /* replicate SFIO special SOB character (not at the beginning!) */
        if(!nRet && i && nChar == 0xff)
            nRet = SFIO_ProcessRxChar(nChar, &pSfioResponse);
    }
    
    if(pSfioResponse && nRet > 0 && nRet < 256)
        FMSTR_AppCmdSetResponseData((FMSTR_ADDR) pSfioResponse, nRet);
    else
        nRet = 0;
    
    return nRet;
}

#endif

#else /* FMSTR_USE_APPCMD */

/* void Application Command API functions */

void FMSTR_AppCmdAck(FMSTR_APPCMD_RESULT nResultCode) 
{ 
	FMSTR_UNUSED(nResultCode);
}

void FMSTR_AppCmdSetResponseData(FMSTR_ADDR pResultData, FMSTR_SIZE nResultDataLen)
{
	FMSTR_UNUSED(pResultData);
	FMSTR_UNUSED(nResultDataLen);
}

FMSTR_APPCMD_CODE FMSTR_GetAppCmd(void) 
{ 
    return FMSTR_APPCMDRESULT_NOCMD; 
}

FMSTR_APPCMD_PDATA FMSTR_GetAppCmdData(FMSTR_SIZE* pDataLen) 
{ 
	FMSTR_UNUSED(pDataLen);
    return NULL; 
}

FMSTR_BOOL FMSTR_RegisterAppCmdCall(FMSTR_APPCMD_CODE nAppCmdCode, FMSTR_PAPPCMDFUNC pCallbackFunc) 
{ 
	FMSTR_UNUSED(nAppCmdCode);
	FMSTR_UNUSED(pCallbackFunc);
    return 0; 
}

#endif /* FMSTR_USE_APPCMD */
