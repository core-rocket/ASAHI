#!/usr/bin/python3
import serial
import time

dev = '/dev/ttyUSB0'
ser = serial.Serial(dev, 9600)

def read_test(num):
    print("read test:")
    s = b""
    for i in range(num):
        c = ser.read()
        s += c
    print("\t", s)

def read_test2(sec):
    data = b''
    start = time.time()
    while True:
        if time.time() - start > sec:
            break
        data = data + ser.readline()
    print(data)

def send_cmd(cmd):
    cmd = b'PMTK' + cmd
    checksum = 0x00
    for b in cmd:
        checksum = checksum ^ b
    cmd = b'$' + cmd
    cmd = cmd + b'*' + format(checksum, 'x').encode() + b'\r\n'
    print("raw cmd: ", cmd)
    ser.write(cmd)

def change_brate(brate):
    global ser
    print("change brate: ", brate)
    send_cmd(b'251,' + str(brate).encode())
    ser.close()
    time.sleep(1)
    ser = serial.Serial(dev, brate)

def change_srate(srate):
    print("change sampling rate: ", srate, "ms")
    send_cmd(b'220,' + str(srate).encode())

print("brate = 9600")
change_brate(115200)
#change_brate(38400)

read_test2(1)
change_srate(1000 / 10)
read_test2(1)
