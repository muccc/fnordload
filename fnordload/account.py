from . import utils
import logging

class Account(object):
    def __init__(self, account_name):
        self._logger = logging.getLogger(__name__)
        self._file_name = account_name
        self._read_value()

    def _write_value(self, value):
        f = open(self._file_name, 'w')
        f.write(str(value))
        f.close()
        utils.sync()
        self._read_value()

    def _read_value(self):
        f = open(self._file_name, 'r')
        self._value = int(f.read())
        f.close()

    @property
    def value(self):
        self._logger.info("Value of account %s: %f" % (self._file_name, self._value))
        return self._value

    def add(self, value):
        self._logger.info("Adding %f to account %s" % (value, self._file_name))
        self._write_value(self.value + value)

    def subtract(self, value):
        self._logger.info("Subtracting %f form account %s" % (value, self._file_name))
        self._write_value(self.value - value)
