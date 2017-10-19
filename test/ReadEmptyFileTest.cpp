#include <fstream>
#include "Common.hpp"
#include "ReadEmptyFileTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"


ReadEmptyFileTest::ReadEmptyFileTest():
Test("ReadEmptyFileTest")
{

}

void ReadEmptyFileTest::init()
{
    restore_image();
    mount_image();
    system("touch /mnt/HELLO.TXT");
    unmount_image();
    load_image();
}

bool ReadEmptyFileTest::run()
{
    if (fat16_init(linux_dev) < 0)
        return false;

    int fd = fat16_open("HELLO.TXT", 'r');
    if (fd < 0)
        return false;

    char buf;
    int ret = fat16_read(fd, &buf, 1);
    if (ret != 0)
        return false;

    if (fat16_close(fd) < 0)
        return false;

    return true;
}
