#!/usr/bin/python

import sys
import select
import tty
import termios
from bluetooth.bluez import BluetoothSocket

s = BluetoothSocket()
s.connect(('00:80:37:14:49:AB', 1))

old_settings = termios.tcgetattr(sys.stdin)
try:
    tty.setcbreak(sys.stdin.fileno())

    while True:
        (inp, _, _) = select.select([sys.stdin, s], [], [])

        # Something from the serial port?
        if s in inp:
            d = s.recv(1)
            sys.stdout.write(d)
            sys.stdout.flush()

        # Something from the local terminal?
        if sys.stdin in inp:
            d = sys.stdin.read(1)
            s.send(d)

finally:
    termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)
