from machine import Pin, ADC, UART
# from utime import sleep

adc = ADC(28)
uart = UART(0, 115200)
led = Pin(25, Pin.OUT)

def readADC(fac):
    return adc.read_u16() * fac / 65536

def main():
    factor = 3.3 * (33 + 4.7 + 4.7 + 1.5) / (4.7 + 4.7 + 1.5)
    led.on()
    while True:
        buf = [readADC(factor)]
        uart.write(bytes([buf[0]]))

if __name__ == "__main__":
    main()
