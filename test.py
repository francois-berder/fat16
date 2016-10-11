import ctypes
import os.path
import sys

# Load FAT16 library
dll_name = "libfat16_driver.so"
dllabspath = os.path.dirname(os.path.abspath(__file__)) + os.path.sep + dll_name
_LIB = ctypes.CDLL(dllabspath)


def main(argv):
    _LIB.linux_load_image(str.encode(argv[1]))

    # Perform tests

    _LIB.linux_release_image()

if __name__ == "__main__":
    main(sys.argv)

