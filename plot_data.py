#!/usr/bin/env python3

# To run, use:
# 	 $ python3 plot_data.py -d <serial_port>

# During development, just use the fake function that gives you some
# numbers within [0, 12] and run:
# 	 $ python3 plot_data.py -g

# To discover <serial_port>, use:
#	 $ ls /dev/
# First without the MCU connected and then with it connected
# The new element is your <serial_port> value

import time, serial, argparse
# If it doesn't work, try to import os, sys, time, serial, argparse, requests

parser = argparse.ArgumentParser(description='MCU to oscilloscope')

parser.add_argument('-d','--dev', help='MCU device descriptor file', default='/dev/ttyACM0')
parser.add_argument('-g','--debug', help='Only Debug', action='store_true')

DEBUG = args['debug']
if DEBUG:
	DEV = args['dev']
	MCU = serial.Serial(DEV, 9600)
else:
	MCU = 0

i = 0
def fake():
	global i
	i = i + 1
	sleep(0.1)
	return int(i / 30) % 13
	
def main():
	global MCU
	MCU.open()
	
	while True:
		#val = MCU.read(1)   # Gets the next value from MCU (blocking call).
							# MCU is will send an analog read from LIN bus periodically
		
		val = fake() # Fakes readings from MCU, used for testing
		
		my_plot_value(val)  # Plots received values in an oscilloscope fashion
