/*
 * App.c
 *
 *  Created on: Aug 17, 2024
 *      Author: Rowan
 */

#include <app/App.h>

#define SLAVE_ADDRESS 0x08


///////////////////////////////////////////////////////////////////////////change me later
#define Stop_Condition 0x13

static volatile appStatus app;
static volatile error_codes error;

//static volatile bool commandReceived = 0;  // Boolean

void init_App(){
    app = idle;
    initializeI2C();
}

void initializeI2C()
{

    // Configure Pins for I2C
    //Set P1.6 and P1.7 as Secondary Module Function Input.
    /*

    * Select Port 1
    * Set Pin 6, 7 to input Secondary Module Function, (UCB0SIMO/UCB0SDA, UCB0SOMI/UCB0SCL).
    */
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P1,
        GPIO_PIN6 + GPIO_PIN7,
        GPIO_SECONDARY_MODULE_FUNCTION
    );

    // eUSCI configuration
    EUSCI_B_I2C_initSlaveParam param = {0};
    param.slaveAddress = SLAVE_ADDRESS;
    param.slaveAddressOffset = EUSCI_B_I2C_OWN_ADDRESS_OFFSET0;
    param.slaveOwnAddressEnable = EUSCI_B_I2C_OWN_ADDRESS_ENABLE;
    EUSCI_B_I2C_initSlave(EUSCI_B0_BASE, &param);

    EUSCI_B_I2C_enable(EUSCI_B0_BASE);

    EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE,
                EUSCI_B_I2C_RECEIVE_INTERRUPT0
                );

    EUSCI_B_I2C_enableInterrupt(EUSCI_B0_BASE,
                EUSCI_B_I2C_RECEIVE_INTERRUPT0
                );

    //Enter low power mode with interrupts enabled
    __bis_SR_register(CPUOFF + GIE);
    __no_operation();
}


void suspendI2CInterrupts() {
    EUSCI_B_I2C_disableInterrupt(EUSCI_B0_BASE,
         EUSCI_B_I2C_RECEIVE_INTERRUPT0
         );
}

void resumeI2CInterrupts() {
    EUSCI_B_I2C_enableInterrupt(EUSCI_B0_BASE,
        EUSCI_B_I2C_RECEIVE_INTERRUPT0
        );
    __bis_SR_register(CPUOFF + GIE); // Enter LPM with interrupts
}



void commandHandler(uint8_t cmd) {

    switch(cmd){

        case SystemStatus:
            app = system_status;
            break;
        case HealthCheck:
            app = health_check;
            break;
        case Reboot:
            app = reboot;
            break;
        case ConverterMonitor:
            app = converter_monitor;
            break;
        case TelecomAcknowledge:
            app = telecom_acknowledge;
            break;

        default: // Unrecognized command
            app =
            break;
    }

}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_B0_VECTOR)))
#endif
void USCIB0_ISR(void)
{
    switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
    {
        case USCI_NONE:             // No interrupts break;
            break;
        case USCI_I2C_UCALIFG:      // Arbitration lost
            break;
        case USCI_I2C_UCNACKIFG:    // NAK received (master only)
            break;
        case USCI_I2C_UCSTTIFG:     // START condition detected with own address (slave mode only)
            break;
        case USCI_I2C_UCSTPIFG:     // STOP condition detected (master & slave mode)
            // Based on current state get payload size
            // Write from sned_payload buffer until payload size is reached
            // Reset counter
            break;
        case USCI_I2C_UCRXIFG3:     // RXIFG3
            break;
        case USCI_I2C_UCTXIFG3:     // TXIFG3
            break;
        case USCI_I2C_UCRXIFG2:     // RXIFG2
            break;
        case USCI_I2C_UCTXIFG2:     // TXIFG2
            break;
        case USCI_I2C_UCRXIFG1:     // RXIFG1
            break;
        case USCI_I2C_UCTXIFG1:     // TXIFG1
            break;
        case USCI_I2C_UCRXIFG0:     // RXIFG0
            // Read byte from master
            /*
            if (commandReceived) {
                // set byte at get_payload_pointer to the byte read
            } else {
                // If no command has been recevied, assume command byte
                commandID = EUSCI_B_I2C_slaveGetData(EUSCI_B0_BASE);
                commandReceived = 1;  // Set command as received
                // Pass to command handler
                //suspendI2CInterrupts(); // Suspend interrupts during command handling
                __bic_SR_register_on_exit(CPUOFF); // Exit LPM
                commandHandler();
            }*/


            cmd_Read[bytes_received] = EUSCI_B_I2C_slaveGetData(EUSCI_B0_BASE);

            if(bytes_received == 0){
                get_command_length();
            }

            //
            if(cmd_Read[bytes_received] == Stop_Condition && command_length == bytes_received){
                suspendI2CInterrupts();
                command_proccess(cmd_Read[bytes_received]);
                bytes_received=0;
            }
            else if(cmd_Read[bytes_received] == Stop_Condition){
                //write me later
                //too early stop condition
                command_process(badCommand);
                bytes_received=0;
                suspendI2CInterrupts();
            }
            else if(command_length == bytes_received){
                //write me later
                //too many bytes
                command_process(badCommand + 1);
                bytes_received=0;
                suspendI2CInterrupts();
            }
            else{
                bytes_received++;
            }
            /*
             * there is gonna be another case where it doesn't give required bytes and stop condition
             * it needs to be checked with timer
             */



            break;
        case USCI_I2C_UCTXIFG0:     // TXIFG0
            break;
        case USCI_I2C_UCBCNTIFG:    // Byte count limit reached (UCBxTBCNT)
            break;
        case USCI_I2C_UCCLTOIFG:    // Clock low timeout - clock held low too long
            break;
        case USCI_I2C_UCBIT9IFG:    // Generated on 9th bit of a transmit (for debugging)
            break;
        default:
            break;
    }
}


void run(){

    switch(app){
        /* Idle state */
        case idle:
            //commandReceived = 0; // Reset whether command was received
            resumeI2CInterrupts(); // Resume interrupts after executing command
            break;
        /* Modes used for each command */
        case system_status:
            // do something
            GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);  // Green LED on
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
            app = idle;
            break;
        case health_check:
            // do something
            GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);  // Green LED off
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
            GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6);  // Red LED on
            GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6);
            app = idle;
            break;
        case reboot:
            // do something
            app = idle;
            break;
        case converter_monitor:
            // do something
            GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6);  // Red and Green LEDs on
            GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6);
            GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
            app = idle;
            break;
        case telecom_acknowledge:
            // do something
            GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6);  // Flash both LEDs
            GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN6);
            //GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN6);
            GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
            GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
            //GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
            app = idle;
            break;
    }

}
