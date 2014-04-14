from lcdproc.server import Server
import logging

class LCD(object):
    def __init__(self, server = "localhost", debug = False):
        self.__logger = logging.getLogger('logger')
        self.__lcd = Server(server, debug=debug)
        self.__lcd.start_session()
        self.__screen = self.__lcd.add_screen("screen")
        self.__screen.set_heartbeat("off")
        self.__screen.set_priority("foreground")
        self.__line1 = self.__screen.add_string_widget("line1", text="", x=1, y=1)
        self.__line2 = self.__screen.add_string_widget("line2", text="", x=1, y=2)
        self.__line3 = self.__screen.add_string_widget("line3", text="", x=1, y=3)
        self.__line4 = self.__screen.add_string_widget("line4", text="", x=1, y=4)

    def clear(self):
        self.write()

    def write(self, line1 = "", line2 = "", line3 = "", line4 = ""):
        self.__line1.set_text(line1)
        self.__line2.set_text(line2)
        self.__line3.set_text(line3)
        self.__line4.set_text(line4)

        self.__logger.info(line1 + ' | ' + line2 + ' | ' + line3 + ' | ' + line4)

    def setup(self):
        self.write("Fnordload booting", "", "Please stand by...")

    def welcome(self, values):
        accepted = [str(x) for x in values]

        if (len(accepted) == 0):
            self.out_of_order()
        else:
            self.__screen.set_backlight("on")
            self.write("Welcome to Fnordload", "Accpeting (Euro):", ", ".join(accepted),"        Insert money")

    def out_of_order(self):
        self.write("Sorry!","Fnordload is currently", "out of order.", "             The MGT")
        self.__screen.set_backlight("off")

    def reading_note(self, value = 0):
        self.__screen.set_backlight("on")

        if value == 0:
            self.write("Reading note...")
        else:
            self.write(value + " Euro note read")

    def cashed_note(self, value):
        self.__screen.set_backlight("on")

        self.write("Cashed " + value + " Euro")

    def payout_in_progress(self):
        self.__screen.set_backlight("blink")
        self.write("Payout in Progress", "", "", "Please stand by")

    def rejected_note(self):
        self.__screen.set_backlight("flash")
        self.write("Sorry, this note", "cannot be accepted" , "at this time.")
