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


#include <sys/stat.h>
#include "Common.hpp"
#include "DeleteDirectoryTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"


DeleteDirectoryTest::DeleteDirectoryTest():
Test("DeleteDirectoryTest")
{

}

void DeleteDirectoryTest::init()
{
    restore_image();
    mount_image();
    system("mkdir /mnt/DOCS");
    system("mkdir /mnt/DOCS/MUSIC");
    unmount_image();
    load_image();
}

bool DeleteDirectoryTest::run()
{
    if (fat16_init(linux_dev, 0) < 0)
        return false;


    if (fat16_rmdir("/DOCS/MUSIC") < 0)
        return false;

    if (fat16_rmdir("/DOCS") < 0)
        return false;

    if (directory_exists("/DOCS/MUSIC"))
        return false;

    if (directory_exists("/DOCS"))
        return false;

    return true;
}

bool DeleteDirectoryTest::directory_exists(const std::string &dirpath)
{
    bool exists = false;
    mount_image();

    std::string path = "/mnt/" + dirpath;
    struct stat info;
    exists = stat(path.c_str(), &info) == 0;
    unmount_image();

    return exists;
}
