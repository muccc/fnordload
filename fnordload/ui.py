class UI(object):
    def __init__(self, keypad, lcd):
        self._lcd = lcd
        self._keypad = keypad
    
    def choose(self, message, options):
        if len(options) < 1 or len(options) > 3:
            raise ValueError('Need between 1 and 3 options')
        
        count = len(options)
        display_options = [''] * 3

        for i in range(count):
            display_options[i] = str(i + 1) + ': ' + options[i]

        self._lcd.write(message, display_options[0], display_options[1], display_options[2])
        
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

