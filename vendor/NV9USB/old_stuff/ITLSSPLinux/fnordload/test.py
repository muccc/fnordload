import RepeatTimer

def hello():
	print "Hello World!"
 
r = RepeatTimer.RepeatTimer(5.0, hello, 3)
r.start()
