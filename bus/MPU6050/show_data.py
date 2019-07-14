#!/usr/bin/python3
import serial
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import animation
from matplotlib import gridspec

s = serial.Serial('/dev/ttyUSB0', timeout=0.1)
s.write(b's')

fig = plt.figure()

acc_x = []
acc_y = []
acc_z = []

gyro_x= []
gyro_y= []
gyro_z= []

temperature = []

def get_data():
    line = s.readline().decode().rstrip()
    data = line.split(" ")
    acc_x.append(float(data[0]))
    acc_y.append(float(data[1]))
    acc_z.append(float(data[2]))
    gyro_x.append(float(data[3]))
    gyro_y.append(float(data[4]))
    gyro_z.append(float(data[5]))
    temperature.append(float(data[6]))

# 1 2
# 3 4
# 5 6
# 7 8

def update(i):
    get_data()
    if i > 50:
        acc_x.pop(0)
        acc_y.pop(0)
        acc_z.pop(0)
        gyro_x.pop(0)
        gyro_y.pop(0)
        gyro_z.pop(0)
        temperature.pop(0)
    p = plt.subplot(4, 2, 1)
    p.cla()
    p.set_xlim([0, 50])
    #p.set_ylim([10.0, 40.0])
    #plt.yticks(np.arange(0.0, 40.0, 0.5))
    p.plot(temperature)

    p = plt.subplot(4, 2, 3)
    p.cla()
    p.set_xlim([0, 50])
    #p.set_ylim([-2.0, 2.0])
    p.plot(acc_x)

    p = plt.subplot(4, 2, 5)
    p.cla()
    p.set_xlim([0, 50])
    #p.set_ylim([-2.0, 2.0])
    p.plot(acc_y)

    p = plt.subplot(4, 2, 7)
    p.cla()
    p.set_xlim([0, 50])
    #p.set_ylim([-2.0, 2.0])
    p.plot(acc_z)

    p = plt.subplot(4, 2, 4)
    p.cla()
    p.set_xlim([0, 50])
    p.plot(gyro_x)

    p = plt.subplot(4, 2, 6)
    p.cla()
    p.set_xlim([0, 50])
    p.plot(gyro_y)

    p = plt.subplot(4, 2, 8)
    p.cla()
    p.set_xlim([0, 50])
    p.plot(gyro_z)

    plt.tight_layout()

ani = animation.FuncAnimation(fig, update, interval=200)
plt.show()
