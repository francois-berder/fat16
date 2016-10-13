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

def unmount_image():
    subprocess.run(['umount', '/mnt'])

def mount_image(image_path):
    subprocess.run(['mount', image_path, '/mnt'])

def delete_file(filename):
    subprocess.run(['rm', '-f', '/mnt/' + filename])

def create_empty_file(image_path, filename):
    mount_image(image_path)
    delete_file(filename)
    subprocess.run(['touch', '/mnt/' + filename])
    unmount_image()

def create_small_file(image_path, filename, content):
    mount_image(image_path)
    small_file = open('/mnt/' + filename, 'w')
    small_file.write(content)
    small_file.close()
    unmount_image()

def test_init(image_path):
    restore_image(image_path)
    _LIB.linux_load_image(str.encode(image_path))
    print('----- init -----')
    ret = _LIB.fat16_init()
    record_test_result('init', ret == 0)
    _LIB.linux_release_image()

def test_read_empty_file(image_path):
    restore_image(image_path)
    create_empty_file(image_path, 'HELLO.TXT')
    _LIB.linux_load_image(str.encode(image_path))
    print('----- read empty file -----')
    mode = (ctypes.c_char)(str.encode('r'))
    fd = _LIB.fat16_open(str.encode('HELLO.TXT'), mode)
    if fd < 0:
        record_test_result('read_empty_file', False)

    buf = (ctypes.c_uint8 * 1)()
    ret = _LIB.fat16_read(fd, buf, 1)
    _LIB.linux_release_image()
    record_test_result('read_empty_file', ret == -9)

def main(argv):
    image_path = 'data/fs.img'
    if len(argv) > 1:
        image_path = argv[1]

    # Perform tests
    test_init(image_path)
    test_read_empty_file(image_path)
    test_read_small_file(image_path)

    # Print test results
    print('\n\n\n##### TEST RESULTS #####')
    for test_name, test_result in test_records.items():
        print('{}: {}'.format(test_name, 'PASS' if test_result else 'FAIL'))

if __name__ == "__main__":
    main(sys.argv)
