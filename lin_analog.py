from machine import Pin, ADC, UART
# from utime import sleep

adc = ADC(28)
uart = UART(0, 115200)
led = Pin(25, Pin.OUT)

def ADC_to_char():
    # Returns value in a range from 0 to 255
    # 255 represents [3.3 * (33 + 4.7 + 4.7 + 1.5) / (4.7 + 4.7 + 1.5)] Volts
    #   255 -> 13.291 V
    # 1 represents 13.291 / 255
    #   1 -> 0.0521 V
    return round(adc.read_u16() * 255 / 65536)

def main():
    factor = 3.3 * (33 + 4.7 + 4.7 + 1.5) / (4.7 + 4.7 + 1.5)
    led.on()

    while True:
        buf = [ADC_to_char()]
        uart.write(bytes([buf[0]]))
        print(buf[0])

if __name__ == "__main__":
    main()
