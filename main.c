/*******************************************************************************
 * File Name:   main.c
 *
 * Description: This is the source code for the PSRAM XIP Example
 *              for using Flash and PSRAM together.
 *
 * Related Document: See README.md
 *
 *
 *******************************************************************************
 * Copyright 2021-2024, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 *******************************************************************************/

/*******************************************************************************
 * Header Files
 *******************************************************************************/
 #include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "cyhal_syspm.h"
#include <stdint.h>
#include <stdio.h>

/*******************************************************************************
 * Macros
 *******************************************************************************/


/*******************************************************************************
 * Global Variables
 *******************************************************************************/

 /* Global array will be stored in PSRAM irrespective of the app Execution source */
 __attribute__((section(".cy_psram_data")))
volatile uint32_t psram_data_array[5] = {0xa5, 0x5a, 0xb6, 0x6b, 0xaa};

/*Global array will be stored in Flash irrespective of the app Execution source*/
__attribute__((section(".cy_xip_data")))
volatile uint32_t flash_data_array[5] = {0xa5, 0x5a, 0xb6, 0x6b, 0xaa};

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/
void print_address_fn_psram(int (*ptr_to_fn)());

/*******************************************************************************
 * Function Definitions
 *******************************************************************************/

/*******************************************************************************
 * Function Name: main()
 ********************************************************************************
 * Summary:
 * This is the main function for CPU. It...
 *    1. Initializes the BSP
 *    2. Enables Global interrupt
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    cyhal_syspm_lock_deepsleep();
    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);

    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        printf("retarget-io init failed");
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
#if defined(APPEXEC_RAM)
    printf("\r\n*******Running application from RAM *******\r\n");
#elif defined(APPEXEC_PSRAM)
    printf("\r\n*******Running application from PSRAM in eXecute-In-Place(XIP) mode *******\r\n");
#else
    printf("\r\n*******Running application from External Flash in eXecute-In-Place(XIP) mode *******\r\n");
#endif

    print_address_fn_psram(main);

    /*Print array addresses*/
    printf("Address of psram_data_array: %p \r\n",psram_data_array);
    printf("Address of flash_data_array: %p \r\n",flash_data_array);

    return 0;
}

/*******************************************************************************
 * Function Definitions
 *******************************************************************************/

/*******************************************************************************
 * Function Name: print_address_fn_psram()
 ********************************************************************************
 * Summary:
 * This is the function that prints the address of functions.
 *
 * Parameters:
 *  int (*ptr_to_fn)() : Pointer to a function that takes no parameters and returns
 *                       integer.
 *
 * Return:
 *  void
 *
 *******************************************************************************/

__attribute__((section(".cy_psram_func")))
void print_address_fn_psram(int (*ptr_to_fn)())
{
    uint32_t local_variable=0;
    /* The least significant bit in function address indicates whether the function contains Thumb instructions or not 
    and hence should not be considered while printing function address hence subtracting 1 from the address before printing.
    From ARM: "If you have a Thumb function, that is a function consisting of Thumb code, and that runs in Thumb state,
     then any pointer to that function must have the least significant bit set."
    */
    /*Print Function addresses*/
    printf("Address of main : %p \r\n",((void *)ptr_to_fn) -1);
    printf("Address of print_address_fn_psram : %p \r\n",((void *)print_address_fn_psram)-1);

    /*Print local variable address*/
    printf("Address of local_variable: %p \r\n",&local_variable);

}
/* [] END OF FILE */
