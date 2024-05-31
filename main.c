/*******************************************************************************
 * File Name:   main.c
 *
 * Description: This is the source code for the Empty Application Example
 *              to be used as starting template.
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

/*******************************************************************************
 * Macros
 *******************************************************************************/

#define ADDRESS_SIZE (4u)         /* Memory address size */
#define NUM_BYTES_PER_LINE (16u)  /* Number of bytes per line */
#define BUFFER_SIZE (64u)         /* Size of buffer for read-write*/
#define PSRAM_ADDRESS (0x2800000) /* PSRAM test address */
#define TEST_DATA_1 (0xA5A5A5A5)  /* Test data 1 */
#define TEST_DATA_2 (0x5A5A5A5A)  /* Test data 2 */

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
cy_smif_psram_device_cfg_t psramCfg =
    {
        .readIdCmd = 0x9F,
        .manufId = 0x0D,
        .knownGoodDie = 0x5D,
        .quadReadCmd = 0xEB,
        .quadWriteCmd = 0x38,
        .smifParams.selectHoldDelay = 0x01,
        .smifParams.subPageNr = 0x01,
};

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/
static void check_status(char *message, uint32_t status);
static void print_array(char *message, uint8_t *buf, uint32_t size);
static cy_rslt_t memory_test(uint32_t test_data, uint32_t start_addr);

/*******************************************************************************
 * Function Definitions
 *******************************************************************************/
/**
 *
 * This structure specifies SMIF parameters for PSRAM device
 *
 */

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

    printf("\r\n*******PSRAM Read and Write in eXecute-In-Place(XIP) mode *******\r\n");

    result = thread_ap_smif_psram_Init(&psramCfg);
    if (result != TRUE)
    {
        printf("PSRAM init failed");
        CY_ASSERT(0);
    }

    /* Perform read-write on PSRAM using test data */
    result = memory_test(TEST_DATA_1, PSRAM_ADDRESS);
    check_status("PSRAM read-write test failed", result);

    /* Perform read-write on PSRAM using test data */
    result = memory_test(TEST_DATA_2, PSRAM_ADDRESS);
    check_status("PSRAM read-write test failed", result);

    printf("\r\nPSRAM read-write successful.\n\r");

    return 0;
}

/******************************************************************************
 * Function Name: print_array
 *******************************************************************************
 * Summary:
 *  Prints the content of the buffer to the UART console.
 *
 * Parameters:
 *  message - message to print before array output
 *  buf - buffer to print on the console.
 *  size - size of the buffer.
 *
 * Return:
 *  void
 *
 ******************************************************************************/
static void print_array(char *message, uint8_t *buf, uint32_t size)
{
    printf("\r\n%s (%u bytes):\r\n", message, (unsigned int)size);
    printf("-------------------------\r\n");

    for (uint32_t index = 0; index < size; index++)
    {
        printf("0x%02X ", buf[index]);

        if (0u == ((index + 1) % NUM_BYTES_PER_LINE))
        {
            printf("\r\n");
        }
    }
}

/*******************************************************************************
 * Function Name: check_status
 ********************************************************************************
 * Summary:
 *  Prints the message, indicates the non-zero status by turning the LED on, and
 *  asserts the non-zero status.
 *
 * Parameters:
 *  message - message to print if status is non-zero.
 *  status - status for evaluation.
 *
 * Return:
 *  void
 *
 *******************************************************************************/
static void check_status(char *message, uint32_t status)
{
    if (status)
    {
        printf("\n\r====================================================\n\r");
        printf("\n\rFAIL: %s\n\r", message);
        printf("Error Code: 0x%x\n\r", (int)status);
        printf("\n\r=====================================================\n\r");
        /* On failure, turn the LED ON */
        cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_ON);
        while (true)
            ; /* Wait forever here when error occurs. */
    }
}

/******************************************************************************
 * Function Name: memory_test
 *******************************************************************************
 * Summary:
 *  Test read and write to PSRAM by writing the test_data of the BUFFER_SIZE to
 *  start_addr
 *
 * Parameters:
 *  test_data: 32-bit data for read and write.
 *  start_addr: memory address where you need to read-write test data.
 *
 * Return:
 *  cy_rslt_t - status of memory test is 0 if passed. otherwise failed.
 *
 ******************************************************************************/
static cy_rslt_t memory_test(uint32_t test_data, uint32_t start_addr)
{
    uint32_t *location_ptr = (uint32_t *)start_addr;
    cy_rslt_t status = 0;

    printf("\r\nWriting data 0x%X to %d bytes memory starting from 0x%X\n\r",
           (int)test_data, BUFFER_SIZE, (int)start_addr);

    for (int i = 0; i < BUFFER_SIZE / 4; i++)
    {
        *location_ptr = test_data;
        location_ptr++;
    }

    location_ptr = (uint32_t *)start_addr;

    printf("\r\nReading from memory at 0x%X\n\r", (int)start_addr);

    /* Print the read data in array*/
    print_array("Read Data", (uint8_t *)location_ptr, BUFFER_SIZE);

    /* Check read data matches with written data */
    for (int i = 0; i < BUFFER_SIZE / 4; i++)
    {
        if (*location_ptr != test_data)
        {
            printf("\n\rRead data doesn't match written data at 0%X\n\r",
                   (int)(start_addr + (i * 4)));
            status++;
        }
        location_ptr++;
    }

    if (status)
        printf("PSRAM Test failed for %d address location(s).\n\r", (int)status);

    return status;
}

/* [] END OF FILE */
