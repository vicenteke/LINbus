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
#define SLAVE_ID                    2
#define MESSAGE                     0x63

#define UART_BASE                   UART1_BASE

typedef struct {
    unsigned int count;
    uint8_t msg;
    uint8_t buf[3];
} data_t;

data_t Data;

void configure_output_pin(uint32_t port, uint8_t pin)
{
    GPIOPinTypeGPIOOutput(port, pin);
    GPIOPadConfigSet(port, pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

uint8_t checksum(data_t d) {
    int soma = d.buf[0] + d.buf[1];
    int res = (soma & 0xFF) + ((soma >> 8) & 0xFF);
    return (~(res) & 0xFF);
}

uint8_t CRC(data_t d){
    unsigned int res = d.msg;
    uint8_t u;
    for (u = 0 ; u < 8 ; u++) {
        res = (res << 1) & 0x1FF;
        res ^= 0xD5 * (res / 256);
    }
    res = (res & 0xFF);

    return res;
}

/* LIN message sequence:
 *
 *   1: Right message
 *   2: sendError() (wrong parity from master)
 *   3: Right message
 * 4-6: Wrong Checksum
 * 7-9: Wrong CRC
 *
 */
void sendData() {

    Data.buf[0] = Data.msg;

    // Calculate CRC and Checksum
    Data.buf[1] = CRC(Data);
    Data.buf[2] = checksum(Data);

    if (Data.count % 9 > 5) {
        // Send wrong CRC
        Data.buf[1] = Data.buf[1] + 1;
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_3, 0xFF); // Yellow
        SysCtlDelay(SysCtlClockGet() / 30);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00); // Off

    } else if (Data.count % 9 > 2) {
        // Send wrong Checksum
        Data.buf[2] = Data.buf[2] + 1;
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2, 0xFF); // Magenta
        SysCtlDelay(SysCtlClockGet() / 30);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00); // Off

    } else {
        // Send data right
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0xFF); // Green
        SysCtlDelay(SysCtlClockGet() / 30);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00); // Off
    }

    // Send msg + CRC + checksum
    UARTCharPut(UART_BASE, Data.buf[0]);
    UARTCharPut(UART_BASE, Data.buf[1]);
    UARTCharPut(UART_BASE, Data.buf[2]);

//    Uncomment lines above to print message sent
//    UARTCharPutNonBlocking(UART0_BASE, Data.buf[0]);
//    UARTCharPutNonBlocking(UART0_BASE, Data.buf[1]);
//    UARTCharPutNonBlocking(UART0_BASE, Data.buf[2]);
//    UARTCharPutNonBlocking(UART0_BASE, '\n');
//    UARTCharPutNonBlocking(UART0_BASE, '\r');

    Data.count = Data.count + 1;
    Data.buf[0] = Data.buf[1] = Data.buf[2] = 0;
}

void sendError() {
    Data.count = Data.count + 1;
    Data.buf[0] = Data.buf[1] = Data.buf[2] = 0x55;

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0xFF); // Red
    SysCtlDelay(SysCtlClockGet() / 30);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00); // Off

    UARTCharPutNonBlocking(UART_BASE, Data.buf[0]);
    UARTCharPutNonBlocking(UART_BASE, Data.buf[1]);
    UARTCharPutNonBlocking(UART_BASE, Data.buf[2]);

    UARTCharPutNonBlocking(UART0_BASE, Data.buf[0]);
    UARTCharPutNonBlocking(UART0_BASE, Data.buf[1]);
    UARTCharPutNonBlocking(UART0_BASE, Data.buf[2]);
    UARTCharPutNonBlocking(UART0_BASE, '\n');
    UARTCharPutNonBlocking(UART0_BASE, '\r');

    Data.buf[0] = Data.buf[1] = Data.buf[2] = 0;
}

void uartHandler1(void) {

    uint32_t status;
    status = UARTIntStatus(UART_BASE, true);
    UARTIntClear(UART_BASE, status);

    IntDisable(INT_UART1);

    uint8_t i = 0;

    while(UARTCharsAvail(UART_BASE)) {
        char input = UARTCharGetNonBlocking(UART_BASE);

        if (i == 1) {
            if (((input >> 2) & 0x3F) == SLAVE_ID) {
                uint8_t parity = 0;
                uint8_t j;
                for ( j = 0 ; j < 6 ; j++) {
                    parity = parity + ((SLAVE_ID >> j) & 0x01);
                }
                parity = parity % 2;
                parity = (parity << 1) & 0x03;

                if (parity == (input & 0x03)) {
                    sendData();
                } else {
                    sendError();
                }
            }
        }
        i = i + 1;
    }
    IntEnable(INT_UART1);
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
    UARTIntEnable(UART_BASE, UART_INT_RX | UART_INT_RT);
    IntEnable(INT_UART1);

    //UART 0 CONFIG
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00); // Off

    Data.count = 0;
    Data.msg = MESSAGE;
    Data.buf[0] = 0;
    Data.buf[1] = 0;
    Data.buf[2] = 0;

    configure_output_pin(GPIO_PORTB_BASE, GPIO_PIN_4);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0xFF);

    IntMasterEnable();

    while(1) {
    }

    return 0;
}
