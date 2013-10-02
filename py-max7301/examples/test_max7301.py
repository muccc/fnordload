import max7301
import time
import logging

logging.basicConfig(level=logging.INFO)

device = max7301.MAX7301()

device.set_pin_as_output(30)
device.set_pin_as_input(31, pullup = False)

while 1:
    device.set_pin(30, 1)
    #time.sleep(0.1)
    print "Pin 31 =", device.get_pin(31)
    device.set_pin(30, 0)
    #time.sleep(0.1)
    print "Pin 31 =", device.get_pin(31)
