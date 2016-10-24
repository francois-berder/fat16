#include <fstream>
#include "Common.hpp"
#include "ReadEmptyFileTest.hpp"
#include "../driver/fat16.h"


ReadEmptyFileTest::ReadEmptyFileTest():
Test("ReadEmptyFileTest")
{

}

void ReadEmptyFileTest::init()
{
    restore_image();
    create_empty_file("HELLO.TXT");
    load_image();
}

bool ReadEmptyFileTest::run()
{
    if (fat16_init() < 0)
        return false;

    int fd = fat16_open("HELLO.TXT", 'r');
    if (fd < 0)
        return false;

    char buf;
    int ret = fat16_read(fd, &buf, 1);
    if (-ret != END_OF_FILE_REACHED)
        return false;

    if (fat16_close(fd) < 0)
        return false;

    return true;
}

void ReadEmptyFileTest::create_empty_file(const std::string &filename)
{
    mount_image();

    std::string path = "/mnt/";
    path += filename;
    std::ofstream file(path);
    file.close();

    unmount_image();
}
