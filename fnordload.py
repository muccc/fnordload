import LCD
import time
import eSSP.eSSP

class fnordload(object):
	def __init__(self, LCDserver = 'localhost', eSSPport = '/dev/ttyACM0',
			inhibits = [0, 0, 0, 0, 0, 0, 0, 0], cointype = 0.5):
		self.__LCD = LCD.LCD(LCDserver)
		self.__eSSP = eSSP.eSSP.eSSP(eSSPport)
		self.__cointype = cointype
		self.__inhibits = inhibits
		self.setup()

	def setup(self):
		self.__LCD.setup()
		self.__eSSP.sync()
		self.__eSSP.enable_higher_protocol()
		self.__channelvalues = self.__eSSP.channel_values()[1]
		self.set_coinlevelinhibits()

	def set_inhibits(self, inhibits = [0, 0, 0, 0, 0, 0, 0, 0]):
		self.__inhibits = inhibits
		self.__eSSP.set_inhibits(self.__eSSP.easy_inhibit(inhibits), '0x00')

	def write_coinlevel(self, newlevel):
		f = open('coins', 'w')
		f.write(str(newlevel))
		f.close()
		self.set_coinlevelinhibits()

	def set_coinlevelinhibits(self):
		f = open('coins', 'r')
		self.__coins = int(f.read())
		f.close()

		newinhibits = []
		for channelvalue, oldinhibit in zip(self.__channelvalues, self.__inhibits):
			if (self.__coins - (channelvalue / self.__cointype) >= 0 and oldinhibit != 0):
				newinhibits.append(1)
			else:
				newinhibits.append(0)

		self.set_inhibits(newinhibits)
	
	def payout(self, value):
		payoutcoins = int(value / self.__cointype)
		print "Payout of " + str(payoutcoins) + " Coins"
		self.write_coinlevel(self.__coins - payoutcoins);
		self.__LCD.payout_in_progress()
		time.sleep(0.2 * payoutcoins)
		self.__eSSP.enable()
		self.welcome()

	def welcome(self):
		self.__LCD.welcome(self.__inhibits, self.__channelvalues)

	def main(self):
		self.welcome()
		self.__eSSP.enable()

		while True:
			poll = self.__eSSP.poll()

			if (len(poll) > 1 and len(poll[1]) == 2 and poll[1][0] == '0xef'):
				if (poll[1][1] == 0):
					self.__LCD.reading_note()
				else:
					self.__eSSP.hold()
					self.__LCD.reading_note(str(self.__channelvalues[poll[1][1] - 1]))

			if (len(poll) > 1 and len(poll[1]) == 2 and poll[1][0] == '0xee'):
				self.__LCD.cashed_note(str(self.__channelvalues[poll[1][1] - 1]))
				self.__eSSP.disable()
				self.payout(self.__channelvalues[poll[1][1] - 1])
			if (len(poll) > 1 and poll[1] == '0xed'):
				self.__LCD.rejected_note()
				time.sleep(2)
				self.welcome()

			time.sleep(0.5)

if __name__ == "__main__":
	fnordload = fnordload(inhibits = [1, 1, 1, 1, 1, 1])

	while True:
		fnordload.main()
