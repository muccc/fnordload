import max7301
import logging
import time
from . import account

class CoinHopper(object):
    def __init__(self, cointype, io_device, payoutIn1 = 4,
                 payoutIn2 = 5, payoutIn3 = 6):
        self._logger = logging.getLogger('logger')

        self._cointype = cointype
        self._iodevice = io_device
        self._payoutIn1 = payoutIn1
        self._payoutIn2 = payoutIn2
        self._payoutIn3 = payoutIn3
        self.setup()

    def setup(self):
        self._account = account.Account('coins')
        self._iodevice.set_pin_as_output(self._payoutIn1)
        self._iodevice.set_pin_as_output(self._payoutIn2)
        self._iodevice.set_pin_as_output(self._payoutIn3)
        self.reset()
        self._iodevice.set_pin(self._payoutIn3, 1)

    @property
    def coin_type(self):
        return self._cointype

    @property
    def coin_level(self):
        return self._account.value

    def increase_coin_level(self, extra_coins):
        self._account.add(extra_coins)

    def payout(self, value):
        payoutcoins = int(value / self._cointype)
        self._logger.info("Payout of " + str(payoutcoins) + " Coins")
        self._account.sub(payoutcoins)
        for i in range(0, payoutcoins):
            self._iodevice.set_pin(self._payoutIn3, 0)
            time.sleep(0.1)
            self._iodevice.set_pin(self._payoutIn3, 1)
            time.sleep(0.1)

    def reset(self):
        self._iodevice.set_pin(self._payoutIn1, 0)
        self._iodevice.set_pin(self._payoutIn2, 1)
        time.sleep(0.5)
        self._iodevice.set_pin(self._payoutIn1, 1)
        self._iodevice.set_pin(self._payoutIn2, 0)

