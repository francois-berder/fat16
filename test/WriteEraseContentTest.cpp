#include <cstdlib>
#include <fstream>
#include "Common.hpp"
#include "WriteEraseContentTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"


WriteEraseContentTest::WriteEraseContentTest():
Test("WriteEraseContentTest")
{
}

void WriteEraseContentTest::init()
{
    restore_image();
    mount_image();
    system("echo \"Hello World\" > /mnt/HELLO.TXT");
    system("mkdir /mnt/TMP");
    system("echo \"Hello World\" > /mnt/TMP/HELLO.TXT");
    unmount_image();
    load_image();
}

bool WriteEraseContentTest::run()
{
    if (fat16_init(linux_dev) < 0)
        return false;

    int fd = fat16_open("HELLO.TXT", 'w');
    if (fd < 0)
        return false;

    if (fat16_close(fd) < 0)
        return false;

    if (!check_file_is_empty("HELLO.TXT"))
        return false;

    fd = fat16_open("/TMP/HELLO.TXT", 'w');
    if (fd < 0)
        return false;

    if (fat16_close(fd) < 0)
        return false;

    if (!check_file_is_empty("/TMP/HELLO.TXT"))
        return false;

    return true;
}

bool WriteEraseContentTest::check_file_is_empty(const std::string &filepath)
{
    bool result = true;
    mount_image();
    std::ifstream file(filepath, std::ifstream::ate | std::ifstream::binary);
    if (!file)
        result = false;

    if (result && file.tellg() != 0)
        result = false;

    file.close();

    unmount_image();

    return result;
}
