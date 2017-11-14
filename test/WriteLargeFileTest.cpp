/*
 * Copyright (C) 2017  Francois Berder <fberder@outlook.fr>
 *
 * This file is part of fat16.
 *
 * fat16 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fat16 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fat16.  If not, see <http://www.gnu.org/licenses/>.
 */


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

void WriteLargeFileTest::init()
{
    restore_image();
    mount_image();
    system("mkdir /mnt/TMP");
    unmount_image();
    load_image();
}

bool WriteLargeFileTest::run()
{
    if (fat16_init(linux_dev, 0) < 0)
        return false;

    {
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

        if (!check_content_file("HELLO.TXT"))
            return false;
    }

    {
        int fd = fat16_open("/TMP/HELLO.TXT", 'w');
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

        if (!check_content_file("/TMP/HELLO.TXT"))
            return false;
    }

    return true;
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
