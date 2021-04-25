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

#from lin_analog import ADC_to_char
import random
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import time, serial, argparse
# If it doesn't work, try to import os, sys, time, serial, argparse, requests

parser = argparse.ArgumentParser(description='MCU to oscilloscope')

parser.add_argument('-d','--dev', help='MCU device descriptor file', default='/dev/ttyACM0')
#parser.add_argument('-g','--debug', help='Only Debug', action='store_true')

args = vars(parser.parse_args())
DEV = args['dev']
MCU = serial.Serial(DEV, 115200)

def treat_data():
	global MCU
	data = MCU.read(1)
	#val = (13.291 / (256 - int.from_bytes(data, "big")))
	val = int.from_bytes(data, "big") * .0521
	print(int.from_bytes(data, "big"))
	return val
	#return (13.291 / (256 - int.from_bytes(data, "big")))
	
x_vals = [1,2,3,4,5,6,7,8,9,10]
y_vals = [0,0,0,0,0,0,0,0,0,0]
	
def animate(i):
	y_vals.append(treat_data())
	y_vals.pop(0)
	plt.cla()
	plt.xlabel('Time (s)')
	plt.ylabel('Voltage (V)')
	plt.ylim([0, 15])
	plt.axes().set_facecolor('#000000')
	plt.grid(True, color='0.05')
	plt.plot(x_vals, y_vals, '#00FF00')
	
def main():
	global MCU
	MCU.close()
	MCU.open()
	
	ani = FuncAnimation(plt.gcf(), animate, interval = 10)
	plt.show()
	
if __name__ == "__main__":
	main()
