#!/usr/bin/env python3
import random
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

#Open file with data to be displayed
f = open("data/data_plot.txt", 'r')
data = f.readlines()

#Convert raw data to voltage
def treat_data():
    return (int(data[n])*13.291/256)
    
#Lists that hold the values to be displayed
vals = [0] * 100
x_vals = np.linspace(29,0,100)

#Graphic settings
fig, ax = plt.subplots()
line, = ax.plot(x_vals, vals, '#00FF00')
ax.legend()
plt.axis([0,29,0,13.4])
plt.xlabel('Time passed(s)')
plt.ylabel('Voltage (V)')
ax.set_facecolor('#000000')
plt.grid(True, color='0.05')

n = 0
lim = len(data)
def animate(i):
    global n
    vals.append(treat_data())
    vals.pop(0)
    line.set_data(x_vals, vals)
    if n >= lim:
        n = 0
    else:
        n = n + 1
    return line
    
    
def main():
    ani = FuncAnimation(fig, animate, interval = 1)
    plt.show()
    
if __name__ == "__main__":
    main()