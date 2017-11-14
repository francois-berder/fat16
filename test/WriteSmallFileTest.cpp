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
    if (fat16_init(linux_dev, 0) < 0)
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
