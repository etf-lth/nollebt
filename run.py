#!/usr/bin/python
#
# Simple serial terminal with a twist (the reset control)
#
# (c) 2012 <fredrik@z80.se>

import sys
import serial
import select
import tty
import termios
        
# Open the main serial link
s = serial.Serial('/dev/ttyUSB1', 9600)

# Perform a hard reset on the module
rs = serial.Serial('/dev/ttyUSB0')
rs.setRTS(True)
rs.setRTS(False)
rs.setRTS(True)

old_settings = termios.tcgetattr(sys.stdin)
try:
    tty.setcbreak(sys.stdin.fileno())

    while True:
        (inp, _, _) = select.select([sys.stdin, s], [], [])

        # Something from the serial port?
        if s in inp:
            d = s.read(1)
            sys.stdout.write(d)
            sys.stdout.flush()

        # Something from the local terminal?
        if sys.stdin in inp:
            d = sys.stdin.read(1)
            s.write(d)

finally:
    termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)
