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
    if (fat16_init(linux_dev, 0) < 0)
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
    std::string path = "/mnt/" + filepath;
    std::ifstream file(path, std::ifstream::ate | std::ifstream::binary);
    if (!file)
        result = false;

    if (result && file.tellg() != 0)
        result = false;

    file.close();

    unmount_image();

    return result;
}
