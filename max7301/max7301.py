import spidev
import logging

class MAX7301(object):
    def __init__(self, spi_dev = 0, init_all = False):   
        self._logger = logging.getLogger(__name__)

        self._spi = spidev.SpiDev()
        self._spi.open(spi_dev, 0)
        
        self._write_register(0x04, 0x01)

        if init_all:
            for pin in range(4, 31+1):
                self.set_pin_as_input(pin)

    def set_pin_as_output(self, pin):
        self._logger.info("Set pin %d as output" % pin)
        register = self._get_conf_register_for_pin(pin)
        content = self._read_register(register)
        content &= ~(0b11 << self._get_bit_shift_for_pin(pin))
        content |= 0b01 << self._get_bit_shift_for_pin(pin)
        self._write_register(register, content)
    
    def set_pin(self, pin, value):
        self._logger.info("Set pin %d to %d" % (pin, value))
        register = self._get_value_register_for_pin(pin)
        self._write_register(register, value)
       
    def set_pin_as_input(self, pin, pullup = True):
        register = self._get_conf_register_for_pin(pin)
        content = self._read_register(register)
        content &= ~(0b11 << self._get_bit_shift_for_pin(pin))
        if pullup:
            self._logger.info("Set pin %d as input with pullup" % pin)
            content |= 0b11 << self._get_bit_shift_for_pin(pin)
        else:
            self._logger.info("Set pin %d as input" % pin)
            content |= 0b10 << self._get_bit_shift_for_pin(pin)
        self._write_register(register, content)
 
    def get_pin(self, pin):
        register = self._get_value_register_for_pin(pin)
        value = self._read_register(register)
        self._logger.info("Get pin %d: Result = %d" % (pin, value))
        return value
    
    def _get_conf_register_for_pin(self, pin):
        return (pin-4)/4 + 0x09
 
    def _get_value_register_for_pin(self, pin):
        return 0x20 + pin
       
    def _read_register(self, register):
        self._spi.xfer2([0x80 | register, 0x00])
        content = self._spi.xfer2([0x80, 0x00])[1]

        self._logger.debug("Read register %02x. Result = %02x" % (register, content))
        return content

    def _write_register(self, register, value):
        self._logger.debug("Set register %02x to %02x" % (register, value))
        self._spi.xfer2([register, value])

    def _get_bit_shift_for_pin(self, pin):
            return ((pin % 4) * 2)
    
