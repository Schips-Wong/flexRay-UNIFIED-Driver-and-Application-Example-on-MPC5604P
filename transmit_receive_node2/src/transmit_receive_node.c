/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2006 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***************************************************************************//*!
*
* @file      transmit_receive_node2.c
*
* @author    R62779
* 
* @version   1.0.1.0
* 
* @date      Dec-3-2008
* 
* @brief     "Transmit Receive" Application example for FlexRay UNIFIED driver.
*
*            Poll driven mode
*
*            Example of usage the message buffers in poll-driven mode.
*
******************************************************************************/

/* Project Include Files */
#include "c_types.h"                    /* Include structures, prototypes, definitions */
#include "jdp.h"                        /* Header file for including device specific architecture headers */

#include "freemaster.h"			/* Header file for FreeMASTER tool */

#include "Fr_UNIFIED_types.h"           /* Standard and driver types */
#include "Fr_UNIFIED.h"                 /* UNIFIED driver implementation */
#include "Fr_UNIFIED_cfg.h"		/* Configuration data for one FlexRay node */

#define LEDS_USED                       /* Leds are used */
//#define FREEMASTER_USED                 /* This define should be commented if the FreeMASTER tool is not used */
#define LED1OUT SIU.GPDO[15].B.PDO
#define LED2OUT SIU.GPDO[14].B.PDO
#define LED3OUT SIU.GPDO[22].B.PDO
#define LED4OUT SIU.GPDO[38].B.PDO

#define TX_SLOT_4                   1   /* Transmit MB 1, slot 4 */
#define RX_SLOT_1                   2   /* Receive MB 2, slot 1 */
#define TX_SLOT_5                   4   /* Transmit MB 4, slot 5 */
#define TX_SLOT_5_TRANSMIT_SIDE     5   /* Double Transmit Buffer - transmit sideMB 5, slot 5 */
#define TX_SLOT_62                  11  /* Transmit MB 11, slot 62 */
#define TX_SLOT_63                  12  /* Transmit MB 12, slot 63 */

Fr_return_type return_value;            /* Variable used for storing of the return values */
Fr_POC_state_type protocol_state;       /* Current protocol state */
Fr_wakeup_state_type wakeup_status;     /* Current wakeup status */
uint8 current_cycle;                    /* The current cycle value */
uint16 macrotick_cnt;			/* current macrotick count*/

/* Definition of the variables used for updating of the transmit MBs */
uint16 tx_data_4[16] = {0};             /* Data array - static segment, slot 4 */
uint16 tx_data_5[16] = {0};             /* Data array - static segment, slot 5 */
uint16 tx_data_62[8] = {0};             /* Data array - dynamic segment, slot 62 */
uint16 tx_data_63[8] = {0};             /* Data array - dynamic segment, slot 63 */
Fr_tx_MB_status_type tx_return_value;   /* Variable used for storing of the return values */
Fr_tx_status_type tx_status;            /* Variable to determine if data has been tranmsitted */

/* Definition of the variables used for storing data and status from the receive MBs */
uint16 rx_data_1[16] = {0};             /* Data array - static segment, slot 1 */
uint8 rx_data_length = 0;               /* Received data length  */
uint16 rx_status_slot = 0;              /* Received frame status */
Fr_rx_MB_status_type rx_return_value;   /* Variable used for storing of the return values */
Fr_rx_status_type rx_status;            /* Variable to determine if a frame has been received */

uint16 mb_access_error = 0;             /* The number of the message buffer access errors */
uint16 chi_error = 0;                   /* The number of the CHI related errors */
uint16 transmission_across_boundary = 0;    /* The number of the transmission across boundary errors */
uint16 violation = 0;                   /* The number of boundary violation errors */
boolean protocol_error = FALSE;         /* Internal protocol error detected */

/*********************************** Definitions *****************************/
/* FreeMASTER TSA support */
    FMSTR_TSA_TABLE_BEGIN(first_table)
    FMSTR_TSA_RO_VAR(tx_data_4[0], FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(tx_data_5[0], FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(tx_data_62[0], FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(tx_data_63[0], FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(rx_data_1[0], FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(protocol_state, FMSTR_TSA_USERTYPE(Fr_POC_state_type))
    FMSTR_TSA_RO_VAR(wakeup_status, FMSTR_TSA_USERTYPE(Fr_wakeup_state_type))
    FMSTR_TSA_RO_VAR(current_cycle, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(mb_access_error, FMSTR_TSA_UINT16) 
    FMSTR_TSA_RO_VAR(violation, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(transmission_across_boundary, FMSTR_TSA_UINT16)
    FMSTR_TSA_RO_VAR(protocol_error, FMSTR_TSA_USERTYPE(boolean))
    FMSTR_TSA_TABLE_END()

    FMSTR_TSA_TABLE_LIST_BEGIN()
    FMSTR_TSA_TABLE(first_table)
    FMSTR_TSA_TABLE_LIST_END()

/* End of FreeMASTER TSA support */


/*********************************** Protypes ********************************/
void InitMCU(void);
void Failed(uint8 number);
void FREEMASTER_ISR(void);
void FLEXRAY_ISR(void);

/*********************************** Functions ******************************/
void Failed(uint8 number) /* Function only for debugging */
{
    /* CC should be restarted */
    //while(1)
    {
        #ifdef FREEMASTER_USED
            /* FreeMASTER polling */
            FMSTR_Poll();
        #endif /* FREEMASTER_USED */
    }
}
void delay(int ms)
{   
   int ii,jj;
   if (ms<1) ms=1;
   for(ii=0;ii<ms;ii++)
     for(jj=0;jj<5000;jj++){}        
}
/******************************************************************************
* Function name : main 
* Description   : Main function of the application 
*                 In this function the MPC560xP microcontroller peripherals, FlexRay module
*                 are initialized. All configured MB are initialized, interrupts are enabled.
* Author        : R62779
* 
* Parameters    : None
*
* Returns       : None
*
******************************************************************************/
void main(void) 
{
     boolean cycle_starts = FALSE;                   /* Did the communication cycle start? */
    
    /* Initialize MPC560xP MCU */
    InitMCU();    
 
    /* Enable the FlexRay CC and force it into FR_POCSTATE_CONFIG */
    return_value = Fr_init(&Fr_HW_cfg_00, &Fr_low_level_cfg_set_00);
    if(return_value == FR_NOT_SUCCESS) Failed(1);   /* Call debug function in case of any error */

    /* Initialization of the FlexRay CC with protocol configuration parameter */
    Fr_set_configuration(&Fr_HW_cfg_00, &Fr_low_level_cfg_set_00);
    
    /* Initialization of all message buffers, receive shadow buffers and FIFO storages */
    return_value = Fr_buffers_init(&Fr_buffer_cfg_00[0], &Fr_buffer_cfg_set_00[0]);
    /* Call debug function - the FR_NUMBER_TXRX_MB parameter is incorrectly configured in the Fr_UNIFIED_cfg.h file */
    if(return_value == FR_NOT_SUCCESS) Failed(0xFF); 
    
    /* Set callback functions */
    /* No FlexRay interrupt is enabled -> no callback function */

    /* Leave FR_POCSTATE_CONFIG state */
    return_value = Fr_leave_configuration_mode();
    if(return_value == FR_NOT_SUCCESS) Failed(2);   /* Call debug function in case of any error */

    /* Retrieve the wakeup state */
    wakeup_status = Fr_get_wakeup_state();
    
    /* Check whether a wakeup pattern has been received */
    if(wakeup_status == FR_WAKEUPSTATE_UNDEFINED)
    {   /* No wakeup pattern has been received */
        /* Initiate wakeup procedure */
        return_value = Fr_send_wakeup();
        if(return_value == FR_NOT_SUCCESS) Failed(3);   /* Call debug function in case of any error     */
    }
   
    protocol_state = Fr_get_POC_state();            /* Load current POC state */
    while(Fr_get_POC_state() != FR_POCSTATE_READY)  /* Wait till the FR CC is not in the FR_POCSTATE_READY */
        ;
    
    /* Initialize startup */
    return_value = Fr_start_communication();
    if(return_value == FR_NOT_SUCCESS) Failed(4);   /* Call debug function in case of any error */
    
    protocol_state = Fr_get_POC_state();            /* Load current POC state */
    while(Fr_get_POC_state() != FR_POCSTATE_NORMAL_ACTIVE)  /* Wait till the FR CC is not in the FR_POCSTATE_NORMAL_ACTIVE */
    {
    #ifdef FREEMASTER_USED
      FMSTR_Poll();
    #endif
    } 
    
    protocol_state = Fr_get_POC_state();            /* Load current POC state */

    /* The first initialization of the MB 1 */
    tx_return_value = Fr_transmit_data(TX_SLOT_4, &tx_data_4[0], 16);
    if(tx_return_value == FR_TXMB_NO_ACCESS) Failed(4);   /* Call debug function in case of any error */

    /* The first initialization of the MB  */
    tx_return_value = Fr_transmit_data(TX_SLOT_5, &tx_data_5[0], 16);
    if(tx_return_value == FR_TXMB_NO_ACCESS) Failed(6);   /* Call debug function in case of any error */
 
    /* The first initialization of the MB  */
    tx_return_value = Fr_transmit_data(TX_SLOT_62, &tx_data_62[0], 8);
    if(tx_return_value == FR_TXMB_NO_ACCESS) Failed(7);   /* Call debug function in case of any error */

    /* The first initialization of the MB  */
    tx_return_value = Fr_transmit_data(TX_SLOT_63, &tx_data_63[0], 8);
    if(tx_return_value == FR_TXMB_NO_ACCESS) Failed(8);   /* Call debug function in case of any error */
 
    /* No FlexRay interrupt is enabled */
    wakeup_status = Fr_get_wakeup_state();          /* Load current wakeup status */
    LED1OUT=1;
	LED2OUT=1;
	//LED3OUT=1;
	LED4OUT=1;
    for(;;)
    {
        /* Check whether or not the communication cycle has been started */
        cycle_starts = Fr_check_cycle_start(&current_cycle);
        if(cycle_starts)                                /* Did the communication cycle start? */
        {
            /* RECEIVE BUFFER - copy received data from receive MB 2 */
            rx_status = Fr_check_rx_status(RX_SLOT_1);  /* Check whether the MB has been updated in last matching slot */
            if(rx_status == FR_RECEIVED)                /* MB data field updated in last matching slot */
            {
                /* Copy data into given data array */
                rx_return_value = Fr_receive_data(RX_SLOT_1, &rx_data_1[0], &rx_data_length, &rx_status_slot);
                tx_data_5[1]++;                         /* Increment variable - to be transmitted later on */
                tx_data_5[2] = rx_status_slot;          /* Copy slot status into variable - to be transmitted later on */
                if(rx_return_value == FR_RXMB_NO_ACCESS) mb_access_error++;        /* Increment variable in case of MB access error */
            }
	    
	    /* TRANSMIT SINGLE BUFFER - update transmit MB 1 with new data */
            tx_status = Fr_check_tx_status(TX_SLOT_4);  /* Check whether data has been transmitted */
            if(tx_status == FR_TRANSMITTED)
            {
                tx_data_4[0] -= 100;                    /* Decrement variable */
                tx_return_value = Fr_transmit_data(TX_SLOT_4, &tx_data_4[0], 16);  /* Update transmit MB with new data */
                if(tx_return_value == FR_TXMB_NO_ACCESS) mb_access_error++;        /* Increment variable in case of MB access error */
            }

            /* TRANSMIT DOUBLE BUFFER - update double transmit MB 4 with new data */
            tx_status = Fr_check_tx_status(TX_SLOT_5);  /* Check whether data has been transferred or transmitted */
            /* Update commit side of double MB in case that Internal Message Transfer has been performed */
            if((tx_status == FR_TRANSMITTED) || (tx_status == FR_INTERNAL_MESSAGE_TRANSFER_DONE))
            {
                tx_data_5[0] += 100;                    /* Increment variable */
                tx_return_value = Fr_transmit_data(TX_SLOT_5, &tx_data_5[0], 16);  /* Update transmit MB with new data */
                if(tx_return_value == FR_TXMB_NO_ACCESS) mb_access_error++;        /* Increment variable in case of MB access error */
            }

            /* TRANSMIT BUFFER - update transmit MB 11 with new data */
            tx_status = Fr_check_tx_status(TX_SLOT_62);   /* Check whether data has been transmitted */
            if(tx_status == FR_TRANSMITTED)
            {
                tx_data_62[0] += 50;                    /* Increment variable */
                tx_return_value = Fr_transmit_data(TX_SLOT_62, &tx_data_62[0], 0); /* Update transmit MB with new data */
                if(tx_return_value == FR_TXMB_NO_ACCESS) mb_access_error++;        /* Increment variable in case of MB access error */
            }
            
            tx_data_5[3]++;							    /* Increment variable - to be transmitted later on */
            tx_data_5[4] = current_cycle;               /* Store current cycle number into transmit data array */
            
            /* Check whether the CHI related error has been occured */
            if(Fr_check_CHI_error() != 0) chi_error++;  /* Increment variable if CHI error occured - to be transmitted later on */
            tx_data_5[6] = chi_error;                   /* Store the number of the CHI related errors */
            
            /* Check whether or not the frame transmission across boundary has been occured */
            /* Increment variable if error occured - to be transmitted later on */
            if(Fr_check_transmission_across_boundary(FR_CHANNEL_AB)) transmission_across_boundary++;
            tx_data_5[7] = transmission_across_boundary;    /* Store the number of errors */
            
            /* Check whether or not the frame transmission in dynamic segment exceeded the dynamic segment boundary */
            /* Increment variable if error occured - to be transmitted later on */
            if(Fr_check_violation(FR_CHANNEL_AB)) violation++;
            tx_data_5[8] = violation;                   /* Store the number of errors */
         }

        /* Check whether or not the protocol engine has detected an internal protocol error */
        protocol_error = Fr_check_internal_protocol_error();    /* TRUE -> Protocol engine goes into the FR_POCSTATE_HALT state */
         
        /* TRANSMIT BUFFER - update transmit MB 12 with new data */
        tx_status = Fr_check_tx_status(TX_SLOT_63);     /* Check whether data has been transmitted */
        if(tx_status == FR_TRANSMITTED)
        {
            tx_data_63[0] -= 50;                        /* Decrement variable */
            tx_return_value = Fr_transmit_data(TX_SLOT_63, &tx_data_63[0], 0);  /* Update transmit MB with new data */
            if(tx_return_value == FR_TXMB_NO_ACCESS) mb_access_error++;         /* Increment variable in case of MB access error */
        }
        
        /* Store the value of access errors to the transmit data array - to be transmitted later on  */
        tx_data_5[5] = mb_access_error;
        delay(500);
        LED4OUT=~LED4OUT;
        /* FreeMASTER routine */
        #ifdef FREEMASTER_USED
            /* FreeMASTER polling */
            FMSTR_Poll();
        #endif /* FREEMASTER_USED */
    }

} /* End of main */


/******************************************************************************
* Function name : InitMCU
* Description   : This function initialize the MPC560xP microcontroller
* Author        : R62779
* 
* Parameters    : None
*
* Returns       : None
*
******************************************************************************/
void InitMCU(void)
{ 
    /* Set PLL0 */
    CGM.FMPLL[0].CR.B.IDF  =  4;       /* 4: use incoming OSC 40MHz / 5 = 8 MHz */
    CGM.FMPLL[0].CR.B.NDIV = 40;       /* VCO = 8 * 40 = 320 MHz */
    CGM.FMPLL[0].CR.B.ODF  = 2;        /* 2 - div8 320 / 8 = 40MHz */
    /* Set PLL1 */
    CGM.FMPLL[1].CR.B.IDF  =  4;       /* 4: use incoming OSC 40MHz /5 = 8 MHz */
    CGM.FMPLL[1].CR.B.NDIV = 40;       /* 8 * 40 = 320 MHz */
    CGM.FMPLL[1].CR.B.ODF  = 1;        /* 1 - div4 320 / 4 = 80MHz */

    /* FlexRay Channel A TXEN : CA_TR_EN */
    SIU.PCR[47].B.PA  = 1;             /* Pin alternate 1 function (FlexRay TR_EN) */
    SIU.PCR[47].B.IBE = 0;             /* Output buffer enable  */
    SIU.PCR[47].B.SRC = 1;             /* Slewrate control - fastest configuration */
    /* FlexRay Channel A TXD : CA_TX */
    SIU.PCR[48].B.PA  = 1;             /* Pin alternate 1 function (FlexRay TR_EN) */
    SIU.PCR[48].B.IBE = 0;             /* Output buffer enable  */
    SIU.PCR[48].B.SRC = 1;             /* Slewrate control - fastest configuration */
    /* FlexRay Channel A RXD : CA_TX */
    SIU.PCR[49].B.PA  = 1;             /* Pin alternate 1 function (FlexRay TR_EN) */
    SIU.PCR[49].B.IBE = 1;             /* Output buffer enable  */
    SIU.PCR[49].B.SRC = 1;             /* Slewrate control - fastest configuration */

    /* FlexRay Channel B TXEN : CB_TR_EN */
    SIU.PCR[52].B.PA  = 1;             /* Pin alternate 1 function (FlexRay TR_EN) */
    SIU.PCR[52].B.IBE = 0;             /* Output buffer enable  */
    SIU.PCR[52].B.SRC = 1;             /* Slewrate control - fastest configuration */
    /* FlexRay Channel B TXD : CB_TX */
    SIU.PCR[51].B.PA  = 1;             /* Pin alternate 1 function (FlexRay TR_EN) */
    SIU.PCR[51].B.IBE = 0;             /* Output buffer enable  */
    SIU.PCR[51].B.SRC = 1;             /* Slewrate control - fastest configuration */
    /* FlexRay Channel A RXD : CB_RX */
    SIU.PCR[50].B.PA  = 1;             /* Pin alternate 1 function (FlexRay TR_EN) */
    SIU.PCR[50].B.IBE = 1;             /* Output buffer enable  */
    SIU.PCR[50].B.SRC = 1;             /* Slewrate control - fastest configuration */
    /* CLKOUT */   
    SIU.PCR[22].B.PA = 1;              /* Pin alternate mode 1 function (clkout) */
  
    #ifdef LEDS_USED    
	SIU.PCR[15].R = 0x0203;//PA15   LED1
	SIU.PCR[14].R = 0x0203;//PA14   LED2
	//SIU.PCR[22].R = 0x0203;//PB6    LED3
	SIU.PCR[38].R = 0x0203;//PC6    LED4
    #endif /*LEDS_USED*/
  
    ME.MER.R = 0x000005FF;             /* Enable all modes */
	
    ME.DRUN.B.DFLAON = 0x3;            /* Data flash is in normal power mode */
    ME.DRUN.B.CFLAON = 0x3;            /* Code flash is in normal power mode */
    ME.DRUN.B.PLL1ON = 0x1;            /* PLL0 is on */
    ME.DRUN.B.PLL2ON = 0x1;            /* PLL1 is on */
    ME.DRUN.B.XOSC0ON = 0x1;           /* XOSC is on */
    ME.DRUN.B.IRCON = 0x1;             /* RC   is on */
    ME.DRUN.B.SYSCLK = 0x4;            /* System clock is system PLL */
	
    ME.RUNPC[0].R = 0x000000FE;        /* Enable Periphs in all running modes */
    ME.LPPC[0].R = 0x00000500;         /* Enable Periphs in low power modes */
	
    CGM.AC0SC.B.SELCTL = 0;            /* Alternative clock domain 0 uses internal RC oscilator 
                                        - timers, ADC, CTU */
    CGM.AC1SC.B.SELCTL = 0;            /* Alternative clock domain 1 uses internal RC oscilator
                                        - only CMU1 */
    CGM.AC2SC.B.SELCTL = 0x5;          /* Alternative clock domain 2 uses secondary PLL 
                                        -  FlexRay */

    CGM.OCEN.R = 0x1;                  /* Enable clock output */
    CGM.OCDSSC.B.SELCTL = 0x3;         /* Secondary PLL clock used as output */
    CGM.OCDSSC.B.SELDIV = 0x3;         /* Divide clock output by 8 */
   
    CGM.AC0DC.B.DE0 = 0x1;             /* Enable divider (defaultly se to 1)*/
    CGM.AC1DC.B.DE0 = 0x1;
    CGM.AC2DC.B.DE0 = 0x1;             /* PE clock divider */
    CGM.AC2DC.B.DE1 = 0x1;
	
    /* Avoid CMU reset in case of Fmosc<Frcosc */
    CGM.CMU_0_CSR.B.RCDIV = 3;
    CGM.CMU_0_LFREFR_A.B.LFREF_A = 0x1;
    CGM.CMU_0_HFREFR_A.B.HFREF_A = 0xFFE;

    /* No interrupts initialization - whole node is completely poll driven */
    
    /* Disable Watchdog */
    SWT.SR.R = 0x0000c520;              /* Key needed */
    SWT.SR.R = 0x0000d928;              /* Key needed */
    SWT.CR.R = 0x8000010A;              /* Disable WEN */

    /* Re-enter in DRUN mode to update (after reset is in DRUN) */
    ME.MCTL.R = (0x3 << 28 | 0x00005AF0);  /* DRUN Mode & Key */
    ME.MCTL.R = (0x3 << 28 | 0x0000A50F);  /* DRUN Mode & Key */
    
    while(CGM.FMPLL[0].CR.B.S_LOCK == 0);  /* Wait for PLL0 lock */
    while(CGM.FMPLL[1].CR.B.S_LOCK == 0);  /* Wait for PLL1 lock */
  
    while(ME.GS.B.S_MTRANS == 1);          /* Wait for mode entry to complete */
    while(ME.GS.B.S_CURRENTMODE != 0x3);   /* Check DRUN mode has been entered */

    #ifdef FREEMASTER_USED
        /* Connect pins to LINFlex - needed for the FreeMASTER tool */
        SIU.PCR[19].B.PA = 1;          /* Pin asigned to LINFlex Rx */
        SIU.PCR[19].B.IBE = 1;         /* Input buffer enable*/
        SIU.PCR[18].B.PA = 1;          /* Pin asigned to LINFlex Tx */
        SIU.PCR[18].B.OBE = 1;         /* Output buffer enable */
    
        LINFLEX_0.LINCR1.B.SLEEP = 0;  /* Wakeup LinFlex0 */
        LINFLEX_0.LINCR1.B.INIT = 1;   /* Request initialization */
        /* Initialize LINFlex as UART */
        LINFLEX_0.UARTCR.B.UART = 1;   /* UART mode enabled */
        LINFLEX_0.UARTCR.B.WL = 1;     /* 8 data bits */
        LINFLEX_0.UARTCR.B.PCE = 0;    /* Parity controll disabled */
        /*Set BaudRate 115200Bd */
        LINFLEX_0.LINIBRR.B.DIV_M = 21; /* Clock divisor's mantisa */
        LINFLEX_0.LINFBRR.B.DIV_F = 11; /* Clock divisor's fraction */
        /* BaudRate=40Mhz/(16*(mantisa+fraction/16))=40MHz/(16*(21+11/16))=115274Bd */
        /* Raise LINFlex interupt priority */
        /* Enable interupts in the LINFlex module */
	/* No interupts used, FREEMaster runs in poling mode */
        
        LINFLEX_0.LINCR1.B.INIT = 0;    /* End of initialization*/
        /* FreeMASTER initialization */
        FMSTR_Init();
    #endif /* FREEMASTER_USED */
}

/******************************************************************************
* Function name : FLEXRAY_ISR
* Description   : Interrupt function from the FlexRay module
*                 The FlexRay UNIFIED driver interrupt handler is called if an interrupt arises
*
* Author        : R62779
* 
* Parameters    : None
*
* Returns       : None
*
******************************************************************************/
void FLEXRAY_ISR(void)
{
    #ifdef LEDS_USED
        LED1OUT=~LED1OUT;
    #endif  /*LEDS_USED */

    Fr_interrupt_handler(); /* Call FlexRay driver interrupt service routine handler */    

  /* Signal the end of the servicing of the interrupt request by writing 0
  to EOIR_PRC0 */
}

/******************************************************************************
* Function name : FREEMASTER_ISR
* Description   : Interrupt function from FreeMASTER tool interrupt (a SCI).
*
* Author        : R62779
* 
* Parameters    : None
*
* Returns       : None
*
******************************************************************************/
void FREEMASTER_ISR(void)
{
    volatile uint32_t temp = 0U;
    
    #ifdef FREEMASTER_USED
        FMSTR_Isr();        /* Calls FreeMASTER service routine */
	temp = LINFLEX_0.UARTSR.R;
	LINFLEX_0.UARTSR.R = temp;
    #endif  /* FREEMASTER_USED */

  /* Signal the end of the servicing of the interrupt request by writing 0
  to EOIR_PRC0 */
}
