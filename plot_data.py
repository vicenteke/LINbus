#!/usr/bin/env python3
import random
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

#Open file with data to be displayed
f = open("data/data_sem_ruido.txt", 'r')
data_s_r = f.readlines()
g = open("data/data_com_ruido.txt", 'r')
data_c_r = g.readlines()

#Functions to convert raw data to voltage
def treat_data_s_r():
	return (13.291 / (256 - int(data_s_r[n])))
	
def treat_data_c_r():
	return (13.291 / (256 - int(data_c_r[n])))
	
#Lists that hold the values to be displayed
s_r_vals = [0] * 300
c_r_vals = [0] * 300
x_vals = np.linspace(29,0,300)

#Graphic settings
fig, ax = plt.subplots()
line, = ax.plot(x_vals,s_r_vals, '#00FF00', label='Sem ruido')
line2, = ax.plot(x_vals,c_r_vals, '#00FFFF', label='Com ruido')
ax.legend()
plt.axis([0,29,0,0.5])
plt.xlabel('Time passed(s)')
plt.ylabel('Voltage (V)')
ax.set_facecolor('#000000')
plt.grid(True, color='0.05')

n = 0
lim = len(data_s_r) if len(data_s_r) > len(data_c_r) else len(data_c_r)
def animate(i):
	global n
	s_r_vals.append(treat_data_s_r())
	s_r_vals.pop(0)
	c_r_vals.append(treat_data_c_r())
	c_r_vals.pop(0)
	line.set_data(x_vals, s_r_vals)
	line2.set_data(x_vals, c_r_vals)
	if n >= lim:
		n = 0
	else:
		n = n + 1
	return line
	
	
def main():
	ani = FuncAnimation(fig, animate, interval = 100)
	plt.show()
	
if __name__ == "__main__":
	main()
