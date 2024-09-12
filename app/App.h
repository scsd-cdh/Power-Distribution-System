/*
 * App.h
 *
 *  Created on: Aug 17, 2024
 *      Author: Rowan
 */

#ifndef APP_APP_H_
#define APP_APP_H_

#define MAX_CMD_SIZE 255

#include <driverlib.h>



static uint8_t cmd_Read[MAX_CMD_SIZE] = {0};
static volatile uint8_t bytes_received = 0;
static volatile uint8_t command_length;


typedef enum
{
    idle,
    system_status,
    health_check,
    reboot,
    telecom_acknowledge,
    converter_monitor,
    error_process
} appStatus;

enum error_codes
{
    too_many_bytes = 0x00,
    too_early_stop_condition = 0x01,
    too_few_bytes = 0x02
};

enum Command
{
    SystemStatus = 0x01,
    HealthCheck = 0x02,
    Reboot = 0x03,
    ConverterMonitor = 0x04,
    TelecomAcknowledge = 0x05,
    badCommand = 0x06
};


enum command_Size{
    SystemStatus_size = 0x02,
    HealthCheck_size = 0x02,
    Reboot_size = 0x02,
    ConverterMonitor_size = 0x04
};

void init_App();

void initializeI2C();
void suspendI2CInterrupts();
void resumeI2CInterrupts();
void commandHandler(uint8_t status);

void run();


#endif /* APP_APP_H_ */
