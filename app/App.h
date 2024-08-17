/*
 * App.h
 *
 *  Created on: Aug 17, 2024
 *      Author: Rowan
 */

#ifndef APP_APP_H_
#define APP_APP_H_


typedef enum
{
    idle,
    system_status,
    health_check,
    reboot,
    telecom_acknowledge,
    converter_monitor,
} appStatus;

enum Command
{
    SystemStatus = 0x01,
    HealthCheck = 0x02,
    Reboot = 0x03,
    ConverterMonitor = 0x04,
    TelecomAcknowledge = 0x05,
};

void init_App();

void initializeI2C();
void suspendI2CInterrupts();
void resumeI2CInterrupts();
void commandHandler();

void run();


#endif /* APP_APP_H_ */
