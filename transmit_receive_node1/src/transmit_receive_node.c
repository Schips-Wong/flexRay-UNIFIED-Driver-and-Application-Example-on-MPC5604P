/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2006 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***************************************************************************//*!
*
* @file      transmit_receive_node1.c
*
* @author    R62779
* 
* @version   1.0.1.0
* 
* @date      Dec-3-2008
* 
* @brief     "Transmit Receive" Application example for the FlexRay UNIFIED Driver.
*
*            Interrupt driven mode
*
*            Example of usage the message buffers in the interrupt-driven mode.
*            The double buffered message buffer operates in interrupt mode
*            with an interrupt enabled at transmit side of the double MB
*
******************************************************************************/

/* Project Include Files */
#include "c_types.h"                    /* Include structures, prototypes, definitions */
#include "jdp.h"                        /* Header file for including device specific architecture headers */

#include "freemaster.h"			/* Header file for FreeMASTER tool */

#include "Fr_UNIFIED_types.h"           /* Standard and driver types */
#include "Fr_UNIFIED.h"                 /* UNIFIED driver implementation */
#include "Fr_UNIFIED_cfg.h"             /* Configuration data for one FlexRay node */

#define LEDS_USED                       /* Leds are used */
#define FREEMASTER_USED                 /* This define should be commented if the FreeMASTER tool is not used */

#define TX_SLOT_1                   0   /* Double transmit MB 0 - commit side, slot 1 */
#define TX_SLOT_1_TRANSMIT_SIDE     1   /* Double transmit MB 1 - transmit side, slot 1 */
#define RX_SLOT_4                   3   /* Receive MB 3, slot 4 */

Fr_return_type return_value;            /* Variable used for storing of the return values */
Fr_POC_state_type protocol_state;       /* Current protocol state */
Fr_wakeup_state_type wakeup_status;     /* Current wakeup status */
uint8 current_cycle;                    /* The current cycle value */
uint16 current_macrotick;               /* The current macrotick value */

/* Definition of the variables used for updating of the transmit MBs */
uint16 tx_data_1[16] = {0};             /* Data array - static segment, slot 1 */
Fr_tx_MB_status_type tx_return_value;   /* Variable used for storing of the return values */

/* Definition of the variables used for storing data and status from the receive MBs */
uint16 rx_data_4[16] = {0};             /* Data array - static segment, slot 4 */
uint8 rx_data_length = 0;               /* Received data length  */
uint16 rx_status_slot = 0;              /* Received frame status */
Fr_rx_MB_status_type rx_return_value;   /* Variable used for storing of the return values */

/* Definition of the variables used for storing data and status of the FIFO A */
uint16 fifo_data[8] = {0};              /* Data array */
uint8 fifo_data_length = 0;             /* Received data length */
uint16 fifo_status_slot = 0;            /* Received frame status */
uint16 fifo_slot_idx = 0;               /* Received frame index */
Fr_FIFO_status_type fifo_return_value;  /* Variable used for storing of the return values */

/*********************************** Definitions *****************************/
/* FreeMASTER TSA support */
    FMSTR_TSA_TABLE_BEGIN(first_table)
        FMSTR_TSA_RO_VAR(tx_data_1[0], FMSTR_TSA_UINT16)
        FMSTR_TSA_RO_VAR(tx_data_1[1], FMSTR_TSA_UINT16)
        FMSTR_TSA_RO_VAR(tx_data_1[5], FMSTR_TSA_UINT16)
        FMSTR_TSA_RO_VAR(tx_data_1[6], FMSTR_TSA_UINT16)
        FMSTR_TSA_RO_VAR(tx_data_1[7], FMSTR_TSA_UINT16)
        FMSTR_TSA_RO_VAR(tx_data_1[8], FMSTR_TSA_UINT16)
        FMSTR_TSA_RO_VAR(tx_data_1[9], FMSTR_TSA_UINT16)
        FMSTR_TSA_RO_VAR(tx_data_1[10], FMSTR_TSA_UINT16)
        FMSTR_TSA_RO_VAR(tx_data_1[12], FMSTR_TSA_UINT16)
        FMSTR_TSA_RO_VAR(tx_data_1[13], FMSTR_TSA_UINT16)
        FMSTR_TSA_RO_VAR(tx_data_1[14], FMSTR_TSA_UINT16)
        FMSTR_TSA_RO_VAR(tx_data_1[15], FMSTR_TSA_UINT16)
        FMSTR_TSA_RO_VAR(rx_data_4[0], FMSTR_TSA_UINT16)
        FMSTR_TSA_RO_VAR(protocol_state, FMSTR_TSA_USERTYPE(Fr_POC_state_type))
        FMSTR_TSA_RO_VAR(wakeup_status, FMSTR_TSA_USERTYPE(Fr_wakeup_state_type))
    FMSTR_TSA_TABLE_END()

    FMSTR_TSA_TABLE_LIST_BEGIN()
        FMSTR_TSA_TABLE(first_table)
    FMSTR_TSA_TABLE_LIST_END()

/* End of FreeMASTER TSA support */

/*********************************** Protypes ********************************/
void InitMCU(void);
void Failed(uint8 number);
void CC_interrupt_slot_1(uint8 buffer_idx);
void CC_interrupt_slot_4(uint8 buffer_idx);
void CC_interrupt_timer_1(void);
void CC_interrupt_timer_2(void);
void CC_interrupt_cycle_start(void);
void CC_interrupt_FIFO_A(uint16 header_idx);
void FLEXRAY_ISR(void);
void FREEMASTER_ISR(void);
/*********************************** Functions ******************************/
void Failed(uint8 number)               /* Function only for debugging */
{
    /* CC should be restarted */
    while(1);
}

/************************************************************************************
* Function name:    CC_interrupt_slot_1
* Description:      Interrupt routine, updates double transmit buffer with new data
*                   Increment variable in case that the buffer updating was successful
* Function arguments: 
*                   buffer_idx - the number of MB which generated the interrupt
*
* Return value:
*                  None 
*************************************************************************************/
void CC_interrupt_slot_1(uint8 buffer_idx)
{
    #ifdef LEDS_USED
       SIU.GPDO[53].B.PDO = 1; /* Set pin */
    #endif /* LEDS_USED */

    /* Update double transmit MB with new data (commit side) */
    tx_return_value = Fr_transmit_data(TX_SLOT_1, &tx_data_1[0], 16);
    
    /* It is necessary to clear the flag at tranmsit side */
    Fr_clear_MB_interrupt_flag(TX_SLOT_1_TRANSMIT_SIDE);
    
    if(tx_return_value == FR_TXMB_UPDATED)
    {
        tx_data_1[0]++;         /* Increment variable */
    }
    #ifdef LEDS_USED
       SIU.GPDO[53].B.PDO = 0; /* Clear pin */
    #endif /* LEDS_USED */
}

/************************************************************************************
* Function name:    CC_interrupt_slot_4
* Description:      Interrupt routine, copy received data into rx_data_4 data array and
*                   update the tx_data_1 array
*
* Function arguments: 
*                   buffer_idx - the number of MB which generated the interrupt
*
* Return value:
*                   None
*************************************************************************************/
void CC_interrupt_slot_4(uint8 buffer_idx)
{
    /* Copy received data into given array */
    rx_return_value = Fr_receive_data(buffer_idx, &rx_data_4[0], &rx_data_length, &rx_status_slot);
    tx_data_1[1] = rx_status_slot;  /* Store status slot */
    tx_data_1[2]++;                 /* Increment data array which should be transmitted in slot 1 */
}


/************************************************************************************
* Function name:    CC_interrupt_timer_1
* Description:      Interrupt routine, called when absolute timer 1 generates interrupt
*
* Function arguments: 
*                   None
*
* Return value:
*                   None
*************************************************************************************/
void CC_interrupt_timer_1(void)
{
    Fr_get_global_time(&current_cycle, &current_macrotick); /* Get the global time */
    tx_data_1[14] = current_macrotick;          /* Store current macrotick value */
    tx_data_1[15] = (uint16) (current_cycle);   /* Store current cycle value */
}

/************************************************************************************
* Function name:    CC_interrupt_timer_2
* Description:      Interrupt routine, called when relative timer 2 generates interrupt	
*
* Function arguments: 
*                   None
*
* Return value:
*                   None
*************************************************************************************/
void CC_interrupt_timer_2(void)
{
    Fr_get_global_time(&current_cycle, &current_macrotick); /* Get the global time */
    tx_data_1[12] = current_macrotick;          /* Store current macrotick value */
    tx_data_1[13] = (uint16) (current_cycle);   /* Store current cycle value */
}

/************************************************************************************
* Function name:    CC_interrupt_cycle_start
* Description:      Interrupt routine, called when cycle start interrupt is generated
*
* Function arguments: 
*                   None
*
* Return value:
*                   None
*************************************************************************************/
void CC_interrupt_cycle_start(void)
{
    #ifdef LEDS_USED
        SIU.GPDO[54].B.PDO = 1; /* Set pin */
    #endif /* LEDS_USED */

    Fr_get_global_time(&current_cycle, &current_macrotick); /* Get the global time */
    tx_data_1[11] = current_cycle;                          /* Store current cycle value */

    #ifdef LEDS_USED
       SIU.GPDO[54].B.PDO = 0; /* Clear pin */
    #endif /* LEDS_USED */
}

/************************************************************************************
* Function name:    CC_interrupt_FIFO_A
* Description:      Interrupt routine, called when FIFO A generates interrupt.
*                   Copy received data into fifo_data array	
*
* Function arguments: 
*                   None
*
* Return value:
*                   None
*************************************************************************************/
void CC_interrupt_FIFO_A(uint16 header_idx)
{

    #ifdef LEDS_USED
       SIU.GPDO[55].B.PDO = 1; /* Set pin */
    #endif /* LEDS_USED */

    /* Copy received data into given array */
    fifo_return_value = Fr_receive_fifo_data(header_idx, &fifo_data[0], &fifo_data_length, &fifo_slot_idx, &fifo_status_slot);
    
    if(fifo_return_value == FR_FIFO_RECEIVED)   /* Data has been received? */
    {                                           /* Yes */
        tx_data_1[3] = header_idx;          /* Store header index into data array - position 8 */
        tx_data_1[4] = fifo_data[0];        /* Store value of the first field into data array - position 9 */
        
        /* Update appropriate variables in case that a frame has been received */
        switch(fifo_slot_idx)
        {
        case 4:                             /* Slot 4 - fault */
            tx_data_1[5] = 0xF0F3;          /* Store 0xF0F3 value into data array - position 5 */
            break;
        case 5:                             /* Slot 5 - fault */
            tx_data_1[6] = 0xF0F2;          /* Store 0xF0F2 value into data array - position 6 */
            break;
        case 62:                            /* Slot 62 */
            tx_data_1[7]++;                 /* Increment variable in case that a frame has been received - position 7 */
            break;
        case 63:                            /* Slot 63 */
            tx_data_1[8]++;                 /* Increment variable in case that a frame has been received - position 8 */
            break;
        default:                            /* Another frame - fault */
            tx_data_1[9] = 0xF0F0;          /* Store 0xF0F0 value into data array - position 9 */
            break;
        }
    }
    else if(fifo_return_value == FR_FIFO_NOT_RECEIVED)  /* Data has not been received */
    {
        tx_data_1[10] = 0xFFFF;              /* Store 0xFFFF value to the transmit data array, position 10 */
    }
    
    #ifdef LEDS_USED
       SIU.GPDO[55].B.PDO = 0; /* Clear pin */
    #endif /* LEDS_USED */
}

/******************************************************************************
* Function name : main 
* Description   : Main function of the application 
*                 In this function the MPC560xP microcontroller peripherals and FlexRay module
*                 are initialized. All configured MBs are initialized, interrupts are enabled.
* Author        : R62779
* 
* Parameters    : None
*
* Returns       : None
*
******************************************************************************/
void main(void) 
{
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
    /* Set callback function in case that an interrupt from MB 0 occurs */
    Fr_set_MB_callback(&CC_interrupt_slot_1, TX_SLOT_1_TRANSMIT_SIDE);  /* Interrupt is enabled for transmit side of the double MB */

    /* Set callback function in case that an interrupt from MB 3 occurs */
    Fr_set_MB_callback(&CC_interrupt_slot_4, RX_SLOT_4);
   
    /* Initialization of the timers */
    Fr_timers_init(&Fr_timers_cfg_00_ptr[0]);

    /* Set callback function in case that an interrupt from timer 1 occurs */
    Fr_set_protocol_0_IRQ_callback(&CC_interrupt_timer_1, FR_TIMER_1_EXPIRED_IRQ);

    /* Set callback function in case that an interrupt from timer 2 occurs */
    Fr_set_protocol_0_IRQ_callback(&CC_interrupt_timer_2, FR_TIMER_2_EXPIRED_IRQ);

    /* Set callback function in case that cycle start interrupt occurs */
    Fr_set_protocol_0_IRQ_callback(&CC_interrupt_cycle_start, FR_CYCLE_START_IRQ);

    /* Set callback function in case that an Interrupt occurs */
    Fr_set_fifo_IRQ_callback(&CC_interrupt_FIFO_A, FR_FIFO_A_IRQ);

    /* Enable interrupts */
    asm("wrteei 1");
    
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
       
    protocol_state = Fr_get_POC_state();            /* Load the current POC state */
    while(Fr_get_POC_state() != FR_POCSTATE_READY); /* Wait till the FR CC is not in the FR_POCSTATE_READY */
    
    /* Initialize startup */
    return_value = Fr_start_communication();
    if(return_value == FR_NOT_SUCCESS) Failed(4);   /* Call debug function in case of any error */
    
    protocol_state = Fr_get_POC_state();            /* Load current POC state */
    while(Fr_get_POC_state() != FR_POCSTATE_NORMAL_ACTIVE) /* Wait till the FR CC is not in the FR_POCSTATE_NORMAL_ACTIVE */
{
    #ifdef FREEMASTER_USED
      FMSTR_Poll();
    #endif
    }
    
    protocol_state = Fr_get_POC_state();            /* Load the current POC state */

    /* The first initialization of the MB 0 */
    tx_return_value = Fr_transmit_data(TX_SLOT_1, &tx_data_1[0], 16);
    if(tx_return_value == FR_TXMB_NO_ACCESS) Failed(5);   /* Call debug function in case of any error */

    wakeup_status = Fr_get_wakeup_state();          /* Load current wakeup status */

    /* Enable appropriate interrupts */
    Fr_enable_interrupts((FR_PROTOCOL_IRQ | FR_FIFO_A_IRQ | FR_RECEIVE_IRQ | FR_TRANSMIT_IRQ), 
                    (FR_TIMER_1_EXPIRED_IRQ | FR_TIMER_2_EXPIRED_IRQ | FR_CYCLE_START_IRQ), 0);

    Fr_start_timer(FR_TIMER_T1);                    /* Start Timer T1 */
    Fr_start_timer(FR_TIMER_T2);                    /* Start Timer T2 */

    for(;;)
    {
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
        /*NOTE: LED1 cannot be used because it is connected to the same pin as FlexRay uses */
        SIU.PCR[53].B.PA = 0;          /* Set pin PB4 as an GPIO pin - LED2 */
        SIU.PCR[53].B.OBE = 1; 	       /* Output buffer enabled */
        SIU.GPDO[53].B.PDO = 1;        /* Turn off led*/ 
        SIU.PCR[54].B.PA = 0;          /* Set pin PB5 as an GPIO pin - LED3*/
        SIU.PCR[54].B.OBE = 1;         /* Output buffer enabled */
        SIU.GPDO[54].B.PDO = 1;        /* Turn off led*/
        SIU.PCR[55].B.PA = 0;          /* Set pin PB6 as an GPIO pin - LED4*/
        SIU.PCR[55].B.OBE = 1;         /* Output buffer enabled */
	SIU.GPDO[55].B.PDO = 1;        /* Turn off led*/
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

    /* Raise FlexRay interupt priority and install the interrupts */
    INTC_InstallINTCInterruptHandler(FLEXRAY_ISR, 133, 2);  /* FlexRay interrupts */
    INTC_InstallINTCInterruptHandler(FLEXRAY_ISR, 134, 2);
    INTC_InstallINTCInterruptHandler(FLEXRAY_ISR, 135, 2);
    INTC_InstallINTCInterruptHandler(FLEXRAY_ISR, 136, 2);
    INTC_InstallINTCInterruptHandler(FLEXRAY_ISR, 137, 2);
    INTC_InstallINTCInterruptHandler(FLEXRAY_ISR, 138, 2);
    INTC_InstallINTCInterruptHandler(FLEXRAY_ISR, 139, 2);
    INTC_InstallINTCInterruptHandler(FLEXRAY_ISR, 140, 2);

    /* Ensure INTC's current priority is 0 */
    INTC.CPR.B.PRI = 0;

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
        /* Raise LINFlex interupt priority and install the interrupts */
        INTC_InstallINTCInterruptHandler(FREEMASTER_ISR, 79, 3); /* SCI interrupt */
        INTC_InstallINTCInterruptHandler(FREEMASTER_ISR, 80, 3); /* SCI interrupt */
        /* Enable interupts in the LINFlex module */
        LINFLEX_0.LINIER.B.DRIE = 1;
        LINFLEX_0.LINIER.B.DTIE = 1;
        LINFLEX_0.LINIER.B.DBFIE = 1;		
        LINFLEX_0.LINIER.B.DBEIE = 1;		
        
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
        SIU.GPDO[53].B.PDO = 1; /* Set pin */
    #endif  /*LEDS_USED */

    Fr_interrupt_handler(); /* Call FlexRay driver interrupt service routine handler */

    #ifdef LEDS_USED
        SIU.GPDO[53].B.PDO = 0; /* Clear pin */
    #endif /* LEDS_USED */

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
