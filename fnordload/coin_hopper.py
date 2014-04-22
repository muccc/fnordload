import max7301

class CoinHopper(object):
    def __init__(self, cointype, payoutIn1 = 4,
                 payoutIn2 = 5, payoutIn3 = 6):
        self._logger = logging.getLogger('logger')

        self._cointype = cointype
        self._iodevice = max7301.MAX7301()
        self._payoutIn1 = payoutIn1
        self._payoutIn2 = payoutIn2
        self._payoutIn3 = payoutIn3
        self.setup()

    def setup(self):
        self._read_coinlevel()
        self._iodevice.set_pin_as_output(self.__payoutIn1)
        self._iodevice.set_pin_as_output(self.__payoutIn2)
        self._iodevice.set_pin_as_output(self.__payoutIn3)
        self.reset()
        self._iodevice.set_pin(self.__payoutIn3, 1)

    def _write_coinlevel(self, newlevel):
        f = open('coins', 'w')
        f.write(str(newlevel))
        f.close()
        import ctypes
        ctypes.CDLL("libc.so.6").sync()
        self._read_coinlevel()

    def _read_coinlevel(self):
        f = open('coins', 'r')
        self._coins = int(f.read())
        f.close()
    
    @property
    def get_coin_type(self):
        return self._cointype

    @property
    def get_coin_level(self):
        return self._coins

    def payout(self, value):
        payoutcoins = int(value / self._cointype)
        self._logger.info("Payout of " + str(payoutcoins) + " Coins")
        self._write_coinlevel(self._coins - payoutcoins);
        for i in range(0, payoutcoins):
            self._iodevice.set_pin(self.__payoutIn3, 0)
            time.sleep(0.1)
            self._iodevice.set_pin(self.__payoutIn3, 1)
            time.sleep(0.1)

    def reset(self):
        self._iodevice.set_pin(self.__payoutIn1, 0)
        self._iodevice.set_pin(self.__payoutIn2, 1)
        time.sleep(0.5)
        self._iodevice.set_pin(self.__payoutIn1, 1)
        self._iodevice.set_pin(self.__payoutIn2, 0)

