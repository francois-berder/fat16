#!/usr/bin/env python3

import ctypes
import os.path
import sys
import subprocess

# Load FAT16 library
dll_name = "libfat16_driver.so"
dllabspath = os.path.dirname(os.path.abspath(__file__)) + os.path.sep + dll_name
_LIB = ctypes.CDLL(dllabspath)

test_records = {}

def record_test_result(test_name, result):
    test_records[test_name] = result

def restore_image(image_path):
    subprocess.run(['git', 'checkout' , image_path])

def mount_image():
    subprocess.run(['mount', image_path, '/mnt'])

def unmount_image():
    subprocess.run(['unmount', '/mnt'])


def test_init(image_path):
    restore_image(image_path)
    print('----- init -----')
    ret = _LIB.fat16_init()
    record_test_result('init', ret == 0)


def main(argv):
    image_path = 'data/fs.img'
    if len(argv) > 1:
        image_path = argv[1]

    _LIB.linux_load_image(str.encode(image_path))

    # Perform tests
    test_init(image_path)

    _LIB.linux_release_image()

    # Print test results
    print('\n\n\n##### TEST RESULTS #####')
    for test_name, test_result in test_records.items():
        print('{}: {}'.format(test_name, 'PASS' if test_result else 'FAIL'))

if __name__ == "__main__":
    main(sys.argv)
