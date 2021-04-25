#!/usr/bin/env python3
import random
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

f = open("data/data_sem_ruido.txt", 'r')
data = f.readlines()
n = 0
def treat_data():
	global n
	if n >= len(data):
		n = 0
	else:
		n = n + 1
	return (13.291 / (256 - int(data[n])))
	
y_vals = [0] * 300
x_vals = np.linspace(29,0,300)
fig, ax = plt.subplots()
line, = ax.plot(x_vals,y_vals, '#00FF00')
plt.axis([0,29,0,0.5])
plt.xlabel('Time passed(ms)')
plt.ylabel('Voltage (V)')
ax.set_facecolor('#000000')
plt.grid(True, color='0.05')

def animate(i):
	y_vals.append(treat_data())
	y_vals.pop(0)
	line.set_data(x_vals, y_vals)
	return line
	
	
def main():
	ani = FuncAnimation(fig, animate, interval = 100)
	plt.show()
	
if __name__ == "__main__":
	main()
