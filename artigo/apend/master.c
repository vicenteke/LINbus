#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_gpio.h"
#include "inc/hw_uart.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"

#define ESC_REG(x)                  (*((volatile uint32_t *)(x)))
#define UART_BASE                   UART1_BASE

unsigned long message_count = 0;

void configure_output_pin(uint32_t port, uint8_t pin)
{
    GPIOPinTypeGPIOOutput(port, pin);
    GPIOPadConfigSet(port, pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

uint8_t checksum(uint8_t val[]) {
    int soma = val[0] + val[1] + val[2];
    int res = (soma & 0xFF) + ((soma >> 8) & 0xFF);
    res = (~(res) & 0xFF);

    if (res == 0) return 1;
    return 0;
}

uint8_t CRCDecode(uint8_t val[]) {
    unsigned int data = ((val[0] << 8) & 0xFF00 ) + (val[1] & 0xFF);
    unsigned int res = (data >> 8) & 0xFF;
    uint8_t i;
    for (i = 0 ; i < 8 ; i++) {
        res = (res << 1) & 0x1FF;
        res += ((data >> (7 - i)) & 0x1) & 0x1FF;
        res ^= 0xD5 * (res / 256);
    }

    res = res & 0xFF;

    if (res == 0) return 1;
    return 0;
}

int sendHeader(uint8_t slaveID) {

    if (slaveID > 59 || slaveID < 2) {
        UARTCharPut(UART0_BASE, 'E');
        UARTCharPut(UART0_BASE, ':');
        UARTCharPut(UART0_BASE, ' ');
        UARTCharPut(UART0_BASE, 'I');
        UARTCharPut(UART0_BASE, 'D');
        UARTCharPut(UART0_BASE, '\n');
        UARTCharPut(UART0_BASE, '\r');
        return -2;
    }

    uint8_t attempts = 0; // Checksum/CRC: if wrong, retry 3 times, return -1 if fail
    uint8_t success = 0;

    uint8_t i = 0;

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, 0xFF); // Cyan
    SysCtlDelay(SysCtlClockGet() / 30);

    while (success == 0 && attempts < 3) {

        message_count += 1;

        UARTCharPutNonBlocking(UART0_BASE, '0' + slaveID);

        uint8_t parity = 0;

        if ((message_count - 2) \% 9 == 0) { // Send wrong parity once in a while
            parity = 0x03;
        } else {
            for (i = 0 ; i < 6 ; i++)
                parity = parity + ((slaveID >> i) & 0x01);

            parity = parity % 2;
            parity = (parity << 1) & 0x03;
        }

        uint8_t id = (((slaveID & 0x3F) << 2) + parity) & 0xFF;

        UARTBreakCtl(UART_BASE, 1); // send BREAK
        UARTCharPutNonBlocking(UART0_BASE, '>');
        UARTCharPutNonBlocking(UART0_BASE, ' ');
        UARTBreakCtl(UART_BASE, 0); // stop BREAK

        UARTCharPut(UART_BASE, 0x55); // send SYNC (0x55)
        UARTCharPut(UART_BASE, id); // send slaveID + parity

        uint8_t received[7];
        i = 0;
        while (!UARTCharsAvail(UART_BASE));
        while (UARTCharsAvail(UART_BASE) || i < 5) {
            char input = UARTCharGet(UART_BASE);
            received[i++] = input;
        }

        // MCP2003 repeats the message sent, so it removes the first two bytes
        received[0] = received[2];
        received[1] = received[3];
        received[2] = received[4];

        attempts = attempts + 1;
        success = 1;

        // Checks checksum
        if (checksum(received) == 0) {
            UARTCharPutNonBlocking(UART0_BASE, 'C');
            success = 0;
        }

        // Checks CRC
        if (CRCDecode(received) == 0){
            UARTCharPutNonBlocking(UART0_BASE, ' ');
            UARTCharPutNonBlocking(UART0_BASE, 'C');
            UARTCharPutNonBlocking(UART0_BASE, 'R');
            UARTCharPutNonBlocking(UART0_BASE, 'C');
            success = 0;
        }

        if (success == 0) {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00); // Off
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0xFF); // Blue
            SysCtlDelay(SysCtlClockGet() / 30);
        } else {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00); // Off
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0xFF); // Green
            SysCtlDelay(SysCtlClockGet() / 30);
            UARTCharPutNonBlocking(UART0_BASE, received[0]); // Print message
        }
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00); // Off
        SysCtlDelay(SysCtlClockGet() / 30);

        UARTCharPutNonBlocking(UART0_BASE, '\n');
        UARTCharPutNonBlocking(UART0_BASE, '\r');
    }

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00); // Off

    if (success == 0) {
        return -1;
    }

    return 1;
}


// MAIN -----------------------------------------------------
int main(void) {

    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // Necessary for UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // LED
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0); // Output to PC
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0) || !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {}

    //UART 1 CONFIG
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART_BASE, SysCtlClockGet(), 300, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    //UART 0 CONFIG
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00); // Off

    configure_output_pin(GPIO_PORTB_BASE, GPIO_PIN_4);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0xFF);

    IntMasterEnable();

    while(1) {
        sendHeader(2);
        SysCtlDelay(SysCtlClockGet() * 1.7);
    }

    return 0;
}
