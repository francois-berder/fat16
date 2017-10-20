#include <fstream>
#include "Common.hpp"
#include "ReadSmallFileTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"

ReadSmallFileTest::ReadSmallFileTest():
Test("ReadSmallFileTest"),
m_content("Hello, World!")
{
}

void ReadSmallFileTest::init()
{
    restore_image();
    mount_image();
    create_small_file("HELLO.TXT", m_content);
    system("mkdir /mnt/TMP");
    create_small_file("/TMP/HELLO.TXT", m_content);
    unmount_image();
    load_image();
}

bool ReadSmallFileTest::run()
{
    if (fat16_init(linux_dev) < 0)
        return false;

    {
        int fd = fat16_open("HELLO.TXT", 'r');
        if (fd < 0)
            return false;

        char buf[20];
        int ret = fat16_read(fd, buf, sizeof(buf));
        if (ret < 0)
            return false;

        buf[ret] = '\0';
        if (m_content != std::string(buf))
            return false;

        if (fat16_close(fd) < 0)
            return false;
    }

    {
        int fd = fat16_open("/TMP/HELLO.TXT", 'r');
        if (fd < 0)
            return false;

        char buf[20];
        int ret = fat16_read(fd, buf, sizeof(buf));
        if (ret < 0)
            return false;

        buf[ret] = '\0';
        if (m_content != std::string(buf))
            return false;

        if (fat16_close(fd) < 0)
            return false;
    }

    return true;
}

void ReadSmallFileTest::create_small_file(const std::string &filename, const std::string &content)
{
    std::string path = "/mnt/";
    path += filename;
    std::ofstream file(path);
    file << content;
    file.close();
}
