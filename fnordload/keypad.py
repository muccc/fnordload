#!/usr/bin/env python

import time
import max7301
import logging

class TimeoutError(Exception):
    pass

class KeyPad(object):
    INPUT_ROW_PINS = [12, 13, 18, 17]
    INPUT_COL_PINS = [15, 14, 16]
    
    MAPPING = [1, 4, 7, '*', 2, 5, 8, 0, 3, 6, 9, '#']
    def __init__(self, io_device):
        self._io_device = io_device

        map(self._io_device.set_pin_as_input, self.INPUT_ROW_PINS)
        map(self._io_device.set_pin_as_output, self.INPUT_COL_PINS)
        map(lambda x: self._io_device.set_pin(x, 1), self.INPUT_COL_PINS)

    def scan(self, scan_time = 0.01):
        pressed_keys = []
        key = 0
        for col in self.INPUT_COL_PINS:
            self._io_device.set_pin(col, 0)
            time.sleep(scan_time)
            for row in self.INPUT_ROW_PINS:
                pin = self._io_device.get_pin(row)
                if pin == 0:
                    pressed_keys.append(self.MAPPING[key])
                key += 1
            self._io_device.set_pin(col, 1)
            time.sleep(scan_time)
        return pressed_keys

    def get_single_key(self, timeout = 30):
        t0 = time.time()
        key = None
        all_released_counter = 0

        while True:
            if time.time() > t0 + timeout:
                raise TimeoutError("No key pressed")

            keys = self.scan()
            if all_released_counter < 1 and keys:
                all_released_counter = 0
                continue

            all_released_counter += 1

            if len(keys) == 1:
                key = keys[0]
                break

        return key


if __name__ == "__main__":
    #logging.basicConfig(level=logging.INFO)
    iodevice = max7301.MAX7301()
    k = KeyPad(iodevice)
    while 1:
        print k.get_single_key()

