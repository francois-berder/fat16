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
#include "DeleteFileTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"


DeleteFileTest::DeleteFileTest():
Test("DeleteFileTest")
{

}

void DeleteFileTest::init()
{
    restore_image();
    mount_image();
    system("echo \"Hello World\" > /mnt/HELLO.TXT");
    system("mkdir /mnt/TMP");
    system("echo \"Hello World\" > /mnt/TMP/HELLO.TXT");
    unmount_image();
    load_image();
}

bool DeleteFileTest::run()
{
    if (fat16_init(linux_dev, 0) < 0)
        return false;


    if (fat16_rm("HELLO.TXT") < 0)
        return false;

    if (fat16_rm("/TMP/HELLO.TXT") < 0)
        return false;

    if (file_exists("HELLO.TXT"))
        return false;

    if (file_exists("/TMP/HELLO.TXT"))
        return false;

    return true;
}

bool DeleteFileTest::file_exists(const std::string &filepath)
{
    bool exists = false;
    mount_image();

    std::string path = "/mnt/" + filepath;
    std::ifstream file(path);
    exists = file.good();
    unmount_image();

    return exists;
}
