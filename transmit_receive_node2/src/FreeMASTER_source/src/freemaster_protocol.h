/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2006 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
****************************************************************************//*!
*
* @file   freemaster_protocol.h
*
* @brief  FreeMASTER protocol header file
*
* @version 1.0.1.0
* 
* @date May-16-2008
* 
*******************************************************************************/

#ifndef __FREEMASTER_PROTOCOL_H
#define __FREEMASTER_PROTOCOL_H

/*-------------------------------------
  command message - standard commands
  -------------------------------------*/
  
#define FMSTR_CMD_READMEM           0x01
#define FMSTR_CMD_WRITEMEM          0x02
#define FMSTR_CMD_WRITEMEMMASK      0x03
#define FMSTR_CMD_READMEM_EX        0x04    /* read a block of memory */
#define FMSTR_CMD_WRITEMEM_EX       0x05    /* write a block of memory */   
#define FMSTR_CMD_WRITEMEMMASK_EX   0x06    /* write block of memory with bit mask */   
#define FMSTR_CMD_SETUPSCOPE        0x08
#define FMSTR_CMD_SETUPREC          0x09
#define FMSTR_CMD_SETUPSCOPE_EX     0x0a    /* setup the osciloscope */ 
#define FMSTR_CMD_SETUPREC_EX       0x0b    /* setup the recorder */     
#define FMSTR_CMD_SENDAPPCMD        0x10    /* send the application command */  
#define FMSTR_CMD_GETTSAINFO        0x11    /* get TSA info */
#define FMSTR_CMD_GETTSAINFO_EX     0x12    /* get TSA info 32bit */

/*-------------------------------------
  command message - Fast Commands
  -------------------------------------*/
  
/* no data part */
#define FMSTR_CMD_GETINFO           0xc0    /* retrieve board information structure */  
#define FMSTR_CMD_STARTREC          0xc1    /* start data recorder */   
#define FMSTR_CMD_STOPREC           0xc2    /* stop data recorder */    
#define FMSTR_CMD_GETRECSTS         0xc3    /* get the recorder status */   
#define FMSTR_CMD_GETRECBUFF        0xc4
#define FMSTR_CMD_READSCOPE         0xc5    /* read the scope data */   
#define FMSTR_CMD_GETAPPCMDSTS      0xc6    /* get the application command status */    
#define FMSTR_CMD_GETINFOBRIEF      0xc8    /* retrieve a subset of board information structure */  
#define FMSTR_CMD_GETRECBUFF_EX     0xc9    /* get the recorder data */ 

/* 2 bytes data part */
#define FMSTR_CMD_READVAR8          0xD0
#define FMSTR_CMD_READVAR16         0xD1
#define FMSTR_CMD_READVAR32         0xD2
#define FMSTR_CMD_GETAPPCMDDATA     0xD3    /* get the application command data */
#define FMSTR_CMD_GETSTRLEN         0xD4    /* get string length (required by TSA) */

/* 4 bytes data part */
#define FMSTR_CMD_READVAR8_EX       0xe0    /* read byte variable */    
#define FMSTR_CMD_READVAR16_EX      0xe1    /* read word variable */    
#define FMSTR_CMD_READVAR32_EX      0xe2    /* read dword variable */   
#define FMSTR_CMD_WRITEVAR8         0xe3    /* write byte variable */   
#define FMSTR_CMD_WRITEVAR16        0xe4    /* write word variable */   
#define FMSTR_CMD_WRITEVAR8MASK     0xe5    /* write specified bits in byte variable  */    
#define FMSTR_CMD_GETSTRLEN_EX      0xe6    /* get string length (required by TSA) */

/* 6 bytes data part */
#define FMSTR_CMD_WRITEVAR32        0xf0    /* write dword variable */  
#define FMSTR_CMD_WRITEVAR16MASK    0xf1    /* write specified bits in word variable */ 

/*-------------------------------------
  response message - status byte
  -------------------------------------*/
  
/* confirmation codes */
#define FMSTR_STS_OK                0x00    /* operation finished successfuly */    
#define FMSTR_STS_RECRUN            0x01    /* data recorder is running */  
#define FMSTR_STS_RECDONE           0x02    /* data recorder is stopped */  

/* error codes */
#define FMSTR_STC_INVCMD            0x81    /* unknown command code */  
#define FMSTR_STC_CMDCSERR          0x82    /* command checksum error */    
#define FMSTR_STC_CMDTOOLONG        0x83    /* comand is too long */    
#define FMSTR_STC_RSPBUFFOVF        0x84    /* the response would not fit into transmit buffer */   
#define FMSTR_STC_INVBUFF           0x85    /* invalid buffer length or operation */    
#define FMSTR_STC_INVSIZE           0x86    /* invalid size specified */    
#define FMSTR_STC_SERVBUSY          0x87    /* service is busy */   
#define FMSTR_STC_NOTINIT           0x88    /* service is not initialised */    
#define FMSTR_STC_EACCESS           0x89    /* access is denied */  

/******************************************************************************
* Protocol constants 
*******************************************************************************/

#define FMSTR_SOB                   0x2b    /* '+' - start of message*/
#define FMSTR_FASTCMD               0xc0    /* code of fast cmd 0xc0> */
#define FMSTR_FASTCMD_DATALEN_MASK  0x30    /* mask of data length part of fast command */
#define FMSTR_FASTCMD_DATALEN_SHIFT 3
#define FMSTR_DESCR_SIZE            25      /* length board desription string */

/* Board configuration flags  */
#define FMSTR_CFGFLAG_BIGENDIAN       0x01      /*/< used when CPU is big endian */

/* TSA-global flags  */
#define FMSTR_TSA_INFO_VERSION_MASK   0x000f    /*/< TSA version  */
#define FMSTR_TSA_INFO_32BIT          0x0100    /*/< TSA address format (16/32 bit) */
#define FMSTR_TSA_INFO_HV2BA          0x0200    /*/< TSA HawkV2 byte-addressing mode */

#endif /* __FREEMASTER_PROTOCOL_H */


