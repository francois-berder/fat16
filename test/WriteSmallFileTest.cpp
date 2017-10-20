#include <cstdlib>
#include <fstream>
#include "Common.hpp"
#include "WriteSmallFileTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"


WriteSmallFileTest::WriteSmallFileTest():
Test("WriteSmallFileTest")
{
}

void WriteSmallFileTest::init()
{
    restore_image();
    mount_image();
    system("mkdir /mnt/TMP");
    unmount_image();
    load_image();
}

bool WriteSmallFileTest::run()
{
    if (fat16_init(linux_dev) < 0)
        return false;

    {
        int fd = fat16_open("HELLO.TXT", 'w');
        if (fd < 0)
            return false;

        std::string content = "Hello World!";
        int ret = fat16_write(fd, (char*)content.c_str(), content.length());
        if (ret != (int)content.length())
            return false;

        if (fat16_close(fd) < 0)
            return false;

        if (!check_content_file("HELLO.TXT", content))
            return false;
    }

    {
        int fd = fat16_open("/TMP/HELLO.TXT", 'w');
        if (fd < 0)
            return false;

        std::string content = "Hello World!";
        int ret = fat16_write(fd, (char*)content.c_str(), content.length());
        if (ret != (int)content.length())
            return false;

        if (fat16_close(fd) < 0)
            return false;

        if (!check_content_file("/TMP/HELLO.TXT", content))
            return false;
    }

    return true;
}

bool WriteSmallFileTest::check_content_file(const std::string &filename, const std::string &content)
{
    bool result = true;

    mount_image();

    std::string path = "/mnt/";
    path += filename;
    std::ifstream file(path);

    file.seekg(0, std::ios::end);
    if (file.tellg() != (int)content.length())
        result = false;

    file.seekg(0, std::ios::beg);
    std::string file_content;
    std::getline(file, file_content);
    if (content != file_content)
        result = false;

    file.close();
    unmount_image();
    return result;
}
