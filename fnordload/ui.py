import keypad

class UI(object):
    def __init__(self, io_device, lcd):
        self._keypad = keypad.KeyPad(io_device)
        self._lcd = lcd
    
    def choose(self, message, options):
        if len(options) < 1 or len(options) > 3:
            raise ValueError('Need between 1 and 3 options')
        
        count = len(options)
        for i in range(count):
            options[i] = str(i + 1) + ': ' + options[i]

        options.extend([''] * (3 - count)) 

        self._lcd.write(message, options[0], options[1], options[2])
        
        while True:
            choice = self._keypad.get_single_key()

            if choice in range(1, count + 1):
                return options[choice - 1]

import max7301
if __name__ == "__main__":
    import lcd
    lcd = lcd.LCD('localhost')
    io_device = max7301.MAX7301()
    ui = UI(io_device = io_device, lcd = lcd)
    print ui.choose('krobin moechte', ['pizza', 'doener'])

