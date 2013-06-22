import serial

ser = serial.Serial('/dev/ttyACM0', 9600)
print ser.portstr

ser.write('\x7F\x80\x01\x11\x65\x82')
line = ser.read(6)
print ' '.join([hex(ord(i)) for i in line])
ser.close()

