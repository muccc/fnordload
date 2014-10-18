import time
import eSSP
import logging
import threading
import Queue

class InvalidNoteError(Exception):
    pass

class TimeoutError(Exception):
    pass

class NoteValidator(object):
    def __init__(self, device = '/dev/ttyACM0', inhibits_mask = [1, 1, 1, 0, 0, 0]):
        self._logger = logging.getLogger(__name__)
        self._eSSP = eSSP.eSSP(device)
        self._inhibits_mask = inhibits_mask
        self._inhibits = [0, 0, 0, 0, 0, 0]
        self._eSSP.sync()
        self._eSSP.enable_higher_protocol()
        self._channelvalues = self._eSSP.channel_values()[1]
        self._thread = threading.Thread(target = self._run)
        self._keep_running = True
        self._essp_lock = threading.RLock()
        self._poll_queue = Queue.Queue(5)
        self._thread.setDaemon(True)

        self._thread.start()
    
    def exit(self):
        self._keep_running = False
        self._thread.join()

    def _set_inhibits(self, inhibits = [0, 0, 0, 0, 0, 0, 0, 0]):
        self._logger.info("New inhibits: %s" % str(inhibits))
        self._inhibits = inhibits
        with self._essp_lock:
            self._eSSP.set_inhibits(self._eSSP.easy_inhibit(inhibits), '0x00')
   
    def set_max_accepted_value(self, max_value):
        self._logger.info("Set max accepted value: %f" % max_value)
        newinhibits = []
        for channelvalue, inhibit_mask in zip(self._channelvalues, self._inhibits_mask):
            if channelvalue <= max_value and  inhibit_mask:
                newinhibits.append(1)
            else:
                newinhibits.append(0)

        self._set_inhibits(newinhibits)
    
    def get_accepted_values(self):
        accepted = [x[0] for x in zip(self._channelvalues, self._inhibits) if x[1]]
        self._logger.info("Accepted values: %s" % str(accepted))
        return accepted

    def read_note(self, timeout = 30, message_callback = lambda x: None):
        self._logger.info("Read note")
        self._reset_poll()
        t0 = time.time()
        with self._essp_lock:
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
                with self._essp_lock:
                    self._eSSP.disable()
                value = self._channelvalues[poll[1][1] - 1]
                self._logger.info("Read note of value: %f" % value)
                return value
            elif (len(poll) > 1 and poll[1] == '0xed'):
                with self._essp_lock:
                    self._eSSP.disable()
                self._logger.warning("Read invalid note")
                raise InvalidNoteError()
            elif (len(poll) > 1 and poll[0] == '0xf0'):
                self._logger.warning(str(poll))

        with self._essp_lock:
            self._eSSP.disable()

        self._logger.warning("Timeout while reading a note")
        raise TimeoutError()

    def _read_poll(self):
        return self._poll_queue.get()

    def _reset_poll(self):
        try:
            while True:
                self._poll_queue.get(block = False)
        except Queue.Empty:
            pass

    def _run(self):
        while self._keep_running:
            with self._essp_lock:
                poll = self._eSSP.poll()

            while self._keep_running:
                try:
                    self._poll_queue.put(poll, timeout = 1)
                except Queue.Full:
                    continue
                break

