from struct import *

command = pack('BBBB', 0x7F, 0x80, 0x01, 0x11)

print command

unpacked = unpack('BBBB', command)

print unpacked
print unpacked.__class__
print len(unpacked) #4

print len(command) #4

print unpacked[0]
