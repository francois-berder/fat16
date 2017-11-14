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
    if (fat16_init(linux_dev, 0) < 0)
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
