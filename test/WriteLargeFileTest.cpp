#include <cstdlib>
#include <fstream>
#include "Common.hpp"
#include "WriteLargeFileTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"

WriteLargeFileTest::WriteLargeFileTest(unsigned int bytes_count):
Test(std::string("WriteLargeFileTest (") + std::to_string(bytes_count) + std::string(")")),
m_bytes_count(bytes_count)
{

}

bool WriteLargeFileTest::run()
{
    if (fat16_init(linux_dev) < 0)
        return false;

    int fd = fat16_open("HELLO.TXT", 'w');
    if (fd < 0)
        return false;

    srand(1);
    for (unsigned int i = 0; i < m_bytes_count; ++i) {
        char buf = rand();
        int ret = fat16_write(fd, &buf, sizeof(buf));
        if (ret != 1)
            return false;
    }

    if (fat16_close(fd) < 0)
        return false;

    return check_content_file("HELLO.TXT");
}

bool WriteLargeFileTest::check_content_file(const std::string &filename)
{
    bool result = true;

    mount_image();

    std::string path = "/mnt/";
    path += filename;
    std::ifstream file(path);
    if (!file)
        return false;

    file.seekg(0, std::ios::end);
    if (file.tellg() != m_bytes_count)
        result = false;
    else {
        file.seekg(0, std::ios::beg);
        srand(1);
        for (unsigned int i = 0; i < m_bytes_count; ++i) {
            char expected = rand();
            char buf = 0;
            file.get(buf);
            if (!file || buf != expected) {
                printf("Found %02x but expected %02X\n", buf, expected);
                result = false;
                break;
            }
        }
    }

    file.close();
    unmount_image();

    return result;
}
