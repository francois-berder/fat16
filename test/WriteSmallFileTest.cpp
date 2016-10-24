#include <cstdlib>
#include <fstream>
#include "Common.hpp"
#include "WriteSmallFileTest.hpp"
#include "../driver/fat16.h"


WriteSmallFileTest::WriteSmallFileTest():
Test("WriteSmallFileTest")
{
}

bool WriteSmallFileTest::run()
{
    if (fat16_init() < 0)
        return false;

    int fd = fat16_open("HELLO.TXT", 'w');
    if (fd < 0)
        return false;

    std::string content = "Hello World!";
    int ret = fat16_write(fd, (char*)content.c_str(), content.length());
    if (ret != content.length())
        return false;

    if (fat16_close(fd) < 0)
        return false;

    return check_content_file("HELLO.TXT", content);
}

bool WriteSmallFileTest::check_content_file(const std::string &filename, const std::string &content)
{
    bool result = true;

    mount_image();

    std::string path = "/mnt/";
    path += filename;
    std::ifstream file(path);

    file.seekg(0, std::ios::end);
    if (file.tellg() != content.length())
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
