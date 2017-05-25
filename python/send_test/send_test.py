#!/usr/bin/python

__author__ = 'gdiaz'

import serial
import syslog
import time
from threading import Timer

#The following line is for serial over GPIO
port = '/dev/ttyACM0'

class GSM(object):
    def __init__(self, serial, debug = False):
        self.gsm = serial
        self.sms = "gms test"
        self.config = "AT+"
        self.debug = debug

    def DEBUG_PRINT(self, msg_type, msg):
        if not(self.debug): return
        if msg_type == "info":
            print chr(27)+"[0;32m"+"[INFO]: "+chr(27)+"[0m" + msg
        elif msg_type == "warn":
            print chr(27)+"[0;33m"+"[WARN]: "+chr(27)+"[0m" + msg
        elif msg_type == "error":
            print chr(27)+"[0;31m"+"[ERROR]: "+chr(27)+"[0m" + msg
        elif msg_type == "alert":
            print chr(27)+"[0;34m"+"[ALERT]: "+chr(27)+"[0m" + msg
        else:
            print "NON implemented Debug print type"

    def read(self):
        # data = self.gsm.readline()
        data = self.gsm.read(50)
        # self.gsm.flush()
        self.DEBUG_PRINT("info", data)

    def write(self, cmd):
        self.gsm.write(cmd.encode("utf-8"))
        self.gsm.flush()

if __name__ == '__main__':
    # gsm_serial = serial.Serial(port, 115200, timeout=5, stopbits=serial.STOPBITS_ONE)
    gsm_serial = serial.Serial(port, 115200, timeout=5)
    gsm_hw = GSM(gsm_serial, debug =True)
    # AT+CSCS="GSM"
    # AT+CMGF=1
    # AT+CMGS="+56964696411"
    # AT+CMGS="56964696411"
    # test read
    while True:
        cmd = raw_input("cmd:")
        if cmd == "end": break
        elif cmd == "send":
            msg_ = "a"+chr(13)+chr(26)
            gsm_hw.write(msg_)
        elif cmd == "read":
            gsm_hw.read()
        else:
            gsm_hw.DEBUG_PRINT("warn", "Sending:"+cmd)
            gsm_hw.write(cmd+"\r")
            gsm_hw.read()
    gsm_serial.close()