#include <cstdlib>
#include <fstream>
#include "Common.hpp"
#include "WriteEraseContentTest.hpp"
#include "../driver/fat16.h"


WriteEraseContentTest::WriteEraseContentTest():
Test("WriteEraseContentTest")
{
}

void WriteEraseContentTest::init()
{
    restore_image();
    mount_image();
    system("echo \"Hello World\" > /mnt/HELLO.TXT");
    unmount_image();
    load_image();
}

bool WriteEraseContentTest::run()
{
    if (fat16_init() < 0)
        return false;

    int fd = fat16_open("HELLO.TXT", 'w');
    if (fd < 0)
        return false;

    if (fat16_close(fd) < 0)
        return false;

    return check_file_is_empty();
}

bool WriteEraseContentTest::check_file_is_empty()
{
    bool result = true;
    mount_image();
    std::ifstream file("/mnt/HELLO.TXT", std::ifstream::ate | std::ifstream::binary);
    if (!file)
        result = false;

    if (result && file.tellg() != 0)
        result = false;

    file.close();

    unmount_image();

    return result;
}
