import time
import eSSP.eSSP
import logging
import threading

class InvalidNoteError(Exception):
    pass

class TimeoutError(Exception):
    pass

class NoteValidator(object):
    def __init__(self, device = '/dev/ttyACM0', inhibits = [0, 0, 0, 0, 0, 0, 0, 0]):
        self._logger = logging.getLogger('logger')
        self._eSSP = eSSP.eSSP.eSSP(eSSPport)
        self._inhibits = inhibits
        self._eSSP.sync()
        self._eSSP.enable_higher_protocol()
        self._channelvalues = self._eSSP.channel_values()[1]
        self._thread = threading.Thread(target = self._run)
        self._keep_running = True
        self._essp_lock = threading.RLock()
        self._thread.start()

    def _set_inhibits(self, inhibits = [0, 0, 0, 0, 0, 0, 0, 0]):
        self._inhibits = inhibits
        self._eSSP.set_inhibits(self._eSSP.easy_inhibit(inhibits), '0x00')
   
    def set_max_accepted_value(self, max_value):
        newinhibits = []
        for channelvalue, oldinhibit in zip(self.__channelvalues, self.__inhibits):
            if channelvalue <= max_value and oldinhibit != 0:
                newinhibits.append(1)
            else:
                newinhibits.append(0)

        self._set_inhibits(newinhibits)
   
    def read_note(self, timeout = 30, message_callback = lambda x: None):
        self._reset_poll()
        t0 = time.time()
        self._eSSP.enable()
        while time.time() < t0 + timeout:
            poll = self._read_poll()
            if len(poll) > 1 and len(poll[1]) == 2 and poll[1][0] == '0xef':
                if poll[1][1] == 0:
                    message_callback(None)
                else:
                    self._eSSP.hold()
                    message_callback(self._channelvalues[poll[1][1] - 1])
            elif len(poll) > 1 and len(poll[1]) == 2 and poll[1][0] == '0xee':
                self._eSSP.disable()
                return self._channelvalues[poll[1][1] - 1]
            elif (len(poll) > 1 and poll[1] == '0xed'):
                raise InvalidNoteError()
        raise TimeoutError()

