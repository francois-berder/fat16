import ctypes
import os.path
import sys

# Load FAT16 library
dll_name = "libfat16_driver.so"
dllabspath = os.path.dirname(os.path.abspath(__file__)) + os.path.sep + dll_name
_LIB = ctypes.CDLL(dllabspath)

test_records = {}

def record_test_result(test_name, result):
    test_records[test_name] = result

def test_init():
    print('----- init -----')
    ret = _LIB.fat16_init()
    record_test_result('init', ret == 0)

def main(argv):
    _LIB.linux_load_image(str.encode(argv[1]))

    # Perform tests
    test_init()

    _LIB.linux_release_image()

if __name__ == "__main__":
    main(sys.argv)

    # Print test results
    print('\n\n\n##### TEST RESULTS #####')
    for test_name, test_result in test_records.items():
        print('{}: {}'.format(test_name, 'PASS' if test_result else 'FAIL'))
