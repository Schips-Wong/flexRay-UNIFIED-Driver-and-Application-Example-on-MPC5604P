/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2006 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***************************************************************************//*!
*
* @file      Fr_UNIFIED_cfg.c
*
* @author    R62779
* 
* @version   1.0.1.0
* 
* @date      Nov-28-2008
* 
* @brief     FlexRay High-Level Driver implementation.
*            This file contains structures with configuration data. An user may
*            modify these structures.
*
******************************************************************************/

#include "Fr_UNIFIED.h"

/******************************************************************************
* Global variables
******************************************************************************/

/* Hardware configuration structure */
/* Number of MB in Segment 1:   11 */
/* Number of MB in Segment 2:   8 */
/* FIFO Depth:                  0 (not configured) */
const Fr_HW_config_type Fr_HW_cfg_00 =
{
    0xFFFE0000,     /* FlexRay module base address */
    0x40004000,     /* FlexRay memory base address (MB headers start at this address) */
    FR_MPC560xP,    /* Type of Freescale FlexRay module */
    FALSE,          /* Synchronization filtering */
    FR_INTERNAL_SYSTEM_BUS_CLOCK,
    //FR_EXTERNAL_OSCILLATOR,
    0,              /* Value of the PRESCALE or BITRATE bit field in the MCR register */
    16,             /* Data size - segment 1 */
    8,              /* Data size - segment 2 */
    10,             /* Last MB in segment 1 (Number of MB in Segment1 - 1) */
    18,             /* Last individual MB (except FIFO); (Number of MB in Segment1 + Number of MB in Segment2 - 1) */
    19,             /* Total number of used MB (Last_individual_MB + 1 + FIFO) */
    TRUE,           /* Allow coldstart */
    0,              /* The value of the TIMEOUT bit field in the SYMATOR register - not implemented for all FlexRay modules */
    0,              /* Offset of the Sync Frame Table in the FlexRay memory */
    FR_DUAL_CHANNEL_MODE    /* Single channel mode disabled */
};

/* Transmit MB configuration structure */
/* Slot 4, payload length 16 Words, Single buffered MB, State transmission mode,  */
/* interrupt disabled, channel AB, filtering disabled */
const Fr_transmit_buffer_config_type Fr_tx_buffer_slot_04_cfg =
{
   4,                           /* Transmit frame ID */
   1747,                        /* Header CRC */
   16,                          /* Payload length */
   FR_SINGLE_TRANSMIT_BUFFER,   /* Transmit MB buffering */
   FR_STATE_TRANSMISSION_MODE,  /* Transmission mode */
   FR_STREAMING_COMMIT_MODE,    /* Transmission commit mode */
   FR_CHANNEL_AB,               /* Transmit channels */
   FALSE,                       /* Payload preamble */
   FALSE,                       /* Transmit cycle counter filter enable */
   0,                           /* Transmit cycle counter filter value */
   0,                           /* Transmit cycle counter filter mask */
   FALSE,                        /* Transmit MB interrupt enable */
   FALSE                        /* Used only for double buffer */
};

/* Transmit MB configuration structure */
/* Slot 5, payload length 16 Words, Single buffered MB, State transmission mode, interrupt enabled, channel AB */
const Fr_transmit_buffer_config_type Fr_tx_buffer_slot_05_cfg =
{
   5,                           /* Transmit frame ID */
   5,                           /* Header CRC */
   16,                          /* Payload length */
   FR_DOUBLE_TRANSMIT_BUFFER,   /* Transmit MB buffering */
   FR_STATE_TRANSMISSION_MODE,  /* Transmission mode */
   FR_STREAMING_COMMIT_MODE,    /* Transmission commit mode */
   FR_CHANNEL_AB,               /* Transmit channels */
   FALSE,                       /* Payload preamble */
   FALSE,                       /* Transmit cycle counter filter enable */
   0,                           /* Transmit cycle counter filter value */
   0,                           /* Transmit cycle counter filter mask */
   FALSE,                       /* Transmit MB interrupt enable */
   FALSE                        /* FALSE - interrupt is enabled at commit side, TRUE - interrupt is enabled at transmit side */
};

/* Receive MB configuration structure */
/* Slot 1, channel A, filtering disabled, interrupt enabled */
const Fr_receive_buffer_config_type  Fr_rx_buffer_slot_01_cfg =
{
   1,                           /* Receive frame ID */
   FR_CHANNEL_A,                /* Receive channel enable */
   FALSE,                       /* Receive cycle counter filter enable */
   0,                           /* Receive cycle counter filter value */
   0,                           /* Receive cycle counter filter mask */
   FALSE                         /* Receive MB interrupt enable */
};

/* Receive MB configuration structure */
/* This MB is not configured in application */
const Fr_receive_buffer_config_type  Fr_rx_buffer_slot_07_cfg =
{
   2,                           /* Receive frame ID */
   FR_CHANNEL_A,                /* Receive channel enable */
   TRUE,                        /* Receive cycle counter filter enable */
   0,                           /* Receive cycle counter filter value */
   0,                           /* Receive cycle counter filter mask */
   FALSE                        /* Receive MB interrupt enable */
};

/* Transmit MB configuration structure */
/* Slot 62, payload length 8 Words, Single buffered MB, Event transmission mode, interrupt disabled, channel A */
const Fr_transmit_buffer_config_type Fr_tx_buffer_slot_62_cfg =
{
   62,                          /* Transmit frame ID */
   0x03BF,                      /* Header CRC */
   8,                           /* Payload length */
   FR_SINGLE_TRANSMIT_BUFFER,   /* Transmit MB buffering */
   FR_EVENT_TRANSMISSION_MODE,  /* Transmission mode */
   FR_IMMEDIATE_COMMIT_MODE,    /* Transmission commit mode */
   FR_CHANNEL_A,                /* Transmit channels */
   FALSE,                       /* Payload preamble */
   FALSE,                       /* Transmit cycle counter filter enable */
   0,                           /* Transmit cycle counter filter value */
   0,                           /* Transmit cycle counter filter mask */
   FALSE,                       /* Transmit MB interrupt enable */
   FALSE                        /* Used only for double buffer */
};

/* Transmit MB configuration structure */
/* Slot 63, payload length 8 Words, Single buffered MB, Event transmission mode, interrupt disabled, channel A */
const Fr_transmit_buffer_config_type Fr_tx_buffer_slot_63_cfg =
{
   63,                          /* Transmit frame ID */
   0x024A,                      /* Header CRC */
   8,                           /* Payload length */
   FR_SINGLE_TRANSMIT_BUFFER,   /* Transmit MB buffering */
   FR_EVENT_TRANSMISSION_MODE,  /* Transmission mode */
   FR_IMMEDIATE_COMMIT_MODE,    /* Transmission commit mode */
   FR_CHANNEL_A,                /* Transmit channels */
   FALSE,                       /* Payload preamble */
   FALSE,                       /* Transmit cycle counter filter enable */
   0,                           /* Transmit cycle counter filter value */
   0,                           /* Transmit cycle counter filter mask */
   FALSE,                       /* Transmit MB interrupt enable */
   FALSE                        /* Used only for double buffer */
};


/* FIFO configuration structure */
/* FIFO A, Depth 10, Data payload 16, receive all frames */
const Fr_FIFO_config_type Fr_FIFOA_cfg =
{
    FR_CHANNEL_A,                       /* FIFO channel */
    10,                                 /* FIFO depth */
    16,                                 /* FIFO entry size */
    0,                                  /* FIFO message ID acceptance filter value */
    0,                                  /* FIFO message ID acceptance filter mask */
    0,                                  /* FIFO frame ID rejection filter value */
    0x07FF,                             /* FIFO frame ID rejection filter mask */
    {                                   /* Range filters (RF) configuration */
        {FALSE, FR_ACCEPTANCE, 0, 0},   /* 1st RF - RF enable, RF mode, RF lower interval, RF upper interval */
        {FALSE, FR_ACCEPTANCE, 0, 0},   /* 2nd RF - RF enable, RF mode, RF lower interval, RF upper interval */
        {FALSE, FR_ACCEPTANCE, 0, 0},   /* 3rd RF - RF enable, RF mode, RF lower interval, RF upper interval */
        {FALSE, FR_ACCEPTANCE, 0, 0},   /* 4th RF - RF enable, RF mode, RF lower interval, RF upper interval */
    },
    FALSE                               /* FIFO interrupt enable, can be enabled also by Fr_enable_interrupts() function */
};

/* Configuration data for Shadow Message Buffers */
const Fr_receive_shadow_buffers_config_type Fr_rx_shadow_cfg =
{
    TRUE,       /* Rx shadow buffer for channel A, seg 1 - enabled? */
    TRUE,       /* Rx shadow buffer for channel A, seg 2 - enabled? */
    TRUE,       /* Rx shadow buffer for channel B, seg 1 - enabled? */
    TRUE,       /* Rx shadow buffer for channel B, seg 2 - enabled? */
    8,          /* Ch A, seg 1 - the current index of the MB header field */
    17,         /* Ch A, seg 2 - the current index of the MB header field */
    9,          /* Ch B, seg 1 - the current index of the MB header field */
    18          /* Ch B, seg 2 - the current index of the MB header field */
};


/* Following array is used to determine which message buffers defined in Fr_buffer_cfg_xx structure */
/* will be used for the FlexRay CC configuration */
const Fr_index_selector_type Fr_buffer_cfg_set_00[] =
{
    0, 1, 3, 5, 6, 7, FR_LAST_MB
};

/* Array of structures with message buffer configuration information */
/* The MBs 6 and FIFO A will not be configured */
const Fr_buffer_info_type Fr_buffer_cfg_00[] =
{ /* Buffer type         Configuration structure ptr    MB index       xx = configuration index used by Fr_buffer_cfg_set_xx */
    {FR_TRANSMIT_BUFFER, &Fr_tx_buffer_slot_04_cfg,     1},         /* 00 */
    {FR_RECEIVE_BUFFER,  &Fr_rx_buffer_slot_01_cfg,     2},         /* 01 */
    {FR_RECEIVE_BUFFER,  &Fr_rx_buffer_slot_07_cfg,     6},         /* 02     */
    {FR_TRANSMIT_BUFFER, &Fr_tx_buffer_slot_05_cfg,     4},         /* 03     */
    {FR_RECEIVE_FIFO,    &Fr_FIFOA_cfg,                 19},        /* 04 */
    {FR_RECEIVE_SHADOW,  &Fr_rx_shadow_cfg,             0},         /* 05 */
    {FR_TRANSMIT_BUFFER, &Fr_tx_buffer_slot_62_cfg,     11},        /* 06 */
    {FR_TRANSMIT_BUFFER, &Fr_tx_buffer_slot_63_cfg,     12},         /* 07 */
};

/* Structure of this type contains configuration
   information of the one low level parameters set */
const Fr_low_level_config_type Fr_low_level_cfg_set_00 =
{
    10,         /* G_COLD_START_ATTEMPTS */
    3,          /* GD_ACTION_POINT_OFFSET */
    83,         /* GD_CAS_RX_LOW_MAX */
    0,          /* GD_DYNAMIC_SLOT_IDLE_PHASE */
    40,         /* GD_MINISLOT */
    3,          /* GD_MINI_SLOT_ACTION_POINT_OFFSET */
    50,         /* GD_STATIC_SLOT */
    13,         /* GD_SYMBOL_WINDOW */
    11,         /* GD_TSS_TRANSMITTER */
    59,         /* GD_WAKEUP_SYMBOL_RX_IDLE */
    50,         /* GD_WAKEUP_SYMBOL_RX_LOW */
    301,        /* GD_WAKEUP_SYMBOL_RX_WINDOW */
    180,        /* GD_WAKEUP_SYMBOL_TX_IDLE */
    60,         /* GD_WAKEUP_SYMBOL_TX_LOW */
    2,          /* G_LISTEN_NOISE */
    5000,       /* G_MACRO_PER_CYCLE */
    10,         /* G_MAX_WITHOUT_CLOCK_CORRECTION_PASSIVE */
    14,         /* G_MAX_WITHOUT_CLOCK_CORRECTION_FATAL */
    22,         /* G_NUMBER_OF_MINISLOTS */
    60,         /* G_NUMBER_OF_STATIC_SLOTS */
    4920,       /* G_OFFSET_CORRECTION_START */
    16,         /* G_PAYLOAD_LENGTH_STATIC */
    5,          /* G_SYNC_NODE_MAX */
    2,          /* G_NETWORK_MANAGEMENT_VECTOR_LENGTH */
    FALSE,      /* G_ALLOW_HALT_DUE_TO_CLOCK */
    20,         /* G_ALLOW_PASSIVE_TO_ACTIVE */
    FR_CHANNEL_AB,  /* P_CHANNELS */
    300,        /* PD_ACCEPTED_STARTUP_RANGE */
    1,          /* P_CLUSTER_DRIFT_DAMPING */
    56,         /* P_DECODING_CORRECTION */
    1,          /* P_DELAY_COMPENSATION_A */
    1,          /* P_DELAY_COMPENSATION_B */
    401202,     /* PD_LISTEN_TIMEOUT */
    601,        /* PD_MAX_DRIFT */
    0,          /* P_EXTERN_OFFSET_CORRECTION */
    0,          /* P_EXTERN_RATE_CORRECTION */
    4,          /* P_KEY_SLOT_ID */
    TRUE,       /* P_KEY_SLOT_USED_FOR_STARTUP */
    TRUE,       /* P_KEY_SLOT_USED_FOR_SYNC */
    1747,       /* P_KEY_SLOT_HEADER_CRC */
    21,         /* P_LATEST_TX */
    5,          /* P_MACRO_INITIAL_OFFSET_A */
    5,          /* P_MACRO_INITIAL_OFFSET_B */
    23,         /* P_MICRO_INITIAL_OFFSET_A */
    23,         /* P_MICRO_INITIAL_OFFSET_B */
    200000,     /* P_MICRO_PER_CYCLE */
    1201,       /* P_OFFSET_CORRECTION_OUT */
    600,        /* P_RATE_CORRECTION_OUT */
    FALSE,      /* P_SINGLE_SLOT_ENABLED */
    FR_CHANNEL_A,   /* P_WAKEUP_CHANNEL */
    16,         /* P_WAKEUP_PATTERN */
    40,         /* P_MICRO_PER_MACRO_NOM */
    8           /* P_PAYLOAD_LENGTH_DYN_MAX */
};
