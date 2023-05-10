                                                    The FlexRay UNIFIED Driver
-----------------------------------------------------------------------------------------------------------------------------------

                                                            RELEASE NOTES
-----------------------------------------------------------------------------------------------------------------------------------
Last update: 26-Jan-2010


For more information on Driver package installation and the application examples description, 
please see the 'UNIFIED_application_examples_user_guide.pdf' and the 'UNIFIED_Driver_User_Guide.pdf' documents.

Available Versions:
-----------------------------------------------------------------------------------------------------------------------------------
1.3.8 (Fr_UNIFIED.c file version v1.0.90.0)
    - Fr_UNIFIED.c - Code cleaning done, FR_REG_FAR macro changed to PTR2FARREG and macro FR_DATA_FAR changed to PTR2FARDATA
                     in order to support Cosmic compiler for S12X platform.    
    - UNIFIED_Driver_User_Guide.pdf - CodeWarrior compiler v2.5 for MPC560xP added
    - UNIFIED_application_examples_user_guide.pdf - CodeWarrior compiler v2.5 for MPC560xP added
1.3.7   (Fr_UNIFIED.c file version v1.0.88.0)
    Fr_UNIFIED.c file modified - functions modified 
					Fr_receive_data() - Interrupt flag clearing in case of NULL frame or invalid frame reception added
    - UNIFIED_Driver_User_Guide.pdf - GreenHills compiler version 5.1.6 added
    - UNIFIED_application_examples_user_guide.pdf - GreenHills compiler version 5.1.6 added
1.3.6   (Fr_UNIFIED.c file version v1.0.85.0)
    - The application examples updated for the xPC560xP Microcontroller
    Fr_UNIFIED.c file modified - functions modified 
					Fr_check_rx_status() - MBIF flag is checked instead DUP flag in the dynamic segment
					Fr_receive_data() - MBIF flag is checked instead DUP flag in the dynamic segment
	- transmit_receive_node2.c - in funciton main() order of tasks changed 
    - UNIFIED_Driver_User_Guide.pdf - target device modified
    - UNIFIED_application_examples_user_guide.pdf - New chapters 2.16 and 4.14 added
                                                  - Minor mistakes and typos correction
    - FlexRay_UNIFIED_Driver_Applications_getting_started.pdf - xPC560xP added

1.3.5	(Fr_UNIFIED.c file version v1.0.84.0)
    - The application examples updated for the MPC5517 Microcontroller
    - Fr_UNIFIED.c file modified - functions Fr_get_wakeup_state(), Fr_get_POC_state(), Fr_send_MTS()
					Fr_get_MTS_state(), Fr_transmit_data(),	Fr_set_global_IRQ_callback(),	Fr_set_protocol_0_IRQ_callback()
					Fr_set_protocol_1_IRQ_callback(),	Fr_set_fifo_IRQ_callback(),	Fr_check_CHI_error(),	Fr_check_cycle_start()
					Fr_check_transmission_across_boundary(), Fr_check_violation(), Fr_check_max_sync_frame(),
          Fr_check_clock_correction_limit_reached(), Fr_check_missing_offset_correction(), Fr_check_missing_rate_correction()
					Fr_check_coldstart_abort(),	Fr_check_internal_protocol_error(),	Fr_check_fatal_protocol_error(),
          Fr_check_protocol_state_changed(), Fr_check_protocol_engine_com_failure() modified to have only one return point
			  - minor modification of function Fr_timers_init()
				- functions Fr_enter_configuration_mode(), Fr_reset_protocol_engine() modified to reflect 
          changes in type definition of Fr_connected_HW_type in file Fr_UNIFIED.h 
				 - constant signed values replaced by unsigned to remove MISRA violations
    - Fr_UNIFIED.h file modified - a new value FR_POCSTATE_UNKNOWN in the type Fr_POC_state_type
                                 - a new value FR_MTS_UNKNOWN in the type Fr_MTS_state_type
                                 - the type Fr_connected_HW_type modification (FR_MPC5514, FR_MPC5516, FR_MPC5517
                                                                   replaced by FR_MPC5510_0M22M and FR_MPC5510_0M76F) 
    - UNIFIED_Driver_User_Guide.pdf - changed to reflect changes in Fr_UNIFIED.h
    - UNIFIED_application_examples_user_guide.pdf - New chapters 2.15 and 4.13 added
                                                  - Chapter Chapter 3.1 Application Examples Memory Requirements and Performance updated
    - FlexRay_UNIFIED_Driver_Applications_getting_started.pdf - MPC5517 added

1.3.4   (Fr_UNIFIED.c file version v1.0.80.0)
    - The application examples for the MPC5567 Microcontroller updated for the CodeWarrior compiler v2.3
    - Fr_UNIFIED.c file modified - the Fr_reset_protocol_engine() and Fr_enter_configuration_mode() routines modified
                                   to correspond with the MPC5517 and xPC560xP FlexRay device changes in Protocol Reset Command
				 - comments type changed
    - Fr_UNIFIED.h file modified - the type Fr_connected_HW_type modified - the MPC5517 and xPC560xP FlexRay modules added
    - UNIFIED_application_examples_user_guide.pdf   - New Chapters 2.2, and 4.2 added
                                                    - Chapter 3.1 Application Examples Memory Requirements and Performance updated
