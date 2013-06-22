from struct import *

stuff = pack('b', 127)

print stuff

unpacked = unpack('b', stuff)

print unpacked
print unpacked.__class__
print len(unpacked)

stuff = stuff + pack('b', 0)

print stuff
