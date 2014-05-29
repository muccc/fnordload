import ctypes
def sync():
    ctypes.CDLL("libc.so.6").sync()

