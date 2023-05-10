/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2006 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
****************************************************************************//*!
*
* @file   freemaster_serial.c
*
* @brief  FreeMASTER SCI communication routines
*
* @version 1.0.1.0
* 
* @date May-16-2008
* 
*******************************************************************************/

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"

#if FMSTR_USE_SCI || FMSTR_USE_JTAG

/***********************************
*  local variables 
***********************************/

/* FreeMASTER communication buffer (in/out) plus the STS and LEN bytes */
static FMSTR_BCHR pcm_pCommBuffer[FMSTR_COMM_BUFFER_SIZE+2];    

/* FreeMASTER runtime flags */
static volatile union 
{
    FMSTR_FLAGS all;
    
    struct 
    {
        unsigned bTxActive : 1;         /* response is being transmitted */
        unsigned bTxWaitTC : 1;         /* response sent, wait for transmission complete */
        unsigned bTxLastCharSOB : 1;    /* last transmitted char was equal to SOB  */
        unsigned bRxLastCharSOB : 1;    /* last received character was SOB */
        unsigned bRxMsgLengthNext : 1;  /* expect the length byte next time */
    } flg;
    
} pcm_wFlags;

/* receive and transmit buffers and counters */
static FMSTR_SIZE8 pcm_nTxTodo;     /* transmission to-do counter (0 when tx is idle) */
static FMSTR_SIZE8 pcm_nRxTodo;     /* reception to-do counter (0 when rx is idle) */
static FMSTR_BPTR  pcm_pTxBuff;     /* pointer to next byte to transmit */
static FMSTR_BPTR  pcm_pRxBuff;     /* pointer to next free place in RX buffer */
static FMSTR_BCHR  pcm_nRxCheckSum; /* checksum of data being received */

/* SHORT_INTR receive queue (circular buffer) */
#if FMSTR_SHORT_INTR
static FMSTR_BCHR  pcm_pRQueueBuffer[FMSTR_COMM_RQUEUE_SIZE];
static FMSTR_BPTR  pcm_pRQueueRP;   /* SHORT_INTR queue read-pointer */
static FMSTR_BPTR  pcm_pRQueueWP;   /* SHORT_INTR queue write-pointer */
#endif

#if FMSTR_USE_JTAG
static FMSTR_U32 pcm_wJtagTxData;   /* four bytes buffer to be sent over JTAG (LSB first) */
static FMSTR_SIZE8 pcm_wJtagTxCtr;  /* counter of bytes in pcm_wJtagTxData */
#endif

/***********************************
*  local function prototypes
***********************************/

static void FMSTR_Listen(void);
static void FMSTR_SendError(FMSTR_BCHR nErrCode);
static void FMSTR_Tx(void);
static void FMSTR_Rx(FMSTR_BCHR nRxChar);
static void FMSTR_RxQueue(FMSTR_BCHR nRxChar);
static void FMSTR_RxDequeue(void);

/**************************************************************************//*!
*
* @brief    Serial communication initialization
*
******************************************************************************/

void FMSTR_InitSerial(void)
{   
    /* initialize all state variables */
    pcm_wFlags.all = 0;
    pcm_nTxTodo = 0;
    pcm_nRxTodo = 0;
    
#if FMSTR_SHORT_INTR
    pcm_pRQueueRP = pcm_pRQueueBuffer;
    pcm_pRQueueWP = pcm_pRQueueBuffer;
#endif

    /* start listening for commands */
    FMSTR_Listen();
}

/**************************************************************************//*!
*
* @brief    Start listening on a serial line
*
* Reset the receiver machine and start listening on a serial line
*
******************************************************************************/

static void FMSTR_Listen(void)
{
    pcm_nRxTodo = 0;

	/* disable transmitter state machine */
   	pcm_wFlags.flg.bTxActive = 0;
   	pcm_wFlags.flg.bTxWaitTC = 0;

    /* disable transmitter, enable receiver (enables single-wire connection) */
#if FMSTR_USE_SCI
    FMSTR_SCI_TD();
    FMSTR_SCI_RE();
#endif

    /* disable transmit, enable receive interrupts */
#if FMSTR_SHORT_INTR || FMSTR_LONG_INTR
#if FMSTR_USE_SCI
    FMSTR_SCI_DTXI();   /* disable SCI transmit interrupt */
    FMSTR_SCI_ERXI();   /* enable SCI recieve interrupt */
    
#elif FMSTR_USE_JTAG
    FMSTR_JTAG_DTXI();  /* disable JTAG transmit interrupt  */
    FMSTR_JTAG_ERXI();  /* enable JTAG recieve interrupt  */
        
#endif
#endif  
}

/**************************************************************************//*!
*
* @brief    Send response of given error code (no data) 
*
* @param    nErrCode - error code to be sent
*
******************************************************************************/

static void FMSTR_SendError(FMSTR_BCHR nErrCode)
{
    /* fill & send single-byte response */
    *pcm_pCommBuffer = nErrCode;
    FMSTR_SendResponse(pcm_pCommBuffer, 1);
}

/**************************************************************************//*!
*
* @brief    Finalize transmit buffer before transmitting 
*
* @param    nLength - response length (1 for status + data length)
*
*
* This Function takes the data already prepared in the transmit buffer 
* (inlcuding the status byte). It computes the check sum and kicks on tx.
*
******************************************************************************/

void FMSTR_SendResponse(FMSTR_BPTR pResponse, FMSTR_SIZE8 nLength)
{
    FMSTR_U16 chSum = 0;
    FMSTR_U8 i, c;

    /* remeber the buffer to be sent */
    pcm_pTxBuff = pResponse;
    
    /* status byte and data are already there, compute checksum only     */
    for (i=0; i<nLength; i++)
    {
    	c = 0;
    	pResponse = FMSTR_ValueFromBuffer8(&c, pResponse);
    	/* add character to checksum */
        chSum += c;
        /* prevent saturation to happen on DSP platforms */
        chSum &= 0xff;
    }
    
    /* store checksum after the message */
    pResponse = FMSTR_ValueToBuffer8(pResponse, (FMSTR_U8) ((~chSum) + 1));

    /* send the message and the checksum and the SOB */
    pcm_nTxTodo = (FMSTR_SIZE8) (nLength + 1); 
    
    /* now transmitting the response */
    pcm_wFlags.flg.bTxActive = 1;
    pcm_wFlags.flg.bTxWaitTC = 0;

    /* do not replicate the initial SOB  */
    pcm_wFlags.flg.bTxLastCharSOB = 0;
    
#if FMSTR_USE_SCI       
    {
        volatile FMSTR_SCISR dummySR;

        /* disable receiver, enable transmitter (single-wire communication) */
        FMSTR_SCI_RD();
        FMSTR_SCI_TE();
        
        /* kick on the SCI transmission (also clears TX Empty flag on some platforms) */
        dummySR = FMSTR_SCI_GETSR();
        FMSTR_SCI_PUTCHAR(FMSTR_SOB);
    }
    
#elif FMSTR_USE_JTAG
    /* kick on the JTAG transmission */
    pcm_wJtagTxData = FMSTR_SOB;
    pcm_wJtagTxCtr = 1;
    
    /* send the next two bytes immediatelly (we can be sure there are two bytes) */
    FMSTR_Tx();
    FMSTR_Tx();
    
    /* send the third byte (if any) or flush the 32bit JTAG word */
    FMSTR_Tx();
        
#endif

    /* TX interrupt enable, RX interrupt disable */
#if FMSTR_LONG_INTR || FMSTR_SHORT_INTR
#if FMSTR_USE_SCI       
    FMSTR_SCI_DRXI();
    FMSTR_SCI_ETXI();
    
#elif FMSTR_USE_JTAG
#if FMSTR_USE_JTAG_TXFIX
    /* in TX-bugfix mode, keep the RX interrupt enabled as it */
    /* is used as "able-to-TX" notification from the PC */
    FMSTR_JTAG_ERXI();
#else
    /* otherwise, JTAG is very same as the SCI */
    FMSTR_JTAG_DRXI();
    FMSTR_JTAG_ETXI();
#endif              

#endif
#endif      
}

/**************************************************************************//*!
*
* @brief    Output buffer transmission
*
* This function sends one character of the transmit buffer. It handles 
* replicating of the SOB characted inside the message body.
*
******************************************************************************/

static void FMSTR_Tx(void)
{
    FMSTR_U8 ch = 0;
    
    if (pcm_nTxTodo)
    {
        /* fetch & send character ready to transmit */
   		FMSTR_ValueFromBuffer8(&ch, pcm_pTxBuff);
        
#if FMSTR_USE_SCI       
        /* just put the byte into the SCI transmit buffer */
        FMSTR_SCI_PUTCHAR((FMSTR_U8) ch);
        
#elif FMSTR_USE_JTAG
        /* put byte to 32bit JTAG buffer */
        pcm_wJtagTxData = (pcm_wJtagTxData << 8) | ch;

        /* another byte  */
        pcm_wJtagTxCtr++;
        
        /* all four bytes ready?  */
        if(pcm_wJtagTxCtr & 0x4)
        {
            FMSTR_JTAG_PUTDWORD(pcm_wJtagTxData);
            pcm_wJtagTxCtr = 0;
        }
            
#endif

        /* SOB replication?      */
        if (ch != FMSTR_SOB || pcm_wFlags.flg.bTxLastCharSOB)
        {
            /* no, advance tx buffer pointer */
            pcm_nTxTodo--;
            pcm_pTxBuff = FMSTR_SkipInBuffer(pcm_pTxBuff, 1);
            pcm_wFlags.flg.bTxLastCharSOB = 0;
        }
        else
        {
            /* yes, repeat the SOB next time */
            pcm_wFlags.flg.bTxLastCharSOB = 1;
        }               
    }
#if FMSTR_USE_JTAG  
    /* on JTAG, the some bytes may still be pending in a 32bit buffer */
    else if(pcm_wJtagTxCtr > 0)
    {
        /* add padding bytes */
        while(!(pcm_wJtagTxCtr & 4))
        {
            pcm_wJtagTxData = (pcm_wJtagTxData << 8) | 0xff;
            pcm_wJtagTxCtr++;
        }

        /* send the word just completed */
        FMSTR_JTAG_PUTDWORD(pcm_wJtagTxData);

        /* done, bTxActive will be deactivated next time */
        pcm_wJtagTxCtr = 0;
    }
#endif  
    /* transmission finished, start listening again */
    else
    {
		/* when SCI TX buffering is enabled, we must first wait until all 
		  characters are physically transmitted (before disabling transmitter) */
#if FMSTR_USE_SCI && FMSTR_SCI_HAS_TXQUEUE
		pcm_wFlags.flg.bTxWaitTC = 1;

		/* wait for SCI TC interrupt */
		#if FMSTR_SHORT_INTR || FMSTR_LONG_INTR
		FMSTR_SCI_ETCI();
		#endif
#else
        /* start listening immediatelly */
        FMSTR_Listen();
#endif
    }
}


/**************************************************************************//*!
*
* @brief  Handle received character 
*
* @param  nRxChar  The character to be processed 
* 
* Handle the character received and -if the message is complete- call the 
* protocol decode routine. 
*
******************************************************************************/

static void FMSTR_Rx(FMSTR_BCHR nRxChar)
{
    /* first, handle the replicated SOB characters */
    if(nRxChar == FMSTR_SOB)
    {
        /* this is the 2nd byte of replicated SOB char */
        if(pcm_wFlags.flg.bRxLastCharSOB)
        {
            /* join the two SOBs into one 0x2b character and continue processing */
            pcm_wFlags.flg.bRxLastCharSOB = 0;
        }
        /* this is either the first byte of replicated SOB or a  */
        /* real Start-of-Block mark - we will decide next time in FMSTR_Rx */
        else
        {
            pcm_wFlags.flg.bRxLastCharSOB = 1;
            return;
        }
    }
    
    /* we have got a common character preceeded by the SOB -  */
    /* this is the command code! */
    if(pcm_wFlags.flg.bRxLastCharSOB)
    {
        /* reset reciving process */
        pcm_pRxBuff = pcm_pCommBuffer;
        *pcm_pRxBuff++ = nRxChar;
        
        /* start computing the checksum */
        pcm_nRxCheckSum = nRxChar;
        pcm_nRxTodo = 0;

        /* fast command? */
        if((nRxChar & FMSTR_FASTCMD) == FMSTR_FASTCMD)
        {
            /* there will be no length information */
            pcm_wFlags.flg.bRxMsgLengthNext = 0;
            /* as it is encoded in the command byte directly */
            pcm_nRxTodo = (FMSTR_SIZE8) (((nRxChar & FMSTR_FASTCMD_DATALEN_MASK) 
                >> FMSTR_FASTCMD_DATALEN_SHIFT) + 1);
        }
        /* standard command */
        else
        {
            /* the message length will come in next byte */
            pcm_wFlags.flg.bRxMsgLengthNext = 1;
        }

        /* command code stored & processed */
        pcm_wFlags.flg.bRxLastCharSOB = 0;
        return;
    }

    /* we are waiting for the length byte */
    if(pcm_wFlags.flg.bRxMsgLengthNext)
    {
        /* this byte, total data length and the checksum */
        pcm_nRxTodo = (FMSTR_SIZE8) (1 + nRxChar + 1);
        /* now read the data bytes */
        pcm_wFlags.flg.bRxMsgLengthNext = 0;

    }
    
    /* waiting for a data byte? */
    if(pcm_nRxTodo)
    {
        /* add this byte to checksum */
        pcm_nRxCheckSum += nRxChar;
        
        /* was it the last byte of the message (checksum)? */
        if(!--pcm_nRxTodo)
        {
            /* receive buffer overflow? */
            if(pcm_pRxBuff == NULL)
            {
                FMSTR_SendError(FMSTR_STC_CMDTOOLONG);
            }
            /* checksum error? */
            else if((pcm_nRxCheckSum & 0xff) != 0)
            {
                FMSTR_SendError(FMSTR_STC_CMDCSERR);
            }
            /* message is okay */
            else 
            {
                /* do decode now! */
                FMSTR_ProtocolDecoder(pcm_pCommBuffer);
            }
        }
        /* not the last character, is there a space in buffer? */
        else if(pcm_pRxBuff)
        {
            if(pcm_pRxBuff < (pcm_pCommBuffer + FMSTR_COMM_BUFFER_SIZE))
            {
                /* store byte  */
                *pcm_pRxBuff++ = nRxChar;
            }
            /* buffer is full! */
            else
            {
                /* NULL rx pointer means buffer overflow - but we still need */
                /* to receive all message characters (for the single-wire mode) */
                /* so keep "receiving" - but throw away all characters from now */
                pcm_pRxBuff = NULL;
            }
        }
    }
}

/*******************************************************************************
*
* @brief    Routine to quick-receive a character (put to a queue only)
*
* This function puts received character into a queue and exits as soon as possible.
*
*******************************************************************************/

#if FMSTR_SHORT_INTR

static void FMSTR_RxQueue(FMSTR_BCHR nRxChar)
{ 
    /* future value of write pointer */
    FMSTR_BPTR wpnext = pcm_pRQueueWP + 1;
    
    if(wpnext >= (pcm_pRQueueBuffer + FMSTR_COMM_RQUEUE_SIZE))
        wpnext = pcm_pRQueueBuffer;
    
    /* any space in queue? */
    if(wpnext != pcm_pRQueueRP)
    {
        *pcm_pRQueueWP = (FMSTR_U8) nRxChar;
        pcm_pRQueueWP = wpnext;
    }
}

#endif /* FMSTR_SHORT_INTR  */

/*******************************************************************************
*
* @brief    Late processing of queued characters
*
* This function takes the queued characters and calls FMSTR_Rx() for each of them,
* just like as the characters would be received from SCI one by one.
*
*******************************************************************************/

#if FMSTR_SHORT_INTR

static void FMSTR_RxDequeue(void)
{ 
    FMSTR_BCHR nChar = 0;
    
    /* get all queued characters */
    while(pcm_pRQueueRP != pcm_pRQueueWP)
    {
        nChar = *pcm_pRQueueRP++;

        if(pcm_pRQueueRP >= (pcm_pRQueueBuffer + FMSTR_COMM_RQUEUE_SIZE))
            pcm_pRQueueRP = pcm_pRQueueBuffer;

        /* emulate the SCI receive event         */
        if(!pcm_wFlags.flg.bTxActive)
            FMSTR_Rx(nChar);
    }
}

#endif /* FMSTR_SHORT_INTR */

/**************************************************************************//*!
*
* @brief    Handle SCI communication (both TX and RX)
*
* This function checks the SCI flags and calls the Rx and/or Tx functions
*
* @note This function can be called either from SCI ISR or from the polling routine
*
******************************************************************************/

#if FMSTR_USE_SCI

void FMSTR_ProcessSCI(void)
{
    /* read & clear status     */
    FMSTR_SCISR wSciSR = FMSTR_SCI_RDCLRSR();

    /* transmitter active and empty? */
    if (pcm_wFlags.flg.bTxActive)
    {
	    /* able to accept another character? */
	    if(wSciSR & FMSTR_SCISR_TDRE)
	        FMSTR_Tx();
	    
#if FMSTR_SCI_HAS_TXQUEUE
		/* waiting for transmission complete flag? */
		if(pcm_wFlags.flg.bTxWaitTC && (wSciSR & FMSTR_SCISR_TC))
		{
			/* after TC, we can switch to listen mode safely */
            FMSTR_Listen();
		}
#endif

        /* read-out and ignore any received character (loopback) */
        if(wSciSR & FMSTR_SCISR_RDRF)
        {
            volatile FMSTR_U16 nRxChar;
            nRxChar = FMSTR_SCI_GETCHAR(); 
        }
    }
    /* transmitter not active, able to receive */
    else
    {
        /* data byte received? */
        if (wSciSR & FMSTR_SCISR_RDRF)
        {
            FMSTR_BCHR nRxChar = 0;
            nRxChar = FMSTR_SCI_GETCHAR();

#if FMSTR_SHORT_INTR
            FMSTR_RxQueue(nRxChar);
#else
            FMSTR_Rx(nRxChar);  
#endif          
        }
    }
}

#endif

/**************************************************************************//*!
*
* @brief    Handle JTAG communication (both TX and RX)
*
* This function checks the JTAG flags and calls the Rx and/or Tx functions
*
* @note This function can be called either from JTAG ISR or from the polling routine
*
******************************************************************************/

#if FMSTR_USE_JTAG

void FMSTR_ProcessJTAG(void)
{
    /* read & clear status     */
    register FMSTR_U16 wJtagSR = FMSTR_JTAG_GETSR();

    /* transmitter active? */
    if (pcm_wFlags.flg.bTxActive)
    {
        /* able to transmit a new character? (TX must be empty = read-out by PC) */
        if(!(wJtagSR & FMSTR_JTAG_OTXRXSR_TDF))
        {
        
#if FMSTR_USE_JTAG_TXFIX
            /* if TDF bit is useless due to silicon bug, use the RX flag */
            /* instead (PC sends us a dummy word to kick the RX flag on) */
            if(wJtagSR & FMSTR_JTAG_OTXRXSR_RDF)
#endif
            {
                /* send one byte always */
                FMSTR_Tx();
                
                /* try to fill-up the full 32bit JTAG word */
                while(pcm_wFlags.flg.bTxActive && pcm_wJtagTxCtr)
                    FMSTR_Tx();
            }               
        }

        /* ignore (read-out) the JTAG-received word */
        if(wJtagSR & FMSTR_JTAG_OTXRXSR_RDF)
        {
            volatile FMSTR_U16 nRxWord;
            nRxWord = FMSTR_JTAG_GETWORD();
        }
    }
    /* transmitter not active */
    else
    {
        /* JTAG 32bit word (four bytes) received? */
        if(wJtagSR & FMSTR_JTAG_OTXRXSR_RDF)
        {
            register FMSTR_U32 nRxDWord;
            FMSTR_INDEX i;
            
            nRxDWord = FMSTR_JTAG_GETDWORD();
            
            /* process all bytes, MSB first */
            for(i=0; i<4; i++)
            {
#if FMSTR_SHORT_INTR
                FMSTR_RxQueue((FMSTR_BCHR)((nRxDWord >> 24) & 0xff));
                
#else
                FMSTR_Rx((FMSTR_BCHR)((nRxDWord >> 24) & 0xff));
            
                /* ignore the rest if previous bytes triggered a transmission */
                /* (i.e. the packet was complete and only filled-up to 32bit word) */
                if(pcm_wFlags.flg.bTxActive)
                    break;
#endif          
                /* next byte of 32bit word */
                nRxDWord = nRxDWord << 8;
            }
        }
    }
}

#endif

/*******************************************************************************
*
* @brief    API: Main "Polling" call from the application main loop
*
* This function either handles all the SCI communictaion (polling-only mode = 
* FMSTR_POLL_DRIVEN) or decodes messages received on the background by SCI interrupt
* (short-interrupt mode = FMSTR_SHORT_INTR). 
*
* In fully interrupt-driven mode (FMSTR_LONG_INTR) this function does nothing.
*
*******************************************************************************/

void FMSTR_Poll(void)
{ 
#if FMSTR_POLL_DRIVEN

#if FMSTR_USE_SCI
    FMSTR_ProcessSCI(); 
    
#elif FMSTR_USE_JTAG
    FMSTR_ProcessJTAG(); 
    
#endif
    
#elif FMSTR_SHORT_INTR

    /* process queued SCI characters */
    FMSTR_RxDequeue(); 
    
#endif
}

#else /* FMSTR_USE_SCI || FMSTR_USE_JTAG */

/* Empty implementation of communication functions

   Without a SCI and JTAG the FreeMASTER driver still passes the compilation,
   but no communication is supported. The user may imlement his own communication
   protocol and use FreeMASTER by calling FMSTR_ProtocolDecoder and 
   overriding the FMSTR_SendResponse calls 
*/

void FMSTR_SendResponse(FMSTR_BPTR pResponse, FMSTR_SIZE8 nLength) 
{
	FMSTR_UNUSED(pResponse);
	FMSTR_UNUSED(nLength);	
}

void FMSTR_Poll(void) 
{
}

#endif /* FMSTR_USE_SCI || FMSTR_USE_JTAG */

