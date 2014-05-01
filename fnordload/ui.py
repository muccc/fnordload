class UI(object):
    def __init__(self, keypad, lcd):
        self._lcd = lcd
        self._keypad = keypad

    def choose(self, message, options):
        visible_options = [option for option in options if option['visible']]
        visible_count = len(visible_options)

        if visible_count < 1 or visible_count > 3:
            raise ValueError('Need between 1 and 3 visible options')

        functions = {}
        for option in options:
            key = option['key']
            if key not in functions:
                functions[key] = option['function']
            else:
                raise ValueError('Got more than one function for a single key')

        display_options = [''] * 3

        for i in range(visible_count):
            display_options[i] = str(visible_options[i]['key']) + ': ' + visible_options[i]['name']

        self._lcd.write(message, display_options[0], display_options[1], display_options[2])

        while True:
            key = self._keypad.get_single_key()
            if key in functions:
                return functions[key]()

    def input_number(self, update_function):
        value = 0
        while True:
            update_function(value)
            k = self._keypad.get_single_key()
            if k == '#':
                break
            if k == '*':
                value = value / 10
            else:
                value *= 10
                value += k

        return value

import max7301
if __name__ == "__main__":
    import lcd
    lcd = lcd.LCD('localhost')
    io_device = max7301.MAX7301()
    ui = UI(io_device = io_device, lcd = lcd)
    print ui.choose('krobin moechte', ['pizza', 'doener'])

